/*
 * hw/misc/tetra-clock-qom.c — Tetragrammatron Clock Device
 *
 * Provides FS/GS/RS/US clock outputs for constitutional clock tree.
 */

#include "qemu/osdep.h"
#include "hw/core/sysbus.h"
#include "hw/core/clock.h"
#include "hw/core/qdev-clock.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "qom/object.h"

#define TYPE_TETRA_CLOCK_QOM "tetra-clock-qom"
typedef struct TetraClockQOMState TetraClockQOMState;
DECLARE_INSTANCE_CHECKER(TetraClockQOMState, TETRA_CLOCK_QOM,
                        TYPE_TETRA_CLOCK_QOM)

struct TetraClockQOMState {
    SysBusDevice parent_obj;
    
    Clock *clk_fs;
    Clock *clk_gs;
    Clock *clk_rs;
    Clock *clk_us;
    Clock *clk_in;
    
    uint32_t gnomon_step;
    uint32_t current_layer;
};

static void tetra_clock_qom_tick(void *opaque, ClockEvent event)
{
    TetraClockQOMState *s = TETRA_CLOCK_QOM(opaque);
    
    if (event == ClockUpdate) {
        s->gnomon_step++;
    }
}

static void tetra_clock_qom_realize(DeviceState *dev, Error **errp)
{
    TetraClockQOMState *s = TETRA_CLOCK_QOM(dev);
    
    s->clk_fs = qdev_init_clock_out(dev, "FS");
    s->clk_gs = qdev_init_clock_out(dev, "GS");
    s->clk_rs = qdev_init_clock_out(dev, "RS");
    s->clk_us = qdev_init_clock_out(dev, "US");
    s->clk_in = qdev_init_clock_in(dev, "tick", tetra_clock_qom_tick, s, ClockUpdate);
    
    clock_set_ns(s->clk_fs, 10);
    clock_set_ns(s->clk_gs, 10);
    clock_set_ns(s->clk_rs, 10);
    clock_set_ns(s->clk_us, 10);
    
    s->gnomon_step = 0;
    s->current_layer = 0x1D;
}

static void tetra_clock_qom_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->realize = tetra_clock_qom_realize;
}

static const TypeInfo tetra_clock_qom_info = {
    .name = TYPE_TETRA_CLOCK_QOM,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(TetraClockQOMState),
    .class_init = tetra_clock_qom_class_init,
};

static void tetra_clock_qom_register_types(void)
{
    type_register_static(&tetra_clock_qom_info);
}

type_init(tetra_clock_qom_register_types);