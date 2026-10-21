
#include <stdio.h>
#include <stdlib.h>
#define MAX 100

void display(int a[], int n, int h) {
    int i, total = 0;
    printf("%d", h);
    for (i = 0; i < n; i++) {
        total += abs(h - a[i]);
        h = a[i];
        printf(" -> %d", h);
    }
    printf("\nTotal movement = %d\n", total);
}

void sort(int a[], int n) {
    int i, j, t;
    for (i = 0; i < n; i++)
        for (j = i+1; j < n; j++)
            if (a[i] > a[j]) {
                t = a[i]; a[i] = a[j]; a[j] = t;
            }
}

void FCFS(int a[], int n, int h) {
    printf("FCFS: ");
    display(a, n, h);
}

void SSTF(int a[], int n, int h) {
    int b[MAX], used[MAX] = {0}, i, j, p, min, start = h;
    for (i = 0; i < n; i++) {
        p = -1; min = 999999;
        for (j = 0; j < n; j++)
            if (!used[j] && abs(h-a[j]) < min)
                min = abs(h-a[j]), p = j;
        b[i] = a[p]; used[p] = 1; h = a[p];
    }
    printf("SSTF: ");
    display(b, n, start);
}

void SCAN(int a[], int n, int h, int size, int d) {
    int b[MAX], i, k = 0;
    sort(a, n);
    if (d) {
        for (i=0;i<n;i++) if(a[i]>=h)b[k++]=a[i];
        b[k++]=size-1;
        for(i=n-1;i>=0;i--)if(a[i]<h)b[k++]=a[i];
    } else {
        for(i=n-1;i>=0;i--)if(a[i]<=h)b[k++]=a[i];
        b[k++]=0;
        for(i=0;i<n;i++)if(a[i]>h)b[k++]=a[i];
    }
    printf("SCAN: ");
    display(b, k, h);
}

void CSCAN(int a[], int n, int h, int size, int d) {
    int b[MAX], i, k=0;
    sort(a,n);
    if(d) {
        for(i=0;i<n;i++)if(a[i]>=h)b[k++]=a[i];
        b[k++]=size-1; b[k++]=0;
        for(i=0;i<n;i++)if(a[i]<h)b[k++]=a[i];
    } else {
        for(i=n-1;i>=0;i--)if(a[i]<=h)b[k++]=a[i];
        b[k++]=0; b[k++]=size-1;
        for(i=n-1;i>=0;i--)if(a[i]>h)b[k++]=a[i];
    }
    printf("CSCAN: ");
    display(b,k,h);
}

void LOOK(int a[], int n, int h, int d) {
    int b[MAX],i,k=0;
    sort(a,n);
    if(d) {
        for(i=0;i<n;i++)if(a[i]>=h)b[k++]=a[i];
        for(i=n-1;i>=0;i--)if(a[i]<h)b[k++]=a[i];
    } else {
        for(i=n-1;i>=0;i--)if(a[i]<=h)b[k++]=a[i];
        for(i=0;i<n;i++)if(a[i]>h)b[k++]=a[i];
    }
    printf("LOOK: ");
    display(b,k,h);
}

void CLOOK(int a[], int n, int h, int d) {
    int b[MAX],i,k=0;
    sort(a,n);
    if(d) {
        for(i=0;i<n;i++)if(a[i]>=h)b[k++]=a[i];
        for(i=0;i<n;i++)if(a[i]<h)b[k++]=a[i];
    } else {
        for(i=n-1;i>=0;i--)if(a[i]<=h)b[k++]=a[i];
        for(i=n-1;i>=0;i--)if(a[i]>h)b[k++]=a[i];
    }
    printf("CLOOK: ");
    display(b,k,h);
}

int main() {
    int a[MAX], n, h, prev, size, i, d;
    printf("Number of requests: ");
    scanf("%d",&n);
    printf("Request queue: ");
    for(i=0;i<n;i++) scanf("%d",&a[i]);
    printf("Head, previous head, disk size: ");
    scanf("%d%d%d",&h,&prev,&size);
    d = h >= prev;

    FCFS(a,n,h);
    SSTF(a,n,h);
    SCAN(a,n,h,size,d);
    CSCAN(a,n,h,size,d);
    LOOK(a,n,h,d);
    CLOOK(a,n,h,d);
    return 0;
}
