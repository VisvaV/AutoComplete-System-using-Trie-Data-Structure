#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>
#include <unordered_map>

using namespace std;

struct TrieNode {
    vector<unique_ptr<TrieNode>> children;
    bool isEndOfWord;
    int frequency;
    string word;

    TrieNode() : children(26), isEndOfWord(false), frequency(0) {}
};

class CompressedTrie {
private:
    unique_ptr<TrieNode> root;
    vector<pair<string, string>> abbreviations;
    unordered_map<string, vector<string>> phoneticMap;

    void insertWord(TrieNode* node, const string& word, int freq = 1) {
        TrieNode* current = node;
        for (char c : word) {
            int index = c - 'a';
            if (!current->children[index]) {
                current->children[index] = make_unique<TrieNode>();
            }
            current = current->children[index].get();
        }
        current->isEndOfWord = true;
        current->frequency += freq;
        current->word = word;
    }

    void loadDictionary() {
        ifstream file("dictionary.txt");
        if (!file.is_open()) {
            cerr << "Error opening dictionary.txt" << endl;
            return;
        }

        string word;
        while (file >> word) {
            insertWord(root.get(), word);
        }
    }

    void loadAbbreviations() {
        ifstream file("abbreviations.txt");
        if (!file.is_open()) {
            cerr << "Error opening abbreviations.txt" << endl;
            return;
        }

        string line, abbr, fullForm;
        while (getline(file, line)) {
            stringstream ss(line);
            ss >> abbr;
            fullForm = line.substr(abbr.size() + 1);
            abbreviations.push_back({abbr, fullForm});
        }
    }
    void loadPhonetics() {
        ifstream file("phonetic.txt");
        if (!file.is_open()) {
            cerr << "Error opening phonetic.txt" << endl;
            return;
        }
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string word;
            vector<string> wordsInRow;

            while (getline(ss, word, ',')) {
                wordsInRow.push_back(word);
            }

            for (const auto& wordInRow : wordsInRow) {
                phoneticMap[wordInRow] = wordsInRow;
            }
        }
    }

    void getSuggestions(TrieNode* node, vector<string>& suggestions, int limit = 3) {
        if (!node) return;
        if (node->isEndOfWord) suggestions.push_back(node->word);

        if (suggestions.size() >= limit) return;

        for (int i = 0; i < 26; ++i) {
            if (node->children[i]) {
                getSuggestions(node->children[i].get(), suggestions, limit);
            }
        }
    }

    TrieNode* searchPrefix(TrieNode* node, const string& prefix) {
        TrieNode* current = node;
        for (char c : prefix) {
            int index = c - 'a';
            if (!current->children[index]) return nullptr;
            current = current->children[index].get();
        }
        return current;
    }

public:
    CompressedTrie() : root(make_unique<TrieNode>()) {
        loadDictionary();
        loadAbbreviations();
        loadPhonetics();
    }

    void autocomplete(const string& prefix) {
        TrieNode* node = searchPrefix(root.get(), prefix);
        vector<string> suggestions;

        getSuggestions(node, suggestions);

        cout << "Suggestions: \n";
        for (const auto& suggestion : suggestions) {
            cout << suggestion << endl;
        }
    }

    void customizeDictionary(const string& word) {
        cout << "Adding word to dictionary: " << word << endl;
        insertWord(root.get(), word);
    }

    void phoneticAutocomplete(const string& word) {
        cout << "Phonetic matches for \"" << word << "\": ";
        auto it = phoneticMap.find(word);
        if (it != phoneticMap.end()) {
            for (const auto& match : it->second) {
                cout << match << " ";
            }
            cout << endl;
        } else {
            cout << "No phonetic matches found." << endl;
        }
    }

    void loadNewDictionary(const string& filepath) {
        ifstream file(filepath);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filepath << endl;
            return;
        }

        string word;
        while (file >> word) {
            insertWord(root.get(), word);
        }
    }
    void autoConvertAbbreviation(string& sentence) {
        stringstream ss(sentence);
        string word, result;

        while (ss >> word) {
            string convertedWord = getAbbreviation(word);
            result += convertedWord + " ";
        }

        sentence = result;
        cout << "Converted sentence: " << sentence << endl;
    }

    string getAbbreviation(const string& word) {
        for (const auto& abbr : abbreviations) {
            if (abbr.first == word) return abbr.second;
        }
        return word;
    }

    void displayHelp() {
        cout << "\n--- Help Menu ---\n";
        cout << "1. Autocomplete word: Suggests possible words based on prefix\n";
        cout << "2. Customize dictionary: Add a new word to the dictionary\n";
        cout << "3. Phonetic matching: Provides suggestions based on phonetic similarity\n";
        cout << "4. Auto convert abbreviation: Converts abbreviations to full form\n";
        cout << "5. Load new dictionary: Loads new words from a text file into the dictionary\n";
        cout << "6. View Dictionary: Displays all the words in the current dictionary\n";
        cout << "7. Delete word: Removes a word from the dictionary\n";
        cout << "8. Exit: Exit the program\n";
    }

    void viewDictionary() {
        cout << "\n--- Current Dictionary ---\n";
        displayDictionary(root.get(), "");
    }

    void deleteWord(const string& word) {
        if (removeWord(root.get(), word)) {
            cout << "Word \"" << word << "\" deleted from the dictionary.\n";
        } else {
            cout << "Word \"" << word << "\" not found in the dictionary.\n";
        }
    }

private:
    void displayDictionary(TrieNode* node, string prefix) {
        if (node->isEndOfWord) {
            cout << prefix << endl;
        }
        for (int i = 0; i < 26; ++i) {
            if (node->children[i]) {
                char nextChar = 'a' + i;
                displayDictionary(node->children[i].get(), prefix + nextChar);
            }
        }
    }

    bool removeWord(TrieNode* node, const string& word, int depth = 0) {
        if (!node) return false;

        if (depth == word.size()) {
            if (!node->isEndOfWord) return false;
            node->isEndOfWord = false;

            bool canDelete = true;
            for (int i = 0; i < 26; ++i) {
                if (node->children[i]) {
                    canDelete = false; // There is at least one child
                    break;
                }
            }
            return canDelete;
        }

        int index = word[depth] - 'a'; // Get index of character
        if (!removeWord(node->children[index].get(), word, depth + 1)) return false;

        node->children[index].reset();

        bool canDelete = !node->isEndOfWord;
        for (int i = 0; i < 26; ++i) {
            if (node->children[i]) {
                canDelete = false;
                break;
            }
        }
        return canDelete;
    }
};

void menu() {
    CompressedTrie trie;
    int choice;
    string input;

    do {
        cout << "\n--- Main Menu ---\n";
        cout << "1. Autocomplete\n";
        cout << "2. Phonetic matching\n";
        cout << "3. Auto convert abbreviation\n";
        cout << "4. Customize dictionary\n";
        cout << "5. Load new dictionary\n";
        cout << "6. View dictionary\n";
        cout << "7. Delete word\n";
        cout << "8. Help\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter prefix: ";
                cin >> input;
                trie.autocomplete(input);
                break;

            case 2:
                cout << "Enter word for phonetic matching: ";
                cin >> input;
                trie.phoneticAutocomplete(input);
                break;

            case 3:
                cout << "Enter sentence to auto-convert abbreviation: ";
                cin.ignore();
                getline(cin, input);
                trie.autoConvertAbbreviation(input);
                break;

            case 4:
                cout << "Enter word to add to dictionary: ";
                cin >> input;
                trie.customizeDictionary(input);
                break;

            case 5:
                cout << "Enter filepath of new dictionary: ";
                cin >> input;
                trie.loadNewDictionary(input);
                break;

            case 6:
                trie.viewDictionary();
                break;

            case 7:
                cout << "Enter word to delete: ";
                cin >> input;
                trie.deleteWord(input);
                break;

            case 8:
                trie.displayHelp();
                break;

            case 9:
                cout << "Exiting program." << endl;
                break;

            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 9);
}
int main() {
    menu();
    return 0;
}
