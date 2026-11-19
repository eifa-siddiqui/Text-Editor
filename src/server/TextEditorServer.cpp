#include "../../include/core/Document.h"
#include "../../include/algorithms/Search.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <commdlg.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>

#pragma comment(lib, "ws2_32.lib")

static Document document;
static std::string currentFilename = "";
static bool documentDirty = false;
static std::string serverDirectory()
{
    char executable[MAX_PATH] = {};
    DWORD length = GetModuleFileNameA(nullptr, executable, MAX_PATH);
    if (length > 0)
    {
        std::string path(executable);
        size_t slash = path.find_last_of("\\/");
        if (slash != std::string::npos)
            return path.substr(0, slash);
    }
    return "";
}

static std::string initialDirectoryFor(const std::string& name)
{
    if (!name.empty())
    {
        size_t slash = name.find_last_of("\\/");
        if (slash != std::string::npos)
            return name.substr(0, slash);
    }
    return serverDirectory();
}

static std::string nativeSaveAs(const std::string& currentName)
{
    char buffer[MAX_PATH] = {};
    std::string initialDir = initialDirectoryFor(currentName);
    if (!currentName.empty())
    {
        std::snprintf(buffer, sizeof(buffer), "%s", currentName.c_str());
    }
    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetForegroundWindow();
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = sizeof(buffer);
    dialog.lpstrInitialDir = initialDir.c_str();
    dialog.lpstrDefExt = "txt";
    dialog.lpstrFilter =
        "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    dialog.nFilterIndex = 1;
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (GetSaveFileNameA(&dialog))
        return std::string(buffer);
    return "";
}

static std::string nativeSavePdf(const std::string& currentName)
{
    char buffer[MAX_PATH] = {};
    std::string initialDir = initialDirectoryFor(currentName);
    if (!currentName.empty())
    {
        std::string name = currentName;
        size_t dot = name.find_last_of(".");
        if (dot != std::string::npos) name = name.substr(0, dot);
        name += ".pdf";
        std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "document.pdf");
    }

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetForegroundWindow();
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = sizeof(buffer);
    dialog.lpstrInitialDir = initialDir.c_str();
    dialog.lpstrDefExt = "pdf";
    dialog.lpstrFilter =
        "PDF Files (*.pdf)\0*.pdf\0All Files (*.*)\0*.*\0";
    dialog.nFilterIndex = 1;
    dialog.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (GetSaveFileNameA(&dialog))
        return std::string(buffer);
    return "";
}

static std::string nativeOpen()
{
    char buffer[MAX_PATH] = {};
    std::string initialDir = initialDirectoryFor(currentFilename);
    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetForegroundWindow();
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = sizeof(buffer);
    dialog.lpstrInitialDir = initialDir.c_str();
    dialog.lpstrFilter =
        "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    dialog.nFilterIndex = 1;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
    if (GetOpenFileNameA(&dialog))
        return std::string(buffer);
    return "";
}

static std::string jsonEscape(const std::string& value)
{
    std::string out;
    for (unsigned char c : value)
    {
        switch (c)
        {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 32)
            {
                char buffer[7];
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                out += buffer;
            }
            else
                out += (char)c;
        }
    }
    return out;
}

static std::string decodeUrl(std::string value)
{
    std::string out;
    for (size_t i = 0; i < value.size(); ++i)
    {
        if (value[i] == '%' && i + 2 < value.size())
        {
            unsigned int decoded;
            if (std::sscanf(value.substr(i + 1, 2).c_str(), "%x", &decoded) == 1)
            {
                out += (char)decoded;
                i += 2;
                continue;
            }
        }
        if (value[i] == '+') out += ' ';
        else out += value[i];
    }
    return out;
}

static std::string queryValue(const std::string& query, const std::string& key)
{
    std::string needle = key + "=";
    size_t start = query.find(needle);
    if (start == std::string::npos) return "";
    start += needle.size();
    size_t end = query.find('&', start);
    return decodeUrl(query.substr(start, end == std::string::npos ? std::string::npos : end - start));
}

static std::string jsonValue(const std::string& body, const std::string& key)
{
    std::string needle = "\"" + key + "\"";
    size_t start = body.find(needle);
    if (start == std::string::npos) return "";
    start = body.find(':', start);
    if (start == std::string::npos) return "";
    start++;
    while (start < body.size() && isspace((unsigned char)body[start])) start++;

    if (body[start] == '"')
    {
        start++;
        std::string out;
        while (start < body.size())
        {
            if (body[start] == '\\' && start + 1 < body.size())
            {
                char c = body[start + 1];
                if (c == 'n') out += '\n';
                else if (c == 'r') out += '\r';
                else if (c == 't') out += '\t';
                else out += c;
                start += 2;
            }
            else if (body[start] == '"')
                break;
            else
                out += body[start++];
        }
        return out;
    }
    return body.substr(start, body.find_first_of(",}", start) - start);
}

static bool jsonBool(const std::string& body, const std::string& key)
{
    std::string value = jsonValue(body, key);
    return value == "true";
}

static std::vector<std::string> splitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    if (!text.empty() && text.back() == '\n')
        lines.push_back("");
    if (lines.empty()) lines.push_back("");
    return lines;
}

static std::string textFromDocument();

static void setDocumentText(const std::string& text)
{
    if (textFromDocument() != text) documentDirty = true;
    std::vector<std::string> oldLines = document.getAllLines();
    std::vector<std::string> newLines = splitLines(text);
    size_t common = (oldLines.size() < newLines.size()) ? oldLines.size() : newLines.size();

    for (size_t i = 0; i < common; ++i)
    {
        if (oldLines[i] != newLines[i])
            document.updateLine((int)i + 1, newLines[i]);
    }

    if (newLines.size() > oldLines.size())
    {
        for (size_t i = oldLines.size(); i < newLines.size(); ++i)
            document.insertLine(newLines[i], (int)i + 1);
    }
    else
    {
        for (int i = (int)oldLines.size(); i > (int)newLines.size(); --i)
            document.deleteLine(i);
    }
}

static std::string documentJson()
{
    std::string text;
    std::vector<std::string> lines = document.getAllLines();
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (i) text += '\n';
        text += lines[i];
    }
    return "{\"text\":\"" + jsonEscape(text) +
           "\",\"lines\":" + std::to_string(document.getLineCount()) +
           ",\"characters\":" + std::to_string(document.getCharacterCount()) +
           ",\"undoCount\":" + std::to_string(document.undoCount()) +
           ",\"redoCount\":" + std::to_string(document.redoCount()) +
           ",\"dirty\":" + std::string(documentDirty ? "true" : "false") +
           ",\"filename\":\"" + jsonEscape(currentFilename) + "\"}";
}

static std::string textFromDocument()
{
    std::string text;
    std::vector<std::string> lines = document.getAllLines();
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (i) text += '\n';
        text += lines[i];
    }
    return text;
}

static std::string documentResultJson(bool success)
{
    std::string text = textFromDocument();
    return "{\"success\":" + std::string(success ? "true" : "false") +
           ",\"text\":\"" + jsonEscape(text) + "\"" +
           ",\"lines\":" + std::to_string(document.getLineCount()) +
           ",\"characters\":" + std::to_string(document.getCharacterCount()) +
           ",\"undoCount\":" + std::to_string(document.undoCount()) +
           ",\"redoCount\":" + std::to_string(document.redoCount()) + "}";
}

static std::string sendResponse(SOCKET client, const std::string& status,
                                const std::string& contentType,
                                const std::string& body)
{
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Cache-Control: no-store\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Connection: close\r\n\r\n"
             << body;
    std::string bytes = response.str();
    send(client, bytes.data(), (int)bytes.size(), 0);
    return "";
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(8080);

    if (bind(server, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR ||
        listen(server, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Unable to start Text Editor server on port 8080\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }

    std::cout << "Text Editor C++ server running at http://127.0.0.1:8080\n";

    while (true)
    {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET) continue;

        std::string request;
        char buffer[4096];
        int received;
        while ((received = recv(client, buffer, sizeof(buffer), 0)) > 0)
        {
            request.append(buffer, received);
            if (request.find("\r\n\r\n") != std::string::npos)
            {
                size_t headerEnd = request.find("\r\n\r\n") + 4;
                size_t lengthPos = request.find("Content-Length:");
                if (lengthPos != std::string::npos)
                {
                    size_t valueStart = lengthPos + 15;
                    size_t valueEnd = request.find("\r\n", valueStart);
                    int contentLength = std::stoi(request.substr(valueStart, valueEnd - valueStart));
                    if ((int)request.size() >= (int)headerEnd + contentLength) break;
                }
                else break;
            }
        }

        size_t headerEnd = request.find("\r\n\r\n");
        std::string body = headerEnd == std::string::npos ? "" : request.substr(headerEnd + 4);
        std::istringstream lines(request);
        std::string method, path, protocol;
        lines >> method >> path >> protocol;
        size_t queryPos = path.find('?');
        std::string route = queryPos == std::string::npos ? path : path.substr(0, queryPos);
        std::string query = queryPos == std::string::npos ? "" : path.substr(queryPos + 1);

        if (method == "OPTIONS")
        {
            sendResponse(client, "204 No Content", "text/plain", "");
        }
        else if (route == "/api/document" && method == "GET")
        {
            sendResponse(client, "200 OK", "application/json", documentJson());
        }
        else if (route == "/api/set" && method == "POST")
        {
            setDocumentText(jsonValue(body, "text"));
            sendResponse(client, "200 OK", "application/json", documentJson());
        }
        else if (route == "/api/open-text" && method == "POST")
        {
            setDocumentText(jsonValue(body, "text"));
            document.clearHistory();
            sendResponse(client, "200 OK", "application/json", documentJson());
        }
        else if (route == "/api/new" && method == "POST")
        {
            document.newDocument();
            currentFilename = "";
            documentDirty = false;
            sendResponse(client, "200 OK", "application/json", documentJson());
        }
        else if (route == "/api/search" && method == "GET")
        {
            std::string pattern = queryValue(query, "pattern");
            SearchOptions options;
            options.caseSensitive = queryValue(query, "caseSensitive") == "true";
            options.wholeWord = queryValue(query, "wholeWord") == "true";
            Search search;
            SearchStats stats;
            std::vector<SearchResult> results;
            if (queryValue(query, "algorithm") == "BM")
                results = search.boyerMooreSearch(document.getAllLines(), pattern, options, stats);
            else
                results = search.searchText(document.getAllLines(), pattern, options, stats);

            std::string matches = "[";
            for (size_t i = 0; i < results.size(); ++i)
            {
                if (i) matches += ",";
                matches += "{\"line\":" + std::to_string(results[i].lineNumber) +
                           ",\"column\":" + std::to_string(results[i].columnNumber) + "}";
            }
            matches += "]";
            std::string json = "{\"matches\":" + matches +
                               ",\"comparisons\":" + std::to_string(stats.comparisons) +
                               ",\"algorithm\":\"" + jsonEscape(queryValue(query, "algorithm")) + "\"}";
            sendResponse(client, "200 OK", "application/json", json);
        }
        else if (route == "/api/replace" && method == "POST")
        {
            std::string pattern = jsonValue(body, "pattern");
            std::string replacement = jsonValue(body, "replacement");
            SearchOptions options;
            options.caseSensitive = jsonBool(body, "caseSensitive");
            options.wholeWord = jsonBool(body, "wholeWord");
            int count = 0;
            std::string occurrence = jsonValue(body, "occurrence");
            if (!occurrence.empty())
                document.replaceOccurrence(pattern, replacement, std::stoi(occurrence), options);
            else
                count = document.replaceAll(pattern, replacement, options);
            sendResponse(client, "200 OK", "application/json",
                         "{\"count\":" + std::to_string(count) + "," +
                         "\"text\":\"" + jsonEscape(textFromDocument()) + "\"" +
                         ",\"undoCount\":" + std::to_string(document.undoCount()) +
                         ",\"redoCount\":" + std::to_string(document.redoCount()) + "}");
        }
        else if (route == "/api/undo" && method == "POST")
        {
            bool changed = document.undo();
            sendResponse(client, "200 OK", "application/json",
                         "{\"changed\":" + std::string(changed ? "true" : "false") + "," +
                         "\"text\":\"" + jsonEscape(textFromDocument()) + "\"" +
                         ",\"undoCount\":" + std::to_string(document.undoCount()) +
                         ",\"redoCount\":" + std::to_string(document.redoCount()) + "}");
        }
        else if (route == "/api/redo" && method == "POST")
        {
            bool changed = document.redo();
            sendResponse(client, "200 OK", "application/json",
                         "{\"changed\":" + std::string(changed ? "true" : "false") + "," +
                         "\"text\":\"" + jsonEscape(textFromDocument()) + "\"" +
                         ",\"undoCount\":" + std::to_string(document.undoCount()) +
                         ",\"redoCount\":" + std::to_string(document.redoCount()) + "}");
        }
        else if (route == "/api/line" && method == "POST")
        {
            std::string operation = jsonValue(body, "operation");
            int position = 0;
            std::string positionText = jsonValue(body, "position");
            if (!positionText.empty()) position = std::stoi(positionText);
            bool success = false;

            if (operation == "insert")
                success = document.insertLine(jsonValue(body, "text"), position);
            else if (operation == "delete")
                success = document.deleteLine(position);
            else if (operation == "update")
                success = document.updateLine(position, jsonValue(body, "text"));
            else if (operation == "deleteRange")
            {
                int last = std::stoi(jsonValue(body, "last"));
                success = document.deleteLines(position, last);
            }

            sendResponse(client, "200 OK", "application/json", documentResultJson(success));
        }
        else if (route == "/api/text" && method == "POST")
        {
            std::string operation = jsonValue(body, "operation");
            int line = std::stoi(jsonValue(body, "line"));
            int column = std::stoi(jsonValue(body, "column"));
            bool success = false;

            if (operation == "insert")
                success = document.insertText(line, column, jsonValue(body, "text"));
            else if (operation == "delete")
            {
                int length = std::stoi(jsonValue(body, "length"));
                success = document.deleteText(line, column, length);
            }

            sendResponse(client, "200 OK", "application/json", documentResultJson(success));
        }
        else if (route == "/api/stats" && method == "GET")
        {
            sendResponse(client, "200 OK", "application/json", documentJson());
        }
        else if (route == "/api/frequency" && method == "GET")
        {
            int count = document.getWordFrequency(queryValue(query, "word"));
            sendResponse(client, "200 OK", "application/json",
                         "{\"count\":" + std::to_string(count) + "}");
        }
        else if (route == "/api/frequencies" && method == "GET")
        {
            std::vector<std::pair<std::string, int>> frequencies = document.getWordFrequencies();
            std::string json = "[";
            for (size_t i = 0; i < frequencies.size(); ++i)
            {
                if (i) json += ",";
                json += "{\"word\":\"" + jsonEscape(frequencies[i].first) +
                       "\",\"count\":" + std::to_string(frequencies[i].second) + "}";
            }
            json += "]";
            sendResponse(client, "200 OK", "application/json", json);
        }
        else if (route == "/api/save" && method == "POST")
        {
            std::string filename = jsonValue(body, "filename");
            if (body.find("\"text\"") != std::string::npos)
                setDocumentText(jsonValue(body, "text"));
            bool saved = false;
            if (filename.empty())
            {
                if (currentFilename.empty())
                    filename = nativeSaveAs("document.txt");
                else
                    filename = currentFilename;
            }

            if (!filename.empty())
            {
                saved = document.saveToFile(filename);
                if (saved) {
                    currentFilename = filename;
                    documentDirty = false;
                }
            }
            sendResponse(client, "200 OK", "application/json",
                         "{\"saved\":" + std::string(saved ? "true" : "false") +
                         ",\"filename\":\"" + jsonEscape(filename) + "\"}");
        }
        else if (route == "/api/save-as" && method == "POST")
        {
            if (body.find("\"text\"") != std::string::npos)
                setDocumentText(jsonValue(body, "text"));
            std::string filename = nativeSaveAs(jsonValue(body, "filename"));
            bool saved = false;
            if (!filename.empty())
            {
                saved = document.saveToFile(filename);
                if (saved) {
                    currentFilename = filename;
                    documentDirty = false;
                }
            }
            sendResponse(client, "200 OK", "application/json",
                         "{\"saved\":" + std::string(saved ? "true" : "false") +
                         ",\"filename\":\"" + jsonEscape(filename) + "\"}");
        }
        else if (route == "/api/open-dialog" && method == "POST")
        {
            std::string filename = nativeOpen();
            if (filename.empty())
            {
                sendResponse(client, "200 OK", "application/json",
                             "{\"opened\":false,\"cancelled\":true}");
            }
            else
            {
                bool opened = document.openFromFile(filename);
                if (opened) {
                    currentFilename = filename;
                    documentDirty = false;
                }
                sendResponse(client, "200 OK", "application/json",
                             "{\"opened\":" + std::string(opened ? "true" : "false") +
                             ",\"cancelled\":false,\"filename\":\"" + jsonEscape(filename) + "\"" +
                             "," + documentJson().substr(1));
            }
        }
        else if (route == "/api/open" && method == "POST")
        {
            std::string filename = jsonValue(body, "filename");
            bool opened = document.openFromFile(filename);
            sendResponse(client, "200 OK", "application/json",
                         opened ? documentJson() : "{\"error\":\"Unable to open file\"}");
        }
        else if (route == "/api/pdf" && method == "POST")
        {
            if (body.find("\"text\"") != std::string::npos)
                setDocumentText(jsonValue(body, "text"));
            std::string filename = nativeSavePdf(currentFilename);
            if (filename.empty())
            {
                sendResponse(client, "200 OK", "application/json",
                             "{\"exported\":false,\"filename\":\"\",\"cancelled\":true}");
            }
            else
            {
            bool exported = document.exportToPDF(filename);
            sendResponse(client, "200 OK", "application/json",
                         "{\"exported\":" + std::string(exported ? "true" : "false") +
                         ",\"filename\":\"" + jsonEscape(filename) +
                         "\",\"cancelled\":false}");
            }
        }
        else
        {
            std::ifstream file("." + route);
            if (route == "/") file.open("ui/index.html", std::ios::binary);
            if (file.is_open())
            {
                std::string body((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
                sendResponse(client, "200 OK",
                             route == "/" || route.find(".html") != std::string::npos
                                 ? "text/html" : "application/octet-stream", body);
            }
            else
            {
                sendResponse(client, "404 Not Found", "text/plain", "Not found");
            }
        }

        closesocket(client);
    }
}
