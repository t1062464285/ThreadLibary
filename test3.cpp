// test for thread:thread cpu:init thread::join
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

void t1_func(void *a);
void child(void *a);
void t3_func(void *a);

void t1_func(void *a) {
    
    int id = (intptr_t)a;
    cout << "Start thread " << id << endl;

    thread t2 ( (thread_startfunc_t) child, (void *) 2);
    thread t3 ( (thread_startfunc_t) child, (void *) 3);
    t2.join();
    t3.join();
    
    string print_string = "thread " + to_string(id) + " finishes\n";
    cout << print_string;
}

void child(void *a) {
	int id = (intptr_t)a;
    cout << "Start thread " << id << endl;

    string print_string = "thread " + to_string(id) + " finishes\n";
    cout << print_string;
}

void parent(void *a) {

    thread t1 ( (thread_startfunc_t) t1_func, (void *) 1);
    t1.join();

    printf("parent thread finishes\n");

}

int main() {
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 0, false, false, 0);

}
