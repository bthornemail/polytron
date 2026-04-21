/*
 * driver/tetra-ctrl.c — Guest driver for Tetra virt-ctrl
 *
 * Linux kernel module providing sysfs interface to the
 * tetragrammatron virt-ctrl MMIO device.
 *
 * Usage:
 *   echo 1 > /sys/devices/platform/tetra-ctrl/gnomon
 *   echo 1 > /sys/devices/platform/tetra-ctrl/omicron
 *   cat /sys/devices/platform/tetra-ctrl/state
 *   cat /sys/devices/platform/tetra-ctrl/sid
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#define TETRA_CTRL_NAME "tetra-ctrl"
#define TETRA_CTRL_CLASS "tetra"
#define TETRA_CTRL_REG_SIZE 256

#define TETRA_CMD_RESET     1
#define TETRA_CMD_GNOMON   4
#define TETRA_CMD_OMICRON 5

#define TETRA_REG_FEATURES   0x00
#define TETRA_REG_COMMAND  0x04
#define TETRA_REG_STATE    0x08
#define TETRA_REG_SID     0x0C
#define TETRA_REG_GNOMON  0x10
#define TETRA_REG_OMICRON 0x14
#define TETRA_REG_CHANNEL  0x18
#define TETRA_REG_FRAMEBUF 0x1C
#define TETRA_REG_CONSTANT 0x20
#define TETRA_REG_PERIOD  0x24
#define TETRA_REG_CYCLE  0x28

struct tetra_ctrl {
    void __iomem *base;
    resource_size_t size;
    struct device *dev;
    u32 features;
    u32 state;
    u32 sid;
    u32 period;
    u32 cycle_pos;
};

static inline u32 tetra_readl(struct tetra_ctrl *tc, u32 offset)
{
    return ioread32be(tc->base + offset);
}

static inline void tetra_writel(struct tetra_ctrl *tc, u32 offset, u32 val)
{
    iowrite32be(val, tc->base + offset);
}

static ssize_t gnomon_show(struct device *dev, struct device_attribute *attr,
                         char *buf)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    return sprintf(buf, "%u\n", tc->features ? 1 : 0);
}

static ssize_t gnomon_store(struct device *dev, struct device_attribute *attr,
                         const char *buf, size_t count)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    unsigned long val;
    
    if (kstrtoul(buf, 0, &val))
        return -EINVAL;
    
    if (val == 1) {
        tetra_writel(tc, TETRA_REG_COMMAND, TETRA_CMD_GNOMON);
        tc->state = tetra_readl(tc, TETRA_REG_STATE);
        tc->sid = tetra_readl(tc, TETRA_REG_SID);
    }
    
    return count;
}
static DEVICE_ATTR_RW(gnomon);

static ssize_t omicron_store(struct device *dev, struct device_attribute *attr,
                         const char *buf, size_t count)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    unsigned long val;
    
    if (kstrtoul(buf, 0, &val))
        return -EINVAL;
    
    if (val == 1) {
        tetra_writel(tc, TETRA_REG_COMMAND, TETRA_CMD_OMICRON);
    }
    
    return count;
}
static DEVICE_ATTR_WO(omicron);

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
                      char *buf)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    tc->state = tetra_readl(tc, TETRA_REG_STATE);
    return sprintf(buf, "0x%08x\n", tc->state);
}
static DEVICE_ATTR_RO(state);

static ssize_t sid_show(struct device *dev, struct device_attribute *attr,
                      char *buf)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    tc->sid = tetra_readl(tc, TETRA_REG_SID);
    return sprintf(buf, "0x%08x\n", tc->sid);
}
static DEVICE_ATTR_RO(sid);

static ssize_t period_show(struct device *dev, struct device_attribute *attr,
                         char *buf)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    tc->period = tetra_readl(tc, TETRA_REG_PERIOD);
    return sprintf(buf, "%u\n", tc->period);
}
static DEVICE_ATTR_RO(period);

static ssize_t cycle_show(struct device *dev, struct device_attribute *attr,
                        char *buf)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    tc->cycle_pos = tetra_readl(tc, TETRA_REG_CYCLE);
    return sprintf(buf, "%u\n", tc->cycle_pos);
}
static DEVICE_ATTR_RO(cycle);

static ssize_t reset_store(struct device *dev, struct device_attribute *attr,
                         const char *buf, size_t count)
{
    struct tetra_ctrl *tc = dev_get_drvdata(dev);
    unsigned long val;
    
    if (kstrtoul(buf, 0, &val))
        return -EINVAL;
    
    if (val == 1) {
        tetra_writel(tc, TETRA_REG_COMMAND, TETRA_CMD_RESET);
        tc->state = tetra_readl(tc, TETRA_REG_STATE);
        tc->sid = tetra_readl(tc, TETRA_REG_SID);
    }
    
    return count;
}
static DEVICE_ATTR_WO(reset);

static struct attribute *tetra_ctrl_attrs[] = {
    &dev_attr_gnomon.attr,
    &dev_attr_omicron.attr,
    &dev_attr_state.attr,
    &dev_attr_sid.attr,
    &dev_attr_period.attr,
    &dev_attr_cycle.attr,
    &dev_attr_reset.attr,
    NULL,
};

static const struct attribute_group tetra_ctrl_attr_group = {
    .attrs = tetra_ctrl_attrs,
};

static int tetra_ctrl_probe(struct platform_device *pdesc)
{
    struct tetra_ctrl *tc;
    struct resource *res;
    int ret;
    
    tc = devm_kzalloc(&pdesc->dev, sizeof(*tc), GFP_KERNEL);
    if (!tc)
        return -ENOMEM;
    
    res = platform_get_resource(pdesc, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(&pdesc->dev, "No memory resource\n");
        return -ENODEV;
    }
    
    tc->size = resource_size(res);
    if (tc->size < TETRA_CTRL_REG_SIZE) {
        dev_err(&pdesc->dev, "Resource size too small\n");
        return -ENODEV;
    }
    
    if (!devm_request_mem_region(&pdesc->dev, res->start, tc->size,
                             TETRA_CTRL_NAME)) {
        dev_err(&pdesc->dev, "Cannot request memory region\n");
        return -EBUSY;
    }
    
    tc->base = devm_ioremap(&pdesc->dev, res->start, tc->size);
    if (!tc->base) {
        dev_err(&pdesc->dev, "ioremap failed\n");
        return -ENOMEM;
    }
    
    tc->features = tetra_readl(tc, TETRA_REG_FEATURES);
    tc->state = tetra_readl(tc, TETRA_REG_STATE);
    tc->sid = tetra_readl(tc, TETRA_REG_SID);
    
    platform_set_drvdata(pdesc, tc);
    
    ret = sysfs_create_group(&pdesc->dev.kobj, &tetra_ctrl_attr_group);
    if (ret) {
        dev_err(&pdesc->dev, "Cannot create sysfs group: %d\n", ret);
        return ret;
    }
    
    dev_info(&pdesc->dev, "Tetragrammatron virt-ctrl initialized\n");
    dev_info(&pdesc->dev, "  state=0x%08x sid=0x%08x\n", tc->state, tc->sid);
    
    return 0;
}

static int tetra_ctrl_remove(struct platform_device *pdesc)
{
    sysfs_remove_group(&pdesc->dev.kobj, &tetra_ctrl_attr_group);
    return 0;
}

static const struct of_device_id tetra_ctrl_of_match[] = {
    { .compatible = "qemu,tetra-ctrl" },
    { }
};
MODULE_DEVICE_TABLE(of, tetra_ctrl_of_match);

static struct platform_driver tetra_ctrl_driver = {
    .probe = tetra_ctrl_probe,
    .remove = tetra_ctrl_remove,
    .driver = {
        .name = TETRA_CTRL_NAME,
        .of_match_table = tetra_ctrl_of_match,
    },
};

static int __init tetra_ctrl_init(void)
{
    return platform_driver_register(&tetra_ctrl_driver);
}

static void __exit tetra_ctrl_exit(void)
{
    platform_driver_unregister(&tetra_ctrl_driver);
}

module_init(tetra_ctrl_init);
module_exit(tetra_ctrl_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Tetragrammatron virt-ctrl guest driver");
MODULE_AUTHOR("Tetragrammatron");