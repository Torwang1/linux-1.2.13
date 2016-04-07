/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the Interfaces handler.
 *
 * Version:	@(#)dev.h	1.0.10	08/12/93
 *
 * Authors:	Ross Biro, <bir7@leland.Stanford.Edu>
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Corey Minyard <wf-rch!minyard@relay.EU.net>
 *		Donald J. Becker, <becker@super.org>
 *		Alan Cox, <A.Cox@swansea.ac.uk>
 *		Bjorn Ekwall. <bj0rn@blox.se>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *		Moved to /usr/include/linux for NET3
 */
#ifndef _LINUX_NETDEVICE_H
#define _LINUX_NETDEVICE_H

#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>

/* for future expansion when we will have different priorities. */
#define DEV_NUMBUFFS	3
#define MAX_ADDR_LEN	7
#define MAX_HEADER	18

#define IS_MYADDR	1		/* address is (one of) our own	*/
#define IS_LOOPBACK	2		/* address is for LOOPBACK	*/
#define IS_BROADCAST	3		/* address is a valid broadcast	*/
#define IS_INVBCAST	4		/* Wrong netmask bcast not for us (unused)*/
#define IS_MULTICAST	5		/* Multicast IP address */

/*
 *	We tag these structures with multicasts.
 */
 
struct dev_mc_list
{	
	struct dev_mc_list *next;
	char dmi_addr[MAX_ADDR_LEN];
	unsigned short dmi_addrlen;
	unsigned short dmi_users;
};

/*
 * The DEVICE structure.
 * Actually, this whole structure is a big mistake.  It mixes I/O
 * data with strictly "high-level" data, and it has to know about
 * almost every data structure used in the INET module.  
 */
struct device 
{

  /*
   * This is the first field of the "visible" part of this structure
   * (i.e. as seen by users in the "Space.c" file).  It is the name
   * the interface.
   */
  char			  *name;

  /* I/O specific fields - FIXME: Merge these and struct ifmap into one */
  /* 设备读缓冲区空间 */
  unsigned long		  rmem_end;		/* shmem "recv" end	*/
  unsigned long		  rmem_start;		/* shmem "recv" start	*/
  /* 设备总缓冲区首地址和尾地址,这其中包含读/写缓冲区 */
  unsigned long		  mem_end;		/* shared mem end	*/
  unsigned long		  mem_start;		/* shared mem start	*/
  /* 设备I/O基地址 */
  unsigned long		  base_addr;		/* device I/O address	*/
  /* 设备使用的中断号 */
  unsigned char		  irq;			/* device IRQ number	*/

  /* Low-level status flags. */
  volatile unsigned char  start,		/* start an operation	*/
                     	       tbusy,		/* transmitter busy，如果再有数据包，需要等待	*/
                        		interrupt;		/* interrupt arrived，正在进行终中断处理	*/

  struct device		  *next;   	/* 构成设备队列，内核dev_base指向这个队列	*/

  /* The device initialization function. Called only once. */
  /* 对于静态定义的设备，init指向的函数在网络栈初始化时被调用，用以初始化设备	*/
  int			  (*init)(struct device *dev);

  /* Some hardware also needs these fields, but they are not part of the
     usual set specified in Space.c. */
  unsigned char		  if_port;		/* Selectable AUI, TP,..*/
  unsigned char		  dma;			/* DMA channel	DMA设备通道号，如果设备使用DMA进行数据传输	*/

  struct enet_statistics* (*get_stats)(struct device *dev);

  /*
   * This marks the end of the "visible" part of the structure. All
   * fields hereafter are internal to the system, and may change at
   * will (read: may be cleaned up at will).
   */

  /* These may be needed for future network-power-down code. */
  unsigned long		  trans_start;	/* Time (in jiffies) of last Tx	*/
  unsigned long		  last_rx;	/* Time of last Rx		*/

  /* 设备状态 && 设备功能 */
  unsigned short	  flags;	/* interface flags (IFF_**)	*/
  unsigned short	  family;	/* address family ID (AF_INET)	*/
  unsigned short	  metric;	/* routing metric (not used)	*/
  unsigned short	  mtu;		/* interface MTU value		*/
  unsigned short	  type;		/* interface hardware type	*/
  unsigned short	  hard_header_len;	/* hardware hdr length	*/
  void			  *priv;	/* pointer to private data	*/

  /* Interface address info. */
  unsigned char		  broadcast[MAX_ADDR_LEN];	/* hw bcast add	*/
  unsigned char		  dev_addr[MAX_ADDR_LEN];	/* hw address	*/
  unsigned char		  addr_len;	/* hardware address length	*/
  unsigned long		  pa_addr;	/* protocol address		*/
  unsigned long		  pa_brdaddr;	/* protocol broadcast addr	*/
  unsigned long		  pa_dstaddr;	/* protocol P-P other side addr	*/
  unsigned long		  pa_mask;	/* protocol netmask		*/
  unsigned short		  pa_alen;	/* protocol address length	*/

  struct dev_mc_list	 *mc_list;	/* Multicast mac addresses	*/
  int			 mc_count;	/* Number of installed mcasts	*/
  
  struct ip_mc_list	 *ip_mc_list;	/* IP multicast filter chain    */
    
  /* For load balancing driver pair support */
  
  unsigned long		   pkt_queue;	/* Packets queued 设备缓存的待发送数数据包个数 */
  struct device		  *slave;	/* Slave device */
  

  /* Pointer to the interface buffers. */
  struct sk_buff_head	  buffs[DEV_NUMBUFFS];

  /* Pointers to interface service routines. */
  int			  (*open)(struct device *dev);
  int			  (*stop)(struct device *dev);
  /* 网卡驱动程序中数据包发送的核心函数 	*/
  int			  (*hard_start_xmit) (struct sk_buff *skb,
					      struct device *dev);
  int			  (*hard_header) (unsigned char *buff,
					  struct device *dev,
					  unsigned short type,
					  void *daddr,
					  void *saddr,
					  unsigned len,
					  struct sk_buff *skb);
  int			  (*rebuild_header)(void *eth, struct device *dev,
				unsigned long raddr, struct sk_buff *skb);
  /* 从接收到的数据包提取MAC首部中，类型字段值，从而选择上层协议	*/
  unsigned short	  (*type_trans) (struct sk_buff *skb,
					 struct device *dev);
#define HAVE_MULTICAST			 
  void			  (*set_multicast_list)(struct device *dev,
  					 int num_addrs, void *addrs);
#define HAVE_SET_MAC_ADDR  		 
  int			  (*set_mac_address)(struct device *dev, void *addr);
#define HAVE_PRIVATE_IOCTL
  int			  (*do_ioctl)(struct device *dev, struct ifreq *ifr, int cmd);
#define HAVE_SET_CONFIG
  int			  (*set_config)(struct device *dev, struct ifmap *map);
  
};

/* 表示网络层协议，每个网络层协议对应一个struct packet_type结构
 * 每当收到一个网络数据包，就会通过device::type_trans()函数返回网络层协议类型，
 * 然后在全局ptype_base链表中，找到对应的网络层协议，调用对应处理函数。
 *
 * 如IP协议对应的ip_rcv
 */
struct packet_type {
  unsigned short	type;	/* This is really htons(ether_type). ETH_P_**  网络字节序*/
  struct device *	dev;	/* NULL 表示任意设备 */
  int			(*func) (struct sk_buff *, struct device *,
				 struct packet_type *);
  void			*data;
  struct packet_type	*next;
};


#ifdef __KERNEL__

#include <linux/notifier.h>

/* Used by dev_rint */
#define IN_SKBUFF	1

extern volatile char in_bh;

extern struct device	loopback_dev;
extern struct device	*dev_base;
extern struct packet_type *ptype_base;


extern int		ip_addr_match(unsigned long addr1, unsigned long addr2);
extern int		ip_chk_addr(unsigned long addr);
extern struct device	*ip_dev_check(unsigned long daddr);
extern unsigned long	ip_my_addr(void);
extern unsigned long	ip_get_mask(unsigned long addr);

/* 链路层模块提供给网络层进行网络层协议注册、注销的函数 */
extern void		dev_add_pack(struct packet_type *pt);
extern void		dev_remove_pack(struct packet_type *pt);

extern struct device	*dev_get(char *name);

extern int		dev_open(struct device *dev);
extern int		dev_close(struct device *dev);

/* 通过该函数向网络介质上发送数据 */
extern void		dev_queue_xmit(struct sk_buff *skb, struct device *dev,
				       int pri);

#define HAVE_NETIF_RX 1
/* 从网卡驱动程序向网络栈传递数据
 * 驱动程序从网卡硬件缓冲区读取数据并封装成sk_buff结构
 * 然后调用netif_rx函数将数据传递给内核网络栈处理
 * netif_rx()的执行环境是中断上下文，中断处理函数要求尽可能
 * 快地完成任务，所以netif_rx()简单将sk_buff结构挂入某个
 * 缓冲队列，然后返回
 */
extern void		netif_rx(struct sk_buff *skb);

/* The old interface to netif_rx(). */
extern int		dev_rint(unsigned char *buff, long len, int flags,
				 struct device * dev);
extern void		dev_transmit(void);
extern int		in_net_bh(void);

/* 对netif_rx()挂入系统的sk_buff结构，真正实在这个函数中处理的 */
extern void		net_bh(void *tmp);

extern void		dev_tint(struct device *dev);
extern int		dev_get_info(char *buffer, char **start, off_t offset, int length);
extern int		dev_ioctl(unsigned int cmd, void *);

extern void		dev_init(void);

/* These functions live elsewhere (drivers/net/net_init.c, but related) */

extern void		ether_setup(struct device *dev);
extern int		ether_config(struct device *dev, struct ifmap *map);

/* Support for loadable net-drivers */
extern int		register_netdev(struct device *dev);
extern void		unregister_netdev(struct device *dev);
extern int 		register_netdevice_notifier(struct notifier_block *nb);
extern int		unregister_netdevice_notifier(struct notifier_block *nb);
/* Functions used for multicast support */
extern void		dev_mc_upload(struct device *dev);
extern void 		dev_mc_delete(struct device *dev, void *addr, int alen, int all);
extern void		dev_mc_add(struct device *dev, void *addr, int alen, int newonly);
extern void		dev_mc_discard(struct device *dev);
/* This is the wrong place but it'll do for the moment */
extern void		ip_mc_allhost(struct device *dev);
#endif /* __KERNEL__ */

#endif	/* _LINUX_DEV_H */
