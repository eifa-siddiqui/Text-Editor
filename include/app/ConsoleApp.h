#pragma once

#include <string>
#include <vector>

#include "../core/Document.h"
#include "../algorithms/Search.h"

class ConsoleApp
{
private:
    Document document;
    Search search;

    void showMenu() const;
    bool readInt(const std::string& prompt, int& value) const;
    bool askYesNo(const std::string& question) const;
    SearchOptions askSearchOptions() const;
    void printMatches(const std::vector<SearchResult>& results);

    void runNewDocument();
    void runOpenFile();
    void runSaveFile();
    void runInsertLine();
    void runDeleteLine();
    void runUpdateLine();
    void runSearch(bool useBoyerMoore);
    void runReplaceAll();
    void runReplaceOccurrence();
    void runUndo();
    void runRedo();
    void runSuggestions();
    void runWordFrequency();
    void runExportPdf();
    void runInsertText();
    void runDeleteText();
    void runDeleteLineRange();

    void handleChoice(int choice);

public:
    int run();
};

