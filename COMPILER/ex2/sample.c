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

// Invalid symbol found - finalize current token
if (sym_idx == -1) {
if (token_len > 0) {
if (is_accepting(current)) {
printf("Recognized token : <%s, %s>\n", token, states[current]);
} else {
printf("Unrecognized token : %s\n", token);
}
}
// Reset for next token
current = start_state;
token_len = 0;
memset(token, 0, sizeof(token));

// Report the invalid symbol itself
printf("Unrecognized token : %c\n", input[i]);
continue;
}

// Valid symbol - try transition
int next = trans[current][sym_idx];
if (next == -1) {
// No transition available - finalize current token
if (token_len > 0) {
if (is_accepting(current)) {
printf("Recognized token : <%s, %s>\n", token, states[current]);
} else {
printf("Unrecognized token : %s\n", token);
}
}
// Start new token from this symbol
current = start_state;
token_len = 0;
memset(token, 0, sizeof(token));
int next2 = trans[current][sym_idx];
if (next2 != -1) {
token[token_len++] = input[i];
current = next2;
}
} else {
// Valid transition - continue building token
token[token_len++] = input[i];
current = next;
}
}

// Finalize remaining token at end of string
if (token_len > 0) {
if (is_accepting(current)) {
printf("Recognized token : <%s, %s>\n", token, states[current]);
} else {
printf("Unrecognized token : %s\n", token);
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