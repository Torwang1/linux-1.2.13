/* interrupt.h */
#ifndef _LINUX_INTERRUPT_H
#define _LINUX_INTERRUPT_H

#include <linux/linkage.h>
#include <asm/bitops.h>

struct bh_struct {
	void (*routine)(void *);
	void *data;
};

/* 位图表示法，bh_active表示bh_base[32]中有哪些bh处于活跃状态
 * 参见:mark_bh()
 */
extern unsigned long bh_active;
/* 位图掩码/使能位
 * 某位为0表示屏蔽该"下半部"，此时及时该下半部处于活跃状态，也不会被系统执行
 * 某位为1表示激活该位对应的下半部
 * 参见:enable_bh() disable_bh()
 */
extern unsigned long bh_mask;
/* 系统目前使用了前8个元素，如enum定义 */
extern struct bh_struct bh_base[32];
/*
 * do_bottom_half() runs at normal kernel priority: all interrupts
 * enabled.  do_bottom_half() is atomic with respect to itself: a
 * bottom_half handler need not be re-entrant.
 */
asmlinkage void do_bottom_half(void);

/* Who gets which entry in bh_base.  Things which will occur most often
   should come first - in which case NET should be up the top with SERIAL/TQUEUE! */
   
enum {
	TIMER_BH = 0,		// timer_bh(kernel/sched.c)
	CONSOLE_BH,
	TQUEUE_BH,			// tqueue_bh(kernel/sched.c)			
	SERIAL_BH,
	NET_BH,                  	// net_bh(net/dev.c)
	IMMEDIATE_BH,
	KEYBOARD_BH,		// immediate_bh(kernel/sched.c)
	CYCLADES_BH
};

/* 标记bh_base中那个bh被激活 */
extern inline void mark_bh(int nr)
{
	set_bit(nr, &bh_active);
}

extern inline void disable_bh(int nr)
{
	clear_bit(nr, &bh_mask);
}

extern inline void enable_bh(int nr)
{
	set_bit(nr, &bh_mask);
}

extern inline void start_bh_atomic(void)
{
	intr_count++;
}

extern inline void end_bh_atomic(void)
{
	if (intr_count == 1 && (bh_active & bh_mask))
		do_bottom_half();
	intr_count--;
}

/*
 * Autoprobing for irqs:
 *
 * probe_irq_on() and probe_irq_off() provide robust primitives
 * for accurate IRQ probing during kernel initialization.  They are
 * reasonably simple to use, are not "fooled" by spurious interrupts,
 * and, unlike other attempts at IRQ probing, they do not get hung on
 * stuck interrupts (such as unused PS2 mouse interfaces on ASUS boards).
 *
 * For reasonably foolproof probing, use them as follows:
 *
 * 1. clear and/or mask the device's internal interrupt.
 * 2. sti();
 * 3. irqs = probe_irq_on();      // "take over" all unassigned idle IRQs
 * 4. enable the device and cause it to trigger an interrupt.
 * 5. wait for the device to interrupt, using non-intrusive polling or a delay.
 * 6. irq = probe_irq_off(irqs);  // get IRQ number, 0=none, negative=multiple
 * 7. service the device to clear its pending interrupt.
 * 8. loop again if paranoia is required.
 *
 * probe_irq_on() returns a mask of snarfed irq's.
 *
 * probe_irq_off() takes the mask as a parameter,
 * and returns the irq number which occurred,
 * or zero if none occurred, or a negative irq number
 * if more than one irq occurred.
 */
extern unsigned int probe_irq_on(void);	/* returns 0 on failure */
extern int probe_irq_off(unsigned int); /* returns 0 or negative on failure */

#endif
