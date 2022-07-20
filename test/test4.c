// ローカル変数スタック長可変化テスト
int main_t0() {
    int a[100];
    int i;
    for (i = 0; i < sizeof(a) / sizeof(a[0]); i = i + 1) {
        a[i] = i;
    }
    return a[1];
}
// インクリメント, デクリメント
int main_t1() {
    int a[5];
    int *p = a;
    int i = 0;
    int j = 0;
    if(i++==1)  return 1;
    if(0==++j) return 2; 
    for (i = 0; i < 5; i++) {
        *p++ = i;
        if(a[i]!= i) return 3;
    }
    if(i != 5) return 4;
    p=a;
    for (j = 0; j > -5; j--) {
        *(++p-1) = j;
        if(a[-j]!= j) return 5;
    }
    return (j != -5);
}
int main() {
    if (main_t0() != 1) return 0;
    if (main_t1() != 0) return 1;
    return 255;
}
