int arr[2];
void putch(int c);
void putint(int c);
void global(){
  arr[0] = 1;
  arr[1] = 5;
}
int main(){
    putint(arr[0]);
    putch(10);
    putint(arr[1]);
    putch(10);
    return 0;
}