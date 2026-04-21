/*
 * hw/misc/tetra-semantic.c — Tetragrammatron Semantic Processor
 *
 * Meta-circular lexer + parser + interpolator device.
 * Input: bytes → Tokens → Parsed channels → Interpolated SID.
 * Integrates with clock tree for FS/GS/RS/US timing.
 */

#include "qemu/osdep.h"
#include "hw/core/sysbus.h"
#include "hw/core/clock.h"
#include "hw/core/qdev-clock.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qom/object.h"

#define TYPE_TETRA_SEMANTIC "tetra-semantic"
typedef struct TetraSemanticState TetraSemanticState;
DECLARE_INSTANCE_CHECKER(TetraSemanticState, TETRA_SEMANTIC, TYPE_TETRA_SEMANTIC)

#define TOKEN_BUFFER_SIZE  256
#define MAX_TOKENS        64

#define CHANNEL_FS        0x1C
#define CHANNEL_GS        0x1D
#define CHANNEL_RS        0x1E
#define CHANNEL_US        0x1F

#define MODE_XX           0
#define MODE_Xx           1
#define MODE_XX_REV       2
#define MODE_xx           3

struct TetraSemanticState {
    SysBusDevice parent_obj;
    MemoryRegion mmio;
    
    uint8_t mode;
    uint8_t current_channel;
    uint8_t token_count;
    uint32_t sid;
    uint32_t token_buffer[TOKEN_BUFFER_SIZE];
    uint8_t channel_buffer[TOKEN_BUFFER_SIZE];
    
    uint32_t fs_sum;
    uint32_t gs_sum;
    uint32_t rs_sum;
    uint32_t us_sum;
    uint32_t fs_count;
    uint32_t gs_count;
    uint32_t rs_count;
    uint32_t us_count;
    
    Clock *clk_fs;
    Clock *clk_gs;
    Clock *clk_rs;
    Clock *clk_us;
};

static uint32_t rotl32(uint32_t x, int n)
{
    n &= 31;
    return (x << n) | (x >> (32 - n));
}

static uint32_t rotr32(uint32_t x, int n)
{
    n &= 31;
    return (x >> n) | (x << (32 - n));
}

static uint32_t K(uint32_t p, uint32_t C)
{
    return rotl32(p, 1) ^ rotl32(p, 3) ^ rotr32(p, 2) ^ C;
}

static uint32_t swap_bytes_16(uint32_t word)
{
    return ((word & 0x00FF) << 8) | ((word & 0xFF00) >> 8);
}

static uint32_t interpolate_xx(TetraSemanticState *s)
{
    uint32_t seed = s->fs_sum;
    uint32_t result = K(seed, s->gs_sum);
    result = K(result, s->rs_sum);
    result = K(result, s->us_sum);
    return result;
}

static uint32_t interpolate_Xx(TetraSemanticState *s)
{
    uint32_t seed = swap_bytes_16(s->fs_sum);
    uint32_t result = K(seed, s->gs_sum);
    result = K(result, s->rs_sum);
    result = K(result, s->us_sum);
    return result;
}

static uint32_t interpolate_xX(TetraSemanticState *s)
{
    uint32_t seed = s->us_sum;
    uint32_t result = K(seed, s->rs_sum);
    result = K(result, s->gs_sum);
    result = K(result, s->fs_sum);
    return result;
}

static uint32_t interpolate_xx_rev(TetraSemanticState *s)
{
    uint32_t seed = swap_bytes_16(s->us_sum);
    uint32_t result = K(seed, s->rs_sum);
    result = K(result, s->gs_sum);
    result = K(result, s->fs_sum);
    return result;
}

static void compute_sid(TetraSemanticState *s)
{
    switch (s->mode) {
    case MODE_XX:
        s->sid = interpolate_xx(s);
        break;
    case MODE_Xx:
        s->sid = interpolate_Xx(s);
        break;
    case MODE_XX_REV:
        s->sid = interpolate_xX(s);
        break;
    case MODE_xx:
        s->sid = interpolate_xx_rev(s);
        break;
    default:
        s->sid = interpolate_xx(s);
    }
}

static uint64_t tetra_semantic_read(void *opaque, hwaddr addr, unsigned size)
{
    TetraSemanticState *s = TETRA_SEMANTIC(opaque);

    switch (addr) {
    case 0x00:
        return (s->fs_count << 24) | (s->gs_count << 16) | 
               (s->rs_count << 8) | s->us_count;
    case 0x04:
        return s->sid;
    case 0x08:
        return s->mode;
    case 0x0C:
        return s->fs_sum;
    case 0x10:
        return s->gs_sum;
    case 0x14:
        return s->rs_sum;
    case 0x18:
        return s->us_sum;
    case 0x1C:
        return s->current_channel;
    case 0x20:
        return s->token_count;
    default:
        return 0;
    }
}

static void tetra_semantic_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    TetraSemanticState *s = TETRA_SEMANTIC(opaque);

    switch (addr) {
    case 0x00:
        s->mode = val & 0x03;
        break;
    case 0x04:
        s->current_channel = val & 0xFF;
        break;
    case 0x08:
        if (s->token_count < MAX_TOKENS) {
            s->channel_buffer[s->token_count] = s->current_channel;
            s->token_buffer[s->token_count] = val & 0xFFFF;
            
            switch (s->current_channel) {
            case CHANNEL_FS:
                s->fs_sum += val & 0xFFFF;
                s->fs_count++;
                if (s->clk_fs) clock_set_ns(s->clk_fs, 10 + (val & 0xFF) * 10);
                break;
            case CHANNEL_GS:
                s->gs_sum += val & 0xFFFF;
                s->gs_count++;
                if (s->clk_gs) clock_set_ns(s->clk_gs, 10 + (val & 0xFF) * 10);
                break;
            case CHANNEL_RS:
                s->rs_sum += val & 0xFFFF;
                s->rs_count++;
                if (s->clk_rs) clock_set_ns(s->clk_rs, 10 + (val & 0xFF) * 10);
                break;
            case CHANNEL_US:
                s->us_sum += val & 0xFFFF;
                s->us_count++;
                if (s->clk_us) clock_set_ns(s->clk_us, 10 + (val & 0xFF) * 10);
                break;
            }
            
            s->token_count++;
            compute_sid(s);
        }
        break;
    case 0x0C:
        if (val == 1) {
            s->fs_sum = s->gs_sum = s->rs_sum = s->us_sum = 0;
            s->fs_count = s->gs_count = s->rs_count = s->us_count = 0;
            s->token_count = 0;
            s->sid = 0;
        }
        break;
    default:
        break;
    }
}

static const MemoryRegionOps tetra_semantic_ops = {
    .read = tetra_semantic_read,
    .write = tetra_semantic_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void tetra_semantic_realize(DeviceState *dev, Error **errp)
{
    TetraSemanticState *s = TETRA_SEMANTIC(dev);

    memory_region_init_io(&s->mmio, OBJECT(s), &tetra_semantic_ops, s,
                        TYPE_TETRA_SEMANTIC, 256);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->mmio);

    s->clk_fs = qdev_init_clock_out(dev, "FS");
    s->clk_gs = qdev_init_clock_out(dev, "GS");
    s->clk_rs = qdev_init_clock_out(dev, "RS");
    s->clk_us = qdev_init_clock_out(dev, "US");

    clock_set_ns(s->clk_fs, 100);
    clock_set_ns(s->clk_gs, 102);
    clock_set_ns(s->clk_rs, 104);
    clock_set_ns(s->clk_us, 106);

    s->mode = MODE_XX;
    s->current_channel = CHANNEL_GS;
    s->token_count = 0;
    s->sid = 0;
    s->fs_sum = s->gs_sum = s->rs_sum = s->us_sum = 0;
    s->fs_count = s->gs_count = s->rs_count = s->us_count = 0;
}

static void tetra_semantic_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = tetra_semantic_realize;
}

static const TypeInfo tetra_semantic_info = {
    .name = TYPE_TETRA_SEMANTIC,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(TetraSemanticState),
    .class_init = tetra_semantic_class_init,
};

static void tetra_semantic_register_types(void)
{
    type_register_static(&tetra_semantic_info);
}

type_init(tetra_semantic_register_types);
