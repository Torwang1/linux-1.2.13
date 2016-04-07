/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the AF_INET socket handler.
 *
 * Version:	@(#)sock.h	1.0.4	05/13/93
 *
 * Authors:	Ross Biro, <bir7@leland.Stanford.Edu>
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *		Corey Minyard <wf-rch!minyard@relay.EU.net>
 *		Florian La Roche <flla@stud.uni-sb.de>
 *
 * Fixes:
 *		Alan Cox	:	Volatiles in skbuff pointers. See
 *					skbuff comments. May be overdone,
 *					better to prove they can be removed
 *					than the reverse.
 *		Alan Cox	:	Added a zapped field for tcp to note
 *					a socket is reset and must stay shut up
 *		Alan Cox	:	New fields for options
 *	Pauline Middelink	:	identd support
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _SOCK_H
#define _SOCK_H

#include <linux/timer.h>
#include <linux/ip.h>		/* struct options */
#include <linux/tcp.h>		/* struct tcphdr */
#include <linux/config.h>

#include <linux/skbuff.h>	/* struct sk_buff */
#include "protocol.h"		/* struct inet_protocol */
#ifdef CONFIG_AX25
#include "ax25.h"
#endif
#ifdef CONFIG_IPX
#include "ipx.h"
#endif
#ifdef CONFIG_ATALK
#include <linux/atalk.h>
#endif

#include <linux/igmp.h>

#define SOCK_ARRAY_SIZE	256		/* Think big (also on some systems a byte is faster */


/*
 * This structure really needs to be cleaned up.
 * Most of it is for TCP, and not used by any of
 * the other protocols.
 */
 /* 在INET层，表示套接字 */
struct sock {
  struct options			*opt;			// IP选项缓存(收到数据包后，填充该结构，方便后续处理使用)
  volatile unsigned long		wmem_alloc;	// 当前已分配写缓冲区的大小
  volatile unsigned long		rmem_alloc;		// 当前已分配读缓冲区的大小

  /* TCP可靠传输使用的序列号	*/
  unsigned long			write_seq;		// 应用程序下次向TCP发送缓冲区写入数据中第一个字节的编号
  unsigned long			sent_seq;		// TCP待发送数据的第一个字节编号，对应TCP首部中"序列号"
  unsigned long			acked_seq;		// TCP等待接收数据的第一个字节编号，对应TCP首部中"确认号"
  unsigned long			copied_seq;		// copied_seq之前数据已经被用户程序读取
  unsigned long			rcv_ack_seq;	// 已被确认字节的下一个字节编号，rcv_ack_seq=2312表示2312之前的字节已经被正确接收
  unsigned long			window_seq;	// window_seq=sent_seq+window(对端通告窗口大小)，表示window_seq之前的字节序列是可以发送的，之后的因为窗口大小而不能发送
  unsigned long			fin_seq;			// 在接收到对方含有FIN数据包后，记录对该数据包进行确认的"确认号"

  /* TCP紧急数据处理	*/
  unsigned long			urg_seq;		// 紧急数据的最大序列号
  unsigned long			urg_data;		// 标志位，置为1时表示接收到紧急数据

  /*
   * Not all are volatile, but some are, so we
   * might as well say they all are.
   */
  volatile char			inuse,			// inuse=1 其它进程正在使用该sock结构，当前进程需要等待，相当于sock结构的锁
					dead,			// dead=1 当前sock结构已经处于释放状态
					urginline,		// urginlin=1 紧急数据被当做普通数据进行处理
					intr,
					blog,			// blog=1 套接字处于节制状态，此时收到的数据报均被丢弃
					done,
					reuse,
					keepopen,		// keepopen=1 使用保活定时器
					linger,			// linger=1	关闭套接字时，操作需要等待套接字关闭或者超时(lingertime)后返回
					delay_acks,		// delay_acks=1 延时应答
					destroy,			// destory=1 改sock结构等待销毁
					ack_timed,
					no_check,		// TCP_NO_CHECK
					zapped,			// In ax25 & ipx means not linked；在TCP中作为"复位标志位"，zapped=1表示收到复位数据包
					broadcast,
					nonagle;		// 是否禁用TCP nagle算法
  unsigned long		lingertime;		// 套接字关闭时，关闭操作在lingertime后仍然未完成，操作返回。只有在linger标志位有效时起作用

  int					proc;			// 该sock结构所属进程pid

  /* 以下3个字段，用于sock之间连接	*/
  struct sock			*next;
  struct sock			*prev; 		/* Doubly linked chain.. */
  struct sock			*pair;		/* 参见:inet_accept函数
  								 * 如果上次inet_accept时，是以阻塞的方式接收的，在等待连接完成连接的过程中，被信号打断了，
  								 * 就要把之前调用sk_prot->accept接收到的连接newsk放入pair字段中暂存。再次调用inet_accept时
  								 * 优先处理pair字段
  								 */

  /* TCP重发队列	*/
  struct sk_buff	* volatile send_head;	// 重发队列头指针
  struct sk_buff	* volatile send_tail;	// 重发队列尾指针

  /* TCP后备接收队列，当sock结构被某个进程锁定，这时通过中断接收到的数据就会挂载到back_log队列中
  * 但在back_log队列中的数据实际上并没有被确认，在从back_log移到reveive_queue后，数据才算被正式接收，发出应答
  * 在向receive_queue队列转移时，可能因为超出接收缓冲区大小上限，而直接丢弃数据,这使得发送端超时重发
  */
  struct sk_buff_head	back_log;		// 已经被缓存但尚未被确认的数据，或者说尚未被TCP协议处理的数据
									// 数据到达时，当前sock被锁定，就直接放到后备队列中
  struct sk_buff	    * partial;			// 用于创建最大长度的待发送数据包.某些情况下回出现多个partial buffer，参见tcp_dequeue_partial注释
  struct timer_list		partial_timer;	// 定时器，用于发送partial指向的数据包，防止数据缓存(等待)时间过长
  
  long				retransmits;		// 记录重发次数

  /* 数据发送/接收队列 	*/
  struct sk_buff_head	write_queue,	// 应用程序已经发送到TCP协议进行排队的数据
					receive_queue;	// 当前连接已经接收并确认过的数据，如果当前连接时监听连接，那么也存放请求连接的套接字(参见destroy_sock代码)
					
  struct proto			*prot;			// 发送数据时，INET调用传输层的接口集合
  
  struct wait_queue	**sleep;			// 阻塞在sock上的进程队列。inet_release()中，如果linger=1时，有可能调用sock_release的进程就会阻塞在该sleep队列上，等待关闭操作完成
  
  unsigned long		daddr;			// 对端地址
  unsigned long		saddr;			// 本地地址
  unsigned short		max_unacked;	// 最大已接收(received_queue)但尚未给出确认的数据字节数
  unsigned short		window;			// 对端通告的窗口大小
  unsigned short		bytes_rcv;		// 该链接上已接受字节数，计数器
/* mss is min(mtu, max_window) */
  unsigned short		mtu;			/* mss negotiated in the syn's 		连接建立时协商的最大传输单元			*/
  volatile unsigned short	mss;       		/* current eff. mss - can change 	最大报文长度 = mtu - IP首部 - tcp首部	*/
  volatile unsigned short	user_mss;  		/* mss requested by user in ioctl	用户指定的mss值							*/

  /* 最大窗口大小和窗口大小钳制值	*/
  volatile unsigned short	max_window;	// 对端曾经通告的最大窗口大小  Nagle中有个条件，发送端累计数据大于max_window>>1时，立刻发送
  unsigned long 		window_clamp;	
  
  unsigned short		num;			// 本地端口号

  /* 以下3个字段用于拥塞控制算法	*/
  volatile unsigned short	cong_window;
  volatile unsigned short	cong_count;
  volatile unsigned short	ssthresh;

  volatile unsigned short	packets_out;	// 已经发出，但尚未得到应答的数据包数目
  volatile unsigned short	shutdown;		// 本地关闭标志位，用于半关闭操作

  /* 							*/
  volatile unsigned long	rtt;				// 往返时间估计值
  volatile unsigned long	mdev;			// mean deviation，即RTTD,绝对偏差
  volatile unsigned long	rto;				// 通过rtt和mdev计算得出的延迟时间
  
/* currently backoff isn't used, but I'm maintaining it in case
 * we want to go back to a backoff formula that needs it
 */
  volatile unsigned short	backoff;			// 退避算法度量值(当前未使用)
  volatile short			err;				// 连接上最近一次出错信息
  unsigned char		protocol;		// 传输层协议值
  volatile unsigned char	state;			// 套接字状态，如TCP_ESTABLISHED。inet_create()函数初始化state=TCP_CLOSED
  volatile unsigned char	ack_backlog;	// 缓存的未应答数据包数目
  unsigned char		max_ack_backlog; 	// 已发送但未被确认的数据报个数上限(参见inet_create()函数中，英文注释)
  
  unsigned char		priority;			// 该套接字优先级，在硬件缓存发送数据包时使用

  unsigned char		debug;
  unsigned short		rcvbuf;			// 最大接收缓冲区大小
  unsigned short		sndbuf;			// 最大发送缓冲区大小
  unsigned short		type;			// 套接字类型，如SOCK_STREAM
  
  unsigned char		localroute;		/* Route locally only 置为1时表示只使用本地路由，一半目的端在相同子网时使用	*/
#ifdef 0	/* 仅考虑IP协议 */
#ifdef CONFIG_IPX
  ipx_address			ipx_dest_addr;
  ipx_interface			*ipx_intrfc;
  unsigned short		ipx_port;
  unsigned short		ipx_type;
#endif
#ifdef CONFIG_AX25
/* Really we want to add a per protocol private area */
  ax25_address		ax25_source_addr,ax25_dest_addr;
  struct sk_buff *volatile ax25_retxq[8];
  char				ax25_state,ax25_vs,ax25_vr,ax25_lastrxnr,ax25_lasttxnr;
  char				ax25_condition;
  char				ax25_retxcnt;
  char				ax25_xx;
  char				ax25_retxqi;
  char				ax25_rrtimer;
  char				ax25_timer;
  unsigned char		ax25_n2;
  unsigned short		ax25_t1,ax25_t2,ax25_t3;
  ax25_digi			*ax25_digipeat;
#endif  
#ifdef CONFIG_ATALK
  struct atalk_sock		at;
#endif
#endif

  /* IP 'private area' or will be eventually */
  /* IP协议的私有数据	*/
  int				ip_ttl;				/* TTL setting */
  int				ip_tos;				/* TOS */
  
  struct tcphdr	dummy_th;			// 缓存TCP首部，发送数据包时使用该字段可以快速创建TCP头部

  /* TCP协议使用的定时器	*/
  struct timer_list	keepalive_timer;	/* TCP keepalive hack		保活定时器 可以用来探测对方窗口大小，防止对方的窗口通告丢失*/
  struct timer_list	retransmit_timer;	/* TCP retransmit timer 	重传定时器 用于数据包超时	*/
  struct timer_list	ack_timer;			/* TCP delayed ack timer	延迟确认定时器	 */

  /*	标志位组合字段，用于表示下文中通用定时器timer的超时原因	*/
  int				ip_xmit_timeout;	/* Why the timeout is running */
  
#ifdef CONFIG_IP_MULTICAST  
  int				ip_mc_ttl;			/* Multicasting TTL */
  int				ip_mc_loop;			/* Loopback (not implemented yet) */
  char				ip_mc_name[MAX_ADDR_LEN];	/* Multicast device name */
  struct ip_mc_socklist		*ip_mc_list;			/* Group array */
#endif  

  /* This part is used for the timeout functions (timer.c). */
  int				timeout;		/* What are we waiting for? 超时原因 */
  struct timer_list	timer;		/* This is the TIME_WAIT/receive timer when we are doing IP */
  
  struct timeval	stamp;

  /* identd */
  struct socket	*socket;		// 指向对应的socket结构
  
  /* Callbacks 在inet_create()函数中赋值默认inet回调函数	*/
  void			(*state_change)(struct sock *sk);			// default_callback1()
  void			(*data_ready)(struct sock *sk,int bytes);	// default_callback2()
  void			(*write_space)(struct sock *sk);			// default_callback3()
  void			(*error_report)(struct sock *sk);			// default_callback1()
};


/** 
 * 协议簇调用具体的传输层协议的接口
 * 
 * 传输层协议都会定义一个struct proto
 * 结构中包含传输层提供的接口，以及，每个传输层协议的专有数据
 * 如: tcp_prot / udp_prot / raw_prot
 */
struct proto {
  struct sk_buff *	(*wmalloc)(struct sock *sk,
						    unsigned long size, int force,
						    int priority);
  struct sk_buff *	(*rmalloc)(struct sock *sk,
						    unsigned long size, int force,
						    int priority);
  void			(*wfree)(struct sock *sk, struct sk_buff *skb,
						    unsigned long size);
  void			(*rfree)(struct sock *sk, struct sk_buff *skb,
						    unsigned long size);
  unsigned long	(*rspace)(struct sock *sk);
  unsigned long	(*wspace)(struct sock *sk);
  void			(*close)(struct sock *sk, int timeout);
  int				(*read)(struct sock *sk, unsigned char *to,
						int len, int nonblock, unsigned flags);
  int				(*write)(struct sock *sk, unsigned char *to,
						 int len, int nonblock, unsigned flags);
  int				(*sendto)(struct sock *sk,
				     	         unsigned char *from, int len, int noblock,
						  unsigned flags, struct sockaddr_in *usin,
						  int addr_len);
  int				(*recvfrom)(struct sock *sk,
						    unsigned char *from, int len, int noblock,
						    unsigned flags, struct sockaddr_in *usin,
						    int *addr_len);
  int				(*build_header)(struct sk_buff *skb,
							unsigned long saddr,
							unsigned long daddr,
							struct device **dev, int type,
							struct options *opt, int len, int tos, int ttl);
  int				(*connect)(struct sock *sk,
						  struct sockaddr_in *usin, int addr_len);
  struct sock *	(*accept) (struct sock *sk, int flags);
  void			(*queue_xmit)(struct sock *sk,
							      struct device *dev, struct sk_buff *skb,
							      int free);
  void			(*retransmit)(struct sock *sk, int all);
  void			(*write_wakeup)(struct sock *sk);
  void			(*read_wakeup)(struct sock *sk);
	  int			(*rcv)(struct sk_buff *buff, struct device *dev,
					       struct options *opt, unsigned long daddr,
					       unsigned short len, unsigned long saddr,
					       int redo, struct inet_protocol *protocol);
  int			(*select)(struct sock *sk, int which,
				  select_table *wait);
  int			(*ioctl)(struct sock *sk, int cmd,
				 unsigned long arg);
  int			(*init)(struct sock *sk);
  void		(*shutdown)(struct sock *sk, int how);
  int			(*setsockopt)(struct sock *sk, int level, int optname,
  						 char *optval, int optlen);
  int			(*getsockopt)(struct sock *sk, int level, int optname,
  						char *optval, int *option);  	 
  
  unsigned short	max_header;
  unsigned long	retransmits;
  struct sock *	sock_array[SOCK_ARRAY_SIZE];	// 按端口号组织使用该传输层协议的sock
  char			name[80];
  int				inuse, 			// 使用该协议的sock数量
  				highestinuse;
};

#define TIME_WRITE	1
#define TIME_CLOSE	2
#define TIME_KEEPOPEN	3
#define TIME_DESTROY	4
#define TIME_DONE	5	/* used to absorb those last few packets */
#define TIME_PROBE0	6
#define SOCK_DESTROY_TIME 1000	/* about 10 seconds			*/

#define PROT_SOCK	1024	/* Sockets 0-1023 can't be bound too unless you are superuser */

#define SHUTDOWN_MASK	3
#define RCV_SHUTDOWN	1
#define SEND_SHUTDOWN	2


extern void			destroy_sock(struct sock *sk);
extern unsigned short		get_new_socknum(struct proto *, unsigned short);
extern void			put_sock(unsigned short, struct sock *); 
extern void			release_sock(struct sock *sk);
extern struct sock		*get_sock(struct proto *, unsigned short,
					  unsigned long, unsigned short,
					  unsigned long);
extern struct sock		*get_sock_mcast(struct sock *, unsigned short,
					  unsigned long, unsigned short,
					  unsigned long);
extern struct sock		*get_sock_raw(struct sock *, unsigned short,
					  unsigned long, unsigned long);

extern struct sk_buff		*sock_wmalloc(struct sock *sk,
					      unsigned long size, int force,
					      int priority);
extern struct sk_buff		*sock_rmalloc(struct sock *sk,
					      unsigned long size, int force,
					      int priority);
extern void			sock_wfree(struct sock *sk, struct sk_buff *skb,
					   unsigned long size);
extern void			sock_rfree(struct sock *sk, struct sk_buff *skb,
					   unsigned long size);
extern unsigned long		sock_rspace(struct sock *sk);
extern unsigned long		sock_wspace(struct sock *sk);

extern int			sock_setsockopt(struct sock *sk,int level,int op,char *optval,int optlen);

extern int			sock_getsockopt(struct sock *sk,int level,int op,char *optval,int *optlen);
extern struct sk_buff 		*sock_alloc_send_skb(struct sock *skb, unsigned long size, int noblock, int *errcode);
extern int			sock_queue_rcv_skb(struct sock *sk, struct sk_buff *skb);

/* declarations from timer.c */
extern struct sock *timer_base;

void delete_timer (struct sock *);
void reset_timer (struct sock *, int, unsigned long);
void net_timer (unsigned long);


#endif	/* _SOCK_H */
