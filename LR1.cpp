//
// Created by Nihileon on 2019/11/18.
//

#include "LR1.h"

bool operator<(const FuncState &x, const FuncState &y) {
    if (x.left < y.left) {
        return true;
    } else if (x.left > y.left) {
        return false;
    }

    if (x.right < y.right) {
        return true;
    } else if (x.right > y.right) {
        return false;
    }

    if (x.pos < y.pos) {
        return true;
    } else if (x.pos > y.pos) {
        return false;
    }

    if (x.next < y.next) {
        return true;
    }
    return false;
}
