#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>
#include <assert.h>

#include "breakpoint.h"



#define offsetof(a ,b) __builtin_offsetof(a, b)
#define get_reg(pid, name) __get_reg(pid, offsetof(struct user, regs.name))


long __get_reg(pid_t pid, int off)
{
long val = 0;
    if((val = ptrace(PTRACE_PEEKUSER, pid, off)) == -1)
        perror("ptace peekuser");

    return val;
}





void bp_set(pid_t pid, breakpoints *bps,  void *addr)
{
long original_data = 0;

    if((original_data = ptrace(PTRACE_PEEKDATA, pid, addr, NULL)) == -1){
        perror("ptrace peekdata");
    }
    if((ptrace(PTRACE_POKEDATA, pid, addr, ((original_data&0xFFFFFFFFFFFFFF00)|0xCC))) == -1){
        perror("ptrace pokedata");
    }

    add_bp_tail(bps, addr, original_data);
}




int breakpoint_handler(pid_t pid, breakpoints *bps, breakpoint *bp)
{
struct user_regs_struct regs;
    if(ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1){
        perror("ptrace getregs");
        return 1;
    }

    if(ptrace(PTRACE_POKEDATA, pid, bp->exception_addr, bp->original_data) == -1){
        perror("ptrace pokedata");
        return 1;
    }

    regs.rip -= 1;

    if(ptrace(PTRACE_SETREGS, pid, NULL, &regs) == -1){
        perror("ptrace setregs");
        return 1;
    }

    bp_del(bps, bp);

    return 0;
}




int wait_for_bp(pid_t pid, breakpoints *bps)
{
int status;
int continue_status = 0;
breakpoint *current = NULL;

    while(1){
        ptrace(PTRACE_CONT, pid, NULL, NULL);
        waitpid(pid, &status, 0);


        if(WIFEXITED(status)){
            printf("exited\n");
            continue_status = 1;
        }

        else if(WIFSTOPPED(status)){
            if(WSTOPSIG(status) == SIGTRAP && (current = get_bp_from_addr(bps, (void *)get_reg(pid, rip))) != NULL){
                continue_status = breakpoint_handler(pid, bps, current);
            }
        }

        else{
            fprintf(stderr, "process stoped by sig = %d\n", WSTOPSIG(status));
        }

        return continue_status;
    }
}




int do_debug(pid_t pid)
{
breakpoints *bps;
int status;

    bps = (breakpoints *)malloc(sizeof(breakpoints));
    assert(bps != NULL);


    if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1){
        perror("ptrace attach");
    }

    waitpid(pid, &status, 0);
    assert(WIFSTOPPED(status));

    while(1){
        if(wait_for_bp(pid, bps) != 0)
            break;

        printf("rip = %lx\n", get_reg(pid, rip));

    }

    return 0;
}




int do_child(int argc, char **argv)
{
char *args[argc+1];

    for(int i=0; i < argc; ++i)
        args[i] = argv[i];

    args[argc] = NULL;

    kill(getpid(), SIGSTOP);

    return execvp(args[0], args);
}




int main(int argc, char **argv)
{
pid_t pid;

    if(argc < 2){
        printf("err\n");
        return 1;
    }

    pid = fork();
    if(pid > 0){
        return do_debug(pid);

    }
    else if(!pid){
        return do_child(argc-1, argv+1);
    }
    else if(pid < 0){
        perror("fork");
        return 1;
    }

    return 0;

}
