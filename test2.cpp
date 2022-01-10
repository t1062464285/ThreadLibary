// test for thread:thread cpu:init thread::join
#include <stdio.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "thread.h"

using namespace std;

void child(void *a) {
    cout << "Start child thread\n";
    int id = (intptr_t)a;

    string print_string = "thread " + to_string(id) + " finishes\n";
    cout << print_string;
}

void parent(void *a) {
    vector<int> thread_id{1, 2, 3};

    for (int i = 0; i < 3; ++i) {
        thread t1((thread_startfunc_t)child, (void *)&thread_id[i]);
        t1.join();
    }

    printf("parent thread finishes\n");
}

int main() {
    cpu::boot(1, (thread_startfunc_t)parent, (void *)0, false, false, 0);
}