#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <errno.h>




long get_regs(pid_t pid)
{
struct user_regs_struct regs;

    if(ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1)
        perror("ptrace getregs");

    else
        return regs.rip;

    return 0;
}




long bp_set(pid_t pid, void *addr)
{
long original_data = 0;
long __original_data = 0;

    if((original_data = ptrace(PTRACE_PEEKDATA, pid, addr, NULL)) == -1){
        perror("ptrace peekdata");
    }
    printf("original_data = %lx\n", original_data);

    if(ptrace(PTRACE_POKEDATA, pid, addr, ((original_data & 0xFFFFFFFFFFFFFF00) | 0xCC)) == -1){
        perror("ptrace pokedata");
    }

    if((__original_data = ptrace(PTRACE_PEEKDATA, pid, addr, NULL)) == -1){
        perror("ptrace peekdata");
    }
    printf("original_data = %lx\n\n", __original_data);

    return original_data;
}



void single_step(pid_t pid)
{
struct user_regs_struct regs;

    if(ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1){
        perror("ptrace getregs");
    }

    else{
        regs.rip -= 1;
        if(ptrace(PTRACE_SETREGS, pid, NULL, &regs) == -1){
            perror("ptrace setregs");
        }
    }
}






int wait_for_bp(pid_t pid)
{
int status;

    while(1){
        ptrace(PTRACE_CONT, pid, NULL, NULL);

        waitpid(pid, &status, 0);

        if(WIFEXITED(status)){
            printf("exited\n");
            return 1;
        }
        if(WIFSTOPPED(status)){

        }
        else
            return 0;
    }
}



int do_trace(pid_t pid)
{
int status;
long original_data;


void *addr = (void *)0x557181eed1c9;

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    waitpid(pid, &status, 0);

    assert(WIFSTOPPED(status));

    while (1){
        if(wait_for_bp(pid) != 0)
            break;
    }
    return 0;
}




int do_child(int argc, char **argv)
{
char *args[argc+1];

    for(int i = 0; i < argc; ++i)
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
        do_trace(pid);
    }
    else if(!pid){
        do_child(argc-1, argv+1);
    }
    else if(pid < 0){
        perror("fork");
    }

    return 0;
}
