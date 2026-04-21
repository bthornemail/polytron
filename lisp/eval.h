/*
 * eval.h — Evaluator on Pairs
 *
 * Builtin symbols (ids 0-14 reserved):
 *   0 quote  1 if  2 lambda  3 define
 *   4 car    5 cdr  6 cons   7 nil    8 t
 *   9 and   10 or  11 xor   12 not   13 lsft  14 rsft
 * User symbols: id >= 15
 */

#ifndef EVAL_H
#define EVAL_H

#include "pair.h"

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

/* Global environment */
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
static Pair p_eval_list(Pair args, Pair env);
static Pair p_apply(Pair fn, Pair args, Pair env);

static Pair p_eval(Pair expr, Pair env) {
    if (is_nil(expr) || is_true(expr)) return expr;
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

#endif