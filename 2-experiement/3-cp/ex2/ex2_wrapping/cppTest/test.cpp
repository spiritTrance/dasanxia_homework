#include <iostream>
#include <fstream>
using namespace std;

int sArray[110];
int n;
void init(int n) {
  int i = 1;
  while (i <= n * n + 1) {
    sArray[i] = -1;
    i = i + 1;
  }
}

int findfa(int a) {
  // cout << sArray[a] << ' ' << a << endl;
  if (sArray[a] == a){
    // cout << 'a'<<a << endl;
    return a;
  }
  else {
    sArray[a] = findfa(sArray[a]);
    // cout <<'a' <<sArray[a] << endl;
    return sArray[a];
  }
}
void mmerge(int a, int b) {
  // cout << a << ' ' << b << endl;
  int m = findfa(a);
  int n = findfa(b);
  if (m != n) sArray[m] = n;
}
int main() {
  int t, m;
  int a, b;
  fstream fin("C:/Users/15781/Desktop/dasanxia_homework/2-experiement/3-cp/ex2/ex2_wrapping/cppTest/62_percolation.in", ios_base::in);
  t = 1;
  while (t) {
    t = t - 1;
    n = 4;
    m = 10;
    int i = 0;
    int flag = 0;
    init(n);
    int k = n * n + 1;
    while (i < m) {
      fin >> a >> b;
      if (!flag) {
        int loc = n * (a - 1) + b;
        cout << loc <<' '<<a<<' '<<b<<' '<<n<< endl;
        cout << sArray[loc + 1] << ' ' << sArray[loc - 1] << ' ' << sArray[loc + n] << ' ' << sArray[loc - n] << endl;
        sArray[loc] = loc;
        if (a == 1) {
          sArray[0] = 0;
          mmerge(loc, 0);
        }
        if (a == n) {
          sArray[k] = k;
          mmerge(loc, k);
        }
        if (b < n && sArray[loc + 1] != -1) {
          mmerge(loc, loc + 1);
        }
        if (b > 1 && sArray[loc - 1] != -1) {
          mmerge(loc, loc - 1);
        }
        if (a < n && sArray[loc + n] != -1) {
          mmerge(loc, loc + n);
        }
        if (a > 1 && sArray[loc - n] != -1) {
          mmerge(loc, loc - n);
        }
        cout << sArray[0] << ' ' << sArray[k] <<' '<<k<<'';
        if (sArray[0] != -1 && sArray[k] != -1){
          cout << findfa(0) << ' ' << findfa(k);
        }
        cout << endl;
        if (sArray[0] != -1 && sArray[k] != -1 && findfa(0) == findfa(k)) {
          cout << "abc" << endl;
          flag = 1;
          int tmp = i + 1;
          cout << tmp << endl;
        }
      }
      i += 1;
    }
    if (!flag) {
      cout << -1 << endl;
    }
  }
  cout << 0 << endl;
  return 0;
}
