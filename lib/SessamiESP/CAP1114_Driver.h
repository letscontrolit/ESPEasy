/***************************************************
 Title : Driver class for CAP1114
 Modified by : Kaz Wong
 Begin Date : 7 Dec 2016
 Datasheet Link : http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1114.pdf
 Description : Class for read/write register for CAP1114
 ****************************************************/
#ifndef CAP1114_DRIVER_H_
#define CAP1114_DRIVER_H_

#include "I2CSensor.h"
#include "CAP1114_Registers.h"

namespace CAP1114 {

#define SLIPOS_CS8  0x02
#define SLIPOS_CS9  0x12
#define SLIPOS_CS10 0x22
#define SLIPOS_CS11 0x32
#define SLIPOS_CS12 0x42
#define SLIPOS_CS13 0x52
#define SLIPOS_CS14 0x62

#define MSControl_INT		1
#define MSControl_PWR_LED	2
#define MSControl_DSLEEP	16
#define MSControl_SLEEP		32
#define MSControl_DEACT		64
enum class LED {
	LED1 = B0,
	LED2 = B1,
	LED3 = B2,
	LED4 = B3,
	LED5 = B4,
	LED6 = B5,
	LED7 = B6,
	LED8 = B7,
	LED9 = HB | B0,
	LED10 = HB | B1,
	LED11 = HB | B2,
};

#define CS_1		1
#define CS_2		2
#define CS_3		4
#define CS_4		8
#define CS_5		16
#define CS_6		32
#define CS_DOWN		64
#define CS_UP		128
#define CS_7		257
#define CS_8		258
#define CS_9		260
#define CS_10		264
#define CS_11		272
#define CS_12		288
#define CS_13		320
#define CS_14		284

enum class Slide {
	PH = B0, TAP = B1, DOWN = B2, UP = B3, RESET = B5, MULT = B6, LID = B7
};

enum class SDelta {
	CS1 = 0x10,
	CS2 = 0x11,
	CS3 = 0x12,
	CS4 = 0x13,
	CS5 = 0x14,
	CS6 = 0x15,
	CS7 = 0x16,
	CS8 = 0x17,
	CS9 = 0x18,
	CS10 = 0x19,
	CS11 = 0x1A,
	CS12 = 0x1B,
	CS13 = 0x1C,
	CS14 = 0x1D
};

#define	SDelta_CS1	0x10
#define	SDelta_CS2	0x11
#define	SDelta_CS3	0x12
#define	SDelta_CS4	0x13
#define	SDelta_CS5	0x14
#define	SDelta_CS6	0x15
#define	SDelta_CS7	0x16
#define	SDelta_CS8	0x17
#define	SDelta_CS9	0x18
#define	SDelta_CS10	0x19
#define	SDelta_CS11	0x1A
#define	SDelta_CS12	0x1B
#define	SDelta_CS13	0x1C
#define	SDelta_CS14	0x1D
	
enum class LEDLinking {
	CS1 = B0,
	CS2 = B1,
	CS3 = B2,
	CS4 = B3,
	CS5 = B4,
	CS6 = B5,
	CS7 = B6,
	Group = B7
};

enum class IntEn {
	S1 = B0,
	S2 = B1,
	S3 = B2,
	S4 = B3,
	S5 = B4,
	S6 = B5,
	S7 = B6,
	G = B7,
	GPIO1 = HB | B0,
	GPIO2 = HB | B1,
	GPIO3 = HB | B2,
	GPIO4 = HB | B3,
	GPIO5 = HB | B4,
	GPIO6 = HB | B5,
	GPIO7 = HB | B6,
	GPIO8 = HB | B7
};

enum class GPIODir {
	LED1_DIR = B0,
	LED2_DIR = B1,
	LED3_DIR = B2,
	LED4_DIR = B3,
	LED5_DIR = B4,
	LED6_DIR = B5,
	LED7_DIR = B6,
	LED8_DIR = B7
};

#define	LEDBhv1		B0
#define	LEDBhv2		B2
#define	LEDBhv3		B4
#define	LEDBhv4		B6
#define	LEDBhv5		(0x100 | B0)
#define	LEDBhv6		(0x100 | B2)
#define	LEDBhv7		(0x100 | B4)
#define	LEDBhv8		(0x100 | B6)
#define	LEDBhv9		(0x200 | B0)
#define	LEDBhv10	(0x200 | B2)
#define	LEDBhv11_C	(0x200 | B4)
#define	LEDBhv11_A	(0x200 | B6)

#define	P1MIN	B0
#define	P1MAX	B4
#define	P2MIN	(0x100 | B0)
#define	P2MAX	(0x100 | B4)
#define	BRMIN	(0x200 | B0)
#define	BRMAX	(0x200 | B4)
#define	DRMIN	(0x300 | B0)
#define	DRMAX	(0x300 | B4)

#define	Thresh_CS1		0x30
#define	Thresh_CS2		0x31
#define	Thresh_CS3		0x32
#define	Thresh_CS4		0x33
#define	Thresh_CS5		0x34
#define	Thresh_CS6		0x35
#define	Thresh_CS7		0x36
#define	Group_Thresh	0x37

class CAP1114_Driver: private I2CSensor {
public:
	//protected:
	CAP1114_Driver();
	virtual ~CAP1114_Driver();
	virtual State initWireI2C();

	/*
	 73h, 74h RW - LED Output CHItrol Registers
	 LED8_DR | LED7_DR | LED6_DR | LED5_DR | LED4_DR | LED3_DR | LED2_DR | LED1_DR
	 - | - | - | - | - | LED11_DR | LED10_DR | LED9_DR
	 */
private:
	static uint8_t st_73, st_74;
protected:
	void UpdateLED(uint8_t *status1 = 0, uint8_t *status2 = 0);
	State GetLED(LED b) const;
	void SetLED(uint16_t value);
	void SetLED(LED b, State st);

	/*
	 00h RW - Main Status CHItrol Registers
	 - | DEACT | SLEEP | DSLEEP | - | - | PWR_LED | INT
	 */
private:
	static uint8_t st_00;
protected:
	void UpdateMSControl(uint8_t *status = 0);
	State GetMSControl(uint8_t b) const;
	void SetMSControl(uint8_t value);
	void SetMSControl(uint8_t b, State st);

	/*
	 03h, 04h R - Button Status Registers
	 UP | DOWN | CS6(DOWN) | CS5(POWER) | CS4(UP) | CS3(LEFT) | CS2(MIDDLE) | CS1(PROXIMITY)
	 CS14 | CS13 | CS12 | CS11 | CS10 | CS9 | CS8 | CS7(RIGHT)
	 */
private:
	static uint8_t st_03, st_04;
protected:
	void UpdateCS(uint8_t *status1 = 0, uint8_t *status2 = 0);
	uint16_t GetCS() const;
	State GetCS(uint16_t b) const;

	/*
	 0Fh R-C - Group Status Registers
	 LID | MULT | RESET | - | UP | DOWN | TAP | PH
	 */
private:
	static uint8_t st_0F;
protected:
	void UpdateSlide(uint8_t *status = 0);
	uint8_t GetSlide() const;
	State GetSlide(Slide b) const;

	/*
	 06h R-C/RW - Slider Position/Volumetric Data Registers
	 - | POS[6:0]

	 Determined by POS_VOL bit (20h)
	 */
private:
	static uint8_t st_06;
protected:
	uint8_t GetSliPos();
	uint8_t GetVolData();
	uint8_t SetVolData(uint8_t);

	/*
	 10h~1Dh R - Sensor Delta Count Registers
	 10h   CS1   Proximity
	 11h   CS2   Middle
	 12h   CS3   Left
	 13h   CS4   Up
	 14h   CS5   Power
	 15h   CS6   Down
	 16h   CS7   Right
	 17h   CS8   Slide 7
	 18h   CS9   Slide 6
	 19h   CS10 Slide 5
	 1Ah   CS11 Slide 4
	 1Bh   CS12 Slide 3
	 1Ch   CS13 Slide 2
	 1Dh   CS14 Slide 1
	 */
protected:
	uint8_t GetSDelta(SDelta b);
	uint8_t GetSDelta(int8_t b);
	
	/*
	 1Fh RW - Data Sensitivity Registers
	  - | DELTA_SENSE[2:0] | BASE_SHIFT[3:0]
	 */
private:
	static uint8_t st_1F_ds, st_1F_bs;
protected:
	 uint8_t GetDeltaSen();
	 uint8_t GetBaseShift();
	 
	 void SetDeltaSen(uint8_t);
	 void SetBaseShift(uint8_t);

	/*
	 20h RW - Configuration Registers
	 TIMEOUT | POS_VOL | BLK_DIG_NOISE | BLK_ANA_NOISE | MAX_DUR_EN_B | RPT_EN_B | MAX_DUR_EN_G | RPT_EN_G
	 */
private:
	static uint8_t st_20;
protected:
	void UpdateConfigRegister();
	bool GetTimeOutConfig();bool GetPosVolConfig();bool GetNoiseConfig(bool *th,
	bool *flag); //noise threshold, noise flag
	bool GetMaxDurCalConfig(bool *sg, bool *gp); //nHI grouped, grouped
	bool GetRptRateConfig(bool *sg, bool *gp); //nHI grouped, grouped

	void SetTimeOutConfig(State);
	void SetPosVolConfig(State);
	void SetNoiseConfig(State th, State flag); //noise threshold, noise flag
	void SetMaxDurCalConfig(State sg, State gp); //nHI grouped, grouped
	void SetRptRateConfig(State sg, State gp); //nHI grouped, grouped

	/*
	 23h, 24h RW - Group Configuration Registers
	 RPT_RATE_PH[3:0] | M_PRESS[3:0]
	 MAX_DUR_G[3:0] | RPT_RATE_SL[3:0]
	 */
protected:
	void GetGroupConfig(uint8_t *rpt_ph = 0, uint8_t *m_press = 0,
			uint8_t *max_dur = 0, uint8_t *rpt_sl = 0);

	/*
	 27h, 28h RW - Interrupt Enable Registers
	 G_INT_EN | S7_INT_EN | S6_INT_EN | S5_INT_EN | S4_INT_EN | S3_INT_EN | S2_INT_EN | S1_INT_EN
	 GPIO8_INT_EN | GPIO7_INT_EN | GPIO6_INT_EN | GPIO5_INT_EN | GPIO4_INT_EN | GPIO3_INT_EN | GPIO2_INT_EN | GPIO1_INT_EN
	 */
private:
	static uint8_t st_27, st_28;
protected:
	void UpdateIntEn(uint8_t *status1 = 0, uint8_t *status2 = 0);
	uint16_t GetIntEn() const;
	State GetIntEn(IntEn b) const;
	void SetIntEn(uint8_t value);
	void SetIntEn(IntEn b, State st);

	/*
	 26h, 46h RW - Calibration Activate Registers
	 Group | CS7 | CS6 | CS5 | CS4 | CS3 | CS2 | CS1
	 - | CS14 | CS13 | CS12 | CS11 | CS10 | CS9 | CS8
	 */
	void SetCalAct(uint8_t);
	void SetGroupCalAct(uint8_t);

	/*
	 2Ah RW - Multiple Touch Configuration Register
	 MULT_BLK_EN | - | - | - | B_MULT_T[1:0] | G_MULT_T[1:0]
	 */
protected:
	uint8_t GetMTConfig();
	//uint8_t GetMTCS();
	//uint8_t GetMTSlide();
	void SetMTConfig(uint8_t value);
	//void SetMTCS(uint8_t);
	//void SetMTSlide(uint8_t);
	
	/*
	 30h~37h R/W - Sensor Threshold Registers
	 30h   CS1   Sensor 1
	 31h   CS2   Sensor 2
	 32h   CS3   Sensor 3
	 33h   CS4   Sensor 4
	 34h   CS5   Sensor 5
	 35h   CS6   Sensor 6
	 36h   CS7   Sensor 7
	 37h   CS8   Group
	 */
protected:
	uint8_t GetThresh(uint8_t sensor);
	void SetThresh(uint8_t sensor, uint8_t value);

	/*
	 3Eh RW - Slider Velocity Configuration Registers
	 ACC_INT_EN | MAX_INT[2:0] | SLIDE_TIME[1:0] | RPT_SCALE[1:0]
	 */
	void SetAccelEN(State);
	void SetIntNum(uint8_t);
	void SetSliTime(uint8_t);
	void SetRptScale(uint8_t);
	
	/*
	 42h RW - Proximity Control Registers
	 CS1_PROX | PROX_SUM | - | PROX_AVG[1:0] | PROX_D_SENSE[2:0]
	 */
private:
	static uint8_t st_42;
protected:
	void UpdateProxRegister();
	bool GetProxEN();
	bool GetProxSum();
	uint8_t GetProxAvg();
	uint8_t GetProxSen();

	void SetProxEN(State);
	void SetProxSum(State);
	void SetProxAvg(uint8_t);
	void SetProxSen(uint8_t);

	/*
	 70h RW - LED/GPIO Direction Register
	 LED8_DIR | LED7_DIR | LED6_DIR | LED5_DIR | LED4_DIRS4 | LED3_DIR | LED2_DIRS2 | LED1_DIR1
	 */
protected:
	uint8_t GetGPIODir();
	State GetGPIODir(GPIODir b);
	void SetGPIODir(uint8_t value);
	void SetGPIODir(GPIODir b, State st);
	
	/*
	 71h RW - LED/GPIO Output Type Register
	 LED8_OT | LED7_OT | LED6_OT | LED5_OT | LED4_OT | LED3_OT | LED2_OT | LED1_OT
	 */
protected:
	void SetOutputType(uint8_t value);

	/*
	 75h, 76h RW - LED Polarity Registers
	 LED8 | LED7 | LED6 | LED5 | LED4 | LED3 | LED2 | LED1
	 - | - | - | - | - | LED11 | LED10 | LED9
	 */
protected:
	uint8_t SetLEDPol(uint8_t value);
	 
	/*
	 80h RW - Sensor LED Linking Registers
	 Group | CS7 | CS6 | CS5 | CS4 | CS3 | CS2 | CS1
	 */
protected:
	uint8_t GetLEDLinking();
	State GetLEDLinking(LEDLinking b);
	void SetLEDLinking(uint8_t value);
	void SetLEDLinking(LEDLinking b, State st);
	
	/*
	 81h, 82h, 83h RW - LED Behavior Registers
	 LED4_CTL[1:0] | LED3_CTL[1:0] | LED2_CTL[1:0] | LED1_CTL[1:0]
	 LED8_CTL[1:0] | LED7_CTL[1:0] | LED6_CTL[1:0] | LED5_CTL[1:0]
	 LED11_ATL[1:0] | LED11_CTL[1:0] | LED10_CTL[1:0] | LED9_CTL[1:0]
	 */
protected:
	uint8_t GetLEDBehavior();
	void SetLEDBehavior(unsigned int b, uint8_t value);
	
	/*
	84h RW - LED Pulse 1 Period Registers
	 TRIG | PER6 | PER5 | PER4 | PER3 | PER2 | PER1 | PER0
	*/
protected:
	void GetPulse1P();
	void GetPulse1Trig();
	void SetPulse1P();
	void SetPulse1Trig();
	
	/*
	85h RW - LED Pulse 2 Period Registers
	 - | PER6 | PER5 | PER4 | PER3 | PER2 | PER1 | PER0
	*/
protected:
	void GetPulse2P();
	void SetPulse2P();
	
	/*
	86h RW - LED Breathe Period Registers
	 - | PER6 | PER5 | PER4 | PER3 | PER2 | PER1 | PER0
	*/
protected:
	void GetBreatheP();
	void SetBreatheP();
	
	/*
	 90h, 91h, 92h, 93h RW - LED Pulse and Breathe Duty Cycle Registers
	 P1_MAX[3:0] | P1_MIN[3:0]
	 P2_MAX[3:0] | P2_MIN[3:0]
	 BR_MAX[3:0] | BR_MIN[3:0]
	 DR_MAX[3:0] | DR_MIN[3:0]
	 */
protected:
	uint8_t GetLEDDC();
	void SetLEDDC(unsigned int b, uint8_t value);
	
};
}
#endif
