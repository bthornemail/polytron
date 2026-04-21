/*
 * CONSTITUTIONAL C: THE PAIR AS PRIMITIVE
 *
 * The only structure is the pair.
 * Everything else is sugar over pairs.
 *
 * A pair is two values: (car . cdr)
 * In this machine, a pair is a 16-bit word:
 *   - high 8 bits: car
 *   - low 8 bits:  cdr
 *
 * The kernel function operates on pairs:
 *   K(pair) = rotl(pair,1) ^ rotl(pair,3) ^ rotr(pair,2) ^ C
 *
 * No malloc. No stdlib. Pure pair algebra.
 */

/* --------------------------------------------------------------------------
 * 0. THE PAIR TYPE
 * -------------------------------------------------------------------------- */

/*
 * A Pair is a 16-bit unsigned integer.
 * Bits 15-8: car (left projection)
 * Bits 7-0:  cdr (right projection)
 *
 * This is the ONLY composite type in the system.
 * Integers, symbols, and nil are special pairs.
 */
typedef unsigned short Pair;

/* --------------------------------------------------------------------------
 * 1. PAIR CONSTRUCTION AND PROJECTION (car, cdr, cons)
 * -------------------------------------------------------------------------- */

/*
 * cons(a, d) -> Pair
 * Combine left and right into a single pair.
 * This is the ONLY way to create structure.
 */
#define cons(a, d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))

/*
 * car(p) -> left projection
 * Extract the left half of the pair.
 */
#define car(p) (((p) >> 8) & 0xFF)

/*
 * cdr(p) -> right projection
 * Extract the right half of the pair.
 */
#define cdr(p) ((p) & 0xFF)

/* --------------------------------------------------------------------------
 * 2. SPECIAL PAIRS (the terminal atoms)
 * -------------------------------------------------------------------------- */

/*
 * nil is the empty pair: (0 . 0)
 * It terminates all proper lists.
 * It is the ground of the system.
 */
#define nil cons(0, 0)

/*
 * t is truth: (1 . 0)
 * It is the canonical true value.
 */
#define t cons(1, 0)

/*
 * Predicates for terminal atoms.
 * An atom is a pair whose cdr is 0 (it carries no further structure).
 */
#define is_nil(p)  ((p) == nil)
#define is_t(p)    ((p) == t)
#define is_atom(p) (cdr(p) == 0)

/*
 * Integer predicate: an integer is an atom whose car is the value.
 * We use cdr=0 to mark it as atomic, car holds the 8-bit value.
 */
#define is_int(p)  (is_atom(p) && !is_nil(p) && !is_t(p))
#define int_val(p) (car(p))

/* --------------------------------------------------------------------------
 * 3. THE KERNEL FUNCTION (the pair transformer)
 * -------------------------------------------------------------------------- */

/*
 * rotl(p, n): rotate pair left by n bits.
 * This moves information between car and cdr positions.
 */
Pair rotl(Pair p, int n) {
    n = n & 15;  /* mod 16 for 16-bit pair */
    return (p << n) | (p >> (16 - n));
}

/*
 * rotr(p, n): rotate pair right by n bits.
 */
Pair rotr(Pair p, int n) {
    n = n & 15;
    return (p >> n) | (p << (16 - n));
}

/*
 * K(p, C): the atomic kernel.
 * K(p) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C
 *
 * This is the ONLY transformation in the system.
 * It operates on pairs and produces pairs.
 */
Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}

/* --------------------------------------------------------------------------
 * 4. LIST OPERATIONS (all sugar over cons)
 * -------------------------------------------------------------------------- */

/*
 * Memory for cons cells.
 * A real Lisp would allocate, but for constitutional purity,
 * we pre-allocate a static array of pairs.
 *
 * Each cons cell in memory is just a Pair stored at an address.
 * The address itself becomes the tagged pointer.
 */
#define MEM_SIZE 256
Pair memory[MEM_SIZE];
int free_ptr = 0;

/*
 * alloc_pair(a, d) -> address
 * Store a pair in memory and return its index.
 * The index can be used as a pointer.
 */
int alloc_pair(Pair a, Pair d) {
    int addr = free_ptr;
    memory[addr] = cons(a, d);
    free_ptr++;
    return addr;
}

/*
 * pair_car(addr) -> left of stored pair
 */
Pair pair_car(int addr) {
    return car(memory[addr]);
}

/*
 * pair_cdr(addr) -> right of stored pair
 */
Pair pair_cdr(int addr) {
    return cdr(memory[addr]);
}

/*
 * list2(a, b) -> address of (a . (b . nil))
 * Build a two-element list.
 */
int list2(Pair a, Pair b) {
    int tail = alloc_pair(b, nil);
    return alloc_pair(a, tail);
}

/*
 * list3(a, b, c) -> address of (a . (b . (c . nil)))
 */
int list3(Pair a, Pair b, Pair c) {
    int tail = alloc_pair(c, nil);
    int mid  = alloc_pair(b, tail);
    return alloc_pair(a, mid);
}

/* --------------------------------------------------------------------------
 * 5. SYMBOLS (pairs of (name . value))
 * -------------------------------------------------------------------------- */

/*
 * A symbol is a pair: (symbol_id . 1) where cdr=1 marks it as a symbol.
 * The car is an index into the symbol table.
 */
#define is_sym(p)  (cdr(p) == 1)
#define sym_id(p)  (car(p))

/*
 * Symbol table: an array of name strings.
 * The symbol's car is the index into this table.
 */
const char* symbol_names[] = {
    "quote",    /* id 0 */
    "if",       /* id 1 */
    "lambda",   /* id 2 */
    "define",   /* id 3 */
    "car",      /* id 4 */
    "cdr",      /* id 5 */
    "cons",     /* id 6 */
    "nil",      /* id 7 */
    "t"         /* id 8 */
};

#define SYM_QUOTE  0
#define SYM_IF     1
#define SYM_LAMBDA 2
#define SYM_DEFINE 3
#define SYM_CAR    4
#define SYM_CDR    5
#define SYM_CONS   6
#define SYM_NIL    7
#define SYM_T      8

/*
 * make_sym(id) -> Pair
 * Create a symbol pair from its id.
 */
#define make_sym(id) cons(id, 1)

/* --------------------------------------------------------------------------
 * 6. ENVIRONMENT (alist of pairs)
 * -------------------------------------------------------------------------- */

/*
 * An environment is a list of pairs: ((var . value) ...)
 * Each binding is a pair: (var . value)
 * The environment itself is a list of these pairs.
 */

int env = 0;  /* address of the global environment list */

/*
 * extend(var, val, env_addr) -> new environment address
 * Add a binding (var . val) to the front of the environment.
 */
int extend(Pair var, Pair val, int env_addr) {
    int binding = alloc_pair(var, val);
    return alloc_pair(binding, env_addr);
}

/*
 * lookup(var, env_addr) -> value
 * Search the environment alist for var.
 * Returns nil if not found.
 */
Pair lookup(Pair var, int env_addr) {
    int current = env_addr;
    while (current != nil) {
        int binding = pair_car(current);
        if (pair_car(binding) == var) {
            return pair_cdr(binding);
        }
        current = pair_cdr(current);
    }
    return nil;
}

/* --------------------------------------------------------------------------
 * 7. EVALUATOR (the interpreter)
 * -------------------------------------------------------------------------- */

/*
 * eval(expr_addr, env_addr) -> value
 * The core evaluator. It dispatches on the type of expr.
 *
 * expr is a pair stored at expr_addr.
 * If expr is:
 *   - integer: return it
 *   - symbol:  look it up
 *   - cons:    apply the car to the evaluated cdr
 */
Pair eval(int expr_addr, int env_addr);

/*
 * apply(fn, args_addr, env_addr) -> value
 * Apply a function to arguments.
 */
Pair apply(Pair fn, int args_addr, int env_addr) {
    /* Primitive: car */
    if (fn == make_sym(SYM_CAR)) {
        return pair_car(pair_car(args_addr));
    }
    /* Primitive: cdr */
    if (fn == make_sym(SYM_CDR)) {
        return pair_cdr(pair_car(args_addr));
    }
    /* Primitive: cons */
    if (fn == make_sym(SYM_CONS)) {
        return alloc_pair(pair_car(args_addr),
                          pair_car(pair_cdr(args_addr)));
    }
    return nil;
}

/*
 * eval_list(args_addr, env_addr) -> address of evaluated args list
 * Evaluate each argument in a list and build a new list of results.
 */
int eval_list(int args_addr, int env_addr) {
    if (args_addr == nil) return nil;
    Pair val = eval(pair_car(args_addr), env_addr);
    int rest = eval_list(pair_cdr(args_addr), env_addr);
    return alloc_pair(val, rest);
}

/*
 * eval: the main evaluator
 */
Pair eval(int expr_addr, int env_addr) {
    Pair expr = memory[expr_addr];

    /* Integer: self-evaluating */
    if (is_int(expr)) {
        return expr;
    }

    /* Symbol: lookup in environment */
    if (is_sym(expr)) {
        return lookup(expr, env_addr);
    }

    /* nil and t are self-evaluating */
    if (is_nil(expr) || is_t(expr)) {
        return expr;
    }

    /* Cons cell: special form or application */
    int op_addr = pair_car(expr_addr);
    Pair op = memory[op_addr];

    /* (quote x) */
    if (op == make_sym(SYM_QUOTE)) {
        return pair_car(pair_cdr(expr_addr));
    }

    /* (if test then else) */
    if (op == make_sym(SYM_IF)) {
        int test_addr = pair_car(pair_cdr(expr_addr));
        int then_addr = pair_car(pair_cdr(pair_cdr(expr_addr)));
        int else_addr = pair_car(pair_cdr(pair_cdr(pair_cdr(expr_addr))));

        Pair test_val = eval(test_addr, env_addr);
        if (!is_nil(test_val)) {
            return eval(then_addr, env_addr);
        } else {
            return eval(else_addr, env_addr);
        }
    }

    /* (define var val) */
    if (op == make_sym(SYM_DEFINE)) {
        int var_addr = pair_car(pair_cdr(expr_addr));
        int val_addr = pair_car(pair_cdr(pair_cdr(expr_addr)));
        Pair var = memory[var_addr];
        Pair val = eval(val_addr, env_addr);
        env = extend(var, val, env);
        return var;
    }

    /* Application: (fn arg1 arg2 ...) */
    Pair fn = eval(op_addr, env_addr);
    int args_addr = pair_cdr(expr_addr);
    int evalled_args = eval_list(args_addr, env_addr);
    return apply(fn, evalled_args, env_addr);
}

/* --------------------------------------------------------------------------
 * 8. READER (text to pairs)
 * -------------------------------------------------------------------------- */

/*
 * The reader converts ASCII text into pairs.
 * For constitutional purity, we hardcode a few expressions.
 */

/* Pre-built expressions in memory */
int expr_nil;      /* nil */
int expr_t;        /* t */
int expr_car_nil;  /* (car nil) */

/*
 * init_memory: set up the initial memory with predefined pairs
 */
void init_memory(void) {
    /* Reset allocator */
    free_ptr = 0;

    /* Store nil and t as atoms */
    expr_nil = alloc_pair(nil, nil);
    expr_t   = alloc_pair(t, nil);

    /* Build (car nil) */
    int sym_car = alloc_pair(make_sym(SYM_CAR), nil);
    int args    = alloc_pair(nil, nil);
    expr_car_nil = alloc_pair(sym_car, args);
}

/* --------------------------------------------------------------------------
 * 9. PRINTER (pairs to text)
 * -------------------------------------------------------------------------- */

/*
 * print_pair(addr): convert a stored pair to text and print
 */
void print_pair(int addr) {
    Pair p = memory[addr];

    if (is_nil(p)) {
        printf("nil");
    } else if (is_t(p)) {
        printf("t");
    } else if (is_int(p)) {
        printf("%d", int_val(p));
    } else if (is_sym(p)) {
        printf("%s", symbol_names[sym_id(p)]);
    } else {
        /* Cons cell: print as (car . cdr) or list */
        printf("(");
        print_pair(pair_car(addr));

        int rest = pair_cdr(addr);
        if (rest == nil) {
            /* nothing */
        } else if (is_atom(memory[rest])) {
            printf(" . ");
            print_pair(rest);
        } else {
            /* proper list: print remaining elements */
            while (rest != nil) {
                printf(" ");
                print_pair(pair_car(rest));
                rest = pair_cdr(rest);
            }
        }
        printf(")");
    }
}

/* --------------------------------------------------------------------------
 * 10. REPL (read-eval-print loop)
 * -------------------------------------------------------------------------- */

/*
 * step(pair_addr): evaluate and print
 */
void step(int expr_addr) {
    printf("> ");
    print_pair(expr_addr);
    printf("\n");

    Pair result = eval(expr_addr, env);
    printf("=> ");
    print_pair(result);
    printf("\n\n");
}

/*
 * main: demonstrate the constitutional pair machine
 */
int main(void) {
    init_memory();

    printf("=== CONSTITUTIONAL PAIR MACHINE ===\n");
    printf("The pair is the primitive.\n");
    printf("cons is the machine. nil is the ground.\n\n");

    /* Show basic pairs */
    printf("--- Basic Pairs ---\n");
    printf("nil = ");
    print_pair(expr_nil);
    printf("\n");

    printf("t = ");
    print_pair(expr_t);
    printf("\n");

    /* Create (1 . 2) */
    int pair_1_2 = alloc_pair(cons(1, 0), cons(2, 0));
    printf("(1 . 2) = ");
    print_pair(pair_1_2);
    printf("\n");

    /* Create list (1 2 3) using cons */
    int list_123 = list3(cons(1, 0), cons(2, 0), cons(3, 0));
    printf("(1 2 3) = ");
    print_pair(list_123);
    printf("\n\n");

    /* Demonstrate evaluator */
    printf("--- Evaluator ---\n");

    /* Define x = 42 */
    int sym_x = alloc_pair(make_sym(9), nil);  /* new symbol id 9 */
    int val_42 = alloc_pair(cons(42, 0), nil);
    int def_x = list3(make_sym(SYM_DEFINE), sym_x, val_42);
    step(def_x);

    /* Evaluate (car (1 . 2)) */
    step(expr_car_nil);

    /* Show kernel operation */
    printf("--- Kernel K(pair) ---\n");
    Pair p = cons(0x12, 0x34);  /* pair (0x12 . 0x34) = 0x1234 */
    Pair C = cons(0x1D, 0x00);  /* constant from constitution */
    Pair result = K(p, C);
    printf("K(0x%04X, 0x%04X) = 0x%04X\n", p, C, result);
    printf("car(result) = 0x%02X, cdr(result) = 0x%02X\n", car(result), cdr(result));

    return 0;
}