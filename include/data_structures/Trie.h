#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

class TrieNode
{
public:
    unordered_map<char, TrieNode*> children;
    bool isEndOfWord;

    TrieNode()
    {
        isEndOfWord = false;
    }
};

class Trie
{
private:
    TrieNode* root;

    void collectWords(
        TrieNode* node,
        string currentWord,
        vector<string>& results
    );

    void deleteTrie(TrieNode* node);

public:
    Trie();

    ~Trie();

    void insertWord(string word);

    bool searchWord(string word);

    vector<string> getSuggestions(string prefix);

    void clear();
};
