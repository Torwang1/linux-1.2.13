/*
 *	Linux NET3:	Internet Gateway Management Protocol  [IGMP]
 *
 *	Authors:
 *		Alan Cox <Alan.Cox@linux.org>	
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#ifndef _LINUX_IGMP_H
#define _LINUX_IGMP_H

/*
 *	IGMP protocol structures
 */

struct igmphdr
{
	unsigned char type;
	unsigned char unused;
	unsigned short csum;
	unsigned long group;
};

/* 定义了igmp的类型，查询报告、成员报告、退出报告 */
#define IGMP_HOST_MEMBERSHIP_QUERY	0x11	/* From RFC1112 */
/* 成员关系报告，这是IGMPV1版本定义的值，IGMPV2版本已经改为16 */
#define IGMP_HOST_MEMBERSHIP_REPORT	0x12	/* Ditto */
#define IGMP_HOST_LEAVE_MESSAGE		0x17	/* An extra BSD seems to send */


/* 对于任何一个主机而言，其网络接口在初始化时默认加入224.0.0.1多播组 */
				/* 224.0.0.1 */
#define IGMP_ALL_HOSTS		htonl(0xE0000001L)

/*
 * struct for keeping the multicast list in
 */

#ifdef __KERNEL__

/* 多播地址以及对应的发送接收接口 */
struct ip_mc_socklist
{
	unsigned long multiaddr[IP_MAX_MEMBERSHIPS];	/* This is a speed trade off */
	struct device *multidev[IP_MAX_MEMBERSHIPS];
};

struct ip_mc_list 
{
	struct device *interface;      // 接收或发送对应多播数据包的接口
	unsigned long multiaddr;	  // 对应的多播地址
	struct ip_mc_list *next;
	struct timer_list timer;
	int tm_running;
	int users;
};
 
extern struct ip_mc_list *ip_mc_head;


extern int igmp_rcv(struct sk_buff *, struct device *, struct options *, unsigned long, unsigned short,
	unsigned long, int , struct inet_protocol *);
extern void ip_mc_drop_device(struct device *dev); 
extern int ip_mc_join_group(struct sock *sk, struct device *dev, unsigned long addr);
extern int ip_mc_leave_group(struct sock *sk, struct device *dev,unsigned long addr);
extern void ip_mc_drop_socket(struct sock *sk);
#endif
#endif
