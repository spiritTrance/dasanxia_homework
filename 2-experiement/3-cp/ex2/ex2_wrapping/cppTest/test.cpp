#include <iostream>
using namespace std;
int *a = nullptr;
void func(){
  int b = 922;
  a = &b;
}
void func2(){
  double a = 99.112;
}
int main(){
  a = new int(9);
  cout << (*a) << endl;
  func();
  cout << (*a) << endl;
  func2();
  cout << (*a) << endl;
}
