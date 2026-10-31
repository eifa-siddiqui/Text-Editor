#include "../../include/algorithms/Replace.h"

using namespace std;

// Build the new text of a line, replacing the matches at `positions`
// (sorted, non-overlapping) with `replacement`.
static string buildReplacedLine(const string& text,
                                const vector<int>& positions,
                                int patternLen,
                                const string& replacement)
{
    string result;
    result.reserve(text.size() + positions.size() * replacement.size());

    int last = 0;
    for (int p : positions)
    {
        result.append(text, last, p - last);
        result += replacement;
        last = p + patternLen;
    }
    result.append(text, last, string::npos);

    return result;
}

ReplaceResult Replace::replaceAll(const vector<string>& lines,
                                  const string& pattern,
                                  const string& replacement,
                                  const SearchOptions& opts) const
{
    ReplaceResult result;

    if (pattern.empty())
        return result;

    KMPMatcher matcher(opts.caseSensitive ? pattern : toLowerCopy(pattern));
    const int m = (int)pattern.size();

    for (int i = 0; i < (int)lines.size(); i++)
    {
        vector<int> positions = findMatchesInLine(lines[i], matcher, opts);

        if (positions.empty())
            continue;

        result.totalReplacements += (int)positions.size();

        string newText = buildReplacedLine(lines[i], positions, m, replacement);

        if (newText != lines[i])               // skip no-op changes
        {
            LineChange change;
            change.lineNumber = i + 1;
            change.oldText    = lines[i];
            change.newText    = std::move(newText);
            result.changes.push_back(std::move(change));
        }
    }

    return result;
}

ReplaceResult Replace::replaceOccurrence(const vector<string>& lines,
                                         const string& pattern,
                                         const string& replacement,
                                         int occurrence,
                                         const SearchOptions& opts) const
{
    ReplaceResult result;

    if (pattern.empty() || occurrence < 1)
        return result;

    KMPMatcher matcher(opts.caseSensitive ? pattern : toLowerCopy(pattern));
    const int m = (int)pattern.size();

    int seen = 0;

    for (int i = 0; i < (int)lines.size(); i++)
    {
        vector<int> positions = findMatchesInLine(lines[i], matcher, opts);

        if (seen + (int)positions.size() < occurrence)
        {
            seen += (int)positions.size();
            continue;
        }

        // The wanted occurrence is inside this line.
        int p = positions[occurrence - seen - 1];

        LineChange change;
        change.lineNumber = i + 1;
        change.oldText    = lines[i];
        change.newText    = lines[i].substr(0, p) + replacement + lines[i].substr(p + m);

        result.totalReplacements = 1;
        if (change.newText != change.oldText)
            result.changes.push_back(std::move(change));

        return result;                          // early exit
    }

    return result;                              // occurrence out of range
}
