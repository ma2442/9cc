#include <stdio.h>
int ngRaw[8];
int ngCol[8];
int ngDown[16];
int ngUp[16];
int rest;
char grid[8][8];
int put(int r, int c, int b) {
    ngRaw[r] = b;
    ngCol[c] = b;
    ngDown[7 - r + c] = b;
    ngUp[7 - r + 7 - c] = b;
    return 0;
}
int isNg(int r, int c) {
    if (ngRaw[r]) return 1;
    if (ngCol[c]) return 1;
    if (ngDown[7 - r + c]) return 1;
    if (ngUp[7 - r + 7 - c]) return 1;
    return 0;
}
int solve(int r, int c) {
    int i;
    int j;
    if (isNg(r, c)) return 0;
    if (rest == 1) {
        grid[r][c] = 81;
        return 1;
    }
    put(r, c, 1);
    rest = rest - 1;
    for (i = 0; i < 8; i = i + 1) {
        for (j = 0; j < 8; j = j + 1) {
            if (solve(i, j)) {
                grid[r][c] = 81;
                return 1;
            }
        }
    }
    put(r, c, 0);
    rest = rest + 1;
    return 0;
}
int main() {
    int i;
    int j;
    int k;
    rest = 8;
    for (i = 0; i < 8; i = i + 1)
        for (j = 0; j < 8; j = j + 1) {
            grid[i][j] = 46;
        }
    for (i = 0; i < 8; i = i + 1)
        for (j = 0; j < 8; j = j + 1)
            if (solve(i, j)) {
                for (k = 0; k < 8; k = k + 1) {
                    printf("%.8s\n", grid[k]);
                }

                return 0;
            }
    return 0;
}
