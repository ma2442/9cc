#include <stdio.h>
#include <stdlib.h>


typedef struct {int valid; int value;} result;
result factor (char*, char*);
char global_err = '\0';

char* findEnd(char* begin){
    while(*begin!='\0'){
        begin++;
    }
    return begin;
}


int exists(char c, char* cset){
    while (*cset != '\0') {
        if (c == *cset) {
            return 1;
        }
        cset++;
    }
    return 0;
}


result nat(char* begin, char* end){
    //printf("nat : %s, %lld\n", begin, (long long int)end - (long long int)begin);
    char* rest;

    int sign;
    int abs = 0;
    char nums[] = "0123456789";
    result res = {0, 0};
    if(begin[0] == '+'){
        rest = &(begin[1]);
        sign = +1;
    } 
    else if(begin[0] == '-'){
        rest = &(begin[1]);
        sign = -1;
    } else {
        rest = &(begin[0]);
        sign = +1;
    }
    
    while(rest != end) {
        res.valid = 0;

        for(int k=0; nums[k] != '\0'; k++) {
            if(*rest == nums[k]) {
                res.valid = 1;
                abs = abs*10 + k;
            }
        }

        if(!res.valid) {
            res.value = 0;
            global_err = *rest;
            return res;
        }
        rest++;
    }

    res.value = sign * abs;
    return res;
}


result term(char* begin, char* end){
    //printf("term : %s, %lld\n", begin, (long long int)end - (long long int)begin);

    result  res = {0, 0};
    char* rest = begin;

    while (rest != end) {
        if( *rest == '*' || *rest == '/') {

            result res1 = term(begin, rest);

            if (!res1.valid) {
                rest++;
                continue;
            }

            result res2 = factor(rest+1, end);

            if (!res2.valid) {
                rest++;
                continue;
            }

            res.valid = 1;

            if( *rest == '*' ) {
                res.value = res1.value * res2.value;
            }
            else if( *rest == '/') {
                res.value = res1.value / res2.value;
            }
            return res;
        }
        rest++;

    }

    return factor(begin, end);
}

    

result expr(char* begin, char* end){
    //printf("expr : %s, %lld\n", begin, (long long int)end - (long long int)begin);
    result res = {0, 0};
    char* rest = begin + 1;

    while (rest != end) {
        if(*rest == '+' || *rest == '-') {

            result res1 = expr(begin, rest);

            if (!res1.valid) {
                rest++;
                continue;
            }

            result res2 = term(rest+1, end);

            if (!res2.valid) {
                rest++;
                continue;
            }

            res.valid = 1;

            printf("  mov rax, %d\n", res1.value);
            printf("  mov rsi, %d\n", res2.value);

            if( *rest == '+' ) {
                res.value = res1.value + res2.value;
                printf("  add rax, rsi\n");
            }
            else if( *rest == '-') {
                res.value = res1.value - res2.value;
                printf("  sub rax, rsi\n");
            }
            return res;
        }
        rest++;
    }

    return term(begin, end);
}



result factor(char* begin, char* end){
    // printf("factor : %s, %lld\n", begin, (long long int)end - (long long int)begin);
    result res = {0, 0};

    if ( *begin == '(' && *(end-1) == ')' ){
        res = expr(begin+1, end-1);
    }
    else {
        res = nat(begin, end);
    }
    return res;
}


result calc(char* begin){
    char* rest = begin;
    char expr_without_space[100];
    int i=0;
    while(*rest !='\0') {
        if(*rest != ' ') {
            if ( !exists( *rest, "()*/+-0123456789" ) ) {
                global_err = *rest;
                result res = {0,0};
                return res;
            }
            expr_without_space[i] = *rest;
            //putchar(expr_without_space[i]);
            i++;
        }
        rest++;
    }
    expr_without_space[i] = '\0';
    return expr(expr_without_space, &(expr_without_space[i]));

}



int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");
    printf("  mov rax, 0\n");

    result res = calc(argv[1]);
    if ( !res.valid ) {
        fprintf(stderr, "予期しない文字です: '%c' \n", global_err);
        return 1;
    }
    fprintf(stderr, "calc: %d\n", res.value);
    //printf("  mov rax, rax\n", atoi(argv[1]));
    printf("  ret\n");
    return 0;
}

