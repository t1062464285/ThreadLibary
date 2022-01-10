
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdio.h>
#include <string>
#include <vector>

#include "thread.h"
using namespace std;

// test for cv.broadcast()
mutex m1;
cv cv1;
bool a2b = false;

void A(void *a) {
    m1.lock();
    intptr_t arg = (intptr_t)a;
    cout << "A\n";
    a2b = true;
    cv1.broadcast();
    m1.unlock();
}

void B(void *a) {
    intptr_t arg = (intptr_t)a;
    m1.lock();
    while (!a2b) {
        cout << "B_Thread " + to_string(arg) + " wait for A\n";
        cv1.wait(m1);
    }
    cout << "B\n";
    m1.unlock();
}
void helper(void *a) {
    thread t1((thread_startfunc_t)B, (void *)0);
    thread t2((thread_startfunc_t)B, (void *)1);
    thread t3((thread_startfunc_t)B, (void *)2);
    thread t4((thread_startfunc_t)A, (void *)0);
}

int main() {
    cpu::boot(1, (thread_startfunc_t)helper, (void *)100, false, false, 0);
}