#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>

class FileManager
{
public:
    bool saveFile(const std::vector<std::string>& lines, const std::string& filename);
    bool loadFile(const std::string& filename, std::vector<std::string>& lines);
};

#endif // FILE_MANAGER_H