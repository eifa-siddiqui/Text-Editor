
#include "../../include/core/Document.h"
#include "../../include/algorithms/Replace.h"
#include <iostream>
#include <sstream>

using namespace std;

Document::Document()
{
}

void Document::addLine(string text)
{
    insertLine(text, getLineCount() + 1);
}

bool Document::insertLine(string text, int position)
{
    if (position < 1 || position > getLineCount() + 1)
        return false;

    lines.insertAtPosition(text, position);

    Action action;
    action.type     = ActionType::INSERT_LINE;
    action.position = position;
    action.newText  = text;
    history.recordAction(std::move(action));

    return true;
}

bool Document::deleteLine(int position)
{
    if (position < 1 || position > getLineCount())
        return false;

    Action action;
    action.type     = ActionType::DELETE_LINE;
    action.position = position;
    action.oldText  = lines.getLine(position);

    lines.deleteAtPosition(position);
    history.recordAction(std::move(action));

    return true;
}

bool Document::deleteLines(int firstPosition, int lastPosition)
{
    if (firstPosition < 1 || lastPosition < firstPosition || lastPosition > getLineCount())
        return false;

    // Delete from the end so every recorded line position remains valid.
    for (int position = lastPosition; position >= firstPosition; --position)
        deleteLine(position);
    return true;
}

string Document::getLine(int position)
{
    return lines.getLine(position);
}

bool Document::updateLine(int position, string newText)
{
    if (position < 1 || position > getLineCount())
        return false;

    string oldText = lines.getLine(position);

    if (!lines.updateLine(position, newText))
        return false;

    Action action;
    action.type     = ActionType::UPDATE_LINE;
    action.position = position;
    action.oldText  = oldText;
    action.newText  = newText;
    history.recordAction(std::move(action));

    return true;
}

bool Document::insertText(int line, int column, string text)
{
    if (line < 1 || line > getLineCount()) return false;
    string oldText = lines.getLine(line);
    if (column < 1 || column > (int)oldText.size() + 1) return false;
    return updateLine(line, oldText.insert((size_t)(column - 1), text));
}

bool Document::deleteText(int line, int column, int length)
{
    if (line < 1 || line > getLineCount() || column < 1 || length < 1) return false;
    string oldText = lines.getLine(line);
    size_t start = (size_t)(column - 1);
    if (start >= oldText.size() || (size_t)length > oldText.size() - start) return false;
    oldText.erase(start, (size_t)length);
    return updateLine(line, oldText);
}

void Document::displayDocument()
{
    cout << "\n---------- DOCUMENT ----------" << endl;

    vector<string> all = lines.getAllLines();

    for (size_t i = 0; i < all.size(); i++)
        cout << (i + 1) << " | " << all[i] << '\n';

    cout << "Lines: " << all.size() << ", Characters: " << getCharacterCount() << '\n';

    cout << "------------------------------" << endl;
}

int Document::getLineCount()
{
    return lines.getSize();
}

int Document::getCharacterCount()
{
    int count = 0;
    for (const string& line : lines.getAllLines()) count += (int)line.size();
    return count;
}

bool Document::isEmpty()
{
    return lines.isEmpty();
}

vector<string> Document::getAllLines()
{
    return lines.getAllLines();
}

void Document::applyChanges(const vector<LineChange>& changes, bool useNewText)
{
    vector<pair<int, string>> updates;
    updates.reserve(changes.size());

    for (const LineChange& c : changes)
        updates.push_back({c.lineNumber, useNewText ? c.newText : c.oldText});

    lines.updateLines(updates);          // single pass over the list
}

bool Document::undo()
{
    if (!history.canUndo())
        return false;

    Action action = history.undo();

    switch (action.type)
    {
    case ActionType::INSERT_LINE:
        lines.deleteAtPosition(action.position);
        break;

    case ActionType::DELETE_LINE:
        lines.insertAtPosition(action.oldText, action.position);
        break;

    case ActionType::UPDATE_LINE:
        lines.updateLine(action.position, action.oldText);
        break;

    case ActionType::REPLACE:
        applyChanges(action.changes, false);
        break;

    case ActionType::NONE:
        return false;
    }

    return true;
}

bool Document::redo()
{
    if (!history.canRedo())
        return false;

    Action action = history.redo();

    switch (action.type)
    {
    case ActionType::INSERT_LINE:
        lines.insertAtPosition(action.newText, action.position);
        break;

    case ActionType::DELETE_LINE:
        lines.deleteAtPosition(action.position);
        break;

    case ActionType::UPDATE_LINE:
        lines.updateLine(action.position, action.newText);
        break;

    case ActionType::REPLACE:
        applyChanges(action.changes, true);
        break;

    case ActionType::NONE:
        return false;
    }

    return true;
}

int Document::undoCount() { return history.undoCount(); }

int Document::redoCount() { return history.redoCount(); }

int Document::replaceAll(string pattern, string replacement, SearchOptions opts)
{
    if (pattern.empty() || lines.isEmpty())
        return 0;

    Replace replacer;
    ReplaceResult result =
        replacer.replaceAll(lines.getAllLines(), pattern, replacement, opts);

    if (result.changes.empty())
        return 0;

    applyChanges(result.changes, true);

    Action action;
    action.type    = ActionType::REPLACE;
    action.changes = std::move(result.changes);
    history.recordAction(std::move(action));

    return result.totalReplacements;
}

bool Document::replaceOccurrence(string pattern, string replacement,
                                 int occurrence, SearchOptions opts)
{
    if (pattern.empty() || lines.isEmpty() || occurrence < 1)
        return false;

    Replace replacer;
    ReplaceResult result = replacer.replaceOccurrence(
        lines.getAllLines(), pattern, replacement, occurrence, opts);

    if (result.changes.empty())
        return false;

    applyChanges(result.changes, true);

    Action action;
    action.type    = ActionType::REPLACE;
    action.changes = std::move(result.changes);
    history.recordAction(std::move(action));

    return true;
}

vector<string> Document::getWordSuggestions(string prefix)
{
    wordTrie.clear();

    vector<string> allLines = lines.getAllLines();

    for (const string& line : allLines)
    {
        stringstream ss(line);
        string word;

        while (ss >> word)
        {
            wordTrie.insertWord(word);
        }
    }

    return wordTrie.getSuggestions(prefix);
}


// Get frequency of all words in document
vector<pair<string, int>> Document::getWordFrequencies()
{
    wordFrequency.clear();

    vector<string> allLines = lines.getAllLines();

    for (const string& line : allLines)
    {
        stringstream ss(line);
        string word;

        while (ss >> word)
        {
            wordFrequency.insertWord(word);
        }
    }

    return wordFrequency.getAllFrequencies();
}

// Get frequency of a specific word
int Document::getWordFrequency(string word)
{
    getWordFrequencies();

    return wordFrequency.getFrequency(word);
}



// Save current document
bool Document::saveToFile(string filename)
{
    vector<string> allLines = lines.getAllLines();

    return fileManager.saveFile(allLines, filename);
}

// Open an existing document
bool Document::openFromFile(string filename)
{
    vector<string> loadedLines;

    if (!fileManager.loadFile(filename, loadedLines))
    {
        return false;
    }

    lines.clear();

    for (const string& line : loadedLines)
    {
        lines.insertAtEnd(line);
    }

    // Clear old Undo/Redo history
    history.clearHistory();

    return true;
}


bool Document::exportToPDF(string filename)
{
    vector<string> allLines = lines.getAllLines();

    return pdfExporter.exportToPDF(allLines, filename);
}

void Document::newDocument()
{
    lines.clear();
    history.clearHistory();
    wordTrie.clear();
    wordFrequency.clear();
}
