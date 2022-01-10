#include "thread.h"

#include <iostream>
#include <queue>

#include "cpu.h"
#include "thread_impl.h"

using namespace std;

// void thread_context_run() {
//     cpu::interrupt_disable();
//     while (cpu::guard.exchange(true)) {}

//     // If the ready queue is empty, suspend the CPU
//     while (!ready_queue.empty()) {
//         cpu::self()->interrupt_enable_suspend();
//     }

//     // Get interrupts, wake up here
//     // take a task from the ready queue to cpu and swap context
//     old_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;
//     cpu::self()->impl_ptr->cpu_context_ptr = ready_queue.front();
//     ready_queue.pop();
//     swapcontext(old_context_ptr, cpu::self()->impl_ptr->cpu_context_ptr);

//     cpu::guard = false;
//     cpu::interrupt_enable();
// }

void run(thread_startfunc_t func, void *arg, thread::impl *impl_ptr) {
    int id = (intptr_t)arg;
    // cout << "Thread function " + to_string(id) + " starts running\n";
    cpu::guard = false;
    if (func == nullptr) {
        throw runtime_error("Running a Nullptr");
    }
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    func(arg);
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    // cout << "Thread function " + to_string(id) + " finish!!!!!!!!!!!!!!!\n";
    while (!finish_queue.empty()) {
        thread::impl *curr_ptr = finish_queue.front();
        finish_queue.pop();
        delete[] curr_ptr->stack_ptr;
        delete curr_ptr->thread_context_ptr;
        delete curr_ptr;
    }

    // Push all threads on the waiting queue to the ready queue
    while (!impl_ptr->exit_waiting_queue.empty()) {
        ucontext_t *front_context = impl_ptr->exit_waiting_queue.front();
        impl_ptr->exit_waiting_queue.pop();
        ready_queue.push(front_context);
    }

    // cout << "Thread " + to_string(id) +
    //             " push everything on the exit waiting queue to the ready "
    //             "queue\n";

    // deallocation based on whether the class has been destroyed
    impl_ptr->finished = true;

    if (impl_ptr->class_destroyed) {
        // add impl_ptr to the finish queue
        finish_queue.push(impl_ptr);
    }

    // cout << "Thread " + to_string(id) + " deallocation finishes\n";

    // cpu::interrupt_disable(); // To protect ready queue

    // Get the new thread from the ready queue and run
    if (!ready_queue.empty()) {
        ucontext_t *new_context = ready_queue.front();
        ready_queue.pop();
        cpu::self()->impl_ptr->cpu_context_ptr = new_context;
        setcontext(new_context);

        cpu::guard = false;
        assert_interrupts_disabled();
        cpu::interrupt_enable();
        // Suspend CPU if there's nothing to run anymore

    } else {
        cpu::self()->interrupt_enable_suspend();
    }

    // cout << "Thread run function" + to_string(id) + " finish!\n\n";
}

thread::thread(thread_startfunc_t func, void *arg) {
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {}

    // Using makecontext to initialize the new thread context
    int id = (intptr_t)arg;
    // cout << "Call thread constructor " + to_string(id) + "\n";

    try {
        impl_ptr = new impl();
        ucontext_t *ucontext_ptr = new ucontext_t;

        // init stack and store the stack pointer to the thread impl class
        char *stack = new char[STACK_SIZE];
        impl_ptr->stack_ptr = stack;

        ucontext_ptr->uc_stack.ss_sp = stack;
        ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
        ucontext_ptr->uc_stack.ss_flags = 0;
        ucontext_ptr->uc_link = nullptr;
        impl_ptr->thread_context_ptr = ucontext_ptr;

        // Initialize the context
        makecontext(ucontext_ptr, (void (*)())run, 3, func, arg, impl_ptr);
        
        ready_queue.push(ucontext_ptr);
        

        // Initialize the thread as not finished
        impl_ptr->finished = false;
        impl_ptr->class_destroyed = false;
    } catch (bad_alloc &BA) {
        runtime_error("Bad Allocate");
    }
    cpu::guard = false;
    cpu::interrupt_enable();
}

thread::~thread() {
    impl_ptr->class_destroyed = true;
    // cout << "Destructor get called@@@@@@@@@@@@@@@@@" << endl;
    if (impl_ptr->finished) {
        delete[] impl_ptr->stack_ptr;
        delete impl_ptr->thread_context_ptr;
        delete impl_ptr;
    }
}

void thread::yield() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    //clear the finish queue
    while (!finish_queue.empty()) {
        thread::impl *curr_ptr = finish_queue.front();
        finish_queue.pop();
        delete[] curr_ptr->stack_ptr;
        delete curr_ptr->thread_context_ptr;
        delete curr_ptr;
    }

    // if the ready is not empty, switch threads
    if (!ready_queue.empty()) {
        // push the current context on the cpu to the ready queue
        ucontext_t *old_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;
        ready_queue.push(old_context_ptr);

        // // Send interrupts to wake up suspending CPUs
        // for (int i = 0; i < cpu_vec.size(); i++) {
        //     cpu* curr_cpu_ptr = cpu_vec[i];
        //     if (curr_cpu_ptr->impl_ptr->is_suspended) {
        //         curr_cpu_ptr->interrupt_send();
        //     }
        // }

        // reset the cpu context to the first context in the ready queue
        
        cpu::self()->impl_ptr->cpu_context_ptr = ready_queue.front();
        ready_queue.pop();
        // cpu::guard = false;

        // store the current context of this thread and run the new context on
        // the cpu
        // cout << "### swap context occurs, YIELD ###" << endl;
        // cpu::interrupt_send()
        // store the current context of this thread and run the new context on
        // the cpu
        swapcontext(old_context_ptr, cpu::self()->impl_ptr->cpu_context_ptr);
    }
    assert_interrupts_disabled();
    cpu::guard = false;
    cpu::interrupt_enable();
}

void thread::join() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {
    }

    // clear the finish queue
    while (!finish_queue.empty()) {

        thread::impl *curr_ptr = finish_queue.front();
        finish_queue.pop();
        delete[] curr_ptr->stack_ptr;
        delete curr_ptr->thread_context_ptr;
        delete curr_ptr;
    }

    // if the thread is not finished, put the cpu context to the exit waiting
    // queue
    if (!impl_ptr->finished) {
        // Assume in T1 we call T2.join(), T3_ptr on the ready queue
        // push the current context on the cpu to the exit waiting queue(T2)
        impl_ptr->exit_waiting_queue.push(
            cpu::self()->impl_ptr->cpu_context_ptr);
        ucontext_t *old_context_ptr =
            cpu::self()->impl_ptr->cpu_context_ptr; // old context ptr refers
                                                    // to T1_ptr

        // if the ready queue is not empty, get the first context from the ready
        // queue and run
        if (!ready_queue.empty()) {
            // // Send interrupts to wake up suspending CPUs
            // for (int i = 0; i < cpu_vec.size(); i++) {
            //     cpu* curr_cpu_ptr = cpu_vec[i];
            //     if (curr_cpu_ptr->impl_ptr->is_suspended) {
            //         curr_cpu_ptr->interrupt_send();
            //     }
            // }

            cpu::self()->impl_ptr->cpu_context_ptr =
                ready_queue.front(); // update cpu ptr to T3 ptr
            ready_queue.pop();
            cpu::guard = false;

            // store the current context of this thread(T1) and run the new
            // context on the cpu(T3)
            // cout << "### swap context occurs, JOIN ###" << endl;
            // cpu::interrupt_send()

            // store the current context of this thread(T1) and run the new
            // context on the cpu(T3)
            swapcontext(old_context_ptr,
                        cpu::self()->impl_ptr->cpu_context_ptr);

        } else {
            // suspend the CPU
            // I WANT TO SAVE THE OLD CONTEXT HERE, HOW SHOULD I DO THAT
            cpu::guard = false;
            cpu::interrupt_enable_suspend();

            // old_context_ptr = cpu::self()->impl_ptr->cpu_context_ptr;

            // //create a new context to swap to
            // char* stack = new char[STACK_SIZE];
            // ucontext_t* ucontext_ptr = new ucontext_t;

            // ucontext_ptr->uc_stack.ss_sp = stack;
            // ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
            // ucontext_ptr->uc_stack.ss_flags = 0;
            // ucontext_ptr->uc_link = nullptr;
            // impl_ptr->thread_context_ptr = ucontext_ptr;

            // // Initialize the context
            // makecontext(ucontext_ptr, (void (*)())thread_context_run, 0);

            // // reset the cpu context pointer
            // cpu::self()->impl_ptr->cpu_context_ptr = ucontext_ptr;

            // swapcontext(old_context_ptr,
            // cpu::self()->impl_ptr->cpu_context_ptr);

            // //After we get back from swapcontext
            // //delete the context we made and reaquire the mutex
            // delete [] stack;
            // delete ucontext_ptr;
        }
    }
    assert_interrupts_disabled();
    cpu::guard = false;
    cpu::interrupt_enable();
}
