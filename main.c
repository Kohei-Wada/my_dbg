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





int wait_for_bp(pid_t pid, breakpoints *bps)
{
int status;

    while(1){
        ptrace(PTRACE_CONT, pid, NULL, NULL);
        waitpid(pid, &status, 0);


        if(WIFEXITED(status)){
            printf("exited\n");
            return 1;
        }

        else if(WIFSTOPPED(status)){
            if(WSTOPSIG(status) == SIGTRAP && bp_is_ours(bps, (void *)get_reg(pid, rip)))
                printf("our breakpoint\n");

            return 0;
        }

        else{
            fprintf(stderr, "process stoped by sig = %d\n", WSTOPSIG(status));
            return 0;

        }
    }
}



void single_step(pid_t pid)
{
    if(ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL) == -1){
        perror("single step");
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
