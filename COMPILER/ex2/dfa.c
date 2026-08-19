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
    char token[100] = "";
    int token_len = 0;
    
    for (int i = 0; input[i] != '\0'; i++) {
        char sym[20];
        sprintf(sym, "%c", input[i]);
        int sym_idx = get_symbol_idx(sym);
        
        // Invalid symbol - output token and this symbol
        if (sym_idx == -1) {
            if (token_len > 0) {
                if (is_accepting(current))
                    printf("Recognized token : <%s,%s>\n", token, states[current]);
                else
                    printf("Unrecognized token : <%s>\n", token);
                token_len = 0;
            }
            printf("Unrecognized token : <%c>\n", input[i]);
            current = start_state;
            continue;
        }
        
        // Valid symbol - check transition
        int next = trans[current][sym_idx];
        if (next == -1) {
            // No transition - output token, reset, try this symbol
            if (token_len > 0) {
                if (is_accepting(current))
                    printf("Recognized token : <%s,%s>\n", token, states[current]);
                else
                    printf("Unrecognized token : <%s>\n", token);
                token_len = 0;
            }
            current = start_state;
            int next2 = trans[current][sym_idx];
            if (next2 != -1) {
                token[token_len++] = input[i];
                current = next2;
            }
        } else {
            // Valid transition - add to token
            token[token_len++] = input[i];
            current = next;
        }
    }
    
    // Output final token
    if (token_len > 0) {
        if (is_accepting(current))
            printf("Recognized token : <%s,%s>\n", token, states[current]);
        else
            printf("Unrecognized token : <%s>\n", token);
    }
}

int main() {
    char temp[20], input[100];
    int choice;
    
    printf("=== DFA LEXER ===\n\n");
    printf("No. of states: ");
    scanf("%d", &num_states);
    getchar();
    
    printf("State names: ");
    for (int i = 0; i < num_states; i++)
        scanf("%s", states[i]);
    getchar();
    
    printf("No. of symbols: ");
    scanf("%d", &num_symbols);
    getchar();
    
    printf("Symbol names: ");
    for (int i = 0; i < num_symbols; i++)
        scanf("%s", symbols[i]);
    getchar();
    
    printf("Start state: ");
    scanf("%s", temp);
    getchar();
    start_state = get_state_idx(temp);
    
    printf("No. of accepting states: ");
    scanf("%d", &num_accepting);
    getchar();
    
    if (num_accepting > 0) {
        printf("Accepting states: ");
        for (int i = 0; i < num_accepting; i++) {
            scanf("%s", temp);
            accepting[i] = get_state_idx(temp);
        }
        getchar();
    }
    
    printf("\nTransition table:\n");
    for (int i = 0; i < num_states; i++) {
        for (int j = 0; j < num_symbols; j++) {
            scanf("%s", temp);
            if (strcmp(temp, "#") == 0 || strcmp(temp, "-1") == 0)
                trans[i][j] = -1;
            else
                trans[i][j] = get_state_idx(temp);
        }
    }
    getchar();
    
    printf("\nDFA ready!\n");
    
    while (1) {
        printf("\n1. Test  2. Exit: ");
        scanf("%d", &choice);
        getchar();
        
        if (choice == 1) {
            printf("Input: ");
            scanf("%s", input);
            getchar();
            test_string(input);
        } else
            break;
    }
    
    return 0;
}