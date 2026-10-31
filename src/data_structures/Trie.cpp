
#include "../../include/data_structures/Trie.h"

using namespace std;

// Constructor
Trie::Trie()
{
    root = new TrieNode();
}

// Insert a word into Trie
void Trie::insertWord(string word)
{
    TrieNode* current = root;

    for (char ch : word)
    {
        if (current->children.find(ch) == current->children.end())
        {
            current->children[ch] = new TrieNode();
        }

        current = current->children[ch];
    }

    current->isEndOfWord = true;
}

// Search a complete word
bool Trie::searchWord(string word)
{
    TrieNode* current = root;

    for (char ch : word)
    {
        if (current->children.find(ch) == current->children.end())
        {
            return false;
        }

        current = current->children[ch];
    }

    return current->isEndOfWord;
}

// Collect all words below a prefix node
void Trie::collectWords(
    TrieNode* node,
    string currentWord,
    vector<string>& results)
{
    if (node->isEndOfWord)
    {
        results.push_back(currentWord);
    }

    for (auto child : node->children)
    {
        collectWords(
            child.second,
            currentWord + child.first,
            results
        );
    }
}

// Get autocomplete suggestions
vector<string> Trie::getSuggestions(string prefix)
{
    vector<string> results;

    TrieNode* current = root;

    for (char ch : prefix)
    {
        if (current->children.find(ch) == current->children.end())
        {
            return results;
        }

        current = current->children[ch];
    }

    collectWords(current, prefix, results);

    return results;
}

// Delete all Trie nodes
void Trie::deleteTrie(TrieNode* node)
{
    for (auto child : node->children)
    {
        deleteTrie(child.second);
    }

    delete node;
}

// Destructor
Trie::~Trie()
{
    deleteTrie(root);
}

void Trie::clear()
{
    deleteTrie(root);
    root = new TrieNode();
}