/*
 *	Definitions for the 'struct sk_buff' memory handlers.
 *
 *	Authors:
 *		Alan Cox, <gw4pts@gw4pts.ampr.org>
 *		Florian La Roche, <rzsfl@rz.uni-sb.de>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */
 
#ifndef _LINUX_SKBUFF_H
#define _LINUX_SKBUFF_H
#include <linux/malloc.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/config.h>

#undef CONFIG_SKB_CHECK

#define HAVE_ALLOC_SKB		/* For the drivers to know */


/* 调用kfree_skb()时传入的参数，表示该skbuff属于读缓冲还是写缓冲 */
#define FREE_READ	1
#define FREE_WRITE	0


struct sk_buff_head {
  struct sk_buff		* volatile next;
  struct sk_buff		* volatile prev;
#if CONFIG_SKB_CHECK
  int				magic_debug_cookie;
#endif
};


struct sk_buff {
  struct sk_buff		* volatile next;
  struct sk_buff		* volatile prev;
#if CONFIG_SKB_CHECK
  int					magic_debug_cookie;
#endif
  struct sk_buff		* volatile link3; 	// tcp协议中，构建包重发队列
  struct sock			*sk;				// 该skb所属sock；当sock是监听套接字时，该字段用来表示已接收套接字的sock结构(参见tcp_find_established)
  volatile unsigned long	when;			//* used to compute rtt's 该数据包的发送时间	*/
  struct timeval		stamp;
  struct device		*dev;			// 接收包时表示接收设备；发送包时若不为NULL，指定发送接口
  struct sk_buff		*mem_addr;		// 指向当前sk_buff，该字段用于释放sk_buff时使用
  union {
	struct tcphdr		*th;
	struct ethhdr		*eth;
	struct iphdr		*iph;
	struct udphdr		*uh;
	unsigned char		*raw;			// 随协议层次变化，链路层时，相当于eth；网络层时，相当于iph
	unsigned long		seq;			// 针对使用tcp协议的待发送数据包而言，该字段表示"该"数据包的确认ack值
									// (数据起始+数据长度)
  } h;
  struct iphdr			*ip_hdr;			//* For IPPROTO_RAW */
  unsigned long		mem_len;		//	(sk_buff结构+数据帧)的总长度
  unsigned long 		len;				// 数据帧长度
  /* 用于分片数据包 */
  unsigned long		fraglen;			// 数据包分片个数
  struct sk_buff		*fraglist;		//* Fragment list 分片队列 */
  unsigned long		truesize;		// 含义与mem_len相同
  unsigned long 		saddr;			// 数据包发送的源IP地址
  unsigned long 		daddr;			// 数据包最终目的端IP地址
  unsigned long		raddr;			//* next hop addr 数据包下一站IP地址*/
  volatile char 			acked,			// 该数据包已经得到确认，可从重发队列中移除
					used,			// 该数据包已经被应用程序读完
					free,			// free = 1时，该数据包无论是否发送成功，在进行完发送操作后立即释放，无需缓存
					arp;				// arp = 0:MAC首部中硬件地址不确定，需要ARP协议询问对方
									// 		   在首部完成创建之前，该数据包一直处于发送缓冲队列中(device结构中的buffs
									// 		   数组元素指向某个队列以及ARP协议的某个队列中)
  unsigned char		tries,			// 数据包已经重传的次数，如果超过某个数值后，放弃发送；SYN的重传次数是3次
  					lock,			// lock表示该数据包是否正在被系统中其他部分使用
  					localroute,		// 路由时使用局域网路由还是广域网路由(如果把192.168.0.11地址的skbuff表示为广域网包，那么会被路由器直接丢掉，而不会在局域网转发)
  					pkt_type;
#define PACKET_HOST			0		/* 发往本机的数据包 */
#define PACKET_BROADCAST	1		/* 广播数据包		*/
#define PACKET_MULTICAST	2		/* 多播数据包		*/
#define PACKET_OTHERHOST	3		/* Unmatched promiscuous 该包是发往其他机器的，如果本机没有配置为转发功能，该数据包即被丢弃 */
  unsigned short		users;			//* User count - see datagram.c (and soon seqpacket.c/stream.c) 使用该包的模块数*/
  unsigned short		pkt_class;		//* For drivers that need to cache the packet type with the skbuff (new PPP) 同pkt_type */
#ifdef CONFIG_SLAVE_BALANCING
  unsigned short		in_dev_queue;	// 数据包是否正缓存于设备缓存队列中
#endif  
  unsigned long		padding[0];		// 填充字节
  unsigned char		data[0];			// 指向数据部分 
};

#define SK_WMEM_MAX	32767		// 写缓冲区最大长度
#define SK_RMEM_MAX	32767		// 读缓冲区最大长度

#ifdef CONFIG_SKB_CHECK
#define SK_FREED_SKB	0x0DE2C0DE
#define SK_GOOD_SKB	0xDEC0DED1
#define SK_HEAD_SKB		0x12231298
#endif

#ifdef __KERNEL__
/*
 *	Handling routines are only of interest to the kernel
 */

#include <asm/system.h>

#if 0
extern void			print_skb(struct sk_buff *);
#endif
extern void			kfree_skb(struct sk_buff *skb, int rw);

extern void			skb_queue_head_init(struct sk_buff_head *list);

extern void			skb_queue_head(struct sk_buff_head *list,struct sk_buff *buf);
extern void			skb_queue_tail(struct sk_buff_head *list,struct sk_buff *buf);

/* 从队列头部取下一个数据包，并从队列中移除它 */
extern struct sk_buff *		skb_dequeue(struct sk_buff_head *list);

extern void 			skb_insert(struct sk_buff *old,struct sk_buff *newsk);
extern void			skb_append(struct sk_buff *old,struct sk_buff *newsk);

/* 将指定数据包从队列中移除 */
extern void			skb_unlink(struct sk_buff *buf);


extern struct sk_buff *		skb_peek_copy(struct sk_buff_head *list);

extern struct sk_buff *		alloc_skb(unsigned int size, int priority);
extern void			kfree_skbmem(struct sk_buff *skb, unsigned size);
extern struct sk_buff *		skb_clone(struct sk_buff *skb, int priority);
extern void			skb_device_lock(struct sk_buff *skb);
extern void			skb_device_unlock(struct sk_buff *skb);
extern void			dev_kfree_skb(struct sk_buff *skb, int mode);
extern int			skb_device_locked(struct sk_buff *skb);
/*
 *	Peek an sk_buff. Unlike most other operations you _MUST_
 *	be careful with this one. A peek leaves the buffer on the
 *	list and someone else may run off with it. For an interrupt
 *	type system cli() peek the buffer copy the data and sti();
 */
 
/* 注意:peek仅从链表中取出下一个数据包，但不从链表中移除它
 * 可能有其他过程移除了那个sk_buff
 */
static __inline__ struct sk_buff *skb_peek(struct sk_buff_head *list_)
{
	struct sk_buff *list = (struct sk_buff *)list_;
	return (list->next != list)? list->next : NULL;
}

#if CONFIG_SKB_CHECK
extern int 			skb_check(struct sk_buff *skb,int,int, char *);
#define IS_SKB(skb)		skb_check((skb), 0, __LINE__,__FILE__)
#define IS_SKB_HEAD(skb)	skb_check((skb), 1, __LINE__,__FILE__)
#else
#define IS_SKB(skb)		
#define IS_SKB_HEAD(skb)	

extern __inline__ void skb_queue_head_init(struct sk_buff_head *list)
{
	list->prev = (struct sk_buff *)list;
	list->next = (struct sk_buff *)list;
}

/*
 *	Insert an sk_buff at the start of a list.
 */

extern __inline__ void skb_queue_head(struct sk_buff_head *list_,struct sk_buff *newsk)
{
	unsigned long flags;
	struct sk_buff *list = (struct sk_buff *)list_;

	save_flags(flags);
	cli();
	newsk->next = list->next;
	newsk->prev = list;
	newsk->next->prev = newsk;
	newsk->prev->next = newsk;
	restore_flags(flags);
}

/*
 *	Insert an sk_buff at the end of a list.
 */

extern __inline__ void skb_queue_tail(struct sk_buff_head *list_, struct sk_buff *newsk)
{
	unsigned long flags;
	struct sk_buff *list = (struct sk_buff *)list_;

	save_flags(flags);
	cli();

	newsk->next = list;
	newsk->prev = list->prev;

	newsk->next->prev = newsk;
	newsk->prev->next = newsk;

	restore_flags(flags);
}

/*
 *	Remove an sk_buff from a list. This routine is also interrupt safe
 *	so you can grab read and free buffers as another process adds them.
 */

extern __inline__ struct sk_buff *skb_dequeue(struct sk_buff_head *list_)
{
	long flags;
	struct sk_buff *result;
	struct sk_buff *list = (struct sk_buff *)list_;

	save_flags(flags);
	cli();

	result = list->next;
	if (result == list) {
		restore_flags(flags);
		return NULL;
	}

	result->next->prev = list;
	list->next = result->next;

	result->next = NULL;
	result->prev = NULL;

	restore_flags(flags);

	return result;
}

/*
 *	Insert a packet before another one in a list.
 */

extern __inline__ void skb_insert(struct sk_buff *old, struct sk_buff *newsk)
{
	unsigned long flags;

	save_flags(flags);
	cli();
	newsk->next = old;
	newsk->prev = old->prev;
	old->prev = newsk;
	newsk->prev->next = newsk;

	restore_flags(flags);
}

/*
 *	Place a packet after a given packet in a list.
 */

extern __inline__ void skb_append(struct sk_buff *old, struct sk_buff *newsk)
{
	unsigned long flags;

	save_flags(flags);
	cli();

	newsk->prev = old;
	newsk->next = old->next;
	newsk->next->prev = newsk;
	old->next = newsk;

	restore_flags(flags);
}

/*
 *	Remove an sk_buff from its list. Works even without knowing the list it
 *	is sitting on, which can be handy at times. It also means that THE LIST
 *	MUST EXIST when you unlink. Thus a list must have its contents unlinked
 *	_FIRST_.
 */

extern __inline__ void skb_unlink(struct sk_buff *skb)
{
	unsigned long flags;

	save_flags(flags);
	cli();

	if(skb->prev && skb->next)
	{
		skb->next->prev = skb->prev;
		skb->prev->next = skb->next;
		skb->next = NULL;
		skb->prev = NULL;
	}
	restore_flags(flags);
}

#endif

extern struct sk_buff *		skb_recv_datagram(struct sock *sk,unsigned flags,int noblock, int *err);
extern int			datagram_select(struct sock *sk, int sel_type, select_table *wait);
extern void			skb_copy_datagram(struct sk_buff *from, int offset, char *to,int size);
extern void			skb_free_datagram(struct sk_buff *skb);

#endif	/* __KERNEL__ */
#endif	/* _LINUX_SKBUFF_H */
