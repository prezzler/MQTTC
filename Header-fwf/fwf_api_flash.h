#ifndef FWF_APP_FLASH_VERSION
#define FWF_APP_FLASH_VERSION          	"CAT_FLASH_1.4"
#define FWF_APP_FLASH_INTERFACEVERSION		0x0001
// CAT_FLASH_1.4 mit Stream-IP
// Ka: EthConf_1.4 ab 2021-07 mit eth_search_link, arps_on_startup_limit im Flash

#define FWF_ETH_CONFIG_NAME 			"EthConf_1.4"		// Versionsstring fuer Ethernet Settings
// EthConf_1.3 	mit Stream-IP fuer Multicast

//==================================================================================
// P.Kaever
//  1.0    	Reservierte Codes fuer Ethernet-IO-Modul
//  1.1     Mit 16 Bit Befehlen fuer reservierte Codes
//  1.2 	FWF_uCMD_GET_CONTROLLER_NAME, FWF_uCMD_SET_CONTROLLER_NAME
//  Interfaceversion
//  2 		uC_in_subnet added
//==================================================================================

//==================================================================================
// Der Aufbau der Schnittstelle fuer UDP-Kommandos und Ereignisse ist wie folgt codiert:
// zunaechst wird das Kommando des PC's beschrieben, in der naechsten Zeile die Antwort des
// Mikrocontrollers.
// Die ersten 3 Byte des vom PC verschickten UDP-Kommandos enthalten Kategorie CAT, Kommando CMD und einen laufenden Zaehler CNT:
// PC Kommandoaufbau: 		CAT CMD CNT
// Die Antwort des Mikrocontrollers enthaelt einen festen Teil mit 4 Byte mit Kategorie, Kommando und dem Zaehler des empfangenen PC-Kommandos
// sowie dem Status STAT der vom Mikrocontroller geschickten Nachricht (Befehlsbearbeitung oder vom Mikrocontroller als Reaktion auf ein Ereignis):
//	uC Nachricht: 	CAT CMD CNT STAT
//
// Auf die beiden festen Anteile von PC- oder Mikrocontrollernachricht folgen spezifische Nutzdaten, deren Position und Laenge angegeben werden:
// Die Position wird relativ zum Beginn der Nutzdaten notiert, darauf folgt die Laenge (beide in Byte gezaehlt). Bei Datenstrukturen mit nicht
// a priori bekannter Laenge wird nur die Anfangsposition angegeben, z. B. Version(0)
// Beispiel:
// PC Kommandoaufbau: 		CAT CMD CNT 		Motor#(0,1) WindA(1,1) WindB(2,1)
// uC Aufbau der Antwort: 	CAT CMD CNT STAT 	Motor#(0,1) WindA(1,1) WindB(2,1)
//
//==================================================================================

//==================================================================================
// Bytepositionen der Antwort
#define FWF_UCMD_RESERVED_COMMAND_CMD_NEGATE    		0   // Position des negierten Commandowertes
#define FWF_UCMD_RESERVED_RESPONSE_CMD_NEGATE   		0   // Position des negiert zurueckgegebenen Commandowertes
#define FWF_UCMD_RESERVED_CMD_PARAM       				1   // Position der Kommandowert-Parameter
#define FWF_UCMD_RESERVED_RESPONSE_OFFS4_READFLASH 		1   // Position der Antwort-Parameter auf das Kommando bei Flash-Lesebefehlen; Abfrage Version, Flashgroesse
#define FWF_UCMD_RESERVED_RESPONSE_STAT4_WRITEFLASH 	1   // Position des Flash-Write-Status auf den Status des Schreibvorgangs bei Flash-Schreibbefehlen
#define FWF_UCMD_RESERVED_RESPONSE_OFFS4_WRITEFLASH 	2   // Position des Flash-Write-Status-Offset auf das Kommando fuer das Schreiben der Antwort

#define FWF_UCMD_RESERVED_RESPONSE_HEADER_LEN_READFLASH		5			// CAT CMD CNT STAT ~CMD 				beruecksichtigt
#define FWF_UCMD_RESERVED_RESPONSE_HEADER_LEN_WRITEFLASH	6			// CAT CMD CNT STAT ~CMD okFlashWrite	beruecksichtigt


#if MICROCONTROLLER_COMPILE_TYPE
typedef struct {
	SINT8 					temperatureoffset;
	CONTROLLER_NAME 		controller_name;
	APPLICATION_NAME 	application_name;	// Application config name
	APPLICATION_NAME 	com_config_name;	// Common & Ethernet config name
} UC_PARAMETER;			// Mikrocontrollerspezifische Parameter
#endif

//==================================================================================
// Kommando- und Event-Codes 
//==================================================================================
typedef enum CPP_ENUM_SIZE_8Bit {
    CMD_FLASH_unknown					= 0x00,
	CMD_GET_CAT_FLASH_VERSION      = 0x02, // Version dieser Kommandokategorie auslesen
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT   FWF_APP_FLASH_VERSION_STRING(0) ethernet_config_name() application_name() aus dem Flash

	CMD_FLASH_TEST_ERRMSG		   	= 0x03,	// Provoziert das Senden einer Fehlernachricht
	//PC CAT CMD CNT		~FWF_uCMD_FLASH_TEST_ERRMSG
	//uC CAT CMD CNT STAT	~FWF_uCMD_FLASH_TEST_ERRMSG	errmsg(0)

	CMD_FLASH_GET_CONNECT_INFO     = 0x04, // Infos dieser Kommandokategorie auslesen
	//PC CAT CMD CNT		~FWF_uCMD_GET_CONNECT_INFO
	//uC CAT CMD CNT STAT   ~FWF_uCMD_GET_CONNECT_INFO  Flashsize(0,2) FlashAppConsistency(2,1) FWF_APP_FLASH_VERSION_STRING(3) CONTROLLER_NAME(3, max.CONTROLLER_NAME_LEN)

	CMD_FLASH_CHECK_PROTECTION		= 0x05, // Nachricht fuer Flash Sicherung
	//PC CAT CMD CNT   		~FWF_uCMD_CHECK_FLASH_PROTECTION(0,1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_CHECK_FLASH_PROTECTION(0,1) UC_DEVICE_ID(1,UC_DEVICE_ID) Protection_Level2(13,2)

	CMD_FLASH_GET_SIZE        	 	= 0x06,
	//PC CAT CMD CNT   		~FWF_uCMD_GET_FLASH_SIZE(0,1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_GET_FLASH_SIZE(0,1)  Flashsize(1,2)

	CMD_FLASH_SET2DEFAULT          = 0x08, // Konfigurationsparameter im Flash auf Default setzen
	//PC CAT CMD CNT   		~FWF_uCMD_SET2DEFAULT(0,1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_SET2DEFAULT(0,1) okwriteflash(1,1) okdefault(2,1)

	CMD_FLASH_ERASE_SECTOR         = 0x09, // Flash mit allen Konfigurationsparametern loeschen
	//PC CAT CMD CNT   		~FWF_uCMD_ERASE_SECTOR(0,1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_ERASE_SECTOR(0,1) writestatus(1,1)		(Status = 0xfe Flash locked, = 0xfd Flash write protected)

	CMD_FLASH_SET_PROT         	= 0x0a,
	//PC CAT CMD CNT   		~FWF_uCMD_SET_FLASH_PROT(0,1) lock8 (0=freigeben 1=sperren)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_SET_FLASH_PROT(0,1) Protection Level 1 Flags => unteres Wort

	CMD_FLASH_GET_ETH_PAR          = 0x0b, // Ethernet-Parameter lesen
	//PC CAT CMD CNT   		~FWF_uCMD_GET_ETH_PAR(0,1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_GET_ETH_PAR(0,1) FWF_ETHERNET_SETTINGS(1)

	CMD_FLASH_SET_ETH_PAR             = 0x0c, // Ethernet-Parameter setzen
	//PC CAT CMD CNT   		~FWF_uCMD_SET_ETH_PAR(0,1) FWF_ETHERNET_SETTINGS(1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_SET_ETH_PAR(0,1) okwriteflash(1,1) FWF_ETHERNET_SETTINGS(2)

	CMD_FLASH_GET_CONTROLLER_NAME    = 0x0d,
	//PC CAT CMD CNT		~FWF_uCMD_GET_CONTROLLER_NAME
	//uC CAT CMD CNT STAT  	~FWF_uCMD_GET_CONTROLLER_NAME CONTROLLER_NAME(0, max.CONTROLLER_NAME_LEN)

	CMD_FLASH_SET_CONTROLLER_NAME	= 0x0e,  // Setzen eines Controllernamens
	//PC CAT CMD CNT 		CONTROLLER_NAME(0, max.CONTROLLER_NAME_LEN)
	//uC CAT CMD CNT STAT   ok(0,1) CONTROLLER_NAME(1, max.CONTROLLER_NAME_LEN)

	CMD_FLASH_GET_TEMPERATURE_OFFSET = 0x0f,
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	signed_temperature_offset(0,1)

	CMD_FLASH_SET_TEMPERATURE_OFFSET = 0x10,
	//PC CAT CMD CNT		signed_temperature_offset(0,1)
	//uC CAT CMD CNT STAT  	signed_temperature_offset(1,1)


	CMD_FLASH_GET_APP_UNSPECIFIC_PARS	= 0x11,  // Lesen aller applikationsunspezifischen Parameter
	//PC CAT CMD CNT		~FWF_uCMD_FLASH_GET_APP_UNSPECIFIC_PARS
	//uC CAT CMD CNT STAT  	~FWF_uCMD_FLASH_GET_APP_UNSPECIFIC_PARS   signed_temperature_offset(1,1) flashsize(2,2) FWF_ETHERNET_SETTINGS(4,FWF_ETHERNET_SETTINGS)
	//						UC_DEVICE_ID(1,UC_DEVICE_ID) Protection_Level2(13,2)	FWF_APP_FLASH_VERSION_STRING()
	//						ethernet_flash_consistent ethernet_config_name()  application_flash_consistent application_name()  CONTROLLER_NAME(0, max.CONTROLLER_NAME_LEN)

	CMD_FLASH_SET_APP_NAME             = 0x12, // Applikationsspezifischen Namen setzen
	//PC CAT CMD CNT   		~FWF_uCMD_FLASH_SET_APP_NAME(0,1)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_FLASH_SET_APP_NAME(0,1) okwriteflash(1,1)


	CMD_FLASH_SET_UART_NPARAM        	= 0x13,
	//PC CAT CMD CNT   		~FWF_uCMD_SET_UART_NPARAM(0,1) Anzahl(1,2)
	//uC CAT CMD CNT STAT   ~FWF_uCMD_SET_UART_NPARAM(0,1) okwriteflash(1,1) Anzahl(2,1)


	CMD_FLASH_GET_uC_FLASH_PARSET		= 0x14,  // Lesen des uC_FLASH_PARSET
	//PC CAT CMD CNT		~FWF_uCMD_FLASH_GET_APP_UNSPECIFIC_PARS
	//uC CAT CMD CNT STAT  	~FWF_uCMD_FLASH_GET_APP_UNSPECIFIC_PARS   uC_FLASH_PARSET


	// TESTSEKTION =================================================================
	CMD_DRESDYN_GETTEST_uC_FLASH_PARSET	= 0x21,  // Lesen der relevanten Struktur uC_FLASH_PARSET
	//PC CAT CMD CNT        inject_err(1,1)
	//uC CAT CMD CNT STAT  	size(1,4) uC_FLASH_PARSET(5,)

	CMD_DRESDYN_SETTEST_uC_FLASH_PARSET	= 0x22,  // Vergleich der relevanten Struktur uC_FLASH_PARSET mit Referenzwerten
	//PC CAT CMD CNT        size(1,4) inject_err(5,1) uC_FLASH_PARSET(6,)
	//uC CAT CMD CNT STAT   ok(1,1)   inject_err(2,1)
	DISSECTOR_CMD_CAT_FLASH             = FWF_CAT_FLASH // Dissector Error
} COMMAND_FOR_FLASH;


#if QT_COMPILATION
#include <QObject>
#include "udp_package.h"
#include "fwf_typ_defines.h"

#if FWF_USE_eCMD_FLASH
namespace api_FLASH {
	Q_NAMESPACE
	typedef COMMAND_FOR_FLASH eCmd;
    Q_ENUM_NS(eCmd)
} // namespace api_FLASH
#endif // #if FWF_USE_eCMD_FLASH
#endif // QT_COMPILATION

#pragma pack(1)
typedef struct  {
	  UINT16 mac[3];			/// uC MAC Address
	  UINT16 uc_ip [2];			/// uC IP Address (own IP)
	  UINT16 uCport;			/// uC RxPort
	  UINT16 pc_slowctl_ip[2];  /// default SlowCtl-PC IP Address
	  UINT16 pc_slowctl_port;	/// PC RxPort fuer uC-Startup_message
	  UINT16 pc_stream_ip[2];   /// default Stream-PC IP Address
	  UINT16 pc_stream_port; 	/// PC RxPort fuer Streamen von Sensordaten
	  UINT16 netmask[2];    	/// uC IP Netmask
	  UINT16 dhcp;          	/// High Byte:Nutzung des DHCP-Servers und Low Byte: Anzahl der Wiederholungen bei Versuch, den DHCP-Server zu kontaktieren
	  UINT16 router[2];     	/// Router IP Address
      UINT8  eth_link_retry_count;      /// Anzahl der Versuche zum Aufbau eines Eth-Links (0: Dauerversuche, ansonsten Anzahl)
      UINT8  arps_on_startup_retry_count; /// Anzahl der Versuche von Arp-Anfragen (0: Dauerversuche, ansonsten Anzahl)
} FWF_ETHERNET_SETTINGS;
#pragma pack()


// TESTSEKTION =================================================================

#pragma pack(1)
typedef struct {
	UINT16 msg_version;			// Version der Messages

	SINT8  			uC_temperature_offset;
	UC_DEVICE_SIG 	device_sig;
	UINT16 			flashsize;
	UINT8  			flash_lock;
    COM_FLASH_PARAM_eSTATUS	com_params_fit_to_flash;
	APP_FLASH_PARAM_eSTATUS app_params_fit_to_flash;
	APPLICATION_NAME	com_config_name;
	APPLICATION_NAME	application_name;
	CONTROLLER_NAME 		controller_name;
	FWF_ETHERNET_SETTINGS 	eth_settings;
} uC_FLASH_PARSET;
#pragma pack()


//==================================================================================
#if MICROCONTROLLER_COMPILE_TYPE
void fwf_app_flash_interpreter(FWF_COM_CMD *command, FWF_COM_RESPONSE *resp);
#endif // MICROCONTROLLER_COMPILE_TYPE

#endif // FWF_APP_FLASH_VERSION
