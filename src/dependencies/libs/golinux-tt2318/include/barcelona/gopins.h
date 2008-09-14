/* include/barcelona/gopins.h
 *
 * one line to give the program's name and an idea of what it does.
 *
 * Copyright (C) 2005,2006,2007 TomTom BV <http://www.tomtom.com/>
 * Authors: Dimitry Andric <dimitry.andric@tomtom.com>
 *          Jeroen Taverne <jeroen.taverne@tomtom.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_GOPINS_H
#define __INCLUDE_BARCELONA_GOPINS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined (__KERNEL__) || (__BOOTLOADER__)

typedef unsigned short gopin_t;

struct gopins {
	/* Just to find the address of the first pin */
	gopin_t FIRST;

	/* Type detection */
	gopin_t TYPE_MAIN_ID0;
	gopin_t TYPE_MAIN_ID1;
	gopin_t TYPE_MAIN_ID2;
	gopin_t TYPE_NEC_LCD;
	gopin_t TYPE_SUB_ID0;
	gopin_t TYPE_SUB_ID1;

	/* SD Card Interface */
	gopin_t SD_PWR_ON;
	gopin_t SDCLK;
	gopin_t SDCMD;
	gopin_t SDDATA0;
	gopin_t SDDATA1;
	gopin_t SDDATA2;
	gopin_t SDDATA3;
	gopin_t WP_SD;
	gopin_t CD_SD;
	gopin_t SW_SD;
	gopin_t EN_SD;
	gopin_t MOVI_PWR_ON;
	gopin_t PULLUP_SD;
	
	/* HSMMC Interface */
	gopin_t HS_SDCLK;
	gopin_t	HS_SDCMD;
	gopin_t	HS_SDDATA0;
	gopin_t	HS_SDDATA1;
	gopin_t	HS_SDDATA2;
	gopin_t	HS_SDDATA3;
	gopin_t	HS_SDDATA4;
	gopin_t	HS_SDDATA5;
	gopin_t	HS_SDDATA6;
	gopin_t	HS_SDDATA7;
	gopin_t HS_MOVI_PWR_ON;

	/* GPS interface */
	gopin_t GPS_RESET;
	gopin_t GPS_ON;
	gopin_t GPS_1PPS;
	gopin_t GPS_REPRO;
	gopin_t TXD_GPS;
	gopin_t RXD_GPS;
	gopin_t RTS_GPS;
	gopin_t CTS_GPS;
	gopin_t GPS_POWERON_OUT;

	/* Harddisk interface */
	gopin_t HDD_LED;
	gopin_t HDD_PWR_ON;
	gopin_t HDD_RST;
	gopin_t HDD_BUF_EN;
	gopin_t HDD_IRQ;
	gopin_t HDD_DRQ;
	gopin_t HDD_DACK;
	gopin_t HDD_CS;

	/* Power management */
	gopin_t ON_OFF;
	gopin_t IGNITION;
	gopin_t ACPWR;
	gopin_t PWR_RST;
	gopin_t CHARGEFAULT;
	gopin_t CHARGE_OUT;
	gopin_t CHARGING;
	gopin_t AIN4_PWR;
	gopin_t BATT_TEMP_OVER;
	gopin_t USB_HP;
	gopin_t USB_SUSPEND_OUT;
	gopin_t USB_PWR_BYPASS;
	gopin_t PWR_MODE;
	gopin_t LOW_DC_VCC;

	/* Docking */
	gopin_t SW_SCL;
	gopin_t SW_SDA;
	gopin_t DOCK_INT;
	gopin_t DOCK_PWREN;
	gopin_t DOCK_SENSE;
	gopin_t DOCK_DESK_SENSE;
	gopin_t DOCK_CRIB_SENSE;
	gopin_t DOCK_VIB_SENSE;
	gopin_t DOCK_MOTOR_SENSE;
	gopin_t DOCK_RADIO_SENSE;
	gopin_t LIGHTS_DETECT;
	gopin_t CTS_DOCK;
	gopin_t RTS_DOCK;
	gopin_t TXD_DOCK;
	gopin_t RXD_DOCK;
	gopin_t RXD_DOCK_INT;
	gopin_t LINEIN_DETECT;
	gopin_t AIN_DOCK_EINT;
	gopin_t DOCK2_PWREN; /* cologne dockpower lights up USB_HOST_DETECT too, valencia USB detect doesn't */
	gopin_t TMC_POWER;

	/* Remote */
	gopin_t FSK_FFS;
	gopin_t FSK_FFE;
	gopin_t FSK_IRQ;
	gopin_t SPICLK;
	gopin_t SPIMSI;
	gopin_t SPIMSO;
	gopin_t FSK_EN;
	gopin_t FSK_CLKOUT;

	/* Accelerometer */
	gopin_t ACC_PWR_ON;
	gopin_t GYRO_EN;

	/* Sound */
	gopin_t MIC_SW;
	gopin_t AMP_ON;
	gopin_t DAC_PWR_ON;
	gopin_t MUTE_EXT;
	gopin_t NAVI_MUTE;
	gopin_t TEL_MUTE;
	gopin_t MUTE_INT;
	gopin_t HEADPHONE_DETECT;
	gopin_t EXTMIC_DETECT;
	gopin_t I2SSDO;
	gopin_t CDCLK;
	gopin_t CDCLK_12MHZ;
	gopin_t I2SSCLK;
	gopin_t I2SLRCK;
	gopin_t I2SSDI;
	gopin_t L3CLOCK;
	gopin_t L3MODE;
	gopin_t L3DATA;

	/* Buzzer */
	gopin_t BUZZER_EN;

	/* Bluetooth */
	gopin_t TXD_BT;
	gopin_t RXD_BT;
	gopin_t RTS_BT;
	gopin_t CTS_BT;
	gopin_t BT_RESET;
	gopin_t BT_MODE;
	gopin_t BT_CLKOUT;

	/* GPS_ON + BT_RESET */
	gopin_t BT_RESET_GPS_OFF;

	/* LCD */
	gopin_t VD0;
	gopin_t VD1;
	gopin_t VD2;
	gopin_t VD3;
	gopin_t VD4;
	gopin_t VD5;
	gopin_t VD6;
	gopin_t VD7;
	gopin_t VD8;
	gopin_t VD9;
	gopin_t VD10;
	gopin_t VD11;
	gopin_t VD12;
	gopin_t VD13;
	gopin_t VD14;
	gopin_t VD15;
	gopin_t VD16;
	gopin_t VD17;
	gopin_t VD18;
	gopin_t VD19;
	gopin_t VD20;
	gopin_t VD21;
	gopin_t VD22;
	gopin_t VD23;
	gopin_t VDEN;
	gopin_t VCLK;
	gopin_t LCD_VCC_PWREN;
	gopin_t LCD_BIAS_PWREN;
	gopin_t LCD_OEN;
	gopin_t HSYNC;
	gopin_t VSYNC;
	gopin_t BACKLIGHT_EN;
	gopin_t BACKLIGHT_PWM;
	gopin_t LCD_CS;
	gopin_t LCD_SCL;
	gopin_t LCD_SDI;
	gopin_t LCD_SDO;
	gopin_t LCD_RESET;
	gopin_t LCD_ID;			/* LCM flex mounted ID */

	/* USB */
	gopin_t USB_HOST_DETECT;
	gopin_t USB_PULL_EN;
	gopin_t USB_PWR_EN;
	gopin_t USB_RST;
	gopin_t USB_SUSPEND;
	gopin_t USB_EJECT;
	gopin_t USB_CLKOUT;
	gopin_t USB_DACK;
	gopin_t USB_DREQ;
	gopin_t USB_IRQ;
	gopin_t USB_VBUS;
	gopin_t USB_RESET;
	gopin_t USB_PHY_PWR_EN;

	/* IIC */
	gopin_t HW_IIC_SDA;
	gopin_t HW_IIC_SCL;

	/* Touchscreen */
	gopin_t XMON;
	gopin_t XPON;
	gopin_t YMON;
	gopin_t YPON;
	gopin_t TSDOWN;

	/* Touchpad */
	gopin_t MEP_DAT;
	gopin_t MEP_ACK;
	gopin_t MEP_CLK;
	gopin_t TOUCHPAD_SW;

	/* CPU */
	gopin_t LOW_CORE;

	/* Flex cable */
	gopin_t FLEX_ID1;
	gopin_t FLEX_ID2;

	/* Light sensor */
	gopin_t LX_EN;

	/* Camera */
	gopin_t	CAM_DPWDN;
	gopin_t	CAMRESET;
	gopin_t	CAMCLKOUT;
	gopin_t	CAMPCLK;
	gopin_t	CAMVSYNC;
	gopin_t	CAMHREF;
	gopin_t	CAMDATA0;
	gopin_t	CAMDATA1;
	gopin_t	CAMDATA2;
	gopin_t	CAMDATA3;
	gopin_t	CAMDATA4;
	gopin_t	CAMDATA5;
	gopin_t	CAMDATA6;
	gopin_t	CAMDATA7;

	/* Factory test points */
	gopin_t FACTORY_TEST_POINT;

	/* GSM */
	gopin_t GSM_SYNC;
	gopin_t GSM_WAKEUP;
	gopin_t GSM_ON;
	gopin_t GSM_OFF;
	gopin_t GSM_DL_EN;
	gopin_t GSM_PORT_SEL;
	gopin_t GSM_RING;
	gopin_t GSM_RESET;
	gopin_t GSM_RXD;
	gopin_t GSM_TXD;
	gopin_t GSM_RTS;
	gopin_t GSM_CTS;
	gopin_t GSM_DSR;
	gopin_t GSM_DTR;
	gopin_t GSM_DCD;

	/* Extra UART(s) */
	gopin_t UART_PWRSAVE;
	gopin_t UART_INTA;
	gopin_t	UART_CSA;
	gopin_t	UART_RXD_IPOD;
	gopin_t UART_INTB;
	gopin_t	UART_CSB;
	gopin_t	UART_RXD_TMC;
	gopin_t UART_RESET;
	gopin_t	UART_CLK;

	/* Dead Reckoning stuff	*/
	gopin_t	ACC_SPI_CSB;	/* Accelerometer SPI cs			*/
	gopin_t ACC_SPI_IRQ;	/* ACC irq pin	*/
	gopin_t	CMP_SPI_CSB;	/* Electronic compass SPI cs	*/
	gopin_t DR_GYRO_HPS;	/* ING300 thingie	*/

	gopin_t DR_CMP_RST;
	gopin_t EN_DR_PWR;

	/* PIC workaround */
	gopin_t PIC_DETECT;

	/* SI4710 FM Transmitter */
	gopin_t EN_FM_PWR;
	gopin_t FM_RCLK;
	gopin_t FM_RST;
	gopin_t EN_FM_RCLK;
	
	/* Debug LED's on SMDK boards */
	gopin_t LED0;
	gopin_t LED1;
	gopin_t LED2;
	gopin_t LED3;

	/* Just to find the address of the last pin */
	gopin_t LAST;
};

struct gotype {
	unsigned id;
	unsigned short caseid;
	unsigned int  backlightfreq;
	unsigned char tsxchannel;
	unsigned char tsychannel;
	unsigned char cputype;
	unsigned char btchip;
	unsigned char btusb;
	unsigned int  btspeed;
	unsigned int  btclock;
	unsigned int  btclass;
	unsigned char handsfree;
	unsigned char headsetgw;
	unsigned char a2dp;
	unsigned char lowbutton;
	unsigned char chargertype;
	unsigned char sdcard;
	unsigned char sdslot;
	unsigned char harddisk;
	unsigned char internalflash;
	unsigned char sdisharedbus;
	unsigned char hsmmcinterface;
	unsigned char tsfets;
	unsigned char needsvcclcmforiddetect;
	unsigned char tfttype;
	unsigned char gpstype;
	unsigned char gpsephemeris;
	unsigned char codectype;
	unsigned char codecmaster;
	unsigned char canrecordaudio;
	unsigned char acctype;
	unsigned char usbslavetype;
	unsigned char usbdevicehostcapable;
	unsigned char gpio_autoshutdown;
	unsigned char ohciports;
	unsigned char harddisktiming;
	unsigned char loquendo;
	unsigned char mp3;
	unsigned char regulatorforcodec;
	unsigned char tftsoftstart;
	unsigned char compass;
	unsigned char keeprtcpowered;
	unsigned char hw_i2c;
	unsigned char glautodetect;
	unsigned char gldetected;
	unsigned char picdetected;
	unsigned char pnp;
	unsigned char usbhost;
	unsigned char unusedpinlevel;
	unsigned char codecamptype;
	unsigned char aectype;
	unsigned char deadreckoning;
	unsigned char detected64mb;
	char name[32];
	char usbname[32];
	char familyname[32];
	char projectname[32];
	char btname[32];
	char requiredbootloader[8];
	char dockdev[8];
	char btdev[8];
	char gpsdev[8];
	char gprsmodemdev[8];
	struct gopins pins;
};

int IO_Init(void);

extern struct gotype gotype_current;

void IO_InitDockUART(void);
void IO_ExitDockUART(void);
void IO_PowerOff(void);
void IOP_Activate(gopin_t pin);
void IOP_Deactivate(gopin_t pin);
void IOP_GeneratePWM(gopin_t pin);
void IOP_GenerateSPI(gopin_t clockpin, gopin_t datapin, unsigned short* c, int len);
void IOP_Suspend(gopin_t pin);
void IOP_SetFunction(gopin_t pin);
void IOP_SetFunction2(gopin_t pin);
void IO_Update(void);
int IO_GetDockState(void);
int IO_CarDocked(void);
signed IOP_SetInterruptOnActivation(gopin_t pin);
signed IOP_SetInterruptOnDeactivation(gopin_t pin);
signed IOP_SetInterruptOnActivated(gopin_t pin);
signed IOP_SetInterruptOnDeactivated(gopin_t pin);
signed IOP_SetInterruptOnToggle(gopin_t pin);
signed IOP_GetGCSNumber(gopin_t pin);
signed IO_GetClkOut(gopin_t pin);
void IOP_SetInput(gopin_t pin);
int IOP_GetInput(gopin_t pin);

#ifdef __KERNEL__
signed IOP_GetInterruptNumber(gopin_t pin);
#define IO_GetGpsDevice()		(gotype_current.gpsdev)
#define IO_GetBluetoothDevice()		(gotype_current.btdev)
#define IO_GetDockDevice()		(gotype_current.dockdev)
#endif
#ifdef __BOOTLOADER__
#define IO_GetGpsDevice()		(gotype_current.gpsdev[6] - '0')
#define IO_GetBluetoothDevice()		(gotype_current.btdev[6] - '0')
#define IO_GetDockDevice()		(gotype_current.dockdev[6] - '0')
#define IO_GetDockDeviceString()	(gotype_current.dockdev)
#endif

#define IO_GetModelId()			(gotype_current.id)
#define IO_GetCaseId()			(gotype_current.caseid)
#define IO_GetModelName()		(gotype_current.name)
#define IO_GetFamilyName()		(gotype_current.familyname)
#define IO_GetProjectName()		(gotype_current.projectname)
#define IO_GetUsbName()			(gotype_current.usbname)
#define IO_GetBtName()			(gotype_current.btname)
#define IO_GetCpuType()			(gotype_current.cputype)
#define IO_GetBluetoothChip()		(gotype_current.btchip)
#define IO_HaveBluetoothUsb()		(gotype_current.btusb)
#define IO_GetBluetoothSpeed()		(gotype_current.btspeed)
#define IO_GetBluetoothClock()		(gotype_current.btclock)
#define IO_NeedsBluetoothCalibration()	(gotype_current.btclock == 16000000)
#define IO_GetBluetoothClass()		(gotype_current.btclass)
#define IO_HaveHandsfree()		(gotype_current.handsfree)
#define IO_HaveHeadsetGw()		(gotype_current.headsetgw)
#define IO_HaveA2DP()			(gotype_current.a2dp)
#define IO_HaveLoweredButton()		(gotype_current.lowbutton)
#define IO_GetChargerType()		(gotype_current.chargertype)
#define IO_HaveSdCardInterface()	(gotype_current.sdcard) 	// SD interface used (movinand or SD card)
#define IO_HaveSdSlot()			(gotype_current.sdslot)
#define IO_HaveHarddisk()		(gotype_current.harddisk)
#define IO_HaveHsMmcInterface()		(gotype_current.hsmmcinterface)
#define IO_HaveTwoConcurrentDisks()     (IO_HaveSdSlot() && IO_HaveHsMmcInterface())
#define IO_HaveAutoformat()		(IO_HaveHarddisk())
#define IO_HaveTsFets()			(gotype_current.tsfets)
#define IO_HaveGpioAutoShutdown()	(gotype_current.gpio_autoshutdown)
#define IO_NeedsVccLcmForLCMIDDetect()	(gotype_current.needsvcclcmforiddetect)
#define IO_GetTftType()			(gotype_current.tfttype)
#define IO_GetTsXChannel()		(gotype_current.tsxchannel)
#define IO_GetTsYChannel()		(gotype_current.tsychannel)
#define IO_GetGpsType()			(gotype_current.gpstype)
#define IO_HaveGpsEphemeris()		(gotype_current.gpsephemeris)
#define IO_GetCodecType()		(gotype_current.codectype)
#define IO_GetCodecMaster()		(gotype_current.codecmaster)
#define IO_GetCodecClkOut()		(IO_GetClkOut(IO_Pin(CDCLK_12MHZ) & PIN_MASK ))
#define IO_CanRecordAudio()		(gotype_current.canrecordaudio)
#define IO_GetAccType()			(gotype_current.acctype)
#define IO_GetUSBSlaveType()		(gotype_current.usbslavetype)
#define IO_GetUSBProductID()		(gotype_current.usbproductid)
#define IO_UsbOhciPortMask()		(gotype_current.ohciports)
#define IO_GetHarddiskTiming()		(gotype_current.harddisktiming)
#define IO_HaveCompass()		(gotype_current.compass)
#define IO_HaveLoquendo()		(gotype_current.loquendo)
#define IO_HaveMp3()			(gotype_current.mp3)
#define IO_HaveRegulatorForCodec()	(gotype_current.regulatorforcodec)
#define IO_HaveRxdDetect()		(IO_HasPin(RXD_DOCK_INT))
#define IO_GetRequiredBootloader()	(gotype_current.requiredbootloader)
#define IO_HaveTftSoftStart()		(gotype_current.tftsoftstart)
#define IO_GetBacklightFreq()		(gotype_current.backlightfreq)
#define IO_HaveKeepRtcPowered()		(gotype_current.keeprtcpowered)
#define IO_HaveHardwareI2C()		(gotype_current.hw_i2c)
#define IO_GetGprsModemDevice()		(gotype_current.gprsmodemdev)
#define IO_HaveGlAutoDetect()		(gotype_current.glautodetect)
#define IO_HaveGlDetected()		(gotype_current.gldetected)
#define IO_HavePicDetected()		(gotype_current.picdetected)
#define IO_HavePNP()			(gotype_current.pnp)
#define IO_HaveUsbHost()		(gotype_current.usbhost)
#define IO_HaveUsbDeviceHostCapable()   (gotype_current.usbdevicehostcapable)
#define IO_GetUnusedPinLevel()		(gotype_current.unusedpinlevel)
#define IO_GetCodecAmpType()		(gotype_current.codecamptype)
#define IO_GetAecType()			(gotype_current.aectype)
#define IO_HaveDeadReckoning()		(gotype_current.deadreckoning)
#define IO_HaveDetected64MB()			(gotype_current.detected64mb)

#define IO_HaveBluetooth()		(IO_HasPin(TXD_BT))
#define IO_HaveHeadphoneConnector()	(IO_HasPin(HEADPHONE_DETECT))
#define IO_HaveSpeaker()		(IO_HasPin(AMP_ON))
#define IO_HaveLightSensor()		(IO_HasPin(LX_EN))
#define IO_HaveTS()			(IO_HasPin(TSDOWN))
#define IO_HaveTP()			(IO_HasPin(TOUCHPAD_SW))
#define IO_HaveCamera()			(IO_HasPin(CAM_DPWDN))
#define IO_HaveBuzzer()			(IO_HasPin(BUZZER_EN))
#define IO_HaveIoExpander()		(IO_HasPin(SW_SDA))
#define IO_HaveGprsModem()		(IO_HasPin(GSM_ON))
#define IO_HaveExternalUart()		(IO_HasPin(UART_INTA))
#define IO_HaveDoubleExternalUart()	(IO_HasPin(UART_INTB))
#define IO_HaveSingleExternalUart()	(IO_HaveExternalUart() && (!IO_HaveDoubleExternalUart()))
#define IO_GetExternalUartClkOut()	(IO_GetClkOut (IO_Pin(UART_CLK)&PIN_MASK))
#define IO_HaveRemote()			(IO_HasPin(FSK_EN))
#define IO_HaveUsbBusPowered()		(IO_HasPin(USB_PWR_BYPASS))
#define IO_HaveRealShutdown()		(IO_HasPin(PWR_RST))
#define IO_HaveDocking()		(IO_HasPin(DOCK_SENSE))
#define IO_HaveDockPower()		(IO_HasPin(DOCK_PWREN))
#define IO_HaveGpioHeadlights()		(IO_HasPin(LIGHTS_DETECT))
#define IO_HaveAlkaline()		(IO_HasPin(CAM_DPWDN))
#define IO_HaveAnalogDockInput()	(IO_HasPin(CAM_DPWDN))
#define IO_HaveNewcastleDock()		(IO_HasPin(DOCK_RADIO_SENSE))
#define IO_HaveInternalFlash()		(gotype_current.internalflash)
#define IO_HaveSdMovinandShared()	(gotype_current.sdisharedbus)
#define IO_PlayStartupSound()		(!IO_HaveBuzzer())
#define IO_HaveFMTransmitter()		(IO_HasPin(FM_RST))
#define IO_HaveForteMediaDSP()		(IO_HasPin(VP_PWRDN))

#define IO_Pin(x)			(gotype_current.pins.x)

#define IO_Activate(x)				IOP_Activate(IO_Pin(x))
#define IO_Deactivate(x)			IOP_Deactivate(IO_Pin(x))
#define IO_GeneratePWM(x)			IOP_GeneratePWM(IO_Pin(x))
#define IO_GenerateSPI(a,b,c,d)			IOP_GenerateSPI(IO_Pin(a),IO_Pin(b),(c),(d))
#define IO_Suspend(x)				IOP_Suspend(IO_Pin(x))
#define IO_SetFunction(x)			IOP_SetFunction(IO_Pin(x))
#define IO_SetFunction2(x)			IOP_SetFunction2(IO_Pin(x))
#define IO_SetInterruptOnActivation(x)		IOP_SetInterruptOnActivation(IO_Pin(x))
#define IO_SetInterruptOnDeactivation(x)	IOP_SetInterruptOnDeactivation(IO_Pin(x))
#define IO_SetInterruptOnActivated(x)		IOP_SetInterruptOnActivated(IO_Pin(x))
#define IO_SetInterruptOnDeactivated(x)		IOP_SetInterruptOnDeactivated(IO_Pin(x))
#define IO_SetInterruptOnToggle(x)		IOP_SetInterruptOnToggle(IO_Pin(x))
#define IO_SetInput(x)				IOP_SetInput(IO_Pin(x))
#define IO_GetInput(x)				IOP_GetInput(IO_Pin(x))
#define IO_GetInterruptNumber(x)		IOP_GetInterruptNumber(IO_Pin(x))
#define IO_GetGCSNumber(x)			IOP_GetGCSNumber(IO_Pin(x))
#define IO_GetPAForGCSNumber(x)			((unsigned long) (x) * 0x8000000)
#define IO_HasPin(x)				(IO_Pin(x) != 0)

/* GO CPU types */
#define GOCPU_UNDEFINED		0	/* Undefined */
#define GOCPU_S3C2410		1	/* Samsung S3C2410 */
#define GOCPU_S3C2440		2	/* Samsung S3C2440 */
#define GOCPU_S3C2442		3	/* Samsung S3C2442 */
#define GOCPU_S3C2412		4	/* Samsung S3C2412 */
#define GOCPU_S3C2443		5	/* Samsung S3C2443 */

/* GO TFT types */
#define GOTFT_UNDEFINED		0	/* Undefined */
#define GOTFT_NEC_NL2432HC22	2	/* NEC NL2432HC22-22B */
#define GOTFT_SAMSUNG_LTV350	3	/* Samsung LTV350QV */
#define GOTFT_SAMSUNG_LTP400	4	/* Samsung LTP400WQ */
#define GOTFT_SAMSUNG_LTE246QV	5	/* Samsung LTE246QV */
#define GOTFT_SAMSUNG_LTE430WQ	6	/* Samsung LTE430WQ */
#define GOTFT_SHARP_LQ043T1	7	/* Sharp LQ043T1DG01 */
#define GOTFT_SAMSUNG_LMS350GF	8	/* Samsung LMS350GF */
#define GOTFT_SHARP_LQ035Q1DG   9       /* Sharp LQ035Q1DG */

/* GO GPS types */
#define GOGPS_UNDEFINED		0	/* Undefined */
#define GOGPS_SIRF1		1	/* Sirf 1 */
#define GOGPS_SIRF2		2	/* Sirf 2 */
#define GOGPS_SIRF3		3	/* Sirf 3 */
#define GOGPS_GL		128	/* Global Locate */

/* GO BT chip types */
#define GOBT_NONE		0	/* No bluetooth chip */
#define GOBT_BC3		3	/* BlueCore 3 */
#define GOBT_BC4		4	/* BlueCore 4 */

/* GO Codec types */
#define GOCODEC_NONE		0	/* No codec */
#define GOCODEC_WM8711		1	/* Wolfson WM8711 */
#define GOCODEC_WM8971		2	/* Wolfson WM8971 */

/* GO Codec types	*/
#define GOCODECCFG_SLAVE		0	/* Codec is slave */
#define GOCODECCFG_EXTERNAL_MASTER	1	/* Codec is master and is fed with external 12 MHZ clock */
#define GOCODECCFG_INTERNAL_MASTER	2	/* Codec is master but fed from S3C24XX internal iis clock */

/* GO Acc types */
#define GOACC_NONE		0
#define GOACC_MXR2312		1
#define GOACC_MXR3999		2
#define GOACC_MXM9301		3
#define GOACC_MXR9500		4

/* GO USB slave types */
#define GOUSB_S3C24XX		0
#define GOUSB_TUSB6250		1
#define GOUSB_NET2272		2
#define GOUSB_S3C2443		3

/* Set if device is having a USB port that is device-capable only (no prague over USB support) */
#define IO_HaveUSBDeviceOnly()		((IO_GetUSBSlaveType() == GOUSB_TUSB6250) || (IO_GetUSBSlaveType() == GOUSB_NET2272))

/* GO Charger tyoes */
#define GOCHARGER_LTC1733	0
#define GOCHARGER_LTC3455	1

/* GO dock types */
#define GODOCK_NONE		0
#define GODOCK_WINDSCREEN	1
#define GODOCK_CRIB		2
#define GODOCK_DESK		3
#define GODOCK_VIB		4
#define GODOCK_MOTOR		5
#define GODOCK_RADIO		6

/* GO case types */
#define GOCASE_MALAGA		0	/* Barcelona, M100,300,500 */
#define GOCASE_BILBAO		1	/* Bilbao */
#define GOCASE_GLASGOW		2	/* Glasgow and Aberdeen */
#define GOCASE_VALENCIA		3	/* V510,710 */
#define GOCASE_EDINBURGH	4	/* Edinburgh */
#define GOCASE_NEWCASTLE	5	/* Newcastle */
#define GOCASE_LIMERICK		6	/* Limerick */
#define GOCASE_MILAN		7	/* Milan */
#define GOCASE_LISBON		8	/* Lisbon */

/* Differential Amp Type */ 
#define GOCODECAMP_SINGLEENDED		0	/* Codec and Amp are used in single ended mode */
#define GOCODECAMP_DIFFERENTIAL_HW	1	/* Codec and Amp are used in differential mode */
#define GOCODECAMP_DIFFERENTIAL_SW	2	/* Amp is used in differential mode, codec output is made differential in software */

/* AEC type */
#define GOAEC_ACOUSTIC		0
#define GOAEC_FM		1

#else /* (__KERNEL__) || (__BOOTLOADER__) */
#error "Never include this header from user space."
#endif /* (__KERNEL__) || (__BOOTLOADER__) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_GOPINS_H */

/* EOF */
