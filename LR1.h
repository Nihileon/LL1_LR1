//
// Created by Nihileon on 2019/11/18.
//

#ifndef LL1_LR1_H
#define LL1_LR1_H

#include <string>
#include <set>
#include <utility>
#include <iostream>
#include <vector>
#include <stack>
#include <map>

using namespace std;

struct FuncState {

    friend bool operator<(const FuncState &, const FuncState &);

    FuncState() = default;

    FuncState(string left, string right, int pos, string next) {
        this->left = std::move(left);
        this->right = std::move(right);
        this->pos = pos;
        this->next = std::move(next);
    }

    bool canReduced() {
        if (right == "#") {
            return true;
        } else if (pos == right.size()) {
            return true;
        }
        return false;
    }

    string left;
    string right;
    //pos指向下个检测的元素 [0,right.size()+1]
    int pos;
    string next;
};


// 文法4.7 p117
class LR1 {
public:
    const string BEGIN_SYMBOL = "S";
    map<string, vector<string>> funcsMap;
    vector<pair<string, string>> funcPairs;
    map<string, set<string>> first;
    set<string> T{"+", "-", "*", "/", "(", ")", "n"};
    set<string> N{"S", "E", "T", "F"};
    FuncState beginFunc = FuncState("S", "E", 0, "$");

    vector<set<FuncState>> closures;
    map<pair<int, string>, int> gotoMap;
    //map[<state nextCh>] = <"S", nextState> / <"R",reduceFuncNo> / <"Accept",>
    map<pair<int, string>, pair<string, int>> actionMap;

    void initFunction() {

        /*
         * 0 S->E
         * 1 E->E+T
         * 2 E->E-T
         * 3 E->T
         * 4 T->T*F
         * 5 T->T/F
         * 6 T->F
         * 7 F->(E)
         * 8 F->n
         */
        auto *v = new vector<string>();
        v->push_back("E");
        funcsMap["S"] = *v;

        v = new vector<string>();
        v->push_back("E+T");
        v->push_back("E-T");
        v->push_back("T");
        funcsMap["E"] = *v;


        v = new vector<string>();
        v->push_back("T*F");
        v->push_back("T/F");
        v->push_back("F");
        funcsMap["T"] = *v;

        v = new vector<string>();
        v->push_back("(E)");
        v->push_back("n");
        funcsMap["F"] = *v;

        for (auto i: funcsMap) {
            for (auto j:i.second) {
                funcPairs.emplace_back(i.first, j);
            }
        }
    }

    void initFirst() {
        for (const auto &i : N) {
            first[i] = set<string>();
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


    set<string> getFirst(string str, set<string> next) {
        set<string> thisFirst;

        for (int i = 0; i < str.size(); i++) {
            string s = str.substr(i, 1);
            if (s == "#") {
                continue;
            }
            if (T.find(s) != T.end()) {
                thisFirst.insert(s);
                return thisFirst;
            } else {
                for (auto i: first[s]) {
                    thisFirst.insert(i);
                }
                if (first[s].find("#") == first[s].end()) {
                    return thisFirst;
                }
                if (i == str.size() - 1) {
                    for (auto nextStr:next) {
                        thisFirst.insert(nextStr);
                    }
                }
            }
        }
        return thisFirst;
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

    int closureExistNo(set<FuncState> newState) {
        for (int i = 0; i < closures.size(); i++) {
            auto state = closures[i];
            if (state.size() == newState.size()) {
                int findCount = 0;
                for (auto fs: newState) {
                    if (state.find(fs) != state.end()) {
                        findCount++;
                    }
                }
                if (findCount == state.size()) {
                    return i;
                }
            }
        }
        return -1;
    }

    set<FuncState> getStateClosure(set<FuncState> state0) {
        int size;
        do {
            size = state0.size();
            for (auto f : state0) {
                if (f.right == "#") {
                    continue;
                }
                if (f.pos < f.right.size() && N.find(f.right.substr(f.pos, 1)) != N.end()) {
                    set<string> nextSet;
                    for (auto fs:state0) {
                        if (f.left == fs.left && f.right == fs.right && f.pos == fs.pos) {
                            nextSet.insert(fs.next);
                        }
                    }
                    set<string> currFuncStateFollow;
                    int nextPos = f.pos + 1;
                    if (nextPos >= f.right.size()) {
                        currFuncStateFollow = nextSet;
                    } else {
                        currFuncStateFollow = getFirst(f.right.substr(nextPos, f.right.size() - nextPos), nextSet);
                    }
                    string left = f.right.substr(f.pos, 1);
                    auto rights = funcsMap[left];
                    for (auto t:currFuncStateFollow) {
                        for (auto right:rights) {
                            FuncState fs(left, right, 0, t);
                            state0.insert(fs);
                        }
                    }
                }
            }
        } while (state0.size() != size);
        return state0;

    }

    set<string> getNext(set<FuncState> state) {
        set<string> next;
        for (auto fs:state) {
            if (fs.pos < fs.right.size()) {
                next.insert(fs.right.substr(fs.pos, 1));
            }
        }
        return next;
    }

    void makeClosure() {
        set<FuncState> state0;
        state0.insert(beginFunc);
        state0 = getStateClosure(state0);
        closures.push_back(state0);
        int closureSize;
        do {
            closureSize = closures.size();
            for (auto curState = 0; curState < closures.size(); curState++) {
                auto c = closures[curState];
                auto next = getNext(c);
                //todo reduce
                for (auto i:c) {
                    if (i.canReduced()) {
                        for (int k = 0; k < funcPairs.size(); k++) {
                            if (funcPairs.at(k).first == i.left && funcPairs.at(k).second == i.right) {
                                actionMap[make_pair(curState, i.next)] = make_pair("R", k);
                                break;
                            }
                        }
                    }
                }
                //shift
                for (auto n:next) {
                    set<FuncState> newClosure;
                    //todo maketable
                    for (auto fs:c) {
                        if (!fs.canReduced() && fs.right.substr(fs.pos, 1) == n) {
                            FuncState newFs = fs;
                            newFs.pos++;
                            newClosure.insert(newFs);
                        }
                    }
                    newClosure = getStateClosure(newClosure);
                    int nextState = closureExistNo(newClosure);
                    if (nextState == -1) {
                        closures.push_back(newClosure);
                        nextState = closures.size() - 1;
                    }

                    if (N.find(n) != N.end()) {
                        gotoMap[pair(curState, n)] = nextState;
                    } else {
                        for (auto fs:c) {
                            if (!fs.canReduced() && fs.right.substr(fs.pos, 1) == n) {
                                actionMap[pair(curState, n)] = pair("S", nextState);
                            }
                        }
                    }
                }
            }
        } while (closures.size() != closureSize);
    }

    void analyze(string s) {
        cout << "begin analyze" << endl;
        auto ana_str = s + "$";
        stack<string> symbol_stack;
        stack<int> state_stack;
        state_stack.push(0);
        symbol_stack.push("-");
        int analyze_ptr = 0;
        auto a = ana_str.substr(0, 1);
        int S;
        do {
            S = state_stack.top();
            a = ana_str.substr(analyze_ptr, 1);
            cout << ana_str.substr(analyze_ptr) << "\t";
            if (actionMap.find(pair(S, a)) != actionMap.end()) {
                auto right = actionMap[pair(S, a)];
                cout << "<" << S << "," << a << ">:<" << right.first << "," << right.second
                     << ">" << endl;
                if (right.first == "S") {
                    auto S_ = right.second;
                    symbol_stack.push(a);
                    state_stack.push(S_);
                    analyze_ptr++;
                } else if (right.first == "R") {
                    if (funcPairs[right.second].first == BEGIN_SYMBOL) {
                        cout << "accept" << endl;
                        return;
                    }
                    for (int i = 0; i < funcPairs.at(right.second).second.size(); i++) {
                        symbol_stack.pop();
                        state_stack.pop();
                    }
                    auto S_ = state_stack.top();
                    if (gotoMap.find(pair(S_, funcPairs.at(right.second).first)) != gotoMap.end()) {
                        symbol_stack.push(funcPairs.at(right.second).first);
                        state_stack.push(gotoMap[pair(S_, funcPairs.at(right.second).first)]);
                    }
                }
                cout << "state:" << state_stack.top() << " size:" << state_stack.size() << "\tsymbol:"
                     << symbol_stack.top() << " size:" << symbol_stack.size() << endl;
            } else {
                cout << "error" << endl;
                break;
            }

        } while (1);

    }


    void printClosure() {
        cout << "closure:" << closures.size() << endl;
        for (int j = 0; j < closures.size(); j++) {
            cout << endl << "closure:" << j << endl;

            for (const auto &i:closures[j]) {
                cout << i.left << "->" << i.right << ":" << to_string(i.pos) << ":" << i.next << endl;
            }
        }
        cout << endl;
    };

    void printActionMap() {
        cout << "action:" << actionMap.size() << endl;
        for (int i = 0; i < closures.size(); i++) {
            for (const auto &j:T) {
                auto x = make_pair(i, j);
                if (actionMap.find(x) != actionMap.end()) {
                    auto y = actionMap[x];
                    cout << "<" << i << "," << j << ">:<" << y.first << "," << y.second
                         << ">" << endl;
                }
            }
            auto x = make_pair(i, "$");
            if (actionMap.find(x) != actionMap.end()) {
                auto y = actionMap[x];
                cout << "<" << i << "," << "$" << ">:<" << y.first << "," << y.second
                     << ">" << endl;
            }
        }
        cout << endl;
    }

    void printGotoMap() {
        cout << "goto:" << gotoMap.size() << endl;
        for (const auto &i : gotoMap) {
            cout << "<" << i.first.first << "," << i.first.second << ">:<" << i.second
                 << ">" << endl;
        }
        cout << endl;
    }

    void printFunc() {
        for (auto i:funcPairs) {
            cout << i.first << "->" << i.second << endl;
        }
    }

public:
    void init() {
        this->initFunction();
        this->initFirst();
        this->addFirst();
        this->makeClosure();
        this->printClosure();
        this->printActionMap();
        this->printGotoMap();
        this->printFunc();
    }
};


#endif //LL1_LR1_H
