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
#include "gl_os.h"
#include "gl_wext.h"
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
#if DBG
int allocatedMemSize = 0;
#endif

extern struct semaphore g_halt_sem;
extern int g_u4HaltFlag;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
static PVOID pvIoBuffer = NULL;
static UINT_32 pvIoBufferSize = 0;
static UINT_32 pvIoBufferUsage = 0;

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
#if defined(MT6620) && CFG_MULTI_ECOVER_SUPPORT
typedef enum _ENUM_WMTHWVER_TYPE_T {
	WMTHWVER_MT6620_E1 = 0x0,
	WMTHWVER_MT6620_E2 = 0x1,
	WMTHWVER_MT6620_E3 = 0x2,
	WMTHWVER_MT6620_E4 = 0x3,
	WMTHWVER_MT6620_E5 = 0x4,
	WMTHWVER_MT6620_E6 = 0x5,
	WMTHWVER_MT6620_MAX,
	WMTHWVER_INVALID = 0xff
} ENUM_WMTHWVER_TYPE_T, *P_ENUM_WMTHWVER_TYPE_T;

extern ENUM_WMTHWVER_TYPE_T mtk_wcn_wmt_hwver_get(VOID);
#elif defined(MT5931) && CFG_MULTI_ECOVER_SUPPORT
typedef enum _ENUM_HWVER_TYPE_T {
	HWVER_MT5931_E1 = 0x1,
	HWVER_MT5931_E2 = 0x2,
	HWVER_MT5931_E3 = 0x3,
	HWVER_MT5931_MAX,
	HWVER_INVALID = 0xff
} ENUM_HWVER_TYPE_T, *P_ENUM_HWVER_TYPE_T;
#endif

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#if CFG_ENABLE_FW_DOWNLOAD

static struct file *filp = NULL;
static uid_t orgfsuid;
static gid_t orgfsgid;
static mm_segment_t orgfs;

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to
*        open firmware image in kernel space
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
*
* \retval WLAN_STATUS_SUCCESS.
* \retval WLAN_STATUS_FAILURE.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS kalFirmwareOpen(IN P_GLUE_INFO_T prGlueInfo)
{
/* FIX ME: since we don't have hotplug script in the filesystem
* , so the request_firmware() KAPI can not work properly
*/

/* save uid and gid used for filesystem access.
* set user and group to 0(root) */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
	orgfsuid = current->fsuid;
	orgfsgid = current->fsgid;
	current->fsuid = current->fsgid = 0;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	struct cred *cred = (struct cred *)get_current_cred();
	orgfsuid = cred->fsuid;
	orgfsgid = cred->fsgid;
	cred->fsuid = cred->fsgid = 0;
#else
	struct cred *cred = get_task_cred(current);
	orgfsuid = cred->fsuid;
	orgfsgid = cred->fsgid;
	cred->fsuid = cred->fsgid = 0;
#endif

	ASSERT(prGlueInfo);

	orgfs = get_fs();
	set_fs(get_ds());

/* open the fw file */
#if defined(MT6620) & CFG_MULTI_ECOVER_SUPPORT
	switch (mtk_wcn_wmt_hwver_get()) {
	case WMTHWVER_MT6620_E1:
	case WMTHWVER_MT6620_E2:
	case WMTHWVER_MT6620_E3:
	case WMTHWVER_MT6620_E4:
	case WMTHWVER_MT6620_E5:
		filp = filp_open("/etc/firmware/" CFG_FW_FILENAME, O_RDONLY, 0);
		break;

	case WMTHWVER_MT6620_E6:
	default:
		filp = filp_open("/etc/firmware/" CFG_FW_FILENAME "_E6",
				 O_RDONLY, 0);
		break;
	}
#elif defined(MT5931) && CFG_MULTI_ECOVER_SUPPORT
	switch (wlanGetEcoVersion(prGlueInfo->prAdapter)) {
	case HWVER_MT5931_E1:
	case HWVER_MT5931_E2:
		filp = filp_open("/etc/firmware/" CFG_FW_FILENAME "_E2",
				 O_RDONLY, 0);
		break;
	case HWVER_MT5931_E3:
	default:
		filp = filp_open("/etc/firmware/" CFG_FW_FILENAME, O_RDONLY, 0);
		break;
	}
#elif defined(MT6628)
	filp =
	    filp_open("/etc/firmware/" CFG_FW_FILENAME "_MT6628", O_RDONLY, 0);
#else
	filp = filp_open("/etc/firmware/" CFG_FW_FILENAME, O_RDONLY, 0);
#endif
	if (IS_ERR(filp)) {
		DBGLOG(INIT, INFO,
		       ("Open FW image: %s failed\n", CFG_FW_FILENAME));
		goto error_open;
	}
	DBGLOG(INIT, INFO, ("Open FW image: %s done\n", CFG_FW_FILENAME));
	return WLAN_STATUS_SUCCESS;

error_open:
/* restore */
	set_fs(orgfs);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
	current->fsuid = orgfsuid;
	current->fsgid = orgfsgid;
#else
	cred->fsuid = orgfsuid;
	cred->fsgid = orgfsgid;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	put_cred(cred);
#endif
#endif
	return WLAN_STATUS_FAILURE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to
*        release firmware image in kernel space
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
*
* \retval WLAN_STATUS_SUCCESS.
* \retval WLAN_STATUS_FAILURE.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS kalFirmwareClose(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	if ((filp != NULL) && !IS_ERR(filp)) {
/* close firmware file */
		filp_close(filp, NULL);

/* restore */
		set_fs(orgfs);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
		current->fsuid = orgfsuid;
		current->fsgid = orgfsgid;
#else
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
			struct cred *cred = (struct cred *)get_current_cred();
#else
			struct cred *cred = get_task_cred(current);
#endif
			cred->fsuid = orgfsuid;
			cred->fsgid = orgfsgid;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
			put_cred(cred);
#endif
		}
#endif
		filp = NULL;
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to
*        load firmware image in kernel space
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
*
* \retval WLAN_STATUS_SUCCESS.
* \retval WLAN_STATUS_FAILURE.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalFirmwareLoad(IN P_GLUE_INFO_T prGlueInfo,
		OUT PVOID prBuf, IN UINT_32 u4Offset, OUT PUINT_32 pu4Size)
{
	ASSERT(prGlueInfo);
	ASSERT(pu4Size);
	ASSERT(prBuf);

/* l = filp->f_path.dentry->d_inode->i_size; */

/* the object must have a read method */
	if ((filp == NULL) || IS_ERR(filp) || (filp->f_op == NULL) ||
	    (filp->f_op->read == NULL)) {
		goto error_read;
	} else {
		filp->f_pos = u4Offset;
		*pu4Size =
		    filp->f_op->read(filp, prBuf, *pu4Size, &filp->f_pos);
	}

	return WLAN_STATUS_SUCCESS;

error_read:
	return WLAN_STATUS_FAILURE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to
*        query firmware image size in kernel space
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
*
* \retval WLAN_STATUS_SUCCESS.
* \retval WLAN_STATUS_FAILURE.
*
*/
/*----------------------------------------------------------------------------*/

WLAN_STATUS kalFirmwareSize(IN P_GLUE_INFO_T prGlueInfo, OUT PUINT_32 pu4Size)
{
	ASSERT(prGlueInfo);
	ASSERT(pu4Size);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12)
	*pu4Size = filp->f_path.dentry->d_inode->i_size;
#else
	*pu4Size = filp->f_dentry->d_inode->i_size;
#endif

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to load firmware image
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
* \param ppvMapFileBuf  Pointer of pointer to memory-mapped firmware image
* \param pu4FileLength  File length and memory mapped length as well

* \retval Map File Handle, used for unammping
*/
/*----------------------------------------------------------------------------*/

PVOID
kalFirmwareImageMapping(IN P_GLUE_INFO_T prGlueInfo,
			OUT PPVOID ppvMapFileBuf, OUT PUINT_32 pu4FileLength)
{
	DEBUGFUNC("kalFirmwareImageMapping");

	ASSERT(prGlueInfo);
	ASSERT(ppvMapFileBuf);
	ASSERT(pu4FileLength);

	do {
/* <1> Open firmware */
		if (kalFirmwareOpen(prGlueInfo) != WLAN_STATUS_SUCCESS) {
			break;
		} else {
			UINT_32 u4FwSize = 0;
			PVOID prFwBuffer = NULL;
/* <2> Query firmare size */
			kalFirmwareSize(prGlueInfo, &u4FwSize);
/* <3> Use vmalloc for allocating large memory trunk */
			prFwBuffer = vmalloc(ALIGN_4(u4FwSize));
/* <4> Load image binary into buffer */
			if (kalFirmwareLoad(prGlueInfo, prFwBuffer, 0,
					    &u4FwSize) != WLAN_STATUS_SUCCESS) {
				vfree(prFwBuffer);
				kalFirmwareClose(prGlueInfo);
				break;
			}
/* <5> write back info */
			*pu4FileLength = u4FwSize;
			*ppvMapFileBuf = prFwBuffer;

			return prFwBuffer;
		}
	} while (FALSE);

	return NULL;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to unload firmware image mapped memory
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
* \param pvFwHandle     Pointer to mapping handle
* \param pvMapFileBuf   Pointer to memory-mapped firmware image
*
* \retval none
*/
/*----------------------------------------------------------------------------*/

VOID
kalFirmwareImageUnmapping(IN P_GLUE_INFO_T prGlueInfo,
			  IN PVOID prFwHandle, IN PVOID pvMapFileBuf)
{
	DEBUGFUNC("kalFirmwareImageUnmapping");

	ASSERT(prGlueInfo);

/* pvMapFileBuf might be NULL when file doesn't exist */
	if (pvMapFileBuf) {
		vfree(pvMapFileBuf);
	}

	kalFirmwareClose(prGlueInfo);
}

#endif

#if 0

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to load firmware image
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
* \param ppvMapFileBuf  Pointer of pointer to memory-mapped firmware image
* \param pu4FileLength  File length and memory mapped length as well

* \retval Map File Handle, used for unammping
*/
/*----------------------------------------------------------------------------*/

PVOID
kalFirmwareImageMapping(IN P_GLUE_INFO_T prGlueInfo,
			OUT PPVOID ppvMapFileBuf, OUT PUINT_32 pu4FileLength)
{
	INT_32 i4Ret = 0;

	DEBUGFUNC("kalFirmwareImageMapping");

	ASSERT(prGlueInfo);
	ASSERT(ppvMapFileBuf);
	ASSERT(pu4FileLength);

	do {
		GL_HIF_INFO_T *prHifInfo = &prGlueInfo->rHifInfo;
		prGlueInfo->prFw = NULL;

/* <1> Open firmware */
		i4Ret = request_firmware(&prGlueInfo->prFw,
					 CFG_FW_FILENAME,
					 &prHifInfo->func->dev);

		if (i4Ret) {
			printk(KERN_INFO DRV_NAME "fw %s:request failed %d\n",
			       CFG_FW_FILENAME, i4Ret);
			break;
		} else {
			*pu4FileLength = prGlueInfo->prFw->size;
			*ppvMapFileBuf = prGlueInfo->prFw->data;
			return prGlueInfo->prFw->data;
		}
	} while (FALSE);

	return NULL;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to unload firmware image mapped memory
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
* \param pvFwHandle     Pointer to mapping handle
* \param pvMapFileBuf   Pointer to memory-mapped firmware image
*
* \retval none
*/
/*----------------------------------------------------------------------------*/

VOID
kalFirmwareImageUnmapping(IN P_GLUE_INFO_T prGlueInfo,
			  IN PVOID prFwHandle, IN PVOID pvMapFileBuf)
{
	DEBUGFUNC("kalFirmwareImageUnmapping");

	ASSERT(prGlueInfo);
	ASSERT(pvMapFileBuf);

	release_firmware(prGlueInfo->prFw);
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to acquire
*        OS SPIN_LOCK.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] rLockCategory  Specify which SPIN_LOCK
* \param[out] pu4Flags      Pointer of a variable for saving IRQ flags
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID
kalAcquireSpinLock(IN P_GLUE_INFO_T prGlueInfo,
		   IN ENUM_SPIN_LOCK_CATEGORY_E rLockCategory,
		   OUT PUINT_32 pu4Flags)
{
	UINT_32 u4Flags = 0;

	ASSERT(prGlueInfo);
	ASSERT(pu4Flags);

	if (rLockCategory < SPIN_LOCK_NUM) {
#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
		spin_lock_bh(&prGlueInfo->rSpinLock[rLockCategory]);
#else /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
		spin_lock_irqsave(&prGlueInfo->rSpinLock[rLockCategory],
				  u4Flags);
#endif /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */

		*pu4Flags = u4Flags;
	}

	return;
}				/* end of kalAcquireSpinLock() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to release
*        OS SPIN_LOCK.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] rLockCategory  Specify which SPIN_LOCK
* \param[in] u4Flags        Saved IRQ flags
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID
kalReleaseSpinLock(IN P_GLUE_INFO_T prGlueInfo,
		   IN ENUM_SPIN_LOCK_CATEGORY_E rLockCategory,
		   IN UINT_32 u4Flags)
{
	ASSERT(prGlueInfo);

	if (rLockCategory < SPIN_LOCK_NUM) {
#if CFG_USE_SPIN_LOCK_BOTTOM_HALF
		spin_unlock_bh(&prGlueInfo->rSpinLock[rLockCategory]);
#else /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
		spin_unlock_irqrestore(&prGlueInfo->rSpinLock[rLockCategory],
				       u4Flags);
#endif /* !CFG_USE_SPIN_LOCK_BOTTOM_HALF */
	}

	return;
}				/* end of kalReleaseSpinLock() */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is provided by GLUE Layer for internal driver stack to update
*        current MAC address.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] pucMacAddr     Pointer of current MAC address
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID kalUpdateMACAddress(IN P_GLUE_INFO_T prGlueInfo, IN PUINT_8 pucMacAddr)
{
	ASSERT(prGlueInfo);
	ASSERT(pucMacAddr);

	if (UNEQUAL_MAC_ADDR(prGlueInfo->prDevHandler->dev_addr, pucMacAddr)) {
		memcpy(prGlueInfo->prDevHandler->dev_addr,
		       pucMacAddr, PARAM_MAC_ADDR_LEN);
	}

	return;
}

#if CFG_TCP_IP_CHKSUM_OFFLOAD
/*----------------------------------------------------------------------------*/
/*!
* \brief To query the packet information for offload related parameters.
*
* \param[in] pvPacket Pointer to the packet descriptor.
* \param[in] pucFlag  Points to the offload related parameter.
*
* \return (none)
*
*/
/*----------------------------------------------------------------------------*/
VOID kalQueryTxChksumOffloadParam(IN PVOID pvPacket, OUT PUINT_8 pucFlag)
{
	struct sk_buff *skb = (struct sk_buff *)pvPacket;
	UINT_8 ucFlag = 0;

	ASSERT(pvPacket);
	ASSERT(pucFlag);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 19)
	if (skb->ip_summed == CHECKSUM_HW)
#else
	if (skb->ip_summed == CHECKSUM_PARTIAL)
#endif
	{
#if DBG
/* Kevin: do double check, we can remove this part in Normal Driver.
* Because we register NIC feature with NETIF_F_IP_CSUM for MT5912B MAC, so
* we'll process IP packet only.
*/
		if (skb->protocol != __constant_htons(ETH_P_IP)) {
/* printk("Wrong skb->protocol( = %08x) for TX Checksum Offload.\n", skb->protocol); */
		} else
#endif
			ucFlag |= (TX_CS_IP_GEN | TX_CS_TCP_UDP_GEN);
	}

	*pucFlag = ucFlag;

	return;
}				/* kalQueryChksumOffloadParam */

/* 4 this is rewritten by Mike */
/*----------------------------------------------------------------------------*/
/*!
* \brief To update the checksum offload status to the packet to be indicated to OS.
*
* \param[in] pvPacket Pointer to the packet descriptor.
* \param[in] pucFlag  Points to the offload related parameter.
*
* \return (none)
*
*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateRxCSUMOffloadParam(IN PVOID pvPacket, IN ENUM_CSUM_RESULT_T aeCSUM[]
    )
{
	struct sk_buff *skb = (struct sk_buff *)pvPacket;

	ASSERT(pvPacket);

	if ((aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_SUCCESS ||
	     aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_SUCCESS) &&
	    ((aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_SUCCESS) ||
	     (aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_SUCCESS))) {
		skb->ip_summed = CHECKSUM_UNNECESSARY;
	} else {
		skb->ip_summed = CHECKSUM_NONE;
#if DBG
		if (aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_NONE &&
		    aeCSUM[CSUM_TYPE_IPV6] == CSUM_RES_NONE) {
			DBGLOG(RX, TRACE, ("RX: \"non-IPv4/IPv6\" Packet\n"));
		} else if (aeCSUM[CSUM_TYPE_IPV4] == CSUM_RES_FAILED) {
			DBGLOG(RX, TRACE, ("RX: \"bad IP Checksum\" Packet\n"));
		} else if (aeCSUM[CSUM_TYPE_TCP] == CSUM_RES_FAILED) {
			DBGLOG(RX, TRACE,
			       ("RX: \"bad TCP Checksum\" Packet\n"));
		} else if (aeCSUM[CSUM_TYPE_UDP] == CSUM_RES_FAILED) {
			DBGLOG(RX, TRACE,
			       ("RX: \"bad UDP Checksum\" Packet\n"));
		} else {
		}
#endif
	}
}				/* kalUpdateRxCSUMOffloadParam */
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is called to free packet allocated from kalPacketAlloc.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] pvPacket       Pointer of the packet descriptor
*
* \return (none)
*/
/*----------------------------------------------------------------------------*/
VOID kalPacketFree(IN P_GLUE_INFO_T prGlueInfo, IN PVOID pvPacket)
{
	dev_kfree_skb((struct sk_buff *)pvPacket);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Only handles driver own creating packet (coalescing buffer).
*
* \param prGlueInfo   Pointer of GLUE Data Structure
* \param u4Size       Pointer of Packet Handle
* \param ppucData     Status Code for OS upper layer
*
* \return NULL: Failed to allocate skb, Not NULL get skb
*/
/*----------------------------------------------------------------------------*/
PVOID
kalPacketAlloc(IN P_GLUE_INFO_T prGlueInfo,
	       IN UINT_32 u4Size, OUT PUINT_8 * ppucData)
{
	struct sk_buff *prSkb = dev_alloc_skb(u4Size);

	if (prSkb) {
		*ppucData = (PUINT_8) (prSkb->data);
	}
#if DBG
	{
		PUINT_32 pu4Head = (PUINT_32) & prSkb->cb[0];
		*pu4Head = (UINT_32) prSkb->head;
		DBGLOG(RX, TRACE,
		       ("prSkb->head = %#lx, prSkb->cb = %#lx\n",
			(UINT_32) prSkb->head, *pu4Head));
	}
#endif
	return (PVOID) prSkb;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Process the received packet for indicating to OS.
*
* \param[in] prGlueInfo     Pointer to the Adapter structure.
* \param[in] pvPacket       Pointer of the packet descriptor
* \param[in] pucPacketStart The starting address of the buffer of Rx packet.
* \param[in] u4PacketLen    The packet length.
* \param[in] pfgIsRetain    Is the packet to be retained.
* \param[in] aerCSUM        The result of TCP/ IP checksum offload.
*
* \retval WLAN_STATUS_SUCCESS.
* \retval WLAN_STATUS_FAILURE.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalProcessRxPacket(IN P_GLUE_INFO_T prGlueInfo,
		   IN PVOID pvPacket,
		   IN PUINT_8 pucPacketStart, IN UINT_32 u4PacketLen,
/* IN PBOOLEAN           pfgIsRetain, */
		   IN BOOLEAN fgIsRetain, IN ENUM_CSUM_RESULT_T aerCSUM[]
    )
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	struct sk_buff *skb = (struct sk_buff *)pvPacket;

	skb->data = (unsigned char *)pucPacketStart;
	skb->tail = (unsigned char *)((UINT_32) (pucPacketStart + u4PacketLen));
	skb->len = (unsigned int)u4PacketLen;

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	kalUpdateRxCSUMOffloadParam(skb, aerCSUM);
#endif

	return rStatus;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief To indicate an array of received packets is available for higher
*        level protocol uses.
*
* \param[in] prGlueInfo Pointer to the Adapter structure.
* \param[in] apvPkts The packet array to be indicated
* \param[in] ucPktNum The number of packets to be indicated
*
* \retval TRUE Success.
*
*/
/*----------------------------------------------------------------------------*/
WLAN_STATUS
kalRxIndicatePkts(IN P_GLUE_INFO_T prGlueInfo,
		  IN PVOID apvPkts[], IN UINT_8 ucPktNum)
{
	UINT_8 ucIdx = 0;
	struct net_device *prNetDev = prGlueInfo->prDevHandler;
	struct sk_buff *prSkb = NULL;

	ASSERT(prGlueInfo);
	ASSERT(apvPkts);

#if CFG_BOW_TEST
	UINT_32 i;
#endif

	for (ucIdx = 0; ucIdx < ucPktNum; ucIdx++) {
		prSkb = apvPkts[ucIdx];
#if DBG
		do {
			PUINT_8 pu4Head = (PUINT_8) & prSkb->cb[0];
			UINT_32 u4HeadValue = 0;
			kalMemCopy(&u4HeadValue, pu4Head, sizeof(u4HeadValue));
			DBGLOG(RX, TRACE,
			       ("prSkb->head = 0x%p, prSkb->cb = 0x%lx\n",
				pu4Head, u4HeadValue));
		} while (0);
#endif

		if (GLUE_GET_PKT_IS_P2P(prSkb)) {
/* P2P */
#if CFG_ENABLE_WIFI_DIRECT
			if (prGlueInfo->prAdapter->fgIsP2PRegistered) {
				prNetDev = kalP2PGetDevHdlr(prGlueInfo);
			}

/* prNetDev->stats.rx_bytes += prSkb->len; */
/* prNetDev->stats.rx_packets++; */
			prGlueInfo->prP2PInfo->rNetDevStats.rx_bytes +=
			    prSkb->len;
			prGlueInfo->prP2PInfo->rNetDevStats.rx_packets++;

#else
			prNetDev = prGlueInfo->prDevHandler;
#endif
		} else if (GLUE_GET_PKT_IS_PAL(prSkb)) {
/* BOW */
#if CFG_ENABLE_BT_OVER_WIFI && CFG_BOW_SEPARATE_DATA_PATH
			if (prGlueInfo->rBowInfo.fgIsNetRegistered) {
				prNetDev = prGlueInfo->rBowInfo.prDevHandler;
			}
#else
			prNetDev = prGlueInfo->prDevHandler;
#endif
		} else {
/* AIS */
			prNetDev = prGlueInfo->prDevHandler;
			prGlueInfo->rNetDevStats.rx_bytes += prSkb->len;
			prGlueInfo->rNetDevStats.rx_packets++;
		}

		prNetDev->last_rx = jiffies;
		prSkb->protocol = eth_type_trans(prSkb, prNetDev);
		prSkb->dev = prNetDev;
/* DBGLOG_MEM32(RX, TRACE, (PUINT_32)prSkb->data, prSkb->len); */
		DBGLOG(RX, EVENT, ("kalRxIndicatePkts len = %d\n", prSkb->len));

#if CFG_BOW_TEST
		DBGLOG(BOW, TRACE, ("Rx sk_buff->len: %d\n", prSkb->len));
		DBGLOG(BOW, TRACE,
		       ("Rx sk_buff->data_len: %d\n", prSkb->data_len));
		DBGLOG(BOW, TRACE, ("Rx sk_buff->data:\n"));

		for (i = 0; i < prSkb->len && i < 64; i++) {
			LOG_FUNC("%02x ", prSkb->data[i]);

			if ((i + 1) % 16 == 0) {
				LOG_FUNC("\n");
			}
		}

		LOG_FUNC("\n");
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
		if (!in_interrupt()) {
			netif_rx_ni(prSkb);	/* only in non-interrupt context */
		} else {
			netif_rx(prSkb);
		}
#else
		netif_rx(prSkb);
#endif

		wlanReturnPacket(prGlueInfo->prAdapter, NULL);
	}

	return WLAN_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Called by driver to indicate event to upper layer, for example, the wpa
*        supplicant or wireless tools.
*
* \param[in] pvAdapter Pointer to the adapter descriptor.
* \param[in] eStatus Indicated status.
* \param[in] pvBuf Indicated message buffer.
* \param[in] u4BufLen Indicated message buffer size.
*
* \return (none)
*
*/
/*----------------------------------------------------------------------------*/
VOID
kalIndicateStatusAndComplete(IN P_GLUE_INFO_T prGlueInfo,
			     IN WLAN_STATUS eStatus,
			     IN PVOID pvBuf, IN UINT_32 u4BufLen)
{
	UINT_32 bufLen;
	P_PARAM_STATUS_INDICATION_T pStatus =
	    (P_PARAM_STATUS_INDICATION_T) pvBuf;
	P_PARAM_AUTH_EVENT_T pAuth = (P_PARAM_AUTH_EVENT_T) pStatus;
	P_PARAM_PMKID_CANDIDATE_LIST_T pPmkid =
	    (P_PARAM_PMKID_CANDIDATE_LIST_T) (pStatus + 1);
	PARAM_MAC_ADDRESS arBssid;
	struct cfg80211_scan_request *prScanRequest = NULL;
	PARAM_SSID_T ssid;
	struct ieee80211_channel *prChannel = NULL;
	struct cfg80211_bss *bss;
	UINT_8 ucChannelNum;
	P_BSS_DESC_T prBssDesc = NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	kalMemZero(arBssid, MAC_ADDR_LEN);

	ASSERT(prGlueInfo);

	switch (eStatus) {
	case WLAN_STATUS_ROAM_OUT_FIND_BEST:
	case WLAN_STATUS_MEDIA_CONNECT:

		prGlueInfo->eParamMediaStateIndicated =
		    PARAM_MEDIA_STATE_CONNECTED;

/* indicate assoc event */
		wlanQueryInformation(prGlueInfo->prAdapter,
				     wlanoidQueryBssid,
				     &arBssid[0], sizeof(arBssid), &bufLen);
		wext_indicate_wext_event(prGlueInfo, SIOCGIWAP, arBssid,
					 bufLen);

/* switch netif on */
		netif_carrier_on(prGlueInfo->prDevHandler);

		do {
/* print message on console */
			wlanQueryInformation(prGlueInfo->prAdapter,
					     wlanoidQuerySsid,
					     &ssid, sizeof(ssid), &bufLen);

			ssid.aucSsid[(ssid.u4SsidLen >= PARAM_MAX_LEN_SSID) ?
				     (PARAM_MAX_LEN_SSID -
				      1) : ssid.u4SsidLen] = '\0';
			DBGLOG(INIT, INFO,
			       ("[wifi] %s netif_carrier_on [ssid:%s " MACSTR
				"]\n",
				prGlueInfo->prDevHandler->name,
				ssid.aucSsid, MAC2STR(arBssid)));
		} while (0);

		if (prGlueInfo->fgIsRegistered == TRUE) {
/* retrieve channel */
			ucChannelNum =
			    wlanGetChannelNumberByNetwork(prGlueInfo->prAdapter,
							  NETWORK_TYPE_AIS_INDEX);
			if (ucChannelNum <= 14) {
				prChannel =
				    ieee80211_get_channel(priv_to_wiphy
							  (prGlueInfo),
							  ieee80211_channel_to_frequency
							  (ucChannelNum,
							   IEEE80211_BAND_2GHZ));
			} else {
				prChannel =
				    ieee80211_get_channel(priv_to_wiphy
							  (prGlueInfo),
							  ieee80211_channel_to_frequency
							  (ucChannelNum,
							   IEEE80211_BAND_5GHZ));
			}

/* ensure BSS exists */
			bss =
			    cfg80211_get_bss(priv_to_wiphy(prGlueInfo),
					     prChannel, arBssid, ssid.aucSsid,
					     ssid.u4SsidLen,
					     WLAN_CAPABILITY_ESS,
					     WLAN_CAPABILITY_ESS);

			if (bss == NULL) {
/* create BSS on-the-fly */
				prBssDesc =
				    wlanGetTargetBssDescByNetwork(prGlueInfo->
								  prAdapter,
								  NETWORK_TYPE_AIS_INDEX);

				if (prBssDesc != NULL) {
					bss =
					    cfg80211_inform_bss(priv_to_wiphy
								(prGlueInfo),
								prChannel,
								arBssid, 0,
/* TSF */
								WLAN_CAPABILITY_ESS,
								prBssDesc->u2BeaconInterval,
/* beacon interval */
								prBssDesc->aucIEBuf,
/* IE */
								prBssDesc->u2IELength,
/* IE Length */
								RCPI_TO_dBm
								(prBssDesc->
								 ucRCPI) * 100,
/* MBM */
								GFP_KERNEL);
				}
			}

/* CFG80211 Indication */
			if (eStatus == WLAN_STATUS_MEDIA_CONNECT
			    && prGlueInfo->prDevHandler->
			    ieee80211_ptr->sme_state ==
			    CFG80211_SME_CONNECTING) {
				cfg80211_connect_result(prGlueInfo->
							prDevHandler, arBssid,
							prGlueInfo->aucReqIe,
							prGlueInfo->u4ReqIeLength,
							prGlueInfo->aucRspIe,
							prGlueInfo->u4RspIeLength,
							WLAN_STATUS_SUCCESS,
							GFP_KERNEL);
			} else if (eStatus == WLAN_STATUS_ROAM_OUT_FIND_BEST
				   && prGlueInfo->prDevHandler->
				   ieee80211_ptr->sme_state ==
				   CFG80211_SME_CONNECTED) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)

				cfg80211_roamed_bss(prGlueInfo->prDevHandler,
						    bss,
						    prGlueInfo->aucReqIe,
						    prGlueInfo->u4ReqIeLength,
						    prGlueInfo->aucRspIe,
						    prGlueInfo->u4RspIeLength,
						    GFP_KERNEL);
#else
				struct ieee80211_channel *prChannel = NULL;
				UINT_8 ucChannelNum =
				    wlanGetChannelNumberByNetwork(prGlueInfo->
								  prAdapter,
								  NETWORK_TYPE_AIS_INDEX);

				if (ucChannelNum <= 14) {
					prChannel =
					    ieee80211_get_channel(priv_to_wiphy
								  (prGlueInfo),
								  ieee80211_channel_to_frequency
								  (ucChannelNum,
								   IEEE80211_BAND_2GHZ));
				} else {
					prChannel =
					    ieee80211_get_channel(priv_to_wiphy
								  (prGlueInfo),
								  ieee80211_channel_to_frequency
								  (ucChannelNum,
								   IEEE80211_BAND_5GHZ));
				}

				cfg80211_roamed(prGlueInfo->prDevHandler,
						prChannel,
						arBssid,
						prGlueInfo->aucReqIe,
						prGlueInfo->u4ReqIeLength,
						prGlueInfo->aucRspIe,
						prGlueInfo->u4RspIeLength,
						GFP_KERNEL);
#endif
			}
		}
		break;

	case WLAN_STATUS_MEDIA_DISCONNECT:
/* indicate disassoc event */
		wext_indicate_wext_event(prGlueInfo, SIOCGIWAP, NULL, 0);
/* For CR 90 and CR99, While supplicant do reassociate, driver will do netif_carrier_off first,
after associated success, at joinComplete(), do netif_carier_on,
but for unknown reason, the supplicant 1x pkt will not called the driver
hardStartXmit, for template workaround these bugs, add this compiling flag
*/
/* switch netif off */

#if 1				/* CONSOLE_MESSAGE */
		DBGLOG(INIT, INFO,
		       ("[wifi] %s netif_carrier_off\n",
			prGlueInfo->prDevHandler->name));
#endif

		netif_carrier_off(prGlueInfo->prDevHandler);

		if (prGlueInfo->fgIsRegistered == TRUE
		    && prGlueInfo->prDevHandler->ieee80211_ptr->sme_state ==
		    CFG80211_SME_CONNECTED) {
/* CFG80211 Indication */
			cfg80211_disconnected(prGlueInfo->prDevHandler,
					      0, NULL, 0, GFP_KERNEL);
		}

		prGlueInfo->eParamMediaStateIndicated =
		    PARAM_MEDIA_STATE_DISCONNECTED;

		break;

	case WLAN_STATUS_SCAN_COMPLETE:
/* indicate scan complete event */
		wext_indicate_wext_event(prGlueInfo, SIOCGIWSCAN, NULL, 0);

/* 1. reset first for newly incoming request */
		GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);
		if (prGlueInfo->prScanRequest != NULL) {
			prScanRequest = prGlueInfo->prScanRequest;
			prGlueInfo->prScanRequest = NULL;
		}
		GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_NET_DEV);

/* 2. then CFG80211 Indication */
		if (prScanRequest != NULL) {
			cfg80211_scan_done(prScanRequest, FALSE);
		}
		break;
	case WLAN_STATUS_CONNECT_INDICATION:
/* indicate AIS Jion fail  event */
		if (prGlueInfo->prDevHandler->ieee80211_ptr->sme_state ==
		    CFG80211_SME_CONNECTING) {
			cfg80211_connect_result(prGlueInfo->prDevHandler,
						prGlueInfo->prAdapter->rWifiVar.
						rAisFsmInfo.prTargetBssDesc->
						aucBSSID, prGlueInfo->aucReqIe,
						prGlueInfo->u4ReqIeLength,
						prGlueInfo->aucRspIe,
						prGlueInfo->u4RspIeLength,
						REASON_CODE_UNSPECIFIED,
						GFP_KERNEL);
		}
		break;

#if 0
	case WLAN_STATUS_MSDU_OK:
		if (netif_running(prGlueInfo->prDevHandler)) {
			netif_wake_queue(prGlueInfo->prDevHandler);
		}
		break;
#endif

	case WLAN_STATUS_MEDIA_SPECIFIC_INDICATION:
		if (pStatus) {
			switch (pStatus->eStatusType) {
			case ENUM_STATUS_TYPE_AUTHENTICATION:
/*
printk(KERN_NOTICE "ENUM_STATUS_TYPE_AUTHENTICATION: L(%ld) [" MACSTR "] F:%lx\n",
pAuth->Request[0].Length,
MAC2STR(pAuth->Request[0].Bssid),
pAuth->Request[0].Flags);
*/
/* indicate (UC/GC) MIC ERROR event only */
				if ((pAuth->arRequest[0].u4Flags ==
				     PARAM_AUTH_REQUEST_PAIRWISE_ERROR) ||
				    (pAuth->arRequest[0].u4Flags ==
				     PARAM_AUTH_REQUEST_GROUP_ERROR)) {
					cfg80211_michael_mic_failure
					    (prGlueInfo->prDevHandler, NULL,
					     (pAuth->arRequest[0].u4Flags ==
					      PARAM_AUTH_REQUEST_PAIRWISE_ERROR)
					     ? NL80211_KEYTYPE_PAIRWISE :
					     NL80211_KEYTYPE_GROUP, 0, NULL,
					     GFP_KERNEL);
					wext_indicate_wext_event(prGlueInfo,
								 IWEVMICHAELMICFAILURE,
								 (unsigned char
								  *)
								 &pAuth->arRequest
								 [0],
								 pAuth->arRequest
								 [0].u4Length);
				}
				break;

			case ENUM_STATUS_TYPE_CANDIDATE_LIST:
/*
printk(KERN_NOTICE "Param_StatusType_PMKID_CandidateList: Ver(%ld) Num(%ld)\n",
pPmkid->u2Version,
pPmkid->u4NumCandidates);
if (pPmkid->u4NumCandidates > 0) {
printk(KERN_NOTICE "candidate[" MACSTR "] preAuth Flag:%lx\n",
MAC2STR(pPmkid->arCandidateList[0].rBSSID),
pPmkid->arCandidateList[0].fgFlags);
}
*/
				{
					UINT_32 i = 0;

					P_PARAM_PMKID_CANDIDATE_T prPmkidCand =
					    (P_PARAM_PMKID_CANDIDATE_T) &
					    pPmkid->arCandidateList[0];

					for (i = 0; i < pPmkid->u4NumCandidates;
					     i++) {
						wext_indicate_wext_event
						    (prGlueInfo, IWEVPMKIDCAND,
						     (unsigned char *)
						     &pPmkid->arCandidateList
						     [i],
						     pPmkid->u4NumCandidates);
						prPmkidCand +=
						    sizeof
						    (PARAM_PMKID_CANDIDATE_T);
					}
				}
				break;

			default:
/* case ENUM_STATUS_TYPE_MEDIA_STREAM_MODE */
/*
printk(KERN_NOTICE "unknown media specific indication type:%x\n",
pStatus->StatusType);
*/
				break;
			}
		} else {
/*
printk(KERN_WARNING "media specific indication buffer NULL\n");
*/
		}
		break;

#if CFG_SUPPORT_BCM && CFG_SUPPORT_BCM_BWCS
	case WLAN_STATUS_BWCS_UPDATE:{
			wext_indicate_wext_event(prGlueInfo, IWEVCUSTOM, pvBuf,
						 sizeof(PTA_IPC_T));
		}

		break;

#endif

	default:
/*
printk(KERN_WARNING "unknown indication:%lx\n", eStatus);
*/
		break;
	}
}				/* kalIndicateStatusAndComplete */

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to update the (re)association request
*        information to the structure used to query and set
*        OID_802_11_ASSOCIATION_INFORMATION.
*
* \param[in] prGlueInfo Pointer to the Glue structure.
* \param[in] pucFrameBody Pointer to the frame body of the last (Re)Association
*                         Request frame from the AP.
* \param[in] u4FrameBodyLen The length of the frame body of the last
*                           (Re)Association Request frame.
* \param[in] fgReassocRequest TRUE, if it is a Reassociation Request frame.
*
* \return (none)
*
*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateReAssocReqInfo(IN P_GLUE_INFO_T prGlueInfo,
			IN PUINT_8 pucFrameBody,
			IN UINT_32 u4FrameBodyLen, IN BOOLEAN fgReassocRequest)
{
	PUINT_8 cp;

	ASSERT(prGlueInfo);

/* reset */
	prGlueInfo->u4ReqIeLength = 0;

	if (fgReassocRequest) {
		if (u4FrameBodyLen < 15) {
/*
printk(KERN_WARNING "frameBodyLen too short:%ld\n", frameBodyLen);
*/
			return;
		}
	} else {
		if (u4FrameBodyLen < 9) {
/*
printk(KERN_WARNING "frameBodyLen too short:%ld\n", frameBodyLen);
*/
			return;
		}
	}

	cp = pucFrameBody;

	if (fgReassocRequest) {
/* Capability information field 2 */
/* Listen interval field 2*/
/* Current AP address 6 */
		cp += 10;
		u4FrameBodyLen -= 10;
	} else {
/* Capability information field 2 */
/* Listen interval field 2*/
		cp += 4;
		u4FrameBodyLen -= 4;
	}

	wext_indicate_wext_event(prGlueInfo, IWEVASSOCREQIE, cp,
				 u4FrameBodyLen);

	if (u4FrameBodyLen <= CFG_CFG80211_IE_BUF_LEN) {
		prGlueInfo->u4ReqIeLength = u4FrameBodyLen;
		kalMemCopy(prGlueInfo->aucReqIe, cp, u4FrameBodyLen);
	}

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This routine is called to update the (re)association
*        response information to the structure used to reply with
*        cfg80211_connect_result
*
* @param prGlueInfo      Pointer to adapter descriptor
* @param pucFrameBody    Pointer to the frame body of the last (Re)Association
*                         Response frame from the AP
* @param u4FrameBodyLen  The length of the frame body of the last
*                          (Re)Association Response frame
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateReAssocRspInfo(IN P_GLUE_INFO_T prGlueInfo,
			IN PUINT_8 pucFrameBody, IN UINT_32 u4FrameBodyLen)
{
	UINT_32 u4IEOffset = 6;	/* cap_info, status_code & assoc_id */
	UINT_32 u4IELength = u4FrameBodyLen - u4IEOffset;

	ASSERT(prGlueInfo);

/* reset */
	prGlueInfo->u4RspIeLength = 0;

	if (u4IELength <= CFG_CFG80211_IE_BUF_LEN) {
		prGlueInfo->u4RspIeLength = u4IELength;
		kalMemCopy(prGlueInfo->aucRspIe,
			   pucFrameBody + u4IEOffset, u4IELength);
	}
}				/* kalUpdateReAssocRspInfo */

/*----------------------------------------------------------------------------*/
/*!
* \brief Notify OS with SendComplete event of the specific packet. Linux should
*        free packets here.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] pvPacket       Pointer of Packet Handle
* \param[in] status         Status Code for OS upper layer
*
* \return -
*/
/*----------------------------------------------------------------------------*/
VOID
kalSendCompleteAndAwakeQueue(IN P_GLUE_INFO_T prGlueInfo, IN PVOID pvPacket)
{
	struct net_device *prDev = NULL;
	struct sk_buff *prSkb = NULL;
	UINT_16 u2QueueIdx = 0;
	UINT_8 ucNetworkType = 0;
	BOOLEAN fgIsValidDevice = TRUE;

	ASSERT(pvPacket);
	ASSERT(prGlueInfo->i4TxPendingFrameNum);

	prSkb = (struct sk_buff *)pvPacket;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 26)
	u2QueueIdx = skb_get_queue_mapping(prSkb);
#endif
	ASSERT(u2QueueIdx < CFG_MAX_TXQ_NUM);

	if (GLUE_GET_PKT_IS_PAL(prSkb)) {
		ucNetworkType = NETWORK_TYPE_BOW_INDEX;
	} else if (GLUE_GET_PKT_IS_P2P(prSkb)) {
		ucNetworkType = NETWORK_TYPE_P2P_INDEX;

#if CFG_ENABLE_WIFI_DIRECT
/* in case packet was sent after P2P device is unregistered */
		if (prGlueInfo->prAdapter->fgIsP2PRegistered == FALSE) {
			fgIsValidDevice = FALSE;
		}
#endif
	} else {
		ucNetworkType = NETWORK_TYPE_AIS_INDEX;
	}

	GLUE_DEC_REF_CNT(prGlueInfo->i4TxPendingFrameNum);
	GLUE_DEC_REF_CNT(prGlueInfo->
			 ai4TxPendingFrameNumPerQueue[ucNetworkType]
			 [u2QueueIdx]);
	prDev = prSkb->dev;

	ASSERT(prDev);

	if (fgIsValidDevice == TRUE) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 26)
		if (netif_subqueue_stopped(prDev, prSkb) &&
		    prGlueInfo->
		    ai4TxPendingFrameNumPerQueue[ucNetworkType][u2QueueIdx] <=
		    CFG_TX_START_NETIF_PER_QUEUE_THRESHOLD) {
			netif_wake_subqueue(prDev, u2QueueIdx);
		}
#else
		if (prGlueInfo->i4TxPendingFrameNum <
		    CFG_TX_STOP_NETIF_QUEUE_THRESHOLD) {
			netif_wake_queue(prGlueInfo->prDevHandler);
		}
#endif
	}

	dev_kfree_skb((struct sk_buff *)pvPacket);

	DBGLOG(TX, EVENT,
	       ("----- pending frame %d -----\n",
		prGlueInfo->i4TxPendingFrameNum));

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Copy Mac Address setting from registry. It's All Zeros in Linux.
*
* \param[in] prAdapter Pointer to the Adapter structure
*
* \param[out] paucMacAddr Pointer to the Mac Address buffer
*
* \retval WLAN_STATUS_SUCCESS
*
* \note
*/
/*----------------------------------------------------------------------------*/
VOID
kalQueryRegistryMacAddr(IN P_GLUE_INFO_T prGlueInfo, OUT PUINT_8 paucMacAddr)
{
	UINT_8 aucZeroMac[MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 }
	DEBUGFUNC("kalQueryRegistryMacAddr");

	ASSERT(prGlueInfo);
	ASSERT(paucMacAddr);

	kalMemCopy((PVOID) paucMacAddr, (PVOID) aucZeroMac, MAC_ADDR_LEN);

	return;
}				/* end of kalQueryRegistryMacAddr() */

#if CFG_SUPPORT_EXT_CONFIG
/*----------------------------------------------------------------------------*/
/*!
* \brief Read external configuration, ex. NVRAM or file
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
*
* \return none
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalReadExtCfg(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

/* External data is given from user space by ioctl or /proc, not read by
driver.
*/
	if (0 != prGlueInfo->u4ExtCfgLength) {
		DBGLOG(INIT, TRACE,
		       ("Read external configuration data -- OK\n"));
	} else {
		DBGLOG(INIT, TRACE,
		       ("Read external configuration data -- fail\n"));
	}

	return prGlueInfo->u4ExtCfgLength;
}
#endif

/*----------------------------------------------------------------------------*/
/*!
* @brief This inline function is to extract some packet information, including
*        user priority, packet length, destination address, 802.1x and BT over Wi-Fi
*        or not.
*
* @param prGlueInfo         Pointer to the glue structure
* @param prNdisPacket       Packet descriptor
* @param pucPriorityParam   User priority
* @param pu4PacketLen       Packet length
* @param pucEthDestAddr Destination address
* @param pfgIs1X            802.1x packet or not
* @param pfgIsPAL           BT over Wi-Fi packet or not
*
* @retval TRUE      Success to extract information
* @retval FALSE     Fail to extract correct information
*/
/*----------------------------------------------------------------------------*/

BOOL
kalQoSFrameClassifierAndPacketInfo(IN P_GLUE_INFO_T prGlueInfo,
				   IN P_NATIVE_PACKET prPacket,
				   OUT PUINT_8 pucPriorityParam,
				   OUT PUINT_32 pu4PacketLen,
				   OUT PUINT_8 pucEthDestAddr,
				   OUT PBOOLEAN pfgIs1X,
				   OUT PBOOLEAN pfgIsPAL,
				   OUT PUINT_8 pucNetworkType)
{
	UINT_32 u4PacketLen;

	UINT_8 ucUserPriority = USER_PRIORITY_DEFAULT;	/* Default */
	UINT_16 u2EtherTypeLen;
	struct sk_buff *prSkb = (struct sk_buff *)prPacket;
	PUINT_8 aucLookAheadBuf = NULL;

	DEBUGFUNC("kalQoSFrameClassifierAndPacketInfo");

	u4PacketLen = prSkb->len;

	if (u4PacketLen < ETH_HLEN) {
		DBGLOG(INIT, WARN,
		       ("Invalid Ether packet length: %d\n", u4PacketLen));
		return FALSE;
	}

	aucLookAheadBuf = prSkb->data;

	*pfgIs1X = FALSE;
	*pfgIsPAL = FALSE;
/* 4 <3> Obtain the User Priority for WMM */
	u2EtherTypeLen =
	    (aucLookAheadBuf[ETH_TYPE_LEN_OFFSET] <<
	     8) | (aucLookAheadBuf[ETH_TYPE_LEN_OFFSET + 1]);

	if ((u2EtherTypeLen == ETH_P_IP) && (u4PacketLen >= LOOK_AHEAD_LEN)) {
		PUINT_8 pucIpHdr = &aucLookAheadBuf[ETH_HLEN];
		UINT_8 ucIpVersion;

		ucIpVersion =
		    (pucIpHdr[0] & IPVH_VERSION_MASK) >> IPVH_VERSION_OFFSET;
/* printk ("ip version %x\n", ucIpVersion); */
		if (ucIpVersion == IPVERSION) {
			UINT_8 ucIpTos;
/* Get the DSCP value from the header of IP packet. */
			ucIpTos = pucIpHdr[1];
			ucUserPriority =
			    ((ucIpTos & IPTOS_PREC_MASK) >> IPTOS_PREC_OFFSET);
		}

/* TODO(Kevin): Add TSPEC classifier here */
	} else if (u2EtherTypeLen == ETH_P_1X) {	/* For Port Control */
/* DBGLOG(REQ, TRACE, ("Tx 1x\n")); */
		*pfgIs1X = TRUE;
	} else if (u2EtherTypeLen == ETH_P_PRE_1X) {	/* For Pre 1x pkt */
/* DBGLOG(REQ, TRACE, ("Tx Pre-1x\n")); */
		*pfgIs1X = TRUE;
	}
#if CFG_SUPPORT_WAPI
	else if (u2EtherTypeLen == ETH_WPI_1X) {
		*pfgIs1X = TRUE;
	}
#endif
	else if (u2EtherTypeLen <= 1500) {	/* 802.3 Frame */
		UINT_8 ucDSAP, ucSSAP, ucControl;
		UINT_8 aucOUI[3];

		ucDSAP = *(PUINT_8) & aucLookAheadBuf[ETH_LLC_OFFSET];
		ucSSAP = *(PUINT_8) & aucLookAheadBuf[ETH_LLC_OFFSET + 1];
		ucControl = *(PUINT_8) & aucLookAheadBuf[ETH_LLC_OFFSET + 2];

		aucOUI[0] = *(PUINT_8) & aucLookAheadBuf[ETH_SNAP_OFFSET];
		aucOUI[1] = *(PUINT_8) & aucLookAheadBuf[ETH_SNAP_OFFSET + 1];
		aucOUI[2] = *(PUINT_8) & aucLookAheadBuf[ETH_SNAP_OFFSET + 2];

		if (ucDSAP == ETH_LLC_DSAP_SNAP &&
		    ucSSAP == ETH_LLC_SSAP_SNAP &&
		    ucControl == ETH_LLC_CONTROL_UNNUMBERED_INFORMATION &&
		    aucOUI[0] == ETH_SNAP_BT_SIG_OUI_0 &&
		    aucOUI[1] == ETH_SNAP_BT_SIG_OUI_1 &&
		    aucOUI[2] == ETH_SNAP_BT_SIG_OUI_2) {
			UINT_16 tmp =
			    ((aucLookAheadBuf[ETH_SNAP_OFFSET +
					      3] <<
			      8) | aucLookAheadBuf[ETH_SNAP_OFFSET + 4]);

			*pfgIsPAL = TRUE;
			ucUserPriority = (UINT_8) prSkb->priority;

			if (tmp == BOW_PROTOCOL_ID_SECURITY_FRAME) {
				*pfgIs1X = TRUE;
			}
		}
	}

/* 4 <4> Return the value of Priority Parameter. */
	*pucPriorityParam = ucUserPriority;

/* 4 <5> Retrieve Packet Information - DA */
/* Packet Length/ Destination Address */
	*pu4PacketLen = u4PacketLen;

	kalMemCopy(pucEthDestAddr, aucLookAheadBuf, PARAM_MAC_ADDR_LEN);

/* <6> Network type */
#if CFG_ENABLE_BT_OVER_WIFI
	if (*pfgIsPAL == TRUE) {
		*pucNetworkType = NETWORK_TYPE_BOW_INDEX;
	} else
#endif
	{
#if CFG_ENABLE_WIFI_DIRECT
		if (prGlueInfo->prAdapter->fgIsP2PRegistered &&
		    GLUE_GET_PKT_IS_P2P(prPacket)) {
			*pucNetworkType = NETWORK_TYPE_P2P_INDEX;
		} else
#endif
		{
			*pucNetworkType = NETWORK_TYPE_AIS_INDEX;
		}
	}
	return TRUE;
}				/* end of kalQoSFrameClassifier() */

VOID
kalOidComplete(IN P_GLUE_INFO_T prGlueInfo,
	       IN BOOLEAN fgSetQuery,
	       IN UINT_32 u4SetQueryInfoLen, IN WLAN_STATUS rOidStatus)
{
	ASSERT(prGlueInfo);
/* remove timeout check timer */
	wlanoidClearTimeoutCheck(prGlueInfo->prAdapter);

/* if (prGlueInfo->u4TimeoutFlag != 1) { */
	prGlueInfo->rPendStatus = rOidStatus;
	complete(&prGlueInfo->rPendComp);
	prGlueInfo->u4OidCompleteFlag = 1;
/* } */
/* else let it timeout on kalIoctl entry */
}

VOID kalOidClearance(IN P_GLUE_INFO_T prGlueInfo)
{
/* if (prGlueInfo->u4TimeoutFlag != 1) { */
/* clear_bit(GLUE_FLAG_OID_BIT, &prGlueInfo->u4Flag); */
	if (prGlueInfo->u4OidCompleteFlag != 1) {
		complete(&prGlueInfo->rPendComp);
	}
/* } */
}

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is used to transfer linux ioctl to OID, and  we
* need to specify the behavior of the OID by ourself
*
* @param prGlueInfo         Pointer to the glue structure
* @param pvInfoBuf          Data buffer
* @param u4InfoBufLen       Data buffer length
* @param fgRead             Is this a read OID
* @param fgWaitResp         does this OID need to wait for values
* @param fgCmd              does this OID compose command packet
* @param pu4QryInfoLen      The data length of the return values
*
* @retval TRUE      Success to extract information
* @retval FALSE     Fail to extract correct information
*/
/*----------------------------------------------------------------------------*/

/* todo: enqueue the i/o requests for multiple processes access */
/*  */
/* currently, return -1 */
/*  */

/* static GL_IO_REQ_T OidEntry; */

WLAN_STATUS
kalIoctl(IN P_GLUE_INFO_T prGlueInfo,
	 IN PFN_OID_HANDLER_FUNC pfnOidHandler,
	 IN PVOID pvInfoBuf,
	 IN UINT_32 u4InfoBufLen,
	 IN BOOL fgRead,
	 IN BOOL fgWaitResp,
	 IN BOOL fgCmd, IN BOOL fgIsP2pOid, OUT PUINT_32 pu4QryInfoLen)
{
	P_GL_IO_REQ_T prIoReq = NULL;
	WLAN_STATUS ret = WLAN_STATUS_SUCCESS;

/* GLUE_SPIN_LOCK_DECLARATION(); */
	ASSERT(prGlueInfo);

/* <1> Check if driver is halt */

/* if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) { */
/*    return WLAN_STATUS_ADAPTER_NOT_READY; */
/* } */

	if (down_interruptible(&g_halt_sem)) {
		return WLAN_STATUS_FAILURE;
	}

	if (g_u4HaltFlag) {
		up(&g_halt_sem);
		return WLAN_STATUS_ADAPTER_NOT_READY;
	}

	if (down_interruptible(&prGlueInfo->ioctl_sem)) {
		up(&g_halt_sem);
		return WLAN_STATUS_FAILURE;
	}

/* <2> TODO: thread-safe */

/* <3> point to the OidEntry of Glue layer */

	prIoReq = &(prGlueInfo->OidEntry);

	ASSERT(prIoReq);

/* <4> Compose the I/O request */
	prIoReq->prAdapter = prGlueInfo->prAdapter;
	prIoReq->pfnOidHandler = pfnOidHandler;
	prIoReq->pvInfoBuf = pvInfoBuf;
	prIoReq->u4InfoBufLen = u4InfoBufLen;
	prIoReq->pu4QryInfoLen = pu4QryInfoLen;
	prIoReq->fgRead = fgRead;
	prIoReq->fgWaitResp = fgWaitResp;
	prIoReq->rStatus = WLAN_STATUS_FAILURE;
#if CFG_ENABLE_WIFI_DIRECT
	prIoReq->fgIsP2pOid = fgIsP2pOid;
#endif

/* <5> Reset the status of pending OID */
	prGlueInfo->rPendStatus = WLAN_STATUS_FAILURE;
/* prGlueInfo->u4TimeoutFlag = 0; */
	prGlueInfo->u4OidCompleteFlag = 0;

/* <6> Check if we use the command queue */
	prIoReq->u4Flag = fgCmd;

/* <7> schedule the OID bit */
	set_bit(GLUE_FLAG_OID_BIT, &prGlueInfo->u4Flag);

/* <8> Wake up tx thread to handle kick start the I/O request */
	wake_up_interruptible(&prGlueInfo->waitq);

/* <9> Block and wait for event or timeout, current the timeout is 5 secs */
/* if (wait_for_completion_interruptible_timeout(&prGlueInfo->rPendComp, 5 * KAL_HZ)) { */
/* if (!wait_for_completion_interruptible(&prGlueInfo->rPendComp)) { */
	wait_for_completion(&prGlueInfo->rPendComp); {
/* Case 1: No timeout. */
/* if return WLAN_STATUS_PENDING, the status of cmd is stored in prGlueInfo  */
		if (prIoReq->rStatus == WLAN_STATUS_PENDING) {
			ret = prGlueInfo->rPendStatus;
		} else {
			ret = prIoReq->rStatus;
		}
	}
#if 0
	else {
/* Case 2: timeout */
/* clear pending OID's cmd in CMD queue */
		if (fgCmd) {
			prGlueInfo->u4TimeoutFlag = 1;
			wlanReleasePendingOid(prGlueInfo->prAdapter, 0);
		}
		ret = WLAN_STATUS_FAILURE;
	}
#endif

	up(&prGlueInfo->ioctl_sem);
	up(&g_halt_sem);

	return ret;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all pending security frames
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID kalClearSecurityFrames(IN P_GLUE_INFO_T prGlueInfo)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;

	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

/* Clear pending security frames in prGlueInfo->rCmdQueue */
	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_SECURITY_FRAME) {
			prCmdInfo->pfCmdTimeoutHandler(prGlueInfo->prAdapter,
						       prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear pending security frames
*        belongs to dedicated network type
*
* \param prGlueInfo         Pointer of GLUE Data Structure
* \param eNetworkTypeIdx    Network Type Index
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID
kalClearSecurityFramesByNetType(IN P_GLUE_INFO_T prGlueInfo,
				IN ENUM_NETWORK_TYPE_INDEX_T eNetworkTypeIdx)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;

	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

/* Clear pending security frames in prGlueInfo->rCmdQueue */
	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_SECURITY_FRAME &&
		    prCmdInfo->eNetworkType == eNetworkTypeIdx) {
			prCmdInfo->pfCmdTimeoutHandler(prGlueInfo->prAdapter,
						       prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all pending management frames
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID kalClearMgmtFrames(IN P_GLUE_INFO_T prGlueInfo)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

/* Clear pending management frames in prGlueInfo->rCmdQueue */
	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_MANAGEMENT_FRAME) {
			wlanReleaseCommand(prGlueInfo->prAdapter, prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear all pending management frames
*           belongs to dedicated network type
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID
kalClearMgmtFramesByNetType(IN P_GLUE_INFO_T prGlueInfo,
			    IN ENUM_NETWORK_TYPE_INDEX_T eNetworkTypeIdx)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

/* Clear pending management frames in prGlueInfo->rCmdQueue */
	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		prCmdInfo = (P_CMD_INFO_T) prQueueEntry;

		if (prCmdInfo->eCmdType == COMMAND_TYPE_MANAGEMENT_FRAME &&
		    prCmdInfo->eNetworkType == eNetworkTypeIdx) {
			wlanReleaseCommand(prGlueInfo->prAdapter, prCmdInfo);
			cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}				/* kalClearMgmtFramesByNetType */

/*----------------------------------------------------------------------------*/
/*!
* @brief This function is a kernel thread function for handling command packets
* Tx requests and interrupt events
*
* @param data       data pointer to private data of tx_thread
*
* @retval           If the function succeeds, the return value is 0.
* Otherwise, an error code is returned.
*
*/
/*----------------------------------------------------------------------------*/

int tx_thread(void *data)
{
	struct net_device *dev = data;
	P_GLUE_INFO_T prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(dev));

	P_QUE_ENTRY_T prQueueEntry = NULL;
	P_GL_IO_REQ_T prIoReq = NULL;
	P_QUE_T prTxQueue = NULL;
	P_QUE_T prCmdQue = NULL;

	int ret = 0;

	BOOLEAN fgNeedHwAccess = FALSE;

	struct sk_buff *prSkb = NULL;

/* for spin lock acquire and release */
	GLUE_SPIN_LOCK_DECLARATION();

	prTxQueue = &prGlueInfo->rTxQueue;
	prCmdQue = &prGlueInfo->rCmdQueue;

	current->flags |= PF_NOFREEZE;

	DBGLOG(INIT, INFO, ("tx_thread starts running... \n"));

	while (TRUE) {
#if CFG_ENABLE_WIFI_DIRECT
/*run p2p multicast list work. */
		if (test_and_clear_bit(GLUE_FLAG_SUB_MOD_MULTICAST_BIT,
				       &prGlueInfo->u4Flag)) {
			p2pSetMulticastListWorkQueueWrapper(prGlueInfo);
		}
#endif

		if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
			DBGLOG(INIT, INFO, ("tx_thread should stop now...\n"));
			break;
		}

/*
* sleep on waitqueue if no events occurred. Event contain (1) GLUE_FLAG_INT
* (2) GLUE_FLAG_OID (3) GLUE_FLAG_TXREQ (4) GLUE_FLAG_HALT
*
*/

		ret = wait_event_interruptible(prGlueInfo->waitq,
					       (prGlueInfo->u4Flag != 0));

#if CFG_DBG_GPIO_PINS
/* TX thread Wake up */
		mtk_wcn_stp_debug_gpio_assert(IDX_TX_THREAD, DBG_TIE_LOW);
#endif
#if CFG_ENABLE_WIFI_DIRECT
/*run p2p multicast list work. */
		if (test_and_clear_bit(GLUE_FLAG_SUB_MOD_MULTICAST_BIT,
				       &prGlueInfo->u4Flag)) {
			p2pSetMulticastListWorkQueueWrapper(prGlueInfo);
		}

		if (test_and_clear_bit(GLUE_FLAG_FRAME_FILTER_BIT,
				       &prGlueInfo->u4Flag)) {
			p2pFuncUpdateMgmtFrameRegister(prGlueInfo->prAdapter,
						       prGlueInfo->
						       prP2PInfo->u4OsMgmtFrameFilter);
		}

#endif
		if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
			DBGLOG(INIT, INFO,
			       ("<1>tx_thread should stop now...\n"));
			break;
		}

		fgNeedHwAccess = FALSE;

/* Handle Interrupt */
		if (test_and_clear_bit(GLUE_FLAG_INT_BIT, &prGlueInfo->u4Flag)) {
			if (fgNeedHwAccess == FALSE) {
				fgNeedHwAccess = TRUE;

				wlanAcquirePowerControl(prGlueInfo->prAdapter);
			}

/* the Wi-Fi interrupt is already disabled in mmc thread,
so we set the flag only to enable the interrupt later  */
			prGlueInfo->prAdapter->fgIsIntEnable = FALSE;
/* wlanISR(prGlueInfo->prAdapter, TRUE); */

			if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
/* Should stop now... skip pending interrupt */
				DBGLOG(INIT, INFO,
				       ("ignore pending interrupt\n"));
			} else {
				wlanIST(prGlueInfo->prAdapter);
			}
		}

/* transfer ioctl to OID request */
#if 0
		if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
			printk(KERN_INFO DRV_NAME
			       "<2>tx_thread should stop now...\n");
			break;
		}
#endif

		do {
			if (test_and_clear_bit(GLUE_FLAG_OID_BIT,
					       &prGlueInfo->u4Flag)) {
/* get current prIoReq */
				prIoReq = &(prGlueInfo->OidEntry);
#if CFG_ENABLE_WIFI_DIRECT
				if (prGlueInfo->prAdapter->fgIsP2PRegistered ==
				    FALSE && prIoReq->fgIsP2pOid == TRUE) {
/*    if this Oid belongs to p2p and p2p module is removed
*      do nothing,
*/
				} else
#endif
				{
					if (FALSE == prIoReq->fgRead) {
						prIoReq->rStatus =
						    wlanSetInformation
						    (prIoReq->prAdapter,
						     prIoReq->pfnOidHandler,
						     prIoReq->pvInfoBuf,
						     prIoReq->u4InfoBufLen,
						     prIoReq->pu4QryInfoLen);
					} else {
						prIoReq->rStatus =
						    wlanQueryInformation
						    (prIoReq->prAdapter,
						     prIoReq->pfnOidHandler,
						     prIoReq->pvInfoBuf,
						     prIoReq->u4InfoBufLen,
						     prIoReq->pu4QryInfoLen);
					}

					if (prIoReq->rStatus !=
					    WLAN_STATUS_PENDING) {
						complete(&prGlueInfo->
							 rPendComp);
					} else {
						wlanoidTimeoutCheck(prGlueInfo->
								    prAdapter,
								    prIoReq->
								    pfnOidHandler);
					}
				}
			}
		} while (FALSE);

/*
*
* if TX request, clear the TXREQ flag. TXREQ set by kalSetEvent/GlueSetEvent
* indicates the following requests occur
*
*/
#if 0
		if (prGlueInfo->u4Flag & GLUE_FLAG_HALT) {
			printk(KERN_INFO DRV_NAME
			       "<3>tx_thread should stop now...\n");
			break;
		}
#endif

		if (test_and_clear_bit(GLUE_FLAG_TXREQ_BIT,
				       &prGlueInfo->u4Flag)) {
/* Process Mailbox Messages */
			wlanProcessMboxMessage(prGlueInfo->prAdapter);

/* Process CMD request */
			do {
				if (prCmdQue->u4NumElem > 0) {
					if (fgNeedHwAccess == FALSE) {
						fgNeedHwAccess = TRUE;

						wlanAcquirePowerControl
						    (prGlueInfo->prAdapter);
					}
					wlanProcessCommandQueue(prGlueInfo->
								prAdapter,
								prCmdQue);
				}
			} while (FALSE);

/* Handle Packet Tx */
			{
				while (QUEUE_IS_NOT_EMPTY(prTxQueue)) {
					GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo,
							       SPIN_LOCK_TX_QUE);
					QUEUE_REMOVE_HEAD(prTxQueue,
							  prQueueEntry,
							  P_QUE_ENTRY_T);
					GLUE_RELEASE_SPIN_LOCK(prGlueInfo,
							       SPIN_LOCK_TX_QUE);

					ASSERT(prQueueEntry);
					if (NULL == prQueueEntry) {
						break;
					}

					prSkb = (struct sk_buff *)
					    GLUE_GET_PKT_DESCRIPTOR
					    (prQueueEntry);
					ASSERT(prSkb);
					if (NULL == prSkb) {
						DBGLOG(INIT, WARN,
						       ("prSkb == NULL in tx\n"));
						continue;
					}

					if (wlanEnqueueTxPacket
					    (prGlueInfo->prAdapter,
					     (P_NATIVE_PACKET)
					     prSkb) == WLAN_STATUS_RESOURCES) {
						GLUE_ACQUIRE_SPIN_LOCK
						    (prGlueInfo,
						     SPIN_LOCK_TX_QUE);
						QUEUE_INSERT_HEAD(prTxQueue,
								  prQueueEntry);
						GLUE_RELEASE_SPIN_LOCK
						    (prGlueInfo,
						     SPIN_LOCK_TX_QUE);

						break;
					}
				}

				if (wlanGetTxPendingFrameCount
				    (prGlueInfo->prAdapter) > 0) {
					wlanTxPendingPackets(prGlueInfo->
							     prAdapter,
							     &fgNeedHwAccess);
				}
			}
		}

/* Process RX, In linux, we don't need to free sk_buff by ourself */

/* In linux, we don't need to free sk_buff by ourself */

/* In linux, we don't do reset */
		if (fgNeedHwAccess == TRUE) {
			wlanReleasePowerControl(prGlueInfo->prAdapter);
		}

/* handle cnmTimer time out */
		if (test_and_clear_bit(GLUE_FLAG_TIMEOUT_BIT,
				       &prGlueInfo->u4Flag)) {
			wlanTimerTimeoutCheck(prGlueInfo->prAdapter);
		}
#if CFG_DBG_GPIO_PINS
/* TX thread go to sleep */
		if (!prGlueInfo->u4Flag) {
			mtk_wcn_stp_debug_gpio_assert(IDX_TX_THREAD,
						      DBG_TIE_HIGH);
		}
#endif
	}

#if 0
	if (fgNeedHwAccess == TRUE) {
		wlanReleasePowerControl(prGlueInfo->prAdapter);
	}
#endif

/* flush the pending TX packets */
	if (prGlueInfo->i4TxPendingFrameNum > 0) {
		kalFlushPendingTxPackets(prGlueInfo);
	}

/* flush pending security frames */
	if (prGlueInfo->i4TxPendingSecurityFrameNum > 0) {
		kalClearSecurityFrames(prGlueInfo);
	}

/* remove pending oid */
	wlanReleasePendingOid(prGlueInfo->prAdapter, 0);

/* In linux, we don't need to free sk_buff by ourself */

	DBGLOG(INIT, INFO, ("mtk_sdiod stops\n"));
	complete(&prGlueInfo->rHaltComp);

	return 0;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to check if card is removed
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
*
* \retval TRUE:     card is removed
*         FALSE:    card is still attached
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalIsCardRemoved(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return FALSE;
/* Linux MMC doesn't have removal notification yet */
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to send command to firmware for overriding netweork address
*
* \param pvGlueInfo Pointer of GLUE Data Structure

* \retval TRUE
*         FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalRetrieveNetworkAddress(IN P_GLUE_INFO_T prGlueInfo,
			  IN OUT PARAM_MAC_ADDRESS * prMacAddr)
{
	ASSERT(prGlueInfo);

	if (prGlueInfo->fgIsMacAddrOverride == FALSE) {
#if !defined(CONFIG_X86)
		UINT_32 i;
		BOOLEAN fgIsReadError = FALSE;

		for (i = 0; i < MAC_ADDR_LEN; i += 2) {
			if (kalCfgDataRead16(prGlueInfo,
					     OFFSET_OF(WIFI_CFG_PARAM_STRUCT,
						       aucMacAddress) + i,
					     (PUINT_16) (((PUINT_8) prMacAddr) +
							 i)) == FALSE) {
				fgIsReadError = TRUE;
				break;
			}
		}

		if (fgIsReadError == TRUE) {
			return FALSE;
		} else {
			return TRUE;
		}
#else
/* x86 Linux doesn't need to override network address so far */
		return FALSE;
#endif
	} else {
		COPY_MAC_ADDR(prMacAddr, prGlueInfo->rMacAddrOverride);

		return TRUE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to flush pending TX packets in glue layer
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID kalFlushPendingTxPackets(IN P_GLUE_INFO_T prGlueInfo)
{
	P_QUE_T prTxQue;
	P_QUE_ENTRY_T prQueueEntry;
	PVOID prPacket;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prTxQue = &(prGlueInfo->rTxQueue);

	if (prGlueInfo->i4TxPendingFrameNum) {
		while (TRUE) {
			GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);
			QUEUE_REMOVE_HEAD(prTxQue, prQueueEntry, P_QUE_ENTRY_T);
			GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_TX_QUE);

			if (prQueueEntry == NULL) {
				break;
			}

			prPacket = GLUE_GET_PKT_DESCRIPTOR(prQueueEntry);

			kalSendComplete(prGlueInfo,
					prPacket, WLAN_STATUS_NOT_ACCEPTED);
		}
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is get indicated media state
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
*
* \retval
*/
/*----------------------------------------------------------------------------*/
ENUM_PARAM_MEDIA_STATE_T kalGetMediaStateIndicated(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->eParamMediaStateIndicated;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to set indicated media state
*
* \param pvGlueInfo     Pointer of GLUE Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID
kalSetMediaStateIndicated(IN P_GLUE_INFO_T prGlueInfo,
			  IN ENUM_PARAM_MEDIA_STATE_T eParamMediaStateIndicate)
{
	ASSERT(prGlueInfo);

	prGlueInfo->eParamMediaStateIndicated = eParamMediaStateIndicate;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to clear pending OID staying in command queue
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID kalOidCmdClearance(IN P_GLUE_INFO_T prGlueInfo)
{
	P_QUE_T prCmdQue;
	QUE_T rTempCmdQue;
	P_QUE_T prTempCmdQue = &rTempCmdQue;
	P_QUE_ENTRY_T prQueueEntry = (P_QUE_ENTRY_T) NULL;
	P_CMD_INFO_T prCmdInfo = (P_CMD_INFO_T) NULL;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);

	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_MOVE_ALL(prTempCmdQue, prCmdQue);

	QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	while (prQueueEntry) {
		if (((P_CMD_INFO_T) prQueueEntry)->fgIsOid) {
			prCmdInfo = (P_CMD_INFO_T) prQueueEntry;
			break;
		} else {
			QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
		}

		QUEUE_REMOVE_HEAD(prTempCmdQue, prQueueEntry, P_QUE_ENTRY_T);
	}

	QUEUE_CONCATENATE_QUEUES(prCmdQue, prTempCmdQue);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);

	if (prCmdInfo) {
		if (prCmdInfo->pfCmdTimeoutHandler) {
			prCmdInfo->pfCmdTimeoutHandler(prGlueInfo->prAdapter,
						       prCmdInfo);
		} else {
			kalOidComplete(prGlueInfo,
				       prCmdInfo->fgSetQuery,
				       0, WLAN_STATUS_NOT_ACCEPTED);
		}

		prGlueInfo->u4OidCompleteFlag = 1;
		cmdBufFreeCmdInfo(prGlueInfo->prAdapter, prCmdInfo);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to insert command into prCmdQueue
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*        prQueueEntry   Pointer of queue entry to be inserted
*
* \retval none
*/
/*----------------------------------------------------------------------------*/
VOID
kalEnqueueCommand(IN P_GLUE_INFO_T prGlueInfo, IN P_QUE_ENTRY_T prQueueEntry)
{
	P_QUE_T prCmdQue;

	GLUE_SPIN_LOCK_DECLARATION();

	ASSERT(prGlueInfo);
	ASSERT(prQueueEntry);

	prCmdQue = &prGlueInfo->rCmdQueue;

	GLUE_ACQUIRE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
	QUEUE_INSERT_TAIL(prCmdQue, prQueueEntry);
	GLUE_RELEASE_SPIN_LOCK(prGlueInfo, SPIN_LOCK_CMD_QUE);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief Handle EVENT_ID_ASSOC_INFO event packet by indicating to OS with
*        proper information
*
* @param pvGlueInfo     Pointer of GLUE Data Structure
* @param prAssocInfo    Pointer of EVENT_ID_ASSOC_INFO Packet
*
* @return none
*/
/*----------------------------------------------------------------------------*/
VOID
kalHandleAssocInfo(IN P_GLUE_INFO_T prGlueInfo,
		   IN P_EVENT_ASSOC_INFO prAssocInfo)
{
/* to do */
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to get firmware load address from registry
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalGetFwLoadAddress(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->rRegInfo.u4LoadAddress;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to get firmware start address from registry
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalGetFwStartAddress(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->rRegInfo.u4StartAddress;
}

/*----------------------------------------------------------------------------*/
/*!
* * @brief Notify OS with SendComplete event of the specific packet. Linux should
* *        free packets here.
* *
* * @param pvGlueInfo     Pointer of GLUE Data Structure
* * @param pvPacket       Pointer of Packet Handle
* * @param status         Status Code for OS upper layer
* *
* * @return none
* */
/*----------------------------------------------------------------------------*/

/* / Todo */
VOID
kalSecurityFrameSendComplete(IN P_GLUE_INFO_T prGlueInfo,
			     IN PVOID pvPacket, IN WLAN_STATUS rStatus)
{
	ASSERT(pvPacket);

	dev_kfree_skb((struct sk_buff *)pvPacket);
	GLUE_DEC_REF_CNT(prGlueInfo->i4TxPendingSecurityFrameNum);
}

UINT_32 kalGetTxPendingFrameCount(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return (UINT_32) (prGlueInfo->i4TxPendingFrameNum);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to retrieve the number of pending commands
*        (including MMPDU, 802.1X and command packets)
*
* \param prGlueInfo     Pointer of GLUE Data Structure
*
* \retval
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalGetTxPendingCmdCount(IN P_GLUE_INFO_T prGlueInfo)
{
	P_QUE_T prCmdQue;

	ASSERT(prGlueInfo);
	prCmdQue = &prGlueInfo->rCmdQueue;

	return prCmdQue->u4NumElem;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Timer Initialization Procedure
*
* \param[in] prGlueInfo     Pointer to GLUE Data Structure
* \param[in] prTimerHandler Pointer to timer handling function, whose only
*                           argument is "prAdapter"
*
* \retval none
*
*/
/*----------------------------------------------------------------------------*/

/* static struct timer_list tickfn; */

VOID kalOsTimerInitialize(IN P_GLUE_INFO_T prGlueInfo, IN PVOID prTimerHandler)
{
	ASSERT(prGlueInfo);

	init_timer(&(prGlueInfo->tickfn));
	prGlueInfo->tickfn.function = prTimerHandler;
	prGlueInfo->tickfn.data = (unsigned long)prGlueInfo;
}

/* Todo */
/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to set the time to do the time out check.
*
* \param[in] prGlueInfo Pointer to GLUE Data Structure
* \param[in] rInterval  Time out interval from current time.
*
* \retval TRUE Success.
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalSetTimer(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 u4Interval)
{
	ASSERT(prGlueInfo);
	del_timer_sync(&(prGlueInfo->tickfn));

	prGlueInfo->tickfn.expires = jiffies + u4Interval * HZ / MSEC_PER_SEC;
	add_timer(&(prGlueInfo->tickfn));

	return TRUE;		/* success */
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is called to cancel
*
* \param[in] prGlueInfo Pointer to GLUE Data Structure
*
* \retval TRUE  :   Timer has been canceled
*         FALAE :   Timer doens't exist
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalCancelTimer(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	clear_bit(GLUE_FLAG_TIMEOUT_BIT, &prGlueInfo->u4Flag);

	if (del_timer_sync(&(prGlueInfo->tickfn)) >= 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is a callback function for scanning done
*
* \param[in] prGlueInfo Pointer to GLUE Data Structure
*
* \retval none
*
*/
/*----------------------------------------------------------------------------*/
VOID
kalScanDone(IN P_GLUE_INFO_T prGlueInfo,
	    IN ENUM_KAL_NETWORK_TYPE_INDEX_T eNetTypeIdx, IN WLAN_STATUS status)
{
	ASSERT(prGlueInfo);

/* check for system configuration for generating error message on scan list */
	wlanCheckSystemConfiguration(prGlueInfo->prAdapter);

	kalIndicateStatusAndComplete(prGlueInfo,
				     WLAN_STATUS_SCAN_COMPLETE, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief This routine is used to generate a random number
*
* \param none
*
* \retval UINT_32
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalRandomNumber(VOID)
{
	UINT_32 number = 0;

	get_random_bytes(&number, 4);

	return number;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief command timeout call-back function
*
* \param[in] prGlueInfo Pointer to the GLUE data structure.
*
* \retval (none)
*/
/*----------------------------------------------------------------------------*/
VOID kalTimeoutHandler(unsigned long arg)
{
	P_GLUE_INFO_T prGlueInfo = (P_GLUE_INFO_T) arg;

	ASSERT(prGlueInfo);

/* Notify tx thread  for timeout event */
	set_bit(GLUE_FLAG_TIMEOUT_BIT, &prGlueInfo->u4Flag);
	wake_up_interruptible(&prGlueInfo->waitq);

	return;
}

VOID kalSetEvent(P_GLUE_INFO_T pr)
{
	set_bit(GLUE_FLAG_TXREQ_BIT, &pr->u4Flag);
	wake_up_interruptible(&pr->waitq);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief to check if configuration file (NVRAM/Registry) exists
*
* \param[in]
*           prGlueInfo
*
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalIsConfigurationExist(IN P_GLUE_INFO_T prGlueInfo)
{
#if !defined(CONFIG_X86)
	ASSERT(prGlueInfo);

	return prGlueInfo->fgNvramAvailable;
#else
/* there is no configuration data for x86-linux */
	return FALSE;
#endif
}

/*----------------------------------------------------------------------------*/
/*!
* \brief to retrieve Registry information
*
* \param[in]
*           prGlueInfo
*
* \return
*           Pointer of REG_INFO_T
*/
/*----------------------------------------------------------------------------*/
P_REG_INFO_T kalGetConfiguration(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return &(prGlueInfo->rRegInfo);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief to retrieve version information of corresponding configuration file
*
* \param[in]
*           prGlueInfo
*
* \param[out]
*           pu2Part1CfgOwnVersion
*           pu2Part1CfgPeerVersion
*           pu2Part2CfgOwnVersion
*           pu2Part2CfgPeerVersion
*
* \return
*           NONE
*/
/*----------------------------------------------------------------------------*/
VOID
kalGetConfigurationVersion(IN P_GLUE_INFO_T prGlueInfo,
			   OUT PUINT_16 pu2Part1CfgOwnVersion,
			   OUT PUINT_16 pu2Part1CfgPeerVersion,
			   OUT PUINT_16 pu2Part2CfgOwnVersion,
			   OUT PUINT_16 pu2Part2CfgPeerVersion)
{
	ASSERT(prGlueInfo);

	ASSERT(pu2Part1CfgOwnVersion);
	ASSERT(pu2Part1CfgPeerVersion);
	ASSERT(pu2Part2CfgOwnVersion);
	ASSERT(pu2Part2CfgPeerVersion);

	kalCfgDataRead16(prGlueInfo,
			 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, u2Part1OwnVersion),
			 pu2Part1CfgOwnVersion);

	kalCfgDataRead16(prGlueInfo,
			 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, u2Part1PeerVersion),
			 pu2Part1CfgPeerVersion);

	kalCfgDataRead16(prGlueInfo,
			 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, u2Part2OwnVersion),
			 pu2Part2CfgOwnVersion);

	kalCfgDataRead16(prGlueInfo,
			 OFFSET_OF(WIFI_CFG_PARAM_STRUCT, u2Part2PeerVersion),
			 pu2Part2CfgPeerVersion);

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief to check if the WPS is active or not
*
* \param[in]
*           prGlueInfo
*
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalWSCGetActiveState(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->fgWpsActive;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief update RSSI and LinkQuality to GLUE layer
*
* \param[in]
*           prGlueInfo
*           eNetTypeIdx
*           cRssi
*           cLinkQuality
*
* \return
*           None
*/
/*----------------------------------------------------------------------------*/
VOID
kalUpdateRSSI(IN P_GLUE_INFO_T prGlueInfo,
	      IN ENUM_KAL_NETWORK_TYPE_INDEX_T eNetTypeIdx,
	      IN INT_8 cRssi, IN INT_8 cLinkQuality)
{
	struct iw_statistics *pStats = (struct iw_statistics *)NULL;

	ASSERT(prGlueInfo);

	switch (eNetTypeIdx) {
	case KAL_NETWORK_TYPE_AIS_INDEX:
		pStats = (struct iw_statistics *)(&(prGlueInfo->rIwStats));
		break;
#if CFG_ENABLE_WIFI_DIRECT
#if CFG_SUPPORT_P2P_RSSI_QUERY
	case KAL_NETWORK_TYPE_P2P_INDEX:
		pStats = (struct iw_statistics *)(&(prGlueInfo->rP2pIwStats));
		break;
#endif
#endif
	default:
		break;
	}

	if (pStats) {
		pStats->qual.qual = cLinkQuality;
		pStats->qual.noise = 0;
		pStats->qual.updated = IW_QUAL_QUAL_UPDATED |
		    IW_QUAL_NOISE_UPDATED;
		pStats->qual.level = 0x100 + cRssi;
		pStats->qual.updated |= IW_QUAL_LEVEL_UPDATED;
	}

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Pre-allocate I/O buffer
*
* \param[in]
*           none
*
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN kalInitIOBuffer(VOID)
{
	UINT_32 u4Size;

	if (CFG_COALESCING_BUFFER_SIZE >= CFG_RX_COALESCING_BUFFER_SIZE) {
		u4Size = CFG_COALESCING_BUFFER_SIZE +
		    sizeof(ENHANCE_MODE_DATA_STRUCT_T);
	} else {
		u4Size = CFG_RX_COALESCING_BUFFER_SIZE +
		    sizeof(ENHANCE_MODE_DATA_STRUCT_T);
	}

	pvIoBuffer = kmalloc(u4Size, GFP_KERNEL);
	if (pvIoBuffer) {
		pvIoBufferSize = u4Size;
		pvIoBufferUsage = 0;

		return TRUE;
	}

	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Free pre-allocated I/O buffer
*
* \param[in]
*           none
*
* \return
*           none
*/
/*----------------------------------------------------------------------------*/
VOID kalUninitIOBuffer(VOID)
{
	if (pvIoBuffer) {
		kfree(pvIoBuffer);

		pvIoBuffer = (PVOID) NULL;
		pvIoBufferSize = 0;
		pvIoBufferUsage = 0;
	}

	return;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Dispatch pre-allocated I/O buffer
*
* \param[in]
*           u4AllocSize
*
* \return
*           PVOID for pointer of pre-allocated I/O buffer
*/
/*----------------------------------------------------------------------------*/
PVOID kalAllocateIOBuffer(IN UINT_32 u4AllocSize)
{
	PVOID ret = (PVOID) NULL;

	if (pvIoBuffer) {
		if (u4AllocSize <= (pvIoBufferSize - pvIoBufferUsage)) {
			ret =
			    (PVOID) &
			    (((PUINT_8) (pvIoBuffer))[pvIoBufferUsage]);
			pvIoBufferUsage += u4AllocSize;
		}
	} else {
/* fault tolerance */
		ret = (PVOID) kalMemAlloc(u4AllocSize, PHY_MEM_TYPE);
	}

	return ret;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Release all dispatched I/O buffer
*
* \param[in]
*           none
*
* \return
*           none
*/
/*----------------------------------------------------------------------------*/
VOID kalReleaseIOBuffer(IN PVOID pvAddr, IN UINT_32 u4Size)
{
	if (pvIoBuffer) {
		pvIoBufferUsage -= u4Size;
	} else {
/* fault tolerance */
		kalMemFree(pvAddr, PHY_MEM_TYPE, u4Size);
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in] prAdapter  Pointer of ADAPTER_T
*
* \return none
*/
/*----------------------------------------------------------------------------*/
VOID
kalGetChannelList(IN P_GLUE_INFO_T prGlueInfo,
		  IN ENUM_BAND_T eSpecificBand,
		  IN UINT_8 ucMaxChannelNum,
		  IN PUINT_8 pucNumOfChannel,
		  IN P_RF_CHANNEL_INFO_T paucChannelList)
{
	rlmDomainGetChnlList(prGlueInfo->prAdapter,
			     eSpecificBand,
			     ucMaxChannelNum, pucNumOfChannel, paucChannelList);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief
*
* \param[in] prAdapter  Pointer of ADAPTER_T
*
* \return none
*/
/*----------------------------------------------------------------------------*/
BOOL kalIsAPmode(IN P_GLUE_INFO_T prGlueInfo)
{
#if CFG_ENABLE_WIFI_DIRECT
	if (IS_NET_ACTIVE(prGlueInfo->prAdapter, NETWORK_TYPE_P2P_INDEX) &&
	    p2pFuncIsAPMode(prGlueInfo->prAdapter->rWifiVar.prP2pFsmInfo))
		return TRUE;
#endif

	return FALSE;
}

#if CFG_SUPPORT_802_11W
/*----------------------------------------------------------------------------*/
/*!
* \brief to check if the MFP is active or not
*
* \param[in]
*           prGlueInfo
*
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
UINT_32 kalGetMfpSetting(IN P_GLUE_INFO_T prGlueInfo)
{
	ASSERT(prGlueInfo);

	return prGlueInfo->rWpaInfo.u4Mfp;
}
#endif

struct file *kalFileOpen(const char *path, int flags, int rights)
{
	struct file *filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if (IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

VOID kalFileClose(struct file * file)
{
	filp_close(file, NULL);
}

UINT_32
kalFileRead(struct file *file,
	    unsigned long long offset, unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

UINT_32
kalFileWrite(struct file * file,
	     unsigned long long offset, unsigned char *data, unsigned int size)
{
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

UINT_32
kalWriteToFile(const PUINT_8 pucPath,
	       BOOLEAN fgDoAppend, PUINT_8 pucData, UINT_32 u4Size)
{
	struct file *file = NULL;
	UINT_32 ret;
	UINT_32 u4Flags = 0;

	if (fgDoAppend) {
		u4Flags = O_APPEND;
	}

	file = kalFileOpen(pucPath, O_WRONLY | O_CREAT | u4Flags, S_IRWXU);
	ret = kalFileWrite(file, 0, pucData, u4Size);
	kalFileClose(file);

	return ret;
}

/*----------------------------------------------------------------------------*/
/*!
* \brief    To indicate BSS-INFO to NL80211 as scanning result
*
* \param[in]
*           prGlueInfo
*           pucBeaconProbeResp
*           u4FrameLen
*
*
*
* \return
*           none
*/
/*----------------------------------------------------------------------------*/
VOID
kalIndicateBssInfo(IN P_GLUE_INFO_T prGlueInfo,
		   IN PUINT_8 pucBeaconProbeResp,
		   IN UINT_32 u4FrameLen,
		   IN UINT_8 ucChannelNum, IN INT_32 i4SignalStrength)
{
	struct wiphy *wiphy;
	struct ieee80211_channel *prChannel = NULL;

	ASSERT(prGlueInfo);
	wiphy = priv_to_wiphy(prGlueInfo);

/* search through channel entries */
	if (ucChannelNum <= 14) {
		prChannel = ieee80211_get_channel(wiphy,
						  ieee80211_channel_to_frequency
						  (ucChannelNum,
						   IEEE80211_BAND_2GHZ));
	} else {
		prChannel = ieee80211_get_channel(wiphy,
						  ieee80211_channel_to_frequency
						  (ucChannelNum,
						   IEEE80211_BAND_5GHZ));
	}

	if (prChannel != NULL && prGlueInfo->prScanRequest != NULL) {
		struct cfg80211_bss *bss;

/* indicate to NL80211 subsystem */
		bss = cfg80211_inform_bss_frame(wiphy,
						prChannel,
						(struct ieee80211_mgmt *)
						pucBeaconProbeResp,
						u4FrameLen,
						i4SignalStrength * 100,
						GFP_KERNEL);

		if (!bss) {
			DBGLOG(REQ, WARN,
			       ("cfg80211_inform_bss_frame() returned with NULL\n"));
		} else {
			cfg80211_put_bss(bss);
		}
	}

	return;
}
