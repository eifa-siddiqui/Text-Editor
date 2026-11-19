#pragma once

#include <string>
#include <vector>
#include <utility>

struct LineNode
{
    std::string text;
    LineNode* prev;
    LineNode* next;

    LineNode(const std::string& val);
};

// ============================================================================
// Class: DoublyLinkedList
// Custom doubly linked list for document line storage.
// Supports O(1) insertions at head/tail and bidirectional indexed traversal.
// ============================================================================
class DoublyLinkedList
{
private:
    LineNode* head;
    LineNode* tail;
    int size;

    LineNode* getNodeAt(int position) const;

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    int getSize() const;
    bool isEmpty() const;
    void clear();
    void insertAtEnd(const std::string& text);
    bool insertAtPosition(const std::string& text, int position);
    bool deleteAtPosition(int position);
    bool updateLine(int position, const std::string& newText);
    std::string getLine(int position) const;
    std::vector<std::string> getAllLines() const;
    void updateLines(const std::vector<std::pair<int, std::string>>& updates);
};
