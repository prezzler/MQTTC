#ifndef FWF_APP_COM_VERSION
#define FWF_APP_COM_VERSION "CAT_COM_1.7"
#define FWF_APP_COM_VERSION_NAME_LEN (16 + 1)

//	7 mit com_GotConnect_with_VersionString
//  8 mit com_GotConnect_with_InterfaceVersion
#define FWF_APP_COM_INTERFACEVERSION		0x0009
//==================================================================================
//  FWF        P.Kaever
//  Header zur Kommunikation mit Ethernet-IO aus der Perspektive des Hosts
//  1.0        02-2015  Basiskommunikation mit Implementierung des Kommandointerpreters
//  1.2        mit aufgeteilten Headern fuer Funktionsgruppen mit DHCP-Server
//  1.3		   mit Rueckmeldung des Bearbeitungsstatus im uC als 4. Byte
//  1.4		   mit phys_Value und streaming und UC_DEVICE_ID
//  1.5        Umstellung auf Strukturen COM_CONNECT_INFO, COM_PORT, HARDWARE_STATUS bzw. auf Strings mit 0-Terminierung
//  1.6		   Strukturtests 
//  1.7		   Connect veraendert mit Reduktion auf den PC_SlowCtl_port und Einfuegen von FWF_uCMD_SET_STREAMING_PARAMS sowie Streaming; Umstellung der enums auf CPP_ENUM_SIZE_8Bit
//  FWF_APP_COM_INTERFACEVERSION 9 mit CMD_COM_CON_HOST2 und Erweiterung um APPLICATION_FEATURES_IN_FIRMWARE
//==================================================================================

//==================================================================================
// Informationen zur IP-Adresse sind auf der Webseite des Mikrocontrollers einzusehen
// Zur Vereinfachung sendet der Mikrocontroller bei Startup ein Telegramm an
// den eingestellten Default-Host: FWF_APP_STARTUPPORT_PC
//
// Zur Vereinfachung sendet der Mikrocontroller bei Startup ein Event-Telegramm an
// den eingestellten Default-Host: FWF_APP_STARTUPPORT_PC

//==================================================================================
// Ablauf zur Verbindungsaufnahme:
// 1. Schritt beim Verbindungsaufbau: FWF_uCMD_SET_PCPORT
// Danach sind alle weiteren Befehle moeglich
// Verbindungsabbau durch FWF_uCMD_DISCON
//==================================================================================

//==================================================================================
// Der Aufbau der Schnittstelle fuer UDP-Kommandos und Ereignisse ist wie folgt codiert:
// zunaechst wird das Kommando des PC's beschrieben, in der naechsten Zeile die Antwort des
// Mikrocontrollers.
// Die ersten 3 Byte des vom PC verschickten UDP-Kommandos enthalten Kategorie CAT, Kommando CMD und einen laufenden Zaehler CNT:
// PC Kommandoaufbau: 		CAT CMD CNT
// Die Antwort des Mikrocontrollers enthaelt einen festen Teil mit 4 Byte mit Kategorie, Kommando und dem Zaehler des empfangenen PC-Kommandos
// sowie dem Status STAT der vom Mikrocontroller geschickten Nachricht (Befehlsbearbeitung oder vom Mikrocontroller als Reaktion auf ein Ereignis):
//	uC Nachricht: 	CAT CMD CNT STAT  (Anmerkung: CNT ist bei Event-Nachrichten stets 0, da kein Bezug zu einem PC-Kommando besteht)
//
// Auf die beiden festen Anteile von PC- oder Mikrocontrollernachricht folgen spezifische Nutzdaten, deren Position und Laenge angegeben werden:
// Die Position wird relativ zum Beginn der Nutzdaten notiert, darauf folgt die Laenge (beide in Byte gezaehlt). Bei Datenstrukturen mit nicht
// a priori bekannter Laenge wird nur die Anfangsposition angegeben, z. B. Version(0)
// Beispiel:
// PC Kommandoaufbau: 		CAT CMD CNT 		Motor#(0,1) WindA(1,1) WindB(2,1)
// uC Aufbau der Antwort: 	CAT CMD CNT STAT 	Motor#(0,1) WindA(1,1) WindB(2,1)
//==================================================================================

#if defined(QT_COMPILATION)
#include "udp_package.h"
#else
#include "fwf_defs.h"
#endif
#include "fwf_typ_defines.h"
#include "fwf_api_categories.h"
#include <stdint.h>
#if MICROCONTROLLER_COMPILE_TYPE
#include "HW_pinfkt.h"
#endif
#include "fwf_api_firmwareupdate.h" 	// APP_FLASH_HWINFO

// WIRESHARK_COMMANDS_COMMUNICATION  -- fuer Wireshark Dissector - Namensaequivalenz zu FWF_CAT_COMMUNICATION erforderlich
typedef enum CPP_ENUM_SIZE_8Bit {
	CMD_COMMUNICATION_unknown = 0,
	CMD_COM_CON_HOST                       = 0x05, // Verbindungsaufforderung mit pc_port (PC-Empfangsport) setzen
    //PC CAT CMD CNT 		pc_port(0,1) pc_port(1,1) -> PORT_STRUCT
    //uC CAT CMD CNT STAT  	uc_port(0,2) pc_port(2,2) -> COM_CONNECT_INFO
    //ALT
    //PC CAT CMD CNT 		pc_port(0,1) pc_port(1,1)
    //uC CAT CMD CNT STAT  	uc_port(0,2) pc_port(2,2)

	CMD_COM_CON_HOST2                       = 0x06, // Verbindungsaufforderung mit pc_port (PC-Empfangsport) setzen
    //PC CAT CMD CNT 		PORT_STRUCT
    //uC CAT CMD CNT STAT   COM_CONNECT_INFO + GIT_info + insert_AllVersionInfo

	CMD_COM_DISCON_HOST                    = 0x07, 	// Verbindung zuruecksetzen -> Nur Basis-Befehle werden bearbeitet;
	//PC CAT CMD CNT                            // falls der Master diesen Befehl im nicht verbundenen Zustand sendet, werden alle Verbindungen disconnected
	//uC CAT CMD CNT STAT 						// Disconnect anderer Steuerungen durch den Master-PC auch ohne Verbindungsaufbau


    CMD_COM_GET_CONNECT_INFO  	    		= 0x08, // Infos dieser Kommandokategorie auslesen
    //PC CAT CMD CNT 	    COM_CONNECT_INFO (nur Testcodes)
    //uC CAT CMD CNT STAT  	COM_CONNECT_INFO

	CMD_COM_PING_REQUEST         		    = 0x09, // PING fuer SCANNEN durch PC; PcReceiveUDPPort ist der Empfangsport fuer die Antwort
    //PC CAT CMD CNT	PcReceiveUDPPort(0,2) -> PORT_STRUCT
    //uC CAT CMD CNT STAT	uC_IP_Adress_string(0)

	CMD_COM_PING_CLEAR           		    = 0x0a, // PING fuer SCANNEN durch PC; PcReceiveUDPPort ist der Empfangsport fuer die Antwort
    //PC CAT CMD CNT
    //uC CAT CMD CNT STAT

	CMD_COM_HEARTBEAT              		    = 0x0b, // Heartbeat
    //PC CAT CMD CNT
    //uC CAT CMD CNT STAT

	CMD_COM_GET_CONNECT_PORT           		= 0x0c, // Abfrage der UDP ucPort#
    //PC CAT CMD CNT
    //uC CAT CMD CNT STAT  	rport(0,1) rport(1,1)

    EVT_STARTUP              		    	= 0x0d,  // Event des Mikrocontrollers bei STARTUP an Default host
    //uC CAT CMD CNT STAT  	COM_CONNECT_INFO + GIT_info

	EVT_STARTUP_CONNECT_INFO  		    	= 0x0e,  // Event des Mikrocontrollers bei STARTUP an Default host
	//uC CAT CMD CNT STAT  	COM_CONNECT_INFO

	CMD_COM_GET_uC_HARDWARE_STATUS         = 0x0f,
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  uC_HARDWARE_STATUS_STRUCT

	CMD_COM_SET_STREAMING_PARAMS  	   	    = 0x11,	// Setzen der Streaming-Parameter IP+Port
	//PC CAT CMD CNT 		STREAMING_PARAMETER_STRUCT(0)
	//uC CAT CMD CNT STAT	STREAMING_PARAMETER_STRUCT(0)

	CMD_COM_GET_EXECUTION_TIME_OF_LAST_COMMAND       = 0x13,
	//PC CAT8:CMD8:CNT8:
	//uC CAT8:CMD8:CNT8:STAT CMD_EXECUTION_TIME_STRUCT
	//
	// Konvertierung zu einer Zeit auf PC; z.B. start_time:
	// qint64 start_time =  (qint64(last_start_time_secs) << 32) + last_start_time_subsec;
	// d_starttime = double(start_time)/double(0xffffffffu); // d_starttime in Sekunden

	CMD_COM_STREAMING_ON_OFF  	   		    = 0x15,	// Einschalten des Datensteaming
	//PC CAT CMD CNT 		STREAMING_CMD_STRUCT
	//uC CAT CMD CNT STAT	STREAMING_CMD_STRUCT  	STREAMING_STATUS_STRUCT

	// Debug ===========================================================
	CMD_COM_GET_DEBUG_FLAGS      		    = 0x16, // Lesen der Debug-Flags fuer Port6 und UART
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT 	dbgflags_port6Flags(0,1) dbgflags_uartFlags(1,1) dbgflags_uart(2,1) dbgflags_level(3,1)

	CMD_COM_SET_DEBUG_FLAGS      		    = 0x17, // Setzen der Debug-Flags fuer Port6 und UART
	//PC CAT CMD CNT 		dbgflags_port6Flags(0,1) dbgflags_uartFlags(1,1) dbgflags_uart(2,1) dbgflags_level(3,1)
	//uC CAT CMD CNT STAT

	CMD_COM_GET_DEBUG_INFO  		    	= 0x18, // Debug Info auslesen
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	dbg_log

	CMD_COM_CLEAR_DEBUG_INFO  		    	= 0x19, // Debug Info loeschen
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT

	CMD_COM_TEST_ERRMSG			    		= 0x1a,	// Provoziert das Senden einer Fehlernachricht
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT	errorstring(0)

	CMD_COM_EXEC_COMMAND			    	= 0x1b,
	//PC CAT CMD CNT 		COM_CMD_STRUCT
	//uC CAT CMD CNT STAT  	COM_CMD_STRUCT

	CMD_COM_ACTIVATE_PHYSICAL_VALUES    	= 0x1c,	// Umstellung auf physikalische Werte
	//PC CAT CMD CNT 		CMD_PHYSICAL_VALUES(0,1)
	//uC CAT CMD CNT STAT	CMD_PHYSICAL_VALUES(0,1)

	CMD_COM_GET_ARP_TABLE     		    	= 0x1d, // Lesen der aktuellen ARP-Tabelle
	// PC CAT CMD CNT
    // uC CAT CMD CNT STAT  String (ARP-Tabelle)

	COM_EVT_ARP_ENTRY          		    	= 0x1e,  // Event des Mikrocontrollers bei STARTUP an Default host
	//uC CAT CMD CNT STAT  	String (ARP-Tabelle)

	// APPLICATION FLASH
	CMD_COM_READ_APPLICATION_FLASH_MEMORY	   	= 0x22,	// Speicherbereich lesen
	//PC CAT CMD CNT 		MemLocation(0,1) Address(1,4) Size(5,2)
	//uC CAT CMD CNT STAT	MemLocation(0,1) Address(1,4) Size(5,2) Data()

	CMD_COM_WRITE_APPLICATION_FLASH_MEMORY      = 0x23,	// Speicherbereich schreiben
	//PC CAT CMD CNT 		MemLocation(0,1) Address(1,4) Size(5,2) Data()
	//uC CAT CMD CNT STAT	MemLocation(0,1) Address(1,4) Size(5,2) Data()

	CMD_COM_CLEAR_APPLICATION_FLASH_MEMORY      = 0x24,	// Speicherbereich loeschen
	// Size = Chip  / Sector / Halfblock /  Block
	//PC CAT CMD CNT 		Address(0,4) Size(5,1)
	//uC CAT CMD CNT STAT	Address(0,4) Size(5,1)

// Obsolet START =====================================================
	CMD_COM_GET_CAT_COM_VERSION             = 0x02, // Version dieser Kommandokategorie auslesen, gefolgt von \0
	// => raus
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT	FWF_APP_COM_VERSION(0)

	CMD_COM_GET_PCPORT           			= 0x12, // Abfrage der UDP ucPort#
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	rport(0,1) rport(1,1)


	CMD_COM_GET_CONNECT_INFO_OLD  			= 0x04, // Infos dieser Kommandokategorie auslesen
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	msg_from_ushort_le(TESTCODE_USHORT)(0,2)  msg_from_uint_le  (TESTCODE_UINT)(2,4)
	// 		uip_uCaddr[0](6,2) uCipaddr[1](8,2) uCport(10,2) PC_SlowCtl_port(12,2) PC_Stream_port(14,2)  app_params_fit_to_flash(16,1) UC_DEVICE_ID(17,12)
	// 		CMD_PHYSICAL_VALUES(29,1) STREAMING_STATUS(30,1) FWF_APP_COMBAS_VERSION_STRING(31) + + conname + Versionsinformationen der Mikrocontrollersoftware


	CMD_COM_ALLucVERSIONS                  = 0x14, // AppID + Versionsinformationen der Mikrocontrollersoftware
	//=> raus
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  uC-AppID + IP+lport (0) + Versionenstring

// Obsolet ENDE =====================================================

	// TESTSEKTION =================================================================
	CMD_COM_GETTEST_COM_CONNECT_INFO	    = 0x33,  // Lesen der relevanten Struktur COM_CONNECT_INFO
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	size(0,4) COM_CONNECT_INFO(4,)

	CMD_COM_SETTEST_COM_CONNECT_INFO	    = 0x34,  // Vergleich der relevanten Struktur COM_CONNECT_INFO mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) COM_CONNECT_INFO(4,)
	//uC CAT CMD CNT STAT  	ok(0,1)

	CMD_COM_GETTEST_APPLICATION_FLASH_LOADER_INFO      = 0x35,  // Lesen der relevanten Struktur APPLICATION_FLASH_LOADER_INFO
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	size(0,4) APPLICATION_FLASH_LOADER_INFO(4,)

	CMD_COM_SETTEST_APPLICATION_FLASH_LOADER_INFO	    = 0x36,  // Vergleich der relevanten Struktur APPLICATION_FLASH_LOADER_INFO mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) APPLICATION_FLASH_LOADER_INFO(4,)
	//uC CAT CMD CNT STAT  	ok(0,1)

	CMD_COM_GETTEST_APP_FLASH_INFO_MSG      = 0x37,  // Lesen der relevanten Struktur APP_FLASH_INFO_MSG
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	size(0,4) APP_FLASH_INFO_MSG(4,)

	CMD_COM_SETTEST_APP_FLASH_INFO_MSG	    = 0x38,  // Vergleich der relevanten Struktur APP_FLASH_INFO_MSG mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) APP_FLASH_INFO_MSG(4,)
	//uC CAT CMD CNT STAT  	ok(0,1)

	CMD_COM_GETTEST_STREAMING_CMD_STRUCT	    = 0x39,  // Lesen der relevanten Struktur STREAMING_CMD_STRUCT
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	size(0,4) STREAMING_CMD_STRUCT(4,)

	CMD_COM_SETTEST_STREAMING_CMD_STRUCT	    = 0x3a,  // Vergleich der relevanten Struktur STREAMING_CMD_STRUCT mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) STREAMING_CMD_STRUCT(4,)
	//uC CAT CMD CNT STAT  	ok(0,1)

	CMD_COM_GETTEST_uC_HARDWARE_STATUS_STRUCT = 0x3b,  // Lesen der relevanten Struktur uC_HARDWARE_STATUS_STRUCT
	//PC CAT CMD CNT
	//uC CAT CMD CNT STAT  	size(0,4) uC_HARDWARE_STATUS_STRUCT(4,)

	CMD_COM_SETTEST_uC_HARDWARE_STATUS_STRUCT	    = 0x3c, // Vergleich der relevanten Struktur uC_HARDWARE_STATUS_STRUCT mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) uC_HARDWARE_STATUS_STRUCT(4,)
	//uC CAT CMD CNT STAT  	ok(0,1)

	CMD_COM_GETTEST_PORT_STRUCT			    = 0x3d,  // Lesen der relevanten Struktur PORT_STRUCT
	//PC CAT CMD CNT 		inject_err(0,1)
	//uC CAT CMD CNT STAT  	size(0,4) inject_err(4,1) PORT_STRUCT(5,)

	CMD_COM_SETTEST_PORT_STRUCT			    = 0x3e,  // Vergleich der relevanten Struktur PORT_STRUCT mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) inject_err(4,1) PORT_STRUCT(5,)
	//uC CAT CMD CNT STAT  	ok(0,1)   inject_err(1,1)


	CMD_COM_GETTEST_STREAMING_PARAMETER_STRUCT	    = 0x3f,  // Lesen der relevanten Struktur STREAMING_PARAMETER_STRUCT
	//PC CAT CMD CNT 		inject_err(0,1)
	//uC CAT CMD CNT STAT  	size(0,4) inject_err(4,1) STREAMING_PARAMETER_STRUCT(5,)

	CMD_COM_SETTEST_STREAMING_PARAMETER_STRUCT	    = 0x40,  // Vergleich der relevanten Struktur STREAMING_PARAMETER_STRUCT mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) inject_err(4,1) STREAMING_PARAMETER_STRUCT(5,)
	//uC CAT CMD CNT STAT  	ok(0,1)   inject_err(1,1)

	CMD_COM_GETTEST_APPLICATION_FLASH_APPINFO		    = 0x41,  // Vergleich der relevanten Struktur APPLICATION_INFO mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) inject_err(4,1) APPLICATION_FLASH_APPINFO(5,)
	//uC CAT CMD CNT STAT  	ok(0,1)   inject_err(1,1)

	CMD_COM_SETTEST_APPLICATION_FLASH_APPINFO		    = 0x42,  // Vergleich der relevanten Struktur APPLICATION_INFO mit Referenzwerten
	//PC CAT CMD CNT		size(0,4) inject_err(4,1) APPLICATION_FLASH_APPINFO(5,)
	//uC CAT CMD CNT STAT  	ok(0,1)   inject_err(1,1)

     DISSECTOR_CMD_CAT_COMMUNICATION        = FWF_CAT_COMMUNICATION // Dissector Error
} COMMAND_FOR_COMMUNICATION;


#if QT_COMPILATION
#include <QObject>

#if FWF_USE_eCMD_COM
namespace api_COM {
	Q_NAMESPACE
	typedef COMMAND_FOR_COMMUNICATION eCmd;
    Q_ENUM_NS(eCmd)
} // namespace api_COM
#endif // #if FWF_USE_eCMD_COM

#endif // QT_COMPILATION


#define FWF_APP_PORT_uC     	 2036 // Port des UDP-Kommandointerpreters im Mikrocontroller
#define FWF_APP_STARTUPPORT_PC   2036 // Erwarteter Default-Port fuer Startup-Nachrichten des Mikrocontrollers
#define FWF_APP_STREAMPORT_PC    2036 // Erwarteter Default-Port fuer Startup-Nachrichten des Mikrocontrollers
#define FWF_TFTP_PORT			 69   // Standard TFTP-Port



#define FWF_COMPILE_DATE_LEN 	128 	// Laenge des gesamten Compile Strings
#define FWF_GITCOMMIT_HASH_LEN 	32 		// Laenge des gesamten Git_commit_hash Strings

typedef char COM_VERSION_NAME[FWF_APP_COM_VERSION_NAME_LEN];
typedef char COMPILE_INFO[FWF_COMPILE_DATE_LEN];
typedef char GITCOMMIT_HASH[FWF_GITCOMMIT_HASH_LEN];


typedef enum CPP_ENUM_SIZE_8Bit { CMD_PHYSICAL_VALUES_NONE= 0, CMD_PHYSICAL_VALUES_OFF = 1, CMD_PHYSICAL_VALUES_ON = 2 } CMD_PHYSICAL_VALUES;

#define STREAMING_TEST_NUMBER_OF_STREAMMSG 	5	// Anzahl der testweise gestreamten Pakete
typedef enum CPP_ENUM_SIZE_8Bit {
    STREAMING_CMD_NONE = 0,  
    STREAMING_CMD_OFF,  
    STREAMING_CMD_IOdata_OnCLK,						/// clock-getriebenes Senden von IO-Daten
    STREAMING_CMD_IOdata_OnChange,					/// Von Signalaenderunggen getriebenes Senden von IO-Daten
    STREAMING_CMD_Testdata_OnCLK,					/// Senden von Testdaten
    STREAMING_CMD_Testdata_OnChange,				/// Von Signalaenderunggen getriebenes Senden von IO-Daten
	STREAMING_CMD_Testdata_FixedNumber_OnCLK,  		/// zum Senden von STREAMING_TEST_NUMBER_OF_STREAMMSG Testdaten auf dem Stream-Port
	STREAMING_CMD_IOdata_FixedNumber_OnCLK,   		/// zum Senden von Nachrichten mit IO-Daten auf dem Stream-Port
} STREAMING_eCMD;


#pragma pack(1)
typedef struct {
	UINT16 			msg_version;			/// Version der Messages
	STREAMING_eCMD 	cmd;					/// Kommando
	UINT16			stream_packet_number;	/// Anzahl der zu streamenden Pakete (Messages - nicht Samples)
} STREAMING_CMD_STRUCT;
#pragma pack()


typedef enum CPP_ENUM_SIZE_8Bit { CMD_MULTICAST_NONE  = 0, CMD_MULTICAST_JOIN = 1,  CMD_MULTICAST_LEAVE = 2 } MULTICAST_JOIN_LEAVE_CMD;
typedef enum CPP_ENUM_SIZE_8Bit { MULTICAST_STAT_NONE = 0, MULTICAST_JOINED   = 1,  MULTICAST_LEFT = 2 } 	  MULTICAST_JOIN_LEAVE_STATUS; // ToDo : ist der Typ obsolet??

#pragma pack(1)
typedef struct {
	UINT16 				msg_version;			/// Version der Messages
	STREAMING_eSTATUS 	status;					/// Status
	UINT16				stream_packet_number;		/// Anzahl der zu streamenden Ethernet-Pakete
	UINT16				structs_per_packet_number;	/// Anzahl der Strukturen pro Ethernet-Paket
} STREAMING_STATUS_STRUCT;
#pragma pack()

// Flags fuer COM_CMD_eCMD - Maske
#define COM_CMD__RESETuC					0x01
#define COM_CMD__ALL_MASK	(COM_CMD__RESETuC )

typedef enum CPP_ENUM_SIZE_8Bit {
    COM_CMD__NONE = 0,
	COM_CMD_RESETuC						= COM_CMD__RESETuC
} COM_CMD_eCMD ;

#pragma pack(1)
typedef struct {
	UINT16 			msg_version;	// Version der Messages
	COM_CMD_eCMD 	cmd;			// Kommando
	UINT16 			parameter;		// Parameter zur Kommandoausfuehrung
} COM_CMD_STRUCT;
#pragma pack()


#pragma pack(1)
typedef struct {
	UINT16							interface_version;
	CATEGORIES_FOR_COMMUNICATION 	category;			// Kommandokategorie
} API_VERSION_INFO;
#pragma pack()

#pragma pack(1)
typedef struct {
	UINT16 	msg_version;			// Version der Messages
	UINT16	stream_ip[2];			//
	UINT16  stream_port;			//
} STREAMING_PARAMETER_STRUCT;
#pragma pack()


#pragma pack(1)
typedef struct {
	UINT16 	msg_version;			// Version der Messages
	UINT8	act_command_number;		// Nummer des aktuellen ausgefuehrten Kommandos
	UINT8	last_command_number;	// Nummer des letzten ausgefuehrten Kommandos
	UINT32  act_start_time_secs;	// Beginn der Kommandoausfuehrung beim Eingang des aktuellen Befehls (in Sekunden High Anteil)
	UINT32 	act_start_time_subsec;	// Beginn der Kommandoausfuehrung beim Eingang des aktuellen Befehls (in Sekundenbruchteilen Low Anteil)
	UINT32  last_start_time_secs;	// Beginn der Kommandoausfuehrung beim letzten Befehl (in Sekunden High Anteil)
	UINT32 	last_start_time_subsec;	// Beginn der Kommandoausfuehrung beim letzten Befehl (in Sekundenbruchteilen Low Anteil)
	UINT32  last_complete_time_secs;	// Ende der Kommandoausfuehrung beim letzten Befehl nach Versenden der Antwort (in Sekunden High Anteil)
	UINT32 	last_complete_time_subsec;	// Ende der Kommandoausfuehrung beim letzten Befehl nach Versenden der Antwort (in Sekundenbruchteilen Low Anteil)
} CMD_EXECUTION_TIME_STRUCT;
#pragma pack()


#pragma pack(1)
typedef struct {
	UINT16 msg_version;			// Interfaceversion 
	UINT16 uc_rx_port;
    UINT16 uc_ip_addr[2];
    UINT16 pc_ip_SlowCtl[2];
    UINT16 pc_ip_Stream[2];
    UINT16 pc_slowctl_port;
	UINT16 pc_stream_port;  
	APP_FLASH_PARAM_eSTATUS app_params_fit_to_flash;
    COM_FLASH_PARAM_eSTATUS com_params_fit_to_flash;
    STREAMING_eSTATUS 		streaming_estatus;
	UINT32 					sysstart_csr;		// RCC->CSR ; 		// SCU_SYSSTATUS;
	COM_VERSION_NAME		com_version_name;	// Versionierung von fwf_api_con.h
	COM_VERSION_NAME		flash_version_name; // Versionierung von fwf_api_flash.h
	CONTROLLER_NAME 		controller_name;	// Name des Controllers
	APPLICATION_NAME		application_name;	// Name der Applikation
	APPLICATION_NAME		app_feature_name;	// Klartext fuer Applikations-Features
	APPLICATION_NAME		com_config_name;	// Name der Etherneteinstellungen mit Versionierung
	PCB_NAME				pcb_name;			// Board name
	COMPILE_INFO			compile_date;
	PIT_ID_NAME				pit_id;
	UINT8					Status_MSG_check; 	// Ergebnis der Pruefung, ob die Message SET GET Funktionen der Applikation korrekt sind
	APP_FLASH_HWINFO		AppFlashHW;			// Informationen zum Hardware-Typ des ApplicationFlash
} COM_CONNECT_INFO;
#pragma pack()


#pragma pack(1)
typedef struct {
	UINT16 msg_version;			// Version der Messages
	UINT16 endianness_ushort;	// 
	UINT32 endianness_uint;		//
	UC_DEVICE_SIG device_sig;
	SINT8  uC_temperature_offset;
    SINT16 uC_temperature_value;
    UINT8  flash_lock;
} uC_HARDWARE_STATUS_STRUCT;
#pragma pack()


#pragma pack(1)
typedef struct {
	UINT16	msg_version;			// Version der Messages
	UINT16 	slowctl_port;			// PC Slow Control RX Port
} PORT_STRUCT;
#pragma pack()


// Strukturdefinition zur Steuerung der DRESDYN_SENSORNODE-SM ueber GUI
typedef struct {
	COM_CMD_STRUCT				cmd_struct;
	PORT_STRUCT					*p_port_struct;
	STREAMING_CMD_STRUCT 		*p_STREAMING_CMD_STRUCT;
	COM_CONNECT_INFO 	 		*p_COM_CONNECT_INFO;
	uC_HARDWARE_STATUS_STRUCT 	*p_uC_HARDWARE_STATUS_STRUCT;
	STREAMING_PARAMETER_STRUCT  *p_STREAMING_PARAMETER_STRUCT;
	APP_FLASH_INFO_MSG			*p_APP_FLASH_INFO_MSG;
	//ToDo :Tests nach firmwareupdate verschieben
	APPLICATION_FLASH_LOADER_INFO *p_APPLICATION_FLASH_LOADER_INFO;
	APPLICATION_FLASH_APPINFO	*p_APPLICATION_FLASH_APPINFO;
	//
	PORT_STRUCT					port_struct;
    uC_HARDWARE_STATUS_STRUCT	hw_status;
} COM_FUNCTION_BLOCK;


#if MICROCONTROLLER_COMPILE_TYPE
void   	fwf_app_com_init(void);
void   	fwf_app_com_interpreter(FWF_COM_CMD *command, FWF_COM_RESPONSE *resp);
u16_t  	fwf_app_com_check_event_and_fill_msg( UINT8 *ret_msg );
void 	fwf_app_com_on_connect   (void);
void 	fwf_app_com_on_disconnect(void);
void 	set_MultiCast_mac( void );


UINT16 build_startup_msg_payload_EVT_STARTUP(UINT8 *startup);  // Alte Implementierung mit EVT_STARTUP
UINT16 build_startup_msg_payload_connect_info(UINT8 *startupmsg);

void uip_prepare_multicast_request(MULTICAST_JOIN_LEAVE_CMD cmd);
UINT16 compose_git_info(UINT8* msg);

#endif // MICROCONTROLLER_COMPILE_TYPE

#endif // FWF_APP_COM_VERSION
