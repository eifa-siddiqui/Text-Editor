
#pragma once

#include <string>
#include <vector>

using namespace std;

class PDFExporter
{
public:
    bool exportToPDF(
        const vector<string>& lines,
        const string& filename
    );
};
