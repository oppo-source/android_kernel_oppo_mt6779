/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 MediaTek Inc.
 */


#ifndef __MTK_SPM_IRQ_EDGE_H__
#define __MTK_SPM_IRQ_EDGE_H__

/*************************************************************
 * Define spm edge trigger interrupt list according to chip.
 * Only included by mtk_spm_irq.c
 ************************************************************/

struct edge_trigger_irq_list {
	const char *name;
	int order;
	unsigned int wakesrc;
};

static struct edge_trigger_irq_list list[] = {
	//{ "mediatek,infracfg_ao",   0,	0 },
	{ "mediatek,kp",		0,	R12_KP_IRQ_B },
	{ "mediatek,mddriver",		0,	R12_MD1_WDT_B },
	{ "mediatek,mt6779-disp_rdma0",   0,	R12_SYS_CIRQ_IRQ_B},
};

#endif /* __MTK_SPM_IRQ_EDGE_H__ */
