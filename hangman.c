/**
 * Project: Hangman Game
 * Author: [Furkan Topraktepe]
 * Description: A console-based Hangman game featuring dynamic file I/O, 
 * difficulty levels, scoring system, and a leaderboard.
 * Developed as a semester project for C Programming.
 */

// Required to prevent Visual Studio security warnings
#define _CRT_SECURE_NO_WARNINGS

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Windows specific library for console colors and clearing
#ifdef _WIN32
#include <windows.h>
#endif

// Constants
#define MAX_WORD_LENGTH 50
#define MAX_HINT_LENGTH 150
#define MAX_WORD_COUNT 100
#define SCORE_FILE "scores.txt"

// ANSI Color Codes
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_PURPLE  "\033[1;35m"

// Structure to hold word and hint data
struct WordWithHint {
    char word[MAX_WORD_LENGTH];
    char hint[MAX_HINT_LENGTH];
};

// Structure for the leaderboard
struct Score {
    char name[30];
    int points;
};

// Global array to store loaded words
struct WordWithHint wordList[MAX_WORD_COUNT];

// Function Prototypes
void displayWord(const char secretWord[], const bool guessedLetters[]);
void drawHangman(int mistakes, int maxMistakes);
bool checkWin(const char secretWord[], const bool guessedLetters[]);
int loadWordsFromFile(const char* filename);
void playGame();
void saveScore(int points);
void showLeaderboard();
void clearScreen();
void enableWindowsColors();

int main() {
    // Enable ANSI colors for Windows 10+
    enableWindowsColors();
    
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    int choice;

    do {
        clearScreen();
        printf("\n" COLOR_BLUE "==========================================" COLOR_RESET "\n");
        printf(COLOR_CYAN "                     HANGMAN GAME       " COLOR_RESET "\n");
        printf(COLOR_BLUE "==========================================" COLOR_RESET "\n");
        printf("1. Play Game\n");
        printf("2. Leaderboard\n");
        printf("3. Exit\n");
        printf(COLOR_YELLOW "Enter choice: " COLOR_RESET);
        
        // Input validation to prevent infinite loops on char input
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); // Clear buffer
            choice = 0; 
        }

        switch (choice) {
            case 1: 
                playGame(); 
                break;
            case 2: 
                showLeaderboard(); 
                break;
            case 3: 
                printf(COLOR_RED "Exiting game... Goodbye!" COLOR_RESET "\n"); 
                break;
            default: 
                printf("Invalid choice! Press Enter to try again."); 
                getchar(); getchar();
        }
    } while (choice != 3);

    return 0;
}

void playGame() {
    clearScreen();
    
    // 1. Difficulty Selection
    int diffChoice, maxMistakes, scoreMultiplier;
    printf("\n" COLOR_PURPLE "--- DIFFICULTY LEVEL ---" COLOR_RESET "\n");
    printf("1. Easy   (9 Lives - x1 Score)\n");
    printf("2. Medium (6 Lives - x2 Score)\n");
    printf("3. Hard   (4 Lives - x3 Score)\n");
    printf("Select difficulty: ");
    scanf("%d", &diffChoice);

    switch(diffChoice) {
        case 1: maxMistakes = 9; scoreMultiplier = 1; break;
        case 3: maxMistakes = 4; scoreMultiplier = 3; break;
        default: maxMistakes = 6; scoreMultiplier = 2; break; // Default to Medium
    }

    clearScreen();
    
    // 2. Theme Selection
    int themeChoice;
    char filename[50];
    char themeName[50];

    printf("\n" COLOR_YELLOW "--- THEME SELECTION ---" COLOR_RESET "\n");
    printf("1. Cities\n2. Foods\n3. Movies\n4. TV Series\n5. Mixed\n");
    printf("Select theme: ");
    scanf("%d", &themeChoice);

    switch (themeChoice) {
        case 1: strcpy(filename, "cities.txt"); strcpy(themeName, "Cities"); break;
        case 2: strcpy(filename, "foods.txt"); strcpy(themeName, "Foods"); break;
        case 3: strcpy(filename, "movies.txt"); strcpy(themeName, "Movies"); break;
        case 4: strcpy(filename, "series.txt"); strcpy(themeName, "TV Series"); break;
        case 5: strcpy(filename, "mixed.txt"); strcpy(themeName, "Mixed"); break;
        default: strcpy(filename, "cities.txt"); strcpy(themeName, "Cities"); break;
    }

    // Load words from the selected file
    int totalWords = loadWordsFromFile(filename);
    if (totalWords == 0) {
        printf(COLOR_RED "ERROR: Could not load '%s'. Check if file exists." COLOR_RESET "\n", filename);
        printf("\nPress Enter to return to menu...");
        getchar(); getchar();
        return;
    }

    // Pick a random word
    int wordIndex = rand() % totalWords;
    const char* secretWord = wordList[wordIndex].word;
    const char* hint = wordList[wordIndex].hint;
    int wordLength = (int)strlen(secretWord);

    bool guessedLetters[26] = { false };
    int mistakes = 0;
    int currentScore = 100 * scoreMultiplier;

    // --- GAME LOOP ---
    while (mistakes < maxMistakes) {
        clearScreen();
        
        printf("\n" COLOR_CYAN ">>> Theme: %s <<<" COLOR_RESET "\n", themeName);
        printf("Hint: %s\n", hint);
        printf("\n----------------------------------------------\n");
        
        drawHangman(mistakes, maxMistakes);
        displayWord(secretWord, guessedLetters);
        
        printf(COLOR_BLUE "Score: %d (Lives Left: %d)" COLOR_RESET "\n", currentScore, maxMistakes - mistakes);

        char guess;
        printf("\nEnter a letter or press '1' for a HINT [-30 Points]: ");
        scanf(" %c", &guess);

        // --- JOKER / HINT LOGIC ---
        if (guess == '1') {
            if (currentScore >= 30) {
                currentScore -= 30;
                char letterToReveal = 0;
                
                // Find the first unrevealed letter
                for(int i = 0; i < wordLength; i++) {
                    if(!guessedLetters[secretWord[i] - 'a']) {
                        letterToReveal = secretWord[i];
                        break;
                    }
                }
                
                if(letterToReveal != 0) {
                    guessedLetters[letterToReveal - 'a'] = true;
                    printf(COLOR_GREEN "HINT USED! Revealed: '%c'" COLOR_RESET "\n", letterToReveal);
                    printf("\a"); // Beep sound
                    // Sleep(1000); // Optional delay
                }
                
                // Check win immediately after hint
                if (checkWin(secretWord, guessedLetters)) { 
                    /* Logic continues below */ 
                } else {
                    continue; // Skip the rest of the loop
                }
            } else {
                printf(COLOR_RED "Not enough points for a hint!" COLOR_RESET "\n");
                printf("Press Enter to continue...");
                getchar(); getchar();
                continue;
            }
        }

        guess = tolower(guess);
        // Validate input (a-z)
        if (guess < 'a' || guess > 'z') continue;

        // Check if already guessed
        if (guessedLetters[guess - 'a']) {
            printf(COLOR_YELLOW "You already guessed '%c'." COLOR_RESET "\n", guess);
            printf("Press Enter...");
            getchar(); getchar(); 
            continue;
        }

        guessedLetters[guess - 'a'] = true;
        bool found = false;
        for (int i = 0; i < wordLength; i++) {
            if (secretWord[i] == guess) found = true;
        }

        if (!found) {
            printf("\a"); // Error sound
            mistakes++;
            currentScore -= (10 * scoreMultiplier); 
            if (currentScore < 0) currentScore = 0;
        }

        // Check Win Condition
        if (checkWin(secretWord, guessedLetters)) {
            clearScreen();
            printf("\n");
            drawHangman(mistakes, maxMistakes);
            displayWord(secretWord, guessedLetters);
            
            printf("\n" COLOR_GREEN "**********************************" COLOR_RESET "\n");
            printf(COLOR_GREEN " CONGRATULATIONS! WORD: %s" COLOR_RESET "\n", secretWord);
            printf(COLOR_YELLOW " TOTAL SCORE: %d" COLOR_RESET "\n", currentScore);
            printf(COLOR_GREEN "**********************************" COLOR_RESET "\n");
            printf("\a\a\a"); // Victory sound
            
            saveScore(currentScore);
            
            printf("\nPress Enter to return to menu...");
            getchar(); getchar();
            break; 
        }
    }

    // Check Loss Condition
    if (mistakes >= maxMistakes) {
        clearScreen();
        drawHangman(mistakes, maxMistakes);
        printf(COLOR_RED "\nGAME OVER! The word was: %s" COLOR_RESET "\n", secretWord);
        printf("\nPress Enter to return to menu...");
        getchar(); getchar();
    }
}

// Function to clear console screen based on OS
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Enable Virtual Terminal Processing for Windows colors
void enableWindowsColors() {
    #ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
    SetConsoleMode(hOut, dwMode);
    #endif
}

// Scale the hangman drawing based on total lives
void drawHangman(int mistakes, int maxMistakes) {
    const char* hangmanArt[] = {
        "  +---+\n  |   |\n      |\n      |\n      |\n      |\n=========\n", 
        "  +---+\n  |   |\n  O   |\n      |\n      |\n      |\n=========\n", 
        "  +---+\n  |   |\n  O   |\n  |   |\n      |\n      |\n=========\n", 
        "  +---+\n  |   |\n  O   |\n /|   |\n      |\n      |\n=========\n", 
        "  +---+\n  |   |\n  O   |\n /|\\  |\n      |\n      |\n=========\n", 
        "  +---+\n  |   |\n  O   |\n /|\\  |\n /    |\n      |\n=========\n", 
        "  +---+\n  |   |\n  O   |\n /|\\  |\n / \\  |\n      |\n=========\n"  
    };

    int artIndex;
    if (mistakes == 0) artIndex = 0;
    else if (mistakes >= maxMistakes) artIndex = 6;
    else {
        // Map current mistakes to the 6 stages of drawing
        artIndex = (mistakes * 6) / maxMistakes;
        if (artIndex == 0 && mistakes > 0) artIndex = 1; 
    }
    printf(COLOR_YELLOW "%s" COLOR_RESET, hangmanArt[artIndex]);
}

void saveScore(int points) {
    char name[30];
    printf("Enter your name for the leaderboard (No spaces): ");
    scanf("%s", name);
    
    FILE* file = fopen(SCORE_FILE, "a");
    if (file) {
        fprintf(file, "%s;%d\n", name, points);
        fclose(file);
        printf(COLOR_CYAN "Score saved successfully!\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "Error saving score!\n" COLOR_RESET);
    }
}

void showLeaderboard() {
    clearScreen();
    struct Score scores[100];
    int count = 0;
    
    FILE* file = fopen(SCORE_FILE, "r");
    if (!file) {
        printf(COLOR_YELLOW "\nNo scores recorded yet. Be the first!\n" COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar(); getchar();
        return;
    }
    
    // Read scores from file
    while (fscanf(file, "%[^;];%d\n", scores[count].name, &scores[count].points) != EOF) {
        count++;
        if (count >= 100) break;
    }
    fclose(file);

    // Bubble Sort (Descending Order)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j].points < scores[j + 1].points) {
                struct Score temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    printf("\n" COLOR_BLUE "--- TOP 5 LEADERBOARD ---" COLOR_RESET "\n");
    int limit = (count < 5) ? count : 5;
    printf("%-15s %s\n", "NAME", "SCORE");
    printf("------------------------\n");
    for (int i = 0; i < limit; i++) {
        printf("%-15s " COLOR_YELLOW "%d" COLOR_RESET "\n", scores[i].name, scores[i].points);
    }
    printf("\nPress Enter to return to menu...");
    getchar(); getchar(); 
}

// Function to read words from a file formatted as "word;hint"
int loadWordsFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) return 0;
    
    char line[250];
    int count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        
        char* token = strtok(line, ";");
        if (token) strcpy(wordList[count].word, token);
        
        token = strtok(NULL, ";");
        if (token) strcpy(wordList[count].hint, token);
        
        count++;
        if (count >= MAX_WORD_COUNT) break;
    }
    fclose(file);
    return count;
}

void displayWord(const char secretWord[], const bool guessedLetters[]) {
    printf("Word: ");
    for (int i = 0; secretWord[i] != '\0'; i++) {
        if (guessedLetters[secretWord[i] - 'a']) {
            printf(COLOR_GREEN "%c " COLOR_RESET, secretWord[i]);
        } else {
            printf("_ ");
        }
    }
    printf("\n");
}

bool checkWin(const char secretWord[], const bool guessedLetters[]) {
    for (int i = 0; secretWord[i] != '\0'; i++) {
        if (!guessedLetters[secretWord[i] - 'a']) return false;
    }
    return true;
}
