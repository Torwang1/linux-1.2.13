/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Global definitions for the INET interface module.
 *
 * Version:	@(#)if.h	1.0.2	04/18/93
 *
 * Authors:	Original taken from Berkeley UNIX 4.3, (c) UCB 1982-1988
 *		Ross Biro, <bir7@leland.Stanford.Edu>
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H

#include <linux/types.h>		/* for "caddr_t" et al		*/
#include <linux/socket.h>		/* for "struct sockaddr" et al	*/

/* Standard interface flags. */
/* 网络设备接口状态常量定义 */
#define	IFF_UP		0x1		/* interface is up		*/
#define	IFF_BROADCAST	0x2		/* broadcast address valid	*/
#define	IFF_DEBUG	0x4		/* turn on debugging		*/
#define	IFF_LOOPBACK	0x8		/* is a loopback net		*/
#define	IFF_POINTOPOINT	0x10		/* interface is has p-p link	*/
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers	*/
#define	IFF_RUNNING	0x40		/* resources allocated		*/
#define	IFF_NOARP	0x80		/* no ARP protocol		*/
#define	IFF_PROMISC	0x100		/* receive all packets		*/
/* Not supported */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets*/

#define IFF_MASTER	0x400		/* master of a load balancer 	*/
#define IFF_SLAVE	0x800		/* slave of a load balancer	*/

#define IFF_MULTICAST	0x1000		/* Supports multicast		*/

/*
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address
 * families, are allocated and attached when an address is set,
 * and are linked together so all addresses for an interface can
 * be located.
 */

/*
 * struct ifaddr结构表示接口的一个地址信息
 * 同一个接口可以有多个不同的地址(struct sockaddr)，每个地址对应不同的协议簇
 * struct ifaddr::*ifa_next把同一个接口的不同地址信息串联，从而可以接口的所有地址信息
 */
struct ifaddr 
{
	struct sockaddr	ifa_addr;	/* address of interface		*/
	union {
		struct sockaddr	ifu_broadaddr;
		struct sockaddr	ifu_dstaddr;
	} ifa_ifu;
	struct iface		*ifa_ifp;	/* back-pointer to interface	*/
	
	/* 串联属于统一接口的不同协议簇中的地址 */
	struct ifaddr		*ifa_next;	/* next address for interface	*/
};

#define	ifa_broadaddr	ifa_ifu.ifu_broadaddr	/* broadcast address	*/
#define	ifa_dstaddr	ifa_ifu.ifu_dstaddr	/* other end of link	*/

/*
 *	Device mapping structure. I'd just gone off and designed a 
 *	beautiful scheme using only loadable modules with arguments
 *	for driver options and along come the PCMCIA people 8)
 *
 *	Ah well. The get() side of this is good for WDSETUP, and it'll
 *	be handy for debugging things. The set side is fine for now and
 *	being very small might be worth keeping for clean configuration.
 */
/* 设置设备的基础信息 */
struct ifmap 
{
	unsigned long mem_start;     // 硬件读写缓冲区首地址
	unsigned long mem_end;       // 硬件读写缓冲区尾地址
	unsigned short base_addr;     // 本设备所使用I/O端口地址
	unsigned char irq;                   // 本设备所使用的中断号
	unsigned char dma;		    // 本设备所以用的DMA通道号
	unsigned char port;		    // 对应device结构中的if_port字段
	/* 3 bytes spare */
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */

/* 表示一个设备接口的信息 */
struct ifreq 
{
#define IFHWADDRLEN	6       // 硬件地址长度
#define	IFNAMSIZ	16
	union
	{
		char	ifrn_name[IFNAMSIZ];		/* if name, e.g. "en0" */
		char	ifrn_hwaddr[IFHWADDRLEN];	/* Obsolete */
	} ifr_ifrn;
	
	union {
		struct	sockaddr ifru_addr;    			// 设备的IP地址
		struct	sockaddr ifru_dstaddr;              // p2p链接中对端地址
		struct	sockaddr ifru_broadaddr;         // 广播IP地址
		struct	sockaddr ifru_netmask;            // 地址掩码
		struct  sockaddr ifru_hwaddr;                 // 设备对应的硬件地址
		short	ifru_flags;                                 // 设备对应的标志字段值
		int	ifru_metric; 						 // 	代价值
		int	ifru_mtu;						 // 设备对应的最大传输单元
		struct  ifmap ifru_map; 				 // 该结构用于设置/获取设备的基本信息
		char	ifru_slave[IFNAMSIZ];	/* Just fits the size */
		caddr_t	ifru_data; 					 // 该字段的作用相当于raw，只表示存储单元或存储数据
	} ifr_ifru;
};

#define ifr_name	ifr_ifrn.ifrn_name	/* interface name 	*/
#define old_ifr_hwaddr	ifr_ifrn.ifrn_hwaddr	/* interface hardware   */
#define ifr_hwaddr	ifr_ifru.ifru_hwaddr	/* MAC address 		*/
#define	ifr_addr	ifr_ifru.ifru_addr	/* address		*/
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-p lnk	*/
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address	*/
#define	ifr_netmask	ifr_ifru.ifru_netmask	/* interface net mask	*/
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags		*/
#define	ifr_metric	ifr_ifru.ifru_metric	/* metric		*/
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu			*/
#define ifr_map		ifr_ifru.ifru_map	/* device map		*/
#define ifr_slave	ifr_ifru.ifru_slave	/* slave device		*/
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface	*/

/*
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
/* 表示系统中所有设备接口信息 */
/* 通常系统将遍历系统中所有设备接口，向ifcu_buf指向的缓冲区中填写设备信息，
 * 直到缓冲区满或者设备接口遍历完 */
struct ifconf 
{
	int	ifc_len;			/* size of buffer	*/
	union 
	{
		caddr_t	ifcu_buf;
		struct	ifreq *ifcu_req;  /*  设备接口信息数组  */
	} ifc_ifcu;
};
#define	ifc_buf	ifc_ifcu.ifcu_buf		/* buffer address	*/
#define	ifc_req	ifc_ifcu.ifcu_req		/* array of structures	*/


/* BSD UNIX expects to find these here, so here we go: */
#include <linux/if_arp.h>
#include <linux/route.h>

#endif /* _NET_IF_H */
