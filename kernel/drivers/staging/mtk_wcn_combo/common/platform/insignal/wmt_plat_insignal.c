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

/*! \file
\brief  Declaration of library functions

Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

#define FM_ANALOG_INPUT (1)

/* George: [FixMe] InSignal origen_b board, hard-coded here. */
/* #define CONFIG_MTK_COMBO_SDMMC3 */

#ifdef DFT_TAG
#undef DFT_TAG
#endif
#define DFT_TAG         "[WMT-PLAT]"

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#define CFG_USE_GPIOLIB (1)
/* Linux headers */
#if CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#define CFG_WMT_WAKELOCK_SUPPORT (1)
#endif
#include <linux/gpio.h>

/* InSignal platform headers */
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>

/* MTK_WCN_COMBO header files */
#include <mach/mtk_wcn_cmb_stub.h>
#include "mtk_wcn_cmb_hw.h"
#include "wmt_exp.h"
#include "wmt_plat.h"


#if defined(CONFIG_ARCH_EXYNOS5)

#define CONFIG_MTK_COMBO_SDMMC

#if defined(CONFIG_MTK_COMBO_SDMMC)

/* extern void sdmmc_cd_control(int state); */

void sdmmc_cd_control(int state)
{
	WMT_ERR_FUNC("card detection: sdmmc_cd_control is null\n");
}

#endif
#endif

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#if 0
#if defined(CONFIG_MTK_COMBO_SDMMC0)
extern void sdmmc0_cd_control(int state);
#define sdmmc_cd_control(x) sdmmc0_cd_control(x)
#endif
#if defined(CONFIG_MTK_COMBO_SDMMC1)
extern void sdmmc1_cd_control(int state);
#define sdmmc_cd_control(x) sdmmc1_cd_control(x)
#endif
#if defined(CONFIG_MTK_COMBO_SDMMC2)
extern void sdmmc2_cd_control(int state);
#define sdmmc_cd_control(x) sdmmc2_cd_control(x)
#endif
#if defined(CONFIG_MTK_COMBO_SDMMC3)
extern void sdmmc3_cd_control(int state);
#define sdmmc_cd_control(x) sdmmc3_cd_control(x)
#endif
#endif

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/* InSignal platform pin definition */
#if 0
#if defined(CONFIG_ARCH_EXYNOS4)
#define GPIO_COMBO_PMU_EN_PIN (EXYNOS4_GPE2(2))
#define GPIO_COMBO_RST_PIN (EXYNOS4_GPE2(1))

#define GPIO_COMBO_URXD_PIN (EXYNOS4_GPA0(0))	/* UART0 RXD, pull up */
#define GPIO_COMBO_UTXD_PIN (EXYNOS4_GPA0(1))	/* UART0 TXD, pull none */
#elif defined(CONFIG_ARCH_S5PV310)
#define GPIO_COMBO_PMU_EN_PIN (S5PV310_GPE2(2))
#define GPIO_COMBO_RST_PIN (S5PV310_GPE2(1))

#define GPIO_COMBO_URXD_PIN (S5PV310_GPA0(0))	/* UART0 RXD, pull up */
#define GPIO_COMBO_UTXD_PIN (S5PV310_GPA0(1))	/* UART0 TXD, pull none */
#endif

#if CONFIG_HAS_WAKELOCK
#define CFG_WMT_WAKELOCK_SUPPORT (1)
#endif
#endif

#if defined(CONFIG_ARCH_EXYNOS5)
/* PMU_EN, module CON5 pin#12, XEINT_23, GPX2[7]/INT2[7] */
#define GPIO_COMBO_PMU_EN_PIN (EXYNOS5_GPX2(7))
/* nRESET, module CON5 pin#16, XuRTSn0, GPA0[3]/UART0 RTS */
#define GPIO_COMBO_RST_PIN (EXYNOS5_GPA0(3))
/* BGF_INT, module CON5 pin#22, XEINT_8, GPX1[0]/INT1[0] */
/*#define GPIO_COMBO_BGF_EINT_PIN (EXYNOS5_GPX1(0))*/
/* WIFI_INT, module CON5 pin#24, XEINT_7, GPX0[7]/INT0[7] */
/*#define GPIO_COMBO_WIFI_EINT_PIN (EXYNOS5_GPX0(7))*/
/* URXD, module CON5 pin#25, XuTXD0, GPA0[0] */
#define GPIO_COMBO_URXD_PIN ( EXYNOS5_GPA0(0) )
/* UTXD, module CON5 pin#27, XuRXD0, GPA0[1] */
#define GPIO_COMBO_UTXD_PIN ( EXYNOS5_GPA0(1) )

#endif

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

static VOID wmt_plat_func_ctrl(UINT32 type, UINT32 on);
static VOID wmt_plat_bgf_eirq_cb(VOID);

/* static INT32 wmt_plat_ldo_ctrl (ENUM_PIN_STATE state); */
static INT32 wmt_plat_pmu_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_rtc_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_rst_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_bgf_eint_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_wifi_eint_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_all_eint_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_uart_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_pcm_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_i2s_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_sdio_pin_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_sync_ctrl(ENUM_PIN_STATE state);
static INT32 wmt_plat_gps_lna_ctrl(ENUM_PIN_STATE state);

static INT32 wmt_plat_dump_pin_conf(VOID);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

UINT32 gWmtDbgLvl = WMT_LOG_INFO;

#if CFG_WMT_WAKELOCK_SUPPORT
static OSAL_SLEEPABLE_LOCK gOsSLock;
static struct wake_lock wmtWakeLock;
#endif

irq_cb wmt_plat_bgf_irq_cb = NULL;
device_audio_if_cb wmt_plat_audio_if_cb = NULL;
const static fp_set_pin gfp_set_pin_table[] = {
	[PIN_LDO] = NULL,	/* no PIN_LDO is used, wmt_plat_ldo_ctrl, */
	[PIN_PMU] = wmt_plat_pmu_ctrl,
	[PIN_RTC] = wmt_plat_rtc_ctrl,
	[PIN_RST] = wmt_plat_rst_ctrl,
	[PIN_BGF_EINT] = wmt_plat_bgf_eint_ctrl,
	[PIN_WIFI_EINT] = wmt_plat_wifi_eint_ctrl,
	[PIN_ALL_EINT] = wmt_plat_all_eint_ctrl,
	[PIN_UART_GRP] = wmt_plat_uart_ctrl,
	[PIN_PCM_GRP] = wmt_plat_pcm_ctrl,
	[PIN_I2S_GRP] = wmt_plat_i2s_ctrl,
	[PIN_SDIO_GRP] = wmt_plat_sdio_pin_ctrl,
	[PIN_GPS_SYNC] = wmt_plat_gps_sync_ctrl,
	[PIN_GPS_LNA] = wmt_plat_gps_lna_ctrl,
};

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

/*!
* \brief audio control callback function for CMB_STUB on ALPS
*
* A platform function required for dynamic binding with CMB_STUB on ALPS.
*
* \param state desired audio interface state to use
* \param flag audio interface control options
*
* \retval 0 operation success
* \retval -1 invalid parameters
* \retval < 0 error for operation fail
*/
INT32 wmt_plat_audio_ctrl(CMB_STUB_AIF_X state, CMB_STUB_AIF_CTRL ctrl)
{
	INT32 iRet;
	UINT32 pinShare;

/* input sanity check */
	if ((CMB_STUB_AIF_MAX <= state)
	    || (CMB_STUB_AIF_CTRL_MAX <= ctrl)) {
        WMT_ERR_FUNC("WMT-PLAT: invalid para, state(%d), ctrl(%d),iRet(%d) \n", state, ctrl, iRet);
		return -1;
	}
    if (0/*I2S/PCM share pin*/) {
        // TODO: [FixMe][GeorgeKuo] how about MT6575? The following is applied to MT6573E1 only!!
        pinShare = 1;
        WMT_INFO_FUNC( "PCM/I2S pin share\n");
    }
    else{ //E1 later
        pinShare = 0;
        WMT_INFO_FUNC( "PCM/I2S pin seperate\n");
    }
	iRet = 0;

/* set host side first */
	switch (state) {
	case CMB_STUB_AIF_0:
/* BT_PCM_OFF & FM line in/out */
		iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_DEINIT);
		iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
		break;

	case CMB_STUB_AIF_1:
		iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_INIT);
		iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_DEINIT);
		break;

	case CMB_STUB_AIF_2:
		iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_DEINIT);
		iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
		break;

	case CMB_STUB_AIF_3:
		iRet += wmt_plat_gpio_ctrl(PIN_PCM_GRP, PIN_STA_INIT);
		iRet += wmt_plat_gpio_ctrl(PIN_I2S_GRP, PIN_STA_INIT);
		break;

	default:
/* FIXME: move to cust folder? */
		WMT_ERR_FUNC("invalid state [%d]\n", state);
		return -1;
		break;
	}

	if (CMB_STUB_AIF_CTRL_EN == ctrl) {
		WMT_INFO_FUNC("call chip aif setting \n");
/* need to control chip side GPIO */
		if (NULL != wmt_plat_audio_if_cb)
		{
		    iRet += (*wmt_plat_audio_if_cb)(state, (pinShare) ? MTK_WCN_BOOL_TRUE : MTK_WCN_BOOL_FALSE);
		}
		else
		{
		    WMT_WARN_FUNC("wmt_plat_audio_if_cb is not registered \n");
		    iRet -= 1;
		}
	} else {
		WMT_INFO_FUNC("skip chip aif setting \n");
	}

	return iRet;
}

#if CFG_WMT_PS_SUPPORT
irqreturn_t irq_handler(int i, void *arg)
{
    wmt_plat_bgf_eirq_cb();
    return IRQ_HANDLED;
}
#endif

static VOID wmt_plat_bgf_eirq_cb(VOID)
{
#if CFG_WMT_PS_SUPPORT
/* #error "need to disable EINT here" */
/* wmt_lib_ps_irq_cb(); */
	if (NULL != wmt_plat_bgf_irq_cb) {
		(*(wmt_plat_bgf_irq_cb)) ();
	} else {
		WMT_WARN_FUNC("WMT-PLAT: wmt_plat_bgf_irq_cb not registered\n");
	}
#else
	return;
#endif
}

VOID wmt_lib_plat_irq_cb_reg(irq_cb bgf_irq_cb)
{
	wmt_plat_bgf_irq_cb = bgf_irq_cb;
}

VOID wmt_lib_plat_aif_cb_reg(device_audio_if_cb aif_ctrl_cb)
{
	wmt_plat_audio_if_cb = aif_ctrl_cb;
}

INT32 wmt_plat_init(P_PWR_SEQ_TIME pPwrSeqTime)
{
    //CMB_STUB_CB stub_cb;
/*PWR_SEQ_TIME pwr_seq_time;*/
	INT32 iret;

    //stub_cb.aif_ctrl_cb = wmt_plat_audio_ctrl;
    //stub_cb.func_ctrl_cb = wmt_plat_func_ctrl;
    //stub_cb.size = sizeof(stub_cb);

/* register to cmb_stub */
    //iret = mtk_wcn_cmb_stub_reg(&stub_cb);

/* init cmb_hw */
	iret += mtk_wcn_cmb_hw_init(pPwrSeqTime);

/*init wmt function ctrl wakelock if wake lock is supported by host platform*/
#ifdef CFG_WMT_WAKELOCK_SUPPORT
	wake_lock_init(&wmtWakeLock, WAKE_LOCK_SUSPEND, "wmtFuncCtrl");
	osal_sleepable_lock_init(&gOsSLock);
#endif

	WMT_DBG_FUNC("init result (%d), return 0\n", iret);

	return 0;
}

INT32 wmt_plat_deinit(VOID)
{
	INT32 iret;

/* 1. de-init cmb_hw */
	iret = mtk_wcn_cmb_hw_deinit();
/* 2. unreg to cmb_stub */
	iret += mtk_wcn_cmb_stub_unreg();
/*3. wmt wakelock deinit*/
#ifdef CFG_WMT_WAKELOCK_SUPPORT
		wake_lock_destroy(&wmtWakeLock);
		osal_sleepable_lock_deinit(&gOsSLock);
		WMT_DBG_FUNC("destroy wmtWakeLock\n");
#endif
	WMT_DBG_FUNC("deinit result (%d), return 0\n", iret);

	return 0;
}

INT32 wmt_plat_sdio_ctrl(WMT_SDIO_SLOT_NUM sdioPortType, ENUM_FUNC_STATE on)
{
	if (FUNC_ON == on) {
/* add control logic here to generate SDIO CARD INSERTION event to mmc/sd
* controller. SDIO card detection operation and detect success messages
* are expected.
*/
		printk(KERN_ERR " detect mmc ===========================\n");
		sdmmc_cd_control(1);
	} else {
/* add control logic here to generate SDIO CARD REMOVAL event to mmc/sd
* controller. SDIO card removal operation and remove success messages
* are expected.
*/
		printk(KERN_ERR " remove mmc ===========================\n");
		sdmmc_cd_control(0);
	}
	return 0;
}

static INT32 wmt_plat_dump_pin_conf(VOID)
{
	WMT_INFO_FUNC("[WMT-PLAT]=>dump wmt pin configuration start<=\n");

#ifdef GPIO_COMBO_PMU_EN_PIN
	WMT_INFO_FUNC("PMU(GPIO%d)\n", GPIO_COMBO_PMU_EN_PIN);
#else
	WMT_INFO_FUNC("PMU(not defined)\n");
#endif

#ifdef GPIO_COMBO_RST_PIN
	WMT_INFO_FUNC("RST(GPIO%d)\n", GPIO_COMBO_RST_PIN);
#else
	WMT_INFO_FUNC("RST(not defined)\n");
#endif

#ifdef GPIO_COMBO_BGF_EINT_PIN
	WMT_INFO_FUNC("BGF_EINT(GPIO%d)\n", GPIO_COMBO_BGF_EINT_PIN);
#else
	WMT_INFO_FUNC("BGF_EINT(not defined)\n");
#endif

#ifdef CUST_EINT_COMBO_BGF_NUM
	WMT_INFO_FUNC("BGF_EINT_NUM(%d)\n", CUST_EINT_COMBO_BGF_NUM);
#else
	WMT_INFO_FUNC("BGF_EINT_NUM(not defined)\n");
#endif

#ifdef GPIO_WIFI_EINT_PIN
	WMT_INFO_FUNC("WIFI_EINT(GPIO%d)\n", GPIO_WIFI_EINT_PIN);
#else
	WMT_INFO_FUNC("WIFI_EINT(not defined)\n");
#endif

#ifdef CUST_EINT_WIFI_NUM
	WMT_INFO_FUNC("WIFI_EINT_NUM(%d)\n", CUST_EINT_WIFI_NUM);
#else
	WMT_INFO_FUNC("WIFI_EINT_NUM(not defined)\n");
#endif

#ifdef GPIO_COMBO_URXD_PIN
	WMT_INFO_FUNC("URXD(GPIO%d)\n", GPIO_COMBO_URXD_PIN);
#else
	WMT_INFO_FUNC("URXD(not defined)\n");
#endif

#ifdef GPIO_COMBO_UTXD_PIN
	WMT_INFO_FUNC("UTXD(GPIO%d)\n", GPIO_COMBO_UTXD_PIN);
#else
	WMT_INFO_FUNC("UTXD(not defined)\n");
#endif

#ifdef GPIO_PCM_DAICLK_PIN
	WMT_INFO_FUNC("DAICLK(GPIO%d)\n", GPIO_PCM_DAICLK_PIN);
#else
	WMT_INFO_FUNC("DAICLK(not defined)\n");
#endif
#ifdef GPIO_PCM_DAIPCMOUT_PIN
	WMT_INFO_FUNC("PCMOUT(GPIO%d)\n", GPIO_PCM_DAIPCMOUT_PIN);
#else
	WMT_INFO_FUNC("PCMOUT(not defined)\n");
#endif
#ifdef GPIO_PCM_DAIPCMIN_PIN
	WMT_INFO_FUNC("PCMIN(GPIO%d)\n", GPIO_PCM_DAIPCMIN_PIN);
#else
	WMT_INFO_FUNC("PCMIN(not defined)\n");
#endif
#ifdef GPIO_PCM_DAISYNC_PIN
	WMT_INFO_FUNC("PCMSYNC(GPIO%d)\n", GPIO_PCM_DAISYNC_PIN);
#else
	WMT_INFO_FUNC("PCMSYNC(not defined)\n");
#endif

#if FM_ANALOG_INPUT
	WMT_DBG_FUNC("FM analog mode is set, no need for I2S GPIOs\n");
#else
/* FM digital input */
#ifdef GPIO_COMBO_I2S_CK_PIN
	WMT_INFO_FUNC("I2S_CK(GPIO%d)\n", GPIO_COMBO_I2S_CK_PIN);
#else
	WMT_INFO_FUNC("I2S_CK(not defined)\n");
#endif
#ifdef GPIO_COMBO_I2S_WS_PIN
	WMT_INFO_FUNC("I2S_WS(GPIO%d)\n", GPIO_COMBO_I2S_WS_PIN);
#else
	WMT_INFO_FUNC("I2S_WS(not defined)\n");
#endif
#ifdef GPIO_COMBO_I2S_DAT_PIN
	WMT_INFO_FUNC("I2S_DAT(GPIO%d)\n", GPIO_COMBO_I2S_DAT_PIN);
#else
	WMT_INFO_FUNC("I2S_DAT(not defined)\n");
#endif
#endif

#ifdef GPIO_GPS_SYNC_PIN
	WMT_INFO_FUNC("GPS_SYNC(GPIO%d)\n", GPIO_GPS_SYNC_PIN);
#else
	WMT_INFO_FUNC("GPS_SYNC(not defined)\n");
#endif

#ifdef GPIO_GPS_LNA_PIN
	WMT_INFO_FUNC("GPS_LNA(GPIO%d)\n", GPIO_GPS_LNA_PIN);
#else
	WMT_INFO_FUNC("GPS_LNA(not defined)\n");
#endif
	WMT_INFO_FUNC("[WMT-PLAT]=>dump wmt pin configuration emds<=\n");
	return 0;
}

INT32 wmt_plat_pwr_ctrl(ENUM_FUNC_STATE state)
{
	INT32 ret = -1;

	switch (state) {
	case FUNC_ON:
/* TODO:[ChangeFeature][George] always output this or by request throuth /proc or sysfs? */
		wmt_plat_dump_pin_conf();
		ret = mtk_wcn_cmb_hw_pwr_on();
		break;

	case FUNC_OFF:
		ret = mtk_wcn_cmb_hw_pwr_off();
		break;

	case FUNC_RST:
		ret = mtk_wcn_cmb_hw_rst();
		break;

	default:
		WMT_WARN_FUNC("invalid state(%d)\n", state);
		break;
	}

	return ret;
}

INT32 wmt_plat_ps_ctrl(ENUM_FUNC_STATE state)
{
	return -1;
}

INT32 wmt_plat_eirq_ctrl(ENUM_PIN_ID id, ENUM_PIN_STATE state)
{
	INT32 iret;

/* TODO: [ChangeFeature][GeorgeKuo]: use another function to handle this, as done in gpio_ctrls */

	if ((PIN_STA_INIT != state)
	    && (PIN_STA_DEINIT != state)
	    && (PIN_STA_EINT_EN != state)
	    && (PIN_STA_EINT_DIS != state)) {
		WMT_WARN_FUNC("invalid PIN_STATE(%d) for PIN(%d)\n", state, id);
		return -1;
	}

	iret = -2;
	switch (id) {
	case PIN_BGF_EINT:
		WMT_INFO_FUNC("BGF EINT INT state(%d)\n", state);

		if (PIN_STA_INIT == state) {
/* set level trigger (low), debounce count (if any), register irq handler wmt_plat_bgf_eirq_cb() */
/* MASK interrupt! */
		} else if (PIN_STA_EINT_EN == state) {
/* UNMASK interrupt! */
			WMT_DBG_FUNC("BGFInt (en) \n");
		} else if (PIN_STA_EINT_DIS == state) {
/* MASK interrupt! */
			WMT_DBG_FUNC("BGFInt (dis) \n");
		} else {
/* MASK interrupt! */
/* de-init: nothing to do? */
		}
		iret = 0;
		break;

	case PIN_ALL_EINT:
#ifdef GPIO_COMBO_ALL_EINT_PIN
		if (PIN_STA_INIT == state) {
#if 0
			mt65xx_eint_set_sens(CUST_EINT_COMBO_ALL_NUM,
					     CUST_EINT_COMBO_ALL_SENSITIVE);
			mt65xx_eint_set_hw_debounce(CUST_EINT_COMBO_ALL_NUM,
						    CUST_EINT_COMBO_ALL_DEBOUNCE_CN);
			mt65xx_eint_registration(CUST_EINT_COMBO_ALL_NUM,
						 CUST_EINT_COMBO_ALL_DEBOUNCE_EN,
						 CUST_EINT_COMBO_ALL_POLARITY,
						 combo_bgf_eirq_handler, 0);
#endif
			mt65xx_eint_mask(CUST_EINT_COMBO_ALL_NUM);	/*2 */
			WMT_DBG_FUNC("ALLInt (INIT but not used yet) \n");
		} else if (PIN_STA_EINT_EN == state) {
/*mt65xx_eint_unmask(CUST_EINT_COMBO_ALL_NUM);*/
			WMT_DBG_FUNC("ALLInt (EN but not used yet) \n");
		} else if (PIN_STA_EINT_DIS == state) {
			mt65xx_eint_mask(CUST_EINT_COMBO_ALL_NUM);
			WMT_DBG_FUNC("ALLInt (DIS but not used yet) \n");
		} else {
			mt65xx_eint_mask(CUST_EINT_COMBO_ALL_NUM);
			WMT_DBG_FUNC("ALLInt (DEINIT but not used yet) \n");
/* de-init: nothing to do in ALPS, such as un-registration... */
		}
#else
		WMT_DBG_FUNC("ALLInt (not used yet) \n");
#endif
		iret = 0;
		break;

	default:
		WMT_WARN_FUNC("unsupported EIRQ(PIN_ID:%d)\n", id);
		iret = -1;
		break;
	}

	return iret;
}

INT32 wmt_plat_gpio_ctrl(ENUM_PIN_ID id, ENUM_PIN_STATE state)
{
	if ((PIN_ID_MAX > id)
	    && (PIN_STA_MAX > state)) {
/* TODO: [FixMe][GeorgeKuo] do sanity check to const function table when init and skip checking here */
		if (gfp_set_pin_table[id]) {
			return (*(gfp_set_pin_table[id])) (state);	/* .handler */
		} else {
			WMT_WARN_FUNC("null fp for gpio_ctrl(%d)\n", id);
			return -2;
		}
	}
	return -1;
}

#if 0
INT32 wmt_plat_ldo_ctrl(ENUM_PIN_STATE state)
{
#ifdef GPIO_COMBO_6620_LDO_EN_PIN
	switch (state) {
	case PIN_STA_INIT:
/*set to gpio output low, disable pull*/
		mt_set_gpio_pull_enable(GPIO_COMBO_6620_LDO_EN_PIN,
					GPIO_PULL_DISABLE);
		mt_set_gpio_dir(GPIO_COMBO_6620_LDO_EN_PIN, GPIO_DIR_OUT);
		mt_set_gpio_mode(GPIO_COMBO_6620_LDO_EN_PIN, GPIO_MODE_GPIO);
		mt_set_gpio_out(GPIO_COMBO_6620_LDO_EN_PIN, GPIO_OUT_ZERO);
		WMT_DBG_FUNC("LDO init (out 0) \n");
		break;

	case PIN_STA_OUT_H:
		mt_set_gpio_out(GPIO_COMBO_6620_LDO_EN_PIN, GPIO_OUT_ONE);
		WMT_DBG_FUNC("LDO (out 1) \n");
		break;

	case PIN_STA_OUT_L:
		mt_set_gpio_out(GPIO_COMBO_6620_LDO_EN_PIN, GPIO_OUT_ZERO);
		WMT_DBG_FUNC("LDO (out 0) \n");
		break;

	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
/*set to gpio input low, pull down enable*/
		mt_set_gpio_mode(GPIO_COMBO_6620_LDO_EN_PIN,
				 GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_6620_LDO_EN_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_select(GPIO_COMBO_6620_LDO_EN_PIN,
					GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(GPIO_COMBO_6620_LDO_EN_PIN,
					GPIO_PULL_ENABLE);
		WMT_DBG_FUNC("LDO deinit (in pd) \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on LDO\n", state);
		break;
	}
#else
	WMT_INFO_FUNC("LDO is not used\n");
#endif
	return 0;
}
#endif
INT32 wmt_plat_pmu_ctrl(ENUM_PIN_STATE state)
{
	if (gpio_request(GPIO_COMBO_PMU_EN_PIN, "GPIO_COMBO_PMU_EN_PIN")) {
		WMT_ERR_FUNC("GPIO_COMBO_PMU_EN_PIN request port error!\n");
		return -1;
	}
	WMT_DBG_FUNC("GPIO_COMBO_PMU_EN_PIN request port OK!\n");

	switch (state) {
	case PIN_STA_INIT:
		/*set to gpio output low, disable pull*/
		/*s3c_gpio_cfgpin(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_OUTPUT);*/
		s3c_gpio_setpull(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_COMBO_PMU_EN_PIN, 0);
		WMT_INFO_FUNC("PMU init (out 0) \n");
		break;

	case PIN_STA_OUT_H:
		/*s3c_gpio_cfgpin(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_OUTPUT);*/
		s3c_gpio_setpull(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_COMBO_PMU_EN_PIN, 1);
		WMT_INFO_FUNC("PMU (out 1) \n");
		break;

	case PIN_STA_OUT_L:
		/*s3c_gpio_cfgpin(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_OUTPUT);*/
		s3c_gpio_setpull(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_COMBO_PMU_EN_PIN, 0);
		WMT_INFO_FUNC("PMU (out 0) \n");
		break;

	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
		/*set to gpio input low, pull down enable*/
		s3c_gpio_cfgpin(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_PULL_DOWN);
		WMT_INFO_FUNC("PMU deinit (in pd) \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on PMU\n",
			state);
		break;
	}

	gpio_free(GPIO_COMBO_PMU_EN_PIN);
	return 0;
}

INT32 wmt_plat_rtc_ctrl(ENUM_PIN_STATE state)
{
	switch (state) {
	case PIN_STA_INIT:
/* enable RTC clock */
		WMT_DBG_FUNC("RTC init \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on RTC\n", state);
		break;
	}
	return 0;
}

INT32 wmt_plat_rst_ctrl(ENUM_PIN_STATE state)
{
	if (gpio_request(GPIO_COMBO_RST_PIN, "GPIO_COMBO_RST_PIN")) {
		WMT_ERR_FUNC("GPIO_COMBO_RST_PIN request port error!\n");
		return -1;
	}
	WMT_DBG_FUNC("GPIO_COMBO_RST_PIN request port OK!\n");

	switch (state) {
	case PIN_STA_INIT:
		/*set to gpio output low, disable pull*/
		/*s3c_gpio_cfgpin(GPIO_COMBO_RST_PIN, S3C_GPIO_OUTPUT);*/
		s3c_gpio_setpull(GPIO_COMBO_RST_PIN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_COMBO_RST_PIN, 0);
		WMT_INFO_FUNC("RST init (out 0) \n");
		break;

	case PIN_STA_OUT_H:
		/*s3c_gpio_cfgpin(GPIO_COMBO_RST_PIN, S3C_GPIO_OUTPUT);*/
		s3c_gpio_setpull(GPIO_COMBO_RST_PIN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_COMBO_RST_PIN, 1);
		WMT_INFO_FUNC("RST (out 1) \n");
		break;

	case PIN_STA_OUT_L:
		/*s3c_gpio_cfgpin(GPIO_COMBO_RST_PIN, S3C_GPIO_OUTPUT);*/
		s3c_gpio_setpull(GPIO_COMBO_RST_PIN, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_COMBO_RST_PIN, 0);
		WMT_INFO_FUNC("RST (out 0) \n");
		break;

	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
		/*set to gpio input low, pull down enable*/
		s3c_gpio_cfgpin(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_COMBO_PMU_EN_PIN, S3C_GPIO_PULL_DOWN);
		WMT_INFO_FUNC("RST deinit (in pd) \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on RST\n", state);
		break;
	}

	gpio_free(GPIO_COMBO_RST_PIN);
	return 0;
}

INT32 wmt_plat_bgf_eint_ctrl(ENUM_PIN_STATE state)
{
	WMT_INFO_FUNC("BGF EINT GPIO state(%d)\n", state);

#ifdef GPIO_COMBO_BGF_EINT_PIN
	switch (state) {
	case PIN_STA_INIT:
/*set to gpio input low, pull down enable*/
		mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN,
				 GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_BGF_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN,
					GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN,
					GPIO_PULL_ENABLE);
		WMT_DBG_FUNC("BGFInt init(in pd) \n");
		break;

	case PIN_STA_MUX:
		mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN,
				 GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
		mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN,
					GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN, GPIO_PULL_UP);
		mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN,
				 GPIO_COMBO_BGF_EINT_PIN_M_EINT);
		WMT_DBG_FUNC("BGFInt mux (eint) \n");
		break;

	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
/*set to gpio input low, pull down enable*/
		mt_set_gpio_mode(GPIO_COMBO_BGF_EINT_PIN,
				 GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_BGF_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_select(GPIO_COMBO_BGF_EINT_PIN,
					GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(GPIO_COMBO_BGF_EINT_PIN,
					GPIO_PULL_ENABLE);
		WMT_DBG_FUNC("BGFInt deinit(in pd) \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on BGF EINT\n",
			      state);
		break;
	}
#else
	WMT_INFO_FUNC("BGF EINT not defined\n", state);
#endif
	return 0;
}

INT32 wmt_plat_wifi_eint_ctrl(ENUM_PIN_STATE state)
{
#ifdef GPIO_WIFI_EINT_PIN
	switch (state) {
	case PIN_STA_INIT:
		mt_set_gpio_pull_enable(GPIO_WIFI_EINT_PIN, GPIO_PULL_DISABLE);
		mt_set_gpio_dir(GPIO_WIFI_EINT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_mode(GPIO_WIFI_EINT_PIN, GPIO_MODE_GPIO);
		mt_set_gpio_out(GPIO_WIFI_EINT_PIN, GPIO_OUT_ONE);
		break;
	case PIN_STA_MUX:
		mt_set_gpio_mode(GPIO_WIFI_EINT_PIN, GPIO_WIFI_EINT_PIN_M_GPIO);
		mt_set_gpio_pull_enable(GPIO_WIFI_EINT_PIN, GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_WIFI_EINT_PIN, GPIO_PULL_UP);
		mt_set_gpio_mode(GPIO_WIFI_EINT_PIN, GPIO_WIFI_EINT_PIN_M_EINT);

		break;
	case PIN_STA_EINT_EN:
		mt65xx_eint_unmask(CUST_EINT_WIFI_NUM);
		break;
	case PIN_STA_EINT_DIS:
		mt65xx_eint_mask(CUST_EINT_WIFI_NUM);
		break;
	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
/*set to gpio input low, pull down enable*/
		mt_set_gpio_mode(GPIO_WIFI_EINT_PIN,
				 GPIO_COMBO_BGF_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_WIFI_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_select(GPIO_WIFI_EINT_PIN, GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(GPIO_WIFI_EINT_PIN, GPIO_PULL_ENABLE);
		break;
	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on WIFI EINT\n",
			      state);
		break;
	}
#else
	WMT_INFO_FUNC("WIFI EINT is controlled by MSDC driver \n");
#endif
	return 0;
}

INT32 wmt_plat_all_eint_ctrl(ENUM_PIN_STATE state)
{
#ifdef GPIO_COMBO_ALL_EINT_PIN
	switch (state) {
	case PIN_STA_INIT:
		mt_set_gpio_mode(GPIO_COMBO_ALL_EINT_PIN,
				 GPIO_COMBO_ALL_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_ALL_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_select(GPIO_COMBO_ALL_EINT_PIN,
					GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(GPIO_COMBO_ALL_EINT_PIN,
					GPIO_PULL_ENABLE);
		WMT_DBG_FUNC("ALLInt init(in pd) \n");
		break;

	case PIN_STA_MUX:
		mt_set_gpio_mode(GPIO_COMBO_ALL_EINT_PIN,
				 GPIO_COMBO_ALL_EINT_PIN_M_GPIO);
		mt_set_gpio_pull_enable(GPIO_COMBO_ALL_EINT_PIN,
					GPIO_PULL_ENABLE);
		mt_set_gpio_pull_select(GPIO_COMBO_ALL_EINT_PIN, GPIO_PULL_UP);
		mt_set_gpio_mode(GPIO_COMBO_ALL_EINT_PIN,
				 GPIO_COMBO_ALL_EINT_PIN_M_EINT);
		break;

	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
/*set to gpio input low, pull down enable*/
		mt_set_gpio_mode(GPIO_COMBO_ALL_EINT_PIN,
				 GPIO_COMBO_ALL_EINT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_ALL_EINT_PIN, GPIO_DIR_IN);
		mt_set_gpio_pull_select(GPIO_COMBO_ALL_EINT_PIN,
					GPIO_PULL_DOWN);
		mt_set_gpio_pull_enable(GPIO_COMBO_ALL_EINT_PIN,
					GPIO_PULL_ENABLE);
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on ALL EINT\n",
			      state);
		break;
	}
#else
	WMT_INFO_FUNC("ALL EINT not defined\n");
#endif
	return 0;
}

INT32 wmt_plat_uart_ctrl(ENUM_PIN_STATE state)
{
	switch (state) {
	case PIN_STA_MUX:
	case PIN_STA_INIT:
		/* pull up */
		s3c_gpio_setpull(GPIO_COMBO_URXD_PIN, S3C_GPIO_PULL_UP);
		s3c_gpio_setpull(GPIO_COMBO_UTXD_PIN, S3C_GPIO_PULL_UP);
		/* UART-x mode */
		s3c_gpio_cfgpin(GPIO_COMBO_URXD_PIN, S3C_GPIO_SFN(0x2));
		s3c_gpio_cfgpin(GPIO_COMBO_UTXD_PIN, S3C_GPIO_SFN(0x2));
		/* driving */
		s5p_gpio_set_drvstr(GPIO_COMBO_URXD_PIN, S5P_GPIO_DRVSTR_LV4);
		s5p_gpio_set_drvstr(GPIO_COMBO_UTXD_PIN, S5P_GPIO_DRVSTR_LV4);
		WMT_INFO_FUNC("UART init (rx pull up, tx pull up) \n");
		break;
	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
		/* input pull down or output low */
		s3c_gpio_setpull(GPIO_COMBO_URXD_PIN, S3C_GPIO_PULL_DOWN);
		s3c_gpio_setpull(GPIO_COMBO_UTXD_PIN, S3C_GPIO_PULL_DOWN);
		s3c_gpio_cfgpin(GPIO_COMBO_URXD_PIN, S3C_GPIO_SFN(0));
		s3c_gpio_cfgpin(GPIO_COMBO_UTXD_PIN, S3C_GPIO_SFN(0));
		WMT_INFO_FUNC("UART deinit (rx in-pd, tx in-pd\n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on UART Group\n",
			      state);
		break;
	}

	return 0;
}

INT32 wmt_plat_pcm_ctrl(ENUM_PIN_STATE state)
{
#ifdef GPIO_COMBO_PCM_CLK
	switch (state) {
	case PIN_STA_MUX:
	case PIN_STA_INIT:
		mt_set_gpio_mode(GPIO_COMBO_PCM_CLK, GPIO_PCM_DAICLK_PIN_M_CLK);
		mt_set_gpio_mode(GPIO_COMBO_PCM_OUT,
				 GPIO_PCM_DAIPCMOUT_PIN_M_DAIPCMOUT);
		mt_set_gpio_mode(GPIO_COMBO_PCM_IN,
				 GPIO_PCM_DAIPCMIN_PIN_M_DAIPCMIN);
		mt_set_gpio_mode(GPIO_COMBO_PCM_SYNC,
				 GPIO_PCM_DAISYNC_PIN_M_BTSYNC);
		WMT_DBG_FUNC("PCM init (pcm) \n");
		break;

	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
		mt_set_gpio_mode(GPIO_PCM_DAICLK_PIN,
				 GPIO_PCM_DAICLK_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_PCM_DAICLK_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_PCM_DAICLK_PIN, GPIO_OUT_ZERO);

		mt_set_gpio_mode(GPIO_PCM_DAIPCMOUT_PIN,
				 GPIO_PCM_DAIPCMOUT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_PCM_DAIPCMOUT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_PCM_DAIPCMOUT_PIN, GPIO_OUT_ZERO);

		mt_set_gpio_mode(GPIO_PCM_DAIPCMIN_PIN,
				 GPIO_PCM_DAIPCMIN_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_PCM_DAIPCMIN_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_PCM_DAIPCMIN_PIN, GPIO_OUT_ZERO);

		mt_set_gpio_mode(GPIO_PCM_DAISYNC_PIN,
				 GPIO_PCM_DAISYNC_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_PCM_DAISYNC_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_PCM_DAISYNC_PIN, GPIO_OUT_ZERO);
		WMT_DBG_FUNC("PCM deinit (out 0) \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on PCM Group\n",
			      state);
		break;
	}

#else
	WMT_INFO_FUNC("GPIO_COMBO_PCM_CLK not defined\n");
#endif

	return 0;
}

INT32 wmt_plat_i2s_ctrl(ENUM_PIN_STATE state)
{
#ifndef FM_ANALOG_INPUT
	switch (state) {
	case PIN_STA_INIT:
	case PIN_STA_MUX:
		mt_set_gpio_mode(GPIO_COMBO_I2S_CK_PIN,
				 GPIO_COMBO_I2S_CK_PIN_M_I2S0_CK);
		mt_set_gpio_mode(GPIO_COMBO_I2S_WS_PIN,
				 GPIO_COMBO_I2S_WS_PIN_M_I2S0_WS);
		mt_set_gpio_mode(GPIO_COMBO_I2S_DAT_PIN,
				 GPIO_COMBO_I2S_DAT_PIN_M_I2S0_DAT);
		WMT_DBG_FUNC("I2S init (mode_03, I2S0) \n");
		break;
	case PIN_STA_IN_L:
	case PIN_STA_DEINIT:
		mt_set_gpio_mode(GPIO_COMBO_I2S_CK_PIN,
				 GPIO_COMBO_I2S_CK_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_I2S_CK_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_COMBO_I2S_CK_PIN, GPIO_OUT_ZERO);

		mt_set_gpio_mode(GPIO_COMBO_I2S_WS_PIN,
				 GPIO_COMBO_I2S_WS_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_I2S_WS_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_COMBO_I2S_WS_PIN, GPIO_OUT_ZERO);

		mt_set_gpio_mode(GPIO_COMBO_I2S_DAT_PIN,
				 GPIO_COMBO_I2S_DAT_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_COMBO_I2S_DAT_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_COMBO_I2S_DAT_PIN, GPIO_OUT_ZERO);
		WMT_DBG_FUNC("I2S deinit (out 0) \n");
		break;
	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on I2S Group\n",
			      state);
		break;
	}
#else
	WMT_INFO_FUNC("FM analog mode is set, skip I2S GPIO settings\n");
#endif

	return 0;
}

INT32 wmt_plat_sdio_pin_ctrl(ENUM_PIN_STATE state)
{
#if 0
	switch (state) {
	case PIN_STA_INIT:
	case PIN_STA_MUX:
/* configure sdio bus pins, pull up enable. */
		WMT_DBG_FUNC("SDIO init (pu) \n");
		break;

	case PIN_STA_DEINIT:
/* configure sdio bus pins, pull down enable. */
		WMT_DBG_FUNC("SDIO deinit (pd) \n");
		break;

	default:
		WMT_WARN_FUNC("Warnning, invalid state(%d) on SDIO Group\n",
			      state);
		break;
	}
#endif
	return 0;
}

static INT32 wmt_plat_gps_sync_ctrl(ENUM_PIN_STATE state)
{
#ifdef GPIO_GPS_SYNC_PIN
	switch (state) {
	case PIN_STA_INIT:
	case PIN_STA_DEINIT:
		mt_set_gpio_mode(GPIO_GPS_SYNC_PIN, GPIO_GPS_SYNC_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_GPS_SYNC_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_GPS_SYNC_PIN, GPIO_OUT_ZERO);
		break;

	case PIN_STA_MUX:
		mt_set_gpio_mode(GPIO_GPS_SYNC_PIN,
				 GPIO_GPS_SYNC_PIN_M_GPS_SYNC);
		break;

	default:
		break;
	}
#endif
	return 0;
}

static INT32 wmt_plat_gps_lna_ctrl(ENUM_PIN_STATE state)
{
#ifdef GPIO_GPS_LNA_PIN
	switch (state) {
	case PIN_STA_INIT:
	case PIN_STA_DEINIT:
		mt_set_gpio_pull_enable(GPIO_GPS_LNA_PIN, GPIO_PULL_DISABLE);
		mt_set_gpio_dir(GPIO_GPS_LNA_PIN, GPIO_DIR_OUT);
		mt_set_gpio_mode(GPIO_GPS_LNA_PIN, GPIO_GPS_LNA_PIN_M_GPIO);
		mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ZERO);
		break;
	case PIN_STA_OUT_H:
		mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ONE);
		break;
	case PIN_STA_OUT_L:
		mt_set_gpio_out(GPIO_GPS_LNA_PIN, GPIO_OUT_ZERO);
		break;

	default:
		WMT_WARN_FUNC("%d mode not defined for  gps lna pin !!!\n",
			      state);
		break;
	}
	return 0;
#else
	WMT_WARN_FUNC("host gps lna pin not defined!!!\n")
	    return 0;
#endif
}

INT32 wmt_plat_wake_lock_ctrl(ENUM_WL_OP opId)
{
#ifdef CFG_WMT_WAKELOCK_SUPPORT
	static INT32 counter = 0;

	osal_lock_sleepable_lock(&gOsSLock);
	if (WL_OP_GET == opId) {
		++counter;
	} else if (WL_OP_PUT == opId) {
		--counter;
	}
	osal_unlock_sleepable_lock(&gOsSLock);
	if (WL_OP_GET == opId && counter == 1) {
		wake_lock(&wmtWakeLock);
		WMT_DBG_FUNC("after wake_lock(%d), counter(%d)\n",
			     wake_lock_active(&wmtWakeLock), counter);
	} else if (WL_OP_PUT == opId && counter == 0) {
		wake_unlock(&wmtWakeLock);
		WMT_DBG_FUNC("after wake_unlock(%d), counter(%d)\n",
			     wake_lock_active(&wmtWakeLock), counter);
	} else {
		WMT_WARN_FUNC("wakelock status(%d), counter(%d)\n",
			      wake_lock_active(&wmtWakeLock), counter);
	}
	return 0;
#else
	WMT_WARN_FUNC("host awake function is not supported.");
	return 0;

#endif
}
