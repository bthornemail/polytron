#include "polytron_accelerator.h"

int main(void) {
    PolytronContext ctx = {0x1337133713371337ULL, 0x0000ULL};
    PolytronCoreRegister reg = {0x7F, 0xABCDEFFF, 0};
    uint64_t next_step = execute_polytron_step(&ctx, 0xDEADBEEFDEADBEEFULL);

    return (reg.instruction_opcode == 0x7F
        && reg.clock_cycle_weight == 0
        && next_step == ctx.active_hardware_axis) ? 0 : 1;
}
