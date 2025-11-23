#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>

#define WORD_LENGTH 5
#define MAX_WORDS 15000


typedef struct Node {
    char word[WORD_LENGTH + 1];
    struct Node *next;
} Node;

char words[MAX_WORDS][WORD_LENGTH + 1];
int wordCount = 0;

// -------------------- LOAD WORDS --------------------
int loadWords(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Could not open %s\n", filename);
        exit(1);
    }

    while (fscanf(file, "%5s", words[wordCount]) != EOF) {
        for (int i = 0; i < WORD_LENGTH; i++)
            words[wordCount][i] = toupper(words[wordCount][i]);
        words[wordCount][WORD_LENGTH] = '\0';
        wordCount++;
        if (wordCount >= MAX_WORDS) break;
    }
    fclose(file);
    return wordCount;
}

// -------------------- FEEDBACK SIMULATION --------------------
void getFeedback(const char *target, const char *guess, char *feedback) {
    // G = green, Y = yellow, . = gray
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i])
            feedback[i] = 'G';
        else if (strchr(target, guess[i]))
            feedback[i] = 'Y';
        else
            feedback[i] = '.';
    }
    feedback[WORD_LENGTH] = '\0';
}

// -------------------- MATCH FUNCTION --------------------
bool matches(const char *word, const char *greens, const char *yellows, const char *yellow_pos, const char *grays) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (greens[i] != '.' && word[i] != greens[i]) return false;
        if (yellow_pos[i] != '.' && word[i] == yellow_pos[i]) return false;
    }

    // Must contain all yellow letters
    for (int i = 0; yellows[i]; i++)
        if (!strchr(word, yellows[i])) return false;

    // Must not contain gray letters
    for (int i = 0; grays[i]; i++)
        if (strchr(word, grays[i])) return false;

    return true;
}

// -------------------- PICK NEXT GUESS --------------------
void pickNextGuess(char *nextGuess, const char *greens, const char *yellows, const char *yellow_pos, const char *grays) {
    for (int i = 0; i < wordCount; i++) {
        if (matches(words[i], greens, yellows, yellow_pos, grays)) {
            strcpy(nextGuess, words[i]);
            return;
        }
    }
    // fallback
    strcpy(nextGuess, words[rand() % wordCount]);
}

// -------------------- UPDATE RULES FROM FEEDBACK --------------------
void updateRules(const char *guess, const char *feedback,
                 char *greens, char *yellows, char *yellow_pos, char *grays) {
    for (int i = 0; i < WORD_LENGTH; i++) {
        char g = guess[i];
        if (feedback[i] == 'G') {
            greens[i] = g;
        } else if (feedback[i] == 'Y') {
            if (!strchr(yellows, g)) {
                int len = strlen(yellows);
                yellows[len] = g;
                yellows[len + 1] = '\0';
            }
            yellow_pos[i] = g;
        } else if (feedback[i] == '.') {
            if (!strchr(grays, g) && !strchr(yellows, g) && !strchr(greens, g)) {
                int len = strlen(grays);
                grays[len] = g;
                grays[len + 1] = '\0';
            }
        }
    }
}

// -------------------- MAIN --------------------
int main() {
    srand(time(NULL));
    loadWords("valid-wordle-words.txt");

    // Choose random target
    const char *target = words[rand() % wordCount];
    printf("🤖 Auto Wordle started! (Target word chosen secretly)\n");
    // Uncomment this line to see target:
    // printf("[DEBUG] Target: %s\n", target);

    char greens[WORD_LENGTH + 1] = ".....";
    char yellows[32] = "";
    char yellow_pos[WORD_LENGTH + 1] = ".....";
    char grays[64] = "";

    char guess[WORD_LENGTH + 1];
    char feedback[WORD_LENGTH + 1];

    // First guess can be any word
    strcpy(guess, words[rand() % wordCount]);

    for (int attempt = 1; attempt <= 6; attempt++) {
        getFeedback(target, guess, feedback);

        printf("\nAttempt %d: Guess = %s → Feedback = %s\n", attempt, guess, feedback);

        if (strcmp(guess, target) == 0) {
            printf("\n🎉 Solved! The word was %s in %d attempts.\n", target, attempt);
            return 0;
        }

        updateRules(guess, feedback, greens, yellows, yellow_pos, grays);

        printf("Known so far:\n");
        printf("Green: %s\nYellow letters: %s\nGray letters: %s\n", greens, yellows, grays);

        pickNextGuess(guess, greens, yellows, yellow_pos, grays);
    }

    printf("\n❌ Failed to solve in 6 attempts. The word was %s.\n", target);
    return 0;
}
