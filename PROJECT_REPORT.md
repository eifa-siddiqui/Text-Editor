# Text Editor Project Report

## 1. Problem description

This project implements a basic line-oriented text editor in C++. It keeps document lines in memory and provides file operations, line and character editing, search, replacement, undo/redo, word suggestions, word frequency, and PDF export. A browser interface is also included in `ui/index.html`.

## 2. Requirements and assumptions

- Positions entered in the C++ console are 1-based.
- Text insertion and deletion address one line at a time. New lines are inserted or removed with the line operations.
- The current implementation stores each line as `std::string`; character columns and lengths therefore count string bytes. ASCII text behaves as expected; a user-visible Unicode character may occupy multiple bytes.
- Search and replacement operate within individual lines. Matches do not cross newline boundaries and accepted matches do not overlap.
- Undo history is limited to the most recent 100 actions; older actions are discarded when the stack fills.
- Files are plain text. The browser UI is a separate implementation and does not call the C++ classes.

## 3. System and design overview

`Document` coordinates the line store, history manager, file manager, search/replacement routines, trie, hash table, and PDF exporter. The thin `src/main.cpp` entry point starts `ConsoleApp`, while `src/app/ConsoleApp.cpp` handles console menu input. `src/server/TextEditorServer.cpp` exposes the same C++ `Document` implementation over a local HTTP API. `ui/index.html` is only the presentation and input layer; it sends browser actions to C++ and does not implement duplicate linked-list, stack, search, replacement, trie, hash-table, file, or PDF logic.

## 4. Data structures and justification

### Document representation: custom doubly linked list

`DoublyLinkedList` stores one node per line. Each node has `prev` and `next` pointers. Insertion or removal at a known node is constant time, and traversal can start from the nearer end. Since the interface addresses lines by number, finding a line still requires traversal; the structure fits line-oriented editing and demonstrates linked-list operations.

### Undo and redo: custom circular array stack

`Stack<T>` is a fixed-capacity circular array. `UndoManager` keeps separate stacks for actions to undo and redo. Push and pop are O(1), and the bounded history prevents unbounded action-count growth. A new edit clears redo history.

### Word frequency: hash table with separate chaining

`HashTable` hashes each word into a bucket and stores collisions in that bucket. Lookup and update are expected O(1) with a reasonable distribution and O(W) in the worst case, where W is the number of distinct words in one bucket.

### Word suggestions: trie

`Trie` stores prefixes as paths and traverses the subtree under the requested prefix to produce completions. Its child maps use `unordered_map` for character-to-node lookup. Prefix navigation takes expected O(p), where p is prefix length; producing suggestions also costs the total size of returned words.

### Other structures

Vectors are used to exchange line snapshots and to store batches of replacement changes. Standard strings are used for line text and the KMP/Boyer–Moore preprocessing tables.

## 5. Important algorithms and complexity

Let n be the number of document lines, N the total number of text bytes, m the pattern length, and k the number of matched/replaced occurrences.

| Operation | Time complexity | Notes |
|---|---:|---|
| Display or copy all lines | O(N) | Visits each line and byte. |
| Count lines | O(1) | Maintained by the linked list. |
| Count characters | O(N) | Sums each line's byte length. Newline separators are not included. |
| Find a line by position | O(min(i, n-i)) | Traverses from the closer end; i is its 1-based position. |
| Insert/delete a line | O(n) | Position lookup dominates; link changes are O(1). |
| Insert/delete text in one line | O(n + r) | n is line length and r is inserted text length; `std::string` shifts/copies bytes. |
| KMP search | O(N + m + k) | O(m) prefix preprocessing; search is linear; result storage is O(k). |
| Boyer–Moore search | O(Nm) worst case | Uses bad-character and good-suffix shifts; often skips text, but this implementation does not claim a sublinear worst-case bound. |
| Replace all | O(N + output size) | Finds non-overlapping matches and constructs changed lines. |
| Undo/redo stack push or pop | O(1) stack slot operation | Copying an `Action` can additionally cost the size of its stored strings/change list. |
| Hash-table word lookup | Expected O(1), worst O(W) | Separate chaining; no automatic resizing. |
| Trie prefix lookup | Expected O(p) | Child lookup uses `unordered_map`; collecting completions adds output traversal. |
| Save/open text file | O(N) | Writes or reads the entire file. |

## 6. Testing strategy and cases

The native implementation was compiled with:

```text
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude ^
  src/main.cpp src/app/ConsoleApp.cpp src/algorithms/Search.cpp src/algorithms/Replace.cpp ^
  src/core/Document.cpp src/data_structures/HashTable.cpp ^
  src/data_structures/Trie.cpp src/file_handling/FileManager.cpp ^
  src/pdf/PDFExporter.cpp src/undo/UndoManager.cpp -o text-editor
```

Representative checks were run against the native classes and console workflow:

1. Empty-document state, line insertion, invalid line position, and character insertion/deletion.
2. KMP and Boyer–Moore search with case-sensitive, case-insensitive, and whole-word options.
3. Replace-all, undo/redo state restoration, and redo invalidation after a new edit.
4. Word frequency, trie suggestions, plain-text save/open, and PDF header generation.
5. Search-demonstration counts record character comparisons performed by KMP and Boyer-Moore.
   The console prints `Character comparisons: <count> (<algorithm>)` after each search.

The remaining boundary cases in the list below are the recommended submission checklist and still need captured
terminal/browser evidence:

5. Delete line ranges; verify undo restores order and redo reapplies deletion.
6. Reject invalid text ranges, empty search strings, missing files, and operations on an empty document.
7. Exceed 100 edits and verify the documented bounded-history behavior.

The browser UI is an independent JavaScript implementation and should be tested separately in a browser. Screenshots or
terminal transcripts should be added to the submission copy of this report when required by the instructor; no screenshot is
claimed here because the repository contains no captured test evidence.

## 7. Limitations

- Text positions count bytes rather than Unicode code points or grapheme clusters.
- Search and replacement do not span line breaks, and search results are non-overlapping.
- Line-number operations require traversal, so large documents can incur linear access time.
- The hash table has a fixed bucket count and no resizing.
- The C++ PDF exporter currently emits one page and does not paginate long documents; it also omits non-ASCII bytes.
- The browser interface is an independent implementation, so changes to C++ logic do not automatically change browser behavior.

## 8. Conclusion

The project applies linked lists, stacks, hashing, tries, KMP, and Boyer–Moore to common editing tasks. It now exposes line-level and within-line text editing in the console, reports line and character counts, and includes undo/redo for edits. Its line-oriented model and byte-based positions are suitable for a course-scale editor but are not intended to replace a full Unicode-aware editor.
