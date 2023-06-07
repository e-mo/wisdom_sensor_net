# RFM69HCW
__Limitations:__

* Supply Voltage: 3.9V
* SWR: 3:1
* +20dBm duty cycle: 1%

__Specs:__

* Supply Current by mode:
	* Sleep: 0.1uA
	* Idle: 1.2uA
	* Standby: 1.25uA
	* RX: 16mA
	* TX:
		* +20dBm: 130mA
		* +17dBm: 95mA
		* +13dBm: 45mA
		* +10dBm: 33mA
		* +0dBm: 20mA
		* -1dBm: 16mA
* Frequency Range:
	* 424-510MHz(433MHz Version)
	* 890-1020MHz(915MHz Version)
* Bitrate: 
	* FSK: 1.2 - 300kbps
	* OOK: 1.2 - 32.768kbps
* FSK Deviation:(FDA) 0.6 - 300kHz
* RX Sensitivity(FSK):
	* FDA = 5kHz, BR = 1.2kbps:  -118dBm
	* FDA = 40kHz, BR = 38.4kbps: -105dBm
	* @5kHz & 1.2kbps: Set RegTestLna to 0x2D to reduce noise floor of the receiver
* RF Output power: -18dBm - +20dBm in 1dB steps

__PA  Output:__

Controlled by RegPaLevel

| Pa0On 	| Pa1On 	| Pa2On 	| Mode                                           	| Power Range   	| Pout Formula         	|
|-------	|-------	|-------	|------------------------------------------------	|---------------	|----------------------	|
| 1     	| 0     	| 0     	| PA0 Out on RFIO                                	| -18 to +13dBm 	| -18dBm + OutputPower 	|
| 0     	| 1     	| 0     	| PA1 enabled on PA_BOOST                        	| -2 to +13dBm  	| -18dBm + OutputPower 	|
| 0     	| 1     	| 1     	| PA1 and PA2 on PA_BOOST                        	| +2 to +17dBm  	| -14dBm + OutputPower 	|
| 0     	| 1     	| 1     	| PA1 and PA2 on PA_BOOST with high power config 	| +5 to +20dBm  	| -11dBm + OutputPower 	|

__High Power Settings__

| Register   	| Address 	| Value for high power 	| Value for RX or PA0 	| Description           	|
|------------	|---------	|----------------------	|---------------------	|-----------------------	|
| RegOcp     	| 0x13    	| 0x0F                 	| 0x1x                	| OCP Control           	|
| RegTestPa1 	| 0x5A    	| 0x5D                 	| 0x55                	| High power PA control 	|
| RegTestPa2 	| 0x5C    	| 0x7C                 	| 0x70                	| High power PA control 	|

__Receiver/LNA:__

LNA gain can be set with RegLna in the LnaZin bit field. When manually set, the receiver will not be able to handle FSK signals with a modulation index < 2 and an input power greater than the 1dB compression point.

Best to keep this set to AGC controlled, and if needed the gain can be read from the LnaCurrentGain bit field in RegLna.



| Register   	| Address 	| Value for high power 	| Value for RX or PA0 	| Description           	|
|------------	|---------	|----------------------	|---------------------	|-----------------------	|
| RegOcp     	| 0x13    	| 0x0F                 	| 0x1x                	| OCP Control           	|
| RegTestPa1 	| 0x5A    	| 0x5D                 	| 0x55                	| High power PA control 	|
| RegTestPa2 	| 0x5C    	| 0x7C                 	| 0x70                	| High power PA control 	|

__Modes:__

Modes are set in RegOpMode(0x01).

| ListenOn in RegOpMode 	| Mode in RegOpMode 	| Selected Mode 	| Enabled Blocks                  	|
|-----------------------	|-------------------	|---------------	|---------------------------------	|
| 0                     	| 000               	| Sleep         	| None                            	|
| 0                     	| 001               	| Standby       	| Top regulator & xtal oscillator 	|
| 0                     	| 010               	| FS            	| Frequency synthesizer           	|
| 0                     	| 011               	| TX            	| Frequency synth. & transmitter  	|
| 0                     	| 100               	| RX            	| Frequency synth. & receiver     	|
| 1                     	| X                 	| Listen        	| Section 4.3 in datasheet        	|

__Listen Mode:__

ListenOn can be set in RegOpMode to 1 while in standby. This causes the radio to remain mostly in idle mode, running only the RC oscillator. It will wake up at preset intervals, and if no signal is detected, will return to idle. ListenResolX and ListenCoefX can be set in RegListen to determine this time period. Detection of a signal is based on the RssiThreshold setting.

| ListenResolX 	| Min Duration(ListenCoef = 1) 	| Max Duration(ListenCoef = 255) 	|
|--------------	|------------------------------	|--------------------------------	|
| 01           	| 64us                         	| 16ms                           	|
| 10           	| 4.1ms                        	| 1.04s                          	|
| 11           	| 0.26s                        	| 67s                            	|

