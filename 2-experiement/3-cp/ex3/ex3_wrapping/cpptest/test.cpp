#include <iostream>
using namespace std;
#define endl '\n'
int array[110];
int n;
void init(int n) {
  int i = 1;
  while (i <= n * n + 1) {
    array[i] = -1;
    i = i + 1;
  }
}

int findfa(int a) {
  if (array[a] == a)
    return a;
  else {
    array[a] = findfa(array[a]);
    return array[a];
  }
}
void mmerge(int a, int b) {
  int m = findfa(a);
  int n = findfa(b);
  if (m != n) array[m] = n;
}
int main() {
  int t, m;
  int a, b;
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
      cin >> a >> b;

      if (!flag) {
        int loc = n * (a - 1) + b;

        array[loc] = loc;
        if (a == 1) {
          array[0] = 0;
          mmerge(loc, 0);
        }
        if (a == n) {
          array[k] = k;
          mmerge(loc, k);
        }
        if (b < n && array[loc + 1] != -1) {
          mmerge(loc, loc + 1);
        }
        if (b > 1 && array[loc - 1] != -1) {
          mmerge(loc, loc - 1);
        }
        if (a < n && array[loc + n] != -1) {
          mmerge(loc, loc + n);
        }
        if (a > 1 && array[loc - n] != -1) {
          mmerge(loc, loc - n);
        }
        cout << array[0] << 'a'<<array[k] <<'a'<<k<< endl;
        if (array[0] != -1 && array[k] != -1 && findfa(0) == findfa(k))
        {
          flag = 1;
          int tmp = i + 1;
          cout << tmp << endl;
        }
      }

      i = i + 1;
    }
    if (!flag) {
      cout << -1 << endl;
    }
  }
  return 0;
}
