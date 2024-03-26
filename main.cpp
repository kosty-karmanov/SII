#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <windows.h>
#include <utility>

using namespace std;
using namespace sf;

struct node {
    int x, y, tm;
};

struct path_point {
    int x, y, type, id;
};

struct order {
    bool type, taken;
    int cargo_id, punkt_id, shelf_id;
};

struct button {
    string text;
    int x, y, width, height, action, font_size;

    button(string s1, int x1, int y1, int width1, int height1, int action1,
           int size) {
        text = s1;
        x = x1;
        y = y1;
        width = width1;
        height = height1;
        action = action1;
        font_size = size;
    }
};

int n = 40, m = 40, seconds_ = 0, ticks_ = 0, N = 1e3;
vector<vector<vector<int>>> banned_time(
    n, vector<vector<int>>(m, vector<int>(N, -1)));
bool simulation_working = false;
deque<order> orders;
vector<button> buttons;


vector<pair<int, int>> bfs(vector<vector<int>>& a, int x1, int y1, int x2,
                           int y2, int n, int m, int time_start, int id) {
    vector<vector<vector<bool>>> dp(
        n, vector<vector<bool>>(m, vector<bool>(N, false)));
    vector<vector<vector<pair<int, int>>>> p(
        n, vector<vector<pair<int, int>>>(m, vector<pair<int, int>>(N)));
    deque<node> d;
    dp[x1][y1][time_start] = true;
    vector<pair<int, int>> move = {{0, 0},
                                   {-1, 0},
                                   {1, 0},
                                   {0, -1},
                                   {0, 1},
    };
    bool found = false;
    d.push_back({x1, y1, time_start});
    while (d.size()) {
        int x = d.front().x;
        int y = d.front().y;
        int time_now = d.front().tm;
        if (time_now == N - 1)
            break;
        d.pop_front();
        if (x == x2 and y == y2)
            found = true;
        for (auto de : move) {
            int xn = x + de.first, yn = y + de.second;
            if (xn > -1 and xn < n and yn > -1 and yn < m) {
                if ((a[xn][yn] == 3 or a[xn][yn] == 5 or a[xn][yn] == 2 or a[xn]
                     [yn] == 0) and (
                        xn != x2 or yn != y2))
                    continue;
                if (banned_time[xn][yn][time_now + 1] != -1)
                    continue;
                if (dp[xn][yn][time_now + 1])
                    continue;
                if (banned_time[xn][yn][time_now] == banned_time[x][y][
                        time_now + 1] and
                    banned_time[x][y][time_now + 1] != -1)
                    continue;

                dp[xn][yn][time_now + 1] = true;
                d.push_back({xn, yn, time_now + 1});
                p[xn][yn][time_now + 1] = {x, y};
            }
        }
    }
    if (!found) {
        return {};
    }
    int time_st = 1e9;
    for (int st = time_start; st < N; st++) {
        if (dp[x2][y2][st]) {
            time_st = st;
            break;
        }
    }
    vector<pair<int, int>> put;
    while (time_st != time_start) {
        put.push_back({x2, y2});
        int x3 = p[x2][y2][time_st].first;
        y2 = p[x2][y2][time_st].second;
        x2 = x3;
        time_st--;
    }
    put.emplace_back(x2, y2);
    reverse(put.begin(), put.end());
    return put;
}

pair<int, int> FindNearest(vector<vector<int>>& a, int x1, int y1, int n, int m,
                           int time_start, int type, int id) {
    vector<vector<vector<bool>>> dp(
        n, vector<vector<bool>>(m, vector<bool>(N, false)));
    deque<node> d;
    dp[x1][y1][time_start] = true;
    vector<pair<int, int>> move = {{-1, 0},
                                   {1, 0},
                                   {0, -1},
                                   {0, 1},
                                   {0, 0}};
    d.push_back({x1, y1, time_start});
    while (d.size()) {
        int x = d.front().x;
        int y = d.front().y;
        int time_now = d.front().tm;
        if (time_now == N - 1)
            break;
        d.pop_front();
        if (a[x][y] == type)
            return {x, y};
        for (auto de : move) {
            int xn = x + de.first, yn = y + de.second;
            if (xn > -1 and xn < n and yn > -1 and yn < m) {
                if ((a[xn][yn] == 3 or a[xn][yn] == 5 or a[xn][yn] == 2) and a[
                        xn][yn] != type)
                    continue;
                if (banned_time[xn][yn][time_now + 1] != -1)
                    continue;
                if (dp[xn][yn][time_now + 1])
                    continue;
                if (a[xn][yn] == 0)
                    continue;
                if (banned_time[xn][yn][time_now] == banned_time[x][y][
                        time_now + 1] and (
                        banned_time[x][y][time_now + 1] != -1))
                    continue;

                dp[xn][yn][time_now + 1] = true;
                d.push_back({xn, yn, time_now + 1});
            }
        }
    }
    return {-1, -1};
}


unordered_map<string, Color> colors = {
    {"none", Color(49, 46, 43)},
    {"road", Color(255, 255, 255)},
    {"dock", Color(87, 91, 253)},
    {"shelf", Color(235, 146, 3)},
    {"selected", Color(0, 255, 0, 150)},
    {"pickup", Color(171, 39, 74)},
};

vector<Color> robot_colors = {Color(219, 68, 64), Color(248, 222, 84),
                              Color(194, 60, 169), Color(16, 216, 227),
                              Color(79, 212, 71), Color(135, 204, 46),
                              Color(232, 4, 127), Color(60, 15, 240)};

int GetEpochTime() {
    const auto p1 = chrono::system_clock::now();
    return chrono::duration_cast<chrono::seconds>(
        p1.time_since_epoch()).count();
}

class Cargo {
private:
    int id_ = 0, shelf_id_ = 0, robot_id_ = 0, pickup_id_ = 0;
    bool on_shelf_ = false, priority_ = false, on_robot_ = false, on_pickup_ =
             false, to_remove_ = false;
    string name_;

public:
    Cargo() = default;

    void create(int id, string name, bool priority) {
        id_ = id;
        name_ = name;
        priority_ = priority;
        on_shelf_ = false;
        shelf_id_ = 0;
    }

    int getId() const {
        return id_;
    }

    bool onShelf() const {
        return on_shelf_;
    }

    bool onRobot() const {
        return on_robot_;
    }

    bool onPcikUp() const {
        return on_pickup_;
    }

    int getShelfId() const {
        return shelf_id_;
    }

    int getRobotId() const {
        return robot_id_;
    }

    int getPickUpId() const {
        return pickup_id_;
    }

    int toRemove() const {
        return to_remove_;
    }

    void putOnPickUp(int id) {
        on_shelf_ = false;
        on_robot_ = false;
        on_pickup_ = true;
        pickup_id_ = id;
    }

    void putOnShelf(int id) {
        on_shelf_ = true;
        on_robot_ = false;
        on_pickup_ = false;
        shelf_id_ = id;
    }

    void putOnRobot(int id) {
        on_shelf_ = false;
        on_robot_ = true;
        on_pickup_ = false;
        robot_id_ = id;
    }

    string getName() const {
        return name_;
    }
};

unordered_map<int, Cargo> cargos;

class Shelf {
private:
    int size_ = 0, free_space_ = 0, x_ = 0, y_ = 0, id_ = 0;
    vector<int> shelf_;

public:
    Shelf() = default;

    void create(int id, int size, int x, int y) {
        id_ = id;
        x_ = x;
        y_ = y;
        size_ = size;
        shelf_.resize(size);
        free_space_ = size;
        shelf_.clear();
        shelf_.resize(size, -1);
    }

    int getSize() const {
        return size_;
    }

    int getFrSpace() const {
        return free_space_;
    }

    int getId() const {
        return id_;
    }

    pair<int, int> getPos() {
        return {x_, y_};
    }

    vector<int> getCargos() {
        return shelf_;
    }

    void changeFreeSize(int d) {
        free_space_ += d;
    }

    int pushCargo(int id) {
        for (int i = 0; i < size_; ++i) {
            if (shelf_[i] == -1) {
                cargos[id].putOnShelf(id_);
                shelf_[i] = id;
                return i;
            }
        }
        return -1;
    }

    Cargo popCargo(int i) {
        Cargo temp = cargos[shelf_[i]];
        shelf_[i] = -1;
        return temp;
    }
};

class PickUp {
private:
    int id_ = 0, x_ = 0, y_ = 0;
    set<int> cargos_;

public:
    PickUp() = default;

    void create(int id, int x, int y) {
        cargos_.clear();
        id_ = id;
        x_ = x;
        y_ = y;
    }

    void addCargo(int id) {
        cargos_.insert(id);
    }

    void popCargo(int id) {
        cargos_.erase(id);
    }

    int getId() const {
        return id_;
    }

    set<int> getCargos() {
        return cargos_;
    }

    pair<int, int> getPos() {
        return {x_, y_};
    }
};

unordered_map<int, PickUp> pickups;

unordered_map<int, Shelf> shelves;

class Robot {
private:
    int x_{}, y_{}, id_ = 0, dock_id_ = 0, cur_path_ = 0, cargo_id_ = -1,
        order_id_ = 0, task_ = -1;
    bool busy_ = false;
    vector<path_point> path_;

public:
    Robot() = default;

    void create(int x, int y, int id, int dock_id) {
        x_ = x;
        y_ = y;
        id_ = id;
        dock_id_ = dock_id;
        busy_ = false;
        path_.clear();
        cur_path_ = 0;
        cargo_id_ = -1;
    }

    void takeCargo(int i) {
        cargo_id_ = i;
    }

    pair<int, int> getPos() {
        return {x_, y_};
    }

    int getId() const {
        return id_;
    }

    int getCurPath() const {
        return cur_path_;
    }

    void setPath(vector<path_point> path) {
        path_ = path;
        busy_ = true;
        cur_path_ = 0;
    }

    int getCargoId() const {
        return cargo_id_;
    }

    int getDockId() const {
        return dock_id_;
    }

    int popCargo() {
        int temp = cargo_id_;
        cargo_id_ = -1;
        return temp;
    }

    bool getStatus() const {
        return busy_;
    }

    vector<path_point> getPath() {
        return path_;
    }

    int getOrderId() const {
        return order_id_;
    }

    void setOrderId(int id) {
        order_id_ = id;
    }

    int getTask() const {
        return task_;
    }

    void setTask(int val) {
        task_ = val;
    }

    void iteration() {
        if (busy_ && !path_.empty()) {
            if (cur_path_ >= path_.size()) {
                busy_ = false;
                return;
            }
            if (task_ == 1 && path_[cur_path_].type == 4) {
                pickups[path_[cur_path_].id].popCargo(
                    orders[order_id_].cargo_id);
                cargo_id_ = orders[order_id_].cargo_id;
                task_ = 2;
            }
            if (task_ == 2 && path_[cur_path_].type == 3) {
                shelves[path_[cur_path_].id].pushCargo(cargo_id_);
                cargo_id_ = -1;
                task_ = -1;
                busy_ = false;
            }
            if (task_ == 3 && (path_[cur_path_].type == 3 || path_[cur_path_].
                               type == 5)) {
                shelves[path_[cur_path_].id].popCargo(
                    orders[order_id_].cargo_id);
                shelves[path_[cur_path_].id].changeFreeSize(1);
                cargo_id_ = orders[order_id_].cargo_id;
                task_ = 4;
            }
            if (task_ == 4 && path_[cur_path_].type == 4) {
                pickups[path_[cur_path_].id].addCargo(cargo_id_);
                cargo_id_ = -1;
                task_ = -1;
                busy_ = false;
            }
            x_ = path_[cur_path_].x;
            y_ = path_[cur_path_].y;
            cur_path_++;
        }
    }
};

unordered_map<int, Robot> robots;

class Dock {
private:
    int size_ = 0, cur_robots_ = 0, id_ = 0, x_{}, y_{};
    vector<int> robots_id;

public:
    Dock() = default;

    void create(int x, int y, int size, int id) {
        x_ = x;
        y_ = y;
        size_ = size;
        cur_robots_ = size_;
        id_ = id;
        robots_id.clear();
        int i = 0;
        while (robots_id.size() != size_) {
            while (robots.find(i) != robots.end()) {
                i++;
            }
            robots[i].create(x_, y_, i, id_);
            robots_id.push_back(i);
        }
    }

    int getFreeRobots() const {
        return cur_robots_;
    }

    vector<int> getRobotsId() {
        return robots_id;
    }

    int getId() const {
        return id_;
    }

    int getSize() const {
        return size_;
    }

    pair<int, int> getPos() {
        return {x_, y_};
    }

    void deleteRobots() const {
        for (int id : robots_id) {
            robots.erase(id);
        }
    }
};

unordered_map<int, Dock> docks;

class Point {
private:
    bool is_road_ = false, is_shelf_ = false, is_dock_ = false, is_pickup_ =
             false, is_none_ = true;
    int shelf_id_ = 0, dock_id_ = 0, pickup_id = 0, x_ = 0, y_ = 0;

public:
    Point() = default;

    void becomeRoad() {
        is_none_ = false;
        if (is_shelf_) {
            shelves.erase(shelf_id_);
            is_shelf_ = false;
        }
        if (is_dock_) {
            is_dock_ = false;
            docks[dock_id_].deleteRobots();
            docks.erase(dock_id_);
            dock_id_ = 0;
        }
        if (is_pickup_) {
            is_pickup_ = false;
            pickups.erase(pickup_id);
            pickup_id = 0;
        }
        is_road_ = true;
    }

    void becomeShelf(int size) {
        is_none_ = false;
        is_road_ = false;
        if (is_dock_) {
            is_dock_ = false;
            docks[dock_id_].deleteRobots();
            docks.erase(dock_id_);
            dock_id_ = 0;
        }
        if (is_pickup_) {
            is_pickup_ = false;
            pickups.erase(pickup_id);
            pickup_id = 0;
        }
        int i = 0;
        while (shelves.find(i) != shelves.end()) {
            i++;
        }
        is_shelf_ = true;
        shelf_id_ = i;
        shelves[i] = Shelf();
        shelves[i].create(i, size, x_, y_);
    }

    void becomeNone() {
        is_road_ = false;
        if (is_dock_) {
            is_dock_ = false;
            docks[dock_id_].deleteRobots();
            docks.erase(dock_id_);
            dock_id_ = 0;
        }
        if (is_shelf_) {
            is_shelf_ = false;
            shelves.erase(shelf_id_);
            shelf_id_ = 0;
        }
        if (is_pickup_) {
            is_pickup_ = false;
            pickups.erase(pickup_id);
            pickup_id = 0;
        }
        is_none_ = true;
    }

    void becomeDock(int size) {
        is_none_ = false;
        is_road_ = false;
        is_dock_ = true;
        if (is_shelf_) {
            is_shelf_ = false;
            shelves.erase(shelf_id_);
            shelf_id_ = 0;
        }
        if (is_pickup_) {
            is_pickup_ = false;
            pickups.erase(pickup_id);
            pickup_id = 0;
        }
        int i = 0;
        while (docks.find(i) != docks.end()) {
            i++;
        }
        dock_id_ = i;
        docks[i] = Dock();
        docks[i].create(x_, y_, size, i);
    }

    void becomePickUp() {
        is_road_ = false;
        is_none_ = false;
        if (is_dock_) {
            is_dock_ = false;
            docks[dock_id_].deleteRobots();
            docks.erase(dock_id_);
            dock_id_ = 0;
        }
        if (is_shelf_) {
            is_shelf_ = false;
            shelves.erase(shelf_id_);
            shelf_id_ = 0;
        }
        is_pickup_ = true;
        int i = 0;
        while (pickups.find(i) != pickups.end()) {
            i++;
        }
        pickup_id = i;
        pickups[i] = PickUp();
        pickups[i].create(i, x_, y_);
    }

    void create(int x, int y) {
        x_ = x;
        y_ = y;
    }

    bool isRoad() const {
        return is_road_;
    }

    bool isShelf() const {
        return is_shelf_;
    }

    bool isNone() const {
        return is_none_;
    }

    bool isDock() const {
        return is_dock_;
    }

    bool isPickUp() const {
        return is_pickup_;
    }

    int getPickUpId() const {
        return pickup_id;
    }

    int getDockId() const {
        return dock_id_;
    }

    int getShelfId() const {
        return shelf_id_;
    }
};

// -----------------------------------------------------------------------------

RenderWindow window(VideoMode(2560, 1440), "Simulation");
RenderWindow help_window(VideoMode(640, 480), "Info");
vector<vector<Point>> field;
int square_size, current_illumination = -1;
bool is_locked_buttons = false, keyboard_locked = false, is_terminal_selected =
         false, term_status = true;
pair<int, int> selected_point = {-1, -1};
string terminal;
deque<int> buttons_bufer;
deque<pair<string, Color>> terminal_messages;
vector<vector<int>> graph;
int cur_my_msg = -1;
deque<string> my_msg;
map<string, Texture> sprites;

// -----------------------------------------------------------------------------

void CreateButtons() {
    buttons = {
        button("Load", n * square_size + 300, 50, 80, 50, 1, 30),
        button("Save", n * square_size + 700, 50, 80, 50, 2, 30),
    };
}

void LoadField() {
    ifstream input_file("C:/Users/kosty/CLionProjects/untitled1/field.txt");
    if (input_file.is_open()) {
        int n1, m1;
        input_file >> n1 >> m1;
        if (n1 != n || m1 != m) {
            cout << "Bad field size: " << n1 << " " << m1 << ". Current: " << n
                << " " << m << '\n';
            return;
        }
        banned_time.clear();
        banned_time.resize(n, vector<vector<int>>(m, vector<int>(N, -1)));
        orders.clear();
        simulation_working = false;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                field[i][j].becomeNone();
            }
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                int type;
                input_file >> type;
                if (type == 1) {
                    field[i][j].becomeRoad();
                } else if (type == 2) {
                    field[i][j].becomeDock(1);
                } else if (type == 3) {
                    field[i][j].becomeShelf(1);
                } else if (type == 4) {
                    field[i][j].becomePickUp();
                }
            }
        }
    }
}

void SaveField() {
    ofstream output_file("C:/Users/kosty/CLionProjects/untitled1/field.txt");
    if (output_file.is_open()) {
        output_file << n << " " << m << "\n";
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (field[i][j].isRoad()) {
                    output_file << "1 ";
                } else if (field[i][j].isDock()) {
                    output_file << "2 ";
                } else if (field[i][j].isShelf()) {
                    output_file << "3 ";
                } else if (field[i][j].isPickUp()) {
                    output_file << "4 ";
                } else {
                    output_file << "0 ";
                }
            }
            output_file << "\n";
        }
    }
}

int Pos2Num(int x, int y) {
    return x * m + y;
}

pair<int, int> Num2Pos(int num) {
    return {num / m, num % m};
}

vector<vector<int>> RebuildMatrix() {
    vector<vector<int>> ans(n, vector<int>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            if (field[i][j].isRoad()) {
                ans[i][j] = 1;
            } else if (field[i][j].isDock()) {
                ans[i][j] = 2;
            } else if (field[i][j].isShelf()) {
                if (shelves[field[i][j].
                        getShelfId()].getFrSpace() > 0)
                    ans[i][j] = 3;
                else
                    ans[i][j] = 5;
            } else if (field[i][j].isPickUp()) {
                ans[i][j] = 4;
            }
        }
    }
    return ans;
}

vector<path_point> GetFullPath(int robot_id, int pickup_id, int shelf_id) {
    vector<path_point> ans;
    vector<vector<int>> a = RebuildMatrix();
    Robot r = robots[robot_id];
    pair<int, int> robot_pos = r.getPos();
    if (shelf_id == -1) {
        PickUp p = pickups[pickup_id];
        pair<int, int> pickup_pos = p.getPos();
        vector<pair<int, int>> path = bfs(a, robot_pos.first, robot_pos.second,
                                          pickup_pos.first, pickup_pos.second,
                                          n, m, ticks_, robot_id + 1);
        if (path.empty()) {
            return {};
        }
        for (auto elem : path) {
            ans.push_back({elem.first, elem.second,
                           -1});
        }
        ans[ans.size() - 1].id = pickup_id;
        ans[ans.size() - 1].type = 4;
        pair<int, int> pos = FindNearest(a, pickup_pos.first, pickup_pos.second,
                                         n, m, ticks_ + ans.size(), 3,
                                         robot_id + 1);
        if (pos.first == -1) {
            return {};
        }
        path = bfs(a, pickup_pos.first, pickup_pos.second, pos.first,
                   pos.second, n, m, ticks_ + ans.size(), robot_id + 1);
        for (auto elem2 : path) {
            ans.push_back({elem2.first, elem2.second,
                           -1});
        }
        ans[ans.size() - 1].id = field[pos.first][pos.second].getShelfId();
        ans[ans.size() - 1].type = 3;
    } else {
        Shelf s = shelves[shelf_id];
        pair<int, int> shelf_pos = s.getPos();
        vector<pair<int, int>> path = bfs(a, robot_pos.first, robot_pos.second,
                                          shelf_pos.first, shelf_pos.second, n,
                                          m, ticks_, robot_id + 1);
        if (path.empty()) {
            return {};
        }
        for (auto elem : path) {
            ans.push_back({elem.first, elem.second,
                           -1});
        }
        ans[ans.size() - 1].id = shelf_id;
        ans[ans.size() - 1].type = 3;
        PickUp p = pickups[pickup_id];
        pair<int, int> pickup_pos = p.getPos();
        path = bfs(a, shelf_pos.first, shelf_pos.second, pickup_pos.first,
                   pickup_pos.second, n, m, ticks_ + ans.size(), robot_id + 1);
        if (path.empty()) {
            return {};
        }
        for (auto elem2 : path) {
            ans.push_back({elem2.first, elem2.second,
                           -1});
        }
        ans[ans.size() - 1].id = pickup_id;
        ans[ans.size() - 1].type = 4;
    }
    return ans;
}

vector<int> Get4Near(int x, int y) {
    vector<int> ans;
    if (y - 1 >= 0 && field[x][y - 1].isRoad()) {
        ans.push_back(Pos2Num(x, y - 1));
    }
    if (y + 1 < m && field[x][y + 1].isRoad()) {
        ans.push_back(Pos2Num(x, y + 1));
    }
    if (x - 1 >= 0 && field[x - 1][y].isRoad()) {
        ans.push_back(Pos2Num(x - 1, y));
    }
    if (x + 1 < n && field[x + 1][y].isRoad()) {
        ans.push_back(Pos2Num(x + 1, y));
    }
    return ans;
}

int GetNearestRobotV3(bool type, int pickup_id, int shelf_id) {
    int mn = 1e9;
    int id = -1;
    for (auto r : robots) {
        if (r.second.getStatus() && r.second.getTask() != -1) {
            continue;
        }
        int sz = GetFullPath(r.second.getId(), pickup_id, shelf_id).size();
        if (sz < mn && sz != 0) {
            mn = sz;
            id = r.first;
        }
    }
    return id;
}

void RobotsIteration() {
    for (auto& r : robots) {
        r.second.iteration();
    }
    int cnt = 0;
    for (order& order_ : orders) {
        if (order_.taken) {
            cnt++;
            continue;
        }
        int id = GetNearestRobotV3(order_.type, order_.punkt_id,
                                   order_.shelf_id);
        if (id != -1) {
            order_.taken = true;
            vector<path_point> path =
                GetFullPath(id, order_.punkt_id, order_.shelf_id);
            robots[id].setPath(path);
            robots[id].setOrderId(cnt);
            if (order_.type) {
                robots[id].setTask(1);
                int shelf_id = path[path.size() - 1].id;
                shelves[shelf_id].changeFreeSize(-1);
                int cur_ticks = ticks_;
                for (auto i : path) {
                    banned_time[i.x][i.y][cur_ticks++] = id + 1;
                    // if (i.type == 4 or i.type == 3) {
                    //     banned_time[i.x][i.y][cur_ticks++] = id + 1;
                    // }
                }
            } else {
                robots[id].setTask(3);
                int cur_ticks = ticks_;
                for (auto i : path) {
                    banned_time[i.x][i.y][cur_ticks++] = id + 1;
                    // if (i.type == 4 or i.type == 3) {
                    //     banned_time[i.x][i.y][cur_ticks++] = id + 1;
                    // }
                }
            }
        }
        cnt++;
    }
    vector<vector<int>> a = RebuildMatrix();
    for (auto& r : robots) {
        if (!r.second.getStatus() || r.second.getTask() == 228) {
            pair<int, int> pos = r.second.getPos();
            pair<int, int> dpos = docks[r.second.getDockId()].getPos();
            if (pos.first == dpos.first && pos.second == dpos.second) {
                continue;
            }
            vector<pair<int, int>> path = bfs(a, pos.first, pos.second,
                                              dpos.first, dpos.second, n, m,
                                              ticks_, r.first + 1);
            if (path.empty()) {
                r.second.setPath({});
                r.second.setTask(228);
                banned_time[pos.first][pos.second][ticks_] = r.first + 1;
                continue;
            }
            r.second.setTask(-1);
            int cur_ticks = ticks_;
            vector<path_point> ans;
            for (auto elem : path) {
                ans.push_back({elem.first, elem.second,
                               a[elem.first][elem.second]});
                ans[ans.size() - 1].type = -1;
                banned_time[elem.first][elem.second][cur_ticks++] = r.first + 1;
            }
            r.second.setPath(ans);
        }
    }
}

bool CanPressButton(int code) {
    int cnt = 0;
    for (int elem : buttons_bufer) {
        if (elem == buttons_bufer[0]) {
            cnt++;
        }
    }
    if (buttons_bufer[1] == buttons_bufer[0] && cnt != 200) {
        return false;
    }
    return true;
}

void Type(RenderWindow& window2, float x, float y, String s,
          Color col = Color::White, int size = 24) {
    Text text;
    Font font;
    font.loadFromFile("../arial.ttf");
    text.setFont(font);
    text.setString(s);
    text.setPosition(x - text.getLocalBounds().width / 2,
                     y - text.getLocalBounds().height / 2);
    text.setFillColor(col);
    text.setCharacterSize(size);
    window2.draw(text);
}

void Type2(RenderWindow& window, int x, int y, const String& s,
           Color col = Color::White, int size = 24) {
    Text text;
    Font font;
    font.loadFromFile("../arial.ttf");
    text.setFont(font);
    text.setString(s);
    text.setPosition((float)x, (float)y);
    text.setFillColor(col);
    text.setCharacterSize(size);
    window.draw(text);
}

pair<int, int> GetTextSize(string s, int size = 24) {
    Text text;
    Font font;
    font.loadFromFile("../arial.ttf");
    text.setFont(font);
    text.setString(s);
    text.setCharacterSize(size);
    return {text.getLocalBounds().height, text.getLocalBounds().width};
}

void DrawRect(RenderWindow& window, float x, float y, float h, Color col) {
    CircleShape circle(h / 2);
    circle.setFillColor(col);
    circle.setPosition(x, y);
    window.draw(circle);
    circle.setPosition(x + h, y);
    window.draw(circle);
    circle.setPosition(x, y + h);
    window.draw(circle);
    circle.setPosition(x + h, y + h);
    window.draw(circle);
    RectangleShape rectangle(Vector2f(h + h, h));
    rectangle.setPosition(x, y + h / 2);
    rectangle.setFillColor(col);
    window.draw(rectangle);
    RectangleShape rectangle2(Vector2f(h, h + h));
    rectangle2.setPosition(x + h / 2, y);
    rectangle2.setFillColor(col);
    window.draw(rectangle2);
}

void LoadSprites() {
    Texture texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/1.png");
    sprites["1100"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/2.png");
    sprites["0110"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/3.png");
    sprites["0011"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/4.png");
    sprites["1001"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/5.png");
    sprites["1111"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/6.png");
    sprites["1010"] = texture;
    sprites["1000"] = texture;
    sprites["0010"] = texture;
    sprites["0000"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/7.png");
    sprites["0101"] = texture;
    sprites["0100"] = texture;
    sprites["0001"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/8.png");
    sprites["1110"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/9.png");
    sprites["1011"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/10.png");
    sprites["0111"] = texture;
    texture.loadFromFile(
        "C:/Users/kosty/CLionProjects/untitled1/SII/Sprites/11.png");
    sprites["1101"] = texture;
}

void LogError(string msg) {
    terminal_messages.emplace_front(msg, Color::Red);
}

void LogMsg(string msg) {
    terminal_messages.emplace_front(msg, Color::Green);
}

vector<string> Split(string msg) {
    vector<string> ans;
    string cur;
    for (char i : msg) {
        if (i == ' ') {
            if (!cur.empty()) {
                ans.push_back(cur);
            }
            cur = "";
        } else {
            cur += i;
        }
    }
    if (!cur.empty()) {
        ans.push_back(cur);
    }
    return ans;
}

int CreateCargo(string name, int pickup_id, bool priority) {
    if (pickups.find(pickup_id) == pickups.end()) {
        return -1;
    }
    int i = 0;
    while (cargos.find(i) != cargos.end()) {
        i++;
    }
    Cargo c;
    c.create(i, name, priority);
    cargos[i] = c;
    cargos[i].putOnPickUp(pickup_id);
    pickups[pickup_id].addCargo(i);
    return i;
}

void ProcessCommand(string command) {
    vector<string> splt = Split(command);
    if (splt[0] == "start") {
        LogMsg("The simulation has started");
        simulation_working = true;
    } else if (splt[0] == "stop") {
        LogMsg("The simulation has stopped");
        simulation_working = false;
    } else if (splt[0] == "get") {
        if (splt.size() != 3) {
            LogError("Invalid command!");
            return;
        }
        int id = 0;
        try {
            id = stoi(splt[2]);
        } catch (const char* error_message) {
            LogError("Invalid command!");
            return;
        }
        if (splt[1] == "robot") {
            if (robots.find(id) == robots.end()) {
                LogError("Invalid robot ID!");
            }
        } else if (splt[1] == "dock") {
            if (docks.find(id) == docks.end()) {
                LogError("Invalid dock ID!");
            }
        } else if (splt[1] == "shelf") {
            if (shelves.find(id) == shelves.end()) {
                LogError("Invalid shelf ID!");
            }
        } else if (splt[1] == "pickup") {
            if (pickups.find(id) == pickups.end()) {
                LogError("Invalid pickup ID!");
            }
        } else if (splt[1] == "cargo") {
            if (cargos.find(id) == cargos.end()) {
                LogError("Invalid cargo ID!");
            }
        }
    } else if (splt[0] == "order") {
        if (pickups.empty()) {
            LogError("No pickups!");
            return;
        }
        if (splt.size() == 2) {
            string command = splt[1];
            if (command == "test") {
                int name = 0;
                for (int i = 0; i < pickups.size(); ++i) {
                    for (int j = 0; j < 1; ++j) {
                        int cargo_id = CreateCargo(to_string(name), name, 0);
                        orders.push_back({true, false, cargo_id, i, -1});
                        name++;
                    }
                }
                LogMsg("Done");
            }
            return;
        }
        if (splt.size() != 3) {
            LogError("Invalid command! order {id} {name}");
            return;
        }
        int id = stoi(splt[1]);
        string name = splt[2];
        for (auto& c : cargos) {
            if (c.second.getName() == name) {
                LogError("Name is already existing");
                return;
            }
        }
        int cargo_id = CreateCargo(name, id, 0);
        if (cargo_id == -1) {
            LogError("Invalid pickup ID!");
            return;
        }
        orders.push_back({true, false, cargo_id, id, -1});
        LogMsg("The order has been created");
    } else if (splt[0] == "receive") {
        if (splt.size() != 3) {
            LogError("Invalid command! receive {name} {id}");
            return;
        }
        string name = splt[1];
        int id = stoi(splt[2]);
        for (auto& c : cargos) {
            if (c.second.getName() == name) {
                orders.push_back({false, false, c.first, id,
                                  c.second.getShelfId()});
                LogMsg("The order is being delivered");
                return;
            }
        }
        LogError("Invalid order name!");
    } else if (splt[0] == "test") {
        if (splt.size() != 4) {
            LogError("Invalid command!");
            return;
        }
        int x = stoi(splt[1]);
        int y = stoi(splt[2]);
        int id = stoi(splt[3]);
        if (robots.find(id) == robots.end()) {
            LogError("Invalid robot ID!");
            return;
        }
        pair<int, int> pos = robots[id].getPos();
        vector<vector<int>> a = RebuildMatrix();
        vector<pair<int, int>> path = bfs(a, pos.first, pos.second, x, y, n, m,
                                          ticks_, id + 1);
        vector<path_point> ans;
        int cur_ticks = ticks_;
        for (auto elem : path) {
            ans.push_back({elem.first, elem.second,
                           a[elem.first][elem.second]});
            banned_time[elem.first][elem.second][cur_ticks++] = id + 1;
        }
        robots[id].setPath(ans);
    } else if (splt[0] == "it") {
        RobotsIteration();
        ticks_++;
    } else {
        LogError("Command, named '" + command + "' does not exist!");
    }
}

string GetMask(int x, int y) {
    string mask = "0000";
    if (y - 1 >= 0 && !field[x][y - 1].isNone()) {
        mask[0] = '1';
    }
    if (x - 1 >= 0 && !field[x - 1][y].isNone()) {
        mask[1] = '1';
    }
    if (y + 1 < n && !field[x][y + 1].isNone()) {
        mask[2] = '1';
    }
    if (x + 1 < n && !field[x + 1][y].isNone()) {
        mask[3] = '1';
    }
    return mask;
}

void DrawField() {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            Color cul = colors["road"];
            if (field[i][j].isNone()) {
                cul = colors["none"];
            }
            Color col;
            if (field[i][j].isRoad()) {
                col = colors["road"];
            }
            if (field[i][j].isShelf()) {
                col = colors["shelf"];
            }
            if (field[i][j].isDock()) {
                col = colors["dock"];
            }
            if (field[i][j].isPickUp()) {
                col = colors["pickup"];
            }
            if (selected_point.first == i && selected_point.second == j) {
                col = colors["selected"];
                cul = colors["selected"];
            }
            RectangleShape box_empty(
                sf::Vector2f(square_size + 2, square_size + 2));
            box_empty.setOutlineColor(colors["none"]);
            box_empty.setOutlineThickness(1);
            box_empty.setPosition(j * square_size, i * square_size);
            box_empty.setFillColor(cul);

            RectangleShape box(Vector2f(square_size, square_size));
            box.setFillColor(cul);
            box.setPosition(j * square_size, i * square_size);
            window.draw(box);
            if (!field[i][j].isNone()) {
                if (field[i][j].isRoad()) {
                    string mask = GetMask(i, j);
                    if (mask != "0000") {
                        Texture tx = sprites[mask];
                        Sprite sp;
                        sp.setTexture(tx);
                        sp.setPosition(j * square_size, i * square_size);
                        sp.setScale((float)(square_size) / (float)80,
                                    (float)(square_size) / (float)80);
                        window.draw(sp);
                    }
                } else {
                    window.draw(box_empty);
                    DrawRect(
                        window,
                        (float)j * (float)square_size + (float)square_size / (
                            float)10,
                        (float)i * (float)square_size + (float)square_size / (
                            float)10,
                        (float)square_size / (float)5 * 2, col);
                }
            } else {
                window.draw(box_empty);
            }
            // Type(window, j * square_size + square_size / 2,
            //      i * square_size + square_size / 2,
            //      to_string(i) + " " + to_string(j), Color(194, 31, 212), 30);
        }
    }
    for (auto r : robots) {
        if (!r.second.getStatus()) {
            continue;
        }
        vector<path_point> path = r.second.getPath();
        int cur = r.second.getCurPath();
        if (cur == 0) {
            continue;
        }
        bool was_type = false;
        for (int i = cur; i < path.size(); ++i) {
            if (was_type) {
                break;
            }
            int x = path[i - 1].x, y = path[i - 1].y, nx = path[i].x, ny = path[
                    i].y;
            if (x == nx && y == ny) {
                continue;
            }
            if (path[i].type != -1) {
                was_type = true;
            }
            RectangleShape line(Vector2f(square_size, 6));
            if (x > nx) {
                line.rotate(90);
                line.setPosition(
                    y * (double)square_size + (double)square_size / (double)2 +
                    3,
                    x * (double)square_size - (double)square_size / (double)2);
            } else if (x < nx) {
                line.rotate(90);
                line.setPosition(
                    y * (double)square_size + (double)square_size / (double)2 +
                    3,
                    x * (double)square_size + (double)square_size / (double)2);
            } else if (y > ny) {
                line.setPosition(
                    y * square_size - (double)square_size / (double)2,
                    x * (double)square_size + (double)square_size /
                    (double)2 - 3);
            } else {
                line.setPosition(
                    y * square_size + (double)square_size / (double)2,
                    x * (double)square_size + (double)square_size /
                    (double)2 - 3);
            }
            line.setFillColor(robot_colors[r.first % robot_colors.size()]);
            window.draw(line);
        }
    }
    for (auto r : robots) {
        pair<int, int> pos = r.second.getPos();
        if (r.second.getStatus()) {
            CircleShape circle(square_size / 2 - square_size / 4);
            circle.setFillColor(robot_colors[r.first % robot_colors.size()]);
            circle.setPosition(pos.second * square_size + square_size / 4,
                               pos.first * square_size + square_size / 4);
            window.draw(circle);
            // Type(window,
            //      (float)pos.second * (float)square_size + (float)square_size / (
            //          float)2,
            //      (float)pos.first * (float)square_size + (float)square_size / (
            //          float)2 - 6 * (float)square_size / (float)72,
            //      to_string(r.second.getId()), Color::White,
            //      40 * square_size / 72);
        }
    }
}

void DrawStats(RenderWindow& window, string s, int x, int y) {
    int height = 50, width = GetTextSize(s, 30).second;
    Color cul = Color::Black;
    CircleShape circle(height / 2);
    circle.setPosition(x, y);
    circle.setFillColor(cul);
    window.draw(circle);

    circle.setPosition(x + width, y);
    window.draw(circle);

    RectangleShape box_but1(Vector2f(width, height));
    box_but1.setPosition(x + height / 2, y);
    box_but1.setFillColor(cul);
    window.draw(box_but1);
    Type(window, x + height / 2 + width / 2,
         y + height / 2 - 8, s, Color::White, 30);
}

void DrawHelpWindow(int x, int y) {
    help_window.setVisible(true);
    help_window.clear(Color(46, 47, 56));
    Point p = field[x][y];
    if (p.isDock()) {
        Type(help_window, 300, 30, "Dock Station", Color::White, 40);
        Dock d = docks[p.getDockId()];
        pair<int, int> pos = d.getPos();
        int size = d.getSize();
        int id = d.getId();
        int free = d.getFreeRobots();
        vector<int> robots = d.getRobotsId();
        DrawStats(help_window, "Id: " + to_string(id), 30, 100);
        DrawStats(help_window, "Size: " + to_string(size), 30, 160);
        DrawStats(help_window, "Free: " + to_string(free), 30, 220);
        DrawStats(help_window, "X: " + to_string(pos.first), 400, 160);
        DrawStats(help_window, "Y: " + to_string(pos.second), 510, 160);
        string ids;
        for (int id1 : robots) {
            ids += to_string(id1) + "; ";
        }
        DrawStats(help_window, "Robots: " + ids, 30, 280);
    } else if (p.isShelf()) {
        Type(help_window, 320, 30, "Shelf", Color::White, 40);
        Shelf s = shelves[p.getShelfId()];
        pair<int, int> pos = s.getPos();
        int size = s.getSize();
        int id = s.getId();
        int free = s.getFrSpace();
        vector<int> cargos = s.getCargos();
        DrawStats(help_window, "Id: " + to_string(id), 30, 100);
        DrawStats(help_window, "Size: " + to_string(size), 30, 160);
        DrawStats(help_window, "Free: " + to_string(free), 30, 220);
        DrawStats(help_window, "X: " + to_string(pos.first), 400, 160);
        DrawStats(help_window, "Y: " + to_string(pos.second), 510, 160);
        string ids;
        for (int id : cargos) {
            ids += to_string(id) + "; ";
        }
        DrawStats(help_window, "Cargos: " + ids, 30, 280);
    } else if (p.isPickUp()) {
        Type(help_window, 320, 30, "Pickup", Color::White, 40);
        PickUp s = pickups[p.getPickUpId()];
        pair<int, int> pos = s.getPos();
        set<int> cargos = s.getCargos();
        int id = s.getId();
        DrawStats(help_window, "Id: " + to_string(id), 30, 100);
        DrawStats(help_window, "X: " + to_string(pos.first), 400, 100);
        DrawStats(help_window, "Y: " + to_string(pos.second), 510, 100);
        string ids;
        for (int id : cargos) {
            ids += to_string(id) + "; ";
        }
        DrawStats(help_window, "Cargos: " + ids, 30, 160);
    } else {
        int id = -1;
        for (auto r : robots) {
            pair<int, int> pos = r.second.getPos();
            if (pos.first == x && pos.second == y) {
                id = r.first;
                break;
            }
        }
        if (id == -1) {
            Type(help_window, 320, 30, "Nothing :)",
                 Color::White, 40);
            DrawStats(help_window, "X: " + to_string(x), 240, 100);
            DrawStats(help_window, "Y: " + to_string(y), 350, 100);
        } else {
            Type(help_window, 320, 30, "Robot", Color::White, 40);
            Robot r = robots[id];
            int cid = r.getCargoId();
            bool status = r.getStatus();
            int did = r.getDockId();
            DrawStats(help_window, "Id: " + to_string(id), 30, 100);
            DrawStats(help_window, "Busy: " + to_string(status), 30, 160);
            DrawStats(help_window, "Cargo Id: " + to_string(cid), 30, 220);
            DrawStats(help_window, "Dock Id: " + to_string(did), 30, 280);
            DrawStats(help_window, "X: " + to_string(x), 400, 160);
            DrawStats(help_window, "Y: " + to_string(y), 510, 160);
        }
    }
    help_window.display();
}

void DrawTerminal() {
    Type(window, 1470, 1375, ">", Color::White, 50);
    if (term_status && is_terminal_selected) {
        int sz = GetTextSize(terminal, 50).second;
        RectangleShape box(Vector2f(3, 50));
        box.setPosition(1500 + sz, 1373);
        box.setFillColor(Color::White);
        window.draw(box);
    }
    for (int i = 0; i < min((int)terminal_messages.size(), (int)15); ++i) {
        if (terminal_messages[i].second == Color::White) {
            Type2(window, 1470, 1290 - 60 * i, ">", Color::White, 50);
        }
        Type2(window, 1500, 1290 - 60 * i, terminal_messages[i].first,
              terminal_messages[i].second, 50);
    }
    Type2(window, 1500, 1365, terminal, Color::White, 50);
}

void DrawButtons(RenderWindow& window) {
    int cnt = 0;
    for (button but : buttons) {
        cnt++;
        Color cul = Color(43, 45, 48);
        if (cnt == current_illumination)
            cul = Color(65, 119, 244);
        CircleShape circle(but.height / 2);
        circle.setPosition(but.x, but.y);
        circle.setFillColor(cul);
        window.draw(circle);

        circle.setPosition(but.x + but.width, but.y);
        window.draw(circle);

        RectangleShape box_but1(Vector2f(but.width, but.height));
        box_but1.setPosition(but.x + but.height / 2, but.y);
        box_but1.setFillColor(cul);
        window.draw(box_but1);
        Type(window, but.x + but.height / 2 + but.width / 2,
             but.y + but.height / 2 - 8, but.text, Color::White, but.font_size);
    }
}

void ButtonsIllumination(RenderWindow& window) {
    int wind_x = window.getPosition().x;

    int wind_y = window.getPosition().y;

    int mouse_x = Mouse::getPosition().x - 14;

    int mouse_y = Mouse::getPosition().y - 60;

    if (mouse_x >= wind_x and mouse_x <= wind_x + window.getSize().x and
        mouse_y >= wind_y and mouse_y <= wind_y + window.getSize().y) {
        int field_x = mouse_x - wind_x;
        int field_y = mouse_y - wind_y;
        int cnt = 0;
        for (button but : buttons) {
            cnt++;
            int x1 = but.x;
            int y1 = but.y;
            int h = but.height;
            int w = but.width;
            if (field_x > x1 and field_x < x1 + w + h and field_y > y1 and
                field_y < y1 + h) {
                current_illumination = cnt;
                return;
            }
        }
        current_illumination = -1;
    }
}

void TerminalKeyborad(Event& event) {
    if (!is_terminal_selected) {
        return;
    }
    if (!CanPressButton(event.key.code)) {
        return;
    }
    if (event.key.code == 8) {
        if (!terminal.empty())
            terminal.pop_back();
    }
    if (event.key.code == 13) {
        if (!terminal.empty()) {
            terminal_messages.emplace_front(terminal, Color::White);
            my_msg.push_front(terminal);
            ProcessCommand(terminal);
            terminal = "";
        }
        cur_my_msg = -1;
    }
    if (event.key.code == 73) {
        if (!my_msg.empty()) {
            cur_my_msg = min(cur_my_msg + 1, (int)my_msg.size() - 1);
            terminal = my_msg[cur_my_msg];
        }
        return;
    }
    if (event.key.code == 74) {
        if (!my_msg.empty()) {
            if (cur_my_msg - 1 < 0) {
                terminal = "";
                cur_my_msg = -1;
            } else {
                cur_my_msg = cur_my_msg - 1;
                terminal = my_msg[cur_my_msg];
            }
        }
        return;
    }
    if (!(event.key.code >= 32 && event.key.code <= 126) || event.key.code ==
        38 || event.key.code == 39) {
        return;
    }
    terminal += char(event.key.code);
    term_status = false;
}

void MouseReaction(Event event) {
    if (!is_locked_buttons) {
        is_locked_buttons = true;
        int x = event.mouseButton.x;
        int y = event.mouseButton.y;
        if (x / square_size < n && y / square_size < m) {
            is_terminal_selected = false;
            if (event.mouseButton.button == Mouse::Left) {
                selected_point = {y / square_size, x / square_size};
            } else {
                if (x / square_size < n && y / square_size < m) {
                    DrawHelpWindow(y / square_size, x / square_size);
                }
            }
        } else {
            int x = event.mouseButton.x;
            int y = event.mouseButton.y;
            for (button but : buttons) {
                int x1 = but.x;
                int y1 = but.y;
                int h = but.height;
                int w = but.width;
                if (x > x1 and x < x1 + w + h and y > y1 and y < y1 + h and
                    event.mouseButton.button == Mouse::Left) {
                    if (but.action == 1) {
                        LoadField();
                    } else if (but.action == 2) {
                        SaveField();
                    }
                }
            }
            is_terminal_selected = true;
        }
    }
}

void KeyboardReaction(Event& event) {
    if (CanPressButton(event.key.code)) {
        if (selected_point.first != -1) {
            if (event.key.code == 114) {
                field[selected_point.first][selected_point.second].becomeRoad();
            }
            if (event.key.code == 100) {
                field[selected_point.first][selected_point.second].
                    becomeDock(1);
            }
            if (event.key.code == 110) {
                field[selected_point.first][selected_point.second].becomeNone();
            }
            if (event.key.code == 115) {
                field[selected_point.first][selected_point.second].
                    becomeShelf(1);
            }
            if (event.key.code == 112) {
                field[selected_point.first][selected_point.second].
                    becomePickUp();
            }
            selected_point = {-1, -1};
        }
    }
}

int main() {
    /*
        * R - поставить дорогу
        * D - поставить док станцию
        * N - поставить пустоту
        * S - поставить полку
        * P - Пункт выдачи/приема заказов
    */
    square_size = 1440 / n;
    field.resize(n, vector<Point>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            field[i][j].create(i, j);
        }
    }
    LoadSprites();
    CreateButtons();
    help_window.setVisible(false);
    int prev = GetEpochTime();
    while (window.isOpen()) {
        if (GetEpochTime() != prev) {
            prev = GetEpochTime();
            seconds_++;
            term_status = !term_status;
            if (simulation_working) {
                RobotsIteration();
                ticks_++;
            }
        }
        window.clear(Color(12, 12, 12));
        DrawButtons(window);
        ButtonsIllumination(window);
        Event event;
        while (window.pollEvent(event) and window.hasFocus()) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }
        while (help_window.pollEvent(event) and help_window.hasFocus()) {
            if (event.type == Event::Closed) {
                help_window.setVisible(false);
            }
        }

        if (event.type == Event::MouseButtonPressed) {
            MouseReaction(event);
        }
        if (event.type == Event::MouseButtonReleased) {
            is_locked_buttons = false;
        }

        if (event.type == Event::KeyPressed or event.type == 4) {
            buttons_bufer.push_front(event.key.code);
            if (buttons_bufer.size() > 200) {
                buttons_bufer.pop_back();
            }
            KeyboardReaction(event);
            TerminalKeyborad(event);
        } else {
            buttons_bufer.push_front(1);
            if (buttons_bufer.size() > 200) {
                buttons_bufer.pop_back();
            }
        }

        DrawField();
        DrawTerminal();
        window.display();
    }
}
