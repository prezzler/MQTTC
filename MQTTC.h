﻿#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>


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
// #define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback
// #else
// #define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, unsigned int)
typedef void (*MQTT_CALLBACK_SIGNATURE)(char*, byte*, unsigned int);
//#endif

//#define checkStringLength(l,s) if (l+2+strnlen(s, this->bufferSize) > this->bufferSize) {_client->stop();return false;} 




//___________________________________________________________________________________________________________________________________________________
// Wifi bzw. Netzwerk Client
struct Client {

    uint8_t ip;
    uint16_t port;
    uint8_t sock;

};
typedef struct Client Client;

// Der FILE Datentyp wird als Platzhalter benutzt, bis die Art des Streams angepasst wird
// daher braucht man vllt sowas:
    // Define FILE type if not already defined
#ifndef FILE
typedef void FILE;  // This is a simple placeholder; you may need to replace it with an appropriate type.
#endif
//ENDE

struct PubSubClient {

    Client* _client;
    uint8_t* buffer;
    uint16_t bufferSize;
    uint16_t keepAlive;
    uint16_t socketTimeout;
    uint16_t nextMsgId;
    unsigned long lastOutActivity;
    unsigned long lastInActivity;
    bool pingOutstanding;
    MQTT_CALLBACK_SIGNATURE callback;

    //uint8_t ip; // geht nicht da ipv4 Addressen aus 4 8Bit Werten besteht!
    uint8_t ip[4];
    const char* domain;
    uint16_t port;
    FILE* stream;
    int _state;


};
typedef struct PubSubClient PubSubClient;


//---------------------------------Connect-Funktion-Opitmierung------------------------------------------------------
struct Connect {
    PubSubClient* PSCsrc;
    const char* id;
    const char* user;
    const char* pass;
    const char* willTopic;
    uint8_t willQos;
    bool willRetain;
    const char* willMessage;
    bool cleanSession;
};
typedef struct Connect Connect;
//-----------------------------------------------------------------------------------



//---------------------------------Deklarierung der Funktionen-------------------------------------------------------

bool checkStringLength(PubSubClient* src, int l, const char* s); // als Ersatz f�r das "CHECK_STRING_LENGHT"-Makro

unsigned long millis();  // nicht die Beste Lösung, da hier keine präzise Zeit gezählt sondern CPU Clocks pro Sekunde

Connect* ConnectKonstruktor(PubSubClient* src); //setzt Standartwerte

    //_______________________________Beginn-PubSubClient-Funktionen_____________________________________________

//void setServer(PubSubClient* src, IPAddress ip, uint16_t port);
//void setServer(PubSubClient* src, uint8_t* ip, uint16_t port);
void setServer(PubSubClient* src, const char* domain, uint16_t port); // erstmal nur das um ersten Test machen zu können
// Man müsste eine Lösung für die Überladungen suchen!!!!
void setCallback(PubSubClient* src, MQTT_CALLBACK_SIGNATURE callback); // implementierung hinzugefügt
void setClient(PubSubClient* src, Client* client);
void setStream(PubSubClient* src, FILE* stream);
void setKeepAlive(PubSubClient* src, uint16_t keepAlive);
void setSocketTimeout(PubSubClient* src, uint16_t timeout);
bool setBufferSize(PubSubClient* src, uint16_t size); // mittels malloc wurde Spicher freigegeben, wo wird er mittels free freigegeben? Muss wom�glich manuell imer am Ende aufgerufen werden

uint16_t getBufferSize(PubSubClient* src);

PubSubClient* Constructor(); //Standardkonstruktor, neues anlegen einer Variable mittels "PubSubClient *A=Constructor()"

bool connectStart(PubSubClient* src, Connect* con);
void disconnect(PubSubClient* src); //unvollst�ndig
bool publish(PubSubClient* src, const char* topic, const char* payload);
size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length);

bool readByteIntoBuff(PubSubClient* src, uint16_t* index);
bool readByte(PubSubClient* src, uint8_t* result);

uint32_t readPacket(PubSubClient* src, uint16_t* lengthLength);

bool connected();

//_______________________________Ende-PubSubClient-Funktionen__________________________________________________

//_______________________________Beginn-Client-Funktionen_____________________________________________


void Client_stop(Client* src);
uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos);
bool write(PubSubClient* src, uint8_t header, uint16_t length);
bool Client_write(const uint8_t* buf, uint16_t len);

// ab jz fehlt alles:
int Client_connectIp(Client* src, uint8_t ip, uint16_t port);
int Client_connectDomain(Client* src, const char* domain, uint16_t port);

uint8_t Client_connected(Client* src);  // Benutzung: Client_connected(&src->_client);
uint8_t Client_read(Client* src);
int Client_available(Client* src); // Benutzung: Client_available(src->_client);
// versenden bzw. schreiben von buffer in den Netzwerk Client 
// void Client::write(const uint8_t *buf, size_t size) {
//   if (_sock != 255)
//     send(_sock, buf, size);
// }
// also kann man schonmal eine Pseudo Write Funktion schreiben

    //_______________________________Ende-Client-Funktionen__________________________________________________

bool loop(PubSubClient* pub, Connect* con);

//---------------------------------Ende Deklarierung der Funktionen-------------------------------------------------------
