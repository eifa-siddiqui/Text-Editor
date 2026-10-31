
#include "../../include/file_handling/FileManager.h"
#include <fstream>

using namespace std;

// Save document into a file
bool FileManager::saveFile(
    const vector<string>& lines,
    const string& filename)
{
    ofstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    for (const string& line : lines)
    {
        file << line << '\n';
    }

    file.close();

    return true;
}

// Load document from a file
bool FileManager::loadFile(
    const string& filename,
    vector<string>& lines)
{
    ifstream file(filename);

    if (!file.is_open())
    {
        return false;
    }

    lines.clear();

    string line;

    while (getline(file, line))
    {
        lines.push_back(line);
    }

    file.close();

    return true;
}
