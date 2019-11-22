#include <iostream>
#include <fstream>
#include "LL1.h"
#include "LR1.h"

int main() {
    LL1 ll1;
    ll1.init();
    ll1.analyze("(((n+n)*(n-n))-((n-n)/(n/n)))");
    cout << endl;


//    LR1 lr1;
//    lr1.init();
//
//    lr1.analyze("(((n+n)*(n-n))-((n-n)/(n/n)))");
    cout << endl;
    return 0;
}