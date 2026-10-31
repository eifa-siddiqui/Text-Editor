
#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include "../include/core/Document.h"
#include "../include/algorithms/Search.h"

using namespace std;

void showMenu()
{
    cout << "\n========== TEXT EDITOR ==========\n";
    cout << "1.  Create New Document\n";
    cout << "2.  Open File\n";
    cout << "3.  Save File\n";
    cout << "4.  Insert Line\n";
    cout << "5.  Delete Line\n";
    cout << "6.  Update Line\n";
    cout << "7.  Display Document\n";
    cout << "8.  KMP Search\n";
    cout << "9.  Boyer-Moore Search\n";
    cout << "10. Replace All\n";
    cout << "11. Replace Specific Occurrence\n";
    cout << "12. Undo\n";
    cout << "13. Redo\n";
    cout << "14. Word Suggestions\n";
    cout << "15. Word Frequency\n";
    cout << "16. Export PDF\n";
    cout << "17. Insert Text at Character Position\n";
    cout << "18. Delete Text Range\n";
    cout << "19. Delete Line Range\n";
    cout << "20. Exit\n";
    cout << "=================================\n";
    cout << "Enter your choice: ";
}

// Reads a whole number safely. Returns false if the input was not a number.
bool readInt(const string &prompt, int &value)
{
    cout << prompt;

    if (!(cin >> value))
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number.\n";
        return false;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return true;
}

bool askYesNo(const string &question)
{
    string answer;
    cout << question << " (y/n): ";
    getline(cin, answer);
    return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
}

SearchOptions askSearchOptions()
{
    SearchOptions opts;
    opts.caseSensitive = askYesNo("Case sensitive?");
    opts.wholeWord     = askYesNo("Match whole word only?");
    return opts;
}

void printMatches(Document &doc, const vector<SearchResult> &results)
{
    cout << results.size() << " match(es) found:\n";

    int number = 1;
    for (const SearchResult &r : results)
    {
        cout << "  #" << number++ << "  Line " << r.lineNumber
             << ", Column " << r.columnNumber
             << "  ->  " << doc.getLine(r.lineNumber) << '\n';
    }
}

int main()
{
    Document doc;
    Search search;

    int choice = 0;
    string text;
    string filename;
    string pattern;
    string replacement;

    do
    {
        showMenu();

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Invalid input! Please enter a number.\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            doc.newDocument();
            cout << "New document created successfully!\n";
            break;
        case 2:
            cout << "Enter filename: ";
            getline(cin, filename);

            if (doc.openFromFile(filename))
                cout << "File opened successfully!\n";
            else
                cout << "Unable to open file!\n";
            break;

        case 3:
            cout << "Enter filename: ";
            getline(cin, filename);

            if (doc.saveToFile(filename))
                cout << "File saved successfully!\n";
            else
                cout << "File saving failed!\n";
            break;

        case 4:
        {
            int position;

            if (!readInt("Enter line position: ", position))
                break;

            cout << "Enter text: ";
            getline(cin, text);

            if (doc.insertLine(text, position))
                cout << "Line inserted!\n";
            else
                cout << "Invalid line position! (valid: 1 to "
                     << doc.getLineCount() + 1 << ")\n";
            break;
        }

        case 5:
        {
            int position;

            if (doc.isEmpty())
            {
                cout << "Document is empty!\n";
                break;
            }

            if (!readInt("Enter line number to delete: ", position))
                break;

            if (doc.deleteLine(position))
                cout << "Line deleted.\n";
            else
                cout << "Invalid line number! (valid: 1 to "
                     << doc.getLineCount() << ")\n";
            break;
        }

        case 6:
        {
            int position;

            if (doc.isEmpty())
            {
                cout << "Document is empty!\n";
                break;
            }

            if (!readInt("Enter line number to update: ", position))
                break;

            cout << "Enter new text: ";
            getline(cin, text);

            if (doc.updateLine(position, text))
                cout << "Line updated!\n";
            else
                cout << "Invalid line number!\n";
            break;
        }

        case 7:
            doc.displayDocument();
            break;

        case 8:
        case 9:
        {
            if (doc.isEmpty())
            {
                cout << "Document is empty! Nothing to search.\n";
                break;
            }

            cout << "Enter search word: ";
            getline(cin, pattern);

            if (pattern.empty())
            {
                cout << "Search string cannot be empty!\n";
                break;
            }

            SearchOptions opts = askSearchOptions();
            vector<SearchResult> results;

            if (choice == 8)
                results = search.searchText(doc.getAllLines(), pattern, opts);
            else
                results = search.boyerMooreSearch(doc.getAllLines(), pattern, opts);

            if (results.empty())
                cout << "\"" << pattern << "\" was NOT found in the document.\n";
            else
                printMatches(doc, results);
            break;
        }

        case 10:
        {
            if (doc.isEmpty())
            {
                cout << "Document is empty! Nothing to replace.\n";
                break;
            }

            cout << "Enter word to replace: ";
            getline(cin, pattern);

            if (pattern.empty())
            {
                cout << "Search string cannot be empty!\n";
                break;
            }

            cout << "Enter replacement: ";
            getline(cin, replacement);

            SearchOptions opts = askSearchOptions();
            int count = doc.replaceAll(pattern, replacement, opts);

            if (count > 0)
                cout << "Replaced " << count << " occurrence(s). (Undo available)\n";
            else
                cout << "Nothing was replaced!\n";
            break;
        }

        case 11:
        {
            if (doc.isEmpty())
            {
                cout << "Document is empty! Nothing to replace.\n";
                break;
            }

            cout << "Enter word to replace: ";
            getline(cin, pattern);

            if (pattern.empty())
            {
                cout << "Search string cannot be empty!\n";
                break;
            }

            SearchOptions opts = askSearchOptions();
            vector<SearchResult> results =
                search.searchText(doc.getAllLines(), pattern, opts);

            if (results.empty())
            {
                cout << "\"" << pattern << "\" was NOT found in the document.\n";
                break;
            }

            printMatches(doc, results);

            int occurrence;
            if (!readInt("Which occurrence number to replace? ", occurrence))
                break;

            if (occurrence < 1 || occurrence > (int)results.size())
            {
                cout << "Invalid occurrence! (valid: 1 to " << results.size() << ")\n";
                break;
            }

            cout << "Enter replacement: ";
            getline(cin, replacement);

            if (doc.replaceOccurrence(pattern, replacement, occurrence, opts))
                cout << "Occurrence #" << occurrence << " replaced. (Undo available)\n";
            else
                cout << "Nothing was replaced!\n";
            break;
        }

        case 12:
            if (doc.undo())
                cout << "Undo successful! (" << doc.undoCount()
                     << " more undo, " << doc.redoCount() << " redo available)\n";
            else
                cout << "Nothing to undo!\n";
            break;

        case 13:
            if (doc.redo())
                cout << "Redo successful! (" << doc.undoCount()
                     << " undo, " << doc.redoCount() << " more redo available)\n";
            else
                cout << "Nothing to redo!\n";
            break;

        case 14:
            cout << "Enter prefix: ";
            getline(cin, pattern);

            {
                vector<string> suggestions =
                    doc.getWordSuggestions(pattern);

                if (suggestions.empty())
                    cout << "No suggestions found!\n";

                for (const string &word : suggestions)
                    cout << word << '\n';
            }
            break;

        case 15:
            cout << "Enter word: ";
            getline(cin, text);

            cout << text << " appears "
                 << doc.getWordFrequency(text)
                 << " time(s).\n";
            break;

        case 16:
            cout << "Enter PDF filename: ";
            getline(cin, filename);

            if (doc.exportToPDF(filename))
                cout << "PDF exported successfully!\n";
            else
                cout << "PDF export failed!\n";
            break;

        case 17:
        {
            int line, column;
            if (!readInt("Enter line number: ", line) ||
                !readInt("Enter insertion column (1-based): ", column)) break;
            cout << "Enter text to insert: ";
            getline(cin, text);
            if (doc.insertText(line, column, text)) cout << "Text inserted.\n";
            else cout << "Invalid line or insertion column.\n";
            break;
        }

        case 18:
        {
            int line, column, length;
            if (!readInt("Enter line number: ", line) ||
                !readInt("Enter start column (1-based): ", column) ||
                !readInt("Enter number of characters to delete: ", length)) break;
            if (doc.deleteText(line, column, length)) cout << "Text range deleted.\n";
            else cout << "Invalid line, column, or range length.\n";
            break;
        }

        case 19:
        {
            int first, last;
            if (!readInt("Enter first line to delete: ", first) ||
                !readInt("Enter last line to delete: ", last)) break;
            if (doc.deleteLines(first, last)) cout << "Line range deleted.\n";
            else cout << "Invalid line range.\n";
            break;
        }

        case 20:
            cout << "Thank you for using Text Editor!\n";
            break;

        default:
            cout << "Invalid choice! Try again.\n";
        }

    } while (choice != 20);

    return 0;
}
