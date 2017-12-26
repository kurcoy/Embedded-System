/*
 * B53 common definitions
 *
 * Copyright (C) 2011-2013 Jonas Gorski <jogo@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __B53_PRIV_H
#define __B53_PRIV_H

#include "b53_regs.h"

//struct b53_device;
//struct net_device;
#define ETH_ALEN 6

enum {
	BCM5325_DEVICE_ID = 0x25,
	BCM5365_DEVICE_ID = 0x65,
	BCM5395_DEVICE_ID = 0x95,
	BCM5397_DEVICE_ID = 0x97,
	BCM5398_DEVICE_ID = 0x98,
	BCM53115_DEVICE_ID = 0x53115,
	BCM53125_DEVICE_ID = 0x53125,
	BCM53128_DEVICE_ID = 0x53128,
	BCM63XX_DEVICE_ID = 0x6300,
	BCM53010_DEVICE_ID = 0x53010,
	BCM53011_DEVICE_ID = 0x53011,
	BCM53012_DEVICE_ID = 0x53012,
	BCM53018_DEVICE_ID = 0x53018,
	BCM53019_DEVICE_ID = 0x53019,
	BCM58XX_DEVICE_ID = 0x5800,
	BCM7445_DEVICE_ID = 0x7445,
};

#define B53_N_PORTS	9
#define B53_N_PORTS_25	6

struct b53_port {
	uint16_t		vlan_ctl_mask;
	struct net_device *bridge_dev;
};

struct b53_vlan {
	uint16_t members;
	uint16_t untag;
	int valid;
};
#if 0
struct b53_device {
	struct dsa_switch *ds;
	struct b53_platform_data *pdata;
	const char *name;

	struct mutex reg_mutex;
	struct mutex stats_mutex;
	const struct b53_io_ops *ops;

	/* chip specific data */
	u32 chip_id;
	uint8_t core_rev;
	uint8_t vta_regs[3];
	uint8_t duplex_reg;
	uint8_t jumbo_pm_reg;
	uint8_t jumbo_size_reg;
	int reset_gpio;
	uint8_t num_arl_entries;

	/* used ports mask */
	uint16_t enabled_ports;
	unsigned int cpu_port;

	/* connect specific data */
	uint8_t current_page;
	struct device *dev;

	/* Master MDIO bus we got probed from */
	struct mii_bus *bus;

	void *priv;

	/* run time configuration */
	bool enable_jumbo;

	unsigned int num_vlans;
	struct b53_vlan *vlans;
	unsigned int num_ports;
	struct b53_port *ports;
};

#define b53_for_each_port(dev, i) \
	for (i = 0; i < B53_N_PORTS; i++) \
		if (dev->enabled_ports & BIT(i))


static inline int is5325(struct b53_device *dev)
{
	return dev->chip_id == BCM5325_DEVICE_ID;
}

static inline int is5365(struct b53_device *dev)
{
#ifdef CONFIG_BCM47XX
	return dev->chip_id == BCM5365_DEVICE_ID;
#else
	return 0;
#endif
}

static inline int is5397_98(struct b53_device *dev)
{
	return dev->chip_id == BCM5397_DEVICE_ID ||
		dev->chip_id == BCM5398_DEVICE_ID;
}

static inline int is539x(struct b53_device *dev)
{
	return dev->chip_id == BCM5395_DEVICE_ID ||
		dev->chip_id == BCM5397_DEVICE_ID ||
		dev->chip_id == BCM5398_DEVICE_ID;
}

static inline int is531x5(struct b53_device *dev)
{
	return dev->chip_id == BCM53115_DEVICE_ID ||
		dev->chip_id == BCM53125_DEVICE_ID ||
		dev->chip_id == BCM53128_DEVICE_ID;
}

static inline int is63xx(struct b53_device *dev)
{
#ifdef CONFIG_BCM63XX
	return dev->chip_id == BCM63XX_DEVICE_ID;
#else
	return 0;
#endif
}

static inline int is5301x(struct b53_device *dev)
{
	return dev->chip_id == BCM53010_DEVICE_ID ||
		dev->chip_id == BCM53011_DEVICE_ID ||
		dev->chip_id == BCM53012_DEVICE_ID ||
		dev->chip_id == BCM53018_DEVICE_ID ||
		dev->chip_id == BCM53019_DEVICE_ID;
}

static inline int is58xx(struct b53_device *dev)
{
	return dev->chip_id == BCM58XX_DEVICE_ID ||
		dev->chip_id == BCM7445_DEVICE_ID;
}

#define B53_CPU_PORT_25	5
#define B53_CPU_PORT	8

static inline int is_cpu_port(struct b53_device *dev, int port)
{
	return dev->cpu_port;
}

struct b53_device *b53_switch_alloc(struct device *base,
				    const struct b53_io_ops *ops,
				    void *priv);

int b53_switch_detect(struct b53_device *dev);

int b53_switch_register(struct b53_device *dev);

static inline void b53_switch_remove(struct b53_device *dev)
{
	dsa_unregister_switch(dev->ds);
}

static inline int b53_read8(struct b53_device *dev, uint8_t page, uint8_t reg, uint8_t *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read8(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read16(struct b53_device *dev, uint8_t page, uint8_t reg, uint16_t *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read16(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read32(struct b53_device *dev, uint8_t page, uint8_t reg, u32 *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read32(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read48(struct b53_device *dev, uint8_t page, uint8_t reg, uint64_t *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read48(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_read64(struct b53_device *dev, uint8_t page, uint8_t reg, uint64_t *val)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->read64(dev, page, reg, val);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write8(struct b53_device *dev, uint8_t page, uint8_t reg, uint8_t value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write8(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write16(struct b53_device *dev, uint8_t page, uint8_t reg,
			      uint16_t value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write16(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write32(struct b53_device *dev, uint8_t page, uint8_t reg,
			      u32 value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write32(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write48(struct b53_device *dev, uint8_t page, uint8_t reg,
			      uint64_t value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write48(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

static inline int b53_write64(struct b53_device *dev, uint8_t page, uint8_t reg,
			       uint64_t value)
{
	int ret;

	mutex_lock(&dev->reg_mutex);
	ret = dev->ops->write64(dev, page, reg, value);
	mutex_unlock(&dev->reg_mutex);

	return ret;
}

struct b53_arl_entry {
	uint8_t port;
	uint8_t mac[ETH_ALEN];
	uint16_t vid;
	uint8_t is_valid:1;
	uint8_t is_age:1;
	uint8_t is_static:1;
};
#endif

static inline void b53_mac_from_uint64_t(uint64_t src, uint8_t *dst)
{
	unsigned int i;

	for (i = 0; i < ETH_ALEN; i++)
		dst[ETH_ALEN - 1 - i] = (src >> (8 * i)) & 0xff;
}

static inline uint64_t b53_mac_to_uint64_t(const uint8_t *src)
{
	unsigned int i;
	uint64_t dst = 0;

	for (i = 0; i < ETH_ALEN; i++)
		dst |= (uint64_t)src[ETH_ALEN - 1 - i] << (8 * i);

	return dst;
}

#if 0
static inline void b53_arl_to_entry(struct b53_arl_entry *ent,
				    uint64_t mac_vid, u32 fwd_entry)
{
	memset(ent, 0, sizeof(*ent));
	ent->port = fwd_entry & ARLTBL_DATA_PORT_ID_MASK;
	ent->is_valid = !!(fwd_entry & ARLTBL_VALID);
	ent->is_age = !!(fwd_entry & ARLTBL_AGE);
	ent->is_static = !!(fwd_entry & ARLTBL_STATIC);
	b53_mac_from_uint64_t(mac_vid, ent->mac);
	ent->vid = mac_vid >> ARLTBL_VID_S;
}

static inline void b53_arl_from_entry(uint64_t *mac_vid, u32 *fwd_entry,
				      const struct b53_arl_entry *ent)
{
	*mac_vid = b53_mac_to_uint64_t(ent->mac);
	*mac_vid |= (uint64_t)(ent->vid & ARLTBL_VID_MASK) << ARLTBL_VID_S;
	*fwd_entry = ent->port & ARLTBL_DATA_PORT_ID_MASK;
	if (ent->is_valid)
		*fwd_entry |= ARLTBL_VALID;
	if (ent->is_static)
		*fwd_entry |= ARLTBL_STATIC;
	if (ent->is_age)
		*fwd_entry |= ARLTBL_AGE;
}
#endif

#ifdef CONFIG_BCM47XX

#include <linux/bcm47xx_nvram.h>
#include <bcm47xx_board.h>
static inline int b53_switch_get_reset_gpio(struct b53_device *dev)
{
	enum bcm47xx_board board = bcm47xx_board_get();

	switch (board) {
	case BCM47XX_BOARD_LINKSYS_WRT300NV11:
	case BCM47XX_BOARD_LINKSYS_WRT310NV1:
		return 8;
	default:
		return bcm47xx_nvram_gpio_pin("robo_reset");
	}
}
#else
static inline int b53_switch_get_reset_gpio(struct b53_device *dev)
{
	return -1;
}
#endif
#endif
