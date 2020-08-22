#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct _breakpoint {
    void *exception_addr;
    long  original_data;
    struct _breakpoint *prev;
    struct _breakpoint *next;
}breakpoint;



typedef struct _breakpoints {
    breakpoint *head;
    breakpoint *tail;
}breakpoints;





void init_bps(breakpoints *bps)
{
    bps -> head = NULL;
    bps -> tail = NULL;
}


void display(breakpoints *bps)
{
breakpoint *current = bps -> head;

    printf("-------------------------------------\n");

    while(current != NULL){
        printf("address = %p\n", current);
        printf("data = %p\n", current -> exception_addr);
        printf("prev = %p\n", current -> prev);
        printf("next = %p\n", current -> next);
        printf("\n");

        current = current -> next;
    }
    printf("-------------------------------------\n");
}




void add_bp_tail(breakpoints *bps, void *addr, long original_data)
{
breakpoint *bp;

    bp = (breakpoint *)malloc(sizeof(breakpoint));
    assert(bp != NULL);

    bp ->exception_addr = addr;
    bp -> next = NULL;
    bp -> original_data = original_data;


    if(bps -> head == NULL){
        bps -> head = bp;
        bp  -> prev = NULL;
    }
    else{
        bps -> tail -> next = bp;
        bp -> prev = bps -> tail;
    }

    bps -> tail = bp;
}




void add_bp_head(breakpoints *bps, void *addr, long original_data)
{
breakpoint *bp;

    bp = (breakpoint *)malloc(sizeof(breakpoint));
    assert(bp != NULL);

    bp -> exception_addr = addr;
    bp -> prev = NULL;
    bp -> original_data = original_data;

    if(bps -> head == NULL){
        bps -> tail = bp;
        bp -> next = NULL;
    }
    else{
        bps -> head -> prev = bp;
        bp -> next = bps -> head;
    }

    bps -> head = bp;
}




void *get_bp_from_addr(breakpoints *bps, void *addr)
{
breakpoint *current;

    current = bps -> head;

    while(current != NULL){
        if(current -> exception_addr == addr)
            return current;

        else
            current = current -> next;
    }
    return NULL;
}





int bp_is_ours(breakpoints *bps, void *addr)
{
    if(get_bp_from_addr(bps, addr) != NULL)
        return 1;

    else
        return 0;
}





void bp_del(breakpoints *bps, breakpoint *bp)
{

    if(bp == NULL)
        return;

    if(bps -> head == bp){
        bp -> next -> prev = NULL;
        bps -> head = bp -> next;
    }
    else if(bps -> tail == bp){
        bp -> prev -> next = NULL;
        bps -> tail = bp -> prev;
    }
    else{
        bp -> prev -> next = bp -> next;
        bp -> next -> prev = bp -> prev;
    }
    free(bp);
}
