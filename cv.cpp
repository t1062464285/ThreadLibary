#include "cv.h"
#include "cpu.h"
#include "mutex.h"
#include "thread.h"
#include "thread_impl.h"
#include "ucontext.h"
#include <atomic>
#include <iostream>
#include <queue>
#include <string>

// void context_run() {
// 	while (!ready_queue.empty()) {
// 		cpu::self()->interrupt_enable_suspend();
// 	}
// 	// take a task from the ready queue to cpu and swap context
// 	old_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;
// 	cpu::self()->impl_ptr->cpu_context_ptr = ready_queue.front();
//     ready_queue.pop();
//    	swapcontext(old_context_ptr, cpu::self()->impl_ptr->cpu_context_ptr);
// }

cv::cv() { impl_ptr = new impl(); }

cv::~cv() { delete impl_ptr; }

void cv::wait(mutex &wait_mutex) {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    // Move the cpu context to the cv waiting queue
    ucontext_t *old_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;
    impl_ptr->cv_queue.push(old_context_ptr);

    // if mutex is free
    if ((wait_mutex.impl_ptr->status == STATUS::FREE) ||
        (wait_mutex.impl_ptr->thread_context_ptr == nullptr)) {
        throw runtime_error("Unlock a Free Lock");
    }
    // if trying to unlock a mutex not held by current cpu context
    if (wait_mutex.impl_ptr->thread_context_ptr !=
        cpu::self()->impl_ptr->cpu_context_ptr) {
        throw runtime_error("Current Thread Doesn't Own the Lock");
    }
    // set the mutex status to be free and store the current mutex pointer
    wait_mutex.impl_ptr->status = STATUS::FREE;
    wait_mutex.impl_ptr->thread_context_ptr = nullptr;
    if (!wait_mutex.impl_ptr->mutex_queue.empty()) {
        ucontext_t *pop_context_ptr = wait_mutex.impl_ptr->mutex_queue.front();
        wait_mutex.impl_ptr->mutex_queue.pop();
        ready_queue.push(pop_context_ptr);
        wait_mutex.impl_ptr->status = STATUS::BUSY;
        wait_mutex.impl_ptr->thread_context_ptr = pop_context_ptr;
    }

    // if the ready queue is not empty, get the first context in the queue and
    // run
    if (!ready_queue.empty()) {

        cpu::self()->impl_ptr->cpu_context_ptr = ready_queue.front();
        ready_queue.pop();
        swapcontext(old_context_ptr, cpu::self()->impl_ptr->cpu_context_ptr);

        // // After we get back from swapcontext (cv.signal() called), reaquire
        // the
        // // mutex
        // wait_mutex.impl_ptr->status = STATUS::BUSY;

        // if the ready queue is empty, suspend the cpu
    } else {
        cpu::self()->interrupt_enable_suspend();

        // old_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;

        // //create a new context to swap to
        // //init stack and store the stack pointer to the thread impl class
        // char* stack = new char[STACK_SIZE];
        // ucontext_t* ucontext_ptr = new ucontext_t;

        // ucontext_ptr->uc_stack.ss_sp = stack;
        // ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
        // ucontext_ptr->uc_stack.ss_flags = 0;
        // ucontext_ptr->uc_link = nullptr;
        // impl_ptr->thread_context_ptr = ucontext_ptr;

        // // Initialize the context
        // makecontext(ucontext_ptr, (void (*)())context_run, 0);

        // // reset the cpu context pointer
        // cpu::self()->impl_ptr->cpu_context_ptr = ucontext_ptr;

        // swapcontext(old_context_ptr, cpu::self()->impl_ptr->cpu_context_ptr);

        // //After we get back from swapcontext (cv.signal() called)
        // //delete the context we made and reaquire the mutex
        // delete [] stack;
        // delete ucontext_ptr;

        // wait_mutex->impl_ptr->status = "BUSY";
    }

    cpu::guard = false;
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    wait_mutex.lock();
}

void cv::signal() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    if (!impl_ptr->cv_queue.empty()) {
        ready_queue.push(impl_ptr->cv_queue.front());
        impl_ptr->cv_queue.pop();
    }
    // // Send interrupts to wake up one suspending CPU
    // for (int i = 0; i < cpu_vec.size(); i++) {
    // 	cpu* curr_cpu_ptr = cpu_vec[i];
    // 	if (curr_cpu_ptr->impl_ptr->is_suspended) {
    // 		curr_cpu_ptr->interrupt_send();
    // 	}
    // }
    assert_interrupts_disabled();
    cpu::guard = false;
    cpu::interrupt_enable();
}

void cv::broadcast() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    while (!impl_ptr->cv_queue.empty()) {
        ready_queue.push(impl_ptr->cv_queue.front());
        impl_ptr->cv_queue.pop();
    }
    // // Send interrupts to wake up suspending CPUs
    // for (int i = 0; i < cpu_vec.size(); i++) {
    // 	cpu* curr_cpu_ptr = cpu_vec[i];
    // 	if (curr_cpu_ptr->impl_ptr->is_suspended) {
    // 		curr_cpu_ptr->interrupt_send();
    // 	}
    // }
    assert_interrupts_disabled();
    cpu::guard = false;
    cpu::interrupt_enable();
}