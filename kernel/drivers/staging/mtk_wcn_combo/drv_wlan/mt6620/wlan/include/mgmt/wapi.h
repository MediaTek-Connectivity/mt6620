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

#ifndef _WAPI_H
#define _WAPI_H

#if CFG_SUPPORT_WAPI

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
#define WAPI_CIPHER_SUITE_WPI           0x01721400	/* WPI_SMS4 */
#define WAPI_AKM_SUITE_802_1X           0x01721400	/* WAI */
#define WAPI_AKM_SUITE_PSK              0x02721400	/* WAI_PSK */

#define ELEM_ID_WAPI                    68	/* WAPI IE */

#define WAPI_IE(fp)                     ((P_WAPI_INFO_ELEM_T)fp)

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

VOID wapiGenerateWAPIIE(IN P_ADAPTER_T prAdapter, IN P_MSDU_INFO_T prMsduInfo);

BOOLEAN
wapiParseWapiIE(IN P_WAPI_INFO_ELEM_T prInfoElem, OUT P_WAPI_INFO_T prWapiInfo);

BOOLEAN
wapiPerformPolicySelection(IN P_ADAPTER_T prAdapter, IN P_BSS_DESC_T prBss);

/* BOOLEAN */
/* wapiUpdateTxKeyIdx ( */
/*    IN  P_STA_RECORD_T     prStaRec, */
/*    IN  UINT_8             ucWlanIdx */
/*    ); */

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#endif
#endif /* _WAPI_H */
