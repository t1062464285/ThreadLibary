#include "cpu.h"

#include <iostream>

#include "thread.h"
#include "thread_impl.h"
#include "ucontext.h"
using namespace std;

/*
 * cpu::init() initializes a CPU.  It is provided by the thread library
 * and called by the infrastructure.  After a CPU is initialized, it
 * should run user threads as they become available.  If func is not
 * nullptr, cpu::init() also creates a user thread that executes func(arg).
 *
 * On success, cpu::init() should not return to the caller.
 */

queue<ucontext_t *> ready_queue;
queue<thread::impl *> finish_queue;
// vector<cpu*> cpu_vec;

void timer_handler() {
    // cout << "Timer interrupts occured\n";
    thread::yield();
}
// void IPI_handler() {return}

class main_impl {
  public:
    // consist four member variables:
    ucontext_t *thread_context_ptr;
    char *stack_ptr;
    // check whether the defined function for this thread has finished(used by
    // join)
    bool finished;
    bool class_destroyed;
    queue<ucontext_t *> exit_waiting_queue;
};

void run_main(thread_startfunc_t func, void *arg, main_impl *impl_ptr) {
    int id = (intptr_t)arg;
    // cout << "Main Thread function " + to_string(id) + " starts running\n";
    // run the thread function
    if (func == nullptr) {
        throw runtime_error("Running a Nullptr");
    }
    cpu::interrupt_enable();
    cpu::guard = false;
    func(arg);

    // cout << "Main Thread function " + to_string(id) +
    //             " finish!!!!!!!!!!!!!!!\n";

    cpu::interrupt_disable();
    while (cpu::guard.exchange(true)) {}
    // Push all threads on the waiting queue to the ready queue
    while (!impl_ptr->exit_waiting_queue.empty()) {
        ucontext_t *front_context = impl_ptr->exit_waiting_queue.front();
        impl_ptr->exit_waiting_queue.pop();

        ready_queue.push(front_context);
    }
    // cpu::interrupt_enable();

    // cout << "Main Thread " + to_string(id) +
    //             " push everything on the exit waiting queue to the ready "
    //             "queue\n";

    // cpu::interrupt_disable();

    // Get the new thread from the ready queue and run
    if (!ready_queue.empty()) {
        ucontext_t *new_context = ready_queue.front();
        ready_queue.pop();
        cpu::self()->impl_ptr->cpu_context_ptr = new_context;
        setcontext(cpu::self()->impl_ptr->cpu_context_ptr);
    } else {
        cpu::interrupt_enable_suspend();
        assert_interrupts_enabled();
    }
    assert_interrupts_disabled();
    cpu::interrupt_enable();
    
    // cout << "Thread run function" + to_string(id) + " finish!\n\n";
}

void cpu::init(thread_startfunc_t func, void *arg) {
    // put yourself to the cpu queue
    // cpu_vec.push_back(cpu::self());

    interrupt_vector_table[0] = timer_handler;
    // interrupt_vector_table[1] = IPI_handler();

    // init the cpu guard as false
    cpu::guard = false;

    // impl_ptr is the cpu impl pointer
    impl_ptr = new impl();

    if (func) {
        // create the main thread
        // cpu::interrupt_enable();

        // make the context of the main thread
        main_impl *main_impl_ptr = new main_impl();
        ucontext_t *ucontext_ptr = new ucontext_t;

        // init stack and store the stack pointer to the thread impl class
        char *stack = new char[STACK_SIZE];
        main_impl_ptr->stack_ptr = stack;

        ucontext_ptr->uc_stack.ss_sp = stack;
        ucontext_ptr->uc_stack.ss_size = STACK_SIZE;
        ucontext_ptr->uc_stack.ss_flags = 0;
        ucontext_ptr->uc_link = nullptr;
        main_impl_ptr->thread_context_ptr = ucontext_ptr;

        // Initialize the context(don't push the context to the ready queue
        // here)
        makecontext(ucontext_ptr, (void (*)())run_main, 3, func, arg,
                    main_impl_ptr);

        // Initialize the thread as not finished
        main_impl_ptr->finished = false;
        main_impl_ptr->class_destroyed = false;

        // set the cpu_context_ptr to the ucontext of the main thread
        impl_ptr->cpu_context_ptr = main_impl_ptr->thread_context_ptr;

        // run the context of the main thread
        // Q: DO I NEED TO DISABLE HERE? DO I NEED A CPU GUARD HERE?
        setcontext(impl_ptr->cpu_context_ptr);

        delete[] main_impl_ptr->stack_ptr;
        delete main_impl_ptr->thread_context_ptr;
        delete main_impl_ptr;
        // cout << "Delete Main info" << endl;

        // let the cpu sleep
    } else {
        // //CHANGE THIS PART LATER
        //   cpu::self()->impl_ptr->is_suspended() = true;
        //   interrupt_enable_suspend();
        interrupt_enable_suspend();
    }
}
