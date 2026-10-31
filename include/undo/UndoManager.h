#pragma once

#include <string>
#include <vector>
#include "../data_structures/Stack.h"

using namespace std;

// What kind of edit an Action describes.
enum class ActionType
{
    NONE,
    INSERT_LINE,   // a line was inserted at `position`
    DELETE_LINE,   // the line at `position` was deleted
    UPDATE_LINE,   // the line at `position` was modified
    REPLACE        // one or more lines were changed by Replace / Replace All
};

// One changed line inside a REPLACE action.
// Only lines that really changed are stored (not the whole document).
struct LineChange
{
    int    lineNumber = 0;   // 1-based
    string oldText;
    string newText;
};

struct Action
{
    ActionType type     = ActionType::NONE;
    int        position = 0;      // 1-based line number (line actions)
    string     oldText;           // text BEFORE the edit (line actions)
    string     newText;           // text AFTER  the edit (line actions)

    vector<LineChange> changes;   // used by REPLACE (sorted by lineNumber)
};

class UndoManager
{
public:
    // Maximum number of edits that can be undone (oldest ones are dropped).
    static const int MAX_HISTORY = 100;

private:
    Stack<Action> undoStack;   // actions that can be undone
    Stack<Action> redoStack;   // actions that were undone

public:
    UndoManager();

    void recordAction(Action action);   // push on undo stack, clears redo stack

    bool canUndo() const;
    bool canRedo() const;

    int  undoCount() const;
    int  redoCount() const;

    Action undo();   // pops undo stack, pushes on redo stack, returns the action
    Action redo();   // pops redo stack, pushes on undo stack, returns the action

    void clearHistory();
};
