/*
 *	Swansea University Computer Society	NET3
 *	
 *	This file declares the constants of special use with the SLIP/CSLIP/
 *	KISS TNC driver.
 */
 
 /* SLIP的全称是Serial Line IP. 它是一种在串行线路上对IP数据包进行封装的简单形式，
 * 在RFC 1055中有详细描述。SLIP适用于家庭中每台计算机几乎都有的RS-232串行端口和
 * 告诉调制解调器接入网络。
 *
 * 下面规则描述了SLIP定义的帧格式:
 *
 * 1. IP数据报以一个称作END(0xC0)的特殊字符结束。同时，为了防止数据报到来之前
 * 线路噪声被当做数据报的内容，大多数实现在数据报的开始处也传送一个END字符
 * (如果线路上有噪声，那么END字符将结束这份错误的报文。这样当前的报文得以
 * 正确的传输，而前一个报文在提交上层时，发现其内容毫无意义而被丢弃。)
 *
 * 2. 如果IP报文中某个字符为END，那么就要连续传输两个字节0xdb和0xdc来取代它。
 * 0xdb字符被称作SLIP的ESC字符(它不同于ASCII中的ESC字符)。
 *
 * 3. 如果IP报文中某个字符为SLIP的ESC字符，那么就要连续传输两个字节0xdb和0xdd来取代它。
 */
 
#ifndef __LINUX_SLIP_H
#define __LINUX_SLIP_H

#define		SL_MODE_SLIP		0
#define		SL_MODE_CSLIP		1
#define 		SL_MODE_KISS		4

#define		SL_OPT_SIXBIT		2
#define		SL_OPT_ADAPTIVE		8


#endif
