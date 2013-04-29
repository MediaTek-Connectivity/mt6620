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

#ifndef _GL_KAL_H
#define _GL_KAL_H

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "config.h"
#include "gl_typedef.h"
#include "gl_os.h"
#include "link.h"
#include "nic/mac.h"
#include "nic/wlan_def.h"
#include "wlan_lib.h"
#include "wlan_oid.h"
#include "gl_wext_priv.h"

#if CFG_ENABLE_BT_OVER_WIFI
#include "nic/bow.h"
#endif

#if DBG
extern int allocatedMemSize;
#endif

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
/* #define USEC_PER_MSEC   (1000) */

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
typedef enum _ENUM_SPIN_LOCK_CATEGORY_E {
	SPIN_LOCK_FSM = 0,

/* FIX ME */
	SPIN_LOCK_RX_QUE,
	SPIN_LOCK_TX_QUE,
	SPIN_LOCK_CMD_QUE,
	SPIN_LOCK_TX_RESOURCE,
	SPIN_LOCK_CMD_RESOURCE,
	SPIN_LOCK_QM_TX_QUEUE,
	SPIN_LOCK_CMD_PENDING,
	SPIN_LOCK_CMD_SEQ_NUM,
	SPIN_LOCK_TX_MSDU_INFO_LIST,
	SPIN_LOCK_TXING_MGMT_LIST,
	SPIN_LOCK_TX_SEQ_NUM,
	SPIN_LOCK_TX_COUNT,
	SPIN_LOCK_TXS_COUNT,
/* end    */
	SPIN_LOCK_TX,
	SPIN_LOCK_IO_REQ,
	SPIN_LOCK_INT,

	SPIN_LOCK_MGT_BUF,
	SPIN_LOCK_MSG_BUF,
	SPIN_LOCK_STA_REC,

	SPIN_LOCK_MAILBOX,
	SPIN_LOCK_TIMER,

	SPIN_LOCK_BOW_TABLE,

	SPIN_LOCK_EHPI_BUS,	/* only for EHPI */
	SPIN_LOCK_NET_DEV,
	SPIN_LOCK_NUM
} ENUM_SPIN_LOCK_CATEGORY_E;

/* event for assoc infomation update */
typedef struct _EVENT_ASSOC_INFO {
	UINT_8 ucAssocReq;	/* 1 for assoc req, 0 for assoc rsp */
	UINT_8 ucReassoc;	/* 0 for assoc, 1 for reassoc */
	UINT_16 u2Length;
	PUINT_8 pucIe;
} EVENT_ASSOC_INFO, *P_EVENT_ASSOC_INFO;

typedef enum _ENUM_KAL_NETWORK_TYPE_INDEX_T {
	KAL_NETWORK_TYPE_AIS_INDEX = 0,
#if CFG_ENABLE_WIFI_DIRECT
	KAL_NETWORK_TYPE_P2P_INDEX,
#endif
#if CFG_ENABLE_BT_OVER_WIFI
	KAL_NETWORK_TYPE_BOW_INDEX,
#endif
	KAL_NETWORK_TYPE_INDEX_NUM
} ENUM_KAL_NETWORK_TYPE_INDEX_T;

typedef enum _ENUM_KAL_MEM_ALLOCATION_TYPE_E {
	PHY_MEM_TYPE,		/* physically continuous */
	VIR_MEM_TYPE,		/* virtually continous */
	MEM_TYPE_NUM
} ENUM_KAL_MEM_ALLOCATION_TYPE;

#if CONFIG_ANDROID		/* Defined in Android kernel source */
typedef struct wake_lock KAL_WAKE_LOCK_T, *P_KAL_WAKE_LOCK_T;
#else
typedef UINT_32 KAL_WAKE_LOCK_T, *P_KAL_WAKE_LOCK_T;
#endif

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
/*----------------------------------------------------------------------------*/
/* Macros of SPIN LOCK operations for using in Driver Layer                   */
/*----------------------------------------------------------------------------*/
#define KAL_SPIN_LOCK_DECLARATION()             UINT_32 __u4Flags

#define KAL_ACQUIRE_SPIN_LOCK(_prAdapter, _rLockCategory)   \
kalAcquireSpinLock(((P_ADAPTER_T)_prAdapter)->prGlueInfo, \
_rLockCategory, \
&__u4Flags)

#define KAL_RELEASE_SPIN_LOCK(_prAdapter, _rLockCategory)   \
kalReleaseSpinLock(((P_ADAPTER_T)_prAdapter)->prGlueInfo, \
_rLockCategory, \
__u4Flags)

/*----------------------------------------------------------------------------*/
/* Macros for accessing Reserved Fields of native packet                      */
/*----------------------------------------------------------------------------*/
#define KAL_GET_PKT_QUEUE_ENTRY(_p)             GLUE_GET_PKT_QUEUE_ENTRY(_p)
#define KAL_GET_PKT_DESCRIPTOR(_prQueueEntry)   GLUE_GET_PKT_DESCRIPTOR( \
_prQueueEntry)
#define KAL_GET_PKT_TID(_p)                     GLUE_GET_PKT_TID(_p)
#define KAL_GET_PKT_IS1X(_p)                    GLUE_GET_PKT_IS1X(_p)
#define KAL_GET_PKT_HEADER_LEN(_p)              GLUE_GET_PKT_HEADER_LEN(_p)
#define KAL_GET_PKT_PAYLOAD_LEN(_p)             GLUE_GET_PKT_PAYLOAD_LEN(_p)
#define KAL_GET_PKT_ARRIVAL_TIME(_p)            GLUE_GET_PKT_ARRIVAL_TIME(_p)

/*----------------------------------------------------------------------------*/
/* Macros of wake_lock operations for using in Driver Layer                   */
/*----------------------------------------------------------------------------*/
#if CONFIG_ANDROID		/* Defined in Android kernel source */
#define KAL_WAKE_LOCK_INIT(_prAdapter, _prWakeLock, _pcName) \
wake_lock_init(_prWakeLock, WAKE_LOCK_SUSPEND, _pcName)

#define KAL_WAKE_LOCK_DESTROY(_prAdapter, _prWakeLock) \
wake_lock_destroy(_prWakeLock)

#define KAL_WAKE_LOCK(_prAdapter, _prWakeLock) \
wake_lock(_prWakeLock)

#define KAL_WAKE_LOCK_TIMEOUT(_prAdapter, _prWakeLock, _u4Timeout) \
wake_lock_timeout(_prWakeLock, _u4Timeout)

#define KAL_WAKE_UNLOCK(_prAdapter, _prWakeLock) \
wake_unlock(_prWakeLock)

#else
#define KAL_WAKE_LOCK_INIT(_prAdapter, _prWakeLock, _pcName)
#define KAL_WAKE_LOCK_DESTROY(_prAdapter, _prWakeLock)
#define KAL_WAKE_LOCK(_prAdapter, _prWakeLock)
#define KAL_WAKE_LOCK_TIMEOUT(_prAdapter, _prWakeLock, _u4Timeout)
#define KAL_WAKE_UNLOCK(_prAdapter, _prWakeLock)
#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief Cache memory allocation
*
* \param[in] u4Size Required memory size.
* \param[in] eMemType  Memory allocation type
*
* \return Pointer to allocated memory
*         or NULL
*/
/*----------------------------------------------------------------------------*/
#if DBG
#define kalMemAlloc(u4Size, eMemType) ({    \
void *pvAddr; \
if (eMemType == PHY_MEM_TYPE) { \
pvAddr =	\
kmalloc(u4Size, \
GFP_KERNEL);   \
} \
else { \
pvAddr = vmalloc(u4Size);   \
} \
if (pvAddr) {   \
allocatedMemSize += \
u4Size;	 \
printk(KERN_INFO	\
DRV_NAME \
"0x%p(%ld) allocated (%s:%s)\n", \
pvAddr, \
(UINT_32)u4Size, \
__FILE__,	\
__FUNCTION__);  \
}   \
pvAddr; \
})
#else
#define kalMemAlloc(u4Size, eMemType) ({    \
void *pvAddr; \
if (eMemType == PHY_MEM_TYPE) { \
pvAddr =	\
kmalloc(u4Size, \
GFP_KERNEL);   \
} \
else { \
pvAddr = vmalloc(u4Size);   \
} \
pvAddr; \
})
#endif

/*----------------------------------------------------------------------------*/
/*!
* \brief Free allocated cache memory
*
* \param[in] pvAddr Required memory size.
* \param[in] eMemType  Memory allocation type
* \param[in] u4Size Allocated memory size.
*
* \return -
*/
/*----------------------------------------------------------------------------*/
#if DBG
#define kalMemFree(pvAddr, eMemType, u4Size)  \
{   \
if (pvAddr) {	\
allocatedMemSize -= u4Size; \
printk(KERN_INFO DRV_NAME "0x%p(%ld) freed (%s:%s)\n", \
pvAddr, (UINT_32)u4Size, __FILE__, __FUNCTION__);  \
}   \
if (eMemType == PHY_MEM_TYPE) {	\
kfree(pvAddr); \
} \
else { \
vfree(pvAddr); \
} \
}
#else
#define kalMemFree(pvAddr, eMemType, u4Size)  \
{   \
if (eMemType == PHY_MEM_TYPE) {	\
kfree(pvAddr); \
} \
else { \
vfree(pvAddr); \
} \
}
#endif

#define kalUdelay(u4USec)                           udelay(u4USec)

#define kalMdelay(u4MSec)                           mdelay(u4MSec)
#define kalMsleep(u4MSec)                           msleep(u4MSec)

/* Copy memory from user space to kernel space */
#define kalMemCopyFromUser(_pvTo, _pvFrom, _u4N)    copy_from_user(_pvTo, \
_pvFrom, \
_u4N)

/* Copy memory from kernel space to user space */
#define kalMemCopyToUser(_pvTo, _pvFrom, _u4N)      copy_to_user(_pvTo,	\
_pvFrom, \
_u4N)

/* Copy memory block with specific size */
#define kalMemCopy(pvDst, pvSrc, u4Size)            memcpy(pvDst, pvSrc, u4Size)

/* Set memory block with specific pattern */
#define kalMemSet(pvAddr, ucPattern, u4Size)        memset(pvAddr, \
ucPattern, \
u4Size)

/* Compare two memory block with specific length.
* Return zero if they are the same.
*/
#define kalMemCmp(pvAddr1, pvAddr2, u4Size)         memcmp(pvAddr1, \
pvAddr2, \
u4Size)

/* Zero specific memory block */
#define kalMemZero(pvAddr, u4Size)                  memset(pvAddr, 0, u4Size)

/* defined for wince sdio driver only */
#if defined(_HIF_SDIO)
#define kalDevSetPowerState(prGlueInfo, ePowerMode) glSetPowerState(prGlueInfo,	\
ePowerMode)
#else
#define kalDevSetPowerState(prGlueInfo, ePowerMode)
#endif

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
#define kalSendComplete(prGlueInfo, pvPacket, status)	\
kalSendCompleteAndAwakeQueue(prGlueInfo, pvPacket)

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is used to locate the starting address of incoming ethernet
*        frame for skb.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] pvPacket       Pointer of Packet Handle
*
* \return starting address of ethernet frame buffer.
*/
/*----------------------------------------------------------------------------*/
#define kalQueryBufferPointer(prGlueInfo, pvPacket)	\
((PUINT_8)((struct sk_buff *)pvPacket)->data)

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is used to query the length of valid buffer which is accessible during
*         port read/write.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] pvPacket       Pointer of Packet Handle
*
* \return starting address of ethernet frame buffer.
*/
/*----------------------------------------------------------------------------*/
#define kalQueryValidBufferLength(prGlueInfo, pvPacket)	    \
((UINT_32)((struct sk_buff *)pvPacket)->end -  \
(UINT_32)((struct sk_buff *)pvPacket)->data)

/*----------------------------------------------------------------------------*/
/*!
* \brief This function is used to copy the entire frame from skb to the destination
*        address in the input parameter.
*
* \param[in] prGlueInfo     Pointer of GLUE Data Structure
* \param[in] pvPacket       Pointer of Packet Handle
* \param[in] pucDestBuffer  Destination Address
*
* \return -
*/
/*----------------------------------------------------------------------------*/
#define kalCopyFrame(prGlueInfo, pvPacket, pucDestBuffer)   \
{ struct sk_buff *skb = (struct sk_buff *)pvPacket; \
memcpy(pucDestBuffer, skb->data, skb->len); }

#define kalGetTimeTick()                            jiffies_to_msecs(jiffies)

#define kalPrint                                    printk

#if !DBG
#if CFG_SUPPORT_XLOG
#define XLOG_TAG   "wlan"

#define XLOG_FUNC(__LEVEL, __FMT ...) \
if (__LEVEL == ANDROID_LOG_ERROR) { \
xlog_printk(ANDROID_LOG_ERROR, XLOG_TAG, ## __FMT); \
} \
else if (__LEVEL == ANDROID_LOG_WARN) {	\
xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, ## __FMT); \
} \
else if (__LEVEL == ANDROID_LOG_INFO) {	\
xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, ## __FMT); \
} \
else if (__LEVEL == ANDROID_LOG_DEBUG) { \
xlog_printk(ANDROID_LOG_DEBUG, XLOG_TAG, ## __FMT); \
} \
else if (__LEVEL == ANDROID_LOG_VERBOSE) { \
xlog_printk(ANDROID_LOG_VERBOSE, XLOG_TAG, ## __FMT); \
}

#define AIS_ERROR_LOGFUNC(_Fmt ...)
#define AIS_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, _Fmt)
#define AIS_INFO_LOGFUNC(_Fmt ...)
#define AIS_STATE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define AIS_EVENT_LOGFUNC(_Fmt ...)
#define AIS_TRACE_LOGFUNC(_Fmt ...)
#define AIS_LOUD_LOGFUNC(_Fmt ...)
#define AIS_TEMP_LOGFUNC(_Fmt ...)

#define AIS_ERROR_LOGDUMP8(x, y)
#define AIS_WARN_LOGDUMP8(x, y)
#define AIS_INFO_LOGDUMP8(x, y)
#define AIS_STATE_LOGDUMP8(x, y)
#define AIS_EVENT_LOGDUMP8(x, y)
#define AIS_TRACE_LOGDUMP8(x, y)
#define AIS_LOUD_LOGDUMP8(x, y)
#define AIS_TEMP_LOGDUMP8(x, y)

#define INTR_ERROR_LOGFUNC(_Fmt ...)
#define INTR_WARN_LOGFUNC(_Fmt ...)
#define INTR_INFO_LOGFUNC(_Fmt ...)
#define INTR_STATE_LOGFUNC(_Fmt ...)
#define INTR_EVENT_LOGFUNC(_Fmt ...)
#define INTR_TRACE_LOGFUNC(_Fmt ...)
#define INTR_LOUD_LOGFUNC(_Fmt ...)
#define INTR_TEMP_LOGFUNC(_Fmt ...)

#define INTR_ERROR_LOGDUMP8(x, y)
#define INTR_WARN_LOGDUMP8(x, y)
#define INTR_INFO_LOGDUMP8(x, y)
#define INTR_STATE_LOGDUMP8(x, y)
#define INTR_EVENT_LOGDUMP8(x, y)
#define INTR_TRACE_LOGDUMP8(x, y)
#define INTR_LOUD_LOGDUMP8(x, y)
#define INTR_TEMP_LOGDUMP8(x, y)

#define INIT_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define INIT_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, \
XLOG_TAG, \
_Fmt)
#define INIT_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define INIT_STATE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define INIT_EVENT_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define INIT_TRACE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_DEBUG, \
XLOG_TAG, \
_Fmt)
#define INIT_LOUD_LOGFUNC(_Fmt ...)
#define INIT_TEMP_LOGFUNC(_Fmt ...)

#define INIT_ERROR_LOGDUMP8(x, y)
#define INIT_WARN_LOGDUMP8(x, y)
#define INIT_INFO_LOGDUMP8(x, y)
#define INIT_STATE_LOGDUMP8(x, y)
#define INIT_EVENT_LOGDUMP8(x, y)
#define INIT_TRACE_LOGDUMP8(x, y)
#define INIT_LOUD_LOGDUMP8(x, y)
#define INIT_TEMP_LOGDUMP8(x, y)

#define AAA_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define AAA_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, _Fmt)
#define AAA_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, _Fmt)
#define AAA_STATE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define AAA_EVENT_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define AAA_TRACE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_DEBUG, \
XLOG_TAG, \
_Fmt)
#define AAA_LOUD_LOGFUNC(_Fmt ...)
#define AAA_TEMP_LOGFUNC(_Fmt ...)

#define AAA_ERROR_LOGDUMP8(x, y)
#define AAA_WARN_LOGDUMP8(x, y)
#define AAA_INFO_LOGDUMP8(x, y)
#define AAA_STATE_LOGDUMP8(x, y)
#define AAA_EVENT_LOGDUMP8(x, y)
#define AAA_TRACE_LOGDUMP8(x, y)
#define AAA_LOUD_LOGDUMP8(x, y)
#define AAA_TEMP_LOGDUMP8(x, y)

#define ROAMING_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define ROAMING_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, \
XLOG_TAG, \
_Fmt)
#define ROAMING_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define ROAMING_STATE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define ROAMING_EVENT_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define ROAMING_TRACE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_DEBUG, \
XLOG_TAG, \
_Fmt)
#define ROAMING_LOUD_LOGFUNC(_Fmt ...)
#define ROAMING_TEMP_LOGFUNC(_Fmt ...)

#define ROAMING_ERROR_LOGDUMP8(x, y)
#define ROAMING_WARN_LOGDUMP8(x, y)
#define ROAMING_INFO_LOGDUMP8(x, y)
#define ROAMING_STATE_LOGDUMP8(x, y)
#define ROAMING_EVENT_LOGDUMP8(x, y)
#define ROAMING_TRACE_LOGDUMP8(x, y)
#define ROAMING_LOUD_LOGDUMP8(x, y)
#define ROAMING_TEMP_LOGDUMP8(x, y)

#define REQ_ERROR_LOGFUNC(_Fmt ...)
#define REQ_WARN_LOGFUNC(_Fmt ...)
#define REQ_INFO_LOGFUNC(_Fmt ...)
#define REQ_STATE_LOGFUNC(_Fmt ...)
#define REQ_EVENT_LOGFUNC(_Fmt ...)
#define REQ_TRACE_LOGFUNC(_Fmt ...)
#define REQ_LOUD_LOGFUNC(_Fmt ...)
#define REQ_TEMP_LOGFUNC(_Fmt ...)

#define REQ_ERROR_LOGDUMP8(x, y)
#define REQ_WARN_LOGDUMP8(x, y)
#define REQ_INFO_LOGDUMP8(x, y)
#define REQ_STATE_LOGDUMP8(x, y)
#define REQ_EVENT_LOGDUMP8(x, y)
#define REQ_TRACE_LOGDUMP8(x, y)
#define REQ_LOUD_LOGDUMP8(x, y)
#define REQ_TEMP_LOGDUMP8(x, y)

#define TX_ERROR_LOGFUNC(_Fmt ...)
#define TX_WARN_LOGFUNC(_Fmt ...)
#define TX_INFO_LOGFUNC(_Fmt ...)
#define TX_STATE_LOGFUNC(_Fmt ...)
#define TX_EVENT_LOGFUNC(_Fmt ...)
#define TX_TRACE_LOGFUNC(_Fmt ...)
#define TX_LOUD_LOGFUNC(_Fmt ...)
#define TX_TEMP_LOGFUNC(_Fmt ...)

#define TX_ERROR_LOGDUMP8(x, y)
#define TX_WARN_LOGDUMP8(x, y)
#define TX_INFO_LOGDUMP8(x, y)
#define TX_STATE_LOGDUMP8(x, y)
#define TX_EVENT_LOGDUMP8(x, y)
#define TX_TRACE_LOGDUMP8(x, y)
#define TX_LOUD_LOGDUMP8(x, y)
#define TX_TEMP_LOGDUMP8(x, y)

#define RX_ERROR_LOGFUNC(_Fmt ...)
#define RX_WARN_LOGFUNC(_Fmt ...)
#define RX_INFO_LOGFUNC(_Fmt ...)
#define RX_STATE_LOGFUNC(_Fmt ...)
#define RX_EVENT_LOGFUNC(_Fmt ...)
#define RX_TRACE_LOGFUNC(_Fmt ...)
#define RX_LOUD_LOGFUNC(_Fmt ...)
#define RX_TEMP_LOGFUNC(_Fmt ...)

#define RX_ERROR_LOGDUMP8(x, y)
#define RX_WARN_LOGDUMP8(x, y)
#define RX_INFO_LOGDUMP8(x, y)
#define RX_STATE_LOGDUMP8(x, y)
#define RX_EVENT_LOGDUMP8(x, y)
#define RX_TRACE_LOGDUMP8(x, y)
#define RX_LOUD_LOGDUMP8(x, y)
#define RX_TEMP_LOGDUMP8(x, y)

#define RFTEST_ERROR_LOGFUNC(_Fmt ...)
#define RFTEST_WARN_LOGFUNC(_Fmt ...)
#define RFTEST_INFO_LOGFUNC(_Fmt ...)
#define RFTEST_STATE_LOGFUNC(_Fmt ...)
#define RFTEST_EVENT_LOGFUNC(_Fmt ...)
#define RFTEST_TRACE_LOGFUNC(_Fmt ...)
#define RFTEST_LOUD_LOGFUNC(_Fmt ...)
#define RFTEST_TEMP_LOGFUNC(_Fmt ...)

#define RFTEST_ERROR_LOGDUMP8(x, y)
#define RFTEST_WARN_LOGDUMP8(x, y)
#define RFTEST_INFO_LOGDUMP8(x, y)
#define RFTEST_STATE_LOGDUMP8(x, y)
#define RFTEST_EVENT_LOGDUMP8(x, y)
#define RFTEST_TRACE_LOGDUMP8(x, y)
#define RFTEST_LOUD_LOGDUMP8(x, y)
#define RFTEST_TEMP_LOGDUMP8(x, y)

#define EMU_ERROR_LOGFUNC(_Fmt ...)
#define EMU_WARN_LOGFUNC(_Fmt ...)
#define EMU_INFO_LOGFUNC(_Fmt ...)
#define EMU_STATE_LOGFUNC(_Fmt ...)
#define EMU_EVENT_LOGFUNC(_Fmt ...)
#define EMU_TRACE_LOGFUNC(_Fmt ...)
#define EMU_LOUD_LOGFUNC(_Fmt ...)
#define EMU_TEMP_LOGFUNC(_Fmt ...)

#define EMU_ERROR_LOGDUMP8(x, y)
#define EMU_WARN_LOGDUMP8(x, y)
#define EMU_INFO_LOGDUMP8(x, y)
#define EMU_STATE_LOGDUMP8(x, y)
#define EMU_EVENT_LOGDUMP8(x, y)
#define EMU_TRACE_LOGDUMP8(x, y)
#define EMU_LOUD_LOGDUMP8(x, y)
#define EMU_TEMP_LOGDUMP8(x, y)

#define HEM_ERROR_LOGFUNC(_Fmt ...)
#define HEM_WARN_LOGFUNC(_Fmt ...)
#define HEM_INFO_LOGFUNC(_Fmt ...)
#define HEM_STATE_LOGFUNC(_Fmt ...)
#define HEM_EVENT_LOGFUNC(_Fmt ...)
#define HEM_TRACE_LOGFUNC(_Fmt ...)
#define HEM_LOUD_LOGFUNC(_Fmt ...)
#define HEM_TEMP_LOGFUNC(_Fmt ...)

#define HEM_ERROR_LOGDUMP8(x, y)
#define HEM_WARN_LOGDUMP8(x, y)
#define HEM_INFO_LOGDUMP8(x, y)
#define HEM_STATE_LOGDUMP8(x, y)
#define HEM_EVENT_LOGDUMP8(x, y)
#define HEM_TRACE_LOGDUMP8(x, y)
#define HEM_LOUD_LOGDUMP8(x, y)
#define HEM_TEMP_LOGDUMP8(x, y)

#define RLM_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define RLM_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, _Fmt)
#define RLM_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, _Fmt)
#define RLM_STATE_LOGFUNC(_Fmt ...)
#define RLM_EVENT_LOGFUNC(_Fmt ...)
#define RLM_TRACE_LOGFUNC(_Fmt ...)
#define RLM_LOUD_LOGFUNC(_Fmt ...)
#define RLM_TEMP_LOGFUNC(_Fmt ...)

#define RLM_ERROR_LOGDUMP8(x, y)
#define RLM_WARN_LOGDUMP8(x, y)
#define RLM_INFO_LOGDUMP8(x, y)
#define RLM_STATE_LOGDUMP8(x, y)
#define RLM_EVENT_LOGDUMP8(x, y)
#define RLM_TRACE_LOGDUMP8(x, y)
#define RLM_LOUD_LOGDUMP8(x, y)
#define RLM_TEMP_LOGDUMP8(x, y)

#define MEM_ERROR_LOGFUNC(_Fmt ...)
#define MEM_WARN_LOGFUNC(_Fmt ...)
#define MEM_INFO_LOGFUNC(_Fmt ...)
#define MEM_STATE_LOGFUNC(_Fmt ...)
#define MEM_EVENT_LOGFUNC(_Fmt ...)
#define MEM_TRACE_LOGFUNC(_Fmt ...)
#define MEM_LOUD_LOGFUNC(_Fmt ...)
#define MEM_TEMP_LOGFUNC(_Fmt ...)

#define MEM_ERROR_LOGDUMP8(x, y)
#define MEM_WARN_LOGDUMP8(x, y)
#define MEM_INFO_LOGDUMP8(x, y)
#define MEM_STATE_LOGDUMP8(x, y)
#define MEM_EVENT_LOGDUMP8(x, y)
#define MEM_TRACE_LOGDUMP8(x, y)
#define MEM_LOUD_LOGDUMP8(x, y)
#define MEM_TEMP_LOGDUMP8(x, y)

#define CNM_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define CNM_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, _Fmt)
#define CNM_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, _Fmt)
#define CNM_STATE_LOGFUNC(_Fmt ...)
#define CNM_EVENT_LOGFUNC(_Fmt ...)
#define CNM_TRACE_LOGFUNC(_Fmt ...)
#define CNM_LOUD_LOGFUNC(_Fmt ...)
#define CNM_TEMP_LOGFUNC(_Fmt ...)

#define CNM_ERROR_LOGDUMP8(x, y)
#define CNM_WARN_LOGDUMP8(x, y)
#define CNM_INFO_LOGDUMP8(x, y)
#define CNM_STATE_LOGDUMP8(x, y)
#define CNM_EVENT_LOGDUMP8(x, y)
#define CNM_TRACE_LOGDUMP8(x, y)
#define CNM_LOUD_LOGDUMP8(x, y)
#define CNM_TEMP_LOGDUMP8(x, y)

#define RSN_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define RSN_WARN_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, _Fmt)
#define RSN_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, _Fmt)
#define RSN_STATE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define RSN_EVENT_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define RSN_TRACE_LOGFUNC(_Fmt ...)
#define RSN_LOUD_LOGFUNC(_Fmt ...)
#define RSN_TEMP_LOGFUNC(_Fmt ...)

#define RSN_ERROR_LOGDUMP8(x, y)
#define RSN_WARN_LOGDUMP8(x, y)
#define RSN_INFO_LOGDUMP8(x, y)
#define RSN_STATE_LOGDUMP8(x, y)
#define RSN_EVENT_LOGDUMP8(x, y)
#define RSN_TRACE_LOGDUMP8(x, y)
#define RSN_LOUD_LOGDUMP8(x, y)
#define RSN_TEMP_LOGDUMP8(x, y)

#define BSS_ERROR_LOGFUNC(_Fmt ...)
#define BSS_WARN_LOGFUNC(_Fmt ...)
#define BSS_INFO_LOGFUNC(_Fmt ...)
#define BSS_STATE_LOGFUNC(_Fmt ...)
#define BSS_EVENT_LOGFUNC(_Fmt ...)
#define BSS_TRACE_LOGFUNC(_Fmt ...)
#define BSS_LOUD_LOGFUNC(_Fmt ...)
#define BSS_TEMP_LOGFUNC(_Fmt ...)

#define BSS_ERROR_LOGDUMP8(x, y)
#define BSS_WARN_LOGDUMP8(x, y)
#define BSS_INFO_LOGDUMP8(x, y)
#define BSS_STATE_LOGDUMP8(x, y)
#define BSS_EVENT_LOGDUMP8(x, y)
#define BSS_TRACE_LOGDUMP8(x, y)
#define BSS_LOUD_LOGDUMP8(x, y)
#define BSS_TEMP_LOGDUMP8(x, y)

#define SCN_ERROR_LOGFUNC(_Fmt ...)
#define SCN_WARN_LOGFUNC(_Fmt ...)
#define SCN_INFO_LOGFUNC(_Fmt ...)
#define SCN_STATE_LOGFUNC(_Fmt ...)
#define SCN_EVENT_LOGFUNC(_Fmt ...)
#define SCN_TRACE_LOGFUNC(_Fmt ...)
#define SCN_LOUD_LOGFUNC(_Fmt ...)
#define SCN_TEMP_LOGFUNC(_Fmt ...)

#define SCN_ERROR_LOGDUMP8(x, y)
#define SCN_WARN_LOGDUMP8(x, y)
#define SCN_INFO_LOGDUMP8(x, y)
#define SCN_STATE_LOGDUMP8(x, y)
#define SCN_EVENT_LOGDUMP8(x, y)
#define SCN_TRACE_LOGDUMP8(x, y)
#define SCN_LOUD_LOGDUMP8(x, y)
#define SCN_TEMP_LOGDUMP8(x, y)

#define SAA_ERROR_LOGFUNC(_Fmt ...)
#define SAA_WARN_LOGFUNC(_Fmt ...)
#define SAA_INFO_LOGFUNC(_Fmt ...)
#define SAA_STATE_LOGFUNC(_Fmt ...)
#define SAA_EVENT_LOGFUNC(_Fmt ...)
#define SAA_TRACE_LOGFUNC(_Fmt ...)
#define SAA_LOUD_LOGFUNC(_Fmt ...)
#define SAA_TEMP_LOGFUNC(_Fmt ...)

#define SAA_ERROR_LOGDUMP8(x, y)
#define SAA_WARN_LOGDUMP8(x, y)
#define SAA_INFO_LOGDUMP8(x, y)
#define SAA_STATE_LOGDUMP8(x, y)
#define SAA_EVENT_LOGDUMP8(x, y)
#define SAA_TRACE_LOGDUMP8(x, y)
#define SAA_LOUD_LOGDUMP8(x, y)
#define SAA_TEMP_LOGDUMP8(x, y)

#define P2P_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define P2P_WARN_LOGFUNC(_Fmt ...)  xlog_printk(ANDROID_LOG_WARN, \
XLOG_TAG, \
_Fmt)
#define P2P_INFO_LOGFUNC(_Fmt ...)  xlog_printk(ANDROID_LOG_INFO, \
XLOG_TAG, \
_Fmt)
#define P2P_STATE_LOGFUNC(_Fmt ...)
#define P2P_EVENT_LOGFUNC(_Fmt ...)
#define P2P_TRACE_LOGFUNC(_Fmt ...)
#define P2P_LOUD_LOGFUNC(_Fmt ...)
#define P2P_TEMP_LOGFUNC(_Fmt ...)

#define P2P_ERROR_LOGDUMP8(x, y)
#define P2P_WARN_LOGDUMP8(x, y)
#define P2P_INFO_LOGDUMP8(x, y)
#define P2P_STATE_LOGDUMP8(x, y)
#define P2P_EVENT_LOGDUMP8(x, y)
#define P2P_TRACE_LOGDUMP8(x, y)
#define P2P_LOUD_LOGDUMP8(x, y)
#define P2P_TEMP_LOGDUMP8(x, y)

#define QM_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define QM_WARN_LOGFUNC(_Fmt ...)  xlog_printk(ANDROID_LOG_WARN, XLOG_TAG, _Fmt)
#define QM_INFO_LOGFUNC(_Fmt ...)  xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, _Fmt)
#define QM_STATE_LOGFUNC(_Fmt ...)
#define QM_EVENT_LOGFUNC(_Fmt ...)
#define QM_TRACE_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_DEBUG, \
XLOG_TAG, \
_Fmt)
#define QM_LOUD_LOGFUNC(_Fmt ...)
#define QM_TEMP_LOGFUNC(_Fmt ...)

#define QM_ERROR_LOGDUMP8(x, y)
#define QM_WARN_LOGDUMP8(x, y)
#define QM_INFO_LOGDUMP8(x, y)
#define QM_STATE_LOGDUMP8(x, y)
#define QM_EVENT_LOGDUMP8(x, y)
#define QM_TRACE_LOGDUMP8(x, y)
#define QM_LOUD_LOGDUMP8(x, y)
#define QM_TEMP_LOGDUMP8(x, y)

#define SEC_ERROR_LOGFUNC(_Fmt ...)
#define SEC_WARN_LOGFUNC(_Fmt ...)
#define SEC_INFO_LOGFUNC(_Fmt ...)
#define SEC_STATE_LOGFUNC(_Fmt ...)
#define SEC_EVENT_LOGFUNC(_Fmt ...)
#define SEC_TRACE_LOGFUNC(_Fmt ...)
#define SEC_LOUD_LOGFUNC(_Fmt ...)
#define SEC_TEMP_LOGFUNC(_Fmt ...)

#define SEC_ERROR_LOGDUMP8(x, y)
#define SEC_WARN_LOGDUMP8(x, y)
#define SEC_INFO_LOGDUMP8(x, y)
#define SEC_STATE_LOGDUMP8(x, y)
#define SEC_EVENT_LOGDUMP8(x, y)
#define SEC_TRACE_LOGDUMP8(x, y)
#define SEC_LOUD_LOGDUMP8(x, y)
#define SEC_TEMP_LOGDUMP8(x, y)

#define BOW_ERROR_LOGFUNC(_Fmt ...)
#define BOW_WARN_LOGFUNC(_Fmt ...)
#define BOW_INFO_LOGFUNC(_Fmt ...)
#define BOW_STATE_LOGFUNC(_Fmt ...)
#define BOW_EVENT_LOGFUNC(_Fmt ...)
#define BOW_TRACE_LOGFUNC(_Fmt ...)
#define BOW_LOUD_LOGFUNC(_Fmt ...)
#define BOW_TEMP_LOGFUNC(_Fmt ...)

#define BOW_ERROR_LOGDUMP8(x, y)
#define BOW_WARN_LOGDUMP8(x, y)
#define BOW_INFO_LOGDUMP8(x, y)
#define BOW_STATE_LOGDUMP8(x, y)
#define BOW_EVENT_LOGDUMP8(x, y)
#define BOW_TRACE_LOGDUMP8(x, y)
#define BOW_LOUD_LOGDUMP8(x, y)
#define BOW_TEMP_LOGDUMP8(x, y)

#define HAL_ERROR_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_ERROR, \
XLOG_TAG, \
_Fmt)
#define HAL_WARN_LOGFUNC(_Fmt ...)
#define HAL_INFO_LOGFUNC(_Fmt ...)
#define HAL_STATE_LOGFUNC(_Fmt ...)
#define HAL_EVENT_LOGFUNC(_Fmt ...)
#define HAL_TRACE_LOGFUNC(_Fmt ...)
#define HAL_LOUD_LOGFUNC(_Fmt ...)
#define HAL_TEMP_LOGFUNC(_Fmt ...)

#define HAL_ERROR_LOGDUMP8(x, y)
#define HAL_WARN_LOGDUMP8(x, y)
#define HAL_INFO_LOGDUMP8(x, y)
#define HAL_STATE_LOGDUMP8(x, y)
#define HAL_EVENT_LOGDUMP8(x, y)
#define HAL_TRACE_LOGDUMP8(x, y)
#define HAL_LOUD_LOGDUMP8(x, y)
#define HAL_TEMP_LOGDUMP8(x, y)

#define WAPI_ERROR_LOGFUNC(_Fmt ...)
#define WAPI_WARN_LOGFUNC(_Fmt ...)
#define WAPI_INFO_LOGFUNC(_Fmt ...)
#define WAPI_STATE_LOGFUNC(_Fmt ...)
#define WAPI_EVENT_LOGFUNC(_Fmt ...)
#define WAPI_TRACE_LOGFUNC(_Fmt ...)
#define WAPI_LOUD_LOGFUNC(_Fmt ...)
#define WAPI_TEMP_LOGFUNC(_Fmt ...)

#define WAPI_ERROR_LOGDUMP8(x, y)
#define WAPI_WARN_LOGDUMP8(x, y)
#define WAPI_INFO_LOGDUMP8(x, y)
#define WAPI_STATE_LOGDUMP8(x, y)
#define WAPI_EVENT_LOGDUMP8(x, y)
#define WAPI_TRACE_LOGDUMP8(x, y)
#define WAPI_LOUD_LOGDUMP8(x, y)
#define WAPI_TEMP_LOGDUMP8(x, y)

#define SW1_ERROR_LOGFUNC(_Fmt ...)
#define SW1_WARN_LOGFUNC(_Fmt ...)
#define SW1_INFO_LOGFUNC(_Fmt ...)
#define SW1_STATE_LOGFUNC(_Fmt ...)
#define SW1_EVENT_LOGFUNC(_Fmt ...)
#define SW1_TRACE_LOGFUNC(_Fmt ...)
#define SW1_LOUD_LOGFUNC(_Fmt ...)
#define SW1_TEMP_LOGFUNC(_Fmt ...)

#define SW1_ERROR_LOGDUMP8(x, y)
#define SW1_WARN_LOGDUMP8(x, y)
#define SW1_INFO_LOGDUMP8(x, y)
#define SW1_STATE_LOGDUMP8(x, y)
#define SW1_EVENT_LOGDUMP8(x, y)
#define SW1_TRACE_LOGDUMP8(x, y)
#define SW1_LOUD_LOGDUMP8(x, y)
#define SW1_TEMP_LOGDUMP8(x, y)

#define SW2_ERROR_LOGFUNC(_Fmt ...)
#define SW2_WARN_LOGFUNC(_Fmt ...)
#define SW2_INFO_LOGFUNC(_Fmt ...)
#define SW2_STATE_LOGFUNC(_Fmt ...)
#define SW2_EVENT_LOGFUNC(_Fmt ...)
#define SW2_TRACE_LOGFUNC(_Fmt ...)
#define SW2_LOUD_LOGFUNC(_Fmt ...)
#define SW2_TEMP_LOGFUNC(_Fmt ...)

#define SW2_ERROR_LOGDUMP8(x, y)
#define SW2_WARN_LOGDUMP8(x, y)
#define SW2_INFO_LOGDUMP8(x, y)
#define SW2_STATE_LOGDUMP8(x, y)
#define SW2_EVENT_LOGDUMP8(x, y)
#define SW2_TRACE_LOGDUMP8(x, y)
#define SW2_LOUD_LOGDUMP8(x, y)
#define SW2_TEMP_LOGDUMP8(x, y)

#define SW3_ERROR_LOGFUNC(_Fmt ...)
#define SW3_WARN_LOGFUNC(_Fmt ...)
#define SW3_INFO_LOGFUNC(_Fmt ...)
#define SW3_STATE_LOGFUNC(_Fmt ...)
#define SW3_EVENT_LOGFUNC(_Fmt ...)
#define SW3_TRACE_LOGFUNC(_Fmt ...)
#define SW3_LOUD_LOGFUNC(_Fmt ...)
#define SW3_TEMP_LOGFUNC(_Fmt ...)

#define SW3_ERROR_LOGDUMP8(x, y)
#define SW3_WARN_LOGDUMP8(x, y)
#define SW3_INFO_LOGDUMP8(x, y)
#define SW3_STATE_LOGDUMP8(x, y)
#define SW3_EVENT_LOGDUMP8(x, y)
#define SW3_TRACE_LOGDUMP8(x, y)
#define SW3_LOUD_LOGDUMP8(x, y)
#define SW3_TEMP_LOGDUMP8(x, y)

#define SW4_ERROR_LOGFUNC(_Fmt ...)
#define SW4_WARN_LOGFUNC(_Fmt ...)
#define SW4_INFO_LOGFUNC(_Fmt ...) xlog_printk(ANDROID_LOG_INFO, XLOG_TAG, _Fmt)
#define SW4_STATE_LOGFUNC(_Fmt ...)
#define SW4_EVENT_LOGFUNC(_Fmt ...)
#define SW4_TRACE_LOGFUNC(_Fmt ...)
#define SW4_LOUD_LOGFUNC(_Fmt ...)
#define SW4_TEMP_LOGFUNC(_Fmt ...)

#define SW4_ERROR_LOGDUMP8(x, y)
#define SW4_WARN_LOGDUMP8(x, y)
#define SW4_INFO_LOGDUMP8(x, y)
#define SW4_STATE_LOGDUMP8(x, y)
#define SW4_EVENT_LOGDUMP8(x, y)
#define SW4_TRACE_LOGDUMP8(x, y) dumpMemory8(ANDROID_LOG_DEBUG, x, y)
#define SW4_LOUD_LOGDUMP8(x, y)
#define SW4_TEMP_LOGDUMP8(x, y)
#else
#define AIS_ERROR_LOGFUNC(_Fmt ...)
#define AIS_WARN_LOGFUNC(_Fmt ...)
#define AIS_INFO_LOGFUNC(_Fmt ...)
#define AIS_STATE_LOGFUNC(_Fmt ...)
#define AIS_EVENT_LOGFUNC(_Fmt ...)
#define AIS_TRACE_LOGFUNC(_Fmt ...)
#define AIS_LOUD_LOGFUNC(_Fmt ...)
#define AIS_TEMP_LOGFUNC(_Fmt ...)

#define INTR_ERROR_LOGFUNC(_Fmt ...)
#define INTR_WARN_LOGFUNC(_Fmt ...)
#define INTR_INFO_LOGFUNC(_Fmt ...)
#define INTR_STATE_LOGFUNC(_Fmt ...)
#define INTR_EVENT_LOGFUNC(_Fmt ...)
#define INTR_TRACE_LOGFUNC(_Fmt ...)
#define INTR_LOUD_LOGFUNC(_Fmt ...)
#define INTR_TEMP_LOGFUNC(_Fmt ...)

#define INIT_ERROR_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define INIT_WARN_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define INIT_INFO_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define INIT_STATE_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define INIT_EVENT_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define INIT_TRACE_LOGFUNC(_Fmt ...)
#define INIT_LOUD_LOGFUNC(_Fmt ...)
#define INIT_TEMP_LOGFUNC(_Fmt ...)

#define AAA_ERROR_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define AAA_WARN_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define AAA_INFO_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define AAA_STATE_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define AAA_EVENT_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define AAA_TRACE_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define AAA_LOUD_LOGFUNC(_Fmt ...)
#define AAA_TEMP_LOGFUNC(_Fmt ...)

#define ROAMING_ERROR_LOGFUNC(_Fmt ...)
#define ROAMING_WARN_LOGFUNC(_Fmt ...)
#define ROAMING_INFO_LOGFUNC(_Fmt ...)
#define ROAMING_STATE_LOGFUNC(_Fmt ...)
#define ROAMING_EVENT_LOGFUNC(_Fmt ...)
#define ROAMING_TRACE_LOGFUNC(_Fmt ...)
#define ROAMING_LOUD_LOGFUNC(_Fmt ...)
#define ROAMING_TEMP_LOGFUNC(_Fmt ...)

#define REQ_ERROR_LOGFUNC(_Fmt ...)
#define REQ_WARN_LOGFUNC(_Fmt ...)
#define REQ_INFO_LOGFUNC(_Fmt ...)
#define REQ_STATE_LOGFUNC(_Fmt ...)
#define REQ_EVENT_LOGFUNC(_Fmt ...)
#define REQ_TRACE_LOGFUNC(_Fmt ...)
#define REQ_LOUD_LOGFUNC(_Fmt ...)
#define REQ_TEMP_LOGFUNC(_Fmt ...)

#define TX_ERROR_LOGFUNC(_Fmt ...)
#define TX_WARN_LOGFUNC(_Fmt ...)
#define TX_INFO_LOGFUNC(_Fmt ...)
#define TX_STATE_LOGFUNC(_Fmt ...)
#define TX_EVENT_LOGFUNC(_Fmt ...)
#define TX_TRACE_LOGFUNC(_Fmt ...)
#define TX_LOUD_LOGFUNC(_Fmt ...)
#define TX_TEMP_LOGFUNC(_Fmt ...)

#define RX_ERROR_LOGFUNC(_Fmt ...)
#define RX_WARN_LOGFUNC(_Fmt ...)
#define RX_INFO_LOGFUNC(_Fmt ...)
#define RX_STATE_LOGFUNC(_Fmt ...)
#define RX_EVENT_LOGFUNC(_Fmt ...)
#define RX_TRACE_LOGFUNC(_Fmt ...)
#define RX_LOUD_LOGFUNC(_Fmt ...)
#define RX_TEMP_LOGFUNC(_Fmt ...)

#define RFTEST_ERROR_LOGFUNC(_Fmt ...)
#define RFTEST_WARN_LOGFUNC(_Fmt ...)
#define RFTEST_INFO_LOGFUNC(_Fmt ...)
#define RFTEST_STATE_LOGFUNC(_Fmt ...)
#define RFTEST_EVENT_LOGFUNC(_Fmt ...)
#define RFTEST_TRACE_LOGFUNC(_Fmt ...)
#define RFTEST_LOUD_LOGFUNC(_Fmt ...)
#define RFTEST_TEMP_LOGFUNC(_Fmt ...)

#define EMU_ERROR_LOGFUNC(_Fmt ...)
#define EMU_WARN_LOGFUNC(_Fmt ...)
#define EMU_INFO_LOGFUNC(_Fmt ...)
#define EMU_STATE_LOGFUNC(_Fmt ...)
#define EMU_EVENT_LOGFUNC(_Fmt ...)
#define EMU_TRACE_LOGFUNC(_Fmt ...)
#define EMU_LOUD_LOGFUNC(_Fmt ...)
#define EMU_TEMP_LOGFUNC(_Fmt ...)

#define HEM_ERROR_LOGFUNC(_Fmt ...)
#define HEM_WARN_LOGFUNC(_Fmt ...)
#define HEM_INFO_LOGFUNC(_Fmt ...)
#define HEM_STATE_LOGFUNC(_Fmt ...)
#define HEM_EVENT_LOGFUNC(_Fmt ...)
#define HEM_TRACE_LOGFUNC(_Fmt ...)
#define HEM_LOUD_LOGFUNC(_Fmt ...)
#define HEM_TEMP_LOGFUNC(_Fmt ...)

#define RLM_ERROR_LOGFUNC(_Fmt ...)
#define RLM_WARN_LOGFUNC(_Fmt ...)
#define RLM_INFO_LOGFUNC(_Fmt ...)
#define RLM_STATE_LOGFUNC(_Fmt ...)
#define RLM_EVENT_LOGFUNC(_Fmt ...)
#define RLM_TRACE_LOGFUNC(_Fmt ...)
#define RLM_LOUD_LOGFUNC(_Fmt ...)
#define RLM_TEMP_LOGFUNC(_Fmt ...)

#define MEM_ERROR_LOGFUNC(_Fmt ...)
#define MEM_WARN_LOGFUNC(_Fmt ...)
#define MEM_INFO_LOGFUNC(_Fmt ...)
#define MEM_STATE_LOGFUNC(_Fmt ...)
#define MEM_EVENT_LOGFUNC(_Fmt ...)
#define MEM_TRACE_LOGFUNC(_Fmt ...)
#define MEM_LOUD_LOGFUNC(_Fmt ...)
#define MEM_TEMP_LOGFUNC(_Fmt ...)

#define CNM_ERROR_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define CNM_WARN_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define CNM_INFO_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define CNM_STATE_LOGFUNC(_Fmt ...)
#define CNM_EVENT_LOGFUNC(_Fmt ...)
#define CNM_TRACE_LOGFUNC(_Fmt ...)
#define CNM_LOUD_LOGFUNC(_Fmt ...)
#define CNM_TEMP_LOGFUNC(_Fmt ...)

#define RSN_ERROR_LOGFUNC(_Fmt ...)
#define RSN_WARN_LOGFUNC(_Fmt ...)
#define RSN_INFO_LOGFUNC(_Fmt ...)
#define RSN_STATE_LOGFUNC(_Fmt ...)
#define RSN_EVENT_LOGFUNC(_Fmt ...)
#define RSN_TRACE_LOGFUNC(_Fmt ...)
#define RSN_LOUD_LOGFUNC(_Fmt ...)
#define RSN_TEMP_LOGFUNC(_Fmt ...)

#define BSS_ERROR_LOGFUNC(_Fmt ...)
#define BSS_WARN_LOGFUNC(_Fmt ...)
#define BSS_INFO_LOGFUNC(_Fmt ...)
#define BSS_STATE_LOGFUNC(_Fmt ...)
#define BSS_EVENT_LOGFUNC(_Fmt ...)
#define BSS_TRACE_LOGFUNC(_Fmt ...)
#define BSS_LOUD_LOGFUNC(_Fmt ...)
#define BSS_TEMP_LOGFUNC(_Fmt ...)

#define SCN_ERROR_LOGFUNC(_Fmt ...)
#define SCN_WARN_LOGFUNC(_Fmt ...)
#define SCN_INFO_LOGFUNC(_Fmt ...)
#define SCN_STATE_LOGFUNC(_Fmt ...)
#define SCN_EVENT_LOGFUNC(_Fmt ...)
#define SCN_TRACE_LOGFUNC(_Fmt ...)
#define SCN_LOUD_LOGFUNC(_Fmt ...)
#define SCN_TEMP_LOGFUNC(_Fmt ...)

#define SAA_ERROR_LOGFUNC(_Fmt ...)
#define SAA_WARN_LOGFUNC(_Fmt ...)
#define SAA_INFO_LOGFUNC(_Fmt ...)
#define SAA_STATE_LOGFUNC(_Fmt ...)
#define SAA_EVENT_LOGFUNC(_Fmt ...)
#define SAA_TRACE_LOGFUNC(_Fmt ...)
#define SAA_LOUD_LOGFUNC(_Fmt ...)
#define SAA_TEMP_LOGFUNC(_Fmt ...)

#define P2P_ERROR_LOGFUNC(_Fmt ...)
#define P2P_WARN_LOGFUNC(_Fmt ...)
#define P2P_INFO_LOGFUNC(_Fmt ...)
#define P2P_STATE_LOGFUNC(_Fmt ...)
#define P2P_EVENT_LOGFUNC(_Fmt ...)
#define P2P_TRACE_LOGFUNC(_Fmt ...)
#define P2P_LOUD_LOGFUNC(_Fmt ...)
#define P2P_TEMP_LOGFUNC(_Fmt ...)

#define QM_ERROR_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define QM_WARN_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define QM_INFO_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define QM_STATE_LOGFUNC(_Fmt ...)
#define QM_EVENT_LOGFUNC(_Fmt ...)
#define QM_TRACE_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define QM_LOUD_LOGFUNC(_Fmt ...)
#define QM_TEMP_LOGFUNC(_Fmt ...)

#define SEC_ERROR_LOGFUNC(_Fmt ...)
#define SEC_WARN_LOGFUNC(_Fmt ...)
#define SEC_INFO_LOGFUNC(_Fmt ...)
#define SEC_STATE_LOGFUNC(_Fmt ...)
#define SEC_EVENT_LOGFUNC(_Fmt ...)
#define SEC_TRACE_LOGFUNC(_Fmt ...)
#define SEC_LOUD_LOGFUNC(_Fmt ...)
#define SEC_TEMP_LOGFUNC(_Fmt ...)

#define BOW_ERROR_LOGFUNC(_Fmt ...)
#define BOW_WARN_LOGFUNC(_Fmt ...)
#define BOW_INFO_LOGFUNC(_Fmt ...)
#define BOW_STATE_LOGFUNC(_Fmt ...)
#define BOW_EVENT_LOGFUNC(_Fmt ...)
#define BOW_TRACE_LOGFUNC(_Fmt ...)
#define BOW_LOUD_LOGFUNC(_Fmt ...)
#define BOW_TEMP_LOGFUNC(_Fmt ...)

#define HAL_ERROR_LOGFUNC(_Fmt ...) kalPrint(_Fmt)
#define HAL_WARN_LOGFUNC(_Fmt ...)
#define HAL_INFO_LOGFUNC(_Fmt ...)
#define HAL_STATE_LOGFUNC(_Fmt ...)
#define HAL_EVENT_LOGFUNC(_Fmt ...)
#define HAL_TRACE_LOGFUNC(_Fmt ...)
#define HAL_LOUD_LOGFUNC(_Fmt ...)
#define HAL_TEMP_LOGFUNC(_Fmt ...)

#define WAPI_ERROR_LOGFUNC(_Fmt ...)
#define WAPI_WARN_LOGFUNC(_Fmt ...)
#define WAPI_INFO_LOGFUNC(_Fmt ...)
#define WAPI_STATE_LOGFUNC(_Fmt ...)
#define WAPI_EVENT_LOGFUNC(_Fmt ...)
#define WAPI_TRACE_LOGFUNC(_Fmt ...)
#define WAPI_LOUD_LOGFUNC(_Fmt ...)
#define WAPI_TEMP_LOGFUNC(_Fmt ...)

#define SW1_ERROR_LOGFUNC(_Fmt ...)
#define SW1_WARN_LOGFUNC(_Fmt ...)
#define SW1_INFO_LOGFUNC(_Fmt ...)
#define SW1_STATE_LOGFUNC(_Fmt ...)
#define SW1_EVENT_LOGFUNC(_Fmt ...)
#define SW1_TRACE_LOGFUNC(_Fmt ...)
#define SW1_LOUD_LOGFUNC(_Fmt ...)
#define SW1_TEMP_LOGFUNC(_Fmt ...)

#define SW2_ERROR_LOGFUNC(_Fmt ...)
#define SW2_WARN_LOGFUNC(_Fmt ...)
#define SW2_INFO_LOGFUNC(_Fmt ...)
#define SW2_STATE_LOGFUNC(_Fmt ...)
#define SW2_EVENT_LOGFUNC(_Fmt ...)
#define SW2_TRACE_LOGFUNC(_Fmt ...)
#define SW2_LOUD_LOGFUNC(_Fmt ...)
#define SW2_TEMP_LOGFUNC(_Fmt ...)

#define SW3_ERROR_LOGFUNC(_Fmt ...)
#define SW3_WARN_LOGFUNC(_Fmt ...)
#define SW3_INFO_LOGFUNC(_Fmt ...)
#define SW3_STATE_LOGFUNC(_Fmt ...)
#define SW3_EVENT_LOGFUNC(_Fmt ...)
#define SW3_TRACE_LOGFUNC(_Fmt ...)
#define SW3_LOUD_LOGFUNC(_Fmt ...)
#define SW3_TEMP_LOGFUNC(_Fmt ...)

#define SW4_ERROR_LOGFUNC(_Fmt ...)
#define SW4_WARN_LOGFUNC(_Fmt ...)
#define SW4_INFO_LOGFUNC(_Fmt ...)
#define SW4_STATE_LOGFUNC(_Fmt ...)
#define SW4_EVENT_LOGFUNC(_Fmt ...)
#define SW4_TRACE_LOGFUNC(_Fmt ...)
#define SW4_LOUD_LOGFUNC(_Fmt ...)
#define SW4_TEMP_LOGFUNC(_Fmt ...)
#endif
#endif

#define kalBreakPoint()	\
do { \
BUG(); \
panic("Oops"); \
} while (0)

#if CFG_ENABLE_AEE_MSG
#define kalSendAeeException                         aee_kernel_exception
#define kalSendAeeWarning                           aee_kernel_warning
#define kalSendAeeReminding                         aee_kernel_reminding
#else
#define kalSendAeeException(_module, _desc, ...)
#define kalSendAeeWarning(_module, _desc, ...)
#define kalSendAeeReminding(_module, _desc, ...)
#endif

#define PRINTF_ARG(...)                             __VA_ARGS__
#define SPRINTF(buf, arg)                           { buf += \
sprintf( \
(char *)(	\
buf), \
PRINTF_ARG \
arg); }

#define USEC_TO_SYSTIME(_usec)      ((_usec) / USEC_PER_MSEC)
#define MSEC_TO_SYSTIME(_msec)      (_msec)

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/* Routines in gl_kal.c                                                       */
/*----------------------------------------------------------------------------*/
VOID
kalAcquireSpinLock(IN P_GLUE_INFO_T prGlueInfo,
		   IN ENUM_SPIN_LOCK_CATEGORY_E rLockCategory,
		   OUT PUINT_32 pu4Flags);

VOID
kalReleaseSpinLock(IN P_GLUE_INFO_T prGlueInfo,
		   IN ENUM_SPIN_LOCK_CATEGORY_E rLockCategory,
		   IN UINT_32 u4Flags);

VOID kalUpdateMACAddress(IN P_GLUE_INFO_T prGlueInfo, IN PUINT_8 pucMacAddr);

VOID kalPacketFree(IN P_GLUE_INFO_T prGlueInfo, IN PVOID pvPacket);

PVOID
kalPacketAlloc(IN P_GLUE_INFO_T prGlueInfo,
	       IN UINT_32 u4Size, OUT PUINT_8 * ppucData);

VOID kalOsTimerInitialize(IN P_GLUE_INFO_T prGlueInfo, IN PVOID prTimerHandler);

BOOL kalSetTimer(IN P_GLUE_INFO_T prGlueInfo, IN OS_SYSTIME rInterval);

WLAN_STATUS
kalProcessRxPacket(IN P_GLUE_INFO_T prGlueInfo,
		   IN PVOID pvPacket,
		   IN PUINT_8 pucPacketStart, IN UINT_32 u4PacketLen,
/* IN PBOOLEAN           pfgIsRetain, */
		   IN BOOLEAN fgIsRetain, IN ENUM_CSUM_RESULT_T aeCSUM[]
    );

WLAN_STATUS
kalRxIndicatePkts(IN P_GLUE_INFO_T prGlueInfo,
		  IN PVOID apvPkts[], IN UINT_8 ucPktNum);

VOID
kalIndicateStatusAndComplete(IN P_GLUE_INFO_T prGlueInfo,
			     IN WLAN_STATUS eStatus,
			     IN PVOID pvBuf, IN UINT_32 u4BufLen);

VOID
kalUpdateReAssocReqInfo(IN P_GLUE_INFO_T prGlueInfo,
			IN PUINT_8 pucFrameBody,
			IN UINT_32 u4FrameBodyLen, IN BOOLEAN fgReassocRequest);

VOID
kalUpdateReAssocRspInfo(IN P_GLUE_INFO_T prGlueInfo,
			IN PUINT_8 pucFrameBody, IN UINT_32 u4FrameBodyLen);

#if CFG_TX_FRAGMENT
BOOLEAN
kalQueryTxPacketHeader(IN P_GLUE_INFO_T prGlueInfo,
		       IN PVOID pvPacket,
		       OUT PUINT_16 pu2EtherTypeLen,
		       OUT PUINT_8 pucEthDestAddr);
#endif /* CFG_TX_FRAGMENT */

VOID
kalSendCompleteAndAwakeQueue(IN P_GLUE_INFO_T prGlueInfo, IN PVOID pvPacket);

#if CFG_TCP_IP_CHKSUM_OFFLOAD
VOID kalQueryTxChksumOffloadParam(IN PVOID pvPacket, OUT PUINT_8 pucFlag);

VOID
kalUpdateRxCSUMOffloadParam(IN PVOID pvPacket, IN ENUM_CSUM_RESULT_T eCSUM[]
    );
#endif /* CFG_TCP_IP_CHKSUM_OFFLOAD */

BOOLEAN
kalRetrieveNetworkAddress(IN P_GLUE_INFO_T prGlueInfo,
			  IN OUT PARAM_MAC_ADDRESS * prMacAddr);

/*----------------------------------------------------------------------------*/
/* Routines in interface - ehpi/sdio.c                                                       */
/*----------------------------------------------------------------------------*/
BOOL
kalDevRegRead(IN P_GLUE_INFO_T prGlueInfo,
	      IN UINT_32 u4Register, OUT PUINT_32 pu4Value);

BOOL
kalDevRegWrite(P_GLUE_INFO_T prGlueInfo,
	       IN UINT_32 u4Register, IN UINT_32 u4Value);

BOOL
kalDevPortRead(IN P_GLUE_INFO_T prGlueInfo,
	       IN UINT_16 u2Port,
	       IN UINT_16 u2Len,
	       OUT PUINT_8 pucBuf, IN UINT_16 u2ValidOutBufSize);

BOOL
kalDevPortWrite(P_GLUE_INFO_T prGlueInfo,
		IN UINT_16 u2Port,
		IN UINT_16 u2Len,
		IN PUINT_8 pucBuf, IN UINT_16 u2ValidInBufSize);

BOOL
kalDevWriteWithSdioCmd52(IN P_GLUE_INFO_T prGlueInfo,
			 IN UINT_32 u4Addr, IN UINT_8 ucData);

#if CFG_SUPPORT_EXT_CONFIG
UINT_32 kalReadExtCfg(IN P_GLUE_INFO_T prGlueInfo);
#endif

BOOL
kalQoSFrameClassifierAndPacketInfo(IN P_GLUE_INFO_T prGlueInfo,
				   IN P_NATIVE_PACKET prPacket,
				   OUT PUINT_8 pucPriorityParam,
				   OUT PUINT_32 pu4PacketLen,
				   OUT PUINT_8 pucEthDestAddr,
				   OUT PBOOLEAN pfgIs1X,
				   OUT PBOOLEAN pfgIsPAL,
				   OUT PUINT_8 pucNetworkType);

inline VOID
kalOidComplete(IN P_GLUE_INFO_T prGlueInfo,
	       IN BOOLEAN fgSetQuery,
	       IN UINT_32 u4SetQueryInfoLen, IN WLAN_STATUS rOidStatus);

WLAN_STATUS
kalIoctl(IN P_GLUE_INFO_T prGlueInfo,
	 IN PFN_OID_HANDLER_FUNC pfnOidHandler,
	 IN PVOID pvInfoBuf,
	 IN UINT_32 u4InfoBufLen,
	 IN BOOL fgRead,
	 IN BOOL fgWaitResp,
	 IN BOOL fgCmd, IN BOOL fgIsP2pOid, OUT PUINT_32 pu4QryInfoLen);

VOID
kalHandleAssocInfo(IN P_GLUE_INFO_T prGlueInfo,
		   IN P_EVENT_ASSOC_INFO prAssocInfo);

#if CFG_ENABLE_FW_DOWNLOAD

PVOID
kalFirmwareImageMapping(IN P_GLUE_INFO_T prGlueInfo,
			OUT PPVOID ppvMapFileBuf, OUT PUINT_32 pu4FileLength);

VOID
kalFirmwareImageUnmapping(IN P_GLUE_INFO_T prGlueInfo,
			  IN PVOID prFwHandle, IN PVOID pvMapFileBuf);
#endif

/*----------------------------------------------------------------------------*/
/* Card Removal Check                                                         */
/*----------------------------------------------------------------------------*/
BOOLEAN kalIsCardRemoved(IN P_GLUE_INFO_T prGlueInfo);

/*----------------------------------------------------------------------------*/
/* TX                                                                         */
/*----------------------------------------------------------------------------*/
VOID kalFlushPendingTxPackets(IN P_GLUE_INFO_T prGlueInfo);

/*----------------------------------------------------------------------------*/
/* Media State Indication                                                     */
/*----------------------------------------------------------------------------*/
ENUM_PARAM_MEDIA_STATE_T kalGetMediaStateIndicated(IN P_GLUE_INFO_T prGlueInfo);

VOID
kalSetMediaStateIndicated(IN P_GLUE_INFO_T prGlueInfo,
			  IN ENUM_PARAM_MEDIA_STATE_T eParamMediaStateIndicate);

/*----------------------------------------------------------------------------*/
/* OID handling                                                               */
/*----------------------------------------------------------------------------*/
VOID kalOidCmdClearance(IN P_GLUE_INFO_T prGlueInfo);

VOID kalOidClearance(IN P_GLUE_INFO_T prGlueInfo);

VOID
kalEnqueueCommand(IN P_GLUE_INFO_T prGlueInfo, IN P_QUE_ENTRY_T prQueueEntry);

#if CFG_ENABLE_BT_OVER_WIFI
/*----------------------------------------------------------------------------*/
/* Bluetooth over Wi-Fi handling                                              */
/*----------------------------------------------------------------------------*/
VOID kalIndicateBOWEvent(IN P_GLUE_INFO_T prGlueInfo, IN P_AMPC_EVENT prEvent);

ENUM_BOW_DEVICE_STATE
kalGetBowState(IN P_GLUE_INFO_T prGlueInfo, IN PARAM_MAC_ADDRESS rPeerAddr);

BOOLEAN
kalSetBowState(IN P_GLUE_INFO_T prGlueInfo,
	       IN ENUM_BOW_DEVICE_STATE eBowState, PARAM_MAC_ADDRESS rPeerAddr);

ENUM_BOW_DEVICE_STATE kalGetBowGlobalState(IN P_GLUE_INFO_T prGlueInfo);

UINT_32 kalGetBowFreqInKHz(IN P_GLUE_INFO_T prGlueInfo);

UINT_8
kalGetBowRole(IN P_GLUE_INFO_T prGlueInfo, IN PARAM_MAC_ADDRESS rPeerAddr);

VOID
kalSetBowRole(IN P_GLUE_INFO_T prGlueInfo,
	      IN UINT_8 ucRole, IN PARAM_MAC_ADDRESS rPeerAddr);

UINT_8 kalGetBowAvailablePhysicalLinkCount(IN P_GLUE_INFO_T prGlueInfo);

#if CFG_BOW_SEPARATE_DATA_PATH
/*----------------------------------------------------------------------------*/
/* Bluetooth over Wi-Fi Net Device Init/Uninit                                */
/*----------------------------------------------------------------------------*/
BOOLEAN kalInitBowDevice(IN P_GLUE_INFO_T prGlueInfo, IN const char *prDevName);

BOOLEAN kalUninitBowDevice(IN P_GLUE_INFO_T prGlueInfo);
#endif /* CFG_BOW_SEPARATE_DATA_PATH */
#endif /* CFG_ENABLE_BT_OVER_WIFI */

/*----------------------------------------------------------------------------*/
/* Firmware Download Handling                                                 */
/*----------------------------------------------------------------------------*/
UINT_32 kalGetFwStartAddress(IN P_GLUE_INFO_T prGlueInfo);

UINT_32 kalGetFwLoadAddress(IN P_GLUE_INFO_T prGlueInfo);

/*----------------------------------------------------------------------------*/
/* Security Frame Clearance                                                   */
/*----------------------------------------------------------------------------*/
VOID kalClearSecurityFrames(IN P_GLUE_INFO_T prGlueInfo);

VOID
kalClearSecurityFramesByNetType(IN P_GLUE_INFO_T prGlueInfo,
				IN ENUM_NETWORK_TYPE_INDEX_T eNetworkTypeIdx);

VOID
kalSecurityFrameSendComplete(IN P_GLUE_INFO_T prGlueInfo,
			     IN PVOID pvPacket, IN WLAN_STATUS rStatus);

/*----------------------------------------------------------------------------*/
/* Management Frame Clearance                                                 */
/*----------------------------------------------------------------------------*/
VOID kalClearMgmtFrames(IN P_GLUE_INFO_T prGlueInfo);

VOID
kalClearMgmtFramesByNetType(IN P_GLUE_INFO_T prGlueInfo,
			    IN ENUM_NETWORK_TYPE_INDEX_T eNetworkTypeIdx);

UINT_32 kalGetTxPendingFrameCount(IN P_GLUE_INFO_T prGlueInfo);

UINT_32 kalGetTxPendingCmdCount(IN P_GLUE_INFO_T prGlueInfo);

BOOLEAN kalSetTimer(IN P_GLUE_INFO_T prGlueInfo, IN UINT_32 u4Interval);

BOOLEAN kalCancelTimer(IN P_GLUE_INFO_T prGlueInfo);

VOID
kalScanDone(IN P_GLUE_INFO_T prGlueInfo,
	    IN ENUM_KAL_NETWORK_TYPE_INDEX_T eNetTypeIdx,
	    IN WLAN_STATUS status);

UINT_32 kalRandomNumber(VOID);

VOID kalTimeoutHandler(unsigned long arg);

VOID kalSetEvent(P_GLUE_INFO_T pr);

/*----------------------------------------------------------------------------*/
/* NVRAM/Registry Service                                                     */
/*----------------------------------------------------------------------------*/
BOOLEAN kalIsConfigurationExist(IN P_GLUE_INFO_T prGlueInfo);

P_REG_INFO_T kalGetConfiguration(IN P_GLUE_INFO_T prGlueInfo);

VOID
kalGetConfigurationVersion(IN P_GLUE_INFO_T prGlueInfo,
			   OUT PUINT_16 pu2Part1CfgOwnVersion,
			   OUT PUINT_16 pu2Part1CfgPeerVersion,
			   OUT PUINT_16 pu2Part2CfgOwnVersion,
			   OUT PUINT_16 pu2Part2CfgPeerVersion);

BOOLEAN
kalCfgDataRead16(IN P_GLUE_INFO_T prGlueInfo,
		 IN UINT_32 u4Offset, OUT PUINT_16 pu2Data);

BOOLEAN
kalCfgDataWrite16(IN P_GLUE_INFO_T prGlueInfo,
		  IN UINT_32 u4Offset, IN UINT_16 u2Data);

/*----------------------------------------------------------------------------*/
/* WSC Connection                                                     */
/*----------------------------------------------------------------------------*/
BOOLEAN kalWSCGetActiveState(IN P_GLUE_INFO_T prGlueInfo);

/*----------------------------------------------------------------------------*/
/* RSSI Updating                                                              */
/*----------------------------------------------------------------------------*/
VOID
kalUpdateRSSI(IN P_GLUE_INFO_T prGlueInfo,
	      IN ENUM_KAL_NETWORK_TYPE_INDEX_T eNetTypeIdx,
	      IN INT_8 cRssi, IN INT_8 cLinkQuality);

/*----------------------------------------------------------------------------*/
/* I/O Buffer Pre-allocation                                                  */
/*----------------------------------------------------------------------------*/
BOOLEAN kalInitIOBuffer(VOID);

VOID kalUninitIOBuffer(VOID);

PVOID kalAllocateIOBuffer(IN UINT_32 u4AllocSize);

VOID kalReleaseIOBuffer(IN PVOID pvAddr, IN UINT_32 u4Size);

VOID
kalGetChannelList(IN P_GLUE_INFO_T prGlueInfo,
		  IN ENUM_BAND_T eSpecificBand,
		  IN UINT_8 ucMaxChannelNum,
		  IN PUINT_8 pucNumOfChannel,
		  IN P_RF_CHANNEL_INFO_T paucChannelList);

BOOL kalIsAPmode(IN P_GLUE_INFO_T prGlueInfo);

#if CFG_SUPPORT_802_11W
/*----------------------------------------------------------------------------*/
/* 802.11W                                                                    */
/*----------------------------------------------------------------------------*/
UINT_32 kalGetMfpSetting(IN P_GLUE_INFO_T prGlueInfo);
#endif

UINT_32
kalWriteToFile(const PUINT_8 pucPath,
	       BOOLEAN fgDoAppend, PUINT_8 pucData, UINT_32 u4Size);

/*----------------------------------------------------------------------------*/
/* NL80211                                                                    */
/*----------------------------------------------------------------------------*/
VOID
kalIndicateBssInfo(IN P_GLUE_INFO_T prGlueInfo,
		   IN PUINT_8 pucFrameBuf,
		   IN UINT_32 u4BufLen,
		   IN UINT_8 ucChannelNum, IN INT_32 i4SignalStrength);

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

int tx_thread(void *data);

#endif /* _GL_KAL_H */
