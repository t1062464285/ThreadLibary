
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

// test for unlocking a a free lock
mutex m1;
cv cv1;
void A(void *a) { m1.unlock(); }

void helper(void *a) { thread t1((thread_startfunc_t)A, (void *)0); }

int main() {
    cpu::boot(1, (thread_startfunc_t)helper, (void *)100, false, false, 0);
}