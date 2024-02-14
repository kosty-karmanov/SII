#include <bits/stdc++.h>

// #define int int64_t

using namespace std;

void BFS(pair<int, int> from, pair<int, int> to, int n) {
    vector<vector<bool>> used(n, vector<bool>(n));
    vector<vector<pair<int, int>>> p(n, vector<pair<int, int>>(n));
    deque<pair<int, int>> dq;
    dq.emplace_back(from);
    used[from.first][from.second] = true;
    p[from.first][from.second] = {-1, -1};
    vector<int> movex = {-1, 1, -2, -2, 1, -1, -2, -2};
    vector<int> movey = {2, 2, 1, -1, -2, -2, -1, -1};
    while (!dq.empty()) {
        pair<int, int> cur = dq.front();
        int x = cur.first;
        int y = cur.second;
        dq.pop_front();
        if (cur == to) {
            break;
        }
        for (int move = 0; move < 8; ++move) {
            if (x + movex[move] >= 0 && x + movex[move] < n &&
                y + movey[move] >= 0 && y + movey[move] < n) {
                if (!used[x + movex[move]][y + movey[move]]) {
                    used[x + movex[move]][y + movey[move]] = true;
                    p[x + movex[move]][y + movey[move]] = {x, y};
                    dq.emplace_back(x + movex[move], y + movey[move]);
                }
            }
        }
    }
    pair<int, int> cur = to;
    vector<pair<int, int>> path;
    while (cur.first != -1) {
        path.emplace_back(cur);
        cur = p[cur.first][cur.second];
    }
    reverse(path.begin(), path.end());
    cout << path.size() - 1 << "\n";
    for (auto elem : path) {
        cout << n - elem.first << " " << n - elem.second << "\n";
    }
}

int main() {
    int n;
    cin >> n;
    int x1, y1, x2, y2;
    cin >> x1 >> y1 >> x2 >> y2;
    x1 = n - x1;
    y1 = n - y1;
    x2 = n - x2;
    y2 = n - y2;

    BFS({x1, y1}, {x2, y2}, n);
}
