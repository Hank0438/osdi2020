void exception_not_implement(unsigned int offset);
void exception_handler();
void decode_exception();

unsigned long el0_svc_handler(unsigned int x8);
int get_syscall_no();
int sys_get_taskid();
void set_trap_ret();
unsigned long do_get_taskid();

