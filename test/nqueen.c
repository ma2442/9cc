extern int printf(char* __format, ...);

int ngRaw[100];
int ngCol[100];
int ngDown[200];
int ngUp[200];
char grid[100][100];
int cnt;
int n_max;  // 1辺のマス数最大
int n;      // 1辺のマス数

int put(int r, int c, int b) {
    ngRaw[r] = ngCol[c] = b;
    ngDown[(n - 1) - r + c] = b;
    ngUp[2 * (n - 1) - r - c] = b;
    return 0;
}
_Bool isNg(int r, int c) {
    if (ngRaw[r] || ngCol[c]) return 1;
    if (ngDown[(n - 1) - r + c]) return 1;
    if (ngUp[2 * (n - 1) - r - c]) return 1;
    return 0;
}
int solve(int r, int c) {
    if (isNg(r, c)) return 0;
    if (c == n - 1) {
        grid[r][c] = 'Q';
        cnt = cnt + 1;
        return 0;
    }
    put(r, c, 1);
    for (int i = 0; i < n; i++) {
        if (solve(i, c + 1)) {
            grid[r][c] = 'Q';
            return 1;
        }
    }
    put(r, c, 0);
    return 0;
}
int main() {
    n_max = 11;
    for (n = 1; n < n_max + 1; n++) {
        cnt = 0;
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) {
                grid[i][j] = '.';
            }
        for (int i = 0; i < n; i++)
            if (solve(i, 0)) {
                for (int j = 0; j < n; j++) printf("%.8s\n", grid[j]);
                return 0;
            }
        printf("%d x %d : %d\n", n, n, cnt);
    }
    return 0;
}
