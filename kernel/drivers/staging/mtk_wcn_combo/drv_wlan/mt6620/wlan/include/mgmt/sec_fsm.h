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
#ifndef _SEC_FSM_H
#define _SEC_FSM_H

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

/* TKIP CounterMeasure interval for Rejoin to Network. */
#define COUNTER_MEASURE_TIMEOUT_INTERVAL_SEC        60

/* Timeout to wait the EAPoL Error Report frame Send out. */
#define EAPOL_REPORT_SEND_TIMEOUT_INTERVAL_SEC       1

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

typedef UINT_32 SEC_STATUS, *P_SEC_STATUS;

#if 0
/* WPA2 PMKID candicate structure */
typedef struct _PMKID_CANDICATE_T {
	UINT_8 aucBssid[MAC_ADDR_LEN];	/* MAC address */
	UINT_32 u4PreAuthFlags;
} PMKID_CANDICATE_T, *P_PMKID_CANDICATE_T;
#endif

typedef SEC_STATUS(*PFN_SEC_FSM_STATE_HANDLER) (VOID);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define SEC_STATE_TRANSITION_FLAG   fgIsTransition
#define SEC_NEXT_STATE_VAR          eNextState

#define SEC_STATE_TRANSITION(prAdapter, prSta, eFromState, eToState) \
{ secFsmTrans_ ## eFromState ## _to_ ## eToState(prAdapter, prSta); \
SEC_NEXT_STATE_VAR = SEC_STATE_ ## eToState; \
SEC_STATE_TRANSITION_FLAG = (BOOLEAN)TRUE; \
}

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*--------------------------------------------------------------*/
/* Routines to handle the sec check                             */
/*--------------------------------------------------------------*/
/***** Routines in sec_fsm.c *****/
VOID secFsmInit(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventInit(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventStart(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEventAbort(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

BOOLEAN
secFsmEventPTKInstalled(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID secFsmEvent2ndEapolTx(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID
secFsmEvent4ndEapolTxDone(IN P_ADAPTER_T prAdapter, IN P_STA_RECORD_T prSta);

VOID
secFsmEventEapolTxDone(IN P_ADAPTER_T prAdapter,
		       IN P_STA_RECORD_T prStaRec,
		       IN ENUM_TX_RESULT_CODE_T rTxDoneStatus);

VOID secFsmEventEapolTxTimeout(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Parm);

VOID
secFsmEventDeauthTxDone(IN P_ADAPTER_T prAdapter,
			IN P_MSDU_INFO_T prMsduInfo,
			IN ENUM_TX_RESULT_CODE_T rTxDoneStatus);

VOID
secFsmEventStartCounterMeasure(IN P_ADAPTER_T prAdapter,
			       IN P_STA_RECORD_T prSta);

VOID
secFsmEventEndOfCounterMeasure(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Parm);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#endif /* _SEC_FSM_H */
