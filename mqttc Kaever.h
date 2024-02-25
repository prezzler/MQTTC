// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1

#if USE_MQTT_NODE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "uip.h"

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

#define MQTT_PORT 				1883

// MQTT_MAX_PACKET_SIZE : Maximum packet size. Override with setBufferSize().
#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 256
#endif

// MQTT_KEEPALIVE : keepAlive intercon in Seconds. Override with setKeepAlive()
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout intercon in Seconds. Override with setSocketTimeout()
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client._state()
#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     (1 << 4)  // Client request to connect to Server
#define MQTTCONNACK     (2 << 4)  // Connect Acknowledgment
#define MQTTPUBLISH     (3 << 4)  // Publish message
#define MQTTPUBACK      (4 << 4)  // Publish Acknowledgment
#define MQTTPUBREC      (5 << 4)  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      (6 << 4)  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     (7 << 4)  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   (8 << 4)  // Client Subscribe request
#define MQTTSUBACK      (9 << 4)  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE (10 << 4) // Client Unsubscribe request
#define MQTTUNSUBACK    (11 << 4) // Unsubscribe Acknowledgment
#define MQTTPINGREQ     (12 << 4) // PING Request C0
#define MQTTPINGRESP    (13 << 4) // PING Response
#define MQTTDISCONNECT  (14 << 4) // Client is Disconnecting
#define MQTTReserved    (15 << 4) // Reserved

#define MQTT_NoRETAIN   (0 << 0)
#define MQTT___RETAIN   (1 << 0)

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

// Maximum size of fixed header and variable length size header
#define MQTT_MAX_HEADER_SIZE 5

//#if defined(ESP8266) || defined(ESP32)
// #include <functional>
// #define MQTT_CALLBACK_SIGNATURE std::function<void(char*, UINT8*, unsigned int)> callback
// #else
// #define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, UINT8*, unsigned int)
// typedef void (*MQTT_CALLBACK_SIGNATURE)(char*, byte*, unsigned int);
//#endif

//#define checkStringLength(l,s) if (l+2+strnlen(s, this->bufferSize) > this->bufferSize) {_client->stop();return false;} 


// The MQTT_FSM server states:
typedef enum CPP_ENUM_SIZE_8Bit {
	MQTT_FSM_NONE		  	 = 0,		/// StateMachine
	MQTT_FSM_TCP_SYN_SENT		,		/// StateMachine
	MQTT_FSM_AWAIT_INIT			,		/// StateMachine
	MQTT_FSM_AWAIT_MQTTConAck	,		/// StateMachine	// Wait for MQTT Connect Acknowledge
	MQTT_FSM_Connected_WORKING	,		/// StateMachine 	// MQTT connect Ack has been received
	MQTT_FSM_INIT_NOACK			,		/// StateMachine
	MQTT_FSM_WORK				,		/// StateMachine
	MQTT_FSM_END							/// StateMachine
} MQTT_state ;


// The MQTT_PUBLISH_states:
typedef enum CPP_ENUM_SIZE_8Bit {
	MQTT_PUBLISH_FirstPublishRequired		= 1,		/// PUBLISH to be sent to Broker
	MQTT_PUBLISH_Scheduled 	,		/// PUBLISH is in process
	MQTT_PUBLISH_SENT		,		/// PUBLISH has been sent to Broker
	MQTT_PUBLISH_REC		,		/// PUBREC has been received
	MQTT_PUBLISH_REL		,		/// PUBREL has been sent
	MQTT_PUBLISH_COMP		,		/// PUBCOMP has been received
	MQTT_PUBLISH_TIMEOUT			/// Broker not responding: Timeout
} MQTT_PUBLISH_state ;

typedef enum CPP_ENUM_SIZE_8Bit {
	MQTT_SUBSCRIBE_FirstSubscribeRequired		= 1,		/// SUBSCRIBE REQ to be sent to Broker
	MQTT_SUBSCRIBE_Scheduled 	,		/// SUBSCRIBE REQ is in process
	MQTT_SUBSCRIBE_SENT		,	    	/// SUBSCRIBE REQ has been sent to Broker
	MQTT_SUBSCRIBE_ACKNOWLEDGED ,       /// SUBSCRIBE ACK has been received
    MQTT_SUBSCRIBE_PUBLISH_ACKED    ,	/// PUBLISH ACK has been sent
    MQTT_SUBSCRIBE_PUBLISH_REC		,	/// PUBREC has been sent
	MQTT_SUBSCRIBE_PUBLISH_REL		,	/// PUBREL has been received
	MQTT_SUBSCRIBE_PUBLISH_COMP		,	/// PUBCOMP has been sent
	MQTT_SUBSCRIBE_PUBLISH_TIMEOUT		/// Broker not responding: Timeout

} MQTT_SUBSCRIBE_state ;


// VORSICHT : Fuer Strukturgleichheit mit struct http_state im allgemeinen Teil sorgen!!!!
typedef struct tcp_MQTT_CLIENT_hasMsg_vars {
  // allgemeiner Teil der Datenstruktur
  struct uip_TCP_conn* 			TCP_connection;
  MQTT_state  state;			// Aktueller Zustand des Servers
  UINT16 countSendData;			// Gibt an, ob zu versendende Daten vorhanden sind (wenn ungleich 0)
  UINT8* pSendData;				// Zeiger auf die zu versendenden Daten
  // Applikationsspezifischer Teil der Serverstruktur
  UINT8  server_index;   		// Index zur Individualisierung der Server Kaever
  UINT32 mqtt_ping_time;		// Zeit seit Verbindungsaufbau oder dem letzten Ping
} TCP_MQTT_CLIENT_CONTEXT;


//___________________________________________________________________________________________________________________________________________________
typedef struct {
	 UINT8 Message_type:4;
	 UINT8 DUP:1;
	 UINT8 Retain:1;	// Broker speichert die Nachricht, um auch an alle zukuenftigen Subs. des Topics zu bedienen
	 UINT8 QoS:2;
} MQTT_MType_DupRetainQos;

typedef union {
	UINT8 						U8;
	MQTT_MType_DupRetainQos 	BA; //Bit Array
} HeaderFlags;

// Netzwerk Client
typedef struct  {
	Union32 ip;	 	// Kaever:
    UINT16 	rPort;	// Remote port des Servers
    //UINT8   sock; 	// Kaever: erklaeren
    UINT8 connected; // Ka
} Client;


#define TOPIC_LENGTH 128 // Maximum topic length

typedef struct  {
	MQTT_SUBSCRIBE_state 	state;
    UINT16 	message_id;
    UINT16 	topic_length;
    uint16_t remainingLength;	// ToDo Wozu
    char 	topic_name[TOPIC_LENGTH];
    UINT8   QoS;
    UINT8 	acked; // Acknowledge has been sent
    HeaderFlags	headerflags;     // Message_type:4; Reserved:4;
} Subscription;


typedef struct  {
    UINT16 	message_id;
    UINT16 	topic_length;
    char 	topic_name[TOPIC_LENGTH];
    UINT8 	acked; 		// Acknowledge has been sent
} Unsubscription;

typedef struct {
	MQTT_PUBLISH_state 	state;
    UINT16 				message_id;
    char  		topic_name [TOPIC_LENGTH];
	UINT8 		topicLen;
    uint16_t 	remainingLenght;
    HeaderFlags	headerflags;     		// Message_type:4; DUP:1; Retain:1; QoS:2;
    uint16_t 	CleanSession;
    UINT32 		lastPublishActivity;	// Timestamp fuer lastPublish
    UINT32 		PublishPeriod;			// PublishPeriod [ms]
} PublishContext;

typedef struct {
    // Empfangene Publish (müssen auf Topic mit Subscription gematcht werden)
    UINT8* 	    payload;
    UINT16 		payloadLen;
    UINT16 		message_id;
    char  		topic_name [TOPIC_LENGTH];
	UINT8 		topicLen;
    uint16_t 	remainingLenght;
    HeaderFlags	headerflags;     		// Message_type:4; DUP:1; Retain:1; QoS:2;
    uint16_t 	CleanSession;
    int Subscription_index;
} RxPublish;



#if 0
// Der FILE Datentyp wird als Platzhalter benutzt, bis die Art des Streams angepasst wird
// daher braucht man vllt sowas:
    // Define FILE type if not already defined
#ifndef FILE
typedef UINT8 *FILE;  // This is a simple placeholder; you may need to replace it with an appropriate type.
#endif
#endif
//ENDE

typedef struct { // Information zum Broker
	TCP_MQTT_CLIENT_CONTEXT* mqtt_connection; 	// zum Versenden von Nachrichten
	// Statusinformation zur Verbindung zum Broker
    UINT8  wBuffer[MQTT_MAX_PACKET_SIZE];		// aktuell zu sendendes Paket 					ToDo: entfernen
    UINT16 wBufferSize;							// Groesse des aktuell zu sendenden Paketes		ToDo: entfernen

    UINT16 keepAlive;	// max. MQTT Wartezeit auf MQTT-Ping
    UINT16 socketTimeout;	// Dauer der Verbindungszeit zum Client
    UINT16 nextMsgId;		// Publish-MsgId
    UINT32 lastOutActivity;	// Timestamp fuer Output
    UINT32 lastInActivity;	// Timestamp fuer Input
    bool   pingOutstanding;	// Info zu ausstehendem Ping
    int _state;				// MQTT Zustand

    //void (*callback)(char*, unsigned char*, unsigned int); // die Behandlungsfunktion alternativ zu callback Kaever
   //  MQTT_CALLBACK_SIGNATURE callback;
    // Broker Verbindungsdaten
    // UINT8 ip[4];		// Broker IP
    // UINT16 port;		// Broker Rx Port : MQTT 1883
    const char* domain; // Broker domain name
    // UINT8* stream;	// unklar
    UINT8  connected;
} PubSubClient;


//---------------------------------Connect-Funktion-Opitmierung------------------------------------------------------
typedef struct  {
    PubSubClient* PSCsrc;
    const char* id;
    const char* user;
    const char* pass;
    const char* willTopic;
    UINT8 		willQos;
    bool  		willRetain;
    const char* willMessage;
    bool  		cleanSession;
} MqttConnect;
//-----------------------------------------------------------------------------------



//---------------------------------Deklarierung der Funktionen-------------------------------------------------------

UINT8 checkStringLength(PubSubClient* src, int l, const char* s); // als Ersatz f�r das "CHECK_STRING_LENGHT"-Makro

UINT32 millis();  // Millisekunden


// MQTT Connect ==============================================
UINT8 isMqttConnect(UINT8 *buffer);
UINT8 createMqttConnectACK(UINT8 *buffer);
UINT8 isMqttConnectACK(UINT8 *buffer);
// PING ===========================================================
UINT8 createMqttPingReq(UINT8 *buffer);
UINT8 isMqttPingRequest(UINT8 *buffer);
UINT8 createMqttPingResp(UINT8 *buffer);
UINT8 isMqttPingResp( PubSubClient* pPubSubClient, UINT8 *buffer);
// PING ===========================================================
char* isMqttSubscribeAck(UINT8* RxBuffer);
Subscription* search_Subscription_topic_match(Subscription aSubscripts[], char* topic, UINT8 MaxSubscriptions);

MqttConnect* MqttConnectKonstruktor(PubSubClient* src); //setzt Standartwerte

    //_______________________________Beginn-PubSubClient-Funktionen_____________________________________________

//void setServer(PubSubClient* src, IPAddress ip, UINT16 port);
//void setServer(PubSubClient* src, UINT8* ip, UINT16 port);
#if 0
void setServer(PubSubClient* src, const char* domain, UINT16 port); // erstmal nur das um ersten Test machen zu können

// Man müsste eine Lösung für die Überladungen suchen!!!!
void setCallback(PubSubClient* src, MQTT_CALLBACK_SIGNATURE callback); // implementierung hinzugefügt
void setClient(PubSubClient* src, Client* client);
void setStream(PubSubClient* src, FILE* stream);
#endif
void setSocketTimeout(PubSubClient* src, UINT16 timeout);

void setKeepAlive(PubSubClient* src, UINT16 keepAlive);

//char setBufferSize(PubSubClient* src, UINT16 size); 		// mittels malloc wurde Speicher freigegeben, wo wird er mittels free freigegeben? Muss womoeglich manuell immer am Ende aufgerufen werden

UINT16 getBufferSize(PubSubClient* src);

PubSubClient* PubSubConstructor(TCP_MQTT_CLIENT_CONTEXT* tcpConnection); //Standardkonstruktor, neues anlegen einer Variable mittels "PubSubClient *A=Constructor()"

char   connectStart(PubSubClient* src, MqttConnect* con);
UINT8* disconnect(PubSubClient* src); //unvollstaendig
char   publish(PubSubClient* src, const char* topic, const char* payload);
size_t buildHeader(UINT8 header, UINT8* buf, UINT16 length);

char readByteIntoBuff(PubSubClient* src, UINT16* index);
#if 0
bool readByte(PubSubClient* src, UINT8* result);
UINT8 Client_read(Client* src);
#endif

uint32_t readPacket(PubSubClient* src, UINT16* lengthLength);

//_______________________________Ende-PubSubClient-Funktionen__________________________________________________

//_______________________________Beginn-Client-Funktionen_____________________________________________

#if 01

//int  Client_connectDomain(Client* src, const char* domain, UINT16 port);
#endif
char Client_write(const UINT8* buf, UINT16 len);
UINT16 writeString(const char* string, UINT8* buf, UINT16 pos);
char write(PubSubClient* src, UINT8 header, UINT16 length);

// ab jz fehlt alles:
int   Client_connectIp(Client* src, UINT8 ip, UINT16 port);

void  Client_stop     (TCP_MQTT_CLIENT_CONTEXT* src);
UINT8 Client_connected(TCP_MQTT_CLIENT_CONTEXT* src);  // Benutzung: Client_connected(&src->_client); // Kaever: erklaeren
int   Client_available(TCP_MQTT_CLIENT_CONTEXT* src);  // Benutzung: Client_available(src->_client); // Kaever: erklaeren

// versenden bzw. schreiben von buffer in den Netzwerk Client 
// void Client::write(const UINT8 *buf, size_t size) {
//   if (_sock != 255)
//     send(_sock, buf, size);
// }
// also kann man schon mal eine Pseudo Write Funktion schreiben

//_______________________________Ende-Client-Funktionen__________________________________________________

UINT8 MQTT_loop(PubSubClient* pub, MqttConnect* con);

//---------------------------------Ende Deklarierung der Funktionen-------------------------------------------------------
#endif // USE_MQTT_NODE

#endif // MQTT_VERSION
