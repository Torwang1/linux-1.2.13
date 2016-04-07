/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the IP protocol.
 *
 * Version:	@(#)ip.h	1.0.2	04/28/93
 *
 * Authors:	Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_IP_H
#define _LINUX_IP_H
#include <asm/byteorder.h>

/* 定义具体选项类型字段值 */
#define IPOPT_END	0
#define IPOPT_NOOP	1
#define IPOPT_SEC	130
#define IPOPT_LSRR	131
#define IPOPT_SSRR	137
#define IPOPT_RR		7
#define IPOPT_SID	136
#define IPOPT_TIMESTAMP	68


#define MAXTTL		255

/* 时间戳选项数据部分 */
struct timestamp {
	__u8	len;                                          // 选项长度
	__u8	ptr;						      // 指针
	union {
#if defined(LITTLE_ENDIAN_BITFIELD)
	__u8	flags:4,					      // 标志字段，该字段指定具体的时间戳类型
		overflow:4;					      // 溢出字段
#elif defined(BIG_ENDIAN_BITFIELD)
	__u8	overflow:4,
		flags:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif						
	__u8	full_char;
	} x;
	__u32	data[9];					      // 在只记录时间戳的情况下，最多可有9个时间戳缓存空间
};


#define MAX_ROUTE	16

struct route {
  char		route_size;					// 选项长度字段
  char		pointer;						// 指针字段
  unsigned long route[MAX_ROUTE];			// 路由数据，虽然MAX_ROUTE定义为16，但实际
  										// 可用的路由数据空间仅为37字节
};

/* 
 * 当接收到的IP数据报包含选项时，
 * 内核将根据数据报中具体的IP选项初始化该结构，
 * 以便内核其他代码的处理
 */
struct options {
  struct route		record_route;
  struct route		loose_route;
  struct route		strict_route;
  struct timestamp	tstamp;
  unsigned short	security;
  unsigned short	compartment;
  unsigned short	handling;
  unsigned short	stream;
  unsigned		tcc;
};


struct iphdr {
#if defined(LITTLE_ENDIAN_BITFIELD)
	__u8	ihl:4,
		version:4;
#elif defined (BIG_ENDIAN_BITFIELD)
	__u8	version:4,
  		ihl:4;                      // 首部长度，该字段以4字节为单位。故首部长度最大可达15 * 4 = 60字节，其中固定长度20字节
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8	tos;
	__u16	tot_len;
	__u16	id;
	__u16	frag_off;
	__u8	ttl;
	__u8	protocol;
	__u16	check;
	__u32	saddr;
	__u32	daddr;
	/*The options start here. */
};


#endif	/* _LINUX_IP_H */
