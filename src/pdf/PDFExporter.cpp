#include "../../include/pdf/PDFExporter.h"
#include <fstream>
#include <string>
#include <vector>

using namespace std;

string escapePDFText(const string& text)
{
    string result;

    for (char ch : text)
    {
        if (ch == '(' || ch == ')' || ch == '\\')
        {
            result += '\\';
        }

        if (ch >= 32 && ch <= 126)
        {
            result += ch;
        }
    }

    return result;
}

bool PDFExporter::exportToPDF(
    const vector<string>& lines,
    const string& filename)
{
    ofstream file(filename, ios::binary);

    if (!file.is_open())
        return false;

    string content = "BT\n/F1 12 Tf\n50 790 Td\n";

    for (const string& line : lines)
    {
        content += "(" + escapePDFText(line) + ") Tj\n";
        content += "0 -18 Td\n";
    }

    content += "ET";

    vector<string> objects;

    objects.push_back("<< /Type /Catalog /Pages 2 0 R >>");

    objects.push_back(
        "<< /Type /Pages /Kids [3 0 R] /Count 1 >>"
    );

    objects.push_back(
        "<< /Type /Page /Parent 2 0 R "
        "/MediaBox [0 0 595 842] "
        "/Resources << /Font << /F1 4 0 R >> >> "
        "/Contents 5 0 R >>"
    );

    objects.push_back(
        "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>"
    );

    objects.push_back(
        "<< /Length " + to_string(content.size()) +
        " >>\nstream\n" + content + "\nendstream"
    );

    file << "%PDF-1.4\n";

    vector<long> offsets;
    offsets.push_back(0);

    for (int i = 0; i < objects.size(); i++)
    {
        offsets.push_back((long)file.tellp());

        file << i + 1 << " 0 obj\n";
        file << objects[i] << "\n";
        file << "endobj\n";
    }

    long xrefPosition = (long)file.tellp();

    file << "xref\n";
    file << "0 " << objects.size() + 1 << "\n";
    file << "0000000000 65535 f \n";

    for (int i = 1; i < offsets.size(); i++)
    {
        char buffer[30];

        snprintf(buffer, sizeof(buffer),
                 "%010ld 00000 n \n", offsets[i]);

        file << buffer;
    }

    file << "trailer\n";
    file << "<< /Size " << objects.size() + 1
         << " /Root 1 0 R >>\n";

    file << "startxref\n";
    file << xrefPosition << "\n";
    file << "%%EOF";

    file.close();

    return true;
}
