
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

// test for multiple mutex and cv
mutex m_abc, m_def;
cv cv_abc, cv_def;
//if it's def's turn, abc_turn = false
bool abc_turn = true; 
int iteration = 0;
//if abc change from 0->1->2 : Thread A->B->C
int abc = 0;
//int def = 0;


// first group: A->C->B->A->C->B ...
void A(void *a) {
    intptr_t arg = (intptr_t)a;
    m_abc.lock();
    while (iteration <= 30){
        if(abc_turn && (abc == 0)){
            abc_turn = false;
            abc = 2;
            cout << "Thread A   --> C\n";
            iteration += 1;
            cv_def.signal();
            cv_abc.wait(m_abc);
        } else {
            cv_abc.wait(m_abc);
        }
    }
    m_abc.unlock();
}

void B(void *a) {
    intptr_t arg = (intptr_t)a;
    m_abc.lock();
    while (iteration <= 30){
        if(abc_turn && (abc == 1)){
            abc_turn = false;
            abc = 0;
            cout << "Thread B   --> A\n";
            iteration += 1;
            cv_def.signal();
            cv_abc.wait(m_abc);
        } else {
            cv_abc.wait(m_abc);
        }
    }
    m_abc.unlock();
}
void C(void *a) {
    intptr_t arg = (intptr_t)a;
    m_abc.lock();
    while (iteration <= 30){
        if(abc_turn && (abc == 2)){
            abc_turn = false;
            abc = 1;
            cout << "Thread C   --> B\n";
            iteration += 1;
            cv_def.signal();
            cv_abc.wait(m_abc);
        } else {
            cv_abc.wait(m_abc);
        }
    }
    m_abc.unlock();
}

//second group is random
void D(void *a) {
    intptr_t arg = (intptr_t)a;
    m_def.lock();
    while (iteration <= 30){
        if(!abc_turn){
            abc_turn = true;
            cout << "Thread D\n";
            iteration += 1;
            cv_abc.broadcast();
            cv_def.wait(m_def);
        } else {
            cv_def.wait(m_def);
        }
    }
    m_def.unlock();
}
void E(void *a) {
    intptr_t arg = (intptr_t)a;
    m_def.lock();
    while (iteration <= 30){
        if(!abc_turn ){
            abc_turn = true;
            cout << "Thread E\n";
            iteration += 1;
            cv_abc.broadcast();
            cv_def.wait(m_def);
        } else {
            cv_def.wait(m_def);
        }
    }
    m_def.unlock();
}
void F(void *a) {
    intptr_t arg = (intptr_t)a;
    m_def.lock();
    while (iteration <= 30){
        if(!abc_turn){
            abc_turn = true;
            cout << "Thread F\n";
            iteration += 1;
            cv_abc.broadcast();
            cv_def.wait(m_def);
        } else {
            cv_def.wait(m_def);
        }
    }
    m_def.unlock();
}
void helper(void *a) {
    thread t6((thread_startfunc_t)F, (void *)5);
    thread t1((thread_startfunc_t)A, (void *)0);
    thread t3((thread_startfunc_t)C, (void *)2);  
    thread t2((thread_startfunc_t)B, (void *)1);
    thread t5((thread_startfunc_t)E, (void *)4);
    thread t4((thread_startfunc_t)D, (void *)3);
    
}

int main() {
    cpu::boot(1, (thread_startfunc_t)helper, (void *)100, false, false, 0);
}