#ifndef WINDOW_H
#define WINDOW_H

#define MAX 50
#define MIN(a, b) ((a) < (b) ? (a) : (b))

void stopAndWait(int frames, int lostFrame);
void goBackN(int frames, int N, int lostFrame);
void selectiveRepeat(int frames, int N, int lostFrame);

#endif
