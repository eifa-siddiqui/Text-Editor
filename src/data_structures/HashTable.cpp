#include "../../include/data_structures/HashTable.h"

using namespace std;

// Constructor
HashTable::HashTable()
{
    capacity = 101;
    table.resize(capacity);
}

// Hash Function
int HashTable::hashFunction(string word)
{
    unsigned long hash = 0;

    for (char ch : word)
    {
        hash = (hash * 31 + (unsigned char)ch) % capacity;
    }

    return hash;
}

// Insert Word / Update Frequency
void HashTable::insertWord(string word)
{
    int index = hashFunction(word);

    for (auto& item : table[index])
    {
        if (item.first == word)
        {
            item.second++;
            return;
        }
    }

    table[index].push_back({word, 1});
}

// Get Frequency of a Word
int HashTable::getFrequency(string word)
{
    int index = hashFunction(word);

    for (const auto& item : table[index])
    {
        if (item.first == word)
        {
            return item.second;
        }
    }

    return 0;
}

// Get All Word Frequencies
vector<pair<string, int>> HashTable::getAllFrequencies()
{
    vector<pair<string, int>> result;

    for (const auto& bucket : table)
    {
        for (const auto& item : bucket)
        {
            result.push_back(item);
        }
    }

    return result;
}

// Clear Hash Table
void HashTable::clear()
{
    for (auto& bucket : table)
    {
        bucket.clear();
    }
}
