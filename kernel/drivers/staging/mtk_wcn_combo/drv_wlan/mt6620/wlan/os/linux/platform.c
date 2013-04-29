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
#include <linux/version.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/fs.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12)
#include <linux/uaccess.h>
#endif

#include "gl_os.h"

#ifndef CONFIG_X86
#if defined(CONFIG_HAS_EARLY_SUSPEND)
#include <linux/earlysuspend.h>
#endif
#endif

extern BOOLEAN fgIsUnderEarlierSuspend;

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/
#define WIFI_NVRAM_FILE_NAME   "/data/nvram/APCFG/APRDEB/WIFI"
#define WIFI_NVRAM_CUSTOM_NAME "/data/nvram/APCFG/APRDEB/WIFI_CUSTOM"

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
static int netdev_event(struct notifier_block *nb,
			unsigned long notification, void *ptr)
{
	UINT_8 ip[4] = { 0 };
	UINT_32 u4NumIPv4 = 0;

/* #ifdef  CONFIG_IPV6 */
#if 0
	UINT_8 ip6[16] = { 0 };	/* FIX ME: avoid to allocate large memory in stack */
	UINT_32 u4NumIPv6 = 0;
#endif
	struct in_ifaddr *ifa = (struct in_ifaddr *)ptr;
	struct net_device *prDev = ifa->ifa_dev->dev;
	UINT_32 i;
	P_PARAM_NETWORK_ADDRESS_IP prParamIpAddr;
	P_GLUE_INFO_T prGlueInfo = NULL;

	if (prDev == NULL) {
		DBGLOG(REQ, INFO, ("netdev_event: device is empty.\n"));
		return NOTIFY_DONE;
	}

	if ((strncmp(prDev->name, "p2p",
		     3) != 0) && (strncmp(prDev->name, "wlan", 4) != 0)) {
		DBGLOG(REQ, INFO, ("netdev_event: xxx\n"));
		return NOTIFY_DONE;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prDev));

	if (prGlueInfo == NULL) {
		DBGLOG(REQ, INFO, ("netdev_event: prGlueInfo is empty.\n"));
		return NOTIFY_DONE;
	}
	ASSERT(prGlueInfo);

	if (fgIsUnderEarlierSuspend == false) {
		DBGLOG(REQ, INFO,
		       ("netdev_event: PARAM_MEDIA_STATE_DISCONNECTED. (%d)\n",
			prGlueInfo->eParamMediaStateIndicated));
		return NOTIFY_DONE;
	}

/* <3> get the IPv4 address */
	if (!prDev || !(prDev->ip_ptr) ||
	    !((struct in_device *)(prDev->ip_ptr))->ifa_list ||
	    !(&(((struct in_device *)(prDev->ip_ptr))->ifa_list->ifa_local))) {
		DBGLOG(REQ, INFO, ("ip is not avaliable.\n"));
		return NOTIFY_DONE;
	}

	kalMemCopy(ip,
		   &(((struct in_device *)(prDev->ip_ptr))->ifa_list->
		     ifa_local), sizeof(ip));
	DBGLOG(REQ, INFO, ("ip is %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]));

/* todo: traverse between list to find whole sets of IPv4 addresses */
	if (!((ip[0] == 0) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 0))) {
		u4NumIPv4++;
	}

/* #ifdef  CONFIG_IPV6 */
#if 0
	if (!prDev || !(prDev->ip6_ptr) ||
	    !((struct in_device *)(prDev->ip6_ptr))->ifa_list ||
	    !(&(((struct in_device *)(prDev->ip6_ptr))->ifa_list->ifa_local))) {
		printk(KERN_INFO "ipv6 is not avaliable.\n");
		return NOTIFY_DONE;
	}

	kalMemCopy(ip6,
		   &(((struct in_device *)(prDev->ip6_ptr))->
		     ifa_list->ifa_local), sizeof(ip6));
	printk(KERN_INFO
	       "ipv6 is %d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d\n",
	       ip6[0], ip6[1], ip6[2], ip6[3], ip6[4], ip6[5], ip6[6], ip6[7],
	       ip6[8], ip6[9], ip6[10], ip6[11], ip6[12], ip6[13], ip6[14],
	       ip6[15]
	    );

/* todo: traverse between list to find whole sets of IPv6 addresses */
	if (!((ip6[0] == 0) &&
	      (ip6[1] == 0) &&
	      (ip6[2] == 0) &&
	      (ip6[3] == 0) && (ip6[4] == 0) && (ip6[5] == 0))) {
/* u4NumIPv6++; */
	}
#endif

/* here we can compare the dev with other network's netdev to */
/* set the proper arp filter */
/*  */
/* IMPORTANT: please make sure if the context can sleep, if the context can't sleep */
/* we should schedule a kernel thread to do this for us */

/* <7> set up the ARP filter */
	{
		WLAN_STATUS rStatus = WLAN_STATUS_FAILURE;
		UINT_32 u4SetInfoLen = 0;
		UINT_8 aucBuf[32] = { 0 };
		UINT_32 u4Len =
		    OFFSET_OF(PARAM_NETWORK_ADDRESS_LIST, arAddress);
		P_PARAM_NETWORK_ADDRESS_LIST prParamNetAddrList =
		    (P_PARAM_NETWORK_ADDRESS_LIST) aucBuf;
		P_PARAM_NETWORK_ADDRESS prParamNetAddr =
		    prParamNetAddrList->arAddress;

/* #ifdef  CONFIG_IPV6 */
#if 0
		prParamNetAddrList->u4AddressCount = u4NumIPv4 + u4NumIPv6;
#else
		prParamNetAddrList->u4AddressCount = u4NumIPv4;
#endif
		prParamNetAddrList->u2AddressType = PARAM_PROTOCOL_ID_TCP_IP;
		for (i = 0; i < u4NumIPv4; i++) {
			prParamNetAddr->u2AddressLength = sizeof(PARAM_NETWORK_ADDRESS_IP);	/* 4;; */
			prParamNetAddr->u2AddressType =
			    PARAM_PROTOCOL_ID_TCP_IP;;
#if 0
			kalMemCopy(prParamNetAddr->aucAddress, ip, sizeof(ip));
			prParamNetAddr = (P_PARAM_NETWORK_ADDRESS) ((UINT_32)
								    prParamNetAddr
								    +
								    sizeof(ip));
			u4Len +=
			    OFFSET_OF(PARAM_NETWORK_ADDRESS,
				      aucAddress) + sizeof(ip);
#else
			prParamIpAddr =
			    (P_PARAM_NETWORK_ADDRESS_IP)
			    prParamNetAddr->aucAddress;
			kalMemCopy(&prParamIpAddr->in_addr, ip, sizeof(ip));
			prParamNetAddr = (P_PARAM_NETWORK_ADDRESS) ((UINT_32)
								    prParamNetAddr
								    +
								    sizeof
								    (PARAM_NETWORK_ADDRESS));
			u4Len +=
			    OFFSET_OF(PARAM_NETWORK_ADDRESS,
				      aucAddress) +
			    sizeof(PARAM_NETWORK_ADDRESS);
#endif
		}
/* #ifdef  CONFIG_IPV6 */
#if 0
		for (i = 0; i < u4NumIPv6; i++) {
			prParamNetAddr->u2AddressLength = 6;;
			prParamNetAddr->u2AddressType =
			    PARAM_PROTOCOL_ID_TCP_IP;;
			kalMemCopy(prParamNetAddr->aucAddress, ip6,
				   sizeof(ip6));
			prParamNetAddr = (P_PARAM_NETWORK_ADDRESS) ((UINT_32)
								    prParamNetAddr
								    +
								    sizeof
								    (ip6));
			u4Len +=
			    OFFSET_OF(PARAM_NETWORK_ADDRESS,
				      aucAddress) + sizeof(ip6);
		}
#endif
		ASSERT(u4Len <= sizeof(aucBuf));

		DBGLOG(REQ, INFO,
		       ("kalIoctl (0x%x, 0x%x)\n", prGlueInfo,
			prParamNetAddrList));

		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetNetworkAddress,
				   (PVOID) prParamNetAddrList,
				   u4Len,
				   FALSE, FALSE, TRUE, FALSE, &u4SetInfoLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, INFO,
			       ("set HW pattern filter fail 0x%lx\n", rStatus));
		}
	}

	return NOTIFY_DONE;
}

static struct notifier_block inetaddr_notifier = {
	.notifier_call = netdev_event,
};

void wlanRegisterNotifier(void)
{
	register_inetaddr_notifier(&inetaddr_notifier);
}

/* EXPORT_SYMBOL(wlanRegisterNotifier); */

void wlanUnregisterNotifier(void)
{
	unregister_inetaddr_notifier(&inetaddr_notifier);
}

/* EXPORT_SYMBOL(wlanUnregisterNotifier); */

#ifndef CONFIG_X86
#if defined(CONFIG_HAS_EARLYSUSPEND)

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will register platform driver to os
*
* \param[in] wlanSuspend    Function pointer to platform suspend function
* \param[in] wlanResume   Function pointer to platform resume   function
*
* \return The result of registering earlysuspend
*/
/*----------------------------------------------------------------------------*/

int glRegisterEarlySuspend(struct early_suspend *prDesc,
			   early_suspend_callback wlanSuspend,
			   late_resume_callback wlanResume)
{
	int ret = 0;

	if (NULL != wlanSuspend)
		prDesc->suspend = wlanSuspend;
	else {
		DBGLOG(REQ, INFO,
		       ("glRegisterEarlySuspend wlanSuspend ERROR.\n"));
		ret = -1;
	}

	if (NULL != wlanResume)
		prDesc->resume = wlanResume;
	else {
		DBGLOG(REQ, INFO,
		       ("glRegisterEarlySuspend wlanResume ERROR.\n"));
		ret = -1;
	}

	register_early_suspend(prDesc);
	return ret;
}

/* EXPORT_SYMBOL(glRegisterEarlySuspend); */

/*----------------------------------------------------------------------------*/
/*!
* \brief This function will un-register platform driver to os
*
* \return The result of un-registering earlysuspend
*/
/*----------------------------------------------------------------------------*/

int glUnregisterEarlySuspend(struct early_suspend *prDesc)
{
	int ret = 0;

	unregister_early_suspend(prDesc);

	prDesc->suspend = NULL;
	prDesc->resume = NULL;

	return ret;
}

/* EXPORT_SYMBOL(glUnregisterEarlySuspend); */
#endif
#endif /* !CONFIG_X86 */

/*----------------------------------------------------------------------------*/
/*!
* \brief Utility function for reading data from files on NVRAM-FS
*
* \param[in]
*           filename
*           len
*           offset
* \param[out]
*           buf
* \return
*           actual length of data being read
*/
/*----------------------------------------------------------------------------*/
static int nvram_read(char *filename, char *buf, ssize_t len, int offset)
{
#if CFG_SUPPORT_NVRAM
	struct file *fd;
	int retLen = -1;

	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = filp_open(filename, O_RDONLY, 0644);

	if (IS_ERR(fd)) {
		DBGLOG(INIT, INFO,
		       ("[MT6620][nvram_read] : failed to open!!\n"));
		return -1;
	}

	do {
		if ((fd->f_op == NULL) || (fd->f_op->read == NULL)) {
			DBGLOG(INIT, INFO,
			       ("[MT6620][nvram_read] : file can not be read!!\n"));
			break;
		}

		if (fd->f_pos != offset) {
			if (fd->f_op->llseek) {
				if (fd->f_op->llseek(fd, offset, 0) != offset) {
					DBGLOG(INIT, INFO,
					       ("[MT6620][nvram_read] : failed to seek!!\n"));
					break;
				}
			} else {
				fd->f_pos = offset;
			}
		}

		retLen = fd->f_op->read(fd, buf, len, &fd->f_pos);
	} while (FALSE);

	filp_close(fd, NULL);

	set_fs(old_fs);

	return retLen;

#else /* !CFG_SUPPORT_NVRAM */

	return -EIO;

#endif
}

/*----------------------------------------------------------------------------*/
/*!
* \brief Utility function for writing data to files on NVRAM-FS
*
* \param[in]
*           filename
*           buf
*           len
*           offset
* \return
*           actual length of data being written
*/
/*----------------------------------------------------------------------------*/
static int nvram_write(char *filename, char *buf, ssize_t len, int offset)
{
#if CFG_SUPPORT_NVRAM
	struct file *fd;
	int retLen = -1;

	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = filp_open(filename, O_WRONLY | O_CREAT, 0644);

	if (IS_ERR(fd)) {
		DBGLOG(INIT, INFO,
		       ("[MT6620][nvram_write] : failed to open!!\n"));
		return -1;
	}

	do {
		if ((fd->f_op == NULL) || (fd->f_op->write == NULL)) {
			DBGLOG(INIT, INFO,
			       ("[MT6620][nvram_write] : file can not be write!!\n"));
			break;
		}
		/* End of if */
		if (fd->f_pos != offset) {
			if (fd->f_op->llseek) {
				if (fd->f_op->llseek(fd, offset, 0) != offset) {
					DBGLOG(INIT, INFO,
					       ("[MT6620][nvram_write] : failed to seek!!\n"));
					break;
				}
			} else {
				fd->f_pos = offset;
			}
		}

		retLen = fd->f_op->write(fd, buf, len, &fd->f_pos);
	} while (FALSE);

	filp_close(fd, NULL);

	set_fs(old_fs);

	return retLen;

#else /* !CFG_SUPPORT_NVRAMS */

	return -EIO;

#endif
}

/*----------------------------------------------------------------------------*/
/*!
* \brief API for reading data on NVRAM
*
* \param[in]
*           prGlueInfo
*           u4Offset
* \param[out]
*           pu2Data
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalCfgDataRead16(IN P_GLUE_INFO_T prGlueInfo,
		 IN UINT_32 u4Offset, OUT PUINT_16 pu2Data)
{
	if (pu2Data == NULL) {
		return FALSE;
	}

	if (nvram_read(WIFI_NVRAM_FILE_NAME,
		       (char *)pu2Data,
		       sizeof(unsigned short),
		       u4Offset) != sizeof(unsigned short)) {
		return FALSE;
	} else {
		return TRUE;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* \brief API for writing data on NVRAM
*
* \param[in]
*           prGlueInfo
*           u4Offset
*           u2Data
* \return
*           TRUE
*           FALSE
*/
/*----------------------------------------------------------------------------*/
BOOLEAN
kalCfgDataWrite16(IN P_GLUE_INFO_T prGlueInfo, UINT_32 u4Offset, UINT_16 u2Data)
{
	if (nvram_write(WIFI_NVRAM_FILE_NAME,
			(char *)&u2Data,
			sizeof(unsigned short),
			u4Offset) != sizeof(unsigned short)) {
		return FALSE;
	} else {
		return TRUE;
	}
}
