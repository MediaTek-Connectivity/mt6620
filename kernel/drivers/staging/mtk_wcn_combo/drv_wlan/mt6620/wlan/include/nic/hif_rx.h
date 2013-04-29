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

#ifndef _HIF_RX_H
#define _HIF_RX_H

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
/*! HIF_RX_HEADER_T */
/* DW 0, Byte 1 */
#define HIF_RX_HDR_PACKET_TYPE_MASK      BITS(0, 1)

/* DW 1, Byte 0 */
#define HIF_RX_HDR_HEADER_LEN            BITS(2, 7)
#define HIF_RX_HDR_HEADER_LEN_OFFSET     2
#define HIF_RX_HDR_HEADER_OFFSET_MASK    BITS(0, 1)

/* DW 1, Byte 1 */
#define HIF_RX_HDR_80211_HEADER_FORMAT   BIT(0)
#define HIF_RX_HDR_DO_REORDER            BIT(1)
#define HIF_RX_HDR_PAL                   BIT(2)
#define HIF_RX_HDR_TCL                   BIT(3)
#define HIF_RX_HDR_NETWORK_IDX_MASK      BITS(4, 7)
#define HIF_RX_HDR_NETWORK_IDX_OFFSET    4

/* DW 1, Byte 2, 3 */
#define HIF_RX_HDR_SEQ_NO_MASK           BITS(0, 11)
#define HIF_RX_HDR_TID_MASK              BITS(12, 14)
#define HIF_RX_HDR_TID_OFFSET            12
#define HIF_RX_HDR_BAR_FRAME             BIT(15)

#define HIF_RX_HDR_FLAG_AMP_WDS             BIT(0)
#define HIF_RX_HDR_FLAG_802_11_FORMAT       BIT(1)
#define HIF_RX_HDR_FLAG_BAR_FRAME           BIT(2)
#define HIF_RX_HDR_FLAG_DO_REORDERING       BIT(3)
#define HIF_RX_HDR_FLAG_CTRL_WARPPER_FRAME  BIT(4)

#define HIF_RX_HW_APPENDED_LEN              4

/* For DW 2, Byte 3 - ucHwChannelNum */
#define HW_CHNL_NUM_MAX_2G4                 14
#define HW_CHNL_NUM_MAX_4G_5G               (255 - HW_CHNL_NUM_MAX_2G4)

/*******************************************************************************
*                         D A T A   T Y P E S
********************************************************************************
*/

typedef struct _HIF_RX_HEADER_T {
	UINT_16 u2PacketLen;
	UINT_16 u2PacketType;
	UINT_8 ucHerderLenOffset;
	UINT_8 uc80211_Reorder_PAL_TCL;
	UINT_16 u2SeqNoTid;
	UINT_8 ucStaRecIdx;
	UINT_8 ucRcpi;
	UINT_8 ucHwChannelNum;
	UINT_8 ucReserved;
} HIF_RX_HEADER_T, *P_HIF_RX_HEADER_T;

typedef enum _ENUM_HIF_RX_PKT_TYPE_T {
	HIF_RX_PKT_TYPE_DATA = 0,
	HIF_RX_PKT_TYPE_EVENT,
	HIF_RX_PKT_TYPE_TX_LOOPBACK,
	HIF_RX_PKT_TYPE_MANAGEMENT,
	HIF_RX_PKT_TYPE_NUM
} ENUM_HIF_RX_PKT_TYPE_T, *P_ENUM_HIF_RX_PKT_TYPE_T;

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
#define HIF_RX_HDR_SIZE        sizeof(HIF_RX_HEADER_T)

#define HIF_RX_HDR_GET_80211_FLAG(_prHifRxHdr) \
(((((_prHifRxHdr)->uc80211_Reorder_PAL_TCL) & \
HIF_RX_HDR_80211_HEADER_FORMAT) ? TRUE : FALSE))
#define HIF_RX_HDR_GET_REORDER_FLAG(_prHifRxHdr) \
(((((_prHifRxHdr)->uc80211_Reorder_PAL_TCL) & \
HIF_RX_HDR_DO_REORDER) ? TRUE : FALSE))
#define HIF_RX_HDR_GET_PAL_FLAG(_prHifRxHdr) \
(((((_prHifRxHdr)->uc80211_Reorder_PAL_TCL) & \
HIF_RX_HDR_PAL) ? TRUE : FALSE))
#define HIF_RX_HDR_GET_TCL_FLAG(_prHifRxHdr) \
(((((_prHifRxHdr)->uc80211_Reorder_PAL_TCL) & \
HIF_RX_HDR_TCL) ? TRUE : FALSE))
#define HIF_RX_HDR_GET_NETWORK_IDX(_prHifRxHdr)	\
((((_prHifRxHdr)->uc80211_Reorder_PAL_TCL) & \
HIF_RX_HDR_NETWORK_IDX_MASK) \
>> HIF_RX_HDR_NETWORK_IDX_OFFSET)

#define HIF_RX_HDR_GET_TID(_prHifRxHdr)	\
((((_prHifRxHdr)->u2SeqNoTid) & HIF_RX_HDR_TID_MASK) \
>> HIF_RX_HDR_TID_OFFSET)
#define HIF_RX_HDR_GET_SN(_prHifRxHdr) \
(((_prHifRxHdr)->u2SeqNoTid) & HIF_RX_HDR_SEQ_NO_MASK)
#define HIF_RX_HDR_GET_BAR_FLAG(_prHifRxHdr) \
(((((_prHifRxHdr)->u2SeqNoTid) & HIF_RX_HDR_BAR_FRAME) ? TRUE : FALSE))

#define HIF_RX_HDR_GET_CHNL_NUM(_prHifRxHdr) \
((((_prHifRxHdr)->ucHwChannelNum) > HW_CHNL_NUM_MAX_4G_5G) ? \
(((_prHifRxHdr)->ucHwChannelNum) - HW_CHNL_NUM_MAX_4G_5G) : \
((_prHifRxHdr)->ucHwChannelNum))

/* To do: support more bands other than 2.4G and 5G */
#define HIF_RX_HDR_GET_RF_BAND(_prHifRxHdr) \
((((_prHifRxHdr)->ucHwChannelNum) <= HW_CHNL_NUM_MAX_2G4) ? \
BAND_2G4 : BAND_5G)

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
__KAL_INLINE__ VOID hifDataTypeCheck(VOID);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/* Kevin: we don't have to call following function to inspect the data structure.
* It will check automatically while at compile time.
* We'll need this for porting driver to different RTOS.
*/
__KAL_INLINE__ VOID hifDataTypeCheck(VOID)
{
	DATA_STRUC_INSPECTING_ASSERT(sizeof(HIF_RX_HEADER_T) == 12);

	return;
}

#endif
