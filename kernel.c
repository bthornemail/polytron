/*
 * TETRAGRAMMATRON CORE — Constitutional Implementation
 *
 * The pair is the primitive.
 * The kernel is the transformer.
 * The s-bit is the structural sign.
 *
 * Every transaction is a binary quadratic form.
 * The invariant is the discriminant.
 *
 * COMPILE: riscv64-linux-gnu-gcc -march=rv64imac -mabi=lp64 
 *          -ffreestanding -nostdlib -no-pie -O2 -Wall
 *          -DSTAGE=3 -T linker.ld -o kernel.elf start.S kernel.c
 *
 * RUN: qemu-system-riscv64 -machine virt -cpu rv64 -bios none 
 *                        -nographic -kernel kernel.elf
 */

#ifndef STAGE
#define STAGE 3
#endif

/* -------------------------------------------------------------------------- */
/* 0. UART (QEMU virt NS16550A at 0x10000000)                                 */
/* -------------------------------------------------------------------------- */
#define UART0_BASE 0x10000000UL
static volatile unsigned char * const uart = (volatile unsigned char *)UART0_BASE;

void uart_putc(char c) {
    while (!(uart[5] & 0x20));
    uart[0] = (unsigned char)c;
}

void uart_puts(const char *s) { while (*s) uart_putc(*s++); }

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
    char buf[12]; int i = 0;
    if (v == 0) { uart_putc('0'); return; }
    while (v) { buf[i++] = '0' + (v % 10); v /= 10; }
    while (i--) uart_putc(buf[i]);
}

/* -------------------------------------------------------------------------- */
/* 1. THE PAIR (the only structure)                                           */
/* -------------------------------------------------------------------------- */
/*
 * A Pair is a 16-bit word.
 * Bits 15-8: car (left projection)
 * Bits 7-0:  cdr (right projection)
 *
 * This is the ONLY composite type in the system.
 */
typedef unsigned short Pair;

#define cons(a,d) ((Pair)(((a)&0xFF)<<8 | ((d)&0xFF)))
#define car(p)   (((p)>>8)&0xFF)
#define cdr(p)  ((p)&0xFF)

/* Terminal atoms */
#define NIL  ((Pair)0x0000)
#define T    ((Pair)0x0100)

#define is_nil(p) ((p)==NIL)
#define is_t(p)  ((p)==T)
#define is_atom(p) (cdr(p)==0)

/* -------------------------------------------------------------------------- */
/* 2. THE KERNEL (the only transformation)                                      */
/* -------------------------------------------------------------------------- */
/*
 * rotl(p, n): rotate pair left by n bits
 * rotr(p, n): rotate pair right by n bits
 *
 * K(p, C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C
 *
 * This is the atomic kernel. It operates on pairs and produces pairs.
 */
static inline Pair rotl(Pair x, int n) {
    n &= 15;
    return (Pair)((x << n) | (x >> (16 - n)));
}

static inline Pair rotr(Pair x, int n) {
    n &= 15;
    return (Pair)((x >> n) | (x << (16 - n)));
}

static Pair K(Pair p, Pair C) {
    return rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C;
}

/* -------------------------------------------------------------------------- */
/* 3. THE FOUR GENERATORS (order 2)                                           */
/* -------------------------------------------------------------------------- */
/*
 * Each generator is an involution: applying it twice returns the original.
 * They correspond to the four planes:
 *   p: Binary  (base 2)
 *   q: Decimal (base 10)
 *   r: Hex     (base 16)
 *   s: Sign    (structural s-bit)
 */

/* p: Binary plane — swap nibbles */
static Pair generator_p(Pair x) {
    return cons(cdr(x), car(x));
}

/* q: Decimal plane — reverse byte order within pair */
static Pair generator_q(Pair x) {
    unsigned char h = car(x);
    unsigned char l = cdr(x);
    unsigned char h_rev = ((h & 0x0F) << 4) | ((h & 0xF0) >> 4);
    unsigned char l_rev = ((l & 0x0F) << 4) | ((l & 0xF0) >> 4);
    return cons(l_rev, h_rev);
}

/* r: Hex plane — rotate the pair 8 bits */
static Pair generator_r(Pair x) {
    return cons(cdr(x), car(x));
}

/* s: Sign plane — flip the structural sign bit (bit 15) */
static Pair generator_s(Pair x) {
    return x ^ 0x8000;
}

/* -------------------------------------------------------------------------- */
/* 4. BINARY QUADRATIC FORMS (the mixers)                                      */
/* -------------------------------------------------------------------------- */
/*
 * A binary quadratic form: M(x, y) = a*x² + b*xy + c*y²
 *
 * In our pair algebra, multiplication is the kernel K.
 * The coefficients a, b, c are determined by the COBS interval.
 *
 * The discriminant: N = b² - 4ac is the invariant.
 */

typedef struct {
    Pair a, b, c;
    Pair discriminant;
} QuadraticForm;

static Pair discriminant_of(Pair a, Pair b, Pair c) {
    Pair b_sq = K(b, b);
    Pair ac = K(a, c);
    Pair ac4 = K(ac, cons(4, 0));
    return b_sq ^ ac4;
}

static Pair apply_form(QuadraticForm *f, Pair x, Pair y) {
    Pair axx = K(f->a, K(x, x));
    Pair bxy = K(f->b, K(x, y));
    Pair cyy = K(f->c, K(y, y));
    return axx ^ bxy ^ cyy;
}

/* -------------------------------------------------------------------------- */
/* 5. COBS FRAMING (the switching unit)                                       */
/* -------------------------------------------------------------------------- */
/*
 * A COBS frame is a switching unit: [0x00][payload with constant s-bit][0x00]
 * The s-bit is constant within the payload.
 */

#define COBS_DELIMITER 0x00
#define MAX_FRAME 256

typedef struct {
    unsigned char data[MAX_FRAME];
    unsigned int length;
    int s_bit;
} COBSFrame;

static COBSFrame cobs_encode(Pair value, int s_bit) {
    COBSFrame frame;
    frame.length = 0;
    frame.s_bit = s_bit;
    unsigned char byte0 = car(value);
    unsigned char byte1 = cdr(value);

    frame.data[frame.length++] = COBS_DELIMITER;

    unsigned char b0 = s_bit ? (byte0 | 0x80) : (byte0 & 0x7F);
    if (b0 == COBS_DELIMITER) frame.data[frame.length++] = 0x01;
    frame.data[frame.length++] = b0;

    unsigned char b1 = s_bit ? (byte1 | 0x80) : (byte1 & 0x7F);
    if (b1 == COBS_DELIMITER) frame.data[frame.length++] = 0x01;
    frame.data[frame.length++] = b1;

    frame.data[frame.length++] = COBS_DELIMITER;
    return frame;
}

/* -------------------------------------------------------------------------- */
/* 6. CONTEXT INVARIANCE TEST                                              */
/* -------------------------------------------------------------------------- */
/*
 * A computation is context-invariant if:
 *   SID(C(v, s=0)) == SID(C(v, s=1))
 *
 * That is, running the same value with s=0 and s=1 produces the same SID.
 * The SID is the kernel applied to the pair with constant C=0x1D.
 */

static Pair compute_sid(Pair nf) {
    return K(nf, 0x1D);
}

static int is_context_invariant(Pair value) {
    COBSFrame frame0 = cobs_encode(value, 0);
    Pair sid0 = compute_sid(cons(frame0.length, frame0.s_bit));

    COBSFrame frame1 = cobs_encode(value, 1);
    Pair sid1 = compute_sid(cons(frame1.length, frame1.s_bit));

    return sid0 == sid1;
}

/* -------------------------------------------------------------------------- */
/* 7. HADAMARD BRIDGE (HexString as universal translator)            */
/* -------------------------------------------------------------------------- */
/*
 * The four planes form a Hadamard complex.
 * HexString is the bridge: any value can be converted to hex and back.
 */

static void to_hexstring(Pair value, char *out) {
    out[0] = "0123456789ABCDEF"[car(value) >> 4];
    out[1] = "0123456789ABCDEF"[car(value) & 0x0F];
    out[2] = "0123456789ABCDEF"[cdr(value) >> 4];
    out[3] = "0123456789ABCDEF"[cdr(value) & 0x0F];
    out[4] = '\0';
}

/* -------------------------------------------------------------------------- */
/* 8. DEMONSTRATION                                                    */
/* -------------------------------------------------------------------------- */

void main(void) {

#if STAGE == 1
    uart_puts("HELLO\n");
    while (1);

#elif STAGE == 2
    uart_puts("=== TETRAGRAMMATRON CORE ===\n");
    uart_puts("The pair is the primitive.\n");
    uart_puts("The kernel is the transformer.\n");
    uart_puts("The s-bit is the structural sign.\n\n");

    Pair value = cons(0x12, 0x34);
    char hex[5];
    to_hexstring(value, hex);
    uart_puts("Value: 0x"); uart_puts(hex);
    uart_puts(" (car="); uart_hex8(car(value)); uart_puts(" cdr="); uart_hex8(cdr(value)); uart_puts(")\n");

    uart_puts("\n--- Generators (order 2) ---\n");

    Pair p_val = generator_p(value);
    to_hexstring(p_val, hex);
    uart_puts("p (binary):  0x"); uart_puts(hex); uart_putc('\n');

    Pair q_val = generator_q(value);
    to_hexstring(q_val, hex);
    uart_puts("q (decimal): 0x"); uart_puts(hex); uart_putc('\n');

    Pair s_val = generator_s(value);
    to_hexstring(s_val, hex);
    uart_puts("s (sign):   0x"); uart_puts(hex); uart_putc('\n');

    uart_puts("\n--- Involution Check ---\n");
    Pair s_twice = generator_s(s_val);
    uart_puts("s(s(x)) == x: ");
    uart_puts(s_twice == value ? "true" : "false");
    uart_putc('\n');

    while (1);

#elif STAGE == 3
    uart_puts("=== TETRAGRAMMATRON CORE ===\n");
    uart_puts("The pair is the primitive.\n");
    uart_puts("The kernel is the transformer.\n");
    uart_puts("The s-bit is the structural sign.\n\n");

    Pair value = cons(0x12, 0x34);
    char hex[5];
    to_hexstring(value, hex);
    uart_puts("Value: 0x"); uart_puts(hex);
    uart_puts(" (car="); uart_hex8(car(value)); uart_puts(" cdr="); uart_hex8(cdr(value)); uart_puts(")\n");

    uart_puts("\n--- Generators (order 2) ---\n");

    Pair p_val = generator_p(value);
    to_hexstring(p_val, hex);
    uart_puts("p (binary):  0x"); uart_puts(hex); uart_putc('\n');

    Pair q_val = generator_q(value);
    to_hexstring(q_val, hex);
    uart_puts("q (decimal): 0x"); uart_puts(hex); uart_putc('\n');

    Pair r_val = generator_r(value);
    to_hexstring(r_val, hex);
    uart_puts("r (hex):    0x"); uart_puts(hex); uart_putc('\n');

    Pair s_val = generator_s(value);
    to_hexstring(s_val, hex);
    uart_puts("s (sign):   0x"); uart_puts(hex); uart_putc('\n');

    uart_puts("\n--- Involution Check ---\n");
    Pair s_twice = generator_s(s_val);
    uart_puts("s(s(x)) == x: ");
    uart_puts(s_twice == value ? "true" : "false");
    uart_putc('\n');

    uart_puts("\n--- Binary Quadratic Form ---\n");
    QuadraticForm form = {
        .a = cons(1, 0),
        .b = cons(2, 0),
        .c = cons(3, 0),
        .discriminant = 0
    };
    form.discriminant = discriminant_of(form.a, form.b, form.c);
    uart_puts("Discriminant (b^2 - 4ac): 0x");
    uart_hex16(form.discriminant);
    uart_putc('\n');

    uart_puts("\n--- Context Invariance ---\n");
    int invariant = is_context_invariant(value);
    uart_puts("Context invariant: ");
    uart_puts(invariant ? "true" : "false");
    uart_putc('\n');

    uart_puts("\n--- Kernel K(p, C) Demo ---\n");
    Pair p = cons(0x12, 0x34);
    Pair C = cons(0x1D, 0x00);
    Pair result = K(p, C);
    uart_puts("K(0x1234, 0x1D00) = 0x");
    uart_hex16(result);
    uart_putc('\n');
    uart_puts("car(result) = 0x"); uart_hex8(car(result));
    uart_puts(" cdr(result) = 0x"); uart_hex8(cdr(result));
    uart_putc('\n');

    while (1);

#elif STAGE == 4
    /* Full demo with evaluator */
    uart_puts("=== FULL TETRAGRAMMATRON ===\n");
    while (1);
#endif
}