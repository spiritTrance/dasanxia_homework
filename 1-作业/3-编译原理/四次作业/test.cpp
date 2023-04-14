#include<bits/stdc++.h>
using namespace std;
struct  AstNode{
    uint32_t  value;
    AstNode*  parent;        //  the  parent  node
    std::vector<AstNode*>  children;          //  children  of  node
    AstNode(uint32_t val,  AstNode*  p  =  nullptr):  parent(p),  value(val){}  
    virtual  ~AstNode()  {
        for(auto  child:  children)  {
                delete  child;
        }
    }
    AstNode(const  AstNode&)  =  delete;
    AstNode&  operator=(const  AstNode&)  =  delete;
};
int main(){
    unsigned a = 09;
    cout << a << endl;
}