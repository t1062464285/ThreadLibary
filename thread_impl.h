#ifndef _THREAD_IMPL_H
#define _THREAD_IMPL_H

#include <iostream>
#include <queue>

#include "thread.h"
#include "ucontext.h"

using namespace std;
enum STATUS { FREE, BUSY };

// The ready queue for all threads
extern queue<ucontext_t *> ready_queue;

// After a thread function has finished, push it to the finish queue to
// deallocate its resources
extern queue<thread::impl *> finish_queue;
extern vector<cpu *> cpu_vec;

class thread::impl {
  public:
    // consist four member variables:
    // Deallocated by the thread run function: thread_context_ptr, stack_ptr,
    // exit_waiting_queue
    //
    ucontext_t *get_context_ptr() const { return thread_context_ptr; }

    ucontext_t *thread_context_ptr;
    char *stack_ptr;
    // check whether the defined function for this thread has finished(used by
    // join)
    bool finished;
    bool class_destroyed;
    queue<ucontext_t *> exit_waiting_queue;
};

class cpu::impl {
  public:
    ucontext_t *cpu_context_ptr;
    bool is_suspended = false;
    static void timer_interrupt();
};

class mutex::impl {
  public:
    // The mutex waiting queue for this mutex class
    queue<ucontext_t *> mutex_queue;
    STATUS status;
    ucontext_t *thread_context_ptr;
};

class cv::impl {
  public:
    // The mutex waiting queue for this mutex class
    queue<ucontext_t *> cv_queue;
};

#endif /* _TREAD_IMPL_H */