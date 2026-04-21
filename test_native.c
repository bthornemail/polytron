/*
 * test_native.c — Native x86_64 test harness for constitutional pair machine
 * Compile: gcc -o test test_native.c && ./test
 * Or run in QEMU: qemu-system-x86_64 -kernel test ...
 */

#include <stdio.h>
#include <stdint.h>

/* UART simulation for native - writes to stdout */
#define UART_BASE 0x10000000UL
static volatile uint8_t * const uart = (volatile uint8_t *)UART_BASE;

static void uart_putc(char c) {
    putchar(c);
}

static void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

static void uart_hex_nibble(unsigned int v) {
    v &= 0xF;
    uart_putc(v < 10 ? '0' + v : 'A' + v - 10);
}

static void uart_hex8(unsigned int v) {
    uart_hex_nibble(v >> 4);
    uart_hex_nibble(v);
}

static void uart_hex16(unsigned int v) {
    uart_hex8(v >> 8);
    uart_hex8(v);
}

static void uart_udec(unsigned int v) {
    char buf[12];
    int i = 0;
    if (v == 0) { uart_putc('0'); return; }
    while (v) { buf[i++] = '0' + (v % 10); v /= 10; }
    while (i--) uart_putc(buf[i]);
}

/* ------------------------------------------------------------------ */
/* pair.h — Constitutional Pair Primitive                                     */
/* ------------------------------------------------------------------ */
typedef uint16_t Pair;

#define cons(a,d)    ((Pair)(((uint16_t)(a)&0xFF)<<8 | ((uint16_t)(d)&0xFF)))
#define car(p)       (((p)>>8)&0xFF)
#define cdr(p)       ((p)&0xFF)

#define NIL          ((Pair)0x0000)
#define PTRUE        ((Pair)0x0100)

#define is_nil(p)    ((p)==NIL)
#define is_true(p)   ((p)==PTRUE)
#define is_atom(p)   (cdr(p)==0)
#define is_sym(p)    (cdr(p)==1)
#define is_ptr(p)    (cdr(p)==2)
#define is_int(p)    (cdr(p)==0 && !is_nil(p) && !is_true(p))

#define int_val(p)   (car(p))
#define sym_id(p)   (car(p))
#define ptr_addr(p)  (car(p))

#define make_int(n)  cons((n)&0xFF, 0)
#define make_sym(id) cons((id)&0xFF, 1)
#define make_ptr(a)  cons((a)&0xFF, 2)

#define MEM_SIZE 512
static Pair memory[MEM_SIZE];
static int  free_ptr = 0;

static void mem_init(void) {
    int i;
    for (i = 0; i < MEM_SIZE; i++) memory[i] = NIL;
    free_ptr = 0;
}

static Pair mem_alloc(Pair a, Pair d) {
    if (free_ptr >= MEM_SIZE - 1) return NIL;
    int addr = free_ptr;
    memory[addr] = a;
    memory[addr + 1] = d;
    free_ptr = addr + 2;
    return make_ptr(addr);
}

static Pair mem_car(Pair p) {
    if (!is_ptr(p)) return NIL;
    int a = ptr_addr(p);
    if (a >= MEM_SIZE) return NIL;
    return memory[a];
}

static Pair mem_cdr(Pair p) {
    if (!is_ptr(p)) return NIL;
    int a = ptr_addr(p);
    if (a + 1 >= MEM_SIZE) return NIL;
    return memory[a + 1];
}

static void mem_setcar(Pair p, Pair v) {
    if (!is_ptr(p)) return;
    int a = ptr_addr(p);
    if (a < MEM_SIZE) memory[a] = v;
}

static void mem_setcdr(Pair p, Pair v) {
    if (!is_ptr(p)) return;
    int a = ptr_addr(p);
    if (a + 1 < MEM_SIZE) memory[a + 1] = v;
}

static Pair pair_list(Pair *items, int count) {
    Pair result = NIL;
    int i;
    for (i = count - 1; i >= 0; i--)
        result = mem_alloc(items[i], result);
    return result;
}

static int pair_length(Pair list) {
    int n = 0;
    while (is_ptr(list)) { n++; list = mem_cdr(list); }
    return n;
}

/* ------------------------------------------------------------------ */
/* kernel.h — The Kernel Function                                       */
/* ------------------------------------------------------------------ */
static inline Pair krotl(Pair p, int n) {
    n &= 15;
    return (Pair)((p << n) | (p >> (16 - n)));
}

static inline Pair krotr(Pair p, int n) {
    n &= 15;
    return (Pair)((p >> n) | (p << (16 - n)));
}

static inline Pair K(Pair p, Pair C) {
    return krotl(p,1) ^ krotl(p,3) ^ krotr(p,2) ^ C;
}

typedef enum { OP_AND=0, OP_OR, OP_XOR, OP_NOT, OP_LSFT, OP_RSFT } Op;

static Op byte_op(unsigned char b) {
    int hi = (b >> 4) & 0xF;
    if (hi <= 1)  return OP_AND;
    if (hi <= 3)  return OP_OR;
    if (hi <= 5)  return OP_XOR;
    if (hi <= 7)  return OP_NOT;
    if (hi <= 11) return OP_LSFT;
    return OP_RSFT;
}

static Pair apply_op(Op op, Pair atom) {
    unsigned int a = car(atom);
    unsigned int d = cdr(atom);
    unsigned int mask = (d == 0) ? 0x7F : 0xFF;
    switch (op) {
        case OP_AND:  a &= 0x1F; break;
        case OP_OR:   a |= 0x01; break;
        case OP_XOR:  a ^= 0x01; break;
        case OP_NOT:  a = (~a) & mask; break;
        case OP_LSFT: a = (a << 1) & mask; break;
        case OP_RSFT: a = a >> 1; break;
    }
    return cons(a, d);
}

static unsigned int brl_hexwt(unsigned char b) {
    static const unsigned int w[8] = {1,2,4,64,16,8,32,128};
    unsigned int r = 0;
    int i;
    for (i = 0; i < 8; i++) if (b & (1u<<i)) r += w[i];
    return r & 0xFF;
}

static Pair byte_to_pair(unsigned char b) {
    int sel = (b >> 6) & 3;
    if (sel == 1) return cons(brl_hexwt(b), 3);
    if (sel == 2) return cons(b % 52, 4);
    return cons(b & 0x7F, 0);
}

static Pair fold_term(Pair term) {
    Pair acc = NIL;
    while (is_ptr(term)) {
        Pair a = mem_car(term);
        acc = (Pair)(acc ^ a);
        term = mem_cdr(term);
    }
    return acc;
}

typedef struct {
    Pair  state;
    Pair  C;
    Pair  term;
    int   tick;
    int   max_len;
} KRewriter;

static void krewriter_init(KRewriter *rw, Pair seed, Pair C, int max_len) {
    rw->state   = seed;
    rw->C       = C;
    rw->term    = NIL;
    rw->tick    = 0;
    rw->max_len = max_len;
}

static Pair krewriter_step(KRewriter *rw) {
    rw->state = K(rw->state, rw->C);
    unsigned char b = (unsigned char)(rw->state & 0xFF);
    Pair atom = byte_to_pair(b);
    Op op = byte_op(b);
    Pair transformed = apply_op(op, atom);
    rw->term = mem_alloc(transformed, rw->term);
    if (rw->max_len > 0 && pair_length(rw->term) > rw->max_len) {
        Pair cur = rw->term;
        int i;
        for (i = 0; i < rw->max_len - 1 && is_ptr(mem_cdr(cur)); i++)
            cur = mem_cdr(cur);
        mem_setcdr(cur, NIL);
    }
    rw->C = fold_term(rw->term);
    rw->tick++;
    return transformed;
}

/* ------------------------------------------------------------------ */
/* print_pair.h — Printer                                                */
/* ------------------------------------------------------------------ */
static const char * const sym_names[] = {
    "quote","if","lambda","define","car","cdr","cons","nil","t",
    "and","or","xor","not","lsft","rsft"
};
#define SYM_TABLE_LEN 15

static void pp_hex8(unsigned int v) {
    uart_hex_nibble(v >> 4);
    uart_hex_nibble(v);
}

static void pp_hex16(unsigned int v) {
    pp_hex8(v >> 8);
    pp_hex8(v);
}

static void pp_atom(Pair p) {
    if (is_nil(p))        { uart_puts("nil"); return; }
    if (p == PTRUE)    { uart_puts("t");   return; }
    if (is_sym(p)) {
        int id = sym_id(p);
        if (id < SYM_TABLE_LEN) uart_puts(sym_names[id]);
        else { uart_puts("sym#"); pp_hex8(id); }
        return;
    }
    if (is_int(p)) {
        unsigned int v = int_val(p);
        char buf[4];
        int i = 0;
        if (!v) { uart_putc('0'); return; }
        while (v) { buf[i++] = '0' + (v % 10); v /= 10; }
        while (i--) uart_putc(buf[i]);
        return;
    }
    uart_puts("0x"); pp_hex16(p);
}

static void pp_pair(Pair p, int dot_mode, int depth);

static void pp_list_body(Pair p, int dot_mode, int depth) {
    while (is_ptr(p)) {
        pp_pair(mem_car(p), dot_mode, depth+1);
        p = mem_cdr(p);
        if (is_ptr(p)) {
            uart_putc(' ');
            if (dot_mode) uart_puts(". ");
        } else if (!is_nil(p)) {
            uart_puts(" . ");
            pp_atom(p);
        }
    }
}

static void pp_pair(Pair p, int dot_mode, int depth) {
    if (depth > 12) { uart_puts("..."); return; }
    if (!is_ptr(p)) { pp_atom(p); return; }
    uart_putc('(');
    if (dot_mode) {
        pp_pair(mem_car(p), dot_mode, depth+1);
        uart_puts(" . ");
        pp_pair(mem_cdr(p), dot_mode, depth+1);
    } else {
        pp_list_body(p, 0, depth);
    }
    uart_putc(')');
}

static void pp_term(Pair p, int dot_mode) {
    pp_pair(p, dot_mode, 0);
    uart_putc('\n');
}

/* ------------------------------------------------------------------ */
/* eval.h — Evaluator                                                 */
/* ------------------------------------------------------------------ */
#define SYM_QUOTE   0
#define SYM_IF      1
#define SYM_LAMBDA  2
#define SYM_DEFINE  3
#define SYM_CAR     4
#define SYM_CDR     5
#define SYM_CONS    6
#define SYM_NIL     7
#define SYM_T       8
#define SYM_AND     9
#define SYM_OR     10
#define SYM_XOR    11
#define SYM_NOT    12
#define SYM_LSFT   13
#define SYM_RSFT   14

static Pair g_env = NIL;

static void env_define(Pair sym, Pair val) {
    Pair binding = mem_alloc(sym, val);
    g_env = mem_alloc(binding, g_env);
}

static Pair env_lookup(Pair sym, Pair env) {
    while (is_ptr(env)) {
        Pair kv = mem_car(env);
        if (is_ptr(kv) && mem_car(kv) == sym)
            return mem_cdr(kv);
        env = mem_cdr(env);
    }
    return NIL;
}

static Pair p_eval(Pair expr, Pair env);

static Pair p_eval_list(Pair args, Pair env) {
    if (is_nil(args) || !is_ptr(args)) return NIL;
    Pair v = p_eval(mem_car(args), env);
    Pair rest = p_eval_list(mem_cdr(args), env);
    return mem_alloc(v, rest);
}

static Pair p_apply(Pair fn, Pair args, Pair env) {
    if (fn == make_sym(SYM_CAR))
        return mem_car(mem_car(args));
    if (fn == make_sym(SYM_CDR))
        return mem_cdr(mem_car(args));
    if (fn == make_sym(SYM_CONS)) {
        Pair a = mem_car(args);
        Pair d = mem_car(mem_cdr(args));
        return mem_alloc(a, d);
    }
    if (fn == make_sym(SYM_AND)) {
        Pair a = mem_car(args), b = mem_car(mem_cdr(args));
        return cons(car(a) & car(b), car(a) == car(b) ? cdr(a) : 0);
    }
    if (fn == make_sym(SYM_OR)) {
        Pair a = mem_car(args), b = mem_car(mem_cdr(args));
        return cons(car(a) | car(b), cdr(a));
    }
    if (fn == make_sym(SYM_XOR)) {
        Pair a = mem_car(args), b = mem_car(mem_cdr(args));
        return cons(car(a) ^ car(b), 0);
    }
    if (fn == make_sym(SYM_NOT)) {
        Pair a = mem_car(args);
        return cons((~car(a)) & 0xFF, cdr(a));
    }
    if (fn == make_sym(SYM_LSFT)) {
        Pair a = mem_car(args);
        return cons((car(a) << 1) & 0xFF, cdr(a));
    }
    if (fn == make_sym(SYM_RSFT)) {
        Pair a = mem_car(args);
        return cons(car(a) >> 1, cdr(a));
    }
    return NIL;
}

static Pair p_eval(Pair expr, Pair env) {
    if (is_nil(expr) || expr == PTRUE) return expr;
    if (is_int(expr)) return expr;

    if (is_sym(expr)) {
        Pair v = env_lookup(expr, env);
        return is_nil(v) ? expr : v;
    }

    if (!is_ptr(expr)) return NIL;

    Pair op = mem_car(expr);
    Pair rest = mem_cdr(expr);

    if (op == make_sym(SYM_QUOTE))
        return mem_car(rest);

    if (op == make_sym(SYM_IF)) {
        Pair test_val = p_eval(mem_car(rest), env);
        rest = mem_cdr(rest);
        Pair then_e = mem_car(rest);
        Pair else_e = mem_car(mem_cdr(rest));
        return p_eval(is_nil(test_val) ? else_e : then_e, env);
    }

    if (op == make_sym(SYM_DEFINE)) {
        Pair sym = mem_car(rest);
        Pair val = p_eval(mem_car(mem_cdr(rest)), env);
        env_define(sym, val);
        g_env = env;
        return sym;
    }

    Pair fn = p_eval(op, env);
    Pair args = p_eval_list(rest, env);
    return p_apply(fn, args, env);
}

/* ------------------------------------------------------------------ */
/* Main                                                               */
/* ------------------------------------------------------------------ */
#ifndef STAGE
#define STAGE 3
#endif

static KRewriter rw;

void main(void) {
    mem_init();

#if STAGE == 1
    uart_puts("HELLO\n");
    while (1);

#elif STAGE == 2
    uart_puts("nil  = 0x"); uart_hex16(NIL); uart_putc('\n');
    uart_puts("t    = 0x"); uart_hex16(PTRUE); uart_putc('\n');

    Pair p12 = cons(1, 2);
    uart_puts("cons(1,2)   = 0x"); uart_hex16(p12); uart_putc('\n');
    uart_puts("car(...)    = "); uart_udec(car(p12)); uart_putc('\n');
    uart_puts("cdr(...)    = "); uart_udec(cdr(p12)); uart_putc('\n');

    Pair p = cons(0x12, 0x34);
    Pair C = cons(0x1D, 0x00);
    Pair r = K(p, C);
    uart_puts("K(0x1234, 0x1D00) = 0x"); uart_hex16(r); uart_putc('\n');

#elif STAGE == 3
    krewriter_init(&rw, cons(0, 1), cons(0, 0x49), 8);

    uart_puts("tick  state   C      atom  term\n");
    int i;
    for (i = 0; i < 24; i++) {
        Pair atom = krewriter_step(&rw);
        uart_udec(rw.tick); uart_puts("  ");
        uart_hex16(rw.state); uart_puts("  ");
        uart_hex16(rw.C); uart_puts("  ");
        pp_atom(atom); uart_puts("  ");
        pp_term(rw.term, 0);
    }

#elif STAGE == 4
    krewriter_init(&rw, cons(0, 1), cons(0, 0x49), 16);
    int i;
    for (i = 0; i < 3; i++) krewriter_step(&rw);
    Pair key0 = rw.term;
    for (i = 0; i < 3; i++) krewriter_step(&rw);

    Pair sym_x = make_sym(15);
    env_define(sym_x, key0);
    uart_puts("x = "); pp_atom(key0); uart_putc('\n');

    Pair and_args = mem_alloc(sym_x, mem_alloc(sym_x, NIL));
    Pair and_expr = mem_alloc(make_sym(SYM_AND), and_args);
    uart_puts("(and x x) = "); pp_atom(p_eval(and_expr, g_env)); uart_putc('\n');

    Pair xor_args = mem_alloc(sym_x, mem_alloc(sym_x, NIL));
    Pair xor_expr = mem_alloc(make_sym(SYM_XOR), xor_args);
    uart_puts("(xor x x) = "); pp_atom(p_eval(xor_expr, g_env)); uart_putc('\n');

    uart_puts("C = 0x"); uart_hex16(rw.C); uart_putc('\n');

#endif
}

int main_wrapper(int argc, char **argv) {
    main();
    return 0;
}