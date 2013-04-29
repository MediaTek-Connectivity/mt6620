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

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "precomp.h"

/*******************************************************************************
*                              C O N S T A N T S
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
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to initialize variables in CNM_INFO_T.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmInit(P_ADAPTER_T prAdapter)
{
	return;
}				/* end of cnmInit() */

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to initialize variables in CNM_INFO_T.
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmUninit(P_ADAPTER_T prAdapter)
{
	return;
}				/* end of cnmUninit() */

/*----------------------------------------------------------------------------*/
/*!
* @brief Before handle the message from other module, it need to obtain
*        the Channel privilege from Channel Manager
*
* @param[in] prMsgHdr   The message need to be handled.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmChMngrRequestPrivilege(P_ADAPTER_T prAdapter, P_MSG_HDR_T prMsgHdr)
{
	P_MSG_CH_REQ_T prMsgChReq;
	P_CMD_CH_PRIVILEGE_T prCmdBody;
	WLAN_STATUS rStatus;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prMsgChReq = (P_MSG_CH_REQ_T) prMsgHdr;

	prCmdBody = (P_CMD_CH_PRIVILEGE_T)
	    cnmMemAlloc(prAdapter, RAM_TYPE_BUF, sizeof(CMD_CH_PRIVILEGE_T));
	ASSERT(prCmdBody);

/* To do: exception handle */
	if (!prCmdBody) {
		DBGLOG(CNM, ERROR,
		       ("ChReq: fail to get buf (net=%d, token=%d)\n",
			prMsgChReq->ucNetTypeIndex, prMsgChReq->ucTokenID));

		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	DBGLOG(CNM, INFO, ("ChReq net=%d token=%d b=%d c=%d s=%d\n",
			   prMsgChReq->ucNetTypeIndex, prMsgChReq->ucTokenID,
			   prMsgChReq->eRfBand, prMsgChReq->ucPrimaryChannel,
			   prMsgChReq->eRfSco));

	prCmdBody->ucNetTypeIndex = prMsgChReq->ucNetTypeIndex;
	prCmdBody->ucTokenID = prMsgChReq->ucTokenID;
	prCmdBody->ucAction = CMD_CH_ACTION_REQ;	/* Request */
	prCmdBody->ucPrimaryChannel = prMsgChReq->ucPrimaryChannel;
	prCmdBody->ucRfSco = (UINT_8) prMsgChReq->eRfSco;
	prCmdBody->ucRfBand = (UINT_8) prMsgChReq->eRfBand;
	prCmdBody->ucReqType = (UINT_8) prMsgChReq->eReqType;
	prCmdBody->ucReserved = 0;
	prCmdBody->u4MaxInterval = prMsgChReq->u4MaxInterval;
	COPY_MAC_ADDR(prCmdBody->aucBSSID, prMsgChReq->aucBSSID);

	ASSERT(prCmdBody->ucNetTypeIndex < NETWORK_TYPE_INDEX_NUM);

/* For monkey testing 20110901 */
	if (prCmdBody->ucNetTypeIndex >= NETWORK_TYPE_INDEX_NUM) {
		DBGLOG(CNM, ERROR, ("CNM: ChReq with wrong netIdx=%d\n\n",
				    prCmdBody->ucNetTypeIndex));
	}

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_CH_PRIVILEGE,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(CMD_CH_PRIVILEGE_T),	/* u4SetQueryInfoLen */
				      (PUINT_8) prCmdBody,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
	    );

	ASSERT(rStatus == WLAN_STATUS_PENDING);

	cnmMemFree(prAdapter, prCmdBody);
	cnmMemFree(prAdapter, prMsgHdr);

	return;
}				/* end of cnmChMngrRequestPrivilege() */

/*----------------------------------------------------------------------------*/
/*!
* @brief Before deliver the message to other module, it need to release
*        the Channel privilege to Channel Manager.
*
* @param[in] prMsgHdr   The message need to be delivered
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmChMngrAbortPrivilege(P_ADAPTER_T prAdapter, P_MSG_HDR_T prMsgHdr)
{
	P_MSG_CH_ABORT_T prMsgChAbort;
	P_CMD_CH_PRIVILEGE_T prCmdBody;
	WLAN_STATUS rStatus;

	ASSERT(prAdapter);
	ASSERT(prMsgHdr);

	prMsgChAbort = (P_MSG_CH_ABORT_T) prMsgHdr;

	prCmdBody = (P_CMD_CH_PRIVILEGE_T)
	    cnmMemAlloc(prAdapter, RAM_TYPE_BUF, sizeof(CMD_CH_PRIVILEGE_T));
	ASSERT(prCmdBody);

/* To do: exception handle */
	if (!prCmdBody) {
		DBGLOG(CNM, ERROR,
		       ("ChAbort: fail to get buf (net=%d, token=%d)\n",
			prMsgChAbort->ucNetTypeIndex, prMsgChAbort->ucTokenID));

		cnmMemFree(prAdapter, prMsgHdr);
		return;
	}

	DBGLOG(CNM, INFO, ("ChAbort net=%d token=%d\n",
			   prMsgChAbort->ucNetTypeIndex,
			   prMsgChAbort->ucTokenID));

	prCmdBody->ucNetTypeIndex = prMsgChAbort->ucNetTypeIndex;
	prCmdBody->ucTokenID = prMsgChAbort->ucTokenID;
	prCmdBody->ucAction = CMD_CH_ACTION_ABORT;	/* Abort */

	ASSERT(prCmdBody->ucNetTypeIndex < NETWORK_TYPE_INDEX_NUM);

/* For monkey testing 20110901 */
	if (prCmdBody->ucNetTypeIndex >= NETWORK_TYPE_INDEX_NUM) {
		DBGLOG(CNM, ERROR, ("CNM: ChAbort with wrong netIdx=%d\n\n",
				    prCmdBody->ucNetTypeIndex));
	}

	rStatus = wlanSendSetQueryCmd(prAdapter,	/* prAdapter */
				      CMD_ID_CH_PRIVILEGE,	/* ucCID */
				      TRUE,	/* fgSetQuery */
				      FALSE,	/* fgNeedResp */
				      FALSE,	/* fgIsOid */
				      NULL,	/* pfCmdDoneHandler */
				      NULL,	/* pfCmdTimeoutHandler */
				      sizeof(CMD_CH_PRIVILEGE_T),	/* u4SetQueryInfoLen */
				      (PUINT_8) prCmdBody,	/* pucInfoBuffer */
				      NULL,	/* pvSetQueryBuffer */
				      0	/* u4SetQueryBufferLen */
	    );

	ASSERT(rStatus == WLAN_STATUS_PENDING);

	cnmMemFree(prAdapter, prCmdBody);
	cnmMemFree(prAdapter, prMsgHdr);

	return;
}				/* end of cnmChMngrAbortPrivilege() */

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmChMngrHandleChEvent(P_ADAPTER_T prAdapter, P_WIFI_EVENT_T prEvent)
{
	P_EVENT_CH_PRIVILEGE_T prEventBody;
	P_MSG_CH_GRANT_T prChResp;

	ASSERT(prAdapter);
	ASSERT(prEvent);

	prEventBody = (P_EVENT_CH_PRIVILEGE_T) (prEvent->aucBuffer);
	prChResp = (P_MSG_CH_GRANT_T)
	    cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(MSG_CH_GRANT_T));
	ASSERT(prChResp);

/* To do: exception handle */
	if (!prChResp) {
		DBGLOG(CNM, ERROR,
		       ("ChGrant: fail to get buf (net=%d, token=%d)\n",
			prEventBody->ucNetTypeIndex, prEventBody->ucTokenID));

		return;
	}

	DBGLOG(CNM, INFO, ("ChGrant net=%d token=%d ch=%d sco=%d\n",
			   prEventBody->ucNetTypeIndex, prEventBody->ucTokenID,
			   prEventBody->ucPrimaryChannel,
			   prEventBody->ucRfSco));

	ASSERT(prEventBody->ucNetTypeIndex < NETWORK_TYPE_INDEX_NUM);
	ASSERT(prEventBody->ucStatus == EVENT_CH_STATUS_GRANT);

/* Decide message ID based on network and response status */
	if (prEventBody->ucNetTypeIndex == NETWORK_TYPE_AIS_INDEX) {
		prChResp->rMsgHdr.eMsgId = MID_CNM_AIS_CH_GRANT;
	}
#if CFG_ENABLE_WIFI_DIRECT
	else if ((prAdapter->fgIsP2PRegistered) &&
		 (prEventBody->ucNetTypeIndex == NETWORK_TYPE_P2P_INDEX)) {
		prChResp->rMsgHdr.eMsgId = MID_CNM_P2P_CH_GRANT;
	}
#endif
#if CFG_ENABLE_BT_OVER_WIFI
	else if (prEventBody->ucNetTypeIndex == NETWORK_TYPE_BOW_INDEX) {
		prChResp->rMsgHdr.eMsgId = MID_CNM_BOW_CH_GRANT;
	}
#endif
	else {
		cnmMemFree(prAdapter, prChResp);
		return;
	}

	prChResp->ucNetTypeIndex = prEventBody->ucNetTypeIndex;
	prChResp->ucTokenID = prEventBody->ucTokenID;
	prChResp->ucPrimaryChannel = prEventBody->ucPrimaryChannel;
	prChResp->eRfSco = (ENUM_CHNL_EXT_T) prEventBody->ucRfSco;
	prChResp->eRfBand = (ENUM_BAND_T) prEventBody->ucRfBand;
	prChResp->eReqType = (ENUM_CH_REQ_TYPE_T) prEventBody->ucReqType;
	prChResp->u4GrantInterval = prEventBody->u4GrantInterval;

	mboxSendMsg(prAdapter,
		    MBOX_ID_0, (P_MSG_HDR_T) prChResp, MSG_SEND_METHOD_BUF);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is invoked for P2P or BOW networks
*
* @param (none)
*
* @return TRUE: suggest to adopt the returned preferred channel
*         FALSE: No suggestion. Caller should adopt its preference
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
cnmPreferredChannel(P_ADAPTER_T prAdapter,
		    P_ENUM_BAND_T prBand,
		    PUINT_8 pucPrimaryChannel, P_ENUM_CHNL_EXT_T prBssSCO)
{
	P_BSS_INFO_T prBssInfo;

	ASSERT(prAdapter);
	ASSERT(prBand);
	ASSERT(pucPrimaryChannel);
	ASSERT(prBssSCO);

	prBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];

	if (RLM_NET_PARAM_VALID(prBssInfo)) {
		*prBand = prBssInfo->eBand;
		*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;
		*prBssSCO = prBssInfo->eBssSCO;

		return TRUE;
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: available channel is limited to return value
*         FALSE: no limited
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
cnmAisInfraChannelFixed(P_ADAPTER_T prAdapter,
			P_ENUM_BAND_T prBand, PUINT_8 pucPrimaryChannel)
{
#if CFG_ENABLE_WIFI_DIRECT || (CFG_ENABLE_BT_OVER_WIFI && \
CFG_BOW_LIMIT_AIS_CHNL)
	P_BSS_INFO_T prBssInfo;
#endif

#if CFG_ENABLE_WIFI_DIRECT
	if (IS_NET_ACTIVE(prAdapter, NETWORK_TYPE_P2P_INDEX) &&
	    p2pFuncIsAPMode(prAdapter->rWifiVar.prP2pFsmInfo)) {
		ASSERT(prAdapter->fgIsP2PRegistered);

		prBssInfo =
		    &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_P2P_INDEX];

		*prBand = prBssInfo->eBand;
		*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;

		return TRUE;
	}
#endif

#if CFG_ENABLE_BT_OVER_WIFI && CFG_BOW_LIMIT_AIS_CHNL
	if (IS_NET_ACTIVE(prAdapter, NETWORK_TYPE_BOW_INDEX)) {
		prBssInfo =
		    &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_BOW_INDEX];

		*prBand = prBssInfo->eBand;
		*pucPrimaryChannel = prBssInfo->ucPrimaryChannel;

		return TRUE;
	}
#endif

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID cnmAisInfraConnectNotify(P_ADAPTER_T prAdapter)
{
#if CFG_ENABLE_BT_OVER_WIFI
	P_BSS_INFO_T prAisBssInfo, prBowBssInfo;

	prAisBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];
	prBowBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_BOW_INDEX];

	if (RLM_NET_PARAM_VALID(prAisBssInfo) &&
	    RLM_NET_PARAM_VALID(prBowBssInfo)) {
		if (prAisBssInfo->eBand != prBowBssInfo->eBand ||
		    prAisBssInfo->ucPrimaryChannel !=
		    prBowBssInfo->ucPrimaryChannel) {
/* Notify BOW to do deactivation */
			bowNotifyAllLinkDisconnected(prAdapter);
		}
	}
#endif
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmAisIbssIsPermitted(P_ADAPTER_T prAdapter)
{
#if CFG_ENABLE_WIFI_DIRECT
	if (IS_NET_ACTIVE(prAdapter, NETWORK_TYPE_P2P_INDEX)) {
		return FALSE;
	}
#endif

#if CFG_ENABLE_BT_OVER_WIFI
	if (IS_NET_ACTIVE(prAdapter, NETWORK_TYPE_BOW_INDEX)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmP2PIsPermitted(P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;

	prBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];

	if (IS_BSS_ACTIVE(prBssInfo) &&
	    prBssInfo->eCurrentOPMode == OP_MODE_IBSS) {
		return FALSE;
	}
#if CFG_ENABLE_BT_OVER_WIFI
	if (IS_NET_ACTIVE(prAdapter, NETWORK_TYPE_BOW_INDEX)) {
/* Notify BOW to do deactivation */
		bowNotifyAllLinkDisconnected(prAdapter);
	}
#endif

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN cnmBowIsPermitted(P_ADAPTER_T prAdapter)
{
	P_BSS_INFO_T prBssInfo;

	prBssInfo = &prAdapter->rWifiVar.arBssInfo[NETWORK_TYPE_AIS_INDEX];

	if (IS_BSS_ACTIVE(prBssInfo) &&
	    prBssInfo->eCurrentOPMode == OP_MODE_IBSS) {
		return FALSE;
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (IS_NET_ACTIVE(prAdapter, NETWORK_TYPE_P2P_INDEX)) {
		return FALSE;
	}
#endif

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param (none)
*
* @return TRUE: permitted
*         FALSE: Not permitted
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
cnmBss40mBwPermitted(P_ADAPTER_T prAdapter,
		     ENUM_NETWORK_TYPE_INDEX_T eNetTypeIdx)
{
	P_BSS_INFO_T prBssInfo;
	UINT_8 i;

/* Note: To support real-time decision instead of current activated-time,
*       the STA roaming case shall be considered about synchronization
*       problem. Another variable fgAssoc40mBwAllowed is added to
*       represent HT capability when association
*/
	for (i = 0; i < NETWORK_TYPE_INDEX_NUM; i++) {
		if (i != (UINT_8) eNetTypeIdx) {
			prBssInfo = &prAdapter->rWifiVar.arBssInfo[i];

			if (IS_BSS_ACTIVE(prBssInfo) &&
			    (prBssInfo->fg40mBwAllowed ||
			     prBssInfo->fgAssoc40mBwAllowed)) {
				return FALSE;
			}
		}
	}

	return TRUE;
}
