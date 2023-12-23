//#if USE_MQTT_SERVER

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
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
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible conues for client.state()
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

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

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




//___________________________________________________________________________________________________________________________________________________
// Wifi bzw. Netzwerk Client
typedef struct  {
    UINT8 ip;
    UINT16 port;
    UINT8 sock;
} Client;


#define TOPIC_LENGTH 128
typedef struct  {
    UINT16 	message_id;
    UINT16 	topic_length;
    char 	topic_name[TOPIC_LENGTH];
    UINT8   Qos;
} Subscription;

typedef struct  {
    UINT16 	message_id;
    UINT16 	topic_length;
    char 	topic_name[TOPIC_LENGTH];
} Unsubscription;

typedef struct {
    UINT16 message_id;
    UINT16 topicLen;
    char topic_name[TOPIC_LENGTH];
    UINT16 remainingLenght;
    UINT8 DUP;
    UINT8 QoS;
    UINT8 Retain;
    UINT16 CleanSession;
    char payload[MQTT_MAX_PACKET_SIZE-MQTT_MAX_HEADER_SIZE];
    UINT8 payloadLen; 
} Publish;

#if 0
// Der FILE Datentyp wird als Platzhalter benutzt, bis die Art des Streams angepasst wird
// daher braucht man vllt sowas:
    // Define FILE type if not already defined
#ifndef FILE
typedef UINT8 *FILE;  // This is a simple placeholder; you may need to replace it with an appropriate type.
#endif
#endif
//ENDE

typedef struct {
    Client* _client;
    UINT8* buffer;
    UINT16 bufferSize;
    UINT16 keepAlive;
    UINT16 socketTimeout;
    UINT16 nextMsgId;
    UINT32 lastOutActivity;
    UINT32 lastInActivity;
    bool 	pingOutstanding;
    //void (*callback)(char*, unsigned char*, unsigned int); // die Behandlungsfunktion alternativ zu callback Kaever
   //  MQTT_CALLBACK_SIGNATURE callback;

    UINT8 ip[4];
    const char* domain;
    UINT16 port;
    UINT8* stream;
    int _state;
} PubSubClient;


//---------------------------------Connect-Funktion-Opitmierung------------------------------------------------------
typedef struct  {
    PubSubClient* PSCsrc;
    const char* id;
    const char* user;
    const char* pass;
    const char* willTopic;
    UINT8 	willQos;
    bool  		willRetain;
    const char* willMessage;
    bool  		cleanSession;
} MqttConnect;
//-----------------------------------------------------------------------------------



//---------------------------------Deklarierung der Funktionen-------------------------------------------------------

char checkStringLength(PubSubClient* src, int l, const char* s); // als Ersatz f�r das "CHECK_STRING_LENGHT"-Makro

UINT32 millis();  // nicht die Beste Lösung, da hier keine präzise Zeit gezählt sondern CPU Clocks pro Sekunde

MqttConnect* MqttConnectKonstruktor(PubSubClient* src); //setzt Standartwerte

    //_______________________________Beginn-PubSubClient-Funktionen_____________________________________________

//void setServer(PubSubClient* src, IPAddress ip, UINT16 port);
//void setServer(PubSubClient* src, UINT8* ip, UINT16 port);
#if 0
void setServer(PubSubClient* src, const char* domain, UINT16 port); // erstmal nur das um ersten Test machen zu können

// Man müsste eine Lösung für die Überladungen suchen!!!!
void setCallback(PubSubClient* src, MQTT_CALLBACK_SIGNATURE callback); // implementierung hinzugefügt
#endif

void setClient(PubSubClient* src, Client* client);
// void setStream(PubSubClient* src, FILE* stream);
void setKeepAlive(PubSubClient* src, UINT16 keepAlive);
void setSocketTimeout(PubSubClient* src, UINT16 timeout);
char setBufferSize(PubSubClient* src, UINT16 size); 		// mittels malloc wurde Speicher freigegeben, wo wird er mittels free freigegeben? Muss wom�glich manuell imer am Ende aufgerufen werden

UINT16 getBufferSize(PubSubClient* src);

PubSubClient* PubSubConstructor(); //Standardkonstruktor, neues anlegen einer Variable mittels "PubSubClient *A=Constructor()"

char connectStart(PubSubClient* src, MqttConnect* con);
void disconnect(PubSubClient* src); //unvollst�ndig
char publish(PubSubClient* src, const char* topic, const char* payload);
size_t buildHeader(UINT8 header, UINT8* buf, UINT16 length);

char readByteIntoBuff(PubSubClient* src, UINT16* index);
bool readByte(PubSubClient* src, UINT8* result);

uint32_t readPacket(PubSubClient* src, UINT16* lengthLength);

char connected();

//_______________________________Ende-PubSubClient-Funktionen__________________________________________________

//_______________________________Beginn-Client-Funktionen_____________________________________________


void Client_stop(Client* src);
UINT16 writeString(const char* string, UINT8* buf, UINT16 pos);
char write(PubSubClient* src, UINT8 header, UINT16 length);
char Client_write(const UINT8* buf, UINT16 len);

// ab jz fehlt alles:
int Client_connectIp(Client* src, UINT8 ip, UINT16 port);
int Client_connectDomain(Client* src, const char* domain, UINT16 port);

UINT8 Client_connected(Client* src);  // Benutzung: Client_connected(&src->_client);
UINT8 Client_read(Client* src);
int Client_available(Client* src); // Benutzung: Client_available(src->_client);
// versenden bzw. schreiben von buffer in den Netzwerk Client 
// void Client::write(const UINT8 *buf, size_t size) {
//   if (_sock != 255)
//     send(_sock, buf, size);
// }
// also kann man schonmal eine Pseudo Write Funktion schreiben

    //_______________________________Ende-Client-Funktionen__________________________________________________

char loop(PubSubClient* pub, MqttConnect* con);

//---------------------------------Ende Deklarierung der Funktionen-------------------------------------------------------
#endif // USE_MQTT_SERVER
