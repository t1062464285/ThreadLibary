// test a thread trying to join with itself.
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include <stdio.h>
#include "thread.h"

using namespace std;

thread* thread_pointer;

void child(void *a) {
    int id = (intptr_t)a;
    cout << "Start child thread " << id << endl;
    string print_string = "thread " + to_string(id) + " finishes\n";
    cout << print_string;
}

void func1(thread* thread_pointer) {
    // int id = (intptr_t)a;
    // cout << "Start child thread " << id << endl;
    thread_pointer->join();
    thread t3 ( (thread_startfunc_t) child, (void *) 3);
    t3.join();
    thread::yield();
    // string print_string = "thread " + to_string(id) + " finishes\n";
    // cout << print_string;
}


void parent(void *a) {
    thread t1 ( (thread_startfunc_t) func1, (thread* ) &t1);
    thread t2 ( (thread_startfunc_t) child, (void *) 2);
    thread::yield();

    printf("parent thread finishes\n");

}

int main() {
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);
}