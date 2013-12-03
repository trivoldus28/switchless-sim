/* TTL modification target for IP tables
 * (C) 2000,2005 by Harald Welte <laforge@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ipt_TTL.h>

MODULE_AUTHOR("Harald Welte <laforge@netfilter.org>");
MODULE_DESCRIPTION("Xtables: IPv4 TTL field modification target");
MODULE_LICENSE("GPL");

static unsigned int
ttl_tg(struct sk_buff *skb, const struct net_device *in,
       const struct net_device *out, unsigned int hooknum,
       const struct xt_target *target, const void *targinfo)
{
	struct iphdr *iph;
	const struct ipt_TTL_info *info = targinfo;
	int new_ttl;

	if (!skb_make_writable(skb, skb->len))
		return NF_DROP;

	iph = ip_hdr(skb);

	switch (info->mode) {
		case IPT_TTL_SET:
			new_ttl = info->ttl;
			break;
		case IPT_TTL_INC:
			new_ttl = iph->ttl + info->ttl;
			if (new_ttl > 255)
				new_ttl = 255;
			break;
		case IPT_TTL_DEC:
			new_ttl = iph->ttl - info->ttl;
			if (new_ttl < 0)
				new_ttl = 0;
			break;
		default:
			new_ttl = iph->ttl;
			break;
	}

	if (new_ttl != iph->ttl) {
		csum_replace2(&iph->check, htons(iph->ttl << 8),
					   htons(new_ttl << 8));
		iph->ttl = new_ttl;
	}

	return XT_CONTINUE;
}

static bool
ttl_tg_check(const char *tablename, const void *e,
             const struct xt_target *target, void *targinfo,
             unsigned int hook_mask)
{
	const struct ipt_TTL_info *info = targinfo;

	if (info->mode > IPT_TTL_MAXMODE) {
		printk(KERN_WARNING "ipt_TTL: invalid or unknown Mode %u\n",
			info->mode);
		return false;
	}
	if (info->mode != IPT_TTL_SET && info->ttl == 0)
		return false;
	return true;
}

static struct xt_target ttl_tg_reg __read_mostly = {
	.name 		= "TTL",
	.family		= AF_INET,
	.target 	= ttl_tg,
	.targetsize	= sizeof(struct ipt_TTL_info),
	.table		= "mangle",
	.checkentry 	= ttl_tg_check,
	.me 		= THIS_MODULE,
};

static int __init ttl_tg_init(void)
{
	return xt_register_target(&ttl_tg_reg);
}

static void __exit ttl_tg_exit(void)
{
	xt_unregister_target(&ttl_tg_reg);
}

module_init(ttl_tg_init);
module_exit(ttl_tg_exit);
