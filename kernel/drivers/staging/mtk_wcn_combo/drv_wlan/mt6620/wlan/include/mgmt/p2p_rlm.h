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

#ifndef _P2P_RLM_H
#define _P2P_RLM_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
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
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

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

VOID rlmBssInitForAP(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

BOOLEAN rlmUpdateBwByChListForAP(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

VOID
rlmUpdateParamsForAP(P_ADAPTER_T prAdapter,
		     P_BSS_INFO_T prBssInfo, BOOLEAN fgUpdateBeacon);

VOID rlmFuncInitialChannelList(IN P_ADAPTER_T prAdapter);

VOID
rlmFuncCommonChannelList(IN P_ADAPTER_T prAdapter,
			 IN P_CHANNEL_ENTRY_FIELD_T prChannelEntryII,
			 IN UINT_8 ucChannelListSize);

UINT_8
rlmFuncFindOperatingClass(IN P_ADAPTER_T prAdapter, IN UINT_8 ucChannelNum);

BOOLEAN
rlmFuncFindAvailableChannel(IN P_ADAPTER_T prAdapter,
			    IN UINT_8 ucCheckChnl,
			    IN PUINT_8 pucSuggestChannel,
			    IN BOOLEAN fgIsSocialChannel,
			    IN BOOLEAN fgIsDefaultChannel);

ENUM_CHNL_EXT_T
rlmDecideScoForAP(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

#endif
