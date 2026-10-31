#include "../../include/undo/UndoManager.h"

UndoManager::UndoManager()
    : undoStack(MAX_HISTORY), redoStack(MAX_HISTORY)
{
}

void UndoManager::recordAction(Action action)
{
    undoStack.push(std::move(action));

    // A new edit makes the old redo history invalid.
    redoStack.clear();
}

bool UndoManager::canUndo() const { return !undoStack.empty(); }
bool UndoManager::canRedo() const { return !redoStack.empty(); }

int UndoManager::undoCount() const { return undoStack.size(); }
int UndoManager::redoCount() const { return redoStack.size(); }

Action UndoManager::undo()
{
    if (undoStack.empty())
        return Action();                       // type == NONE

    Action action = undoStack.top();           // copy: the same action is pushed on redo
    undoStack.pop();
    redoStack.push(action);

    return action;
}

Action UndoManager::redo()
{
    if (redoStack.empty())
        return Action();

    Action action = redoStack.top();
    redoStack.pop();
    undoStack.push(action);

    return action;
}

void UndoManager::clearHistory()
{
    undoStack.clear();
    redoStack.clear();
}
