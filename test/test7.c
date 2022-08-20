int main_t1() {
    if (sizeof(int[10]) != 40) return 1;
    if (sizeof(int(*)[10]) != 8) return 2; // ptr to array of int
    return 0;
}

int main(){
    if(main_t1()) return 1;
    return 255;
}

