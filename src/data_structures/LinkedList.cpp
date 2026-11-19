#include "../../include/data_structures/LinkedList.h"

LineNode::LineNode(const std::string& val)
    : text(val), prev(nullptr), next(nullptr)
{
}

DoublyLinkedList::DoublyLinkedList()
    : head(nullptr), tail(nullptr), size(0)
{
}

DoublyLinkedList::~DoublyLinkedList()
{
    clear();
}

int DoublyLinkedList::getSize() const
{
    return size;
}

bool DoublyLinkedList::isEmpty() const
{
    return size == 0;
}

LineNode* DoublyLinkedList::getNodeAt(int position) const
{
    if (position < 1 || position > size)
        return nullptr;

    if (position <= size / 2)
    {
        LineNode* curr = head;
        for (int i = 1; i < position; ++i)
            curr = curr->next;
        return curr;
    }

    LineNode* curr = tail;
    for (int i = size; i > position; --i)
        curr = curr->prev;
    return curr;
}

void DoublyLinkedList::clear()
{
    LineNode* curr = head;
    while (curr != nullptr)
    {
        LineNode* next = curr->next;
        delete curr;
        curr = next;
    }
    head = nullptr;
    tail = nullptr;
    size = 0;
}

void DoublyLinkedList::insertAtEnd(const std::string& text)
{
    LineNode* newNode = new LineNode(text);
    if (!tail)
    {
        head = tail = newNode;
    }
    else
    {
        tail->next = newNode;
        newNode->prev = tail;
        tail = newNode;
    }
    ++size;
}

bool DoublyLinkedList::insertAtPosition(const std::string& text, int position)
{
    if (position < 1 || position > size + 1)
        return false;

    if (position == size + 1)
    {
        insertAtEnd(text);
        return true;
    }

    LineNode* newNode = new LineNode(text);
    if (position == 1)
    {
        newNode->next = head;
        if (head) head->prev = newNode;
        head = newNode;
        if (!tail) tail = newNode;
    }
    else
    {
        LineNode* curr = getNodeAt(position);
        newNode->prev = curr->prev;
        newNode->next = curr;
        curr->prev->next = newNode;
        curr->prev = newNode;
    }
    ++size;
    return true;
}

bool DoublyLinkedList::deleteAtPosition(int position)
{
    if (position < 1 || position > size)
        return false;

    LineNode* target = getNodeAt(position);
    if (!target) return false;

    if (target == head) head = target->next;
    if (target == tail) tail = target->prev;
    if (target->prev) target->prev->next = target->next;
    if (target->next) target->next->prev = target->prev;

    delete target;
    --size;
    return true;
}

bool DoublyLinkedList::updateLine(int position, const std::string& newText)
{
    LineNode* node = getNodeAt(position);
    if (!node) return false;
    node->text = newText;
    return true;
}

std::string DoublyLinkedList::getLine(int position) const
{
    LineNode* node = getNodeAt(position);
    return node ? node->text : "";
}

std::vector<std::string> DoublyLinkedList::getAllLines() const
{
    std::vector<std::string> lines;
    lines.reserve(size);
    for (LineNode* curr = head; curr != nullptr; curr = curr->next)
        lines.push_back(curr->text);
    return lines;
}

void DoublyLinkedList::updateLines(
    const std::vector<std::pair<int, std::string>>& updates)
{
    if (updates.empty() || head == nullptr)
        return;

    int updateIdx = 0;
    int currentLine = 1;
    LineNode* curr = head;
    while (curr != nullptr && updateIdx < static_cast<int>(updates.size()))
    {
        if (currentLine == updates[updateIdx].first)
        {
            curr->text = updates[updateIdx].second;
            ++updateIdx;
        }
        curr = curr->next;
        ++currentLine;
    }
}
