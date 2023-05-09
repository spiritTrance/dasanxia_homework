#include<iostream>
using namespace std;
struct aa
{
    int a, b;
};

int main(){
    aa a;
    a.a = 3;
    a.b = 4;
    aa b;
    a.a = 999;
    b = a;
    cout << b.a << b.b << endl;
}