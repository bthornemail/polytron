/*
 * pair.h — Constitutional Pair Primitive
 *
 * A Pair is a 16-bit word:
 *   bits 15-8 : car (left projection)
 *   bits  7-0 : cdr (right projection)
 *
 * Special atoms (cdr == 0):
 *   nil  = 0x0000   empty / ground
 *   t    = 0x0100   truth
 *   int  = (n<<8)|0  integer n in car
 *
 * Cons pointer (cdr == 2):
 *   ptr  = (addr<<8)|2  memory index in car (0..255)
 */

#ifndef PAIR_H
#define PAIR_H

#include <stdint.h>

typedef uint16_t Pair;

/* Construction */
#define cons(a,d)    ((Pair)(((uint16_t)(a)&0xFF)<<8 | ((uint16_t)(d)&0xFF)))
#define car(p)       (((p)>>8)&0xFF)
#define cdr(p)       ((p)&0xFF)

/* Terminals */
#define NIL          ((Pair)0x0000)
#define PTRUE        ((Pair)0x0100)

/* Predicates */
#define is_nil(p)    ((p)==NIL)
#define is_true(p)   ((p)==PTRUE)
#define is_atom(p)   (cdr(p)==0)
#define is_sym(p)    (cdr(p)==1)
#define is_ptr(p)   (cdr(p)==2)
#define is_int(p)   (cdr(p)==0 && !is_nil(p) && !is_true(p))

/* Value extraction */
#define int_val(p)  (car(p))
#define sym_id(p)   (car(p))
#define ptr_addr(p) (car(p))

/* Constructors */
#define make_int(n)  cons((n)&0xFF, 0)
#define make_sym(id) cons((id)&0xFF, 1)
#define make_ptr(a)  cons((a)&0xFF, 2)

/* Memory: 256 pair slots, 2 Pair per cons (car + cdr) */
#define MEM_SIZE 512
static Pair memory[MEM_SIZE];
static int  free_ptr = 0;

static void mem_init(void) {
    int i;
    for (i = 0; i < MEM_SIZE; i++) memory[i] = NIL;
    free_ptr = 0;
}

/* alloc: store pair at next even slot, return ptr Pair */
static Pair mem_alloc(Pair a, Pair d) {
    if (free_ptr >= MEM_SIZE - 1) return NIL;
    int addr = free_ptr;
    memory[addr]     = a;
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

/* Build a proper list from array */
static Pair pair_list(Pair *items, int count) {
    Pair result = NIL;
    int i;
    for (i = count - 1; i >= 0; i--)
        result = mem_alloc(items[i], result);
    return result;
}

/* Length of proper list */
static int pair_length(Pair list) {
    int n = 0;
    while (is_ptr(list)) { n++; list = mem_cdr(list); }
    return n;
}

#endif