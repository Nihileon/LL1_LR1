#include <iostream>
#include <fstream>
#include "LL1.h"
#include "LR1.h"

int main() {
    LL1 ll1;
    ll1.init();

    ll1.analyze("i+i*i");
    cout << endl;
    ll1.analyze("i+i*(i+i)");
    cout << endl;


    LR1 lr1;
    lr1.init();

    lr1.analyze("******i=****i");
    cout << endl;
    lr1.analyze("****i==***i");
    return 0;
}