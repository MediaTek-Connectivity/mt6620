/*******************************************************************************
* Copyright (c) 2013, MediaTek Inc.
*
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License version 2 as published by the Free
* Software Foundation.
*
* Alternatively, this software may be distributed under the terms of BSD
* license.
********************************************************************************
*/

/*******************************************************************************
* THIS SOFTWARE IS PROVIDED BY MEDIATEK "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT OR FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL MEDIATEK BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
*/

/*! \file
\brief  Declaration of library functions

Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

#ifndef _MTK_WCN_CMB_STUB_H_
#define _MTK_WCN_CMB_STUB_H_

#include <linux/types.h>
typedef enum {
	COMBO_AUDIO_STATE_0 = 0,	/* 0000: BT_PCM_OFF & FM analog (line in/out) */
	COMBO_AUDIO_STATE_1 = 1,	/* 0001: BT_PCM_ON & FM analog (in/out) */
	COMBO_AUDIO_STATE_2 = 2,	/* 0010: BT_PCM_OFF & FM digital (I2S) */
	COMBO_AUDIO_STATE_3 = 3,	/* 0011: BT_PCM_ON & FM digital (I2S) (invalid in 73evb & 1.2 phone configuration) */
	COMBO_AUDIO_STATE_MAX = 4,
} COMBO_AUDIO_STATE;

typedef enum {
	COMBO_FUNC_TYPE_BT = 0,
	COMBO_FUNC_TYPE_FM = 1,
	COMBO_FUNC_TYPE_GPS = 2,
	COMBO_FUNC_TYPE_WIFI = 3,
	COMBO_FUNC_TYPE_WMT = 4,
	COMBO_FUNC_TYPE_STP = 5,
	COMBO_FUNC_TYPE_NUM = 6
} COMBO_FUNC_TYPE;

typedef enum {
	COMBO_IF_UART = 0,
	COMBO_IF_MSDC = 1,
	COMBO_IF_MAX,
} COMBO_IF;

/******************************************************************************
*					F U N C T I O N   D E C L A R A T I O N S
*******************************************************************************
*/

/* [GeorgeKuo] Stub functions for other kernel built-in modules to call.
* Keep them unchanged temporarily. Move mt_combo functions to mtk_wcn_combo.
*/
/* extern int mt_combo_audio_ctrl_ex(COMBO_AUDIO_STATE state, u32 clt_ctrl); */
static inline int mt_combo_audio_ctrl(COMBO_AUDIO_STATE state)
{
/* return mt_combo_audio_ctrl_ex(state, 1); */
	return 0;
}

/* extern int mt_combo_plt_enter_deep_idle(COMBO_IF src); */
/* extern int mt_combo_plt_exit_deep_idle(COMBO_IF src); */

/* Use new mtk_wcn_stub APIs instead of old mt_combo ones for kernel to control
* function on/off.
*/
/* extern void mtk_wcn_cmb_stub_func_ctrl (unsigned int type, unsigned int on); */
/* extern int board_sdio_ctrl (unsigned int sdio_port_num, unsigned int on); */
/* #include <mach/mt_combo.h> jake */
/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef enum {
	CMB_STUB_AIF_0 = 0,	/* 0000: BT_PCM_OFF & FM analog (line in/out) */
	CMB_STUB_AIF_1 = 1,	/* 0001: BT_PCM_ON & FM analog (in/out) */
	CMB_STUB_AIF_2 = 2,	/* 0010: BT_PCM_OFF & FM digital (I2S) */
	CMB_STUB_AIF_3 = 3,	/* 0011: BT_PCM_ON & FM digital (I2S) (invalid in 73evb & 1.2 phone configuration) */
	CMB_STUB_AIF_MAX = 4,
} CMB_STUB_AIF_X;

/*COMBO_CHIP_AUDIO_PIN_CTRL*/
typedef enum {
	CMB_STUB_AIF_CTRL_DIS = 0,
	CMB_STUB_AIF_CTRL_EN = 1,
	CMB_STUB_AIF_CTRL_MAX = 2,
} CMB_STUB_AIF_CTRL;

typedef void (*wmt_bgf_eirq_cb) (void);
typedef int (*wmt_aif_ctrl_cb) (CMB_STUB_AIF_X, CMB_STUB_AIF_CTRL);
typedef void (*wmt_func_ctrl_cb) (unsigned int, unsigned int);

typedef struct _CMB_STUB_CB_ {
	unsigned int size;	/* structure size */
	/*wmt_bgf_eirq_cb bgf_eirq_cb;*//* remove bgf_eirq_cb from stub. handle it in platform */
	wmt_aif_ctrl_cb aif_ctrl_cb;
	wmt_func_ctrl_cb func_ctrl_cb;
} CMB_STUB_CB, *P_CMB_STUB_CB;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

extern int mtk_wcn_cmb_stub_reg(P_CMB_STUB_CB p_stub_cb);
extern int mtk_wcn_cmb_stub_unreg(void);

extern int mtk_wcn_cmb_stub_aif_ctrl(CMB_STUB_AIF_X state,
				     CMB_STUB_AIF_CTRL ctrl);

/* TODO: [FixMe][GeorgeKuo]: put prototypes into mt_combo.h for board.c temporarily for non-finished porting */
/* TODO: old: rfkill->board.c->mt_combo->wmt_lib_plat */
/* TODO: new: rfkill->mtk_wcn_cmb_stub_alps->wmt_plat_alps */
#if 0
extern int mtk_wcn_cmb_stub_func_ctrl(unsigned int type, unsigned int on);
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _MTK_WCN_CMB_STUB_H_ */
