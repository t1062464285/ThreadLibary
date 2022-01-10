
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

// test for cv.signal()

int numCokes = 10;
int MAX = 50;
int iteration = 0;
mutex cokeLock;
cv waitingProducers, waitingConsumers;

int child_done = 0; // global variable; shared between the two threads

void producer(void *a) {
    intptr_t arg = (intptr_t)a;

    while (iteration < 1000) {
        cokeLock.lock();

        while (numCokes == MAX) {
            waitingProducers.wait(cokeLock);
        }
        // add coke to machine
        numCokes++;
        cout << to_string(iteration) + ":\n" + "producer " + to_string(arg) +
                    " adds 1 coke\n" +
                    "Current coke number : " + to_string(numCokes) + "\n\n";
        waitingConsumers.signal();
        iteration += 1;
        thread::yield();
        cokeLock.unlock();
    }
}

void consumer(void *a) {
    intptr_t arg = (intptr_t)a;

    while (iteration < 1000) {
        cokeLock.lock();
        while (numCokes == 0) {
            waitingConsumers.wait(cokeLock);
        }

        // take coke out of machine
        numCokes--;
        cout << to_string(iteration) + ":\n" + "consumer " + to_string(arg) +
                    " takes 1 coke\n" +
                    "Current coke number : " + to_string(numCokes) + "\n\n";
        waitingProducers.signal();
        iteration += 1;
        thread::yield();
        cokeLock.unlock();
    }
}
void helper(void *a) {
    thread t1((thread_startfunc_t)producer, (void *)0);
    thread t2((thread_startfunc_t)consumer, (void *)0);
    thread t3((thread_startfunc_t)consumer, (void *)1);
    thread t4((thread_startfunc_t)consumer, (void *)2);
    thread t5((thread_startfunc_t)producer, (void *)1);
}

int main() {
    cpu::boot(1, (thread_startfunc_t)helper, (void *)100, true, false, 0);
}