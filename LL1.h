//
// Created by Nihileon on 2019/11/18.
//

#ifndef LL1_LL1_H
#define LL1_LL1_H


#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct NTPair {
    string N;
    string T;

    friend bool operator<(const NTPair &, const NTPair &);

    NTPair(string N, string T) {
        this->N = std::move(N);
        this->T = std::move(T);
    }
};

//example 4.3 p84 ppt p47
class LL1 {

public:
    const string BEGIN_SYMBOL = "E";
    set<string> T{"#", "+", "*", "i", "(", ")", "$"};
    set<string> N{"E", "S", "T", "D", "F"};
    map<string, vector<string>> funcsMap;
    map<string, set<string>> first;
    map<string, set<string>> follow;
    map<NTPair, pair<string, string>> table;

    void initFunction() {

        auto *v = new vector<string>();
        v->push_back("TS");
        funcsMap["E"] = *v;

        //E'->S
        v = new vector<string>();
        v->push_back("+TS");
        //epsilon ->#
        v->push_back("#");
        funcsMap["S"] = *v;

        //T'->D
        v = new vector<string>();
        v->push_back("FD");
        funcsMap["T"] = *v;

        v = new vector<string>();
        v->push_back("*FD");
        v->push_back("#");
        funcsMap["D"] = *v;

        v = new vector<string>();
        v->push_back("(E)");
        v->push_back("i");
        funcsMap["F"] = *v;
    }

    void initFirst() {
        for (const auto &i : N) {
            first[i] = set<string>();
        }
    }

    void initFollow() {
        for (const auto &i : N) {
            follow[i] = set<string>();
        }
    }

    static map<string, set<string>> cloneMap(map<string, set<string>> &m) {
        auto newMap = map<string, set<string>>();
        for (const auto &i : m) {
            newMap[i.first] = i.second;
        }
        return newMap;
    }

    static bool isMapSetChanged(map<string, set<string>> &originMapSet, map<string, set<string>> &newMapSet) {
        for (const auto &i : originMapSet) {
            auto originSet = i.second;
            auto newSet = newMapSet[i.first];
            for (const auto &j : newSet) {
                if (originSet.find(j) == originSet.end()) {
                    return true;
                }
            }
        }
        return false;
    }

    void addFirst() {
        auto lastMap = cloneMap(first);
        for (const auto &func : funcsMap) {
            auto left = func.first;
            auto rights = func.second;
            //第一个为终结符
            for (const string &right : rights) {
                auto firstCharAtRight = right.substr(0, 1);
                if (T.find(firstCharAtRight) != T.end()) {
                    first[left].insert(firstCharAtRight);
                } else {
                    for (auto ch : right) {
                        string s = string(1, ch);
                        if (T.find(s) != T.end()) {
                            first[left].insert(s);
                            break;
                        } else if (N.find(s) != N.end()) {
                            for (const auto &t : first[s]) {
                                first[left].insert(t);
                            }
                            if (first[s].find("#") == first[s].end()) {
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                }
            }
        }
        if (isMapSetChanged(lastMap, first)) {
            addFirst();
        }
    }

    void addFollow() {
        auto lastMap = cloneMap(follow);
        follow["E"].insert("$");

        for (const auto &funcIter : funcsMap) {
            for (const auto &r : funcIter.second) {
                for (int i = 0; i < r.size() - 1; i++) {
                    auto former = string(1, r[i]);
                    auto latter = string(1, r[i + 1]);
                    if (N.find(former) != N.end()) {
                        if (N.find(latter) != N.end()) {
                            follow[former].insert(first[latter].begin(), first[latter].end());
                        } else if (T.find(latter) != T.end()) {
                            follow[former].insert(latter);
                        }
                    }
                }
            }
        }

        for (const auto &funcIter : funcsMap) {
            auto left = funcIter.first;
            for (const auto &r : funcIter.second) {
                for (int i = r.size() - 1; i >= 0; i--) {
                    auto ch = string(1, r[i]);
                    if (N.find(ch) != N.end()) {
                        follow[ch].insert(follow[left].begin(), follow[left].end());
                    }
                    if (N.find(ch) == N.end() || first[ch].find("#") == first[ch].end()) {
                        break;
                    }
                }
            }
        }

        if (isMapSetChanged(lastMap, follow)) {
            addFollow();
        } else {
            for (const auto &i : follow) {
                auto key = i.first;
                auto &value = follow[key];
                for (auto iter = value.begin(); iter != value.end();) {
                    if (*iter == "#") {
                        value.erase(iter++);
                    } else {
                        iter++;
                    }
                }
            }
        }
    }

    void makeTable() {
        for (const auto &func : funcsMap) {
            auto left = func.first;
            auto right = func.second;
            for (const auto &str : right) {
                auto ch = str.substr(0, 1);
                if (ch == "#") {
                    for (const auto &followCh : follow[left]) {
                        NTPair ntPair(left, followCh);
                        table[ntPair] = make_pair(left, str);
                    }
                } else if (T.find(ch) != T.end()) {
                    NTPair ntPair(left, ch);
                    table[ntPair] = make_pair(left, str);
                } else if (N.find(ch) != N.end()) {
                    for (const auto &firstCh : first[ch]) {
                        NTPair ntPair(left, firstCh);
                        table[ntPair] = make_pair(left, str);
                    }
                }
            }
        }
    }

    void analyze(string str) {
        stack<string> st;
        st.push("$");
        st.push(BEGIN_SYMBOL);
        auto ana_str = str + "$";
        int analyze_ptr = 0;
        auto a = ana_str.substr(0, 1);
        string X = "";
        do {
            X = st.top();
            if (T.find(X) != T.end() || X == "$") {
//                cout << X << " " << a << endl;
                if (X == "$") {
                    cout << "accept" << endl;
                    return;
                } else if (X == a) {
                    st.pop();
                    analyze_ptr++;
                    if (analyze_ptr >= ana_str.size()) {
                        cout << "analyze_ptr out of range error" << endl;
                        return;
                    }
                    a = ana_str.substr(analyze_ptr, 1);
                } else {
                    cout << "error1" << endl;
                    return;
                }
            } else {
                if (table.find(NTPair(X, a)) != table.end()) {
                    st.pop();
                    auto func = table[NTPair(X, a)];
                    auto right_str = func.second;
                    for (int i = right_str.size() - 1; i >= 0; i--) {
                        if (string(1, right_str.at(i)) == "#") {
                            break;
                        }
                        st.push(string(1, right_str.at(i)));
                    }
                    cout << X << "->" << func.second << endl;
                } else {
                    cout << "error 2" << endl;
                    return;
                }
            }
        } while (1);
    }

    void printFirst() {
        cout << "first:" << first.size() << endl;
        for (const auto &i : first) {
            cout << i.first << "->";
            for (const auto &j : i.second) {
                cout << j << " ";
            }
            cout << "\n";
        }
        cout << "\n";
    }

    void printFollow() {
        cout <<"follow:" <<endl;
        for (const auto &i :follow) {
            cout << i.first << "->";
            for (const auto &j : i.second) {
                cout << j << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    void printTable() {
        cout << "table:" <<table.size() << endl;
        for (auto i: table) {
            cout << "<" << i.first.N << "," << i.first.T << ">" << "->" << i.second.second << endl;
        }
        cout << endl;
    }

    void init() {
        this->initFunction();
        this->initFirst();
        this->initFollow();
        this->addFirst();
        this->addFollow();
        this->makeTable();

        this->printFirst();
        this->printFollow();
        this->printTable();
    }
};


#endif //LL1_LL1_H
