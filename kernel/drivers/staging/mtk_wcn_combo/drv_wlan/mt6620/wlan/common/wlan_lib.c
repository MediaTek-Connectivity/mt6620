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
#include "mgmt/ais_fsm.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/* 6.1.1.2 Interpretation of priority parameter in MAC service primitives */
/* Static convert the Priority Parameter/TID(User Priority/TS Identifier) to Traffic Class */
const UINT_8 aucPriorityParam2TC[] = {
	TC1_INDEX,
	TC0_INDEX,
	TC0_INDEX,
	TC1_INDEX,
	TC2_INDEX,
	TC2_INDEX,
	TC3_INDEX,
	TC3_INDEX
};

#if QM_TEST_MODE
extern QUE_MGT_T g_rQM;
#endif
/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef struct _CODE_MAPPING_T {
	UINT_32 u4RegisterValue;
	INT_32 u4TxpowerOffset;
} CODE_MAPPING_T, *P_CODE_MAPPING_T;

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
BOOLEAN fgIsBusAccessFailed = FALSE;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define SIGNED_EXTEND(n, _sValue) \
(((_sValue) & BIT((n) - 1)) ? ((_sValue) | BITS(n, 31)) : \
((_sValue) & ~BITS(n, 31)))

/* TODO: Check */
/* OID set handlers without the need to access HW register */
PFN_OID_HANDLER_FUNC apfnOidSetHandlerWOHwAccess[] = {
	wlanoidSetChannel,
	wlanoidSetBeaconInterval,
	wlanoidSetAtimWindow,
	wlanoidSetFrequency,
};

/* TODO: Check */
/* OID query handlers without the need to access HW register */
PFN_OID_HANDLER_FUNC apfnOidQueryHandlerWOHwAccess[] = {
	wlanoidQueryBssid,
	wlanoidQuerySsid,
	wlanoidQueryInfrastructureMode,
	wlanoidQueryAuthMode,
	wlanoidQueryEncryptionStatus,
	wlanoidQueryPmkid,
	wlanoidQueryNetworkTypeInUse,
	wlanoidQueryBssidList,
	wlanoidQueryAcpiDevicePowerState,
	wlanoidQuerySupportedRates,
	wlanoidQueryDesiredRates,
	wlanoidQuery802dot11PowerSaveProfile,
	wlanoidQueryBeaconInterval,
	wlanoidQueryAtimWindow,
	wlanoidQueryFrequency,
};

/* OID set handlers allowed in RF test mode */
PFN_OID_HANDLER_FUNC apfnOidSetHandlerAllowedInRFTest[] = {
	wlanoidRftestSetTestMode,
	wlanoidRftestSetAbortTestMode,
	wlanoidRftestSetAutoTest,
	wlanoidSetMcrWrite,
	wlanoidSetEepromWrite
};

/* OID query handlers allowed in RF test mode */
PFN_OID_HANDLER_FUNC apfnOidQueryHandlerAllowedInRFTest[] = {
	wlanoidRftestQueryAutoTest,
	wlanoidQueryMcrRead,
	wlanoidQueryEepromRead
}

;

PFN_OID_HANDLER_FUNC apfnOidWOTimeoutCheck[] = {
	wlanoidRftestSetTestMode,
	wlanoidRftestSetAbortTestMode,
	wlanoidSetAcpiDevicePowerState,
};

/*******************************************************************************
*                                 M A C R O S
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
/*----------------------------------------------------------------------------*/
/*!
* \brief This is a private routine, which is used to check if HW access is needed
*        for the OID query/ set handlers.
*
* \param[IN] pfnOidHandler Pointer to the OID handler.
* \param[IN] fgSetInfo     It is a Set information handler.
*
* \retval TRUE This function needs HW access
* \retval FALSE This function does not need HW access
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanIsHandlerNeedHwAccess(IN PFN_OID_HANDLER_FUNC pfnOidHandler,
			  IN BOOLEAN fgSetInfo)
{
	PFN_OID_HANDLER_FUNC *apfnOidHandlerWOHwAccess;
	UINT_32 i;
	UINT_32 u4NumOfElem;

	if (fgSetInfo) {
		apfnOidHandlerWOHwAccess = apfnOidSetHandlerWOHwAccess;
		u4NumOfElem = sizeof(apfnOidSetHandlerWOHwAccess) /
		    sizeof(PFN_OID_HANDLER_FUNC);
	} else {
		apfnOidHandlerWOHwAccess = apfnOidQueryHandlerWOHwAccess;
		u4NumOfElem = sizeof(apfnOidQueryHandlerWOHwAccess) /
		    sizeof(PFN_OID_HANDLER_FUNC);
	}

	for (i = 0; i < u4NumOfElem; i++) {
		if (apfnOidHandlerWOHwAccess[i] == pfnOidHandler) {
			return FALSE;
		}
	}

	return TRUE;
}				/* wlanIsHandlerNeedHwAccess */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to set flag for later handling card
*        ejected event.
*
* \param[in] prAdapter Pointer to the Adapter structure.
*
* \return (none)
*
* \note When surprised removal happens, Glue layer should invoke this
*       function to notify WPDD not to do any hw access.
*/
/*----------------------------------------------------------------------------*/
VOID wlanCardEjected(IN P_ADAPTER_T prAdapter)
{
	DEBUGFUNC("wlanCardEjected");
/* INITLOG(("\n")); */

	ASSERT(prAdapter);

/* mark that the card is being ejected, NDIS will shut us down soon */
	nicTxRelease(prAdapter);
}				/* wlanCardEjected */

/*----------------------------------------------------------------------------*/
/*!
* \brief Create adapter object
*
* \param prAdapter This routine is call to allocate the driver software objects.
*                  If fails, return NULL.
* \retval NULL If it fails, NULL is returned.
* \retval NOT NULL If the adapter was initialized successfully.
*/
/*----------------------------------------------------------------------------*/
P_ADAPTER_T wlanAdapterCreate(IN P_GLUE_INFO_T prGlueInfo)
{
	P_ADAPTER_T prAdpater = (P_ADAPTER_T) NULL;

	DEBUGFUNC("wlanAdapterCreate");

	do {
		prAdpater = (P_ADAPTER_T) kalMemAlloc(sizeof(ADAPTER_T),
						      VIR_MEM_TYPE);

		if (!prAdpater) {
			DBGLOG(INIT, ERROR,
			       ("Allocate ADAPTER memory ==> FAILED\n"));
			break;
		}
#if QM_TEST_MODE
		g_rQM.prAdapter = prAdpater;
#endif
		kalMemZero(prAdpater, sizeof(ADAPTER_T));
		prAdpater->prGlueInfo = prGlueInfo;
	} while (FALSE);

	return prAdpater;
}				/* wlanAdapterCreate */

/*----------------------------------------------------------------------------*/
/*!
* \brief Destroy adapter object
*
* \param prAdapter This routine is call to destroy the driver software objects.
*                  If fails, return NULL.
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanAdapterDestroy(IN P_ADAPTER_T prAdapter)
{
	if (!prAdapter) {
		return;
	}

	kalMemFree(prAdapter, VIR_MEM_TYPE, sizeof(ADAPTER_T));

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Initialize the adapter. The sequence is
*        1. Disable interrupt
*        2. Read adapter configuration from EEPROM and registry, verify chip ID.
*        3. Create NIC Tx/Rx resource.
*        4. Initialize the chip
*        5. Initialize the protocol
*        6. Enable Interrupt
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanAdapterStart(IN P_ADAPTER_T prAdapter,
		 IN P_REG_INFO_T prRegInfo,
		 IN PVOID pvFwImageMapFile, IN UINT_32 u4FwImageFileLength)
{
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	UINT_32 i, u4Value = 0;
	UINT_32 u4WHISR = 0;
	UINT_8 aucTxCount[8];

#if CFG_ENABLE_FW_DOWNLOAD
	UINT_32 u4FwLoadAddr, u4ImgSecSize;
#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
	UINT_32 j;
	P_FIRMWARE_DIVIDED_DOWNLOAD_T prFwHead;
	BOOLEAN fgValidHead;
	const UINT_32 u4CRCOffset = offsetof(FIRMWARE_DIVIDED_DOWNLOAD_T,
					     u4NumOfEntries);
#endif
#endif
#if (defined(MT5931) && (!CFG_SUPPORT_BCM_BWCS))
	PARAM_PTA_IPC_T rBwcsPta;
	UINT_32 u4SetInfoLen;
#endif

	ASSERT(prAdapter);

	DEBUGFUNC("wlanAdapterStart");

/* 4 <0> Reset variables in ADAPTER_T */
	prAdapter->fgIsFwOwn = TRUE;
	prAdapter->fgIsEnterD3ReqIssued = FALSE;

	QUEUE_INITIALIZE(&(prAdapter->rPendingCmdQueue));

/* Initialize rWlanInfo */
	kalMemSet(&(prAdapter->rWlanInfo), 0, sizeof(WLAN_INFO_T));

/* 4 <0.1> reset fgIsBusAccessFailed */
	fgIsBusAccessFailed = FALSE;

	do {
		if ((u4Status = nicAllocateAdapterMemory(prAdapter)) !=
		    WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
			       ("nicAllocateAdapterMemory Error!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

		prAdapter->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;

#if defined(MT6620) || defined(MT6628)
		DBGLOG(INIT, TRACE, ("wlanAdapterStart(): Acquiring LP-OWN\n"));
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
#if !CFG_ENABLE_FULL_PM
		nicpmSetDriverOwn(prAdapter);
#endif

		if (prAdapter->fgIsFwOwn == TRUE) {
			DBGLOG(INIT, ERROR, ("nicpmSetDriverOwn() failed!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

/* 4 <1> Initialize the Adapter */
		if ((u4Status = nicInitializeAdapter(prAdapter)) !=
		    WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, ("nicInitializeAdapter failed!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}
#endif

/* 4 <2> Initialize System Service (MGMT Memory pool and STA_REC) */
		nicInitSystemService(prAdapter);

/* 4 <3> Initialize Tx */
		nicTxInitialize(prAdapter);
		wlanDefTxPowerCfg(prAdapter);

/* 4 <4> Initialize Rx */
		nicRxInitialize(prAdapter);

#if CFG_ENABLE_FW_DOWNLOAD
#if defined(MT6620) || defined(MT6628)
		if (pvFwImageMapFile) {
/* 1. disable interrupt, download is done by polling mode only */
			nicDisableInterrupt(prAdapter);

/* 2. Initialize Tx Resource to fw download state */
			nicTxInitResetResource(prAdapter);

/* 3. FW download here */
			u4FwLoadAddr = prRegInfo->u4LoadAddress;

#if CFG_ENABLE_FW_DIVIDED_DOWNLOAD
/* 3a. parse file header for decision of divided firmware download or not */
			prFwHead =
			    (P_FIRMWARE_DIVIDED_DOWNLOAD_T) pvFwImageMapFile;

			if (prFwHead->u4Signature == MTK_WIFI_SIGNATURE &&
			    prFwHead->u4CRC ==
			    wlanCRC32((PUINT_8) pvFwImageMapFile + u4CRCOffset,
				      u4FwImageFileLength - u4CRCOffset)) {
				fgValidHead = TRUE;
			} else {
				fgValidHead = FALSE;
			}

/* 3b. engage divided firmware downloading */
			if (fgValidHead == TRUE) {
				for (i = 0; i < prFwHead->u4NumOfEntries; i++) {
#if CFG_ENABLE_FW_DOWNLOAD_AGGREGATION
					if (wlanImageSectionDownloadAggregated
					    (prAdapter,
					     prFwHead->arSection[i].u4DestAddr,
					     prFwHead->arSection[i].u4Length,
					     (PUINT_8)
					     pvFwImageMapFile +
					     prFwHead->arSection[i].u4Offset) !=
					    WLAN_STATUS_SUCCESS) {
						DBGLOG(INIT, ERROR,
						       ("Firmware scatter download failed!\n"));
						u4Status = WLAN_STATUS_FAILURE;
					}
#else
					for (j = 0;
					     j <
					     prFwHead->arSection[i].u4Length;
					     j += CMD_PKT_SIZE_FOR_IMAGE) {
						if (j +
						    CMD_PKT_SIZE_FOR_IMAGE <
						    prFwHead->
						    arSection[i].u4Length)
							u4ImgSecSize =
							    CMD_PKT_SIZE_FOR_IMAGE;
						else
							u4ImgSecSize =
							    prFwHead->arSection
							    [i].u4Length - j;

						if (wlanImageSectionDownload
						    (prAdapter,
						     prFwHead->
						     arSection[i].u4DestAddr +
						     j, u4ImgSecSize, (PUINT_8)
						     pvFwImageMapFile +
						     prFwHead->arSection[i].
						     u4Offset + j) !=
						    WLAN_STATUS_SUCCESS) {
							DBGLOG(INIT, ERROR,
							       ("Firmware scatter download failed!\n"));
							u4Status =
							    WLAN_STATUS_FAILURE;
							break;
						}
					}
#endif

/* escape from loop if any pending error occurs */
					if (u4Status == WLAN_STATUS_FAILURE) {
						break;
					}
				}
			} else
#endif
#if CFG_ENABLE_FW_DOWNLOAD_AGGREGATION
				if (wlanImageSectionDownloadAggregated
				    (prAdapter, u4FwLoadAddr,
				     u4FwImageFileLength, (PUINT_8)
				     pvFwImageMapFile)
				    != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR,
				       ("Firmware scatter download failed!\n"));
				u4Status = WLAN_STATUS_FAILURE;
			}
#else
				for (i = 0;
				     i < u4FwImageFileLength;
				     i += CMD_PKT_SIZE_FOR_IMAGE) {
					if (i + CMD_PKT_SIZE_FOR_IMAGE <
					    u4FwImageFileLength)
						u4ImgSecSize =
						    CMD_PKT_SIZE_FOR_IMAGE;
					else
						u4ImgSecSize =
						    u4FwImageFileLength - i;

					if (wlanImageSectionDownload(prAdapter,
								     u4FwLoadAddr
								     + i,
								     u4ImgSecSize,
								     (PUINT_8)
								     pvFwImageMapFile
								     + i) !=
					    WLAN_STATUS_SUCCESS) {
						DBGLOG(INIT, ERROR,
						       ("Firmware scatter download failed!\n"));
						u4Status = WLAN_STATUS_FAILURE;
						break;
					}
				}
#endif

			if (u4Status != WLAN_STATUS_SUCCESS) {
				break;
			}
#if !CFG_ENABLE_FW_DOWNLOAD_ACK
/* Send INIT_CMD_ID_QUERY_PENDING_ERROR command and wait for response */
			if (wlanImageQueryStatus(prAdapter) !=
			    WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR,
				       ("Firmware download failed!\n"));
				u4Status = WLAN_STATUS_FAILURE;
				break;
			}
#endif
		} else {
			DBGLOG(INIT, ERROR, ("No Firmware found!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

/* 4. send Wi-Fi Start command */
#if CFG_OVERRIDE_FW_START_ADDRESS
		wlanConfigWifiFunc(prAdapter, TRUE, prRegInfo->u4StartAddress);
#else
		wlanConfigWifiFunc(prAdapter, FALSE, 0);
#endif
#elif defined(MT5931)
		if (pvFwImageMapFile) {
			DBGLOG(INIT, TRACE,
			       ("Download Address: 0x%08X\n",
				prRegInfo->u4LoadAddress));
			DBGLOG(INIT, TRACE,
			       ("Firmware Length:  0x%08X\n",
				u4FwImageFileLength));

			do {
#if CFG_SUPPORT_WHOLE_CHIP_RESET
#define RESET_RDY_INTERVAL (120)

/* 1.0 whole-chip reset except HIFSYS */
				HAL_MCR_WR(prAdapter, MCR_WMCSR,
					   WMCSR_CHIP_RST);
				HAL_MCR_WR(prAdapter, MCR_WMCSR, 0);

/* 1.0.1 delay for EEIF ready */
				kalMsleep(RESET_RDY_INTERVAL);
#endif

/* 1.1 wait for INIT_RDY */
				i = 0;
				while (1) {
					HAL_MCR_RD(prAdapter,
						   MCR_WMCSR, &u4Value);

					if (u4Value & WMCSR_INI_RDY) {
						DBGLOG(INIT, TRACE,
						       ("INIT-RDY detected\n"));
						break;
					} else
					    if (kalIsCardRemoved
						(prAdapter->prGlueInfo)
						== TRUE
						|| fgIsBusAccessFailed ==
						TRUE) {
						u4Status = WLAN_STATUS_FAILURE;
						break;
					} else if (i >=
						   CFG_RESPONSE_POLLING_TIMEOUT)
					{
						DBGLOG(INIT, ERROR,
						       ("Waiting for Init Ready bit: Timeout\n"));
						u4Status = WLAN_STATUS_FAILURE;
						break;
					} else {
						i++;
						kalMsleep(10);
					}
				}

				if (u4Status != WLAN_STATUS_SUCCESS) {
					break;
				}

/* 1.2 set KSEL/FLEN */
				HAL_MCR_WR(prAdapter,
					   MCR_FWCFG, u4FwImageFileLength >> 6);

/* 1.3 enable FWDL_EN */
				HAL_MCR_WR(prAdapter, MCR_WMCSR, WMCSR_FWDLEN);

/* 1.4 wait for PLL_RDY */
				i = 0;
				while (1) {
					HAL_MCR_RD(prAdapter,
						   MCR_WMCSR, &u4Value);

					if (u4Value & WMCSR_PLLRDY) {
						DBGLOG(INIT, TRACE,
						       ("PLL-RDY detected\n"));
						break;
					} else
					    if (kalIsCardRemoved
						(prAdapter->prGlueInfo)
						== TRUE
						|| fgIsBusAccessFailed ==
						TRUE) {
						u4Status = WLAN_STATUS_FAILURE;
						break;
					} else if (i >=
						   CFG_RESPONSE_POLLING_TIMEOUT)
					{
						DBGLOG(INIT, ERROR,
						       ("Waiting for PLL Ready bit: Timeout\n"));
						u4Status = WLAN_STATUS_FAILURE;
						break;
					} else {
						i++;
						kalMsleep(10);
					}
				}

				if (u4Status != WLAN_STATUS_SUCCESS) {
					break;
				}

/* 2.1 turn on HIFSYS firmware download mode */
				HAL_MCR_WR(prAdapter,
					   MCR_FWDLSR, FWDLSR_FWDL_MODE);

/* 2.2 set starting address */
				u4FwLoadAddr = prRegInfo->u4LoadAddress;
				HAL_MCR_WR(prAdapter,
					   MCR_FWDLDSAR, u4FwLoadAddr);

/* 3. upload firmware */
				for (i = 0;
				     i < u4FwImageFileLength;
				     i += CMD_PKT_SIZE_FOR_IMAGE) {
					if (i + CMD_PKT_SIZE_FOR_IMAGE <
					    u4FwImageFileLength)
						u4ImgSecSize =
						    CMD_PKT_SIZE_FOR_IMAGE;
					else
						u4ImgSecSize =
						    u4FwImageFileLength - i;

					if (wlanImageSectionDownload(prAdapter,
								     u4FwLoadAddr
								     + i,
								     u4ImgSecSize,
								     (PUINT_8)
								     pvFwImageMapFile
								     + i) !=
					    WLAN_STATUS_SUCCESS) {
						DBGLOG(INIT, ERROR,
						       ("Firmware scatter download failed!\n"));
						u4Status = WLAN_STATUS_FAILURE;
						break;
					}
				}

				if (u4Status != WLAN_STATUS_SUCCESS) {
					break;
				}

/* 4.1 poll FWDL_OK & FWDL_FAIL bits */
				i = 0;
				while (1) {
					HAL_MCR_RD(prAdapter,
						   MCR_WMCSR, &u4Value);

					if (u4Value & WMCSR_DL_OK) {
						DBGLOG(INIT, TRACE,
						       ("DL_OK detected\n"));
						break;
					} else
					    if (kalIsCardRemoved
						(prAdapter->prGlueInfo)
						== TRUE
						|| fgIsBusAccessFailed == TRUE
						|| (u4Value & WMCSR_DL_FAIL)) {
						DBGLOG(INIT, ERROR,
						       ("DL_FAIL detected: 0x%08X\n",
							u4Value));
						u4Status = WLAN_STATUS_FAILURE;
						break;
					} else if (i >=
						   CFG_RESPONSE_POLLING_TIMEOUT)
					{
						DBGLOG(INIT, ERROR,
						       ("Waiting for DL_OK/DL_FAIL bit: Timeout\n"));
						u4Status = WLAN_STATUS_FAILURE;
						break;
					} else {
						i++;
						kalMsleep(10);
					}
				}

				if (u4Status != WLAN_STATUS_SUCCESS) {
					break;
				}

/* 4.2 turn off HIFSYS download mode */
				HAL_MCR_WR(prAdapter, MCR_FWDLSR, 0);
			} while (FALSE);

			if (u4Status != WLAN_STATUS_SUCCESS) {
				break;
			}

/* 5. disable interrupt */
			nicDisableInterrupt(prAdapter);
		} else {
			DBGLOG(INIT, ERROR, ("No Firmware found!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}
#endif
#endif

		DBGLOG(INIT, TRACE,
		       ("wlanAdapterStart(): Waiting for Ready bit..\n"));
/* 4 <5> check Wi-Fi FW asserts ready bit */
		i = 0;
		while (1) {
			HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);

			if (u4Value & WCIR_WLAN_READY) {
				DBGLOG(INIT, TRACE, ("Ready bit asserted\n"));
				break;
			} else if (kalIsCardRemoved(prAdapter->prGlueInfo) ==
				   TRUE || fgIsBusAccessFailed == TRUE) {
				u4Status = WLAN_STATUS_FAILURE;
				break;
			} else if (i >= CFG_RESPONSE_POLLING_TIMEOUT) {
				UINT_32 u4MailBox0;

				nicGetMailbox(prAdapter, 0, &u4MailBox0);
				DBGLOG(INIT, ERROR,
				       ("Waiting for Ready bit: Timeout, ID=%d\n",
					(u4MailBox0 & 0x0000FFFF)));
				u4Status = WLAN_STATUS_FAILURE;
				break;
			} else {
				i++;
				kalMsleep(10);
			}
		}

#if defined(MT5931)
/* Acquire LP-OWN */
		DBGLOG(INIT, TRACE, ("wlanAdapterStart(): Acquiring LP-OWN\n"));
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);
#if !CFG_ENABLE_FULL_PM
		nicpmSetDriverOwn(prAdapter);
#endif

		if (prAdapter->fgIsFwOwn == TRUE) {
			DBGLOG(INIT, ERROR, ("nicpmSetDriverOwn() failed!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

/* 4 <1> Initialize the Adapter */
		if ((u4Status = nicInitializeAdapter(prAdapter)) !=
		    WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, ("nicInitializeAdapter failed!\n"));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		}

/* post initialization for MT5931 due to some CR is only accessible after driver own */
		nicRxPostInitialize(prAdapter);
#endif

		if (u4Status == WLAN_STATUS_SUCCESS) {
/* 1. reset interrupt status */
			HAL_READ_INTR_STATUS(prAdapter, 4, (PUINT_8) & u4WHISR);
			if (HAL_IS_TX_DONE_INTR(u4WHISR)) {
				HAL_READ_TX_RELEASED_COUNT(prAdapter,
							   aucTxCount);
			}

/* 2. reset TX Resource for normal operation */
			nicTxResetResource(prAdapter);

#if CFG_SUPPORT_OSC_SETTING && defined(MT5931)
			wlanSetMcuOscStableTime(prAdapter, 0);
#endif

/* 3. query for permanent address by polling */
			wlanQueryPermanentAddress(prAdapter);

#if (CFG_SUPPORT_NIC_CAPABILITY == 1)
/* 4. query for NIC capability */
			wlanQueryNicCapability(prAdapter);
#endif

/* 5. Override network address */
			wlanUpdateNetworkAddress(prAdapter);

/* 6. indicate disconnection as default status */
			kalIndicateStatusAndComplete(prAdapter->prGlueInfo,
						     WLAN_STATUS_MEDIA_DISCONNECT,
						     NULL, 0);
		}

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

		if (u4Status != WLAN_STATUS_SUCCESS) {
			break;
		}

/* OID timeout timer initialize */
		cnmTimerInitTimer(prAdapter,
				  &prAdapter->rOidTimeoutTimer,
				  (PFN_MGMT_TIMEOUT_FUNC) wlanReleasePendingOid,
				  (UINT_32) NULL);

/* Power state initialization */
		prAdapter->fgWiFiInSleepyState = FALSE;
		prAdapter->rAcpiState = ACPI_STATE_D0;

/* Online scan option */
		if (prRegInfo->fgDisOnlineScan == 0) {
			prAdapter->fgEnOnlineScan = TRUE;
		} else {
			prAdapter->fgEnOnlineScan = FALSE;
		}

/* Beacon lost detection option */
		if (prRegInfo->fgDisBcnLostDetection != 0) {
			prAdapter->fgDisBcnLostDetection = TRUE;
		}

/* Load compile time constant */
		prAdapter->rWlanInfo.u2BeaconPeriod =
		    CFG_INIT_ADHOC_BEACON_INTERVAL;
		prAdapter->rWlanInfo.u2AtimWindow = CFG_INIT_ADHOC_ATIM_WINDOW;

#if 1				/* set PM parameters */
		prAdapter->fgEnArpFilter = prRegInfo->fgEnArpFilter;
		prAdapter->u4PsCurrentMeasureEn =
		    prRegInfo->u4PsCurrentMeasureEn;

		prAdapter->u4UapsdAcBmp = prRegInfo->u4UapsdAcBmp;

		prAdapter->u4MaxSpLen = prRegInfo->u4MaxSpLen;

		DBGLOG(INIT, TRACE,
		       ("[1] fgEnArpFilter:0x%x, u4UapsdAcBmp:0x%x, u4MaxSpLen:0x%x",
			prAdapter->fgEnArpFilter,
			prAdapter->u4UapsdAcBmp, prAdapter->u4MaxSpLen));

		prAdapter->fgEnCtiaPowerMode = FALSE;

#endif

/* MGMT Initialization */
		nicInitMGMT(prAdapter, prRegInfo);

/* Enable WZC Disassociation */
		prAdapter->rWifiVar.fgSupportWZCDisassociation = TRUE;

/* Apply Rate Setting */
		if ((ENUM_REGISTRY_FIXED_RATE_T) (prRegInfo->u4FixedRate) <
		    FIXED_RATE_NUM) {
			prAdapter->rWifiVar.eRateSetting =
			    (ENUM_REGISTRY_FIXED_RATE_T)
			    (prRegInfo->u4FixedRate);
		} else {
			prAdapter->rWifiVar.eRateSetting = FIXED_RATE_NONE;
		}

		if (prAdapter->rWifiVar.eRateSetting == FIXED_RATE_NONE) {
/* Enable Auto (Long/Short) Preamble */
			prAdapter->rWifiVar.ePreambleType = PREAMBLE_TYPE_AUTO;
		} else if ((prAdapter->rWifiVar.eRateSetting >=
			    FIXED_RATE_MCS0_20M_400NS &&
			    prAdapter->rWifiVar.eRateSetting <=
			    FIXED_RATE_MCS7_20M_400NS)
			   || (prAdapter->rWifiVar.eRateSetting >=
			       FIXED_RATE_MCS0_40M_400NS &&
			       prAdapter->rWifiVar.eRateSetting <=
			       FIXED_RATE_MCS32_400NS)) {
/* Force Short Preamble */
			prAdapter->rWifiVar.ePreambleType = PREAMBLE_TYPE_SHORT;
		} else {
/* Force Long Preamble */
			prAdapter->rWifiVar.ePreambleType = PREAMBLE_TYPE_LONG;
		}

/* Disable Hidden SSID Join */
		prAdapter->rWifiVar.fgEnableJoinToHiddenSSID = FALSE;

/* Enable Short Slot Time */
		prAdapter->rWifiVar.fgIsShortSlotTimeOptionEnable = TRUE;

/* configure available PHY type set */
		nicSetAvailablePhyTypeSet(prAdapter);

#if 1				/* set PM parameters */
		{
#if  CFG_SUPPORT_PWR_MGT
			prAdapter->u4PowerMode = prRegInfo->u4PowerMode;
			prAdapter->rWlanInfo.
			    arPowerSaveMode[NETWORK_TYPE_P2P_INDEX].
			    ucNetTypeIndex = NETWORK_TYPE_P2P_INDEX;
			prAdapter->rWlanInfo.
			    arPowerSaveMode[NETWORK_TYPE_P2P_INDEX].
			    ucPsProfile = ENUM_PSP_FAST_SWITCH;
#else
			prAdapter->u4PowerMode = ENUM_PSP_CONTINUOUS_ACTIVE;
#endif
			{	/*CR:WCNAE00007101 */
				struct net_device *prDev =
				    prAdapter->prGlueInfo->prDevHandler;

				if (prDev != NULL) {
					glBusSetIrq(prDev,
						    NULL,
						    prAdapter->prGlueInfo);
				} else {
					printk(KERN_INFO
					       "Skip glBusSetIrq because of the prDev\n");
				}
			}
			nicConfigPowerSaveProfile(prAdapter, NETWORK_TYPE_AIS_INDEX,	/* FIXIT */
						  prAdapter->u4PowerMode,
						  FALSE);
		}

#endif

#if CFG_SUPPORT_NVRAM
/* load manufacture data */
		wlanLoadManufactureData(prAdapter, prRegInfo);
#endif

#if (defined(MT5931) && (!CFG_SUPPORT_BCM_BWCS))
/* Enable DPD calibration. */
		rBwcsPta.u.aucBTPParams[0] = 0x00;
		rBwcsPta.u.aucBTPParams[1] = 0x01;
		rBwcsPta.u.aucBTPParams[2] = 0x00;
		rBwcsPta.u.aucBTPParams[3] = 0x80;

		wlanoidSetBT(prAdapter,
			     (PVOID) & rBwcsPta,
			     sizeof(PARAM_PTA_IPC_T), &u4SetInfoLen);
#endif

#if 0
/* Update Auto rate parameters in FW */
		nicRlmArUpdateParms(prAdapter,
				    prRegInfo->u4ArSysParam0,
				    prRegInfo->u4ArSysParam1,
				    prRegInfo->u4ArSysParam2,
				    prRegInfo->u4ArSysParam3);
#endif

#if (MT6620_E1_ASIC_HIFSYS_WORKAROUND == 1)
/* clock gating workaround */
		prAdapter->fgIsClockGatingEnabled = FALSE;
#endif
	} while (FALSE);

	if (u4Status == WLAN_STATUS_SUCCESS) {
/* restore to hardware default */
		HAL_SET_INTR_STATUS_READ_CLEAR(prAdapter);
		HAL_SET_MAILBOX_READ_CLEAR(prAdapter, FALSE);

/* Enable interrupt */
		nicEnableInterrupt(prAdapter);
	} else {
/* release allocated memory */
		nicReleaseAdapterMemory(prAdapter);
	}

	return u4Status;
}				/* wlanAdapterStart */

/*----------------------------------------------------------------------------*/
/*!
* \brief Uninitialize the adapter
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanAdapterStop(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i, u4Value = 0;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

#if (MT6620_E1_ASIC_HIFSYS_WORKAROUND == 1)
	if (prAdapter->fgIsClockGatingEnabled == TRUE) {
		nicDisableClockGating(prAdapter);
	}
#endif

/* MGMT - unitialization */
	nicUninitMGMT(prAdapter);

	if (prAdapter->rAcpiState == ACPI_STATE_D0 &&
#if (CFG_CHIP_RESET_SUPPORT == 1)
	    kalIsResetting() == FALSE &&
#endif
	    kalIsCardRemoved(prAdapter->prGlueInfo) == FALSE) {
/* 0. Disable interrupt, this can be done without Driver own */
		nicDisableInterrupt(prAdapter);

		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

/* 1. Set CMD to FW to tell WIFI to stop (enter power off state) */
		if (prAdapter->fgIsFwOwn == FALSE &&
		    wlanSendNicPowerCtrlCmd(prAdapter,
					    1) == WLAN_STATUS_SUCCESS) {
/* 2. Clear pending interrupt */
			i = 0;
			while (i < CFG_IST_LOOP_COUNT &&
			       nicProcessIST(prAdapter) !=
			       WLAN_STATUS_NOT_INDICATING)
				i++;
			;

/* 3. Wait til RDY bit has been cleaerd */
			i = 0;
			while (1) {
				HAL_MCR_RD(prAdapter, MCR_WCIR, &u4Value);

				if ((u4Value & WCIR_WLAN_READY) == 0)
					break;
				else if (kalIsCardRemoved(prAdapter->prGlueInfo)
					 == TRUE
					 || fgIsBusAccessFailed == TRUE
					 || i >= CFG_RESPONSE_POLLING_TIMEOUT) {
					break;
				} else {
					i++;
					kalMsleep(10);
				}
			}
		}

/* 4. Set Onwership to F/W */
		nicpmSetFWOwn(prAdapter, FALSE);

#if CFG_FORCE_RESET_UNDER_BUS_ERROR
		if (HAL_TEST_FLAG(prAdapter, ADAPTER_FLAG_HW_ERR) == TRUE) {
/* force acquire firmware own */
			kalDevRegWrite(prAdapter->prGlueInfo,
				       MCR_WHLPCR, WHLPCR_FW_OWN_REQ_CLR);

/* delay for 10ms */
			kalMdelay(10);

/* force firmware reset via software interrupt */
			kalDevRegWrite(prAdapter->prGlueInfo,
				       MCR_WSICR, WSICR_H2D_SW_INT_SET);

/* force release firmware own */
			kalDevRegWrite(prAdapter->prGlueInfo,
				       MCR_WHLPCR, WHLPCR_FW_OWN_REQ_SET);
		}
#endif

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	}

	nicRxUninitialize(prAdapter);

	nicTxRelease(prAdapter);

/* System Service Uninitialization */
	nicUninitSystemService(prAdapter);

	nicReleaseAdapterMemory(prAdapter);

#if defined(_HIF_SPI)
/* Note: restore the SPI Mode Select from 32 bit to default */
	nicRestoreSpiDefMode(prAdapter);
#endif

	return u4Status;
}				/* wlanAdapterStop */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called by ISR (interrupt).
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \retval TRUE: NIC's interrupt
* \retval FALSE: Not NIC's interrupt
*/
/*----------------------------------------------------------------------------*/
BOOL wlanISR(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgGlobalIntrCtrl)
{
	ASSERT(prAdapter);

	if (fgGlobalIntrCtrl) {
		nicDisableInterrupt(prAdapter);

/* wlanIST(prAdapter); */
	}

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called by IST (task_let).
*
* \param prAdapter      Pointer of Adapter Data Structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanIST(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

	nicProcessIST(prAdapter);

	nicEnableInterrupt(prAdapter);

	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will check command queue to find out if any could be dequeued
*        and/or send to HIF to MT6620
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param prCmdQue       Pointer of Command Queue (in Glue Layer)
*
* \retval WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanProcessCommandQueue(IN P_ADAPTER_T prAdapter, IN P_QUE_T prCmdQue)
{
	WLAN_STATUS rStatus;
	QUE_T rTempCmdQue, rMergeCmdQue, rStandInCmdQue;
	P_QUE_T prTempCmdQue, prMergeCmdQue, prStandInCmdQue;
	P_QUE_ENTRY_T prQueueEntry;
	P_CMD_INFO_T prCmdInfo;
	P_MSDU_INFO_T prMsduInfo;
	ENUM_FRAME_ACTION_T eFrameAction = FRAME_ACTION_DROP_PKT;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	ASSERT(prCmdQue);

	prTempCmdQue = &rTempCmdQue;
	prMergeCmdQue = &rMergeCmdQue;
	prStandInCmdQue = &rStandInCmdQue;

	QUEUE_INITIALIZE(prTempCmdQue);
	QUEUE_INITIALIZE(prMergeCmdQue);
	QUEUE_INITIALIZE(prStandInCmdQue);

/* 4 <1> Move whole list of CMD_INFO to temp queue */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);

/* 4 <2> Dequeue from head and check it is able to be sent */
	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		switch (prCmdInfo->eCmdType) {
		case COMMAND_TYPE_GENERAL_IOCTL:
		case COMMAND_TYPE_NETWORK_IOCTL:
/* command packet will be always sent */
			eFrameAction = FRAME_ACTION_TX_PKT;
			break;

		case COMMAND_TYPE_SECURITY_FRAME:
/* inquire with QM */
			eFrameAction = qmGetFrameAction(prAdapter,
							prCmdInfo->eNetworkType,
							prCmdInfo->ucStaRecIndex,
							NULL,
							FRAME_TYPE_802_1X);
			break;

		case COMMAND_TYPE_MANAGEMENT_FRAME:
/* inquire with QM */
			prMsduInfo = (P_MSDU_INFO_T) (prCmdInfo->prPacket);

			eFrameAction = qmGetFrameAction(prAdapter,
							prMsduInfo->ucNetworkType,
							prMsduInfo->ucStaRecIndex,
							prMsduInfo,
							FRAME_TYPE_MMPDU);
			break;

		default:
			ASSERT(0);
			break;
		}

/* 4 <3> handling upon dequeue result */
		if (eFrameAction == FRAME_ACTION_DROP_PKT) {
			wlanReleaseCommand(prAdapter, prCmdInfo);
		} else if (eFrameAction == FRAME_ACTION_QUEUE_PKT) {
			QUEUE_INSERT_TAIL(prMergeCmdQue, prQueueEntry);
		} else if (eFrameAction == FRAME_ACTION_TX_PKT) {
/* 4 <4> Send the command */
			rStatus = wlanSendCommand(prAdapter, prCmdInfo);

			if (rStatus == WLAN_STATUS_RESOURCES) {
/* no more TC4 resource for further transmission */
				QUEUE_INSERT_TAIL(prMergeCmdQue, prQueueEntry);
				break;
			} else if (rStatus == WLAN_STATUS_PENDING) {
/* command packet which needs further handling upon response */
				KAL_ACQUIRE_SPIN_LOCK(prAdapter,
						      SPIN_LOCK_CMD_PENDING);
				QUEUE_INSERT_TAIL(&
						  (prAdapter->rPendingCmdQueue),
						  prQueueEntry);
				KAL_RELEASE_SPIN_LOCK(prAdapter,
						      SPIN_LOCK_CMD_PENDING);
			} else {
				P_CMD_INFO_T prCmdInfo =
				    (P_CMD_INFO_T) prQueueEntry;

				if (rStatus == WLAN_STATUS_SUCCESS) {
					if (prCmdInfo->pfCmdDoneHandler) {
						prCmdInfo->
						    pfCmdDoneHandler(prAdapter,
								     prCmdInfo,
								     prCmdInfo->pucInfoBuffer);
					}
				} else {
					if (prCmdInfo->fgIsOid) {
						kalOidComplete(prAdapter->
							       prGlueInfo,
							       prCmdInfo->
							       fgSetQuery,
							       prCmdInfo->
							       u4SetInfoLen,
							       rStatus);
					}
				}

				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
			}
		} else {
			ASSERT(0);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

/* 4 <3> Merge back to original queue */
/* 4 <3.1> Merge prMergeCmdQue & prTempCmdQue */
	QUEUE_CONCATENATE_QUEUES(prMergeCmdQue, prTempCmdQue);

/* 4 <3.2> Move prCmdQue to prStandInQue, due to prCmdQue might differ due to incoming 802.1X frames */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prStandInCmdQue, prCmdQue);

/* 4 <3.3> concatenate prStandInQue to prMergeCmdQue */
	QUEUE_CONCATENATE_QUEUES(prMergeCmdQue, prStandInCmdQue);

/* 4 <3.4> then move prMergeCmdQue to prCmdQue */
	QUEUE_MOVE_ALL(prCmdQue, prMergeCmdQue);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_QUE);

	return WLAN_STATUS_SUCCESS;
}				/* end of wlanProcessCommandQueue() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will take CMD_INFO_T which carry some informations of
*        incoming OID and notify the NIC_TX to send CMD.
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param prCmdInfo      Pointer of P_CMD_INFO_T
*
* \retval WLAN_STATUS_SUCCESS   : CMD was written to HIF and be freed(CMD Done) immediately.
* \retval WLAN_STATUS_RESOURCE  : No resource for current command, need to wait for previous
*                                 frame finishing their transmission.
* \retval WLAN_STATUS_FAILURE   : Get failure while access HIF or been rejected.
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanSendCommand(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	P_TX_CTRL_T prTxCtrl;
	UINT_8 ucTC;		/* "Traffic Class" SW(Driver) resource classification */
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	prTxCtrl = &prAdapter->rTxCtrl;

/* DbgPrint("wlanSendCommand()\n"); */
/*  */
/*  */
#if DBG && 0
	LOG_FUNC("wlanSendCommand()\n");
	LOG_FUNC
	    ("CmdType %u NetworkType %u StaRecIndex %u Oid %u CID 0x%x SetQuery %u NeedResp %u CmdSeqNum %u\n",
	     prCmdInfo->eCmdType, prCmdInfo->eNetworkType,
	     prCmdInfo->ucStaRecIndex, prCmdInfo->fgIsOid, prCmdInfo->ucCID,
	     prCmdInfo->fgSetQuery, prCmdInfo->fgNeedResp,
	     prCmdInfo->ucCmdSeqNum);
#endif

#if (MT6620_E1_ASIC_HIFSYS_WORKAROUND == 1)
	if (prAdapter->fgIsClockGatingEnabled == TRUE) {
		nicDisableClockGating(prAdapter);
	}
#endif

	do {
/* <0> card removal check */
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
		    || fgIsBusAccessFailed == TRUE) {
			rStatus = WLAN_STATUS_FAILURE;
			break;
		}

/* <1> Normal case of sending CMD Packet */
		if (!prCmdInfo->fgDriverDomainMCR) {
/* <1.1> Assign Traffic Class(TC) = TC4. */
			ucTC = TC4_INDEX;

/* <1.2> Check if pending packet or resource was exhausted */
			if ((rStatus =
			     nicTxAcquireResource(prAdapter,
						  ucTC)) ==
			    WLAN_STATUS_RESOURCES) {
				DbgPrint("NO Resource:%d\n", ucTC);
				break;
			}

/* <1.3> Forward CMD_INFO_T to NIC Layer */
			rStatus = nicTxCmd(prAdapter, prCmdInfo, ucTC);

/* <1.4> Set Pending in response to Query Command/Need Response */
			if (rStatus == WLAN_STATUS_SUCCESS) {
				if ((!prCmdInfo->fgSetQuery) ||
				    (prCmdInfo->fgNeedResp)) {
					rStatus = WLAN_STATUS_PENDING;
				}
			}
		}
/* <2> Special case for access Driver Domain MCR */
		else {
			P_CMD_ACCESS_REG prCmdAccessReg;
			prCmdAccessReg =
			    (P_CMD_ACCESS_REG) (prCmdInfo->pucInfoBuffer +
						CMD_HDR_SIZE);

			if (prCmdInfo->fgSetQuery) {
				HAL_MCR_WR(prAdapter, (prCmdAccessReg->u4Address & BITS(2, 31)),	/* address is in DWORD unit */
					   prCmdAccessReg->u4Data);
			} else {
				P_CMD_ACCESS_REG prEventAccessReg;
				UINT_32 u4Address;

				u4Address = prCmdAccessReg->u4Address;
				prEventAccessReg =
				    (P_CMD_ACCESS_REG) prCmdInfo->pucInfoBuffer;
				prEventAccessReg->u4Address = u4Address;

				HAL_MCR_RD(prAdapter, prEventAccessReg->u4Address & BITS(2, 31),	/* address is in DWORD unit */
					   &prEventAccessReg->u4Data);
			}
		}
	} while (FALSE);

#if (MT6620_E1_ASIC_HIFSYS_WORKAROUND == 1)
	if (prAdapter->fgIsClockGatingEnabled == FALSE) {
		nicEnableClockGating(prAdapter);
	}
#endif

	return rStatus;
}				/* end of wlanSendCommand() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will release thd CMD_INFO upon its attribution
*
* \param prAdapter  Pointer of Adapter Data Structure
* \param prCmdInfo  Pointer of CMD_INFO_T
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanReleaseCommand(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	P_TX_CTRL_T prTxCtrl;
	P_MSDU_INFO_T prMsduInfo;

	ASSERT(prAdapter);
	ASSERT(prCmdInfo);

	prTxCtrl = &prAdapter->rTxCtrl;

	switch (prCmdInfo->eCmdType) {
	case COMMAND_TYPE_GENERAL_IOCTL:
	case COMMAND_TYPE_NETWORK_IOCTL:
		if (prCmdInfo->fgIsOid) {
			kalOidComplete(prAdapter->prGlueInfo,
				       prCmdInfo->fgSetQuery,
				       prCmdInfo->u4SetInfoLen,
				       WLAN_STATUS_FAILURE);
		}
		break;

	case COMMAND_TYPE_SECURITY_FRAME:
		kalSecurityFrameSendComplete(prAdapter->prGlueInfo,
					     prCmdInfo->prPacket,
					     WLAN_STATUS_FAILURE);
		break;

	case COMMAND_TYPE_MANAGEMENT_FRAME:
		prMsduInfo = (P_MSDU_INFO_T) prCmdInfo->prPacket;

/* invoke callbacks */
		if (prMsduInfo->pfTxDoneHandler != NULL) {
			prMsduInfo->pfTxDoneHandler(prAdapter,
						    prMsduInfo,
						    TX_RESULT_DROPPED_IN_DRIVER);
		}

		GLUE_DEC_REF_CNT(prTxCtrl->i4TxMgmtPendingNum);
		cnmMgtPktFree(prAdapter, prMsduInfo);
		break;

	default:
		ASSERT(0);
		break;
	}

	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
}				/* end of wlanReleaseCommand() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will search the CMD Queue to look for the pending OID and
*        compelete it immediately when system request a reset.
*
* \param prAdapter  ointer of Adapter Data Structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanReleasePendingOid(IN P_ADAPTER_T prAdapter, IN UINT_32 u4Data)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("wlanReleasePendingOid");

	ASSERT(prAdapter);

	DBGLOG(INIT, ERROR, ("OID Timeout! Releasing pending OIDs ..\n"));

	do {
/* 1: Clear Pending OID in prAdapter->rPendingCmdQueue */
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

		prCmdQue = &prAdapter->rPendingCmdQueue;
		QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
		while (prQueueEntry) {
			prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

			if (prCmdInfo->fgIsOid) {
				if (prCmdInfo->pfCmdTimeoutHandler) {
					prCmdInfo->
					    pfCmdTimeoutHandler(prAdapter,
								prCmdInfo);
				} else
					kalOidComplete(prAdapter->prGlueInfo,
						       prCmdInfo->fgSetQuery,
						       0, WLAN_STATUS_FAILURE);

				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
			} else {
				QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
			}

			QUEUE_REMOVE_HEAD(prTempCmdQue,
					  prQueueEntry, P_QUE_ENTRY_T);
		}

		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

/* 2: Clear pending OID in glue layer command queue */
		kalOidCmdClearance(prAdapter->prGlueInfo);

/* 3: Clear pending OID queued in pvOidEntry with REQ_FLAG_OID set */
		kalOidClearance(prAdapter->prGlueInfo);
	} while (FALSE);

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will search the CMD Queue to look for the pending CMD/OID for specific
*        NETWORK TYPE and compelete it immediately when system request a reset.
*
* \param prAdapter  ointer of Adapter Data Structure
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID
wlanReleasePendingCMDbyNetwork(IN P_ADAPTER_T prAdapter,
			       IN ENUM_NETWORK_TYPE_INDEX_T eNetworkType)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	do {
/* 1: Clear Pending OID in prAdapter->rPendingCmdQueue */
		KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);

		prCmdQue = &prAdapter->rPendingCmdQueue;
		QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
		while (prQueueEntry) {
			prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

			DBGLOG(P2P, TRACE,
			       ("Pending CMD for Network Type:%d \n",
				prCmdInfo->eNetworkType));

			if (prCmdInfo->eNetworkType == eNetworkType) {
				if (prCmdInfo->pfCmdTimeoutHandler) {
					prCmdInfo->
					    pfCmdTimeoutHandler(prAdapter,
								prCmdInfo);
				} else
					kalOidComplete(prAdapter->prGlueInfo,
						       prCmdInfo->fgSetQuery,
						       0, WLAN_STATUS_FAILURE);

				cmdBufFreeCmdInfo(prAdapter, prCmdInfo);
			} else {
				QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
			}

			QUEUE_REMOVE_HEAD(prTempCmdQue,
					  prQueueEntry, P_QUE_ENTRY_T);
		}

		KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_CMD_PENDING);
	} while (FALSE);

	return;
}				/* wlanReleasePendingCMDbyNetwork */

/*----------------------------------------------------------------------------*/
/*!
* \brief Return the packet buffer and reallocate one to the RFB
*
* \param prAdapter      Pointer of Adapter Data Structure
* \param pvPacket       Pointer of returned packet
*
* \retval WLAN_STATUS_SUCCESS: Success
* \retval WLAN_STATUS_FAILURE: Failed
*/
/*----------------------------------------------------------------------------*/
VOID wlanReturnPacket(IN P_ADAPTER_T prAdapter, IN PVOID pvPacket)
{
	P_RX_CTRL_T prRxCtrl;
	P_SW_RFB_T prSwRfb = NULL;

	KAL_SPIN_LOCK_DECLARATION();

	DEBUGFUNC("wlanReturnPacket");

	ASSERT(prAdapter);

	prRxCtrl = &prAdapter->rRxCtrl;
	ASSERT(prRxCtrl);

	if (pvPacket) {
		kalPacketFree(prAdapter->prGlueInfo, pvPacket);
		RX_ADD_CNT(prRxCtrl, RX_DATA_RETURNED_COUNT, 1);
#if CFG_NATIVE_802_11
		if (GLUE_TEST_FLAG(prAdapter->prGlueInfo, GLUE_FLAG_HALT)) {
		}
#endif
	}

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
	QUEUE_REMOVE_HEAD(&prRxCtrl->rIndicatedRfbList, prSwRfb, P_SW_RFB_T);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_RX_QUE);
	if (!prSwRfb) {
		ASSERT(0);
		return;
	}

	if (nicRxSetupRFB(prAdapter, prSwRfb)) {
		ASSERT(0);
		return;
	}
	nicRxReturnRFB(prAdapter, prSwRfb);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is a required function that returns information about
*        the capabilities and status of the driver and/or its network adapter.
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] pfnOidQryHandler Function pointer for the OID query handler.
* \param[IN] pvInfoBuf        Points to a buffer for return the query information.
* \param[IN] u4QueryBufferLen Specifies the number of bytes at pvInfoBuf.
* \param[OUT] pu4QueryInfoLen  Points to the number of bytes it written or is needed.
*
* \retval WLAN_STATUS_xxx Different WLAN_STATUS code returned by different handlers.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanQueryInformation(IN P_ADAPTER_T prAdapter,
		     IN PFN_OID_HANDLER_FUNC pfnOidQryHandler,
		     IN PVOID pvInfoBuf,
		     IN UINT_32 u4InfoBufLen, OUT PUINT_32 pu4QryInfoLen)
{
	WLAN_STATUS status = WLAN_STATUS_FAILURE;

	ASSERT(prAdapter);
	ASSERT(pu4QryInfoLen);

/* ignore any OID request after connected, under PS current measurement mode */
	if (prAdapter->u4PsCurrentMeasureEn &&
	    (prAdapter->prGlueInfo->eParamMediaStateIndicated ==
	     PARAM_MEDIA_STATE_CONNECTED)) {
		return WLAN_STATUS_SUCCESS;	/* note: return WLAN_STATUS_FAILURE or WLAN_STATUS_SUCCESS for blocking OIDs during current measurement ?? */
	}
#if 1
/* most OID handler will just queue a command packet */
	status = pfnOidQryHandler(prAdapter,
				  pvInfoBuf, u4InfoBufLen, pu4QryInfoLen);
#else
	if (wlanIsHandlerNeedHwAccess(pfnOidQryHandler, FALSE)) {
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

/* Reset sleepy state */
		if (prAdapter->fgWiFiInSleepyState == TRUE) {
			prAdapter->fgWiFiInSleepyState = FALSE;
		}

		status = pfnOidQryHandler(prAdapter,
					  pvInfoBuf,
					  u4InfoBufLen, pu4QryInfoLen);

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	} else {
		status = pfnOidQryHandler(prAdapter,
					  pvInfoBuf,
					  u4InfoBufLen, pu4QryInfoLen);
	}
#endif

	return status;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is a required function that allows bound protocol drivers,
*        or NDIS, to request changes in the state information that the miniport
*        maintains for particular object identifiers, such as changes in multicast
*        addresses.
*
* \param[IN] prAdapter     Pointer to the Glue info structure.
* \param[IN] pfnOidSetHandler     Points to the OID set handlers.
* \param[IN] pvInfoBuf     Points to a buffer containing the OID-specific data for the set.
* \param[IN] u4InfoBufLen  Specifies the number of bytes at prSetBuffer.
* \param[OUT] pu4SetInfoLen Points to the number of bytes it read or is needed.
*
* \retval WLAN_STATUS_xxx Different WLAN_STATUS code returned by different handlers.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanSetInformation(IN P_ADAPTER_T prAdapter,
		   IN PFN_OID_HANDLER_FUNC pfnOidSetHandler,
		   IN PVOID pvInfoBuf,
		   IN UINT_32 u4InfoBufLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS status = WLAN_STATUS_FAILURE;

	ASSERT(prAdapter);
	ASSERT(pu4SetInfoLen);

/* ignore any OID request after connected, under PS current measurement mode */
	if (prAdapter->u4PsCurrentMeasureEn &&
	    (prAdapter->prGlueInfo->eParamMediaStateIndicated ==
	     PARAM_MEDIA_STATE_CONNECTED)) {
		return WLAN_STATUS_SUCCESS;	/* note: return WLAN_STATUS_FAILURE or WLAN_STATUS_SUCCESS for blocking OIDs during current measurement ?? */
	}
#if 1
/* most OID handler will just queue a command packet
* for power state transition OIDs, handler will acquire power control by itself
*/
	status = pfnOidSetHandler(prAdapter,
				  pvInfoBuf, u4InfoBufLen, pu4SetInfoLen);
#else
	if (wlanIsHandlerNeedHwAccess(pfnOidSetHandler, TRUE)) {
		ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

/* Reset sleepy state */
		if (prAdapter->fgWiFiInSleepyState == TRUE) {
			prAdapter->fgWiFiInSleepyState = FALSE;
		}

		status = pfnOidSetHandler(prAdapter,
					  pvInfoBuf,
					  u4InfoBufLen, pu4SetInfoLen);

		RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);
	} else {
		status = pfnOidSetHandler(prAdapter,
					  pvInfoBuf,
					  u4InfoBufLen, pu4SetInfoLen);
	}
#endif

	return status;
}

#if CFG_SUPPORT_WAPI
/*----------------------------------------------------------------------------*/
/*!
* \brief This function is a used to query driver's config wapi mode or not
*
* \param[IN] prAdapter     Pointer to the Glue info structure.
*
* \retval TRUE for use wapi mode
*
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanQueryWapiMode(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->rWifiVar.rConnSettings.fgWapiMode;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to set RX filter to Promiscuous Mode.
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] fgEnablePromiscuousMode Enable/ disable RX Promiscuous Mode.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID
wlanSetPromiscuousMode(IN P_ADAPTER_T prAdapter,
		       IN BOOLEAN fgEnablePromiscuousMode)
{
	ASSERT(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to set RX filter to allow to receive
*        broadcast address packets.
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] fgEnableBroadcast Enable/ disable broadcast packet to be received.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID wlanRxSetBroadcast(IN P_ADAPTER_T prAdapter, IN BOOLEAN fgEnableBroadcast)
{
	ASSERT(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to send out CMD_NIC_POWER_CTRL command packet
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] ucPowerMode      refer to CMD/EVENT document
*
* \return WLAN_STATUS_SUCCESS
* \return WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanSendNicPowerCtrlCmd(IN P_ADAPTER_T prAdapter, IN UINT_8 ucPowerMode)
{
	WLAN_STATUS status = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_8 ucTC, ucCmdSeqNum;

	ASSERT(prAdapter);

	prGlueInfo = prAdapter->prGlueInfo;

/* 1. Prepare CMD */
	prCmdInfo =
	    cmdBufAllocateCmdInfo(prAdapter,
				  (CMD_HDR_SIZE + sizeof(CMD_NIC_POWER_CTRL)));
	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

/* 2.1 increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);
	DBGLOG(REQ, TRACE, ("ucCmdSeqNum =%d\n", ucCmdSeqNum));

/* 2.2 Setup common CMD Info Packet */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen =
	    (UINT_16) (CMD_HDR_SIZE + sizeof(CMD_NIC_POWER_CTRL));
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = NULL;
	prCmdInfo->fgIsOid = TRUE;
	prCmdInfo->ucCID = CMD_ID_NIC_POWER_CTRL;
	prCmdInfo->fgSetQuery = TRUE;
	prCmdInfo->fgNeedResp = FALSE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = sizeof(CMD_NIC_POWER_CTRL);

/* 2.3 Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount_UserPriority = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	kalMemZero(prWifiCmd->aucBuffer, sizeof(CMD_NIC_POWER_CTRL));
	((P_CMD_NIC_POWER_CTRL) (prWifiCmd->aucBuffer))->ucPowerMode =
	    ucPowerMode;

/* 3. Issue CMD for entering specific power mode */
	ucTC = TC4_INDEX;

	while (1) {
/* 3.0 Removal check */
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
		    || fgIsBusAccessFailed == TRUE) {
			status = WLAN_STATUS_FAILURE;
			break;
		}

/* 3.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter,
					 ucTC) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter,
						 ucTC) != WLAN_STATUS_SUCCESS) {
				DBGLOG(INIT, ERROR,
				       ("Fail to get TX resource return within timeout\n"));
				status = WLAN_STATUS_FAILURE;
				break;
			} else {
				continue;
			}
		}

/* 3.2 Send CMD Info Packet */
		if (nicTxCmd(prAdapter, prCmdInfo, ucTC) != WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR,
			       ("Fail to transmit CMD_NIC_POWER_CTRL command\n"));
			status = WLAN_STATUS_FAILURE;
		}

		break;
	}
	;

/* 4. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

/* 5. Add flag */
	if (ucPowerMode == 1) {
		prAdapter->fgIsEnterD3ReqIssued = TRUE;
	}

	return status;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to check if it is RF test mode and
*        the OID is allowed to be called or not
*
* \param[IN] prAdapter        Pointer to the Adapter structure.
* \param[IN] fgEnableBroadcast Enable/ disable broadcast packet to be received.
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanIsHandlerAllowedInRFTest(IN PFN_OID_HANDLER_FUNC pfnOidHandler,
			     IN BOOLEAN fgSetInfo)
{
	PFN_OID_HANDLER_FUNC *apfnOidHandlerAllowedInRFTest;
	UINT_32 i;
	UINT_32 u4NumOfElem;

	if (fgSetInfo) {
		apfnOidHandlerAllowedInRFTest =
		    apfnOidSetHandlerAllowedInRFTest;
		u4NumOfElem = sizeof(apfnOidSetHandlerAllowedInRFTest) /
		    sizeof(PFN_OID_HANDLER_FUNC);
	} else {
		apfnOidHandlerAllowedInRFTest =
		    apfnOidQueryHandlerAllowedInRFTest;
		u4NumOfElem = sizeof(apfnOidQueryHandlerAllowedInRFTest) /
		    sizeof(PFN_OID_HANDLER_FUNC);
	}

	for (i = 0; i < u4NumOfElem; i++) {
		if (apfnOidHandlerAllowedInRFTest[i] == pfnOidHandler) {
			return TRUE;
		}
	}

	return FALSE;
}

#if CFG_ENABLE_FW_DOWNLOAD
#if CFG_ENABLE_FW_DOWNLOAD_AGGREGATION
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to download FW image in an aggregated way
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanImageSectionDownloadAggregated(IN P_ADAPTER_T prAdapter,
				   IN UINT_32 u4DestAddr,
				   IN UINT_32 u4ImgSecSize,
				   IN PUINT_8 pucImgSecBuf)
{
#if defined(MT6620) || defined(MT6628)
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_DOWNLOAD_BUF prInitCmdDownloadBuf;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	PUINT_8 pucOutputBuf = (PUINT_8) NULL;	/* Pointer to Transmit Data Structure Frame */
	UINT_32 u4PktCnt, u4Offset, u4Length;
	UINT_32 u4TotalLength;

	ASSERT(prAdapter);
	ASSERT(pucImgSecBuf);

	pucOutputBuf = prAdapter->rTxCtrl.pucTxCoalescingBufPtr;

	DEBUGFUNC("wlanImageSectionDownloadAggregated");

	if (u4ImgSecSize == 0) {
		return WLAN_STATUS_SUCCESS;
	}

/* 1. Allocate CMD Info Packet and Pre-fill Headers */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
					  sizeof(INIT_HIF_TX_HEADER_T) +
					  sizeof(INIT_CMD_DOWNLOAD_BUF) +
					  CMD_PKT_SIZE_FOR_IMAGE);

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen =
	    sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_DOWNLOAD_BUF) +
	    CMD_PKT_SIZE_FOR_IMAGE;

/* 2. Use TC0's resource to download image. (only TC0 is allowed) */
	ucTC = TC0_INDEX;

/* 3. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->ucEtherTypeOffset = 0;
	prInitHifTxHeader->ucCSflags = 0;
	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_DOWNLOAD_BUF;

/* 4. Setup CMD_DOWNLOAD_BUF */
	prInitCmdDownloadBuf =
	    (P_INIT_CMD_DOWNLOAD_BUF) (prInitHifTxHeader->
				       rInitWifiCmd.aucBuffer);
	prInitCmdDownloadBuf->u4DataMode = 0
#if CFG_ENABLE_FW_ENCRYPTION
	    | DOWNLOAD_BUF_ENCRYPTION_MODE
#endif
	    ;

/* 5.0 reset loop control variable */
	u4TotalLength = 0;
	u4Offset = u4PktCnt = 0;

/* 5.1 main loop for maximize transmission count per access */
	while (u4Offset < u4ImgSecSize) {
		if (nicTxAcquireResource(prAdapter,
					 ucTC) == WLAN_STATUS_SUCCESS) {
/* 5.1.1 calculate u4Length */
			if (u4Offset + CMD_PKT_SIZE_FOR_IMAGE < u4ImgSecSize) {
				u4Length = CMD_PKT_SIZE_FOR_IMAGE;
			} else {
				u4Length = u4ImgSecSize - u4Offset;
			}

/* 5.1.1 increase command sequence number */
			ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);
			prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

/* 5.1.2 update HIF TX hardware header */
			prInitHifTxHeader->u2TxByteCount =
			    ALIGN_4(sizeof(INIT_HIF_TX_HEADER_T) +
				    sizeof(INIT_CMD_DOWNLOAD_BUF) +
				    (UINT_16) u4Length);

/* 5.1.3 fill command header */
			prInitCmdDownloadBuf->u4Address = u4DestAddr + u4Offset;
			prInitCmdDownloadBuf->u4Length = u4Length;
			prInitCmdDownloadBuf->u4CRC32 =
			    wlanCRC32(pucImgSecBuf + u4Offset, u4Length);

/* 5.1.4.1 copy header to coalescing buffer */
			kalMemCopy(pucOutputBuf + u4TotalLength,
				   (PVOID) prCmdInfo->pucInfoBuffer,
				   sizeof(INIT_HIF_TX_HEADER_T) +
				   sizeof(INIT_CMD_DOWNLOAD_BUF));

/* 5.1.4.2 copy payload to coalescing buffer */
			kalMemCopy(pucOutputBuf + u4TotalLength +
				   sizeof(INIT_HIF_TX_HEADER_T) +
				   sizeof(INIT_CMD_DOWNLOAD_BUF),
				   pucImgSecBuf + u4Offset, u4Length);

/* 5.1.4.3 update length and other variables */
			u4TotalLength += ALIGN_4(sizeof(INIT_HIF_TX_HEADER_T) +
						 sizeof(INIT_CMD_DOWNLOAD_BUF) +
						 u4Length);
			u4Offset += u4Length;
			u4PktCnt++;

			if (u4Offset < u4ImgSecSize) {
				continue;
			}
		} else if (u4PktCnt == 0) {
/* no resource, so get some back */
			if (nicTxPollingResource(prAdapter,
						 ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("Fail to get TX resource return within timeout\n"));
				break;
			}
		}

		if (u4PktCnt != 0) {
/* start transmission */
			HAL_WRITE_TX_PORT(prAdapter,
					  0,
					  u4TotalLength,
					  (PUINT_8) pucOutputBuf,
					  prAdapter->u4CoalescingBufCachedSize);

/* reset varaibles */
			u4PktCnt = 0;
			u4TotalLength = 0;
		}
	}

/* 8. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;

#else
#error "Only MT6620/MT6628 supports firmware download in an aggregated way"

	return WLAN_STATUS_FAILURE;

#endif
}

#endif
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to download FW image.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanImageSectionDownload(IN P_ADAPTER_T prAdapter,
			 IN UINT_32 u4DestAddr,
			 IN UINT_32 u4ImgSecSize, IN PUINT_8 pucImgSecBuf)
{
#if defined(MT6620) || defined(MT6628)

	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_DOWNLOAD_BUF prInitCmdDownloadBuf;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(pucImgSecBuf);
	ASSERT(u4ImgSecSize <= CMD_PKT_SIZE_FOR_IMAGE);

	DEBUGFUNC("wlanImageSectionDownload");

	if (u4ImgSecSize == 0) {
		return WLAN_STATUS_SUCCESS;
	}

/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
					  sizeof(INIT_HIF_TX_HEADER_T) +
					  sizeof(INIT_CMD_DOWNLOAD_BUF) +
					  u4ImgSecSize);

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

	prCmdInfo->u2InfoBufLen =
	    sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_DOWNLOAD_BUF) +
	    (UINT_16) u4ImgSecSize;

/* 2. Use TC0's resource to download image. (only TC0 is allowed) */
	ucTC = TC0_INDEX;

/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_DOWNLOAD_BUF;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

/* 5. Setup CMD_DOWNLOAD_BUF */
	prInitCmdDownloadBuf =
	    (P_INIT_CMD_DOWNLOAD_BUF) (prInitHifTxHeader->
				       rInitWifiCmd.aucBuffer);
	prInitCmdDownloadBuf->u4Address = u4DestAddr;
	prInitCmdDownloadBuf->u4Length = u4ImgSecSize;
	prInitCmdDownloadBuf->u4CRC32 = wlanCRC32(pucImgSecBuf, u4ImgSecSize);
	prInitCmdDownloadBuf->u4DataMode = 0
#if CFG_ENABLE_FW_DOWNLOAD_ACK
	    | DOWNLOAD_BUF_ACK_OPTION	/* ACK needed */
#endif
#if CFG_ENABLE_FW_ENCRYPTION
	    | DOWNLOAD_BUF_ENCRYPTION_MODE
#endif
	    ;
	kalMemCopy(prInitCmdDownloadBuf->aucBuffer, pucImgSecBuf, u4ImgSecSize);

/* 6. Send FW_Download command */
	while (1) {
/* 6.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter,
					 ucTC) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter,
						 ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("Fail to get TX resource return within timeout\n"));
				break;
			} else {
				continue;
			}
		}

/* 6.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo,
				 ucTC) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR,
			       ("Fail to transmit image download command\n"));
		}

		break;
	}
	;

#if CFG_ENABLE_FW_DOWNLOAD_ACK
/* 7. Wait for INIT_EVENT_ID_CMD_RESULT */
	u4Status = wlanImageSectionDownloadStatus(prAdapter, ucCmdSeqNum);
#endif

/* 8. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;

#elif defined(MT5931)

	UINT_32 i, u4Value;
	P_HIF_HW_TX_HEADER_T prHifTxHeader;

	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);
	ASSERT(pucImgSecBuf);
	ASSERT(u4ImgSecSize <= CMD_PKT_SIZE_FOR_IMAGE);

	DEBUGFUNC("wlanImageSectionDownload");
	DBGLOG(INIT, TRACE,
	       ("Destination: 0x%08X / Length: 0x%08X\n", u4DestAddr,
		u4ImgSecSize));

	if (u4ImgSecSize == 0) {
		return WLAN_STATUS_SUCCESS;
	}

/* 1. Use TX coalescing buffer */
	prHifTxHeader =
	    (P_HIF_HW_TX_HEADER_T) prAdapter->pucCoalescingBufCached;

/* 2. Setup HIF_TX_HEADER */
	prHifTxHeader->u2TxByteCount =
	    (UINT_16) (ALIGN_4(sizeof(HIF_HW_TX_HEADER_T) + u4ImgSecSize));
	prHifTxHeader->ucEtherTypeOffset = 0;
	prHifTxHeader->ucCSflags = 0;

/* 3. Copy payload */
	kalMemCopy(prHifTxHeader->aucBuffer, pucImgSecBuf, u4ImgSecSize);

/* 3.1 add 4-bytes zero tail */
	kalMemZero(&(prHifTxHeader->aucBuffer[ALIGN_4(u4ImgSecSize)]),
		   sizeof(HIF_HW_TX_HEADER_T));

/* 4. Poll til FWDL_RDY = 1 */
	i = 0;
	while (1) {
		HAL_MCR_RD(prAdapter, MCR_FWDLSR, &u4Value);

		if (u4Value & FWDLSR_FWDL_RDY) {
			DBGLOG(INIT, TRACE, ("FWDL_RDY detected\n"));
			break;
		} else if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
			   || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
			break;
		} else if (i >= CFG_RESPONSE_POLLING_TIMEOUT) {
			DBGLOG(INIT, ERROR,
			       ("Waiting for FWDL_RDY: Timeout (0x%08X)\n",
				u4Value));
			u4Status = WLAN_STATUS_FAILURE;
			break;
		} else {
			i++;
			kalMsleep(10);
		}
	}

/* 5. Send firmware */
	HAL_PORT_WR(prAdapter,
		    MCR_FWDLDR,
		    prHifTxHeader->u2TxByteCount,
		    (PUINT_8) prHifTxHeader,
		    prAdapter->u4CoalescingBufCachedSize);

	return u4Status;

#endif
}

#if !CFG_ENABLE_FW_DOWNLOAD_ACK
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm previously firmware download is done without error
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanImageQueryStatus(IN P_ADAPTER_T prAdapter)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) +
			 sizeof(INIT_EVENT_PENDING_ERROR)];
	UINT_32 u4RxPktLength;
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_PENDING_ERROR prEventPendingError;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;
	UINT_8 ucTC, ucCmdSeqNum;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanImageQueryStatus");

/* 1. Allocate CMD Info Packet and it Buffer. */
	prCmdInfo =
	    cmdBufAllocateCmdInfo(prAdapter, sizeof(INIT_HIF_TX_HEADER_T));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo, sizeof(INIT_HIF_TX_HEADER_T));
	prCmdInfo->u2InfoBufLen = sizeof(INIT_HIF_TX_HEADER_T);

/* 2. Use TC0's resource to download image. (only TC0 is allowed) */
	ucTC = TC0_INDEX;

/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_QUERY_PENDING_ERROR;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

/* 5. Send command */
	while (1) {
/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter,
					 ucTC) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter,
						 ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("Fail to get TX resource return within timeout\n"));
				break;
			} else {
				continue;
			}
		}

/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo,
				 ucTC) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR,
			       ("Fail to transmit image download command\n"));
		}

		break;
	}
	;

/* 6. Wait for INIT_EVENT_ID_PENDING_ERROR */
	do {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
		    || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR,
			       ("Bus error(%d)/Card removed(%d)\n",
				fgIsBusAccessFailed,
				kalIsCardRemoved(prAdapter->prGlueInfo)));
		} else if (nicRxWaitResponse(prAdapter,
					     0,
					     aucBuffer,
					     sizeof(INIT_HIF_RX_HEADER_T) +
					     sizeof(INIT_EVENT_PENDING_ERROR),
					     &u4RxPktLength) !=
			   WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, ("No RX response\n"));
		} else {
			prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;

/* EID / SeqNum check */
			if (prInitHifRxHeader->rInitWifiEvent.ucEID !=
			    INIT_EVENT_ID_PENDING_ERROR) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("EVENT-ID Mismatch: %d\n",
					prInitHifRxHeader->rInitWifiEvent.
					ucEID));
			} else if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum !=
				   ucCmdSeqNum) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("SEQ-NUM Mismatch: %d (expected: %d)\n",
					prInitHifRxHeader->
					rInitWifiEvent.ucSeqNum, ucCmdSeqNum));
			} else {
				prEventPendingError =
				    (P_INIT_EVENT_PENDING_ERROR)
				    (prInitHifRxHeader->rInitWifiEvent.
				     aucBuffer);
				if (prEventPendingError->ucStatus != 0) {	/* 0 for download success */
					u4Status = WLAN_STATUS_FAILURE;
					DBGLOG(INIT, ERROR,
					       ("ERROR CODE: %d\n",
						prEventCmdResult->ucStatus));
				} else {
					u4Status = WLAN_STATUS_SUCCESS;
				}
			}
		}
	} while (FALSE);

/* 7. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

#else
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to confirm the status of
*        previously downloaded firmware scatter
*
* @param prAdapter      Pointer to the Adapter structure.
*        ucCmdSeqNum    Sequence number of previous firmware scatter
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanImageSectionDownloadStatus(IN P_ADAPTER_T prAdapter, IN UINT_8 ucCmdSeqNum)
{
	UINT_8 aucBuffer[sizeof(INIT_HIF_RX_HEADER_T) +
			 sizeof(INIT_EVENT_CMD_RESULT)];
	P_INIT_HIF_RX_HEADER_T prInitHifRxHeader;
	P_INIT_EVENT_CMD_RESULT prEventCmdResult;
	UINT_32 u4RxPktLength;
	WLAN_STATUS u4Status;

	ASSERT(prAdapter);

	do {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == TRUE
		    || fgIsBusAccessFailed == TRUE) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR,
			       ("Bus error(%d)/Card removed(%d)\n",
				fgIsBusAccessFailed,
				kalIsCardRemoved(prAdapter->prGlueInfo)));
		} else if (nicRxWaitResponse(prAdapter,
					     0,
					     aucBuffer,
					     sizeof(INIT_HIF_RX_HEADER_T) +
					     sizeof(INIT_EVENT_CMD_RESULT),
					     &u4RxPktLength) !=
			   WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR, ("No RX response\n"));
		} else {
			prInitHifRxHeader = (P_INIT_HIF_RX_HEADER_T) aucBuffer;

/* EID / SeqNum check */
			if (prInitHifRxHeader->rInitWifiEvent.ucEID !=
			    INIT_EVENT_ID_CMD_RESULT) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("EVENT-ID Mismatch: %d\n",
					prInitHifRxHeader->rInitWifiEvent.
					ucEID));
			} else if (prInitHifRxHeader->rInitWifiEvent.ucSeqNum !=
				   ucCmdSeqNum) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("SEQ-NUM Mismatch: %d\n",
					prInitHifRxHeader->
					rInitWifiEvent.ucSeqNum));
			} else {
				prEventCmdResult =
				    (P_INIT_EVENT_CMD_RESULT)
				    (prInitHifRxHeader->rInitWifiEvent.
				     aucBuffer);
				if (prEventCmdResult->ucStatus != 0) {	/* 0 for download success */
					u4Status = WLAN_STATUS_FAILURE;
					DBGLOG(INIT, ERROR,
					       ("ERROR CODE: %d\n",
						prEventCmdResult->ucStatus));
				} else {
					u4Status = WLAN_STATUS_SUCCESS;
				}
			}
		}
	} while (FALSE);

	return u4Status;
}

#endif
/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to start FW normal operation.
*
* @param prAdapter      Pointer to the Adapter structure.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanConfigWifiFunc(IN P_ADAPTER_T prAdapter,
		   IN BOOLEAN fgEnable, IN UINT_32 u4StartAddress)
{
	P_CMD_INFO_T prCmdInfo;
	P_INIT_HIF_TX_HEADER_T prInitHifTxHeader;
	P_INIT_CMD_WIFI_START prInitCmdWifiStart;
	UINT_8 ucTC, ucCmdSeqNum;
	WLAN_STATUS u4Status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanConfigWifiFunc");

/* 1. Allocate CMD Info Packet and its Buffer. */
	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
					  sizeof(INIT_HIF_TX_HEADER_T) +
					  sizeof(INIT_CMD_WIFI_START));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

	kalMemZero(prCmdInfo, sizeof(INIT_HIF_TX_HEADER_T) +
		   sizeof(INIT_CMD_WIFI_START));
	prCmdInfo->u2InfoBufLen =
	    sizeof(INIT_HIF_TX_HEADER_T) + sizeof(INIT_CMD_WIFI_START);

/* 2. Always use TC0 */
	ucTC = TC0_INDEX;

/* 3. increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* 4. Setup common CMD Info Packet */
	prInitHifTxHeader = (P_INIT_HIF_TX_HEADER_T) (prCmdInfo->pucInfoBuffer);
	prInitHifTxHeader->rInitWifiCmd.ucCID = INIT_CMD_ID_WIFI_START;
	prInitHifTxHeader->rInitWifiCmd.ucSeqNum = ucCmdSeqNum;

	prInitCmdWifiStart =
	    (P_INIT_CMD_WIFI_START) (prInitHifTxHeader->rInitWifiCmd.aucBuffer);
	prInitCmdWifiStart->u4Override = (fgEnable == TRUE ? 1 : 0);
	prInitCmdWifiStart->u4Address = u4StartAddress;

/* 5. Seend WIFI start command */
	while (1) {
/* 5.1 Acquire TX Resource */
		if (nicTxAcquireResource(prAdapter,
					 ucTC) == WLAN_STATUS_RESOURCES) {
			if (nicTxPollingResource(prAdapter,
						 ucTC) != WLAN_STATUS_SUCCESS) {
				u4Status = WLAN_STATUS_FAILURE;
				DBGLOG(INIT, ERROR,
				       ("Fail to get TX resource return within timeout\n"));
				break;
			} else {
				continue;
			}
		}

/* 5.2 Send CMD Info Packet */
		if (nicTxInitCmd(prAdapter, prCmdInfo,
				 ucTC) != WLAN_STATUS_SUCCESS) {
			u4Status = WLAN_STATUS_FAILURE;
			DBGLOG(INIT, ERROR,
			       ("Fail to transmit WIFI start command\n"));
		}

		break;
	}
	;

/* 6. Free CMD Info Packet. */
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return u4Status;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to generate CRC32 checksum
*
* @param buf Pointer to the data.
* @param len data length
*
* @return crc32 value
*/
/*----------------------------------------------------------------------------*/
UINT_32 wlanCRC32(PUINT_8 buf, UINT_32 len)
{
	UINT_32 i, crc32 = 0xFFFFFFFF;
	const UINT_32 crc32_ccitt_table[256] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
		0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
		0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
		0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
		0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
		0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
		0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
		0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
		0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
		0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
		0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
		0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
		0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
		0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
		0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
		0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
		0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
		0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
		0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
		0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
		0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
		0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
		0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
		0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
		0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
		0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
		0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
		0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
		0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
		0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
		0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
		0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
		0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
		0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
		0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
		0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
		0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
		0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
		0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
		0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
		0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
		0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
		0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
		0x2d02ef8d
	};

	for (i = 0; i < len; i++)
		crc32 =
		    crc32_ccitt_table[(crc32 ^ buf[i]) & 0xff] ^ (crc32 >> 8);

	return ~crc32;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to process queued RX packets
*
* @param prAdapter          Pointer to the Adapter structure.
*        prSwRfbListHead    Pointer to head of RX packets link list
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanProcessQueuedSwRfb(IN P_ADAPTER_T prAdapter, IN P_SW_RFB_T prSwRfbListHead)
{
	P_SW_RFB_T prSwRfb, prNextSwRfb;
	P_TX_CTRL_T prTxCtrl;
	P_RX_CTRL_T prRxCtrl;

	ASSERT(prAdapter);
	ASSERT(prSwRfbListHead);

	prTxCtrl = &prAdapter->rTxCtrl;
	prRxCtrl = &prAdapter->rRxCtrl;

	prSwRfb = prSwRfbListHead;

	do {
/* save next first */
		prNextSwRfb = (P_SW_RFB_T) QUEUE_GET_NEXT_ENTRY((P_QUE_ENTRY_T)
								prSwRfb);

		switch (prSwRfb->eDst) {
		case RX_PKT_DESTINATION_HOST:
			nicRxProcessPktWithoutReorder(prAdapter, prSwRfb);
			break;

		case RX_PKT_DESTINATION_FORWARD:
			nicRxProcessForwardPkt(prAdapter, prSwRfb);
			break;

		case RX_PKT_DESTINATION_HOST_WITH_FORWARD:
			nicRxProcessGOBroadcastPkt(prAdapter, prSwRfb);
			break;

		case RX_PKT_DESTINATION_NULL:
			nicRxReturnRFB(prAdapter, prSwRfb);
			break;

		default:
			break;
		}

#if CFG_HIF_RX_STARVATION_WARNING
		prRxCtrl->u4DequeuedCnt++;
#endif
		prSwRfb = prNextSwRfb;
	} while (prSwRfb);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to purge queued TX packets
*        by indicating failure to OS and returned to free list
*
* @param prAdapter          Pointer to the Adapter structure.
*        prMsduInfoListHead Pointer to head of TX packets link list
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanProcessQueuedMsduInfo(IN P_ADAPTER_T prAdapter,
			  IN P_MSDU_INFO_T prMsduInfoListHead)
{
	ASSERT(prAdapter);
	ASSERT(prMsduInfoListHead);

	nicTxFreeMsduInfoPacket(prAdapter, prMsduInfoListHead);
	nicTxReturnMsduInfo(prAdapter, prMsduInfoListHead);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if the OID handler needs timeout
*
* @param prAdapter          Pointer to the Adapter structure.
*        pfnOidHandler      Pointer to the OID handler
*
* @return TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanoidTimeoutCheck(IN P_ADAPTER_T prAdapter,
		    IN PFN_OID_HANDLER_FUNC pfnOidHandler)
{
	PFN_OID_HANDLER_FUNC *apfnOidHandlerWOTimeoutCheck;
	UINT_32 i;
	UINT_32 u4NumOfElem;

	apfnOidHandlerWOTimeoutCheck = apfnOidWOTimeoutCheck;
	u4NumOfElem = sizeof(apfnOidWOTimeoutCheck) /
	    sizeof(PFN_OID_HANDLER_FUNC);

	for (i = 0; i < u4NumOfElem; i++) {
		if (apfnOidHandlerWOTimeoutCheck[i] == pfnOidHandler) {
			return FALSE;
		}
	}

/* set timer if need timeout check */
/* cnmTimerStartTimer(prAdapter, */
/*        &(prAdapter->rOidTimeoutTimer), */
/*        1000); */
	cnmTimerStartTimer(prAdapter, &(prAdapter->rOidTimeoutTimer), 2000);

	return TRUE;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to clear any pending OID timeout check
*
* @param prAdapter          Pointer to the Adapter structure.
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanoidClearTimeoutCheck(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	cnmTimerStopTimer(prAdapter, &(prAdapter->rOidTimeoutTimer));
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to set up the MCUSYS's OSC stable time
*
* @param prAdapter          Pointer to the Adapter structure.
*
* @return none
*/
/*----------------------------------------------------------------------------*/

#if CFG_SUPPORT_OSC_SETTING && defined(MT5931)
WLAN_STATUS
wlanSetMcuOscStableTime(IN P_ADAPTER_T prAdapter, IN UINT_16 u2OscStableTime)
{
	UINT_8 ucCmdSeqNum = 0;
	P_CMD_INFO_T prCmdInfo = NULL;
	P_WIFI_CMD_T prWifiCmd = NULL;
	P_CMD_MCU_LP_PARAM_T prMcuSetOscCmd = NULL;
	WLAN_STATUS status = WLAN_STATUS_SUCCESS;

	ASSERT(prAdapter);

	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
					  CMD_HDR_SIZE +
					  sizeof(CMD_MCU_LP_PARAM_T));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* compose CMD_MCU_LP_PARAM_T cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen = CMD_HDR_SIZE + sizeof(CMD_MCU_LP_PARAM_T);
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = NULL;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = CMD_ID_SET_OSC;
	prCmdInfo->fgSetQuery = TRUE;
	prCmdInfo->fgNeedResp = FALSE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = sizeof(CMD_MCU_LP_PARAM_T);

/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount_UserPriority = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

/* configure CMD_MCU_LP_PARAM_T */
	prMcuSetOscCmd = (P_CMD_MCU_LP_PARAM_T) (prWifiCmd->aucBuffer);
	prMcuSetOscCmd->u2OscStableTime = u2OscStableTime;

	status = wlanSendCommand(prAdapter, prCmdInfo);
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	return status;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to update network address in firmware domain
*
* @param prAdapter          Pointer to the Adapter structure.
*
* @return WLAN_STATUS_FAILURE   The request could not be processed
*         WLAN_STATUS_PENDING   The request has been queued for later processing
*         WLAN_STATUS_SUCCESS   The request has been processed
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanUpdateNetworkAddress(IN P_ADAPTER_T prAdapter)
{
	const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;
	PARAM_MAC_ADDRESS rMacAddr;
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	P_CMD_BASIC_CONFIG prCmdBasicConfig;
	UINT_32 u4SysTime;

	DEBUGFUNC("wlanUpdateNetworkAddress");

	ASSERT(prAdapter);

	if (kalRetrieveNetworkAddress(prAdapter->prGlueInfo,
				      &rMacAddr) == FALSE
	    || IS_BMCAST_MAC_ADDR(rMacAddr)
	    || EQUAL_MAC_ADDR(aucZeroMacAddr, rMacAddr)) {
/* eFUSE has a valid address, don't do anything */
		if (prAdapter->fgIsEmbbededMacAddrValid == TRUE) {
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, INFO, ("Using embedded MAC address"));
#endif
			return WLAN_STATUS_SUCCESS;
		} else {
#if CFG_SHOW_MACADDR_SOURCE
			DBGLOG(INIT, INFO,
			       ("Using dynamically generated MAC address"));
#endif
/* dynamic generate */
			u4SysTime = (UINT_32) kalGetTimeTick();

			rMacAddr[0] = 0x00;
			rMacAddr[1] = 0x08;
			rMacAddr[2] = 0x22;

			kalMemCopy(&rMacAddr[3], &u4SysTime, 3);
		}
	} else {
#if CFG_SHOW_MACADDR_SOURCE
		DBGLOG(INIT, INFO, ("Using host-supplied MAC address"));
#endif
	}

	prCmdInfo = cmdBufAllocateCmdInfo(prAdapter,
					  CMD_HDR_SIZE +
					  sizeof(CMD_BASIC_CONFIG));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen = CMD_HDR_SIZE + sizeof(CMD_BASIC_CONFIG);
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = NULL;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = CMD_ID_BASIC_CONFIG;
	prCmdInfo->fgSetQuery = TRUE;
	prCmdInfo->fgNeedResp = FALSE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = sizeof(CMD_BASIC_CONFIG);

/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount_UserPriority = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

/* configure CMD_BASIC_CONFIG */
	prCmdBasicConfig = (P_CMD_BASIC_CONFIG) (prWifiCmd->aucBuffer);
	kalMemCopy(&(prCmdBasicConfig->rMyMacAddr),
		   &rMacAddr, PARAM_MAC_ADDR_LEN);
	prCmdBasicConfig->ucNative80211 = 0;
	prCmdBasicConfig->rCsumOffload.u2RxChecksum = 0;
	prCmdBasicConfig->rCsumOffload.u2TxChecksum = 0;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	if (prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_TX_TCP)
		prCmdBasicConfig->rCsumOffload.u2TxChecksum |= BIT(2);

	if (prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_TX_UDP)
		prCmdBasicConfig->rCsumOffload.u2TxChecksum |= BIT(1);

	if (prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_TX_IP)
		prCmdBasicConfig->rCsumOffload.u2TxChecksum |= BIT(0);

	if (prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_RX_TCP)
		prCmdBasicConfig->rCsumOffload.u2RxChecksum |= BIT(2);

	if (prAdapter->u4CSUMFlags & CSUM_OFFLOAD_EN_RX_UDP)
		prCmdBasicConfig->rCsumOffload.u2RxChecksum |= BIT(1);

	if (prAdapter->u4CSUMFlags &
	    (CSUM_OFFLOAD_EN_RX_IPv4 | CSUM_OFFLOAD_EN_RX_IPv6))
		prCmdBasicConfig->rCsumOffload.u2RxChecksum |= BIT(0);
#endif

	if (wlanSendCommand(prAdapter, prCmdInfo) == WLAN_STATUS_RESOURCES) {
		prCmdInfo->pfCmdDoneHandler = nicCmdEventQueryAddress;
		kalEnqueueCommand(prAdapter->prGlueInfo,
				  (P_QUE_ENTRY_T) prCmdInfo);

		return WLAN_STATUS_PENDING;
	} else {
		nicCmdEventQueryAddress(prAdapter,
					prCmdInfo, (PUINT_8) prCmdBasicConfig);
		cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

		return WLAN_STATUS_SUCCESS;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if the device is in RF test mode
*
* @param pfnOidHandler      Pointer to the OID handler
*
* @return TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanQueryTestMode(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->fgTestMode;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to identify 802.1x and Bluetooth-over-Wi-Fi
*        security frames, and queued into command queue for strict ordering
*        due to 802.1x frames before add-key OIDs are not to be encrypted
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prPacket       Pointer of native packet
*
* @return TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
wlanProcessSecurityFrame(IN P_ADAPTER_T prAdapter, IN P_NATIVE_PACKET prPacket)
{
	UINT_8 ucPriorityParam;
	UINT_8 aucEthDestAddr[PARAM_MAC_ADDR_LEN];
	BOOLEAN fgIs1x = FALSE;
	BOOLEAN fgIsPAL = FALSE;
	UINT_32 u4PacketLen;
	ULONG u4SysTime;
	UINT_8 ucNetworkType;
	P_CMD_INFO_T prCmdInfo;

	ASSERT(prAdapter);
	ASSERT(prPacket);

	if (kalQoSFrameClassifierAndPacketInfo(prAdapter->prGlueInfo,
					       prPacket,
					       &ucPriorityParam,
					       &u4PacketLen,
					       aucEthDestAddr,
					       &fgIs1x,
					       &fgIsPAL,
					       &ucNetworkType) == TRUE) {
		if (fgIs1x == FALSE) {
			return FALSE;
		} else {
			KAL_SPIN_LOCK_DECLARATION();
			KAL_ACQUIRE_SPIN_LOCK(prAdapter,
					      SPIN_LOCK_CMD_RESOURCE);
			QUEUE_REMOVE_HEAD(&prAdapter->rFreeCmdList, prCmdInfo,
					  P_CMD_INFO_T);
			KAL_RELEASE_SPIN_LOCK(prAdapter,
					      SPIN_LOCK_CMD_RESOURCE);

			DBGLOG(RSN, INFO, ("T1X len=%d\n", u4PacketLen));

			if (prCmdInfo) {
				P_STA_RECORD_T prStaRec;
/* fill arrival time */
				u4SysTime = (OS_SYSTIME) kalGetTimeTick();
				GLUE_SET_PKT_ARRIVAL_TIME(prPacket, u4SysTime);

				kalMemZero(prCmdInfo, sizeof(CMD_INFO_T));

				prCmdInfo->eCmdType =
				    COMMAND_TYPE_SECURITY_FRAME;
				prCmdInfo->u2InfoBufLen = (UINT_16) u4PacketLen;
				prCmdInfo->pucInfoBuffer = NULL;
				prCmdInfo->prPacket = prPacket;
#if 0
				prCmdInfo->ucStaRecIndex =
				    qmGetStaRecIdx(prAdapter, aucEthDestAddr,
						   (ENUM_NETWORK_TYPE_INDEX_T)
						   ucNetworkType);
#endif
				prStaRec = cnmGetStaRecByAddress(prAdapter,
								 (ENUM_NETWORK_TYPE_INDEX_T)
								 ucNetworkType,
								 aucEthDestAddr);
				if (prStaRec) {
					prCmdInfo->ucStaRecIndex =
					    prStaRec->ucIndex;
				} else {
					prCmdInfo->ucStaRecIndex =
					    STA_REC_INDEX_NOT_FOUND;
				}

				prCmdInfo->eNetworkType =
				    (ENUM_NETWORK_TYPE_INDEX_T)
				    ucNetworkType;
				prCmdInfo->pfCmdDoneHandler =
				    wlanSecurityFrameTxDone;
				prCmdInfo->pfCmdTimeoutHandler =
				    wlanSecurityFrameTxTimeout;
				prCmdInfo->fgIsOid = FALSE;
				prCmdInfo->fgSetQuery = TRUE;
				prCmdInfo->fgNeedResp = FALSE;

				kalEnqueueCommand(prAdapter->prGlueInfo,
						  (P_QUE_ENTRY_T) prCmdInfo);

				return TRUE;
			} else {
				ASSERT(0);
				return FALSE;
			}
		}
	} else {
		return FALSE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called when 802.1x or Bluetooth-over-Wi-Fi
*        security frames has been sent to firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prCmdInfo      Pointer of CMD_INFO_T
* @param pucEventBuf    meaningless, only for API compatibility
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID
wlanSecurityFrameTxDone(IN P_ADAPTER_T prAdapter,
			IN P_CMD_INFO_T prCmdInfo, IN PUINT_8 pucEventBuf)
{
	ASSERT(prAdapter);
	ASSERT(prCmdInfo);
	if (prCmdInfo->eNetworkType == NETWORK_TYPE_AIS_INDEX &&
	    prAdapter->rWifiVar.rAisSpecificBssInfo.fgCounterMeasure) {
		P_STA_RECORD_T prSta = cnmGetStaRecByIndex(prAdapter,
							   prCmdInfo->ucStaRecIndex);
		if (prSta) {
			kalMsleep(10);
			secFsmEventEapolTxDone(prAdapter,
					       prSta, TX_RESULT_SUCCESS);
		}
	}

	kalSecurityFrameSendComplete(prAdapter->prGlueInfo,
				     prCmdInfo->prPacket, WLAN_STATUS_SUCCESS);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called when 802.1x or Bluetooth-over-Wi-Fi
*        security frames has failed sending to firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prCmdInfo      Pointer of CMD_INFO_T
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID
wlanSecurityFrameTxTimeout(IN P_ADAPTER_T prAdapter, IN P_CMD_INFO_T prCmdInfo)
{
	ASSERT(prAdapter);
	ASSERT(prCmdInfo);

	kalSecurityFrameSendComplete(prAdapter->prGlueInfo,
				     prCmdInfo->prPacket, WLAN_STATUS_FAILURE);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called before AIS is starting a new scan
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearScanningResult(IN P_ADAPTER_T prAdapter)
{
	BOOLEAN fgKeepCurrOne = FALSE;
	UINT_32 i;

	ASSERT(prAdapter);

/* clear scanning result */
	if (kalGetMediaStateIndicated(prAdapter->prGlueInfo) ==
	    PARAM_MEDIA_STATE_CONNECTED) {
		for (i = 0; i < prAdapter->rWlanInfo.u4ScanResultNum; i++) {
			if (EQUAL_MAC_ADDR
			    (prAdapter->rWlanInfo.rCurrBssId.arMacAddress,
			     prAdapter->rWlanInfo.arScanResult[i]
			     .arMacAddress)) {
				fgKeepCurrOne = TRUE;

				if (i != 0) {
/* copy structure */
					kalMemCopy(&
						   (prAdapter->
						    rWlanInfo.arScanResult[0]),
						   &(prAdapter->
						     rWlanInfo.arScanResult[i]),
						   OFFSET_OF(PARAM_BSSID_EX_T,
							     aucIEs));
				}

				if (prAdapter->rWlanInfo.
				    arScanResult[i].u4IELength > 0) {
					if (prAdapter->
					    rWlanInfo.apucScanResultIEs[i] !=
					    &(prAdapter->rWlanInfo.
					      aucScanIEBuf[0])) {
/* move IEs to head */
						kalMemCopy(prAdapter->
							   rWlanInfo.aucScanIEBuf,
							   prAdapter->rWlanInfo.
							   apucScanResultIEs[i],
							   prAdapter->rWlanInfo.
							   arScanResult
							   [i].u4IELength);
					}

/* modify IE pointer */
					prAdapter->rWlanInfo.
					    apucScanResultIEs[0] =
					    &(prAdapter->
					      rWlanInfo.aucScanIEBuf[0]);
				} else {
					prAdapter->rWlanInfo.
					    apucScanResultIEs[0] = NULL;
				}

				break;
			}
		}
	}

	if (fgKeepCurrOne == TRUE) {
		prAdapter->rWlanInfo.u4ScanResultNum = 1;
		prAdapter->rWlanInfo.u4ScanIEBufferUsage =
		    ALIGN_4(prAdapter->rWlanInfo.arScanResult[0].u4IELength);
	} else {
		prAdapter->rWlanInfo.u4ScanResultNum = 0;
		prAdapter->rWlanInfo.u4ScanIEBufferUsage = 0;
	}

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called when AIS received a beacon timeout event
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param arBSSID        MAC address of the specified BSS
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID wlanClearBssInScanningResult(IN P_ADAPTER_T prAdapter, IN PUINT_8 arBSSID)
{
	UINT_32 i, j, u4IELength = 0, u4IEMoveLength;
	PUINT_8 pucIEPtr;

	ASSERT(prAdapter);

/* clear scanning result */
	i = 0;
	while (1) {
		if (i >= prAdapter->rWlanInfo.u4ScanResultNum) {
			break;
		}

		if (EQUAL_MAC_ADDR(arBSSID,
				   prAdapter->rWlanInfo.
				   arScanResult[i].arMacAddress)) {
/* backup current IE length */
			u4IELength =
			    ALIGN_4(prAdapter->rWlanInfo.
				    arScanResult[i].u4IELength);
			pucIEPtr = prAdapter->rWlanInfo.apucScanResultIEs[i];

/* removed from middle */
			for (j = i + 1;
			     j < prAdapter->rWlanInfo.u4ScanResultNum; j++) {
				kalMemCopy(&
					   (prAdapter->rWlanInfo.
					    arScanResult[j - 1]),
					   &(prAdapter->rWlanInfo.
					     arScanResult[j]),
					   OFFSET_OF(PARAM_BSSID_EX_T, aucIEs));

				prAdapter->rWlanInfo.apucScanResultIEs[j - 1] =
				    prAdapter->rWlanInfo.apucScanResultIEs[j];
			}

			prAdapter->rWlanInfo.u4ScanResultNum--;

/* remove IE buffer if needed := move rest of IE buffer */
			if (u4IELength > 0) {
				u4IEMoveLength =
				    prAdapter->rWlanInfo.u4ScanIEBufferUsage -
				    (((UINT_32) pucIEPtr) +
				     u4IELength -
				     ((UINT_32)
				      (&
				       (prAdapter->rWlanInfo.
					aucScanIEBuf[0]))));

				kalMemCopy(pucIEPtr,
					   (PUINT_8) (((UINT_32) pucIEPtr) +
						      u4IELength),
					   u4IEMoveLength);

				prAdapter->rWlanInfo.u4ScanIEBufferUsage -=
				    u4IELength;

/* correction of pointers to IE buffer */
				for (j = 0;
				     j < prAdapter->rWlanInfo.u4ScanResultNum;
				     j++) {
					if (prAdapter->
					    rWlanInfo.apucScanResultIEs[j] >
					    pucIEPtr) {
						prAdapter->
						    rWlanInfo.apucScanResultIEs
						    [j] =
						    (PUINT_8) ((UINT_32)
							       (prAdapter->rWlanInfo.apucScanResultIEs
								[j])
							       - u4IELength);
					}
				}
			}
		}

		i++;
	}

	return;
}

#if CFG_TEST_WIFI_DIRECT_GO
VOID wlanEnableP2pFunction(IN P_ADAPTER_T prAdapter)
{
#if 0
	P_MSG_P2P_FUNCTION_SWITCH_T prMsgFuncSwitch =
	    (P_MSG_P2P_FUNCTION_SWITCH_T) NULL;

	prMsgFuncSwitch = (P_MSG_P2P_FUNCTION_SWITCH_T) cnmMemAlloc(prAdapter,
								    RAM_TYPE_MSG,
								    sizeof
								    (MSG_P2P_FUNCTION_SWITCH_T));
	if (!prMsgFuncSwitch) {
		ASSERT(FALSE);
		return;
	}

	prMsgFuncSwitch->rMsgHdr.eMsgId = MID_MNY_P2P_FUN_SWITCH;
	prMsgFuncSwitch->fgIsFuncOn = TRUE;

	mboxSendMsg(prAdapter,
		    MBOX_ID_0,
		    (P_MSG_HDR_T) prMsgFuncSwitch, MSG_SEND_METHOD_BUF);
#endif
	return;
}

VOID wlanEnableATGO(IN P_ADAPTER_T prAdapter)
{
	P_MSG_P2P_CONNECTION_REQUEST_T prMsgConnReq =
	    (P_MSG_P2P_CONNECTION_REQUEST_T) NULL;
	UINT_8 aucTargetDeviceID[MAC_ADDR_LEN] =
	    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

	prMsgConnReq = (P_MSG_P2P_CONNECTION_REQUEST_T) cnmMemAlloc(prAdapter,
								    RAM_TYPE_MSG,
								    sizeof
								    (MSG_P2P_CONNECTION_REQUEST_T));
	if (!prMsgConnReq) {
		ASSERT(FALSE);
		return;
	}

	prMsgConnReq->rMsgHdr.eMsgId = MID_MNY_P2P_CONNECTION_REQ;

/*=====Param Modified for test=====*/
	COPY_MAC_ADDR(prMsgConnReq->aucDeviceID, aucTargetDeviceID);
	prMsgConnReq->fgIsTobeGO = TRUE;
	prMsgConnReq->fgIsPersistentGroup = FALSE;

/*=====Param Modified for test=====*/

	mboxSendMsg(prAdapter,
		    MBOX_ID_0, (P_MSG_HDR_T) prMsgConnReq, MSG_SEND_METHOD_BUF);

	return;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to retrieve permanent address from firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanQueryPermanentAddress(IN P_ADAPTER_T prAdapter)
{
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_32 u4RxPktLength;
	UINT_8 aucBuffer[sizeof(WIFI_EVENT_T) + sizeof(EVENT_BASIC_CONFIG)];
	P_HIF_RX_HEADER_T prHifRxHdr;
	P_WIFI_EVENT_T prEvent;
	P_EVENT_BASIC_CONFIG prEventBasicConfig;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanQueryPermanentAddress");

/* 1. Allocate CMD Info Packet and its Buffer */
	prCmdInfo =
	    cmdBufAllocateCmdInfo(prAdapter, CMD_HDR_SIZE +
				  sizeof(CMD_BASIC_CONFIG));
	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen = CMD_HDR_SIZE + sizeof(CMD_BASIC_CONFIG);
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = CMD_ID_BASIC_CONFIG;
	prCmdInfo->fgSetQuery = FALSE;
	prCmdInfo->fgNeedResp = TRUE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = sizeof(CMD_BASIC_CONFIG);

/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount_UserPriority = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	wlanSendCommand(prAdapter, prCmdInfo);
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	if (nicRxWaitResponse(prAdapter,
			      1,
			      aucBuffer,
			      sizeof(WIFI_EVENT_T) + sizeof(EVENT_BASIC_CONFIG),
			      &u4RxPktLength) != WLAN_STATUS_SUCCESS) {
		return WLAN_STATUS_FAILURE;
	}

/* header checking .. */
	prHifRxHdr = (P_HIF_RX_HEADER_T) aucBuffer;
	if (prHifRxHdr->u2PacketType != HIF_RX_PKT_TYPE_EVENT) {
		return WLAN_STATUS_FAILURE;
	}

	prEvent = (P_WIFI_EVENT_T) aucBuffer;
	if (prEvent->ucEID != EVENT_ID_BASIC_CONFIG) {
		return WLAN_STATUS_FAILURE;
	}

	prEventBasicConfig = (P_EVENT_BASIC_CONFIG) (prEvent->aucBuffer);

	COPY_MAC_ADDR(prAdapter->rWifiVar.aucPermanentAddress,
		      &(prEventBasicConfig->rMyMacAddr));
	COPY_MAC_ADDR(prAdapter->rWifiVar.aucMacAddress,
		      &(prEventBasicConfig->rMyMacAddr));

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to retrieve NIC capability from firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanQueryNicCapability(IN P_ADAPTER_T prAdapter)
{
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_32 u4RxPktLength;
	UINT_8 aucBuffer[sizeof(WIFI_EVENT_T) + sizeof(EVENT_NIC_CAPABILITY)];
	P_HIF_RX_HEADER_T prHifRxHdr;
	P_WIFI_EVENT_T prEvent;
	P_EVENT_NIC_CAPABILITY prEventNicCapability;

	ASSERT(prAdapter);

	DEBUGFUNC("wlanQueryNicCapability");

/* 1. Allocate CMD Info Packet and its Buffer */
	prCmdInfo =
	    cmdBufAllocateCmdInfo(prAdapter, CMD_HDR_SIZE +
				  sizeof(EVENT_NIC_CAPABILITY));
	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}

/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen = CMD_HDR_SIZE + sizeof(EVENT_NIC_CAPABILITY);
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = CMD_ID_GET_NIC_CAPABILITY;
	prCmdInfo->fgSetQuery = FALSE;
	prCmdInfo->fgNeedResp = TRUE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = 0;

/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount_UserPriority = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;

	wlanSendCommand(prAdapter, prCmdInfo);
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	if (nicRxWaitResponse(prAdapter,
			      1,
			      aucBuffer,
			      sizeof(WIFI_EVENT_T) +
			      sizeof(EVENT_NIC_CAPABILITY),
			      &u4RxPktLength) != WLAN_STATUS_SUCCESS) {
		return WLAN_STATUS_FAILURE;
	}

/* header checking .. */
	prHifRxHdr = (P_HIF_RX_HEADER_T) aucBuffer;
	if (prHifRxHdr->u2PacketType != HIF_RX_PKT_TYPE_EVENT) {
		return WLAN_STATUS_FAILURE;
	}

	prEvent = (P_WIFI_EVENT_T) aucBuffer;
	if (prEvent->ucEID != EVENT_ID_NIC_CAPABILITY) {
		return WLAN_STATUS_FAILURE;
	}

	prEventNicCapability = (P_EVENT_NIC_CAPABILITY) (prEvent->aucBuffer);

	prAdapter->rVerInfo.u2FwProductID = prEventNicCapability->u2ProductID;
	prAdapter->rVerInfo.u2FwOwnVersion = prEventNicCapability->u2FwVersion;
	prAdapter->rVerInfo.u2FwPeerVersion =
	    prEventNicCapability->u2DriverVersion;
	prAdapter->fgIsHw5GBandDisabled =
	    (BOOLEAN) prEventNicCapability->ucHw5GBandDisabled;
	prAdapter->fgIsEepromUsed =
	    (BOOLEAN) prEventNicCapability->ucEepromUsed;
	prAdapter->fgIsEfuseValid =
	    (BOOLEAN) prEventNicCapability->ucEfuseValid;
	prAdapter->fgIsEmbbededMacAddrValid =
	    (BOOLEAN) prEventNicCapability->ucMacAddrValid;

#if CFG_ENABLE_CAL_LOG
	DBGLOG(INIT, INFO, (" RF CAL FAIL  = (%d),BB CAL FAIL  = (%d)\n",
			    prEventNicCapability->ucRfCalFail,
			    prEventNicCapability->ucBbCalFail));
#endif
	return WLAN_STATUS_SUCCESS;
}

#ifdef MT6628
static INT_32 wlanChangeCodeWord(INT_32 au4Input)
{
	UINT_16 i;

#if TXPWR_USE_PDSLOPE
	CODE_MAPPING_T arCodeTable[] = {
		{0X100, -40},
		{0X104, -35},
		{0X128, -30},
		{0X14C, -25},
		{0X170, -20},
		{0X194, -15},
		{0X1B8, -10},
		{0X1DC, -5},
		{0, 0},
		{0X24, 5},
		{0X48, 10},
		{0X6C, 15},
		{0X90, 20},
		{0XB4, 25},
		{0XD8, 30},
		{0XFC, 35},
		{0XFF, 40},
	};
#else
	CODE_MAPPING_T arCodeTable[] = {
		{0X100, 0x80},
		{0X104, 0x80},
		{0X128, 0x80},
		{0X14C, 0x80},
		{0X170, 0x80},
		{0X194, 0x94},
		{0X1B8, 0XB8},
		{0X1DC, 0xDC},
		{0, 0},
		{0X24, 0x24},
		{0X48, 0x48},
		{0X6C, 0x6c},
		{0X90, 0x7F},
		{0XB4, 0x7F},
		{0XD8, 0x7F},
		{0XFC, 0x7F},
		{0XFF, 0x7F},
	};
#endif

	for (i = 0; i < sizeof(arCodeTable) / sizeof(CODE_MAPPING_T); i++) {
		if (arCodeTable[i].u4RegisterValue == au4Input) {
			return arCodeTable[i].u4TxpowerOffset;
		}
	}

	return 0;
}
#endif
#if TXPWR_USE_PDSLOPE

/*----------------------------------------------------------------------------*/
/*!
* @brief
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanQueryPdMcr(IN P_ADAPTER_T prAdapter, P_PARAM_MCR_RW_STRUC_T prMcrRdInfo)
{
	UINT_8 ucCmdSeqNum;
	P_CMD_INFO_T prCmdInfo;
	P_WIFI_CMD_T prWifiCmd;
	UINT_32 u4RxPktLength;
	UINT_8 aucBuffer[sizeof(WIFI_EVENT_T) + sizeof(CMD_ACCESS_REG)];
	P_HIF_RX_HEADER_T prHifRxHdr;
	P_WIFI_EVENT_T prEvent;
	P_CMD_ACCESS_REG prCmdMcrQuery;

	ASSERT(prAdapter);

/* 1. Allocate CMD Info Packet and its Buffer */
	prCmdInfo =
	    cmdBufAllocateCmdInfo(prAdapter, CMD_HDR_SIZE +
				  sizeof(CMD_ACCESS_REG));

	if (!prCmdInfo) {
		DBGLOG(INIT, ERROR, ("Allocate CMD_INFO_T ==> FAILED.\n"));
		return WLAN_STATUS_FAILURE;
	}
/* increase command sequence number */
	ucCmdSeqNum = nicIncreaseCmdSeqNum(prAdapter);

/* compose CMD_BUILD_CONNECTION cmd pkt */
	prCmdInfo->eCmdType = COMMAND_TYPE_GENERAL_IOCTL;
	prCmdInfo->u2InfoBufLen =
	    (UINT_16) (CMD_HDR_SIZE + sizeof(CMD_ACCESS_REG));
	prCmdInfo->pfCmdDoneHandler = NULL;
	prCmdInfo->pfCmdTimeoutHandler = nicOidCmdTimeoutCommon;
	prCmdInfo->fgIsOid = FALSE;
	prCmdInfo->ucCID = CMD_ID_ACCESS_REG;
	prCmdInfo->fgSetQuery = FALSE;
	prCmdInfo->fgNeedResp = TRUE;
	prCmdInfo->fgDriverDomainMCR = FALSE;
	prCmdInfo->ucCmdSeqNum = ucCmdSeqNum;
	prCmdInfo->u4SetInfoLen = sizeof(CMD_ACCESS_REG);

/* Setup WIFI_CMD_T */
	prWifiCmd = (P_WIFI_CMD_T) (prCmdInfo->pucInfoBuffer);
	prWifiCmd->u2TxByteCount_UserPriority = prCmdInfo->u2InfoBufLen;
	prWifiCmd->ucCID = prCmdInfo->ucCID;
	prWifiCmd->ucSetQuery = prCmdInfo->fgSetQuery;
	prWifiCmd->ucSeqNum = prCmdInfo->ucCmdSeqNum;
	kalMemCopy(prWifiCmd->aucBuffer, prMcrRdInfo, sizeof(CMD_ACCESS_REG));

	wlanSendCommand(prAdapter, prCmdInfo);
	cmdBufFreeCmdInfo(prAdapter, prCmdInfo);

	if (nicRxWaitResponse(prAdapter,
			      1,
			      aucBuffer,
			      sizeof(WIFI_EVENT_T) + sizeof(CMD_ACCESS_REG),
			      &u4RxPktLength) != WLAN_STATUS_SUCCESS) {
		return WLAN_STATUS_FAILURE;
	}

/* header checking .. */
	prHifRxHdr = (P_HIF_RX_HEADER_T) aucBuffer;
	if (prHifRxHdr->u2PacketType != HIF_RX_PKT_TYPE_EVENT) {
		return WLAN_STATUS_FAILURE;
	}

	prEvent = (P_WIFI_EVENT_T) aucBuffer;

	if (prEvent->ucEID != EVENT_ID_ACCESS_REG) {
		return WLAN_STATUS_FAILURE;
	}

	prCmdMcrQuery = (P_CMD_ACCESS_REG) (prEvent->aucBuffer);
	prMcrRdInfo->u4McrOffset = prCmdMcrQuery->u4Address;
	prMcrRdInfo->u4McrData = prCmdMcrQuery->u4Data;

	return WLAN_STATUS_SUCCESS;
}

static INT_32 wlanIntRound(INT_32 au4Input)
{
	if (au4Input >= 0) {
		if ((au4Input % 10) == 5) {
			au4Input = au4Input + 5;
			return au4Input;
		}
	}

	if (au4Input < 0) {
		if ((au4Input % 10) == -5) {
			au4Input = au4Input - 5;
			return au4Input;
		}
	}

	return au4Input;
}

static INT_32 wlanCal6628EfuseForm(IN P_ADAPTER_T prAdapter, INT_32 au4Input)
{
	PARAM_MCR_RW_STRUC_T rMcrRdInfo;
	INT_32 au4PdSlope, au4TxPwrOffset, au4TxPwrOffset_Round;
	INT_8 auTxPwrOffset_Round;

	rMcrRdInfo.u4McrOffset = 0x60205c68;
	rMcrRdInfo.u4McrData = 0;
	au4TxPwrOffset = au4Input;
	wlanQueryPdMcr(prAdapter, &rMcrRdInfo);

	au4PdSlope = (rMcrRdInfo.u4McrData) & BITS(0, 6);
	au4TxPwrOffset_Round = wlanIntRound((au4TxPwrOffset * au4PdSlope)) / 10;

	au4TxPwrOffset_Round = -au4TxPwrOffset_Round;

	if (au4TxPwrOffset_Round < -128) {
		au4TxPwrOffset_Round = 128;
	} else if (au4TxPwrOffset_Round < 0) {
		au4TxPwrOffset_Round += 256;
	} else if (au4TxPwrOffset_Round > 127) {
		au4TxPwrOffset_Round = 127;
	}

	auTxPwrOffset_Round = (UINT8) au4TxPwrOffset_Round;

	return au4TxPwrOffset_Round;
}

#endif

#ifdef MT6628
static VOID wlanChangeNvram6620to6628(PUINT_8 pucEFUSE)
{
#define EFUSE_CH_OFFSET1_L_MASK_6620         BITS(0, 8)
#define EFUSE_CH_OFFSET1_L_SHIFT_6620        0
#define EFUSE_CH_OFFSET1_M_MASK_6620         BITS(9, 17)
#define EFUSE_CH_OFFSET1_M_SHIFT_6620        9
#define EFUSE_CH_OFFSET1_H_MASK_6620         BITS(18, 26)
#define EFUSE_CH_OFFSET1_H_SHIFT_6620        18
#define EFUSE_CH_OFFSET1_VLD_MASK_6620       BIT(27)
#define EFUSE_CH_OFFSET1_VLD_SHIFT_6620      27

#define EFUSE_CH_OFFSET1_L_MASK_5931         BITS(0, 7)
#define EFUSE_CH_OFFSET1_L_SHIFT_5931        0
#define EFUSE_CH_OFFSET1_M_MASK_5931         BITS(8, 15)
#define EFUSE_CH_OFFSET1_M_SHIFT_5931        8
#define EFUSE_CH_OFFSET1_H_MASK_5931         BITS(16, 23)
#define EFUSE_CH_OFFSET1_H_SHIFT_5931        16
#define EFUSE_CH_OFFSET1_VLD_MASK_5931       BIT(24)
#define EFUSE_CH_OFFSET1_VLD_SHIFT_5931      24
#define EFUSE_ALL_CH_OFFSET1_MASK_5931       BITS(25, 27)
#define EFUSE_ALL_CH_OFFSET1_SHIFT_5931      25

	INT_32 au4ChOffset;
	INT_16 au2ChOffsetL, au2ChOffsetM, au2ChOffsetH;

	au4ChOffset = *(UINT_32 *) (pucEFUSE + 72);

	if ((au4ChOffset & EFUSE_CH_OFFSET1_VLD_MASK_6620) &&
	    ((*(UINT_32 *) (pucEFUSE + 28)) == 0)) {
		au2ChOffsetL = ((au4ChOffset & EFUSE_CH_OFFSET1_L_MASK_6620) >>
				EFUSE_CH_OFFSET1_L_SHIFT_6620);

		au2ChOffsetM = ((au4ChOffset & EFUSE_CH_OFFSET1_M_MASK_6620) >>
				EFUSE_CH_OFFSET1_M_SHIFT_6620);

		au2ChOffsetH = ((au4ChOffset & EFUSE_CH_OFFSET1_H_MASK_6620) >>
				EFUSE_CH_OFFSET1_H_SHIFT_6620);

		au2ChOffsetL = wlanChangeCodeWord(au2ChOffsetL);
		au2ChOffsetM = wlanChangeCodeWord(au2ChOffsetM);
		au2ChOffsetH = wlanChangeCodeWord(au2ChOffsetH);

		au4ChOffset = 0;
		au4ChOffset |= *(UINT_32 *) (pucEFUSE + 72)
		    >> (EFUSE_CH_OFFSET1_VLD_SHIFT_6620 -
			EFUSE_CH_OFFSET1_VLD_SHIFT_5931) &
		    EFUSE_CH_OFFSET1_VLD_MASK_5931;

		au4ChOffset |=
		    ((((UINT_32) au2ChOffsetL) <<
		      EFUSE_CH_OFFSET1_L_SHIFT_5931) &
		     EFUSE_CH_OFFSET1_L_MASK_5931);
		au4ChOffset |=
		    ((((UINT_32) au2ChOffsetM) <<
		      EFUSE_CH_OFFSET1_M_SHIFT_5931) &
		     EFUSE_CH_OFFSET1_M_MASK_5931);
		au4ChOffset |=
		    ((((UINT_32) au2ChOffsetH) <<
		      EFUSE_CH_OFFSET1_H_SHIFT_5931) &
		     EFUSE_CH_OFFSET1_H_MASK_5931);

		*((INT_32 *) ((pucEFUSE + 28))) = au4ChOffset;
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to load manufacture data from NVRAM
* if available and valid
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param prRegInfo      Pointer of REG_INFO_T
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_FAILURE
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanLoadManufactureData(IN P_ADAPTER_T prAdapter, IN P_REG_INFO_T prRegInfo)
{
#if CFG_SUPPORT_RDD_TEST_MODE
	CMD_RDD_CH_T rRddParam;
#endif

	ASSERT(prAdapter);

/* 1. Version Check */
	kalGetConfigurationVersion(prAdapter->prGlueInfo,
				   &(prAdapter->rVerInfo.u2Part1CfgOwnVersion),
				   &(prAdapter->rVerInfo.u2Part1CfgPeerVersion),
				   &(prAdapter->rVerInfo.u2Part2CfgOwnVersion),
				   &(prAdapter->rVerInfo.
				     u2Part2CfgPeerVersion));

#if (CFG_SW_NVRAM_VERSION_CHECK == 1)
	if (CFG_DRV_OWN_VERSION < prAdapter->rVerInfo.u2Part1CfgPeerVersion
	    || CFG_DRV_OWN_VERSION < prAdapter->rVerInfo.u2Part2CfgPeerVersion
	    || prAdapter->rVerInfo.u2Part1CfgOwnVersion < CFG_DRV_PEER_VERSION
	    || prAdapter->rVerInfo.u2Part2CfgOwnVersion <
	    CFG_DRV_PEER_VERSION) {
		return WLAN_STATUS_FAILURE;
	}
#endif

/* MT6620 E1/E2 would be ignored directly */
	if (prAdapter->rVerInfo.u2Part1CfgOwnVersion == 0x0001) {
		prRegInfo->ucTxPwrValid = 1;
	} else {
/* 2. Load TX power gain parameters if valid */
		if (prRegInfo->ucTxPwrValid != 0) {
/* send to F/W */
			nicUpdateTxPower(prAdapter,
					 (P_CMD_TX_PWR_T) (&
							   (prRegInfo->
							    rTxPwr)));
		}
	}

/* 3. Check if needs to support 5GHz */
	if (prRegInfo->ucEnable5GBand) {
/* check if it is disabled by hardware */
		if (prAdapter->fgIsHw5GBandDisabled
		    || prRegInfo->ucSupport5GBand == 0) {
			prAdapter->fgEnable5GBand = FALSE;
		} else {
			prAdapter->fgEnable5GBand = TRUE;
		}
	} else {
		prAdapter->fgEnable5GBand = FALSE;
	}

/* 4. Send EFUSE data */
#if  defined(MT6628)
	wlanChangeNvram6620to6628(prRegInfo->aucEFUSE);
#endif

	wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SET_PHY_PARAM,
			    TRUE,
			    FALSE,
			    FALSE,
			    NULL,
			    NULL,
			    sizeof(CMD_PHY_PARAM_T),
			    (PUINT_8) (prRegInfo->aucEFUSE), NULL, 0);

#if CFG_SUPPORT_RDD_TEST_MODE
	rRddParam.ucRddTestMode = (UINT_8) prRegInfo->u4RddTestMode;
	rRddParam.ucRddShutCh = (UINT_8) prRegInfo->u4RddShutFreq;
	rRddParam.ucRddStartCh =
	    (UINT_8) nicFreq2ChannelNum(prRegInfo->u4RddStartFreq);
	rRddParam.ucRddStopCh =
	    (UINT_8) nicFreq2ChannelNum(prRegInfo->u4RddStopFreq);
	rRddParam.ucRddDfs = (UINT_8) prRegInfo->u4RddDfs;
	prAdapter->ucRddStatus = 0;
	nicUpdateRddTestMode(prAdapter, (P_CMD_RDD_CH_T) (&rRddParam));
#endif

/* 5. Get 16-bits Country Code and Bandwidth */
	prAdapter->rWifiVar.rConnSettings.u2CountryCode =
	    (((UINT_16) prRegInfo->au2CountryCode[0]) << 8) |
	    (((UINT_16) prRegInfo->au2CountryCode[1]) & BITS(0, 7));

#if 0				/* Bandwidth control will be controlled by GUI. 20110930
				 * So ignore the setting from registry/NVRAM
				 */
	prAdapter->rWifiVar.rConnSettings.uc2G4BandwidthMode =
	    prRegInfo->uc2G4BwFixed20M ? CONFIG_BW_20M : CONFIG_BW_20_40M;
	prAdapter->rWifiVar.rConnSettings.uc5GBandwidthMode =
	    prRegInfo->uc5GBwFixed20M ? CONFIG_BW_20M : CONFIG_BW_20_40M;
#endif

/* 6. Set domain and channel information to chip */
	rlmDomainSendCmd(prAdapter, FALSE);

/* 7. set band edge tx power if available */
	if (prRegInfo->fg2G4BandEdgePwrUsed) {
		CMD_EDGE_TXPWR_LIMIT_T rCmdEdgeTxPwrLimit;

		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrCCK
		    = prRegInfo->cBandEdgeMaxPwrCCK;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM20
		    = prRegInfo->cBandEdgeMaxPwrOFDM20;
		rCmdEdgeTxPwrLimit.cBandEdgeMaxPwrOFDM40
		    = prRegInfo->cBandEdgeMaxPwrOFDM40;

		wlanSendSetQueryCmd(prAdapter,
				    CMD_ID_SET_EDGE_TXPWR_LIMIT,
				    TRUE,
				    FALSE,
				    FALSE,
				    NULL,
				    NULL,
				    sizeof(CMD_EDGE_TXPWR_LIMIT_T),
				    (PUINT_8) & rCmdEdgeTxPwrLimit, NULL, 0);
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check
*        Media Stream Mode is set to non-default value or not,
*        and clear to default value if above criteria is met
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return TRUE
*           The media stream mode was non-default value and has been reset
*         FALSE
*           The media stream mode is default value
*/
/*----------------------------------------------------------------------------*/
BOOLEAN wlanResetMediaStreamMode(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	if (prAdapter->rWlanInfo.eLinkAttr.ucMediaStreamMode != 0) {
		prAdapter->rWlanInfo.eLinkAttr.ucMediaStreamMode = 0;

		return TRUE;
	} else {
		return FALSE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if any pending timer has expired
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanTimerTimeoutCheck(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	cnmTimerDoTimeOutCheck(prAdapter);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to check if any pending mailbox message
*        to be handled
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanProcessMboxMessage(IN P_ADAPTER_T prAdapter)
{
	UINT_32 i;

	ASSERT(prAdapter);

	for (i = 0; i < MBOX_ID_TOTAL_NUM; i++)
		mboxRcvAllMsg(prAdapter, (ENUM_MBOX_ID_T) i);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to enqueue a single TX packet into CORE
*
* @param prAdapter      Pointer of Adapter Data Structure
*        prNativePacket Pointer of Native Packet
*
* @return WLAN_STATUS_SUCCESS
*         WLAN_STATUS_RESOURCES
*         WLAN_STATUS_INVALID_PACKET
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanEnqueueTxPacket(IN P_ADAPTER_T prAdapter, IN P_NATIVE_PACKET prNativePacket)
{
	P_TX_CTRL_T prTxCtrl;
	P_MSDU_INFO_T prMsduInfo;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);

	prTxCtrl = &prAdapter->rTxCtrl;

	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_MSDU_INFO_LIST);
	QUEUE_REMOVE_HEAD(&prTxCtrl->rFreeMsduInfoList,
			  prMsduInfo, P_MSDU_INFO_T);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_TX_MSDU_INFO_LIST);

	if (prMsduInfo == NULL) {
		return WLAN_STATUS_RESOURCES;
	} else {
		prMsduInfo->eSrc = TX_PACKET_OS;

		if (nicTxFillMsduInfo(prAdapter, prMsduInfo, prNativePacket) == FALSE) {	/* packet is not extractable */
			kalSendComplete(prAdapter->prGlueInfo,
					prNativePacket,
					WLAN_STATUS_INVALID_PACKET);

			nicTxReturnMsduInfo(prAdapter, prMsduInfo);

			return WLAN_STATUS_INVALID_PACKET;
		} else {
/* enqueue to QM */
			nicTxEnqueueMsdu(prAdapter, prMsduInfo);

			return WLAN_STATUS_SUCCESS;
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to flush pending TX packets in CORE
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanFlushTxPendingPackets(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return nicTxFlush(prAdapter);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief this function sends pending MSDU_INFO_T to MT6620
*
* @param prAdapter      Pointer to the Adapter structure.
* @param pfgHwAccess    Pointer for tracking LP-OWN status
*
* @retval WLAN_STATUS_SUCCESS   Reset is done successfully.
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
wlanTxPendingPackets(IN P_ADAPTER_T prAdapter, IN OUT PBOOLEAN pfgHwAccess)
{
	P_TX_CTRL_T prTxCtrl;
	P_MSDU_INFO_T prMsduInfo;

	KAL_SPIN_LOCK_DECLARATION();

	ASSERT(prAdapter);
	prTxCtrl = &prAdapter->rTxCtrl;

	ASSERT(pfgHwAccess);

/* <1> dequeue packet by txDequeuTxPackets() */
	KAL_ACQUIRE_SPIN_LOCK(prAdapter, SPIN_LOCK_QM_TX_QUEUE);
	prMsduInfo = qmDequeueTxPackets(prAdapter, &prTxCtrl->rTc);
	KAL_RELEASE_SPIN_LOCK(prAdapter, SPIN_LOCK_QM_TX_QUEUE);

	if (prMsduInfo != NULL) {
		if (kalIsCardRemoved(prAdapter->prGlueInfo) == FALSE) {
/* <2> Acquire LP-OWN if necessary */
			if (*pfgHwAccess == FALSE) {
				*pfgHwAccess = TRUE;

				wlanAcquirePowerControl(prAdapter);
			}
#if (MT6620_E1_ASIC_HIFSYS_WORKAROUND == 1)
			if (prAdapter->fgIsClockGatingEnabled == TRUE) {
				nicDisableClockGating(prAdapter);
			}
#endif
/* <3> send packets */
			nicTxMsduInfoList(prAdapter, prMsduInfo);

/* <4> update TC by txAdjustTcQuotas() */
			nicTxAdjustTcq(prAdapter);
		} else {
			wlanProcessQueuedMsduInfo(prAdapter, prMsduInfo);
		}
	}
#if (MT6620_E1_ASIC_HIFSYS_WORKAROUND == 1)
	if (prAdapter->fgIsClockGatingEnabled == FALSE) {
		nicEnableClockGating(prAdapter);
	}
#endif

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to acquire power control from firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanAcquirePowerControl(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	ACQUIRE_POWER_CONTROL_FROM_PM(prAdapter);

/* Reset sleepy state */
	if (prAdapter->fgWiFiInSleepyState == TRUE) {
		prAdapter->fgWiFiInSleepyState = FALSE;
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to release power control to firmware
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanReleasePowerControl(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	RECLAIM_POWER_CONTROL_TO_PM(prAdapter, FALSE);

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is called to report currently pending TX frames count
*        (command packets are not included)
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return number of pending TX frames
*/
/*----------------------------------------------------------------------------*/
UINT_32 wlanGetTxPendingFrameCount(IN P_ADAPTER_T prAdapter)
{
	P_TX_CTRL_T prTxCtrl;
	UINT_32 u4Num;

	ASSERT(prAdapter);
	prTxCtrl = &prAdapter->rTxCtrl;

	u4Num = kalGetTxPendingFrameCount(prAdapter->prGlueInfo) +
	    (UINT_32) (prTxCtrl->i4PendingFwdFrameCount);

	return u4Num;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to report current ACPI state
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return ACPI_STATE_D0 Normal Operation Mode
*         ACPI_STATE_D3 Suspend Mode
*/
/*----------------------------------------------------------------------------*/
ENUM_ACPI_STATE_T wlanGetAcpiState(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	return prAdapter->rAcpiState;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to update current ACPI state only
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param ePowerState    ACPI_STATE_D0 Normal Operation Mode
*                       ACPI_STATE_D3 Suspend Mode
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID
wlanSetAcpiState(IN P_ADAPTER_T prAdapter, IN ENUM_ACPI_STATE_T ePowerState)
{
	ASSERT(prAdapter);
	ASSERT(ePowerState <= ACPI_STATE_D3);

	prAdapter->rAcpiState = ePowerState;

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to query ECO version from HIFSYS CR
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return zero      Unable to retrieve ECO version information
*         non-zero  ECO version (1-based)
*/
/*----------------------------------------------------------------------------*/
UINT_8 wlanGetEcoVersion(IN P_ADAPTER_T prAdapter)
{
	ASSERT(prAdapter);

	if (nicVerifyChipID(prAdapter) == TRUE) {
		return prAdapter->ucRevID + 1;
	} else {
		return 0;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to setting the default Tx Power configuration
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return zero      Unable to retrieve ECO version information
*         non-zero  ECO version (1-based)
*/
/*----------------------------------------------------------------------------*/
VOID wlanDefTxPowerCfg(IN P_ADAPTER_T prAdapter)
{
	UINT_8 i;
	P_GLUE_INFO_T prGlueInfo = prAdapter->prGlueInfo;
	P_SET_TXPWR_CTRL_T prTxpwr;

	ASSERT(prGlueInfo);

	prTxpwr = &prGlueInfo->rTxPwr;

	prTxpwr->c2GLegacyStaPwrOffset = 0;
	prTxpwr->c2GHotspotPwrOffset = 0;
	prTxpwr->c2GP2pPwrOffset = 0;
	prTxpwr->c2GBowPwrOffset = 0;
	prTxpwr->c5GLegacyStaPwrOffset = 0;
	prTxpwr->c5GHotspotPwrOffset = 0;
	prTxpwr->c5GP2pPwrOffset = 0;
	prTxpwr->c5GBowPwrOffset = 0;
	prTxpwr->ucConcurrencePolicy = 0;
	for (i = 0; i < 3; i++)
		prTxpwr->acReserved1[i] = 0;

	for (i = 0; i < 14; i++)
		prTxpwr->acTxPwrLimit2G[i] = 63;

	for (i = 0; i < 4; i++)
		prTxpwr->acTxPwrLimit5G[i] = 63;

	for (i = 0; i < 2; i++)
		prTxpwr->acReserved2[i] = 0;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        set preferred band configuration corresponding to network type
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param eBand          Given band
* @param eNetTypeIndex  Given Network Type
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID
wlanSetPreferBandByNetwork(IN P_ADAPTER_T prAdapter,
			   IN ENUM_BAND_T eBand,
			   IN ENUM_NETWORK_TYPE_INDEX_T eNetTypeIndex)
{
	ASSERT(prAdapter);
	ASSERT(eBand <= BAND_NUM);
	ASSERT(eNetTypeIndex <= NETWORK_TYPE_INDEX_NUM);

/* 1. set prefer band according to network type */
	prAdapter->aePreferBand[eNetTypeIndex] = eBand;

/* 2. remove buffered BSS descriptors correspondingly */
	if (eBand == BAND_2G4) {
		scanRemoveBssDescByBandAndNetwork(prAdapter,
						  BAND_5G, eNetTypeIndex);
	} else if (eBand == BAND_5G) {
		scanRemoveBssDescByBandAndNetwork(prAdapter,
						  BAND_2G4, eNetTypeIndex);
	}

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        get channel information corresponding to specified network type
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param eNetTypeIndex  Given Network Type
*
* @return channel number
*/
/*----------------------------------------------------------------------------*/
UINT_8
wlanGetChannelNumberByNetwork(IN P_ADAPTER_T prAdapter,
			      IN ENUM_NETWORK_TYPE_INDEX_T eNetTypeIndex)
{
	P_BSS_INFO_T prBssInfo;

	ASSERT(prAdapter);
	ASSERT(eNetTypeIndex <= NETWORK_TYPE_INDEX_NUM);

	prBssInfo = &(prAdapter->rWifiVar.arBssInfo[eNetTypeIndex]);

	return prBssInfo->ucPrimaryChannel;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        get BSS descriptor information corresponding to specified network type
*
* @param prAdapter      Pointer of Adapter Data Structure
* @param eNetTypeIndex  Given Network Type
*
* @return pointer to BSS_DESC_T
*/
/*----------------------------------------------------------------------------*/
P_BSS_DESC_T
wlanGetTargetBssDescByNetwork(IN P_ADAPTER_T prAdapter,
			      IN ENUM_NETWORK_TYPE_INDEX_T eNetTypeIndex)
{
	ASSERT(prAdapter);
	ASSERT(eNetTypeIndex <= NETWORK_TYPE_INDEX_NUM);

	switch (eNetTypeIndex) {
	case NETWORK_TYPE_AIS_INDEX:
		return prAdapter->rWifiVar.rAisFsmInfo.prTargetBssDesc;

	case NETWORK_TYPE_P2P_INDEX:
		return NULL;

	case NETWORK_TYPE_BOW_INDEX:
		return prAdapter->rWifiVar.rBowFsmInfo.prTargetBssDesc;

	default:
		return NULL;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is to
*        check unconfigured system properties and generate related message on
*        scan list to notify users
*
* @param prAdapter      Pointer of Adapter Data Structure
*
* @return WLAN_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS wlanCheckSystemConfiguration(IN P_ADAPTER_T prAdapter)
{
#if (CFG_NVRAM_EXISTENCE_CHECK == 1) || (CFG_SW_NVRAM_VERSION_CHECK == 1)
	const UINT_8 aucZeroMacAddr[] = NULL_MAC_ADDR;
	const UINT_8 aucBCAddr[] = BC_MAC_ADDR;
	BOOLEAN fgIsConfExist = TRUE;
	BOOLEAN fgGenErrMsg = FALSE;
	P_REG_INFO_T prRegInfo = NULL;
	P_WLAN_BEACON_FRAME_T prBeacon = NULL;
	P_IE_SSID_T prSsid = NULL;
	UINT_32 u4ErrCode = 0;
	UINT_8 aucErrMsg[32];
	PARAM_SSID_T rSsid;
	PARAM_802_11_CONFIG_T rConfiguration;
	PARAM_RATES_EX rSupportedRates;
#endif

	DEBUGFUNC("wlanCheckSystemConfiguration");

	ASSERT(prAdapter);

#if (CFG_NVRAM_EXISTENCE_CHECK == 1)
	if (kalIsConfigurationExist(prAdapter->prGlueInfo) == FALSE) {
		fgIsConfExist = FALSE;
		fgGenErrMsg = TRUE;
	}
#endif

#if (CFG_SW_NVRAM_VERSION_CHECK == 1)
	prRegInfo = kalGetConfiguration(prAdapter->prGlueInfo);

	if (fgIsConfExist == TRUE && (CFG_DRV_OWN_VERSION < prAdapter->rVerInfo.u2Part1CfgPeerVersion || CFG_DRV_OWN_VERSION < prAdapter->rVerInfo.u2Part2CfgPeerVersion || prAdapter->rVerInfo.u2Part1CfgOwnVersion < CFG_DRV_PEER_VERSION || prAdapter->rVerInfo.u2Part2CfgOwnVersion < CFG_DRV_PEER_VERSION	/* NVRAM */
				      || CFG_DRV_OWN_VERSION <
				      prAdapter->rVerInfo.u2FwPeerVersion
				      || prAdapter->rVerInfo.u2FwOwnVersion <
				      CFG_DRV_PEER_VERSION
				      || (prAdapter->fgIsEmbbededMacAddrValid ==
					  FALSE
					  &&
					  (IS_BMCAST_MAC_ADDR
					   (prRegInfo->aucMacAddr)
					   || EQUAL_MAC_ADDR(aucZeroMacAddr,
							     prRegInfo->
							     aucMacAddr)))
				      || prRegInfo->ucTxPwrValid == 0)) {
		fgGenErrMsg = TRUE;
	}
#endif

	if (fgGenErrMsg == TRUE) {
		prBeacon =
		    cnmMemAlloc(prAdapter,
				RAM_TYPE_BUF,
				sizeof(WLAN_BEACON_FRAME_T) +
				sizeof(IE_SSID_T));

/* initialization */
		kalMemZero(prBeacon, sizeof(WLAN_BEACON_FRAME_T) +
			   sizeof(IE_SSID_T));

/* prBeacon initialization */
		prBeacon->u2FrameCtrl = MAC_FRAME_BEACON;
		COPY_MAC_ADDR(prBeacon->aucDestAddr, aucBCAddr);
		COPY_MAC_ADDR(prBeacon->aucSrcAddr, aucZeroMacAddr);
		COPY_MAC_ADDR(prBeacon->aucBSSID, aucZeroMacAddr);
		prBeacon->u2BeaconInterval = 100;
		prBeacon->u2CapInfo = CAP_INFO_ESS;

/* prSSID initialization */
		prSsid = (P_IE_SSID_T) (&prBeacon->aucInfoElem[0]);
		prSsid->ucId = ELEM_ID_SSID;

/* rConfiguration initialization */
		rConfiguration.u4Length = sizeof(PARAM_802_11_CONFIG_T);
		rConfiguration.u4BeaconPeriod = 100;
		rConfiguration.u4ATIMWindow = 1;
		rConfiguration.u4DSConfig = 2412;
		rConfiguration.rFHConfig.u4Length =
		    sizeof(PARAM_802_11_CONFIG_FH_T);

/* rSupportedRates initialization */
		kalMemZero(rSupportedRates, sizeof(PARAM_RATES_EX));
	}
#if (CFG_NVRAM_EXISTENCE_CHECK == 1)
#define NVRAM_ERR_MSG "NVRAM WARNING: Err = 0x01"
	if (kalIsConfigurationExist(prAdapter->prGlueInfo) == FALSE) {
		COPY_SSID(prSsid->aucSSID,
			  prSsid->ucLength,
			  NVRAM_ERR_MSG, strlen(NVRAM_ERR_MSG));

		kalIndicateBssInfo(prAdapter->prGlueInfo,
				   (PUINT_8) prBeacon,
				   OFFSET_OF(WLAN_BEACON_FRAME_T,
					     aucInfoElem) +
				   OFFSET_OF(IE_SSID_T,
					     aucSSID) + prSsid->ucLength, 1, 0);

		COPY_SSID(rSsid.aucSsid, rSsid.u4SsidLen, NVRAM_ERR_MSG,
			  strlen(NVRAM_ERR_MSG));
		nicAddScanResult(prAdapter,
				 prBeacon->aucBSSID,
				 &rSsid,
				 0,
				 0,
				 PARAM_NETWORK_TYPE_FH,
				 &rConfiguration,
				 NET_TYPE_INFRA,
				 rSupportedRates,
				 OFFSET_OF(WLAN_BEACON_FRAME_T,
					   aucInfoElem) + OFFSET_OF(IE_SSID_T,
								    aucSSID) +
				 prSsid->ucLength - WLAN_MAC_MGMT_HEADER_LEN,
				 (PUINT_8) ((UINT_32) (prBeacon) +
					    WLAN_MAC_MGMT_HEADER_LEN));
	}
#endif

#if (CFG_SW_NVRAM_VERSION_CHECK == 1)
#define VER_ERR_MSG     "NVRAM WARNING: Err = 0x%02X"
	if (fgIsConfExist == TRUE) {
		if ((CFG_DRV_OWN_VERSION < prAdapter->rVerInfo.u2Part1CfgPeerVersion || CFG_DRV_OWN_VERSION < prAdapter->rVerInfo.u2Part2CfgPeerVersion || prAdapter->rVerInfo.u2Part1CfgOwnVersion < CFG_DRV_PEER_VERSION || prAdapter->rVerInfo.u2Part2CfgOwnVersion < CFG_DRV_PEER_VERSION	/* NVRAM */
		     || CFG_DRV_OWN_VERSION <
		     prAdapter->rVerInfo.u2FwPeerVersion
		     || prAdapter->rVerInfo.u2FwOwnVersion <
		     CFG_DRV_PEER_VERSION)) {
			u4ErrCode |= NVRAM_ERROR_VERSION_MISMATCH;
		}

		if (prRegInfo->ucTxPwrValid == 0) {
			u4ErrCode |= NVRAM_ERROR_INVALID_TXPWR;
		}

		if (prAdapter->fgIsEmbbededMacAddrValid == FALSE &&
		    (IS_BMCAST_MAC_ADDR(prRegInfo->aucMacAddr) ||
		     EQUAL_MAC_ADDR(aucZeroMacAddr, prRegInfo->aucMacAddr))) {
			u4ErrCode |= NVRAM_ERROR_INVALID_MAC_ADDR;
		}

		if (u4ErrCode != 0) {
			sprintf(aucErrMsg, VER_ERR_MSG,
				(unsigned int)u4ErrCode);
			COPY_SSID(prSsid->aucSSID, prSsid->ucLength, aucErrMsg,
				  strlen(aucErrMsg));

			kalIndicateBssInfo(prAdapter->prGlueInfo,
					   (PUINT_8) prBeacon,
					   OFFSET_OF(WLAN_BEACON_FRAME_T,
						     aucInfoElem) +
					   OFFSET_OF(IE_SSID_T, aucSSID)
					   + prSsid->ucLength, 1, 0);

			COPY_SSID(rSsid.aucSsid,
				  rSsid.u4SsidLen,
				  NVRAM_ERR_MSG, strlen(NVRAM_ERR_MSG));
			nicAddScanResult(prAdapter,
					 prBeacon->aucBSSID,
					 &rSsid,
					 0,
					 0,
					 PARAM_NETWORK_TYPE_FH,
					 &rConfiguration,
					 NET_TYPE_INFRA,
					 rSupportedRates,
					 OFFSET_OF(WLAN_BEACON_FRAME_T,
						   aucInfoElem) +
					 OFFSET_OF(IE_SSID_T, aucSSID)
					 + prSsid->ucLength -
					 WLAN_MAC_MGMT_HEADER_LEN,
					 (PUINT_8) ((UINT_32) (prBeacon) +
						    WLAN_MAC_MGMT_HEADER_LEN));
		}
	}
#endif

	if (fgGenErrMsg == TRUE) {
		cnmMemFree(prAdapter, prBeacon);
	}

	return WLAN_STATUS_SUCCESS;
}
