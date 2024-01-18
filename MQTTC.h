// MQTT_VERSION : Pick the version
// #define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1

// #if USE_MQTT_SERVER

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MQTT_VERSION_3_1 3
#define MQTT_VERSION_3_1_1 4

#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

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
// #define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client._state()
#define MQTT_CONNECTION_TIMEOUT -4
#define MQTT_CONNECTION_LOST -3
#define MQTT_CONNECT_FAILED -2
#define MQTT_DISCONNECTED -1
#define MQTT_CONNECTED 0
#define MQTT_CONNECT_BAD_PROTOCOL 1
#define MQTT_CONNECT_BAD_CLIENT_ID 2
#define MQTT_CONNECT_UNAVAILABLE 3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED 5

#define MQTTCONNECT (1 << 4)      // Client request to connect to Server
#define MQTTCONNACK (2 << 4)      // Connect Acknowledgment
#define MQTTPUBLISH (3 << 4)      // Publish message
#define MQTTPUBACK (4 << 4)       // Publish Acknowledgment
#define MQTTPUBREC (5 << 4)       // Publish Received (assured delivery part 1)
#define MQTTPUBREL (6 << 4)       // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP (7 << 4)      // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE (8 << 4)    // Client Subscribe request
#define MQTTSUBACK (9 << 4)       // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE (10 << 4) // Client Unsubscribe request
#define MQTTUNSUBACK (11 << 4)    // Unsubscribe Acknowledgment
#define MQTTPINGREQ (12 << 4)     // PING Request
#define MQTTPINGRESP (13 << 4)    // PING Response
#define MQTTDISCONNECT (14 << 4)  // Client is Disconnecting
#define MQTTReserved (15 << 4)    // Reserved

#define MQTTQOS0 (0 << 1)
#define MQTTQOS1 (1 << 1)
#define MQTTQOS2 (2 << 1)

// Maximum size of fixed header and variable length size header
#define MQTT_MAX_HEADER_SIZE 5

// Zur Verwendung des Callbacks
typedef void (*MQTT_CALLBACK_SIGNATURE)(char *, UINT8 *, unsigned int);
//___________________________________________________________________________________________________________________________________________________
// Wifi bzw. Netzwerk Client
// typedef struct
// {
//     UINT8 ip; // Kaever: vermutlich falsch
//     UINT16 port;
//     UINT8 sock; // Kaever: erklaeren
// } Client;

#define TOPIC_LENGTH 128
typedef struct
{
    UINT16 message_id;
    UINT16 topic_length;
    char topic_name[TOPIC_LENGTH];
    UINT8 Qos;
} Subscription;

typedef struct
{
    UINT16 message_id;
    UINT16 topic_length;
    char topic_name[TOPIC_LENGTH];
} Unsubscription;

typedef struct
{
    UINT16 message_id;
    char topic_name[TOPIC_LENGTH];
    UINT8 topicLen;
    uint16_t remainingLenght;
    uint8_t DUP;
    uint8_t QoS;
    uint8_t Retain;
    uint16_t CleanSession;
} Publish;

typedef struct
{ // Information zum Broker
    // Client *_client;        // zum Versenden von Nachrichten
    //  Statusinformation zur Verbindung zum Broker
    UINT8 *buffer;          // aktuell zu sendendes Paket
    UINT16 bufferSize;      // Groesse des aktuell zu sendenden Paketes
    UINT16 keepAlive;       // max. MQTT Wartezeit auf MQTT-Ping
    UINT16 socketTimeout;   //  Gibt an wie lange der Client auf einkommende Information wartet, wenn Daten erwartet werden. Beispielsweise, während dem lesen von einem MQTT Paket.
    UINT16 nextMsgId;       // Publish-MsgId
    UINT32 lastOutActivity; // Timestamp fuer Output
    UINT32 lastInActivity;  // Timestamp fuer Input
    bool pingOutstanding;   // Info zu ausstehendem Ping
    int _state;             // MQTT Zustand
    MQTT_CALLBACK_SIGNATURE callback;
    // void (*callback)(char*, unsigned char*, unsigned int); // die Behandlungsfunktion alternativ zu callback Kaever

} PubSubClient;

//---------------------------------Connect-Funktion-Opitmierung------------------------------------------------------
typedef struct
{
    PubSubClient *PSCsrc;
    const char *id;
    const char *user;
    const char *pwd;
    const char *willTopic;
    UINT8 willQos;
    bool willRetain;
    const char *willMessage;
    bool cleanSession;
    // Broker Verbindungsdaten
    const char *ip;     // Broker IP
    const char *domain; // Broker domain name
    UINT16 port;        // Broker Rx Port : MQTT 1883

} MqttConnect;
//-----------------------------------------------------------------------------------

//---------------------------------Deklarierung der Funktionen-------------------------------------------------------

UINT8 checkStringLength(PubSubClient *src, int l, const char *s); // als Ersatz f�r das "CHECK_STRING_LENGHT"-Makro

UINT32 millis();

void setConnectUser(MqttConnect *src, char *user, char *pwd);
void setConnectWill(MqttConnect *src, char *willTopic, UINT8 willQos, bool willRetain, char *willMessage);
MqttConnect *MqttConnectKonstruktor(PubSubClient *src, char *id, char *ip, UINT16 port) // setzt Standartwerte

    //_______________________________Beginn-PubSubClient-Funktionen_____________________________________________
    PubSubClient *PubSubConstructor(); // Standardkonstruktor, neues anlegen einer Variable mittels "PubSubClient *A=Constructor()"

void setCallback(PubSubClient *src, MQTT_CALLBACK_SIGNATURE callback);
void setSocketTimeout(PubSubClient *src, UINT16 timeout);
void setKeepAlive(PubSubClient *src, UINT16 keepAlive);
// char setBufferSize(PubSubClient *src, UINT16 size); // mittels malloc wurde Speicher freigegeben, wo wird er mittels free freigegeben? Muss womoeglich manuell immer am Ende aufgerufen werden
UINT16 getBufferSize(PubSubClient *src);

bool connected(PubSubClient *src); // Kaever: erklaeren

UINT16 writeString(const char *string, uint8_t *buf, uint16_t pos);

char write(PubSubClient *src, uint8_t header, uint16_t length)

    char connectStart(PubSubClient *src, MqttConnect *con);
UINT8 *disconnect(PubSubClient *src); // unvollstaendig
char publish(PubSubClient *src, const char *topic, const char *payload);
size_t buildHeader(UINT8 header, UINT8 *buf, UINT16 length);

bool readByteIntoBuff(PubSubClient *src, UINT16 *index);
bool readByte(PubSubClient *src, UINT8 *result);

uint32_t readPacket(PubSubClient *src, UINT16 *lengthLength);

//_______________________________Ende-PubSubClient-Funktionen__________________________________________________

void Client_stop(Client *src);
int Client_connectDomain(Client *src, const char *domain, UINT16 port);
#endif
char Client_write(const UINT8 *buf, UINT16 len);
UINT16 writeString(const char *string, UINT8 *buf, UINT16 pos);
char write(PubSubClient *src, UINT8 header, UINT16 length);

// ab jz fehlt alles:
int Client_connectIp(Client *src, UINT8 ip, UINT16 port);

UINT8 Client_connected(Client *src); // Benutzung: Client_connected(&src->_client); // Kaever: erklaeren
UINT8 Client_read(Client *src);
int Client_available(Client *src); // Benutzung: Client_available(src->_client); // Kaever: erklaeren
// versenden bzw. schreiben von buffer in den Netzwerk Client
// void Client::write(const UINT8 *buf, size_t size) {
//   if (_sock != 255)
//     send(_sock, buf, size);
// }
// also kann man schon mal eine Pseudo Write Funktion schreiben

//_______________________________Ende-Client-Funktionen__________________________________________________

UINT8 MQTT_loop(PubSubClient *pub, MqttConnect *con);

//---------------------------------Ende Deklarierung der Funktionen-------------------------------------------------------
#endif // USE_MQTT_SERVER

#endif // MQTT_VERSION
