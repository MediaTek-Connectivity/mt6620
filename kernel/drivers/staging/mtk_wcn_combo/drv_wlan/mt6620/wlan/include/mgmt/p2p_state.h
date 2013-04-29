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

#ifndef _P2P_STATE_H
#define _P2P_STATE_H

BOOLEAN
p2pStateInit_IDLE(IN P_ADAPTER_T prAdapter,
		  IN P_P2P_FSM_INFO_T prP2pFsmInfo,
		  IN P_BSS_INFO_T prP2pBssInfo,
		  OUT P_ENUM_P2P_STATE_T peNextState);

VOID
p2pStateAbort_IDLE(IN P_ADAPTER_T prAdapter,
		   IN P_P2P_FSM_INFO_T prP2pFsmInfo,
		   IN ENUM_P2P_STATE_T eNextState);

VOID
p2pStateInit_SCAN(IN P_ADAPTER_T prAdapter, IN P_P2P_FSM_INFO_T prP2pFsmInfo);

VOID
p2pStateAbort_SCAN(IN P_ADAPTER_T prAdapter,
		   IN P_P2P_FSM_INFO_T prP2pFsmInfo,
		   IN ENUM_P2P_STATE_T eNextState);

VOID
p2pStateInit_AP_CHANNEL_DETECT(IN P_ADAPTER_T prAdapter,
			       IN P_P2P_FSM_INFO_T prP2pFsmInfo);

VOID
p2pStateAbort_AP_CHANNEL_DETECT(IN P_ADAPTER_T prAdapter,
				IN P_P2P_FSM_INFO_T prP2pFsmInfo,
				IN P_P2P_SPECIFIC_BSS_INFO_T
				prP2pSpecificBssInfo,
				IN ENUM_P2P_STATE_T eNextState);

VOID
p2pStateInit_CHNL_ON_HAND(IN P_ADAPTER_T prAdapter,
			  IN P_BSS_INFO_T prP2pBssInfo,
			  IN P_P2P_FSM_INFO_T prP2pFsmInfo);

VOID
p2pStateAbort_CHNL_ON_HAND(IN P_ADAPTER_T prAdapter,
			   IN P_P2P_FSM_INFO_T prP2pFsmInfo,
			   IN P_BSS_INFO_T prP2pBssInfo,
			   IN ENUM_P2P_STATE_T eNextState);

VOID
p2pStateAbort_REQING_CHANNEL(IN P_ADAPTER_T prAdapter,
			     IN P_P2P_FSM_INFO_T prP2pFsmInfo,
			     IN ENUM_P2P_STATE_T eNextState);

VOID
p2pStateInit_GC_JOIN(IN P_ADAPTER_T prAdapter,
		     IN P_P2P_FSM_INFO_T prP2pFsmInfo,
		     IN P_BSS_INFO_T prP2pBssInfo,
		     IN P_P2P_JOIN_INFO_T prJoinInfo,
		     IN P_BSS_DESC_T prBssDesc);

VOID
p2pStateAbort_GC_JOIN(IN P_ADAPTER_T prAdapter,
		      IN P_P2P_FSM_INFO_T prP2pFsmInfo,
		      IN P_P2P_JOIN_INFO_T prJoinInfo,
		      IN ENUM_P2P_STATE_T eNextState);

#endif
