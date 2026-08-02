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
    int current = start_state;
    
    for (int i = 0; input[i] != '\0'; i++) {
        char sym[20];
        sprintf(sym, "%c", input[i]);
        
        int sym_idx = get_symbol_idx(sym);
        if (sym_idx == -1) {
            printf("REJECTED (invalid symbol)\n");
            return;
        }
        
        int next = trans[current][sym_idx];
        if (next == -1) {
            printf("REJECTED (no transition)\n");
            return;
        }
        
        current = next;
    }
    
    printf(is_accepting(current) ? "ACCEPTED\n" : "REJECTED (not accepting)\n");
}

int main() {
    char temp[20];
    int choice;
    char input[100];
    
    printf("=== DFA LEXER ===\n\n");
    
    printf("No. of states: ");
    fflush(stdout);
    if (scanf("%d", &num_states) != 1 || num_states <= 0 || num_states > MAX) {
        printf("ERROR: Invalid states count (1-%d)\n", MAX);
        return 1;
    }
    getchar();
    
    printf("State names (space-separated): ");
    fflush(stdout);
    for (int i = 0; i < num_states; i++) {
        if (scanf("%s", states[i]) != 1) {
            printf("ERROR: Failed to read state\n");
            return 1;
        }
    }
    getchar();
    
    printf("No. of symbols: ");
    fflush(stdout);
    if (scanf("%d", &num_symbols) != 1 || num_symbols <= 0 || num_symbols > MAX) {
        printf("ERROR: Invalid symbols count (1-%d)\n", MAX);
        return 1;
    }
    getchar();
    
    printf("Symbol names (space-separated, can be strings): ");
    fflush(stdout);
    for (int i = 0; i < num_symbols; i++) {
        if (scanf("%s", symbols[i]) != 1) {
            printf("ERROR: Failed to read symbol\n");
            return 1;
        }
    }
    getchar();
    
    printf("Start state: ");
    fflush(stdout);
    if (scanf("%s", temp) != 1) {
        printf("ERROR: Failed to read start state\n");
        return 1;
    }
    getchar();
    start_state = get_state_idx(temp);
    if (start_state == -1) {
        printf("ERROR: Start state '%s' not found\n", temp);
        return 1;
    }
    
    printf("No. of accepting states: ");
    fflush(stdout);
    if (scanf("%d", &num_accepting) != 1 || num_accepting < 0) {
        printf("ERROR: Invalid accepting states count\n");
        return 1;
    }
    getchar();
    
    if (num_accepting > 0) {
        printf("Accepting state names (space-separated): ");
        fflush(stdout);
        for (int i = 0; i < num_accepting; i++) {
            if (scanf("%s", temp) != 1) {
                printf("ERROR: Failed to read accepting state\n");
                return 1;
            }
            int idx = get_state_idx(temp);
            if (idx == -1) {
                printf("ERROR: Accepting state '%s' not found\n", temp);
                return 1;
            }
            accepting[i] = idx;
        }
        getchar();
    }
    
    printf("\nTransition table (%d states x %d symbols):\n", num_states, num_symbols);
    fflush(stdout);
    for (int i = 0; i < num_states; i++) {
        for (int j = 0; j < num_symbols; j++) {
            char token[20];
            if (scanf("%s", token) != 1) {
                printf("ERROR: Failed to read transition\n");
                return 1;
            }
            
            if (strcmp(token, "#") == 0 || strcmp(token, "-1") == 0) {
                trans[i][j] = -1;
            } else {
                int idx = get_state_idx(token);
                if (idx == -1) {
                    printf("ERROR: Invalid state '%s' in transition\n", token);
                    return 1;
                }
                trans[i][j] = idx;
            }
        }
    }
    getchar();
    
    printf("\nDFA initialized successfully!\n");
    
    do {
        printf("\n1. Test string  2. Exit: ");
        fflush(stdout);
        if (scanf("%d", &choice) != 1) {
            printf("ERROR: Invalid input\n");
            while (getchar() != '\n');
            continue;
        }
        getchar();
        
        if (choice == 1) {
            printf("Input string: ");
            fflush(stdout);
            if (scanf("%s", input) != 1) {
                printf("ERROR: Failed to read input\n");
                continue;
            }
            getchar();
            test_string(input);
        } else if (choice == 2) {
            printf("Exit\n");
        } else {
            printf("ERROR: Invalid choice (1 or 2)\n");
        }
    } while (choice != 2);
    
    return 0;
}