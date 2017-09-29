/*
 * Copyright (c) 2006, Chips & Media.  All rights reserved.
 *
 * Copyright (C) 2004-2013 Freescale Semiconductor, Inc.
 */

/* The following programs are the sole property of Freescale Semiconductor Inc.,
 * and contain its proprietary and confidential information. */

/*!
 * @file vpu_io.h
 *
 * @brief VPU system ioctrl definition
 *
 * @ingroup VPU
 */

#ifndef __VPU__IO__H
#define __VPU__IO__H

/*!
 * @brief  vpu memory description structure
 */
typedef struct vpu_mem_desc {
	int size;		/*!requested memory size */
	unsigned long phy_addr;	/*!physical memory address allocated */
	unsigned long cpu_addr;	/*!cpu addr for system free usage */
	unsigned long virt_uaddr;	/*!virtual user space address */
} vpu_mem_desc;

typedef struct iram_t {
        unsigned long start;
        unsigned long end;
} iram_t;

#ifndef	true
#define true	1
#endif
#ifndef	false
#define false	0
#endif

#define	VPU_IOC_MAGIC		'V'

#define	VPU_IOC_PHYMEM_ALLOC	_IO(VPU_IOC_MAGIC, 0)
#define	VPU_IOC_PHYMEM_FREE	_IO(VPU_IOC_MAGIC, 1)
#define VPU_IOC_WAIT4INT	_IO(VPU_IOC_MAGIC, 2)
#define	VPU_IOC_PHYMEM_DUMP	_IO(VPU_IOC_MAGIC, 3)
#define	VPU_IOC_REG_DUMP	_IO(VPU_IOC_MAGIC, 4)
#define	VPU_IOC_IRAM_BASE	_IO(VPU_IOC_MAGIC, 6)
#define	VPU_IOC_CLKGATE_SETTING	_IO(VPU_IOC_MAGIC, 7)
#define VPU_IOC_GET_WORK_ADDR   _IO(VPU_IOC_MAGIC, 8)
#define VPU_IOC_REQ_VSHARE_MEM  _IO(VPU_IOC_MAGIC, 9)
#define VPU_IOC_SYS_SW_RESET	_IO(VPU_IOC_MAGIC, 11)
#define VPU_IOC_GET_SHARE_MEM   _IO(VPU_IOC_MAGIC, 12)
#define VPU_IOC_QUERY_BITWORK_MEM  _IO(VPU_IOC_MAGIC, 13)
#define VPU_IOC_SET_BITWORK_MEM    _IO(VPU_IOC_MAGIC, 14)
#define VPU_IOC_PHYMEM_CHECK	_IO(VPU_IOC_MAGIC, 15)
#define VPU_IOC_LOCK_DEV	_IO(VPU_IOC_MAGIC, 16)

typedef void (*vpu_callback) (int status);

int IOSystemInit(void *callback);
int IOSystemShutdown(void);
int IOGetPhyMem(vpu_mem_desc * buff);
int IOFreePhyMem(vpu_mem_desc * buff);
int IOGetVirtMem(vpu_mem_desc * buff);
int IOFreeVirtMem(vpu_mem_desc * buff);
int IOGetVShareMem(int size);
int IOWaitForInt(int timeout_in_ms);
int IOPhyMemCheck(unsigned long phyaddr, const char *name);
int IOGetIramBase(iram_t * iram);
int IOClkGateSet(int on);
int IOGetPhyShareMem(vpu_mem_desc * buff);
int IOSysSWReset(void);
int IOLockDev(int on);

unsigned long VpuWriteReg(unsigned long addr, unsigned int data);
unsigned long VpuReadReg(unsigned long addr);

void ResetVpu(void);
int isVpuInitialized(void);

#endif
