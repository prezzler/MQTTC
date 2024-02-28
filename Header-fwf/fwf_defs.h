//==================================================================================
//  File Name   FWF_defs.h
//  Resp.       P.Kaever
//  Header zur Steuerung der bedingten uebersetzung
//  1.0         03-2013 Basiskonfiguration
//  1.1         02-2013 Debug Features
//  1.2         07-2014 mit unterschiedlichen Hardwarekonstellationen
//==================================================================================
#ifndef __FWF_DEFS_H
#define __FWF_DEFS_H

// Board und Debugging ===============================================
#define BOARD_PROPOX_CM4						0x01
#define BOARD_DISCOVERY                         0x02
#define BOARD_NUCLEO_CM4F4x9                    0x03

#define BOARD_FWF_GrunzeModulCM4				0x04
#define BOARD_FWF_DRESDYN                       0x05
#define BOARD_FWF_PM80                          0x06
//===================================================================
#define BOARD          			 			    BOARD_FWF_GrunzeModulCM4

// == DEBUG EINSTELLUNGEN ===============================================================
#define FWF_DBG_VERBOSITY               							1   // Standard Ausgaben (mit File, Funktion, Line  1: keine Ausgabe 2: mit Ausgabe 5: mit Zeit)
#define FWF_DEBUG_IP												1
// FWF_DBG_SEMIHOSTING
/// Ok: CooCox mit StLink + BOARD_NUCLEO_CM4F4x9
//  Vorsicht: CooCox mit StLink V3Mini >>>> funktioniert nicht mit BOARD_FWF_DRESDYN
#define FWF_DBG_SEMIHOSTING             							0	// Debug Ausgaben auf Konsole
//=====================================================================================================


//=====================================================================================================
//=====================================================================================================
//=====================================================================================================
// Applikationsspezifische Konfiguration - 1 aus n waehlen und auf 1 setzen
// APPLICATION DEFINE BEGIN

    // FWF Generic Funktionspakete
    #define FWF_APP_PIEZO_CONTROLLER_CM4_FWKT2016                   0       // Ethernet IO Piezo-Antriebe FWKT 2016
    #define FWF_APP_PIEZO_CONTROLLER_CM4_XFEL                       0       // Ethernet IO Piezo-Antriebe XFEL
    #define FWF_APP_PIEZO_CONTROLLER_CM4_FWKT2021                   0       // Ethernet IO Piezo-Antriebe FWKT 2021

    #define FWF_APP_PM80_PROTO                                      0       // Pulsmodul PM80 Prototyp
    #define FWF_APP_LPM_2022                                        0       // Pulsmodul PM80 und PM45 Version 2022 Serie

    #define FWF_APP_DRESDYN_INTERLOCK                               0       // Dresdyn Interlock Knoten
    #define FWF_APP_DRESDYN_Fanout_and_MEASURE_CLOCK              	0       // Dresdyn mit Messtakterzeugung und - ueberwachung
    #define FWF_APP_DRESDYN_SENSORNODE                              0       // Dresdyn Sensorknoten
    #define FWF_APP_DRESDYN_ANGLES                					0       // Dresdyn Winkelgeberknoten mit Encoderueberwachung, Kameratrigger-Erzeugung; Wichtig: BOARD_FWF_DRESDYN
    #define FWF_APP_DRESDYN_HEATINGNODE                             0       // Dresdyn mit Heizungsansteuerung
	#define FWF_APP_DRESDYN_TESTGENERATOR							0		// Dresdyn Zentrifugen-Teststand Testgenerator

    #define FWF_APP_SENSYS_SINAMICS_ENCODER2TRIGGER_PULSE           0       // Flexiplant/SENSYS mit Sinamics FU, Encoderueberwachung, Kameratrigger-Erzeugung
    #define FWF_APP_SENSYS_PD45_ENCODER2TRIGGER_PULSE               0       // Flexiplant/SENSYS mit PD50 Ansteuerung, Encoderueberwachung, Kameratrigger-Erzeugung
    #define FWF_APP_HTW_ENCODER_to_TRIGGER_PULSE                    0       // HTW Labor Oertel mit Wenderelais fuer Rueckwaertsfahrt des Bandes

    #define FWF_APP_SINAMICSV20_FU                                  0       // Simatic Frequenzumrichter z.B. fuer DRESDYN Zentrifugentest
    #define FWF_APP_SINAMICSV20                                     0       // Simatic Frequenzumrichter ggf. + Encoderueberwachung und PWM

    #define FWF_APP_ADC_TI_ADS9224_EVAL                             0       // TestBoard fuer ADC TI_ADS9224
    #define FWF_APP_STEPPER_PHYTRON                                 0       // Schrittmotorsteuerung fuer Phytron-Treiber-Module
    #define FWF_APP_STEPPER_STMdrv                                  0       // Schrittmotorsteuerung fuer STM-Treiber-Module
    #define FWF_APP_MUON_DETECTOR                                   0       // Muonen-Detektor
    #define FWF_APP_HALLSENSOR23                                    0       // Hallsensor 2023 TMAG5170
	#define FWF_APP_PRESSURE_ANALOG									0		// Drucksensor

	#define FWF_APP_ADCDAC_ARM_CM4                                  0       // Test ADC DAC z.B. mit 32 Bit ADC
    #define FWF_APP_PIXE_2DAC_CM4                                   0       // Ethernet IO Highspeed Pixe XRCAM
    #define FWF_APP_PELTIER                                         0		// Peltier-Steuerung fuer J.Grenzer (mit EloTherm)

    #define FWF_APP_HTW_EVA_UEBUNG                                  10       // EvalBoard fuer Uebung mit LED's, Schaltern, Potentiometern MQTT
    #define FWF_APP_HW_Eval_of_Boards                               0       // Ethernet IO Tests mit SPI, Encoder, Clocks PE14 1ms Clock 500 Hz
    #define FWF_APP_HW_EVB_EIO2_CM4                                 0       // Ethernet IO Tests mit SPI, Encoder, Clocks
    #define FWF_APP_HLD_24KV_LADECONTROLLER                         0       // HLD 24 kV Ladenetzteil

// APPLICATION DEFINE END
// Alte Defines: ToDo: Bereinigen
//    #define FWF_APP_MV40                                            0       // Ethernet IO MV40 und Test der Eio4.1-Karte
// FWF_STEPPER_APP - fuer ZRT

//=====================================================================================================
//=====================================================================================================
//=====================================================================================================

//========== Spezifische Funktionalitaet ==============================================================
#define USE_TFTP_SERVER					1

// Flash on Chip
#define FWF_FLASH_USE_CM4               1

// ETHERNET
#define FWF_ETHERNET_USE                1
#define FWF_ETHERNET_IRQ_EBL			0  // ETH_DMASR: mehrere Interruptursachen, die vollstaendig behandelt werden muessen => aufwendig
#define FWF_ETHERNET_PTP_EBL			1  // PTP IEEE1588
#define FWF_ETHERNET_ISR_MESSAGE        0  // ISR zur Behandlung einer Ethernet-Nachricht

#define FWF_SEND_STARTUP_MESSAGE        1  // Automatischer Verbindungsaufbau nach Startup des uC
#define FWF_DEFAULT_SEND_CHANGE_EVENTS  1


// Info: IDE-spezifische Einstellungen ================================================================================
#if 0 // CooCoxIDE fuer
    BOARD_NUCLEO_CM4F4x9 mit STLinkV2
    BOARD_DISCOVERY      mit STLinkV2
    BOARD_PROPOX_CM4     mit JLink,
    BOARD_FWF_DRESDYN    mit JLink
#endif

#ifdef CooCoxIDE		  // definiert im Projektfile der CooCox IDE
    #define IDE_USED                            "CooCox"
	#if FWF_DBG_SEMIHOSTING
		#define DEBUG_VIA_CooCox_SEMIHOSTING        1
	#else
		#define DEBUG_VIA_CooCox_SEMIHOSTING        0
	#endif
#endif // CooCoxIDE


#ifdef FWF_STM32CubeIDE 	// definiert im Projektfile der STM32CubeIDE
    #define IDE_USED                                "STM.Cube"
	#if FWF_DBG_SEMIHOSTING
		#define DEBUG_VIA_STM32CubeIDE_SEMIHOSTING        1
	#else
		#define DEBUG_VIA_STM32CubeIDE_SEMIHOSTING        0
	#endif
#endif // STM32CubeIDE
// ENDE IDE-spezifische Einstellungen ============================================================================================



// Clock, Prozessortypen =================================================
// CLOCK Einstellungen
#define MHz1                                1000000        // 1 MHz - 1 us timer clock
#define KHz1                                   1000        // 1 KHz - 1 ms ISR

// Prozessortypen
#define uC_STM_ARM966ES                         0x1
#define uC_STM32F407VET                         0x2     /// @Propox
#define uC_STM32F407VGT                         0x3
#define uC_STM32F4XX                            0x4     /// STM32F429 oder STM32F439 Nucleo

// MAC - Einstellungen - je nach Schaltplan	fuer die Boards:
#define MAC_TXatPB                              0x01    // MAC_TXatPB:  Propox, BOARD_FWF_GrunzeModulCM4 DISCOVERY
#define MAC_TXatPGB                             0x02    // MAC_TXatPGB: Nucleo


// BOARD-spezifische Hardwarekonfiguration ============================================================================================
#if (BOARD == BOARD_NUCLEO_CM4F4x9)                // Getestet Eth-ok
    #define BOARD_NAME                              "BOARD_NUCLEO_CM4F4x9"
    #define MICROCONTROLLER_COMPILE_TYPE            uC_STM32F4XX
    #define uC_Type                                 "uC_STM32F4XX"
    #define FWF_HSE_CLOCK                          (8 * MHz1)       // HSE Clock externer Takt am HSE-Pin  Nucleo: 8    Propox: 25
    #define PLL_M                                   4
    #define PLL_N                                   168
    #define MAC_PINS                                MAC_TXatPGB     // Nucleo: MAC_TXatPGB  Propox: MAC_TXatPB
    #define FWF_DEBUG_VIA_UART3                     1               // Nucleo: 1
    #define FWF_DEBUG_VIA_UART1                     0               // Dresdyn
    #define FWF_UART1_ON_A9_A10                     0               //
	#define _FWF_USE_APP_FLASH_TYPE   				0 				// kein FWF_APP_FLASH_TYPE
#endif // BOARD_NUCLEO_CM4F4x9

#if (BOARD == BOARD_PROPOX_CM4)
    #define BOARD_NAME                              "BOARD_PROPOX_CM4"
    #define MICROCONTROLLER_COMPILE_TYPE            uC_STM32F407VGT
    #define uC_Type                                 "STM32F407VGT"
    #define FWF_HSE_CLOCK                          (25 * MHz1)       // HSE Clock externer Takt am HSE-Pin  Nucleo: 8   Propox: 25
    #define PLL_M                                   25
    #define PLL_N                                   336
    #define MAC_PINS                                MAC_TXatPB      // Nucleo: MAC_TXatPGB                  Propox: MAC_TXatPB
    #define FWF_DEBUG_VIA_UART3                     0               // Nucleo: 1
    #define FWF_DEBUG_VIA_UART1                     0               // Dresdyn
    #define FWF_UART1_ON_A9_A10                     0               //
	#define _FWF_USE_APP_FLASH_TYPE   				0 				// kein FWF_APP_FLASH_TYPE
#endif // BOARD_PROPOX_CM4

#if (BOARD == BOARD_DISCOVERY)                      // Getestet Eth-ok
    #define BOARD_NAME                              "BOARD_DISCOVERY"
    #define MICROCONTROLLER_COMPILE_TYPE            uC_STM32F407VGT
    #define uC_Type                                 "STM32F407VGT"
    #define FWF_HSE_CLOCK                          (8 * MHz1)       // HSE Clock externer Takt am HSE-Pin  Nucleo: 8    Propox: 25
    #define PLL_M                                   4
    #define PLL_N                                   168
    #define MAC_PINS                                MAC_TXatPB      // DISCOVERY: MAC_TXatPB
    #define FWF_DEBUG_VIA_UART3                     0               // DISCOVERY
    #define FWF_DEBUG_VIA_UART1                     1               // DISCOVERY
    #define FWF_UART1_ON_A9_A10                     1               // DISCOVERY
	#define _FWF_USE_APP_FLASH_TYPE   				0 				// kein FWF_APP_FLASH_TYPE
#endif // BOARD_DISCOVERY

#if (BOARD == BOARD_FWF_GrunzeModulCM4)
    #define BOARD_NAME                              "BOARD_FWF_GrunzeModulCM4"
    #define MICROCONTROLLER_COMPILE_TYPE            uC_STM32F407VGT
    #define uC_Type                                 "STM32F407VGT"
    #define FWF_HSE_CLOCK                          (25 * MHz1)       // HSE Clock externer Takt am HSE-Pin  Nucleo: 8   Propox: 25
    #define PLL_M                                   25
    #define PLL_N                                   336
    #define MAC_PINS                                MAC_TXatPB      // Nucleo: MAC_TXatPGB                  Propox: MAC_TXatPB
    #define FWF_DEBUG_VIA_UART3                     0               // Nucleo: 1
    #define FWF_DEBUG_VIA_UART1                     1               // Dresdyn
    #define FWF_UART1_ON_A9_A10                     1               //
	#define _FWF_USE_APP_FLASH_TYPE   				1		 		// Flash ist verbaut - Kennung wird in fwf_api_firmwareupdate eingetragen
#endif // BOARD_FWF_GrunzeModulCM4

#if (BOARD == BOARD_FWF_DRESDYN)
    #define BOARD_NAME                              "BOARD_FWF_DRESDYN"
    #define MICROCONTROLLER_COMPILE_TYPE            uC_STM32F407VGT
    #define uC_Type                                 "STM32F407VGT"
    #define FWF_HSE_CLOCK                           (25 * MHz1)     // HSE Clock externer Takt am HSE-Pin  Nucleo: 8  Propox: 25
    #define PLL_M                                   (FWF_HSE_CLOCK / MHz1)   // default 25
    #define PLL_N                                   336
    #define MAC_PINS                                MAC_TXatPB      // Nucleo: MAC_TXatPGB                  Propox: MAC_TXatPB
    #define FWF_DEBUG_VIA_UART3                     0               // Nucleo: 1
    #define FWF_DEBUG_VIA_UART1                     1               // Dresdyn:1
    #define FWF_UART1_ON_A9_A10                     1               //
	#define _FWF_USE_APP_FLASH_TYPE   				1		 		// Flash ist verbaut - Kennung wird in fwf_api_firmwareupdate eingetragen
#endif // BOARD_FWF_DRESDYN

#if (BOARD == BOARD_FWF_PM80)                	// Getestet Eth-ok
    #define BOARD_NAME                              "BOARD_FWF_PM80"
    #define MICROCONTROLLER_COMPILE_TYPE            uC_STM32F4XX
    #define uC_Type                                 "STM32F407VGT"
    #define FWF_HSE_CLOCK                           (25 * MHz1)       // HSE Clock externer Takt am HSE-Pin
    #define PLL_M                                   (FWF_HSE_CLOCK / MHz1)   // default 25
    #define PLL_N                                   336
    #define MAC_PINS                                MAC_TXatPB      // Nucleo: MAC_TXatPGB                  Propox: MAC_TXatPB
    #define FWF_DEBUG_VIA_UART3                     0               // Nucleo: 1
    #define FWF_DEBUG_VIA_UART1                     0               // Dresdyn
    #define FWF_UART1_ON_A9_A10                     0               //
	#define _FWF_USE_APP_FLASH_TYPE   				1		 		// Flash ist verbaut - Kennung wird in fwf_api_firmwareupdate eingetragen
#endif // BOARD_FWF_PM80
// ENDE BOARD-spezifische Hardwarekonfiguration ============================================================================================

#ifndef BOARD_PROPOX_CM4
 #error " no valid BOARD "
#endif

#if MICROCONTROLLER_COMPILE_TYPE==uC_STM_ARM966ES || \
    MICROCONTROLLER_COMPILE_TYPE==uC_STM32F407VET || \
    MICROCONTROLLER_COMPILE_TYPE==uC_STM32F407VGT || \
    MICROCONTROLLER_COMPILE_TYPE==uC_STM32F4XX
#define HSI_VALUE                               (16 * MHz1)
#endif
    // IDE, Prozessor und Board ==================================================================


//================ DEBUG & TEST ===================================================
#define FWF_TYPES_USED              1	// UINT8, ... UINT32 werden verwendet
#define DEBUG						1
#define DEBUG_GLOBAL_ENABLE			1
#define DEBUG_LEVEL_FWF         	2

#define  FWF_DBGCMD_PRINTFv 		FWF_DBG3_PRINTFv

//=====================================================================================================
// Version
//=====================================================================================================
#define FWF_APP_COM_VERSION_uC  "V2.9"   			// Version der Gesamtsoftware
#define FWF_APP_COM_VERSION_uC_LEN (5 + 15 + 8)    	// maximale Laenge des Gesamt-Versionsstrings mit Datum


//=====================================================================================================
// Standard Hardware Konfiguration ====================================================================
//=====================================================================================================
#define FWF_WATCHDOG_MODE               1 // 1: Timer Mode (Reset per Software moeglich), 0: Watchdog (Reset in jedem Fall nach Ablauf der Wartezeit)
#define FWF_WATCHDOG                    0


#define FWF_USE_TASTATUR				0
#define FWF_USE_DISPLAY					0
#define FWF_USE_CM4TEMPERATURE_SENSOR	1	// Verwendung des chipinternen Temperatursensors


#define FWF_NUM_UDP_UARTS               0  // Anzahl der UDP-Server, die UARTs bedienen
#define FWF_UARTx8HARDWARE              0  // Hardware mit 8xUART via SPI

// Einstellungen an den Ports
#define FWF_SPI0_STEPPER_MOTOR_FIX      0
#define FWF_SPI1_STEPPER_MOTOR_FIX      0
#define FWF_SPI0_STEPPER_MOTOR_MUX      0
#define FWF_SPI1_STEPPER_MOTOR_MUX      0

#define FWF_SPI0_ENCODER_MUX            0 // ZRT

// STANDARD DEBUG EINSTELLUNGEN ===============================================================
#define FWF_DBG_HW_SUPPORT              0 // Debugging Unterstuetzung in Hardware
#define FWF_DEBUG_VIA_UART              0  // Debug-Ausgabe ueber uart0


//=====================================================================================================
// Spezifische Applikationen ==========================================================================
//=====================================================================================================


#if FWF_APP_PIEZO_CONTROLLER_CM4_FWKT2016      	  		// Ethernet IO Piezo-Antriebe fuer FWKT 2016
	#define PIT_ID   		"AAA00228"
	#define FWF_APP_NAME 	"EIO2.PIEZO.cm4"			// Typname
	#define PIEZO_CONTROLLER_APP_FWKT2016		1
	#define PIEZO_CLOSED_LOOP_CONTROL			0
	// Belegung von Hardware-Pins
	// Debugging Hilfe durch
	#undef FWF_PORT6_OUT
	#define FWF_PORT6_OUT                 		0
	#define FWF_DBG_PWM                     	0   	// 6.4 6.6
#endif // FWF_APP_PIEZO_CONTROLLER_CM4_FWKT2016

#if FWF_APP_PIEZO_CONTROLLER_CM4_FWKT2021      	  		// Ethernet IO Piezo-Antriebe fuer FWKT 2021
	#define PIT_ID   		"AAA002xx"
	#define FWF_APP_NAME 	"EIO2.PIEZO2021.cm4"			// Typname
	#define PIEZO_CONTROLLER_APP_2021   		1
	#define PIEZO_CLOSED_LOOP_CONTROL	 		0
	#define FWF_USING_PIEZODRIVE_2021_GENERATOR_PARSET		1
	// Belegung von Hardware-Pins
	// Debugging Hilfe durch
	#undef FWF_PORT6_OUT
	#define FWF_PORT6_OUT                 		0
	#define FWF_DBG_PWM                     	0   	// 6.4 6.6
#endif // FWF_APP_PIEZO_CONTROLLER_CM4_FWKT2016


#if FWF_APP_PIEZO_CONTROLLER_CM4_XFEL  	  		// Ethernet IO Piezo-Antriebe fuer XFEL
	#define PIT_ID   		"AAA00265"
	#define FWF_APP_NAME 	"EIO2.PIEZO_XFEL2"	// Typname fuer Flash 	PIEZO_XFEL	PIEZO_XFEL2
// Auswahl der Hardwareversion fuer PiezoController
	#define PIEZO_CONTROLLER_APP_XFEL			1
	#define PIEZO_CLOSED_LOOP_CONTROL			1
	#define  FWF_USING_PIEZODRIVE_XFEL_GENERATOR_PARSET	1
	// Belegung von Hardware-Pins
	// Debugging Hilfe durch
	#undef FWF_PORT6_OUT
	#define FWF_PORT6_OUT                 		0
	#define FWF_DBG_PWM                     	0   	// 6.4 6.6
#endif // FWF_APP_PIEZO_CONTROLLER_CM4_XFEL

#if FWF_APP_PM80_PROTO        	  			// Ethernet IO fuer Penelope PM80
	#define PIT_ID   		"AAA00359"
	#define FWF_APP_NAME 	"EIO2.LPMProto"	// Typname
#endif // FWF_APP_PM80_PROTO

#if FWF_APP_LPM_2022    	  				// Ethernet IO fuer Penelope PM80
	#define PIT_ID   		"AAA00xxx"
	#define FWF_APP_NAME 	"EIO2.LPM_2022"	// Typname
	#define FWF_USE_SYSTICK_TIMER 				1
	#define FWF_SPI2_ON_PB14_PB15				1
#endif // FWF_APP_LPM_2022

#if FWF_APP_HLD_24KV_LADECONTROLLER
	#define PIT_ID   		"AAA00xxx"
	#define FWF_APP_NAME 	"EIO2.HLD24kV.cm4" 		// Typname
	#define HLD_24KV_SIMULATE 					0		// Herrmansdoerfer

	#define HLD_24KV_APP						1
	#define FPGA_MODULE_USED					1
	#define FWF_USE_SYSTICK_TIMER 				1
	#define FWF_TEST_APB1_viaTIMER          	1       // 1: 1 ms SystickTimer		else: TIM2    USES PE14
#endif // FWF_APP_HLD_24KV_LADECONTROLLER


#if FWF_APP_ADC_TI_ADS9224_EVAL
	#define PIT_ID   		"AAA00xxx"
	#define FWF_APP_NAME 	"EIO2.TI_ADS9224_EVAL.cm4" 		// Typname
	#define ADC_TI_ADS9224_APP						1
#endif // FWF_APP_ADC_TI_ADS9224_EVAL


#if FWF_APP_MV40        	  		// Ethernet IO fuer MV40
	#define PIT_ID   		"AAA00228"
	#define FWF_APP_NAME 	"EIO2.MV40.cm4"	// Typname
	#define FWF_MV40_APP						1

	#define FWF_USE_DISPLAY						0
	#define DISPLAY_MD050SD						0	// Touch display N.Winkler /// mv40.h fehlt

// Belegung von Hardware-Pins
// Debugging Hilfe durch
	#undef  FWF_PORT6_OUT
	#define FWF_PORT6_OUT                 	0
	#define FWF_DBG_PWM                     0   // 6.4 6.6
	#define FWF_USE_SYSTICK_TIMER 			1
	#define FWF_TEST_APB1_viaTIMER          	1       // 1: 1 ms SystickTimer		else: TIM2    USES PE14
#endif // FWF_APP_MV40

#if FWF_APP_STEPPER_PHYTRON
    #define PIT_ID   		"AAA00xxx"
    #define FWF_APP_NAME 	"EIO2.STEPPER_PHYTRON.cm4"   // Typname
    #define FWF_STEPPPER_PHYTRON_APP        1
	#define FWF_USE_SYSTICK_TIMER 			1
	#define FWF_TEST_APB1_viaTIMER          1       // 1: 1 ms SystickTimer		else: TIM2    USES PE14
	#define FWF_USE_LIB_MOTION_PLANNING		1
#endif // FWF_APP_STEPPER_PHYTRON

#if FWF_APP_STEPPER_STMdrv
	#define PIT_ID   		"AAA00xxx"
	#define FWF_APP_NAME 	"STEPPER_STMdrv.cm4"   // Typname
	#define FWF_STEPPPER_STM_DRV_APP        1
	#define FWF_USE_SYSTICK_TIMER 			1
	#define FWF_TEST_APB1_viaTIMER        	1       // 1: 1 ms SystickTimer		else: TIM2    USES PE14
	#define FWF_USE_LIB_MOTION_PLANNING		1

	#define USE_SECOP_SERVER				1
	#define USE_MQTT_NODE					0
	#define USE_MQTT_BROKER 				0  				// Code fuer Broker

#endif // FWF_APP_STEPPER_STMdrv

#if FWF_APP_ADCDAC_ARM_CM4
	#define PIT_ID   		"AAA00---"
	#define FWF_APP_NAME 	"EIO2.ADCDAC.cm4"		// Typname
	#define FWF_ADCDAC_APP					1

#endif


    // Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_MUON_DETECTOR
    #define PIT_ID   			"AAA00xxx"
    #define FWF_APP_NAME 		"MUON_DETECTOR"      // Typname
    #define MUON_DETECTOR_APP           1
    // Clock-Ueberwachung
    #define FWF_SPI2_ON_PB14_PB15               1       // Alternative Pinbelegung der SPI2
    #undef FWF_DEBUG_VIA_UART3
    #undef FWF_DEBUG_VIA_UART1
    #undef FWF_UART1_ON_A9_A10
    #define FWF_DEBUG_VIA_UART1                 0       // Testgeneratorboard Grunze
    #define FWF_UART1_ON_A9_A10                 1       //

	#define EIO4_LED_MATRIX						10
	#define FWF_USE_SYSTICK_TIMER 				1
#endif // FWF_APP_MUON_DETECTOR


#if FWF_APP_HALLSENSOR23
    #define PIT_ID   			"AAA00xxx"
    #define FWF_APP_NAME 		"EIO2.HALLSENSOR23"      // Typname

	#define DRESDYN_GENERATE_ADC_CLOCK			1		// Option zum lokalen Erzeugen einer ADC-Clock ueber PB8, PB9
    // Clock-Ueberwachung
    #define FWF_SPI2_ON_PB14_PB15               1       // Alternative Pinbelegung der SPI2
    #undef FWF_DEBUG_VIA_UART3
    #undef FWF_DEBUG_VIA_UART1
    #undef FWF_UART1_ON_A9_A10
    #define FWF_DEBUG_VIA_UART1                 1       // Testgeneratorboard Grunze
    #define FWF_UART1_ON_A9_A10                 1       //
	#define FWF_USE_SYSTICK_TIMER 				1
	#define FWF_TEST_APB1_viaTIMER          	1       // 1: 1 ms SystickTimer		else: TIM2 USES PE14
#endif // FWF_APP_HALLSENSOR23


#if FWF_APP_HW_EVB_EIO2_CM4        	  			// Ethernet IO2 Testplatine
	#define PIT_ID   		"AAA00---"
	#define FWF_APP_NAME 	"EIO.TESTBOARD.cm4"	// Typname
	#define FWF_EVALBOARD1_APP				1
	#define ENCODER_TEST_APP				1
	#define FWF_EVALBOARD_USE_TOUCH			0

	#undef   FWF_DEBUG_VIA_UART3
	#define FWF_DEBUG_VIA_UART3             1

	// Belegung von Hardware-Pins
	#define FWF_USE_ENCODER_1				1
	#define FWF_USE_ENCODER_2				1

	#undef  FWF_USE_TASTATUR
	#define FWF_USE_TASTATUR				1

	#define FWF_USE_DISPLAY					0 	// 1

	// Debugging Hilfe durch
	#undef  FWF_PORT6_OUT
	#define FWF_PORT6_OUT                 	0
	#define FWF_DBG_PWM                     0   // 6.4 6.6
#endif // FWF_APP_HW_EVB_EIO2_CM4


#if FWF_APP_HW_Eval_of_Boards
    #define PIT_ID   		"AAA00---"
    #define FWF_APP_NAME 	"EIO.HW_EVALUATION.cm4"   // Typname
    #define FWF_HW_EVALUATION_APP           1
	#define FWF_USE_SYSTICK_TIMER 			1
    #define FWF_TEST_APB1_viaTIMER          1       // Output 500 Hz an PE14 zum Test der APB1-Clock        1: 1 ms SystickTimer		else: TIM2    USES PE14

    // Debugging Hilfe durch
    #undef  FWF_PORT6_OUT
    #define FWF_PORT6_OUT                   0
    #define FWF_DBG_PWM                     0   // 6.4 6.6
#endif


	// Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_DRESDYN_TESTGENERATOR
	#define PIT_ID   		"AAA00xxx"
	#define FWF_APP_NAME 	"DRESDYN_TESTGENERATOR"		// Typname
	#define DRESDYN_TESTGENERATOR_APP			1
	// Clock-Ueberwachung
	#define DRESDYN_MEASURE_CLOCK				0		// Funktion Vermessen der Clock eingeschaltet
	#define DRESDYN_MEASURE_CLOCK_TIM9			1		// Vermessen der Clock ueber TIM9
    #define FWF_SPI2_ON_PB14_PB15               1       // Alternative Pinbelegung der SPI2
    #undef FWF_DEBUG_VIA_UART3
    #undef FWF_DEBUG_VIA_UART1
    #undef FWF_UART1_ON_A9_A10
    #define FWF_DEBUG_VIA_UART1                 1       // Testgeneratorboard Grunze
    #define FWF_UART1_ON_A9_A10                 1       //
	#define FWF_USE_SYSTICK_TIMER 				1
#endif // FWF_APP_DRESDYN_TESTGENERATOR


#if FWF_APP_DRESDYN_SENSORNODE							// Dresdyn Sensorknoten
	#define PIT_ID   		"AAA00229"					// UG117:AAA00229 ; UG102:AAA00313
	#define FWF_APP_NAME 	"DRESDYN_SENSORNODE.cm4"	// Typname
	#define DRESDYN_SENSORNODE_APP					1
	#define TIMESTAMP_APP 							0	// Kommandointerface fuer Zeitabgleich mit fwf_app_TIMESTAMP_interpreter

	#define DRESDYN_GENERATE_ADC_CLOCK				0	// Option zum lokalen Erzeugen einer ADC-Clock ueber PB8, PB9
	#define DRESDYN_MEASURE_CLOCK					1	// Funktion Vermessen der Clock eingeschaltet
	#define DRESDYN_MEASURE_CLOCK_TIM9				1	// Vermessen der Clock ueber TIM9

	#define RESET_PHY 								1	// Ruecksetzen des PHY-Chips

	#undef FWF_DEBUG_VIA_UART
	#define FWF_DEBUG_VIA_UART						1
	#define FWF_USE_SYSTICK_TIMER 					1
// Timestamp-Methode festlegen
#if 1
	#define DRESDYN_SENSORNODE_TIMESTAMP_at_ADC_READ		1		// beim Samplen der ADC's Timestamp lesen
#else
	#define TIMESTAMP_ENABLE_SW			1	// ETH->PTPTSHR ETH->PTPTSLR in ptp_function_block bei Senden oder Empfangen der MSG TIMESTAMP_AT_MSG_OUT TIMESTAMP_AT_MSG_IN
	#define TIMESTAMP_AT_MSG_IN			1	// Timestamp at ETH_CheckFrameReceived()
	#define TIMESTAMP_AT_MSG_OUT		0	// Timestamp at ETH_CheckFrameSend()
#endif
// Keller ueber RS485 festlegen
#if 1 // KEIN KELLER_DRUCKSENSOR
	#define FWF_APP_USES_UART1			0
	#define FWF_USE_KELLER_DRUCK_RS485	0   // Verwendung der Keller-Drucksensoren ueber RS485
#else
	#define FWF_APP_USES_UART3			1
	#define FWF_USE_KELLER_DRUCK_RS485	1   // Verwendung der Keller-Drucksensoren ueber RS485
#endif

#endif //FWF_APP_DRESDYN_SENSORNODE


	// Applikationsspezifische Konfiguration  =============================================================
#if FWF_APP_DRESDYN_ANGLES
	#undef BOARD
	#define BOARD          			 			    BOARD_FWF_GrunzeModulCM4

	#define PIT_ID   		"AAA00351"
	#define FWF_APP_NAME 	"DRESDYN_ANGLES"			// Typname
	#define DRESDYN_ANGLES_APP					1
	#define ENCODER_TO_CAMERA_TRIGGER_SUBAPP	1		// Erzeugung von Triggern aus Encodersignalen
	#define TIMESTAMP_APP						0		// Reset der uC-Time

	#define DRESDYN_GENERATE_ADC_CLOCK			0		// Option zum lokalen Erzeugen einer ADC-Clock ueber PB8, PB9
	// Clock-Ueberwachung
	#define DRESDYN_MEASURE_CLOCK				1		// Funktion Vermessen der Clock eingeschaltet
    #define DRESDYN_MEASURE_CLOCK_TIM9          1       // Vermessen der Clock ueber TIM9
    #define DRESDYN_MEASURE_CLOCK_TIM12         0       // Vermessen der Clock ueber TIM12

	// Applikationsflash
	#undef  _FWF_USE_APP_FLASH_TYPE
	#define _FWF_USE_APP_FLASH_TYPE				0		// kein Applikations-Flash wegen PD0 FPGA_CS
#endif // FWF_APP_DRESDYN_ANGLES


// Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_DRESDYN_Fanout_and_MEASURE_CLOCK
	#undef  BOARD
	#define BOARD          			 			    BOARD_FWF_DRESDYN

	#define PIT_ID   		"AAA00353"
	#define FWF_APP_NAME 	"DRESDYN_Clock_Fanout+Check"	// Typname

	#define DRESDYN_CLOCKGEN_AND_CHECK_APP			1		// Erzeugung und Pruefung der Clock
	#define DRESDYN_GENERATE_ADC_CLOCK				0		// Option zum lokalen Erzeugen einer ADC-Clock ueber PB8, PB9
	#define DRESDYN_MEASURE_CLOCK					1		// Funktion Vermessen der Clock eingeschaltet
	#define DRESDYN_MEASURE_CLOCK_TIM9				1		// Vermessen der Clock ueber TIM9
	#define FWF_USE_SYSTICK_TIMER 					1
#endif // FWF_APP_DRESDYN_Fanout_and_MEASURE_CLOCK


#if FWF_APP_DRESDYN_INTERLOCK
	#define PIT_ID   		"AAA00352"
	#define FWF_APP_NAME 	"DRESDYN_Interlock"				// Typname
	#define DRESDYN_INTERLOCK_APP					1		// Erzeugung von Interlocks
	#define FWF_USE_SYSTICK_TIMER 					1
	#define FWF_TEST_APB1_viaTIMER          		0       // 1: 1 ms SystickTimer		else: TIM2    USES PE14 - Nur fuer Tests des Systick
#endif //FWF_APP_DRESDYN_INTERLOCK


#if FWF_APP_DRESDYN_HEATINGNODE
	#define PIT_ID   		"AAA00354"
	#define FWF_APP_NAME 	"DRESDYN_HEATING_NODE"			// Typname
	#define DRESDYN_HEATING_APP						1

	#define RESET_PHY 								1		// Ruecksetzen des PHY-Chips

	#define DRESDYN_GENERATE_ADC_CLOCK				0 		// Option zum lokalen Erzeugen einer ADC-Clock ueber PB8, PB9
	#define DRESDYN_MEASURE_CLOCK					1		// Funktion Vermessen der Clock eingeschaltet
	#define DRESDYN_MEASURE_CLOCK_TIM9				1		// Vermessen der Clock ueber TIM9
	#define FWF_USE_SYSTICK_TIMER 					1
#endif // FWF_APP_DRESDYN_HEATINGNODE

// Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_SENSYS_SINAMICS_ENCODER2TRIGGER_PULSE
	#undef  DEBUG_GLOBAL_ENABLE
	#define DEBUG_GLOBAL_ENABLE					10
	#define PIT_ID   		"AAA00302"
	#define FWF_APP_NAME 	"EIO2.SenSys_EncoderPulse"		// Typname
	#define DIO_APP								1			// Inkrementalgeberauswertung
	#define ENCODER_CHECK_BORDERS_APP			1			// Ueberpruefung der Encoder
	#define ENCODER_GENERATE_TRIGGER_SUBAPP		1			// Erzeugung von Triggern aus Encodersignalen
	#define FWF_USE_ENCODER_3					1			// 1 Flexiplant @ TIM3 - ggf. Kollision mit PWM_A6TIM3
	#define FWF_USE_ENCODER_4					0			// 0 Flexiplant @ TIM4
	#define LIGHT_PWM_SUBAPP					1			// Erzeugung der PWM's fuer Licht
	#define DIO_PD45_PORT_SUBAPP				0			// Telops Kamera Lichtansteuerung
    #define TIMER_TRIGGERED_SUBAPP              0			// Timergesteuertes Triggern der Kameras
	#define SINAMICSV20_APP						1			// Frequenzumrichteransteuerung
	#define TIMESTAMP_APP						1			// Reset der uC-Time
	// PWM1 PA8 TIM1 und PWM3 PA0 TIM5 sind immer aktiv
	#define PWM_C6TIM8							1			// Verwendung von PWM4@C6-TIM8
	#define PWM_A6TIM3							0			// Verwendung von PWM2@A6-TIM3
	// Belegung von Hardware-Pins fuer Anschluss Encoder
	#define FWF_APP_USES_UART3						1
#endif // FWF_APP_SENSYS_SINAMICS_ENCODER2TRIGGER_PULSE

// Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_SENSYS_PD45_ENCODER2TRIGGER_PULSE
	#undef  DEBUG_GLOBAL_ENABLE
	#define DEBUG_GLOBAL_ENABLE					10
	#define PIT_ID   		"AAA00302"
	#define FWF_APP_NAME 	"EIO2.SenSys_PD45_EncoderPulse"	// Typname
	#define DIO_APP								1			// Inkrementalgeberauswertung
    #define DIO_PD45_PORT_SUBAPP                1           // PD4,5 statt Kameratrigger => digitale Outputs z.B zur Bedienung der Telops Lichtanlage
	#define TIMESTAMP_APP						1			// Reset der uC-Time
	#define ENCODER_CHECK_BORDERS_APP			1			// Ueberpruefung der Encoder
	#define ENCODER_GENERATE_TRIGGER_SUBAPP		1			// Erzeugung von Triggern aus Encodersignalen
	#define LIGHT_PWM_SUBAPP					1			// Erzeugung der PWM's fuer Licht
	#define FWF_USE_ENCODER_3					0			// 1 Flexiplant @ TIM3 - ggf. Kollision mit PWM_A6TIM3
	#define FWF_USE_ENCODER_4					1			// 0 Flexiplant @ TIM4 - ggf. Kollision mit TIMER_TRIGGERED_SUBAPP
    #define TIMER_TRIGGERED_SUBAPP              0           // Timergesteuertes Triggern  der Kameras - TIM4 - Kollision mit FWF_USE_ENCODER_4
	// PWM1 PA8 TIM1 und PWM3 PA0 TIM5 sind immer aktiv
	#define PWM_C6TIM8							1			// Verwendung von PWM4@C6-TIM8
	#define PWM_A6TIM3							1			// Verwendung von PWM2@A6-TIM3
	// Belegung von Hardware-Pins fuer Anschluss Encoder
	#define FWF_APP_USES_UART3						1
#endif // FWF_APP_SENSYS_PD45_ENCODER2TRIGGER_PULSE

#if FWF_APP_HTW_ENCODER_to_TRIGGER_PULSE
	#undef  DEBUG_GLOBAL_ENABLE
	#define DEBUG_GLOBAL_ENABLE					10
	#define PIT_ID   		"AAA00302"
	#define FWF_APP_NAME 	"EIO2.HTW_EncoderPulse"			// Typname
	#define DIO_APP								1			// Inkrementalgeberauswertung
    #define DIO_PD45_PORT_SUBAPP                1           // PD4,5 statt Kameratrigger => Bedienung des Wenderelais
	#define ENCODER_CHECK_BORDERS_APP			1			// Ueberpruefung der Encoder
	#define ENCODER_GENERATE_TRIGGER_SUBAPP		1			// Erzeugung von Triggern aus Encodersignalen
	#define FWF_USE_ENCODER_3					1			// 1 HTW
	#define FWF_USE_ENCODER_4					0			// 0 HTW
	#define LIGHT_PWM_SUBAPP					1			// Erzeugung der PWM's fuer Licht
    #define TIMER_TRIGGERED_SUBAPP              1           // auch Timergesteuertes Triggern der Kameras
	#define TIMESTAMP_APP						1			// Reset der uC-Time
	#define PWM_C6TIM8							1			// Verwendung von PWM4@C6-TIM8
	#define PWM_A6TIM3							0			// Verwendung von PWM2@A6-TIM3
	// Belegung von Hardware-Pins fuer Anschluss Encoder
	#define FWF_APP_USES_UART3						1
#endif // FWF_APP_HTW_ENCODER_to_TRIGGER_PULSE


// Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_SINAMICSV20_FU	// Fuer Dresdyn Beschleunigungstestplatz
#define PIT_ID   		"AAA00425"
#define FWF_APP_NAME 	"SINAMICSV20.UART"				// Typname
#define SINAMICSV20_APP						1			// Frequenzumrichteransteuerung
#define DIO_APP								0			// Inkrementalgeberauswertung
#define ENCODER_CHECK_BORDERS_APP			0			// Ueberpruefung der Encoder
#define TIMER_TRIGGERED_SUBAPP              0           // auch Timergesteuertes Triggern der Kameras
#define FWF_APP_USES_UART2					1			// RS485 ? fuer Grunze-Hutschienen-Modul
// Belegung von Hardware-Pins fuer Anschluss Encoder
#define FWF_USE_ENCODER_1					0
#define FWF_USE_ENCODER_2					0
#endif // FWF_APP_SINAMICSV20_FU


#if FWF_APP_SINAMICSV20		// Fuer Sensys - Kameratrigger
	#define PIT_ID   "AAA00302"
	#define FWF_APP_NAME "SINAMICSV20.UART+Cam.trig"		// Typname
	#define SINAMICSV20_APP						1			// Frequenzumrichteransteuerung
	#define DIO_APP								1			// Inkrementalgeberauswertung
	#define AIO_APP								1			// Analogfunktionen
	#define ENCODER_CHECK_BORDERS_APP			1			// Ueberpruefung der Encoder
	#define ENCODER_GENERATE_TRIGGER_SUBAPP		1			//
	#define TIMER_TRIGGERED_SUBAPP              0           // auch Timergesteuertes Triggern  der Kameras
	#define LIGHT_PWM_SUBAPP					0			// Erzeugung der PWM's fuer Licht
	#define DIO_PD45_PORT_SUBAPP				0			// Bedienung der Telops Lichtanlage
	#define FWF_APP_USES_UART3					1			// RS485
	// Belegung von Hardware-Pins fuer Anschluss Encoder
	#define FWF_USE_ENCODER_1					1
	#define FWF_USE_ENCODER_2					1
#endif // FWF_APP_SINAMICSV20



// Applikationsspezifische Konfiguration   =============================================================
#if FWF_APP_HTW_EVA_UEBUNG
	#define PIT_ID   		"HTW__HTW"
	#define FWF_APP_NAME 	"EIO2.HTW.UEBUNG" 	// Typname

	#define HTW_EVA_UEBUNG_APP				1
	#define DIO_APP							1
	#define FWF_CHECK_DIG_EVENTS_APP		1	// Pruefen von Veraenderungen an den Digitalports und Event-Message
	#define AIO_APP							1
	#define DIO_PD45_PORT_SUBAPP			0	// zur Ausgabe in fwf_api_encoder.c

	#define MEASURE_CMD_EXECUTION_TIME		10
	#define USE_COLOR_CONVERSION			1

	#define FWF_SPI0_RELAIS_LED             0
	#define FWF_SPI0_420mA_OUT              0
	#define FWF_SPI0_420mA_OUT_PULSED       0
	#define FWF_SPI0_DAC10V_HARDWARE        0
	#define FWF_SPI0_RELAIS_LED             0
	#define FWF_SPI1_420mA_IN               0
	#define FWF_SPI1_ADC_10V_HARDWARE       0
	// Debugging Hilfe durch Hardwareausgabe
	#undef  FWF_DEBUG_VIA_UART
	#define FWF_DEBUG_VIA_UART             	1    // Debugging Unterstuetzung in Hardware
	#define FWF_DBG_PWM                     1    // 6.4 6.6
	//#define LIGHT_PWM_SUBAPP 				1 	 // Nutzung der PWM's
	#if FWF_PORT6_OUT
	#define HW_DEBUG_PORT6_0   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_1   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_2   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_3   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_4   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_5   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_6   1    // Anzeige von Fehlern
	#define HW_DEBUG_PORT6_7   1    // Anzeige von Fehlern
	#endif

	#define FWF_TCP_SERVER_HTW				1
	#define FWF_USE_SYSTICK_TIMER 			1

	#define USE_SECOP_SERVER				1
	#define USE_MQTT_NODE					1
	#define USE_MQTT_BROKER 				0  				// Code fuer Broker

#endif // FWF_APP_HTW_EVB_UEBUNG



#if FWF_APP_PIXE_2DAC_CM4        				// Ethernet IO Highspeed Pixe
	#define PIT_ID   			"AAA00199"
	#define FWF_APP_NAME 		"EIO2.PIXE.cm4"		// Typname


	#define PIXE_CONTROLLER_APP				1
	// Belegung von Hardware-Pins
	#define DAC1_DMA_ON 	1	// Ch1
	#define DAC2_DMA_ON 	1	// Ch2

	// Debugging Hilfe durch
	#undef FWF_PORT6_OUT
	#define FWF_PORT6_OUT                 	0
	#define FWF_DBG_PWM                     0   // 6.4 6.6
#endif // FWF_APP_PIXE_2DAC_CM4


#if FWF_APP_PELTIER        // Ethernet IO Peltier-Steuerung
	#define PIT_ID   			"AAA00xxx"
	#define FWF_APP_NAME 		"EIO.PELTIER.cm4"
#endif // FWF_APP_PELTIER

#if FWF_APP_PRESSURE_ANALOG							// Drucksensor Analog
	#define PIT_ID   		"AAA00xxx"				// UG117:
	#define FWF_APP_NAME 	"PRESSURE_ANALOG.cm4"	// Typname
	#define TIMESTAMP_APP 							0	// Kommandointerface fuer Zeitabgleich mit fwf_app_TIMESTAMP_interpreter

	#define DRESDYN_GENERATE_ADC_CLOCK				0	// Option zum lokalen Erzeugen einer ADC-Clock ueber PB8, PB9

	#define RESET_PHY 								1	// Ruecksetzen des PHY-Chips

	#undef FWF_DEBUG_VIA_UART
	#define FWF_DEBUG_VIA_UART						1
	#define FWF_USE_SYSTICK_TIMER 					1
// Timestamp-Methode festlegen
#if 1
	#define PRESSURE_ANALOG_TIMESTAMP_at_ADC_READ		1		// beim Samplen der ADC's Timestamp lesen
#else
	#define TIMESTAMP_ENABLE_SW			1	// ETH->PTPTSHR ETH->PTPTSLR in ptp_function_block bei Senden oder Empfangen der MSG TIMESTAMP_AT_MSG_OUT TIMESTAMP_AT_MSG_IN
	#define TIMESTAMP_AT_MSG_IN			1	// Timestamp at ETH_CheckFrameReceived()
	#define TIMESTAMP_AT_MSG_OUT		0	// Timestamp at ETH_CheckFrameSend()
#endif

#endif //FWF_APP_DRESDYN_SENSORNODE

//=========================================================================================
// ungueltige Hardware Kombinationen =======================================================
//=========================================================================================
#ifndef FWF_APP_NAME
    #error  "FWF_Application-SETUP invalid"
#endif
#if FWF_SPI1_420mA_IN & !FWF_UARTx8HARDWARE
   #error "Hardware-Combination invalid"
#endif

//=========================================================================================
// abgeleitete Hardware Kombination =======================================================
//=========================================================================================
#define FWF_TIMER_USE                   (FWF_TIM_0 + FWF_TIM_1)
#if FWF_STEPPER_APP
#define FWF_STEPPER_MOTOR_NUM           (FWF_SPI0_STEPPER_MOTOR_FIX + FWF_SPI1_STEPPER_MOTOR_FIX + FWF_SPI0_STEPPER_MOTOR_MUX + FWF_SPI1_STEPPER_MOTOR_MUX)
#define FWF_SPI0_STEPPER_HARDWARE       (FWF_SPI0_STEPPER_MOTOR_FIX + FWF_SPI0_STEPPER_MOTOR_MUX)
#define FWF_SPI1_STEPPER_HARDWARE       (FWF_SPI1_STEPPER_MOTOR_FIX + FWF_SPI1_STEPPER_MOTOR_MUX)
#define FWF_STEPPER_SPI_USE             (FWF_SPI0_STEPPER_HARDWARE + FWF_SPI1_STEPPER_HARDWARE)  // Schrittmotorschnittstellen in Verwendung

#define FWF_SPI0_USE                    (FWF_SPI0_RELAIS_LED + FWF_SPI0_STEPPER_HARDWARE + FWF_SPI0_DAC10V_HARDWARE + FWF_SPI0_420mA_OUT  + FWF_SPI0_ENCODER_MUX)
#define FWF_SPI1_USE                    (FWF_SPI1_STEPPER_HARDWARE + FWF_SPI1_ADC_10V_HARDWARE + FWF_SPI1_COUNTER_HARDWARE + FWF_SPI1_420mA_IN + FWF_SPI1_ECAT_HARDWARE)
#endif

#define FWF_UART_0                      ( FWF_UDP_SERVER_UART0 + FWF_DEBUG_VIA_UART)
#define FWF_UART_1                      ( FWF_UDP_SERVER_UART1)
#define FWF_UART_APP					( FWF_UDP_SERVER_UART0 + FWF_UDP_SERVER_UART1)

#define HW_DEBUG_PORT6              	(HW_DEBUG_PORT6_0 + HW_DEBUG_PORT6_1 + HW_DEBUG_PORT6_2 + HW_DEBUG_PORT6_3 + HW_DEBUG_PORT6_5 + HW_DEBUG_PORT6_7)

//=========================================================================================
// Multiple Server
//=========================================================================================
#define FWF_DIVERSE_UDP_SERVER          (FWF_UDP_SERVER_UART0 + FWF_UDP_SERVER_UART1 ) // zur Verteilung der UDP-Pakete in uip_process

#define FWF_USE_DHCP                      0

#if FWF_ETHERNET || ETHERCAT
  #define FWF_TIM_0                       1
#else
  #define FWF_TIM_0                       0
#endif
#if FWF_STEPPER_SPI_USE
  #define FWF_TIM_1                       1
  #define FWF_TIMER1_PRESCALE             51
#else
  #define FWF_TIM_1                       0
  #define FWF_TIMER1_PRESCALE             12
#endif



#if FWF_SPI0_420mA_OUT
  #define FWF_TIM_3                     1
#endif

#if ETHERCAT
  #define FWF_SPI1_ECAT_HARDWARE        1
#else
  #define FWF_SPI1_ECAT_HARDWARE        0
#endif

// Konfiguration der Hardware ==============================================
//  FWF_STEPPER_HARDWARE  Schrittmotor




#endif /* __FWF_DEFS */
