/* =====================================================================
   DFA-BASED LEXER  --  longest-match tokenizer driven by a user-supplied DFA
   ===================================================================== */

#include <stdio.h>
#include <string.h>

#define MAX_STATES  50
#define MAX_SYMBOLS 20
#define NAME_LEN    20
#define BUF_LEN     100

char states[MAX_STATES][NAME_LEN];
char symbols[MAX_SYMBOLS][NAME_LEN];
int  trans[MAX_STATES][MAX_SYMBOLS];
int  accepting[MAX_STATES];

int num_states, num_symbols, num_accepting, start_state;

/* ---------------- helper functions ---------------- */

/* index of a state name, -1 if it does not exist */
int get_state_idx(char *name) {
    for (int i = 0; i < num_states; i++)
        if (strcmp(states[i], name) == 0)
            return i;
    return -1;
}

/* index of an input symbol, -1 if unknown */
int get_symbol_idx(char *name) {
    for (int j = 0; j < num_symbols; j++)
        if (strcmp(symbols[j], name) == 0)
            return j;
    return -1;
}

/* 1 if the state index is a final/accepting state */
int is_accepting(int state) {
    if (state < 0) return 0;
    for (int i = 0; i < num_accepting; i++)
        if (accepting[i] == state)
            return 1;
    return 0;
}

/* ---------------- the scanner ---------------- */

void test_string(char *input) {
    int len = strlen(input);
    int i   = 0;

    printf("\n--- Scanning \"%s\" ---\n", input);

    while (i < len) {
        int  state = start_state;
        int  blen  = 0;
        int  bad   = 0;
        char buffer[BUF_LEN];

        /* --- longest match: run until stuck --- */
        while (i < len) {
            char temp[2];
            temp[0] = input[i];
            temp[1] = '\0';

            int sym = get_symbol_idx(temp);

            if (sym == -1) {
                if (blen == 0) {
                    buffer[blen++] = input[i++];
                    bad = 1;
                }
                break;
            }

            if (trans[state][sym] == -1) {
                if (blen == 0) {
                    buffer[blen++] = input[i++];
                    bad = 1;
                }
                break;
            }

            state = trans[state][sym];
            buffer[blen++] = input[i++];
        }

        buffer[blen] = '\0';

        /* --- classify the buffer --- */
        if (!bad && blen > 0 && is_accepting(state))
            printf("Recognized token   : <%s, %s>\n",
                   buffer, states[state]);
        else
            printf("Unrecognized token : %s\n", buffer);
    }
}

/* ---------------- driver ---------------- */

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
        }
        else
            break;
    }

    return 0;
}