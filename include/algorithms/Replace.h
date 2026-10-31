#pragma once

#include <string>
#include <vector>
#include "Search.h"
#include "../undo/UndoManager.h"   // LineChange

using namespace std;

struct ReplaceResult
{
    vector<LineChange> changes;      // ONLY the lines that were modified (sorted by line)
    int                totalReplacements = 0;
};

// Replace works on a copy of the lines and only DESCRIBES the changes.
// The Document applies them, which also lets it store them for Undo.
// Uses the same matching rules as Search (KMP, non-overlapping, same options),
// so "occurrence #k" in Replace is exactly the k-th result shown by Search.
class Replace
{
public:
    // Replace every occurrence.                              O(total text)
    ReplaceResult replaceAll(
        const vector<string>& lines,
        const string& pattern,
        const string& replacement,
        const SearchOptions& opts = SearchOptions()) const;

    // Replace only the k-th occurrence (k starts at 1, counted over the
    // whole document in reading order).  Stops scanning as soon as it is
    // found. totalReplacements == 0 when k is out of range.
    ReplaceResult replaceOccurrence(
        const vector<string>& lines,
        const string& pattern,
        const string& replacement,
        int occurrence,
        const SearchOptions& opts = SearchOptions()) const;
};
