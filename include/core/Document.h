#pragma once

#include <string>
#include <vector>
#include <utility>

#include "../data_structures/LinkedList.h"
#include "../data_structures/Trie.h"
#include "../data_structures/HashTable.h"
#include "../undo/UndoManager.h"
#include "../file_handling/FileManager.h"
#include "../pdf/PDFExporter.h"
#include "../algorithms/Search.h"

class Document
{
private:
    DoublyLinkedList lines;
    UndoManager      history;
    FileManager      fileManager;
    PDFExporter      pdfExporter;
    Trie             wordTrie;
    HashTable        wordFrequency;

    void applyChanges(const std::vector<LineChange>& changes, bool useNewText);

public:
    Document();

    void addLine(std::string text);
    bool insertLine(std::string text, int position);
    bool deleteLine(int position);
    bool deleteLines(int firstPosition, int lastPosition);
    bool updateLine(int position, std::string newText);
    bool insertText(int line, int column, std::string text);
    bool deleteText(int line, int column, int length);
    std::string getLine(int position);

    void displayDocument();
    int getLineCount();
    int getCharacterCount();
    bool isEmpty();
    std::vector<std::string> getAllLines();

    int replaceAll(std::string pattern, std::string replacement, SearchOptions opts);
    bool replaceOccurrence(std::string pattern, std::string replacement,
                           int occurrence, SearchOptions opts);

    bool undo();
    bool redo();
    int undoCount();
    int redoCount();

    std::vector<std::string> getWordSuggestions(std::string prefix);
    std::vector<std::pair<std::string, int>> getWordFrequencies();
    int getWordFrequency(std::string word);

    bool saveToFile(std::string filename);
    bool openFromFile(std::string filename);
    bool exportToPDF(std::string filename);
    void newDocument();
    void clearHistory();
};

