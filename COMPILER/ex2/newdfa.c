#include <stdio.h>
#include <string.h>

#define MAX 50

int num_states, num_symbols, start_state, num_accepting;
char states[MAX][10], symbols[MAX][20];
int accepting[MAX];
int trans[MAX][MAX];

int get_state_idx(char *s) {
    for (int i = 0; i < num_states; i++)
        if (strcmp(states[i], s) == 0) return i;
    return -1;
}

int get_symbol_idx(char *s) {
    for (int i = 0; i < num_symbols; i++)
        if (strcmp(symbols[i], s) == 0) return i;
    return -1;
}

int is_accepting(int state) {
    for (int i = 0; i < num_accepting; i++)
        if (accepting[i] == state) return 1;
    return 0;
}

void test_string(char *input) {
    int i = 0;
    int len = strlen(input);

    while (i < len) {
        int current = start_state;
        int last_accepting_idx = -1;
        int last_accepting_state = -1;
        int j = i;

        // Trace forward as far as possible
        while (j < len) {
            char sym[20];
            sprintf(sym, "%c", input[j]);
            int sym_idx = get_symbol_idx(sym);

            if (sym_idx == -1) break; // Invalid character completely

            int next = trans[current][sym_idx];
            if (next == -1) break; // No transition rule available

            current = next;
            if (is_accepting(current)) {
                last_accepting_idx = j;
                last_accepting_state = current;
            }
            j++;
        }

        // Tokenization Decision
        if (last_accepting_idx != -1) {
            // Found a valid match along the path! Extract and print it
            int match_len = last_accepting_idx - i + 1;
            char token[100] = {0};
            strncpy(token, &input[i], match_len);
            printf("Recognized token : <%s, %s>\n", token, states[last_accepting_state]);
            i = last_accepting_idx + 1; // Move past the successfully recognized token
        } else {
            // No valid prefix state matches. The single starting character is unrecognized
            printf("Unrecognized token : %c\n", input[i]);
            i++; // Advance exactly one character to re-try
        }
    }
}

int main() {
    char temp[20];
    int choice;
    char input[100];

    printf("=== DFA LEXER ===\n\n");

    printf("No. of states: ");
    scanf("%d", &num_states);

    printf("State names (space-separated): ");
    for (int i = 0; i < num_states; i++)
        scanf("%s", states[i]);

    printf("No. of symbols: ");
    scanf("%d", &num_symbols);

    printf("Symbol names (space-separated, can be strings): ");
    for (int i = 0; i < num_symbols; i++)
        scanf("%s", symbols[i]);

    printf("Start state: ");
    scanf("%s", temp);
    start_state = get_state_idx(temp);

    printf("No. of accepting states: ");
    scanf("%d", &num_accepting);

    if (num_accepting > 0) {
        printf("Accepting state names (space-separated): ");
        for (int i = 0; i < num_accepting; i++) {
            scanf("%s", temp);
            accepting[i] = get_state_idx(temp);
        }
    }

    printf("\nTransition table (%d states x %d symbols):\n", num_states, num_symbols);
    printf("(Use '#' or '-1' for no transition/dead state)\n");
    for (int i = 0; i < num_states; i++) {
        for (int j = 0; j < num_symbols; j++) {
            scanf("%s", temp);
            if (strcmp(temp, "#") == 0 || strcmp(temp, "-1") == 0) {
                trans[i][j] = -1;
            } else {
                trans[i][j] = get_state_idx(temp);
            }
        }
    }

    printf("\nDFA initialized successfully!\n");

    do {
        printf("\n1. Test string  2. Exit: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Input string: ");
            scanf("%s", input);
            test_string(input);
        } else if (choice != 2) {
            printf("ERROR: Invalid choice (1 or 2)\n");
        }
    } while (choice != 2);

    return 0;
}




/*
1. Test string 2. Exit: 1Input string: babbcdabRecognized token : <bab, C>Unrecognized token : bUnrecognized token : cUnrecognized token : dUnrecognized token : aUnrecognized token : b  
...it check for longest match . bab result infinal state stil check for other , dont display recof, babb result in unrec . cz c is unrecog, and d unrecog , ab recognized this is that i want
*/