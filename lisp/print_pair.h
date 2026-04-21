/*
 * print_pair.h — Pair Printer
 */

#ifndef PRINT_PAIR_H
#define PRINT_PAIR_H

#include "pair.h"

extern void uart_putc(char c);
extern void uart_puts(const char *s);

static void pp_hex8(unsigned int v) {
    const char *h = "0123456789ABCDEF";
    uart_putc(h[(v>>4)&0xF]);
    uart_putc(h[v&0xF]);
}

static void pp_hex16(unsigned int v) { pp_hex8(v>>8); pp_hex8(v&0xFF); }

static const char * const sym_names[] = {
    "quote","if","lambda","define","car","cdr","cons","nil","t",
    "and","or","xor","not","lsft","rsft"
};
#define SYM_TABLE_LEN 15

static void pp_atom(Pair p) {
    if (is_nil(p))        { uart_puts("nil"); return; }
    if (is_true(p))       { uart_puts("t");   return; }
    if (is_sym(p)) {
        int id = sym_id(p);
        if (id < SYM_TABLE_LEN) uart_puts(sym_names[id]);
        else { uart_puts("sym#"); pp_hex8(id); }
        return;
    }
    if (is_int(p)) {
        unsigned int v = int_val(p);
        char buf[4]; int i = 0;
        if (!v) { uart_putc('0'); return; }
        while (v) { buf[i++] = '0' + (v % 10); v /= 10; }
        while (i--) uart_putc(buf[i]);
        return;
    }
    /* raw pair */
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

#endif