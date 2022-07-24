### To Do

- [ ] struct
- [ ] 構造体メンバアクセス演算子 .
- [ ] アロー演算子 ->

### Essential To Do 使用中かつ代替不可能

- [ ] 関数宣言
- [ ] extern
- [ ] #include <..>, #include ".."
- [ ] #define
- [ ] #ifdef, #endif, #ifndef

### Want To Do 使用中だが代替可能

- [ ] do{ } while( );
- [ ] sizeof(int), sizeof((int\*)[10]) のような sizeof の直接型指定
- [ ] {} 配列初期化
- [ ] 文字リテラル 'a' ..
- [ ] エスケープシーケンス '\0', '\n'..
- [ ] typedef
- [ ] continue, break
- [ ] void
- [ ] size_t
- [ ] NULL
- [ ] 三項演算子 ?:
- [ ] スタティックキャスト (int) x

### Optional To Do 未使用

- [ ] ,
- [ ] & | ^ ~ >> <<
- [ ] &= ^= |= <<= >>=
- [ ] static
- [ ] "abc" "def" => "abcdef" のような文字列リテラルの分割記法の実装
- [ ] (int\* p)[10] のような入れ子になっている型定義
- [ ] 共用体

### Issue

- [ ] 文字型配列の定義と文字列リテラルによる初期化（char を一つずつスタックに入れる）実装
- [ ] 関数の引数の型をコール時に確定させる

  - 新 node が ND_FUNC_CALL_ARGS のとき、
    node->type に グローバル変数 funcs より型を設定

- [ ] 関数の返り値の型をコール時に確定させる

  - [ ] 外部関数（やり方不明）
  - [x] 内部関数

- [ ] find_struct, find_func, find_lvar を線形探索から二分探索に変更
      おそらく structs, funcs, locals を二分木にする必要あり
- [ ] struct のアラインメントを可変にする（現在は 8 バイト固定、コピーも QWORD ごと）
      メンバのアラインメントの最大値に合わせる。
      8 バイトの例: long long がメンバにる。
      4 バイトの例: アラインメントが 4 の struct がメンバにいて、それが最大。
      1 バイトの例: \_Bool や char、及びそれらからなる struct や array のみがメンバ。

### Completed

- [x] 否定 !
- [x] && ||
- [x] \_Bool
- [x] 剰余演算子 %, %=
- [x] +=, -=, \*=, /=
- [x] インクリメント ++, デクリメント --
- [x] ローカル変数分のスタック確保を 26 個分固定から可変に変更
- [x] 多次元配列の実装
