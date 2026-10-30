
#include <stdio.h>
#include <stdlib.h>
#define MAX 100

void display(int a[], int n, int h)
{
    int i, total = 0;
    printf("Head: %d", h);
    for (i = 0; i < n; i++)
    {
        total += abs(h - a[i]);
        h = a[i];
        printf(" -> %d", h);
    }
    printf("\nTotal movement = %d\n", total);
}

void sort(int a[], int n)
{
    int i, j, t;
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (a[j] > a[j + 1])
            {
                t = a[j];
                a[j] = a[j + 1];
                a[j + 1] = t;
            }
}

void FCFS(int r[], int n, int h)
{
    printf("\nFCFS\n");
    display(r, n, h);
}

void SSTF(int r[], int n, int h)
{
    int done[MAX] = {0}, s[MAX];
    int i, j, p, min, start = h;

    for (i = 0; i < n; i++)
    {
        min = 99999;
        p = -1;
        for (j = 0; j < n; j++)
            if (!done[j] && abs(h - r[j]) < min)
                min = abs(h - r[j]), p = j;

        done[p] = 1;
        s[i] = r[p];
        h = r[p];
    }
    printf("\nSSTF\n");
    display(s, n, start);
}

void SCAN(int r[], int n, int h, int size, int dir)
{
    int a[MAX], s[MAX + 1], i, k = 0;
    for (i = 0; i < n; i++)
        a[i] = r[i];
    sort(a, n);
    if (dir)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= h)
                s[k++] = a[i];
        s[k++] = size - 1;
        for (i = n - 1; i >= 0; i--)
            if (a[i] < h)
                s[k++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= h)
                s[k++] = a[i];
        s[k++] = 0;
        for (i = 0; i < n; i++)
            if (a[i] > h)
                s[k++] = a[i];
    }
    printf("\nSCAN\n");
    display(s, k, h);
}

void CSCAN(int r[], int n, int h, int size, int dir)
{
    int a[MAX], s[MAX + 2], i, k = 0;
    for (i = 0; i < n; i++)
        a[i] = r[i];
    sort(a, n);

    if (dir)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= h)
                s[k++] = a[i];
        s[k++] = size - 1;
        s[k++] = 0;
        for (i = 0; i < n; i++)
            if (a[i] < h)
                s[k++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= h)
                s[k++] = a[i];
        s[k++] = 0;
        s[k++] = size - 1;
        for (i = n - 1; i >= 0; i--)
            if (a[i] > h)
                s[k++] = a[i];
    }
    printf("\nCSCAN\n");
    display(s, k, h);
}

void LOOK(int r[], int n, int h, int dir)
{
    int a[MAX], s[MAX], i, k = 0;
    for (i = 0; i < n; i++)
        a[i] = r[i];
    sort(a, n);

    if (dir)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= h)
                s[k++] = a[i];
        for (i = n - 1; i >= 0; i--)
            if (a[i] < h)
                s[k++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= h)
                s[k++] = a[i];
        for (i = 0; i < n; i++)
            if (a[i] > h)
                s[k++] = a[i];
    }
    printf("\nLOOK\n");
    display(s, k, h);
}

void CLOOK(int r[], int n, int h, int dir)
{
    int a[MAX], s[MAX], i, k = 0;
    for (i = 0; i < n; i++)
        a[i] = r[i];
    sort(a, n);

    if (dir)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= h)
                s[k++] = a[i];
        for (i = 0; i < n; i++)
            if (a[i] < h)
                s[k++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= h)
                s[k++] = a[i];
        for (i = n - 1; i >= 0; i--)
            if (a[i] > h)
                s[k++] = a[i];
    }
    printf("\nCLOOK\n");
    display(s, k, h);
}

int main()
{
    int r[MAX], n, h, prev, size, dir, ch, i;

    printf("Number of requests: ");
    scanf("%d", &n);
    if (n < 1 || n > MAX)
        return 1;

    printf("Request queue: ");
    for (i = 0; i < n; i++)
        scanf("%d", &r[i]);

    printf("Initial head: ");
    scanf("%d", &h);
    printf("Previous head: ");
    scanf("%d", &prev);
    printf("Number of cylinders: ");
    scanf("%d", &size);

    if (size < 1 || h < 0 || h >= size ||
        prev < 0 || prev >= size)
        return 1;

    for (i = 0; i < n; i++)
        if (r[i] < 0 || r[i] >= size)
            return 1;

    dir = (h >= prev);
    printf("Direction: %s\n", dir ? "Right" : "Left");

    do
    {
        printf("\n1.FCFS  2.SSTF  3.SCAN  4.CSCAN\n");
        printf("5.LOOK  6.CLOOK  7.Exit\nChoice: ");
        scanf("%d", &ch);

        switch (ch)
        {
        case 1:
            FCFS(r, n, h);
            break;
        case 2:
            SSTF(r, n, h);
            break;
        case 3:
            SCAN(r, n, h, size, dir);
            break;
        case 4:
            CSCAN(r, n, h, size, dir);
            break;
        case 5:
            LOOK(r, n, h, dir);
            break;
        case 6:
            CLOOK(r, n, h, dir);
            break;
        case 7:
            printf("Exit\n");
            break;
        default:
            printf("Invalid choice\n");
        }
    } while (ch != 7);

    return 0;
}
