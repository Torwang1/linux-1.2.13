#ifndef _LINUX_SOCKET_H
#define _LINUX_SOCKET_H

#include <linux/sockios.h>		/* the SIOCxxx I/O controls	*/

/* 通用地址接口 */
struct sockaddr {
  unsigned short	sa_family;	/* address family, AF_xxx	*/
  char			sa_data[14];	/* 14 bytes of protocol address	*/
};

/* 设置套接字关闭时的行为 */
struct linger {
  int 			l_onoff;	/* Linger active   1:等待套接字完全关闭后，返回	*/
  int			l_linger;	/* How long to linger for	等待超时时间		*/
};

/* Socket types. */
#define SOCK_STREAM		1		/* stream (connection) socket	*/
#define SOCK_DGRAM			2		/* datagram (conn.less) socket	*/
#define SOCK_RAW			3		/* raw socket			*/
#define SOCK_RDM			4		/* reliably-delivered message	*/
#define SOCK_SEQPACKET		5		/* sequential packet socket	*/
#define SOCK_PACKET			10		/* linux specific way of	*/
									/* getting packets at the dev	*/
									/* level.  For writing rarp and	*/
									/* other similar things on the	*/
									/* user level.			*/

/* Supported address families. 域 */
#define AF_UNSPEC	0		// 需要zero申请到sockaddr结构，如果忘了赋值协议簇，那么默认就是AF_UNSPEC
#define AF_UNIX		1
#define AF_INET		2
#define AF_AX25		3
#define AF_IPX		4
#define AF_APPLETALK	5

#define AF_MAX		8	/* For now.. */

/* Protocol families, same as address families. */
#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_INET		AF_INET
#define PF_AX25		AF_AX25
#define PF_IPX		AF_IPX
#define PF_APPLETALK	AF_APPLETALK

#define PF_MAX		AF_MAX

/* Flags we can use with send/ and recv. */
/* 数据处理方式标志位 */
#define MSG_OOB			1		// 带外数据，作为紧急数据处理
#define MSG_PEEK		2
#define MSG_DONTROUTE	4

/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
/* 选项设置级别 socket option level */
#define SOL_SOCKET	1		// socket层的选项
#define SOL_IP		0		// ip协议选项
#define SOL_IPX		256
#define SOL_AX25	257
#define SOL_ATALK	258
#define SOL_TCP		6		// tcp协议选项
#define SOL_UDP		17		// udp协议选项

/* For setsockoptions(2) */
/* 具体选项类型 operation name	*/
#define SO_DEBUG		1
#define SO_REUSEADDR	2
#define SO_TYPE			3
#define SO_ERROR		4
#define SO_DONTROUTE	5
#define SO_BROADCAST	6
#define SO_SNDBUF		7	// 设置发送缓冲区大小
#define SO_RCVBUF		8	// 设置接收缓冲区大小
#define SO_KEEPALIVE	9
#define SO_OOBINLINE	10	// 设置紧急数据处理方式，1表示按照普通数据处理
#define SO_NO_CHECK	11
#define SO_PRIORITY		12
#define SO_LINGER		13
/* To add :#define SO_REUSEPORT 14 */

/* IP options */
#define 	IP_TOS		1
#define	IPTOS_LOWDELAY	0x10
#define	IPTOS_THROUGHPUT	0x08
#define	IPTOS_RELIABILITY	0x04
#define	IP_TTL		2
#ifdef V1_3_WILL_DO_THIS_FUNKY_STUFF
#define IP_HRDINCL	3
#define IP_OPTIONS	4
#endif

#define IP_MULTICAST_IF			32
#define IP_MULTICAST_TTL 		33
#define IP_MULTICAST_LOOP 		34
#define IP_ADD_MEMBERSHIP		35
#define IP_DROP_MEMBERSHIP		36


/* These need to appear somewhere around here */
#define IP_DEFAULT_MULTICAST_TTL        1
#define IP_DEFAULT_MULTICAST_LOOP       1
#define IP_MAX_MEMBERSHIPS              20
 
/* IPX options */
#define IPX_TYPE	1

/* TCP options - this way around because someone left a set in the c library includes */
#define TCP_NODELAY		1
#define TCP_MAXSEG		2

/* The various priorities. */
/* 数据包缓存到设备队列中的优先级 */
#define SOPRI_INTERACTIVE	0
#define SOPRI_NORMAL		1
#define SOPRI_BACKGROUND	2

#endif /* _LINUX_SOCKET_H */
