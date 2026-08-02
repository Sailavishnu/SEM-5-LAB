/* ================================================================
   HAMMING CODE - MODULAR PROGRAM (Encode + Error Detect/Correct)
   ================================================================
   Convention used (matches faculty's board method):

   Position :  ... 11 10  9  8  7  6  5  4  3  2  1
   Type     :  ... D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1

   - Data bits fill all NON power-of-2 positions.
   - The FIRST character typed by the user goes into the HIGHEST
     numbered data slot (e.g. D7), and the LAST character goes into
     the LOWEST numbered data slot (D1). This matches reading the
     frame right-to-left as D1 D2 D3 ... on paper.
   - Parity bit Pk sits at position 2^(k-1) and covers every position
     whose binary representation has that bit set
     ("take k, skip k, take k, skip k ...").
   ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX 100

int code[MAX];      /* bit value at each position (1-indexed)      */
int filled[MAX];    /* whether that position's value is known yet  */
char label[MAX][4]; /* "D7", "P2" ... text label for each position */

/* ---------- helper ---------- */
int isPowerOf2(int pos)
{
    return pos > 0 && (pos & (pos - 1)) == 0;
}

/* ================= STEP 1 & 2 : find p ================= */
int findParityBits(int n)
{
    int p = 1;
    printf("\nStep 2:\nFinding parity bits (p)\n");
    while (1)
    {
        printf("\nTry p = %d\n%d + %d + 1 <= 2^%d  ", p, n, p, p);
        if (n + p + 1 <= (int)pow(2, p))
        {
            printf("-> Yes\n");
            break;
        }
        else
        {
            printf("-> No\n");
            p++;
        }
    }
    printf("\nRequired parity bits = %d\n", p);
    return p;
}

/* ============ build D7..D1 / P4..P1 style labels ============ */
void assignLabels(int totalLen, int p)
{
    int n = totalLen - p;
    int dNum = n;
    for (int j = totalLen; j >= 1; j--)
    {
        if (isPowerOf2(j))
        {
            int k = (int)(log2(j)) + 1;
            sprintf(label[j], "P%d", k);
        }
        else
        {
            sprintf(label[j], "D%d", dNum);
            dNum--;
        }
    }
}

/* ================= STEP 3 : place data bits ================= */
void placeDataBits(const char *data, int totalLen)
{
    int idx = 0;
    for (int j = totalLen; j >= 1; j--)
    {
        if (!isPowerOf2(j))
        {
            code[j] = data[idx] - '0';
            filled[j] = 1;
            idx++;
        }
        else
        {
            code[j] = 0;
            filled[j] = 0;
        }
    }
}

/* ================= display the frame (position/type/value) ================= */
void displayFrame(int totalLen)
{
    printf("\nPosition :\n");
    for (int j = totalLen; j >= 1; j--)
        printf("%3d", j);

    printf("\n\nType :\n");
    for (int j = totalLen; j >= 1; j--)
        printf("%3s", label[j]);

    printf("\n\nValue :\n");
    for (int j = totalLen; j >= 1; j--)
    {
        if (filled[j])
            printf("%3d", code[j]);
        else
            printf("  _");
    }
    printf("\n");
}

/* ================= STEP 4 : calculate each parity bit ================= */
void calculateParity(int totalLen, int p, int parityType)
{
    printf("\nStep 4:\nCalculating parity bits (%s parity)\n",
           parityType == 0 ? "Even" : "Odd");

    for (int k = 1; k <= p; k++)
    {
        int parityPos = 1 << (k - 1);
        if (parityPos > totalLen)
            continue;

        printf("\nCalculate P%d\n\nChecking positions\n", k);
        for (int j = 1; j <= totalLen; j++)
            if (j & parityPos)
                printf("%d ", j);

        printf("\n\nValues\n");
        int count = 0;
        for (int j = 1; j <= totalLen; j++)
        {
            if (j & parityPos)
            {
                if (j == parityPos)
                    printf("_ ");
                else
                {
                    printf("%d ", code[j]);
                    count += code[j];
                }
            }
        }

        int bit;
        if (parityType == 0)
            bit = (count % 2 == 0) ? 0 : 1; /* even */
        else
            bit = (count % 2 == 0) ? 1 : 0; /* odd  */

        code[parityPos] = bit;
        filled[parityPos] = 1;

        printf("\n\nP%d = %d\n", k, bit);
    }
}

/* ================= orchestrator : encode ================= */
void generateHammingCode(const char *data, int parityType)
{
    int n = strlen(data);
    printf("\nStep 1:\nNumber of data bits (n) = %d\n", n);

    int p = findParityBits(n);
    int totalLen = n + p;
    printf("Total bits = %d\n", totalLen);

    assignLabels(totalLen, p);
    placeDataBits(data, totalLen);

    printf("\nStep 3:\nInsert empty parity locations\n");
    displayFrame(totalLen);

    calculateParity(totalLen, p, parityType);

    printf("\nFinal Hamming Code\n");
    displayFrame(totalLen);

    printf("\n\nData to be transmitted: ");
    for (int j = totalLen; j >= 1; j--)
        printf("%d", code[j]);
    printf("\n");
}

/* ================= detect error (syndrome) ================= */
int detectError(int totalLen, int p, int parityType)
{
    int syndrome = 0;
    printf("\nChecking received code:\n");

    for (int k = p; k >= 1; k--)
    {
        int parityPos = 1 << (k - 1);
        if (parityPos > totalLen)
            continue;

        int count = 0;
        for (int j = 1; j <= totalLen; j++)
            if (j & parityPos)
                count += code[j]; /* parity bit itself included */

        int checkBit;
        if (parityType == 0)
            checkBit = (count % 2 == 0) ? 0 : 1;
        else
            checkBit = (count % 2 == 0) ? 1 : 0;

        printf("Checking P%d ... = %d\n", k, checkBit);
        if (checkBit)
            syndrome += parityPos;
    }

    printf("\nSyndrome (binary value of check bits) = %d\n", syndrome);
    return syndrome;
}

/* ================= correct the flagged bit ================= */
void correctError(int errorPos, int totalLen)
{
    if (errorPos == 0)
    {
        printf("\nNo error detected.\n");
    }
    else if (errorPos > totalLen)
    {
        printf("\nError position out of range - more than 1 bit may be corrupted.\n");
    }
    else
    {
        printf("\nError found at Position %d -> flipping bit to correct.\n", errorPos);
        code[errorPos] = code[errorPos] ? 0 : 1;
    }
    printf("\nCorrected Code\n");
    displayFrame(totalLen);
    printf("\n");
}

/* ================= pull the original data bits back out ================= */
void extractData(int totalLen)
{
    printf("\nOriginal Data: ");
    for (int j = totalLen; j >= 1; j--)
        if (!isPowerOf2(j))
            printf("%d", code[j]);
    printf("\n");
}

/* ================= main menu ================= */
int main()
{
    int choice, parityType, totalLen, p;
    char input[MAX];

    printf("=== HAMMING CODE PROGRAM ===\n");
    printf("1. Encode data (generate Hamming code)\n");
    printf("2. Check received code for error and correct it\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    printf("Enter parity type (0 = Even, 1 = Odd): ");
    scanf("%d", &parityType);

    if (choice == 1)
    {
        printf("Enter data bits (e.g. 1011010): ");
        scanf("%s", input);
        generateHammingCode(input, parityType);
    }
    else if (choice == 2)
    {
        printf("Enter received code (position N ... position 1, e.g. 10101010111): ");
        scanf("%s", input);

        totalLen = strlen(input);
        p = 0;
        while ((int)pow(2, p) < totalLen + 1)
            p++;

        assignLabels(totalLen, p);
        for (int j = totalLen, idx = 0; j >= 1; j--, idx++)
        {
            code[j] = input[idx] - '0';
            filled[j] = 1;
        }

        printf("\nReceived Code\n");
        displayFrame(totalLen);

        int syndrome = detectError(totalLen, p, parityType);
        correctError(syndrome, totalLen);
        extractData(totalLen);
    }
    else
    {
        printf("Invalid choice.\n");
    }

    return 0;
}