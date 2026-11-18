#include "../../include/app/ConsoleApp.h"

#include <iostream>
#include <limits>

using namespace std;

void ConsoleApp::showMenu() const
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

bool ConsoleApp::readInt(const string& prompt, int& value) const
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

bool ConsoleApp::askYesNo(const string& question) const
{
    string answer;
    cout << question << " (y/n): ";
    getline(cin, answer);
    return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
}

SearchOptions ConsoleApp::askSearchOptions() const
{
    SearchOptions options;
    options.caseSensitive = askYesNo("Case sensitive?");
    options.wholeWord = askYesNo("Match whole word only?");
    return options;
}

void ConsoleApp::printMatches(const vector<SearchResult>& results)
{
    cout << results.size() << " match(es) found:\n";

    int number = 1;
    for (const SearchResult& result : results)
    {
        cout << "  #" << number++ << "  Line " << result.lineNumber
             << ", Column " << result.columnNumber
             << "  ->  " << document.getLine(result.lineNumber) << '\n';
    }
}

void ConsoleApp::runNewDocument()
{
    document.newDocument();
    cout << "New document created successfully!\n";
}

void ConsoleApp::runOpenFile()
{
    string filename;
    cout << "Enter filename: ";
    getline(cin, filename);

    if (document.openFromFile(filename))
        cout << "File opened successfully!\n";
    else
        cout << "Unable to open file!\n";
}

void ConsoleApp::runSaveFile()
{
    string filename;
    cout << "Enter filename: ";
    getline(cin, filename);

    if (document.saveToFile(filename))
        cout << "File saved successfully!\n";
    else
        cout << "File saving failed!\n";
}

void ConsoleApp::runInsertLine()
{
    int position;
    if (!readInt("Enter line position: ", position))
        return;

    string text;
    cout << "Enter text: ";
    getline(cin, text);

    if (document.insertLine(text, position))
        cout << "Line inserted!\n";
    else
        cout << "Invalid line position! (valid: 1 to "
             << document.getLineCount() + 1 << ")\n";
}

void ConsoleApp::runDeleteLine()
{
    if (document.isEmpty())
    {
        cout << "Document is empty!\n";
        return;
    }

    int position;
    if (!readInt("Enter line number to delete: ", position))
        return;

    if (document.deleteLine(position))
        cout << "Line deleted.\n";
    else
        cout << "Invalid line number! (valid: 1 to "
             << document.getLineCount() << ")\n";
}

void ConsoleApp::runUpdateLine()
{
    if (document.isEmpty())
    {
        cout << "Document is empty!\n";
        return;
    }

    int position;
    if (!readInt("Enter line number to update: ", position))
        return;

    string text;
    cout << "Enter new text: ";
    getline(cin, text);

    if (document.updateLine(position, text))
        cout << "Line updated!\n";
    else
        cout << "Invalid line number!\n";
}

void ConsoleApp::runSearch(bool useBoyerMoore)
{
    if (document.isEmpty())
    {
        cout << "Document is empty! Nothing to search.\n";
        return;
    }

    string pattern;
    cout << "Enter search word: ";
    getline(cin, pattern);

    if (pattern.empty())
    {
        cout << "Search string cannot be empty!\n";
        return;
    }

    SearchOptions options = askSearchOptions();
    vector<SearchResult> results;
    SearchStats stats;

    if (useBoyerMoore)
        results = search.boyerMooreSearch(document.getAllLines(), pattern, options, stats);
    else
        results = search.searchText(document.getAllLines(), pattern, options, stats);

    cout << "Character comparisons: " << stats.comparisons
         << " (" << (useBoyerMoore ? "Boyer-Moore" : "KMP") << ") \n";

    if (results.empty())
        cout << "\"" << pattern << "\" was NOT found in the document.\n";
    else
        printMatches(results);
}

void ConsoleApp::runReplaceAll()
{
    if (document.isEmpty())
    {
        cout << "Document is empty! Nothing to replace.\n";
        return;
    }

    string pattern;
    cout << "Enter word to replace: ";
    getline(cin, pattern);

    if (pattern.empty())
    {
        cout << "Search string cannot be empty!\n";
        return;
    }

    string replacement;
    cout << "Enter replacement: ";
    getline(cin, replacement);

    int count = document.replaceAll(pattern, replacement, askSearchOptions());

    if (count > 0)
        cout << "Replaced " << count << " occurrence(s). (Undo available)\n";
    else
        cout << "Nothing was replaced!\n";
}

void ConsoleApp::runReplaceOccurrence()
{
    if (document.isEmpty())
    {
        cout << "Document is empty! Nothing to replace.\n";
        return;
    }

    string pattern;
    cout << "Enter word to replace: ";
    getline(cin, pattern);

    if (pattern.empty())
    {
        cout << "Search string cannot be empty!\n";
        return;
    }

    SearchOptions options = askSearchOptions();
    vector<SearchResult> results =
        search.searchText(document.getAllLines(), pattern, options);

    if (results.empty())
    {
        cout << "\"" << pattern << "\" was NOT found in the document.\n";
        return;
    }

    printMatches(results);

    int occurrence;
    if (!readInt("Which occurrence number to replace? ", occurrence))
        return;

    if (occurrence < 1 || occurrence > static_cast<int>(results.size()))
    {
        cout << "Invalid occurrence! (valid: 1 to " << results.size() << ")\n";
        return;
    }

    string replacement;
    cout << "Enter replacement: ";
    getline(cin, replacement);

    if (document.replaceOccurrence(pattern, replacement, occurrence, options))
        cout << "Occurrence #" << occurrence << " replaced. (Undo available)\n";
    else
        cout << "Nothing was replaced!\n";
}

void ConsoleApp::runUndo()
{
    if (document.undo())
        cout << "Undo successful! (" << document.undoCount()
             << " more undo, " << document.redoCount() << " redo available)\n";
    else
        cout << "Nothing to undo!\n";
}

void ConsoleApp::runRedo()
{
    if (document.redo())
        cout << "Redo successful! (" << document.undoCount()
             << " undo, " << document.redoCount() << " more redo available)\n";
    else
        cout << "Nothing to redo!\n";
}

void ConsoleApp::runSuggestions()
{
    string prefix;
    cout << "Enter prefix: ";
    getline(cin, prefix);

    vector<string> suggestions = document.getWordSuggestions(prefix);
    if (suggestions.empty())
        cout << "No suggestions found!\n";

    for (const string& word : suggestions)
        cout << word << '\n';
}

void ConsoleApp::runWordFrequency()
{
    string word;
    cout << "Enter word: ";
    getline(cin, word);

    cout << word << " appears "
         << document.getWordFrequency(word)
         << " time(s).\n";
}

void ConsoleApp::runExportPdf()
{
    string filename;
    cout << "Enter PDF filename: ";
    getline(cin, filename);

    if (document.exportToPDF(filename))
        cout << "PDF exported successfully!\n";
    else
        cout << "PDF export failed!\n";
}

void ConsoleApp::runInsertText()
{
    int line;
    int column;
    if (!readInt("Enter line number: ", line) ||
        !readInt("Enter insertion column (1-based): ", column))
        return;

    string text;
    cout << "Enter text to insert: ";
    getline(cin, text);

    if (document.insertText(line, column, text))
        cout << "Text inserted.\n";
    else
        cout << "Invalid line or insertion column.\n";
}

void ConsoleApp::runDeleteText()
{
    int line;
    int column;
    int length;
    if (!readInt("Enter line number: ", line) ||
        !readInt("Enter start column (1-based): ", column) ||
        !readInt("Enter number of characters to delete: ", length))
        return;

    if (document.deleteText(line, column, length))
        cout << "Text range deleted.\n";
    else
        cout << "Invalid line, column, or range length.\n";
}

void ConsoleApp::runDeleteLineRange()
{
    int first;
    int last;
    if (!readInt("Enter first line to delete: ", first) ||
        !readInt("Enter last line to delete: ", last))
        return;

    if (document.deleteLines(first, last))
        cout << "Line range deleted.\n";
    else
        cout << "Invalid line range.\n";
}

void ConsoleApp::handleChoice(int choice)
{
    switch (choice)
    {
    case 1: runNewDocument(); break;
    case 2: runOpenFile(); break;
    case 3: runSaveFile(); break;
    case 4: runInsertLine(); break;
    case 5: runDeleteLine(); break;
    case 6: runUpdateLine(); break;
    case 7: document.displayDocument(); break;
    case 8: runSearch(false); break;
    case 9: runSearch(true); break;
    case 10: runReplaceAll(); break;
    case 11: runReplaceOccurrence(); break;
    case 12: runUndo(); break;
    case 13: runRedo(); break;
    case 14: runSuggestions(); break;
    case 15: runWordFrequency(); break;
    case 16: runExportPdf(); break;
    case 17: runInsertText(); break;
    case 18: runDeleteText(); break;
    case 19: runDeleteLineRange(); break;
    case 20: cout << "Thank you for using Text Editor!\n"; break;
    default: cout << "Invalid choice! Try again.\n";
    }
}

int ConsoleApp::run()
{
    int choice = 0;

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
        handleChoice(choice);
    } while (choice != 20);

    return 0;
}
