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

#ifndef _RLM_PROTECTION_H
#define _RLM_PROTECTION_H

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
typedef enum _ENUM_SYS_PROTECT_MODE_T {
	SYS_PROTECT_MODE_NONE = 0,	/* Mode 0 */
	SYS_PROTECT_MODE_ERP,	/* Mode 1 */
	SYS_PROTECT_MODE_NON_HT,	/* Mode 2 */
	SYS_PROTECT_MODE_20M,	/* Mode 3 */

	SYS_PROTECT_MODE_NUM
} ENUM_SYS_PROTECT_MODE_T, *P_ENUM_SYS_PROTECT_MODE_T;

/* This definition follows HT Protection field of HT Operation IE */
typedef enum _ENUM_HT_PROTECT_MODE_T {
	HT_PROTECT_MODE_NONE = 0,
	HT_PROTECT_MODE_NON_MEMBER,
	HT_PROTECT_MODE_20M,
	HT_PROTECT_MODE_NON_HT,

	HT_PROTECT_MODE_NUM
} ENUM_HT_PROTECT_MODE_T, *P_ENUM_HT_PROTECT_MODE_T;

typedef enum _ENUM_GF_MODE_T {
	GF_MODE_NORMAL = 0,
	GF_MODE_PROTECT,
	GF_MODE_DISALLOWED,

	GF_MODE_NUM
} ENUM_GF_MODE_T, *P_ENUM_GF_MODE_T;

typedef enum _ENUM_RIFS_MODE_T {
	RIFS_MODE_NORMAL = 0,
	RIFS_MODE_DISALLOWED,

	RIFS_MODE_NUM
} ENUM_RIFS_MODE_T, *P_ENUM_RIFS_MODE_T;

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

#endif /* _RLM_PROTECTION_H */
