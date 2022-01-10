

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

// test for yield in child threads and join

void child(void *a) {
    int id = (intptr_t)a;
    cout << "Start child thread " << id << endl;
    string print_string = "yield thread " + to_string(id) + " \n";
    cout << print_string;
    thread::yield();
    print_string = "thread " + to_string(id) + " finishes\n";
    cout << print_string;
}

void func1(void *a) {
    int id = (intptr_t)a;
    cout << "Start func1 thread " << id << endl;
    thread t3((thread_startfunc_t)child, (void *)3);
    thread t4((thread_startfunc_t)child, (void *)4);
    thread t5((thread_startfunc_t)child, (void *)5);
    t3.join();
    string print_string = "thread " + to_string(id) + " finishes\n";
    cout << print_string;
}

void parent(void *a) {
    thread t1((thread_startfunc_t)func1, (void *)1);
    thread t2((thread_startfunc_t)child, (void *)2);
    t1.join();

    printf("parent thread finishes\n");
}

int main() {
    cpu::boot(1, (thread_startfunc_t)parent, (void *)0, false, false, 0);
}