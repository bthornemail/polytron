/*
 * hw/misc/tetra-ctrl.c — Tetragrammatron virt-ctrl Device
 *
 * MMIO device for constitutional computing control.
 * Integrates with clock tree for FS/GS/RS/US.
 */

#include "qemu/osdep.h"
#include "hw/core/sysbus.h"
#include "hw/core/clock.h"
#include "hw/core/qdev-clock.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qom/object.h"

#define TYPE_TETRA_CTRL "tetra-ctrl"
typedef struct TetraCtrlState TetraCtrlState;
DECLARE_INSTANCE_CHECKER(TetraCtrlState, TETRA_CTRL, TYPE_TETRA_CTRL)

#define GNOMON_PERIOD       8

struct TetraCtrlState {
    SysBusDevice parent_obj;
    MemoryRegion mmio;
    uint32_t state;
    uint32_t constant;
    uint32_t sid;
    uint32_t gnomon_step;
    uint32_t omicron_mode;
    uint32_t channel;
    uint32_t fb_offset;
    uint32_t period;
    uint32_t cycle_pos;
    uint32_t history[GNOMON_PERIOD];
    int history_idx;
    
    void *fb_ptr;
    size_t fb_size;
    
    Clock *clk_fs;
    Clock *clk_gs;
    Clock *clk_rs;
    Clock *clk_us;
};

#define TETRA_FEATURE_POWER_CTRL   (1 << 0)
#define TETRA_FEATURE_GNOMON       (1 << 1)
#define TETRA_FEATURE_OMICRON      (1 << 2)
#define TETRA_FEATURE_CHANNEL      (1 << 3)
#define TETRA_FEATURE_FRAMEBUF     (1 << 4)

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

static uint32_t tetra_kernel(uint32_t p, uint32_t C)
{
    return rotl32(p, 1) ^ rotl32(p, 3) ^ rotr32(p, 2) ^ C;
}

static uint64_t tetra_ctrl_read(void *opaque, hwaddr addr, unsigned size)
{
    TetraCtrlState *s = TETRA_CTRL(opaque);

    switch (addr) {
    case 0x00:
        return TETRA_FEATURE_POWER_CTRL | TETRA_FEATURE_GNOMON |
               TETRA_FEATURE_OMICRON | TETRA_FEATURE_CHANNEL |
               TETRA_FEATURE_FRAMEBUF;
    case 0x08:
        return s->state;
    case 0x0C:
        return s->sid;
    case 0x18:
        return s->channel;
    case 0x1C:
        return s->fb_offset;
    case 0x24:
        return s->period;
    case 0x28:
        return s->cycle_pos;
    default:
        return 0;
    }
}

static void tetra_ctrl_write(void *opaque, hwaddr addr, uint64_t val, unsigned size)
{
    TetraCtrlState *s = TETRA_CTRL(opaque);

    switch (addr) {
    case 0x04:
        if (val == 1) {
            s->state = 0x4242;
            s->sid = tetra_kernel(s->state, 0x1D);
            s->gnomon_step = 0;
            s->history_idx = 0;
            memset(s->history, 0, sizeof(s->history));
        } else if (val == 4) {
            s->state = tetra_kernel(s->state, s->constant);
            s->sid = tetra_kernel(s->state, 0x1D);
            s->gnomon_step++;
            
            if (s->clk_fs) {
                uint32_t period = 10 + ((s->state & 0xFF) * 10);
                clock_set_ns(s->clk_fs, period);
                clock_set_ns(s->clk_gs, period + 2);
                clock_set_ns(s->clk_rs, period + 4);
                clock_set_ns(s->clk_us, period + 6);
            }
            
            if (s->fb_ptr && s->fb_offset < s->fb_size) {
                uint32_t *fb = (uint32_t *)s->fb_ptr;
                fb[s->fb_offset / 4] = s->sid;
            }
        } else if (val == 5) {
            s->omicron_mode ^= 1;
        }
        break;
    case 0x08:
        s->state = val;
        s->sid = tetra_kernel(val, 0x1D);
        break;
    case 0x18:
        s->channel = val & 0xFF;
        break;
    case 0x1C:
        s->fb_offset = val;
        break;
    case 0x20:
        s->constant = val;
        break;
    default:
        break;
    }
}

static const MemoryRegionOps tetra_ctrl_ops = {
    .read = tetra_ctrl_read,
    .write = tetra_ctrl_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void tetra_ctrl_realize(DeviceState *dev, Error **errp)
{
    TetraCtrlState *s = TETRA_CTRL(dev);
    uint32_t base_period;

    memory_region_init_io(&s->mmio, OBJECT(s), &tetra_ctrl_ops, s,
                        TYPE_TETRA_CTRL, 256);
    sysbus_init_mmio(SYS_BUS_DEVICE(dev), &s->mmio);

    s->clk_fs = qdev_init_clock_out(dev, "FS");
    s->clk_gs = qdev_init_clock_out(dev, "GS");
    s->clk_rs = qdev_init_clock_out(dev, "RS");
    s->clk_us = qdev_init_clock_out(dev, "US");
    
    base_period = 10 + ((s->state & 0xFF) * 10);
    clock_set_ns(s->clk_fs, base_period);
    clock_set_ns(s->clk_gs, base_period + 2);
    clock_set_ns(s->clk_rs, base_period + 4);
    clock_set_ns(s->clk_us, base_period + 6);

    s->state = 0x4242;
    s->constant = 0x1D;
    s->sid = tetra_kernel(s->state, 0x1D);
    s->gnomon_step = 0;
    s->channel = 0x1D;
    s->fb_offset = 0;
    s->fb_size = 0;
    s->fb_ptr = NULL;
}

static void tetra_ctrl_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = tetra_ctrl_realize;
}

static const TypeInfo tetra_ctrl_info = {
    .name = TYPE_TETRA_CTRL,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(TetraCtrlState),
    .class_init = tetra_ctrl_class_init,
};

static void tetra_ctrl_register_types(void)
{
    type_register_static(&tetra_ctrl_info);
}

type_init(tetra_ctrl_register_types);