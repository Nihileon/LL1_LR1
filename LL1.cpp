//
// Created by Nihileon on 2019/11/18.
//

#include "LL1.h"

bool operator<(const NTPair &x, const NTPair &y) {
    if (x.N < y.N) {
        return true;
    } else if (x.N > y.N) {
        return false;
    } else {
        if (x.T < y.T) {
            return true;
        } else {
            return false;
        }
    }
}

