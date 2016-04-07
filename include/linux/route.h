/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Global definitions for the IP router interface.
 *
 * Version:	@(#)route.h	1.0.3	05/27/93
 *
 * Authors:	Original taken from Berkeley UNIX 4.3, (c) UCB 1986-1988
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_ROUTE_H
#define _LINUX_ROUTE_H

#include <linux/if.h>


/* This structure gets passed by the SIOCADDRTOLD and SIOCDELRTOLD calls. */

struct old_rtentry {
	unsigned long	rt_genmask;
	struct sockaddr	rt_dst;
	struct sockaddr	rt_gateway;
	short		rt_flags;
	short		rt_refcnt;
	unsigned long	rt_use;				// 该路由表项是否正在使用
	char		*rt_dev; 				// 该路由路径的出站接口设备
};

/* This structure gets passed by the SIOCADDRT and SIOCDELRT calls. */
struct rtentry {
	unsigned long	rt_hash;	/* hash key for lookups		*/
	struct sockaddr	rt_dst;		/* target address		*/
	struct sockaddr	rt_gateway;	/* gateway addr (RTF_GATEWAY)	*
								 * 当无法直接到达目的地址，需要网关进行转发*
								 * 该字段代表中间网关的地址				   */
	struct sockaddr	rt_genmask;	/* target network mask (IP)	*/
	short		rt_flags;
	short		rt_refcnt;
	unsigned long	rt_use;
	struct ifnet	*rt_ifp;
	short		rt_metric;		/* +1 for binary compatibility!	*/
	char		*rt_dev;				/* forcing the device at add	*/
	unsigned long	rt_mss;			/* per route MTU/Window 该路径的最大传输单元 */
	unsigned long	rt_window;		/* Window clamping 路径的窗口大小。窗口是一种限制数据包发送过快的措施，*
								 * 发送数据包的个数或总字节数应该在窗口内。*/
};

/* 定义路由表项标志 */
#define	RTF_UP			0x0001		/* route usable		  */
#define	RTF_GATEWAY	0x0002		/* destination is a gateway	  */
#define	RTF_HOST		0x0004		/* host entry (net otherwise)	  */
#define 	RTF_REINSTATE	0x0008		/* reinstate route after tmout	  *
									 * 每个表项都有一个超时时间，当超时*
									 * 发生时，该表项需要重新确认，才有效*
									 **/
#define	RTF_DYNAMIC	0x0010		/* created dyn. (by redirect)	  */
#define	RTF_MODIFIED	0x0020		/* modified dyn. (by redirect)	  */
#define 	RTF_MSS		0x0040		/* specific MSS for this route 该表项制定了最大传输单元	  */
#define 	RTF_WINDOW	0x0080		/* per route window clampin 该表项指定了窗口大小  */

/*
 *	REMOVE THESE BY 1.2.0 !!!!!!!!!!!!!!!!!
 */
 
#define	RTF_MTU		RTF_MSS
#define rt_mtu		rt_mss		

#endif	/* _LINUX_ROUTE_H */
