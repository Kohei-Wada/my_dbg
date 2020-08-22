
typedef struct _breakpoint {
    void *exception_addr;
    long original_data;
    struct _breakpoint *prev;
    struct _breakpoint *next;
}breakpoint;


typedef struct _breakpoints {
    breakpoint *head;
    breakpoint *tail;
}breakpoints;




void init_bps(breakpoints *bps);
void display(breakpoints *bps);
void add_bp_tail(breakpoints *bps, void *addr, long original_data);
void add_bp_head(breakpoints *bps, void *addr, long original_data);
void *get_bp_from_addr(breakpoints *bps, void *addr);
void bp_del(breakpoints *bps, breakpoint *bp);
