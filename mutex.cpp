#include "mutex.h"
#include "cpu.h"
#include "thread.h"
#include "thread_impl.h"
#include "ucontext.h"
#include <atomic>
#include <iostream>
#include <queue>
#include <string>

using namespace std;

mutex::mutex() {
    impl_ptr = new impl();
    // init the mutex status to be free
    impl_ptr->status = STATUS::FREE;
}

mutex::~mutex() { delete impl_ptr; }

void mutex::lock() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    if (impl_ptr->status == STATUS::FREE) {
        // set status to busy and record the current context pointer
        impl_ptr->status = STATUS::BUSY;
        impl_ptr->thread_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;

    } else {
        // push the current context on the cpu to the mutex waiting queue
        ucontext_t *saved_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;
        impl_ptr->mutex_queue.push(saved_context_ptr);

        // if the ready queue is not empty, get the first context in the queue
        // and run
        if (!ready_queue.empty()) {
            cpu::self()->impl_ptr->cpu_context_ptr = ready_queue.front();
            ready_queue.pop();

            cpu::guard = false;
            swapcontext(saved_context_ptr,
                        cpu::self()->impl_ptr->cpu_context_ptr);

            // if the ready queue is empty, suspend the cpu
        } else {
            cpu::self()->interrupt_enable_suspend();
        }
    }
    assert_interrupts_disabled();
    cpu::guard = false;
    cpu::interrupt_enable();
}

void mutex::unlock() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }
    // if mutex is free
    if ((impl_ptr->status == STATUS::FREE) ||
        (impl_ptr->thread_context_ptr == nullptr)) {
        throw runtime_error("Unlock a Free Lock");
    }
    // if trying to unlock a mutex not held by current cpu context
    if (impl_ptr->thread_context_ptr !=
        cpu::self()->impl_ptr->cpu_context_ptr) {
        throw runtime_error("Current Thread Doesn't Own the Lock");
    }
    impl_ptr->status = STATUS::FREE;
    impl_ptr->thread_context_ptr = nullptr;

    if (!impl_ptr->mutex_queue.empty()) {
        ucontext_t *pop_context_ptr = impl_ptr->mutex_queue.front();
        impl_ptr->mutex_queue.pop();
        ready_queue.push(pop_context_ptr);
        impl_ptr->status = STATUS::BUSY;
        impl_ptr->thread_context_ptr = pop_context_ptr;
    }
    assert_interrupts_disabled();
    cpu::guard = false;
    cpu::interrupt_enable();
}