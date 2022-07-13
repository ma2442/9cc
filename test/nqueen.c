int ngRaw[100];
int ngCol[100];
int ngDown[200];
int ngUp[200];
char grid[100][100];
int cnt;
int n_max;  // 1辺のマス数最大
int n;      // 1辺のマス数

int put(int r, int c, int b) {
    ngRaw[r] = b;
    ngCol[c] = b;
    ngDown[(n - 1) - r + c] = b;
    ngUp[2 * (n - 1) - r - c] = b;
    return 0;
}
int isNg(int r, int c) {
    if (ngRaw[r]) return 1;
    if (ngCol[c]) return 1;
    if (ngDown[(n - 1) - r + c]) return 1;
    if (ngUp[2 * (n - 1) - r - c]) return 1;
    return 0;
}
int solve(int r, int c) {
    int i;
    if (isNg(r, c)) return 0;
    if (c == n - 1) {
        grid[r][c] = 81;  // 'Q'
        cnt = cnt + 1;
        return 0;
    }
    put(r, c, 1);
    for (i = 0; i < n; i = i + 1) {
        if (solve(i, c + 1)) {
            grid[r][c] = 81;  // 'Q'
            return 1;
        }
    }
    put(r, c, 0);
    return 0;
}
int main() {
    n_max = 11;
    int i;
    int j;
    for (n = 1; n < n_max + 1; n = n + 1) {
        cnt = 0;
        for (i = 0; i < n; i = i + 1)
            for (j = 0; j < n; j = j + 1) {
                grid[i][j] = 46;  // '.'
            }
        for (i = 0; i < n; i = i + 1)
            if (solve(i, 0)) {
                for (j = 0; j < n; j = j + 1) printf("%.8s\n", grid[j]);
                return 0;
            }
        printf("%d x %d : %d\n", n, n, cnt);
    }
    return 0;
}
