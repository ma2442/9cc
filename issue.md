### To Do

-   [ ] 文字型配列の定義と文字列リテラルによる初期化（char を一つずつスタックに入れる）実装
-   [ ] #include <..>
-   [ ] #ifdef, #endif, #ifndef
-   [ ] static
-   [ ] const
-   [ ] restrict

### Essential To Do 使用中かつ代替不可能

### Want To Do 使用中だが代替可能

-   [ ] 共用体
-   [ ] sizeof(int), sizeof((int\*)[10]) のような sizeof の直接型指定
-   [ ] {} 配列初期化

### Optional To Do 未使用

-   [ ] ,
-   [ ] "abc" "def" => "abcdef" のような文字列リテラルの分割記法の実装
-   [ ] (int\* p)[10] のような入れ子になっている型定義

### Issue

-   [ ] 引数を取る #define 実装
-   [ ] #include ".." の include ディレクトリへの探索
-   [ ] goto に対応する label が存在するかどうか関数内で判定

    -   新 node が ND_FUNC_CALL_ARGS のとき、
        node->type に グローバル変数 funcs より型を設定

-   [ ] find_struct, find_func, find_lvar を線形探索から二分探索に変更
        おそらく structs, funcs, locals を二分木にする必要あり

### Completed

-   [x] find.c のみのセルフコンパイルだと定義前変数への代入エラーが表示されず落ちる
-   [x] find.c のみのセルフコンパイルだと int i=0; のような初期化が動かない(0 が push されない)
-   [x] #define (但し引数を取らない)
-   [x] #include ".." (但しソースと同じフォルダのみ)
-   [x] 文字列リテラルの配置を.rodata に変更して不変値にする
-   [x] extern 変数
-   [x] 関数の引数の型をコール時に確定させる
-   [x] 関数の返り値の型をコール時に確定させる

    -   [x] 外部関数（#include などで extern 宣言を導入）
    -   [x] 内部関数

-   [x] 関数宣言, extern func
-   [x] typedef Type Typename;
        enum や struct の型定義も同時に行える
-   [x] キャスト (int) x
-   [x] 整数リテラル 接尾辞 u l ll lu ul llu ull
-   [x] prefix 0x 0 0b
-   [x] long long, long, short
-   [x] unsigned, signed
-   [x] &= ^= |= <<= >>=
-   [x] & | ^ ~ >> <<
-   [x] void
-   [x] goto label:
-   [x] switch(){case : default:}
-   [x] continue, break
-   [x] 三項演算子 ?:
-   [x] do{ } while( );
-   [x] (不要) size_t : (stddef.h で #define)
-   [x] 文字リテラル 'a' ..
-   [x] エスケープシーケンス '\0', '\n'..
-   [x] 変数,enum,struct スコープをブロックごとに変更
-   [x] enum (Tagname) {Valdef(, Valdef2..)}
        Tagname 及び Val のスコープは変数同様グローバルとローカルがある
        同一スコープで Val と同じ名前の変数があればエラー（逆も然り）
-   [x] (不要) NULL (stddef.h で #define)
-   [x] struct (Tagname) {..}
        Tagname のスコープは変数同様グローバルとローカルがある
    -   [x] struct Tagname {..}
    -   [x] Tagname がない場合は自動割当
-   [x] struct のアラインメントを可変にする（現在は 8 バイト固定、コピーも QWORD ごと）
        メンバのアラインメントの最大値に合わせる。
        8 バイトの例: long long がメンバにる。
        4 バイトの例: アラインメントが 4 の struct がメンバにいて、それが最大。
        1 バイトの例: \_Bool や char、及びそれらからなる struct や array のみがメンバ。
-   [x] 構造体メンバアクセス演算子 .
-   [x] アロー演算子 ->
-   [x] 否定 !
-   [x] && ||
-   [x] \_Bool
-   [x] 剰余演算子 %, %=
-   [x] +=, -=, \*=, /=
-   [x] インクリメント ++, デクリメント --
-   [x] ローカル変数分のスタック確保を 26 個分固定から可変に変更
-   [x] 多次元配列の実装
