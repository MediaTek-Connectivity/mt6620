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

#ifndef _MTK_WCN_CMB_HW_H_
#define _MTK_WCN_CMB_HW_H_

#include "osal_typedef.h"

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

typedef struct _PWR_SEQ_TIME_ {
	UINT32 rtcStableTime;
	UINT32 ldoStableTime;
	UINT32 rstStableTime;
	UINT32 offStableTime;
	UINT32 onStableTime;
} PWR_SEQ_TIME, *P_PWR_SEQ_TIME;

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

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

extern INT32 mtk_wcn_cmb_hw_pwr_off(VOID);
extern INT32 mtk_wcn_cmb_hw_pwr_on(VOID);
extern INT32 mtk_wcn_cmb_hw_rst(VOID);
extern INT32 mtk_wcn_cmb_hw_init(P_PWR_SEQ_TIME pPwrSeqTime);
extern INT32 mtk_wcn_cmb_hw_deinit(VOID);
extern INT32 mtk_wcn_cmb_hw_state_show(VOID);

#endif /* _MTK_WCN_CMB_HW_H_ */
