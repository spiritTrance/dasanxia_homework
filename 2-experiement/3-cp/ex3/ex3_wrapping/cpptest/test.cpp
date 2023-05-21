// void putint(int a);
// int fibonacci(int x){
//     if (x == 1)
//         return x;
//     else
//         return x * fibonacci(x - 1);
// }
// int main(){
//     return fibonacci(5);
// }
// int add(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10){
//     return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
// }
// int main(){
//     int a = 1;
//     int b = 3;
//     int c = 1 - b;
//     return c;
// }
void putint(int a);
int b = 9;
int main(){
    int a = 9;
    if (a == 8){
        a = 8;
    }
    else{
        a = 6;
    }
    for (int i = 1; i <= 100;i++){
        for (int j = i; j >= 0;j--){
            int ans = i * j + a / j;
        }
    }
        putint(a);
    return a;
}