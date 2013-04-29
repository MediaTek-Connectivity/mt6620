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

#ifndef _HIF_SDIO_CHRDEV_H_

#define _HIF_SDIO_CHRDEV_H_

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <linux/kthread.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>

#include "wmt_exp.h"
#include "wmt_plat.h"
#ifdef CFG_WMT_PS_SUPPORT
#undef CFG_WMT_PS_SUPPORT
#endif

extern INT32 hif_sdio_create_dev_node(void);
extern INT32 hif_sdio_remove_dev_node(void);
extern INT32 hifsdiod_start(void);
extern INT32 hifsdiod_stop(void);
INT32 hif_sdio_match_chipid_by_dev_id(const struct sdio_device_id *id);
INT32 hif_sdio_is_chipid_valid(INT32 chipId);

#endif /*_HIF_SDIO_CHRDEV_H_*/
