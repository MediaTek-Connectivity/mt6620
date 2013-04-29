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

#ifndef _RLM_H
#define _RLM_H

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
#define ELEM_EXT_CAP_DEFAULT_VAL \
(ELEM_EXT_CAP_20_40_COEXIST_SUPPORT /*| ELEM_EXT_CAP_PSMP_CAP*/)

#if CFG_SUPPORT_RX_STBC
#define FIELD_HT_CAP_INFO_RX_STBC   HT_CAP_INFO_RX_STBC_1_SS
#else
#define FIELD_HT_CAP_INFO_RX_STBC   HT_CAP_INFO_RX_STBC_NO_SUPPORTED
#endif

#if CFG_SUPPORT_RX_SGI
#define FIELD_HT_CAP_INFO_SGI_20M   HT_CAP_INFO_SHORT_GI_20M
#define FIELD_HT_CAP_INFO_SGI_40M   HT_CAP_INFO_SHORT_GI_40M
#else
#define FIELD_HT_CAP_INFO_SGI_20M   0
#define FIELD_HT_CAP_INFO_SGI_40M   0
#endif

#if CFG_SUPPORT_RX_HT_GF
#define FIELD_HT_CAP_INFO_HT_GF     HT_CAP_INFO_HT_GF
#else
#define FIELD_HT_CAP_INFO_HT_GF     0
#endif

#define HT_CAP_INFO_DEFAULT_VAL	\
(HT_CAP_INFO_SUP_CHNL_WIDTH | FIELD_HT_CAP_INFO_HT_GF |	\
FIELD_HT_CAP_INFO_SGI_20M | FIELD_HT_CAP_INFO_SGI_40M | \
FIELD_HT_CAP_INFO_RX_STBC | HT_CAP_INFO_DSSS_CCK_IN_40M)

#define AMPDU_PARAM_DEFAULT_VAL	\
(AMPDU_PARAM_MAX_AMPDU_LEN_64K | AMPDU_PARAM_MSS_NO_RESTRICIT)

#define SUP_MCS_TX_DEFAULT_VAL \
SUP_MCS_TX_SET_DEFINED		/* TX defined and TX/RX equal (TBD) */

#if CFG_SUPPORT_MFB
#define FIELD_HT_EXT_CAP_MFB    HT_EXT_CAP_MCS_FEEDBACK_BOTH
#else
#define FIELD_HT_EXT_CAP_MFB    HT_EXT_CAP_MCS_FEEDBACK_NO_FB
#endif

#if CFG_SUPPORT_RX_RDG
#define FIELD_HT_EXT_CAP_RDR    HT_EXT_CAP_RD_RESPONDER
#else
#define FIELD_HT_EXT_CAP_RDR    0
#endif

#if CFG_SUPPORT_MFB || CFG_SUPPORT_RX_RDG
#define FIELD_HT_EXT_CAP_HTC    HT_EXT_CAP_HTC_SUPPORT
#else
#define FIELD_HT_EXT_CAP_HTC    0
#endif

#define HT_EXT_CAP_DEFAULT_VAL \
(HT_EXT_CAP_PCO | HT_EXT_CAP_PCO_TRANS_TIME_NONE | \
FIELD_HT_EXT_CAP_MFB | FIELD_HT_EXT_CAP_HTC | \
FIELD_HT_EXT_CAP_RDR)

#define TX_BEAMFORMING_CAP_DEFAULT_VAL              0
#define ASEL_CAP_DEFAULT_VAL                        0

/* Define bandwidth from user setting */
#define CONFIG_BW_20_40M            0
#define CONFIG_BW_20M               1	/* 20MHz only */

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

/* It is used for RLM module to judge if specific network is valid
* Note: Ad-hoc mode of AIS is not included now. (TBD)
*/
#define RLM_NET_PARAM_VALID(_prBssInfo)	\
(IS_BSS_ACTIVE(_prBssInfo) && \
((_prBssInfo)->eConnectionState == PARAM_MEDIA_STATE_CONNECTED || \
(_prBssInfo)->eCurrentOPMode == OP_MODE_ACCESS_POINT || \
(_prBssInfo)->eCurrentOPMode == OP_MODE_IBSS || \
RLM_NET_IS_BOW(_prBssInfo)) \
)

#define RLM_NET_IS_11N(_prBssInfo) \
((_prBssInfo)->ucPhyTypeSet & PHY_TYPE_SET_802_11N)
#define RLM_NET_IS_11GN(_prBssInfo) \
((_prBssInfo)->ucPhyTypeSet & PHY_TYPE_SET_802_11GN)

/* This macro is used to sweep all 3 networks */
#define RLM_NET_FOR_EACH(_ucNetIdx) \
for ((_ucNetIdx) = 0; \
(_ucNetIdx) < NETWORK_TYPE_INDEX_NUM; \
(_ucNetIdx)++)

/* This macro is used to sweep all networks excluding BOW */
#if CFG_ENABLE_BT_OVER_WIFI
/* Note: value of enum NETWORK_TYPE_BOW_INDEX is validated in
*       rlmStuctureCheck().
*/
#define RLM_NET_FOR_EACH_NO_BOW(_ucNetIdx) \
for ((_ucNetIdx) = 0; \
(_ucNetIdx) < NETWORK_TYPE_BOW_INDEX; \
(_ucNetIdx)++)

#define RLM_NET_IS_BOW(_prBssInfo) \
((_prBssInfo)->ucNetTypeIndex == NETWORK_TYPE_BOW_INDEX)

#else
#define RLM_NET_FOR_EACH_NO_BOW(_ucNetIdx)  RLM_NET_FOR_EACH(_ucNetIdx)
#define RLM_NET_IS_BOW(_prBssInfo)          (FALSE)

#endif /* end of CFG_ENABLE_BT_OVER_WIFI */

/* The bandwidth modes are not used anymore. They represent if AP
* can use 20/40 bandwidth, not all modes. (20110411)
*/
#define RLM_AP_IS_BW_40_ALLOWED(_prAdapter, _prBssInfo)	\
(((_prBssInfo)->eBand == BAND_2G4 && \
(_prAdapter)->rWifiVar.rConnSettings.uc2G4BandwidthMode \
== CONFIG_BW_20_40M) || \
((_prBssInfo)->eBand == BAND_5G && \
(_prAdapter)->rWifiVar.rConnSettings.uc5GBandwidthMode \
== CONFIG_BW_20_40M))

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
VOID rlmFsmEventInit(P_ADAPTER_T prAdapter);

VOID rlmFsmEventUninit(P_ADAPTER_T prAdapter);

VOID rlmReqGenerateHtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo);

VOID rlmReqGenerateExtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo);

VOID rlmRspGenerateHtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo);

VOID rlmRspGenerateExtCapIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo);

VOID rlmRspGenerateHtOpIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo);

VOID rlmRspGenerateErpIE(P_ADAPTER_T prAdapter, P_MSDU_INFO_T prMsduInfo);

VOID
rlmProcessBcn(P_ADAPTER_T prAdapter,
	      P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength);

VOID
rlmProcessAssocRsp(P_ADAPTER_T prAdapter,
		   P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength);

VOID
rlmFillSyncCmdParam(P_CMD_SET_BSS_RLM_PARAM_T prCmdBody,
		    P_BSS_INFO_T prBssInfo);

VOID rlmSyncOperationParams(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

VOID rlmBssInitForAPandIbss(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

VOID
rlmProcessAssocReq(P_ADAPTER_T prAdapter,
		   P_SW_RFB_T prSwRfb, PUINT_8 pucIE, UINT_16 u2IELength);

VOID rlmBssAborted(P_ADAPTER_T prAdapter, P_BSS_INFO_T prBssInfo);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#ifndef _lint
__KAL_INLINE__ VOID rlmDataTypeCheck(VOID)
{
#if CFG_ENABLE_BT_OVER_WIFI
	DATA_STRUC_INSPECTING_ASSERT(NETWORK_TYPE_AIS_INDEX <
				     NETWORK_TYPE_BOW_INDEX);

#if CFG_ENABLE_WIFI_DIRECT
	DATA_STRUC_INSPECTING_ASSERT(NETWORK_TYPE_P2P_INDEX <
				     NETWORK_TYPE_BOW_INDEX);
#endif
#endif

	return;
}
#endif /* _lint */

#endif /* _RLM_H */
