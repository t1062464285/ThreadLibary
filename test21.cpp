
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

// test for unlocking a not-owned lock
mutex m1;
cv cv1;
void A(void *a) {
    m1.lock();
    thread::yield();
    m1.unlock();
}

void B(void *a) { cv1.wait(m1); }

void helper(void *a) {
    thread t1((thread_startfunc_t)A, (void *)0);
    thread t2((thread_startfunc_t)B, (void *)1);
}

int main() {
    cpu::boot(1, (thread_startfunc_t)helper, (void *)100, false, false, 0);
}