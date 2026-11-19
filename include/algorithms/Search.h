#pragma once

#include <string>
#include <vector>

using namespace std;

// Position of one match. Both numbers are 1-based.
struct SearchResult
{
    int lineNumber;
    int columnNumber;
};

// Options shared by Search and Replace.
struct SearchOptions
{
    bool caseSensitive = true;    // false -> "Hello" matches "hello"
    bool wholeWord     = false;   // true  -> "the" does not match inside "other"
};

struct SearchStats
{
    long long comparisons = 0;
};

// ---------------------------------------------------------------------------
// PatternMatcher: a pattern that is pre-processed ONCE and can then be
// searched in many lines.  findNext() returns the index of the first match at
// or after `from`, or -1 when there is none.
// ---------------------------------------------------------------------------
class PatternMatcher
{
public:
    virtual ~PatternMatcher();
    virtual int findNext(const string& text, int from) const = 0;
    virtual int findNext(const string& text, int from, SearchStats& stats) const = 0;
    virtual int patternLength() const = 0;
};

// Knuth-Morris-Pratt.   Preprocessing O(m), search O(n), extra space O(m).
class KMPMatcher : public PatternMatcher
{
private:
    string      pattern;
    vector<int> lps;          // longest proper prefix which is also a suffix

public:
    explicit KMPMatcher(const string& pat);
    int findNext(const string& text, int from) const override;
    int findNext(const string& text, int from, SearchStats& stats) const override;
    int patternLength() const override;
};

// Boyer-Moore with BOTH the bad-character and the (strong) good-suffix rule.
// Preprocessing O(m + sigma), search O(n) on average (sub-linear in practice),
// O(n) worst case for the first match with the good-suffix rule.
class BoyerMooreMatcher : public PatternMatcher
{
private:
    string      pattern;
    vector<int> badChar;      // last index of each byte in the pattern (-1 if absent)
    vector<int> goodSuffix;   // shift table of size m + 1

    void buildBadChar();
    void buildGoodSuffix();

public:
    explicit BoyerMooreMatcher(const string& pat);
    int findNext(const string& text, int from) const override;
    int findNext(const string& text, int from, SearchStats& stats) const override;
    int patternLength() const override { return (int)pattern.size(); }
};

// ---------------------------------------------------------------------------
// Helpers used by Search and Replace (single source of truth for what an
// "occurrence" is: matches are found left-to-right and never overlap).
// ---------------------------------------------------------------------------
string toLowerCopy(const string& s);

// 0-based start indices of every occurrence of the pattern in `line`.
// `matcher` must have been built from the pattern converted with the same
// case rule (lower-cased when opts.caseSensitive == false).
vector<int> findMatchesInLine(const string& line,
                              const PatternMatcher& matcher,
                              const SearchOptions& opts);

vector<int> findMatchesInLine(const string& line,
                              const PatternMatcher& matcher,
                              const SearchOptions& opts,
                              SearchStats& stats);

class Search
{
public:
    // KMP search over all lines.  Empty pattern -> empty result.
    vector<SearchResult> searchText(
        const vector<string>& lines,
        const string& pattern,
        const SearchOptions& opts = SearchOptions()) const;

    // Boyer-Moore search over all lines.  Empty pattern -> empty result.
    vector<SearchResult> boyerMooreSearch(
        const vector<string>& lines,
        const string& pattern,
        const SearchOptions& opts = SearchOptions()) const;

    vector<SearchResult> searchText(
        const vector<string>& lines,
        const string& pattern,
        const SearchOptions& opts,
        SearchStats& stats) const;

    vector<SearchResult> boyerMooreSearch(
        const vector<string>& lines,
        const string& pattern,
        const SearchOptions& opts,
        SearchStats& stats) const;
};
