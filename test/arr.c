int a[2][3];

int main() {
    int i;  // 宣言
    int j;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1)
        for (j = 0; j < sizeof(a[0]) / sizeof(a[0][0]); j = j + 1) {
            a[i][j] = 10 * i + j + 1;
        }
    if (a[0][0] == 1)
        if (a[0][2] == 3)
            if (a[1][0] == 11)
                if (a[1][2] == 13) return 1; /* おかしな 場合の処理 */
    return 0;
}
