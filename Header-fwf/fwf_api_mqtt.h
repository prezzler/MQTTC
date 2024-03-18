#ifndef MQTT_HEADER
#define MQTT_HEADER
#define FWF_APP_MQTT_VERSION "CAT_MQTT1.1"

#define FWF_APP_MQTT_INTERFACEVERSION		0x0002

#include "fwf_typ_defines.h"
//==================================================================================
//  FWF        P.Kaever
//  Header zur Kommunikation mit Ethernet-IO
//  1.0        	02-2019  Ueberwachung, Interlock
//  0x0002		mit Testablaeufen fuer Interlock
//==================================================================================

//==================================================================================
// MQTT Ansteuerung
//==================================================================================

//==================================================================================
// Kommando- und Event-Codes 
// WIRESHARK_COMMANDS_MQTT  -- fuer Wireshark Dissector - Namensaequivalenz zu FWF_CAT_MQTT erforderlich
//==================================================================================

#if QT_COMPILATION
#include <QObject>
#include "udp_package.h"
#include "fwf_typ_defines.h"

#if FWF_USE_eCMD_MQTT
namespace api_MQTT {
	Q_NAMESPACE
	typedef COMMAND_FOR_MQTT eCmd;
    Q_ENUM_NS(eCmd)
} // namespace api_MQTT
#endif // #if FWF_USE_eCMD_MQTT
#endif // QT_COMPILATION


#define SWITCH_EVENT_MESSAGE "SWITCH_EVENT OCCURRED"


// Publisher (Kategorie HTW Uebung 0x2a) ==============================================================================================
// Wenn die Publisher-Funktion eingeschaltet ist, sendet der uC zyklisch den aktuellen IO Status (MQTT_PUBLISH_IO_STATUS)
#pragma pack(1)
typedef struct {
    UINT16  msg_version;        // Version des Interface
    UINT8   switches;           // Zustaende der Schalter
    float   voltage_ADC1;       // Spannung am Potentiometer 1
    float   voltage_ADC2;       // Spannung am Potentiometer 2
    UINT8   led;                // LED - Zustaende
} MQTT_PUBLISH_IO_STATUS;
#pragma pack()



#pragma pack(1)
typedef struct {
	UINT16  			msg_version;	    			// Version der Messages
	UINT8   			msg_on_switch_change_enabled;  	//  Switch change event: send message on input change
} MQTT_APPSTATUS;
#pragma pack()


// Strukturdefinition zur Steuerung der MQTT-SM
typedef struct {
    UINT8				msg_publish_event;		//  Publish event: Nachricht zu versenden
    MQTT_APPSTATUS 		app_status;
} MQTT_FUNCTION_BLOCK;

#define MQTT_TIMEOUT_IN_MS				50

#if USE_MQTT_NODE
// #define MQTT_PORT 				1883

#if _FWF_APP_MQTT_MODULE_C
#define extern
#endif

#undef extern


#if _FWF_APP_MQTT_MODULE_C

#define TCP_CMD_SIZE  10
#define TCP_PAR1_SIZE 4
#define TCP_PAR2_SIZE 10

#endif //_FWF_APP_MQTT_MODULE_C

#define MaxSubscriptions_of_this_Node	10
#define MaxPublishStructs_of_this_Node  5


// Standardfunktionen fuer Aufrufinterface
void MQTT_init(void);
void MQTT_Client_init(void); // Verbindung initialisieren

UINT16  MQTT_Client_create_connect(UINT8*  buffer);

// Call fuer TCP_connection

void   fwf_MQTT_Node(u8_t flag);
UINT16 MQTT_Client_Response_Processing(UINT8 *RxBuffer);
UINT16 MQTT_Client_MsgOutstanding(UINT8 * TxBuffer);


void   fwf_MQTT_Broker(u8_t flag);
UINT16 MQTT_ServerRequest_to_Response(UINT8 *msg_in, UINT16 msg_in_len);
UINT16 MQTT_Server_MsgOutstanding(UINT8 * pMsg);

void    init_MQTT_ADC(void);

#endif // USE_MQTT_NODE

#endif // MQTT_HEADER
