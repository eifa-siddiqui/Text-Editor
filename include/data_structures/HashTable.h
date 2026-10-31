#pragma once

#include <string>
#include <vector>
#include <utility>

using namespace std;

class HashTable
{
private:
    vector<vector<pair<string, int>>> table;
    int capacity;

    int hashFunction(string word);

public:
    HashTable();

    void insertWord(string word);

    int getFrequency(string word);

    vector<pair<string, int>> getAllFrequencies();

    void clear();
};
