// task->state
#define INIT                0
#define RUN_IN_USER_MODE    1
#define IN_OUT_KERNEL       2
#define RUN_IN_KERNEL_MODE  3
#define IRQ_CONTEXT         4
#define CONTEXT_SWITCH      5
#define ZOMBIE              6
#define EXC_CONTEXT         7

/*
 * PSR bits
 */
#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d


// the cpu_context's order must be the same as switch_to
struct cpu_context {
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long fp;
    unsigned long sp;
    unsigned long pc;
} __attribute__ ((aligned (8)));

struct user_context {
    unsigned long sp_el0;   // user stack
    unsigned long elr_el1;  // user pc 
    unsigned long spsr_el1; // user cpu state
} __attribute__ ((aligned (8)));

#define MAX_PROCESS_PAGES			16	

struct user_page {
	unsigned long phys_addr;
	unsigned long virt_addr;
};

struct mm_struct {
	unsigned long pgd;
	int user_pages_count;
	struct user_page user_pages[MAX_PROCESS_PAGES];
	int kernel_pages_count;
	unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

struct task {
    struct cpu_context cpu_context;
    struct user_context user_context;
    struct mm_struct mm;
    long counter;
    long priority;  
    long state;
    unsigned long task_id;
    unsigned long parent_id;
    int reschedule_flag;
    unsigned long trapframe; // only for syscall, eg. fork
    // struct trapframe_regs trapframe_regs;
};

struct task_manager {
    struct task *task_pool[64];
    char kstack_pool_prevent_stack_overflow[4096];
    char kstack_pool[64][4096];
    char ustack_pool[64][4096];
    unsigned long zombie_num;
    unsigned int task_num;
    unsigned long queue_bitmap;
    unsigned long avail_bitmap;
    // struct task*(*current)();
};


struct trapframe_regs {
	unsigned long regs[31];
	unsigned long sp_el0; // sp
	unsigned long elr_el1; // pc
	unsigned long spsr_el1; // pstate
} __attribute__ ((aligned (8)));

struct task* get_current();
void set_current(struct task* task_struct);

void task_manager_init(void(*func)());
int privilege_task_create(void(*func)(), int fork_flag);
struct trapframe_regs* get_task_trapframe(struct task *task);
void context_switch(struct task* next);
void schedule();

void do_exec(unsigned long start, unsigned long size, void(*func)());
int do_fork();
int do_exit(int status);
void zombie_reaper(struct task* check_task);

void kernel_test();
void idle();
void user_test();
void foo();

void final_test_foo();
void final_test(); 
void final_user_test();
void final_idle();

#define N 3
#define CNT 0x10
