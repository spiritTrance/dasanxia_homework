#include <bits/stdc++.h>
using namespace std;
struct DFA
{
private:
    ifstream fin;
    int a;

public:
    DFA(){
        
    }
    DFA(string s) :fin(s) {}
    string rm(ifstream& fin){
        return "999";
    }
    void run(){
        rm(fin);
        cout << this->a << endl;
    }
};
int main()
{
        ifstream fin("a.cpp");
        string s;
        while (getline(fin, s))
        {
            cout << s << ' ' << s.length() << endl;
        }
}


