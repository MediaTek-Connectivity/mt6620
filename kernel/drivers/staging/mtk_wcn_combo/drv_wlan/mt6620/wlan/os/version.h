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

#ifndef _VERSION_H
#define _VERSION_H
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

#ifndef NIC_AUTHOR
#define NIC_AUTHOR      "NIC_AUTHOR"
#endif
#ifndef NIC_DESC
#define NIC_DESC        "NIC_DESC"
#endif

#ifndef NIC_NAME
#if defined(MT6620)
#define NIC_NAME            "MT6620"
#define NIC_DEVICE_ID       "MT6620"
#define NIC_DEVICE_ID_LOW   "mt6620"
#elif defined(MT5931)
#define NIC_NAME            "MT5931"
#define NIC_DEVICE_ID       "MT5931"
#define NIC_DEVICE_ID_LOW   "mt5931"
#elif defined(MT6628)
#define NIC_NAME            "MT6628"
#define NIC_DEVICE_ID       "MT6628"
#define NIC_DEVICE_ID_LOW   "mt6628"
#endif
#endif

/* NIC driver information */
#define NIC_VENDOR                      "MediaTek Inc."
#define NIC_VENDOR_OUI                  { 0x00, 0x0C, 0xE7 }

#if defined(MT6620)
#define NIC_PRODUCT_NAME \
"MediaTek Inc. MT6620 Wireless LAN Adapter"
#define NIC_DRIVER_NAME	\
"MediaTek Inc. MT6620 Wireless LAN Adapter Driver"
#elif defined(MT5931)
#define NIC_PRODUCT_NAME \
"MediaTek Inc. MT5931 Wireless LAN Adapter"
#define NIC_DRIVER_NAME	\
"MediaTek Inc. MT5931 Wireless LAN Adapter Driver"
#elif defined(MT6628)
#define NIC_PRODUCT_NAME \
"MediaTek Inc. MT6628 Wireless LAN Adapter"
#define NIC_DRIVER_NAME	\
"MediaTek Inc. MT6628 Wireless LAN Adapter Driver"
#endif

/* Define our driver version */
#define NIC_DRIVER_MAJOR_VERSION        2
#define NIC_DRIVER_MINOR_VERSION        0
#define NIC_DRIVER_VERSION              2, 0, 1, 1
#define NIC_DRIVER_VERSION_STRING       "2.0.1.1"

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

#endif /* _VERSION_H */
