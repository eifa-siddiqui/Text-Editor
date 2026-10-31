#include "../../include/algorithms/Search.h"
#include <algorithm>
#include <cctype>

using namespace std;

// ===========================================================================
// Helpers
// ===========================================================================

string toLowerCopy(const string& s)
{
    string r(s);
    for (size_t i = 0; i < r.size(); i++)
        r[i] = (char)tolower((unsigned char)r[i]);
    return r;
}

static bool isWordChar(char c)
{
    unsigned char u = (unsigned char)c;
    return isalnum(u) || c == '_';
}

static bool isWholeWordAt(const string& text, int start, int len)
{
    if (start > 0 && isWordChar(text[start - 1]))
        return false;

    int end = start + len;
    if (end < (int)text.size() && isWordChar(text[end]))
        return false;

    return true;
}

vector<int> findMatchesInLine(const string& line,
                              const PatternMatcher& matcher,
                              const SearchOptions& opts)
{
    vector<int> positions;
    const int m = matcher.patternLength();

    if (m == 0 || (int)line.size() < m)
        return positions;

    // Only copy the line when we really need a lower-cased version.
    string lowered;
    const string* text = &line;
    if (!opts.caseSensitive)
    {
        lowered = toLowerCopy(line);
        text    = &lowered;
    }

    int pos = 0;
    while (true)
    {
        int p = matcher.findNext(*text, pos);
        if (p < 0)
            break;

        if (!opts.wholeWord || isWholeWordAt(line, p, m))
        {
            positions.push_back(p);
            pos = p + m;        // accepted: continue after the match (no overlap)
        }
        else
        {
            pos = p + 1;        // rejected (inside a bigger word): try next start
        }
    }

    return positions;
}

// ===========================================================================
// KMP
// ===========================================================================

KMPMatcher::KMPMatcher(const string& pat) : pattern(pat), lps(pat.size(), 0)
{
    int len = 0;

    for (int i = 1; i < (int)pattern.size(); )
    {
        if (pattern[i] == pattern[len])
        {
            lps[i++] = ++len;
        }
        else if (len != 0)
        {
            len = lps[len - 1];
        }
        else
        {
            lps[i++] = 0;
        }
    }
}

int KMPMatcher::findNext(const string& text, int from) const
{
    const int n = (int)text.size();
    const int m = (int)pattern.size();

    if (m == 0 || from < 0)
        return -1;

    int j = 0;
    for (int i = from; i < n; i++)
    {
        while (j > 0 && text[i] != pattern[j])
            j = lps[j - 1];

        if (text[i] == pattern[j])
            j++;

        if (j == m)
            return i - m + 1;
    }

    return -1;
}

// ===========================================================================
// Boyer-Moore (bad character + strong good suffix)
// ===========================================================================

BoyerMooreMatcher::BoyerMooreMatcher(const string& pat)
    : pattern(pat), badChar(256, -1), goodSuffix(pat.size() + 1, 0)
{
    if (pattern.empty())
        return;

    buildBadChar();
    buildGoodSuffix();
}

void BoyerMooreMatcher::buildBadChar()
{
    for (int i = 0; i < (int)pattern.size(); i++)
        badChar[(unsigned char)pattern[i]] = i;
}

void BoyerMooreMatcher::buildGoodSuffix()
{
    const int m = (int)pattern.size();
    vector<int> border(m + 1, 0);

    // Case 1: the matched suffix occurs elsewhere in the pattern.
    int i = m, j = m + 1;
    border[i] = j;

    while (i > 0)
    {
        while (j <= m && pattern[i - 1] != pattern[j - 1])
        {
            if (goodSuffix[j] == 0)
                goodSuffix[j] = j - i;
            j = border[j];
        }
        i--;
        j--;
        border[i] = j;
    }

    // Case 2: only a part of the matched suffix is a prefix of the pattern.
    j = border[0];
    for (i = 0; i <= m; i++)
    {
        if (goodSuffix[i] == 0)
            goodSuffix[i] = j;

        if (i == j)
            j = border[j];
    }
}

int BoyerMooreMatcher::findNext(const string& text, int from) const
{
    const int n = (int)text.size();
    const int m = (int)pattern.size();

    if (m == 0 || from < 0)
        return -1;

    int s = from;                       // current alignment of the pattern
    while (s <= n - m)
    {
        int j = m - 1;

        while (j >= 0 && pattern[j] == text[s + j])
            j--;

        if (j < 0)
            return s;

        int bad  = j - badChar[(unsigned char)text[s + j]];
        int good = goodSuffix[j + 1];
        s += max(bad, good);            // both are >= 1 here
    }

    return -1;
}

// ===========================================================================
// Search over the whole document
// ===========================================================================

static vector<SearchResult> runSearch(const vector<string>& lines,
                                      const PatternMatcher& matcher,
                                      const SearchOptions& opts)
{
    vector<SearchResult> results;

    for (int line = 0; line < (int)lines.size(); line++)
    {
        vector<int> hits = findMatchesInLine(lines[line], matcher, opts);

        for (int p : hits)
            results.push_back({line + 1, p + 1});
    }

    return results;
}

vector<SearchResult> Search::searchText(const vector<string>& lines,
                                        const string& pattern,
                                        const SearchOptions& opts) const
{
    if (pattern.empty())
        return vector<SearchResult>();

    KMPMatcher matcher(opts.caseSensitive ? pattern : toLowerCopy(pattern));
    return runSearch(lines, matcher, opts);
}

vector<SearchResult> Search::boyerMooreSearch(const vector<string>& lines,
                                              const string& pattern,
                                              const SearchOptions& opts) const
{
    if (pattern.empty())
        return vector<SearchResult>();

    BoyerMooreMatcher matcher(opts.caseSensitive ? pattern : toLowerCopy(pattern));
    return runSearch(lines, matcher, opts);
}
