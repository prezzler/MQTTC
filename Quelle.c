#include <stdio.h>
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

// MQTT_KEEPALIVE : keepAlive interval in Seconds. Override with setKeepAlive()
#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 15
#endif

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds. Override with setSocketTimeout()
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 15
#endif

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client.state()
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

//#define CHECK_STRING_LENGTH(l,s) if (l+2+strnlen(s, this->bufferSize) > this->bufferSize) {_client->stop();return false;} 




//___________________________________________________________________________________________________________________________________________________

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

struct PubSubClient{

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
	FILE *stream;
	int _state;


};
typedef struct PubSubClient PubSubClient;


//---------------------------------Connect-Funktion-Opitmierung------------------------------------------------------
struct Connect{
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

Connect* ConnectKonstruktor(PubSubClient* src) //setzt Standartwerte
{
    Connect tmp;
    tmp.PSCsrc = &src;
    tmp.id = NULL;
    tmp.pass = NULL;
    tmp.user = NULL;
    tmp.willMessage = NULL;
    tmp.willQos = NULL;
    tmp.willRetain = NULL;
    tmp.willTopic = NULL;

    return &tmp;
}
//-----------------------------------------------------------------------------------



//---------------------------------Deklarierung der Funktionen-------------------------------------------------------

bool checkStringLength(PubSubClient* src, int l, const char* s); // als Ersatz f�r das "CHECK_STRING_LENGHT"-Makro

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

bool connectStart(PubSubClient* src, Connect* val);
void disconnect(PubSubClient* src); //unvollst�ndig
bool publish(PubSubClient* src, const char* topic, const char* payload);


//TODO �berladung existiert in C nicht, Funktionen m�ssen anderen namen haben
// bool PubSubClient_connect(PubSubClient* src, const char* id);
// bool PubSubClient_connect(PubSubClient* src, const char* id, const char* user, const char* pass);
// bool PubSubClient_connect(PubSubClient* src, const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage);
// bool PubSubClient_connect(PubSubClient* src, const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage);
// bool PubSubClient_connect(PubSubClient* src, const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage, bool cleanSession);






    //_______________________________Beginn-Client-Funktionen_____________________________________________


int connect(Client* src, uint8_t ip, uint16_t port);
int connect(Client* src, const char* host, uint16_t port);
uint8_t connected(Client* src);
int Client_available(Client* src);
void Client_stop(Client* src);
uint8_t Client_connected(Client* src);
// versenden bzw. schreiben von buffer in den Netzwerk Client 
// void Client::write(const uint8_t *buf, size_t size) {
//   if (_sock != 255)
//     send(_sock, buf, size);
// }
// also kann man schonmal eine Pseudo Write Funktion schreiben
bool Client_write(const uint8_t* buf, uint16_t len);

    //_______________________________Ende-Client-Funktionen__________________________________________________

//---------------------------------Ende Deklarierung der Funktionen-------------------------------------------------------


int main()
{
	//PubSubClient *original = Constructor();
	return 0;
}


//void setServer(PubSubClient* src, IPAddress ip, uint16_t port)
//{
//
//}

void setServer(PubSubClient* src, uint8_t* ip, uint16_t port)
{
    if( ip != NULL && port != NULL){
        for(int i = 0; i<4; i++){ // ip size is 4 Byte
            src->ip[i] = ip[i];
        }

        src->port = port;
    }
}

// void setServer(PubSubClient* src, const char* domain, uint16_t port)
// {
    
// }

void setCallback(PubSubClient* src, MQTT_CALLBACK_SIGNATURE callback)
{
	src->callback = callback;
}
// void invokeCallback(PubSubClient* src, char* topic, byte* payload, unsigned int length) {
// 				if (src->callback != NULL) {
// 					src->callback(topic, payload, length);
// 				}
// }
// das braucht man um die callback funktion zuzuweisen
// aber eig soll das ja automatisch passieren, wenn eine Nachricht ankommt
// und diese Information soll dann in invokeCallback(...) als Parameter an die 
// personalisierte Callback Funktion übergeben werden!

void setClient(PubSubClient* src, Client* client)
{
    if (src != NULL ) { 
        src->_client = client;
    }
}

void setStream(PubSubClient* src, FILE* stream)
{
	src->stream = &stream;
}

void setKeepAlive(PubSubClient* src, uint16_t keepAlive)
{
	src->keepAlive = keepAlive;
}

void setSocketTimeout(PubSubClient* src, uint16_t timeout)
{
	src->socketTimeout = timeout;
}

bool setBufferSize(PubSubClient* src, uint16_t size)
{
	if (size == 0) {
		// Cannot set it back to 0
		return false;
	}
	if (src->bufferSize == 0) {
		src->buffer = (uint8_t*)malloc(size);
	}
	else {
		uint8_t* newBuffer = (uint8_t*)realloc(src->buffer, size);
		if (newBuffer != NULL) {
			src->buffer = newBuffer;
		}
		else {
			return false;
		}
	}
	src->buffer = size;
	return (src->buffer != NULL);
}

uint16_t getBufferSize(PubSubClient* src) {
	return src->bufferSize;
}

PubSubClient* Constructor() {
	PubSubClient tmp;
	tmp._state = MQTT_DISCONNECTED;
	//tmp._client = NULL;
    setClient(&tmp, NULL);
	tmp.stream = NULL;
	setCallback(&tmp,NULL);
	tmp.bufferSize = 0;
	setBufferSize(&tmp, MQTT_MAX_PACKET_SIZE);
	setKeepAlive(&tmp, MQTT_KEEPALIVE);
	setSocketTimeout(&tmp, MQTT_SOCKET_TIMEOUT);

	return &tmp;
}

void disconnect(PubSubClient* src) {
	src->buffer[0] = MQTTDISCONNECT;
	src->buffer[1] = 0;
	/*_client->write(this->buffer, 2);
	_state = MQTT_DISCONNECTED;
	_client->flush();
	_client->stop();*/
	src->lastInActivity = src->lastOutActivity = millis(); //millis ist f�r arduino, kann man aber auch mit time.h bib realisieren
}

bool publish(PubSubClient* src, const char* topic, const char* payload) {




}
 // TODO:
 // PubSubClient.connected() und Client.connected() 
 // und Client.connect() (mit Domain bzw. IP und Port) muss gemacht werden
 // weiß nicht woher nextMsgId kommt aber ich schreibs mal als int hin
bool connectStart(PubSubClient* src, Connect* val){ 
    if (!src->connected()) {
        int result = 0;
        if(src->_client->connected()){
            result = 1;
        } else {
            if (src->domain != NULL) {
                result = src->_client->connect(src->domain, src->port);
            }
            else  {
                result = src->_client->connect(src->ip, src->port);
            }
        }

        if (result == 1) {
            src->nextMsgId = 1;
            // Leave room in the buffer for header and variable length field
            uint16_t length = MQTT_MAX_HEADER_SIZE; // 5
            unsigned int j;
        
#if MQTT_VERSION == MQTT_VERSION_3_1
            uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
            for (j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) {
                src->buffer[length++] = d[j];       // schreiben ab Index 5 das 0x00,0x04,'M','Q','T','T',MQTT_VERSION hin und landen an stelle 12
            }
// Als nächstes werden die Connect Flags (8Bit) gesetzt ---------------
// Erinnerung:
//0... .... = User Name Flag: Not set
//.0.. .... = Password Flag: Not set
//..0. .... = Will Retain: Not set
//...0 0... = QoS Level: At most once delivery (Fire and Forget) (0)
//.... .0.. = Will Flag: Not set
//.... ..1. = Clean Session Flag: Set
//.... ...0 = (Reserved): Not set
// | ist der bitweise OR Operator d.h. 0100 0000 | 0000 01000 = 0100 0100 

            uint8_t v;
            if (val->willTopic != NULL) {
                v = 0x04|(val->willQos<<3)|(val->willRetain<<5);
            }else {
                v = 0x00;
            }
            if (val->cleanSession) {
                v = v|0x02;
            }

            if(val->user != NULL) {
                v = v|0x80;

                if(val->pass != NULL) {
                    v = v|(0x80>>1);
                }
            }
            // die Connect Flags werden als ein 1Byte Element mit an den Buffer gehangen
            src->buffer[length++] = v;

            // keepAlive wird an den Buffer angefügt. keepAlive ist 16 bit daher obere 8 Bits als erstes und dann untere 8+8=16
            src->buffer[length++] = ((src->keepAlive) >> 8);
            src->buffer[length++] = ((src->keepAlive) & 0xFF);

            // ID Größe wird geprüft und in Buffer hinzugefügt
            CHECK_STRING_LENGTH(&src, length, &val->id);
            length = writeString(&val->id,&src->buffer,length);
            
            // WillTopic wird geprüft und in Buffer hinzugefügt
            if (val->willTopic) {
                CHECK_STRING_LENGTH(&src,length,&val->willTopic);
                length = writeString(&val->willTopic,src->buffer,length);
                CHECK_STRING_LENGTH(&src,length,&val->willMessage);
                length = writeString(&val->willMessage,src->buffer,length);
            }

            // Username und Password wird geprüft und in Buffer hinzugefügt
            if(val->user != NULL) {
                CHECK_STRING_LENGTH(&src,length,&val->user);
                length = writeString(&val->user,&src->buffer,length);
                if(val->pass != NULL) {
                    CHECK_STRING_LENGTH(&src,length,&val->pass);
                    length = writeString(&val->pass,&src->buffer,length);
                }
            }

            write(&src,MQTTCONNECT,length-MQTT_MAX_HEADER_SIZE);

            src->lastInActivity = src->lastOutActivity = millis();

            while (!Client_available(src->_client)) {    // while (!_client->available())
            // die Schleife wird wiederholt, bis Information vom Client verfügbar ist 
            // wenn der Rückgabewert true ist, dann heißt es dass eine Nachricht angekommen ist   
                unsigned long t = millis();
                //Zeit wird gestoppt
                if (t - src->lastInActivity >= ((int32_t)src->socketTimeout * 1000UL)) {
                    src->_state = MQTT_CONNECTION_TIMEOUT;
                    Client_stop(src->_client);
                    return false;
                }
            }
            // weil, die while Schleife verlassen wurde, gehen wir davon aus, dass eine einkommende Nachricht vorliegt
            uint8_t llen;
            uint32_t len = readPacket(&llen);

            if (len == 4) { // wenn die Länge des angekommenen Pakets 4 ist, handelt es sich um ein CONACK
                if (src->buffer[3] == 0) {  // letzter Byte eines CONACK ist der RC, wenn der 0 ist --> erfolgreiche Verbindung
                    src->lastInActivity = millis();
                    src->pingOutstanding = false;
                    src->_state = MQTT_CONNECTED;
                    return true;
                }
                else {
                    src->_state = src->buffer[3];   // sonst wird der RC Wert des CONACKS in State geschrieben
                }
            }
            Client_stop(src->_client);
        }
        else {
            src->_state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}

//_____________________hier weiterarbeiten, Schritte: Client anlegen, pseudo initialisierung, C++ in C �bersetzen

bool PubSubClient_connect(PubSubClient* src, const char* id) {
    return PubSubClient_connect(&src, id, NULL, NULL, 0, 0, 0, 0, 1);
}

bool PubSubClient_connect(PubSubClient* src, const char* id, const char* user, const char* pass) {
    return PubSubClient_connect(&src, id, user, pass, 0, 0, 0, 0, 1);
}

bool PubSubClient_connect(PubSubClient* src, const char* id, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) {
    return PubSubClient_connect(&src, id, NULL, NULL, willTopic, willQos, willRetain, willMessage, 1);
}

bool PubSubClient_connect(PubSubClient* src, const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage) {
    return PubSubClient_connect(&src, id, user, pass, willTopic, willQos, willRetain, willMessage, 1);
}

bool PubSubClient_connect(PubSubClient* src, const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, bool willRetain, const char* willMessage, bool cleanSession) {
    if (!Client_connected(src->_client)) {
        int result = 0;

        

        if (Client_connected(src->_client)) {
            result = 1;
        }
        else {
            if (src->domain != NULL) {
                result = Client_connect(src->_client, src->domain, src->port);
            }
            else {
                result = Client_connect(src->_client, src->ip, src->port);
            }
        }

        if (result == 1) {
            src->nextMsgId = 1;
            // Leave room in the buffer for header and variable length field
            uint16_t length = MQTT_MAX_HEADER_SIZE;
            unsigned int j;

#if MQTT_VERSION == MQTT_VERSION_3_1
            uint8_t d[9] = { 0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION };
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            uint8_t d[7] = { 0x00,0x04,'M','Q','T','T',MQTT_VERSION };
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
            for (j = 0; j < MQTT_HEADER_VERSION_LENGTH; j++) {
                src->buffer[length++] = d[j];
            }

            uint8_t v;
            if (willTopic) {
                v = 0x04 | (willQos << 3) | (willRetain << 5);
            }
            else {
                v = 0x00;
            }
            if (cleanSession) {
                v = v | 0x02;
            }

            if (user != NULL) {
                v = v | 0x80;

                if (pass != NULL) {
                    v = v | (0x80 >> 1);
                }
            }
            src->buffer[length++] = v;

            src->buffer[length++] = ((src->keepAlive) >> 8);
            src->buffer[length++] = ((src->keepAlive) & 0xFF);

            //CHECK_STRING_LENGTH(length, id)
            checkStringLength(&src, length, id);
                length = writeString(id, src->buffer, length);
            if (willTopic) {
                //CHECK_STRING_LENGTH(length, willTopic)
                checkStringLength(&src, length, willTopic);
                    length = writeString(willTopic, src->buffer, length);
                //CHECK_STRING_LENGTH(length, willMessage)
                    checkStringLength(&src, length, willMessage);
                    length = writeString(willMessage, src->buffer, length);
            }

            if (user != NULL) {
                //CHECK_STRING_LENGTH(length, user)
                checkStringLength(&src, length, user);
                    length = writeString(user, src->buffer, length);
                if (pass != NULL) {
                    //CHECK_STRING_LENGTH(length, pass)
                    checkStringLength(&src, length, pass);
                        length = writeString(pass, src->buffer, length);
                }
            }

            write(MQTTCONNECT, src->buffer, length - MQTT_MAX_HEADER_SIZE);

            src->lastInActivity = src->lastOutActivity = millis();

            while (!Client_available(src->_client)) {    // while (!_client->available())
                unsigned long t = millis();
                if (t - src->lastInActivity >= ((int32_t)src->socketTimeout * 1000UL)) {
                    src->_state = MQTT_CONNECTION_TIMEOUT;
                    Client_stop(src->_client);
                    return false;
                }
            }
            uint8_t llen;
            uint32_t len = readPacket(&llen);

            if (len == 4) {
                if (src->buffer[3] == 0) {
                    src->lastInActivity = millis();
                    src->pingOutstanding = false;
                    src->_state = MQTT_CONNECTED;
                    return true;
                }
                else {
                    src->_state = src->buffer[3];
                }
            }
            Client_stop(src->_client);
        }
        else {
            src->_state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}


int Client_connect(Client* src, uint8_t ip, uint16_t port)
{

}
int Client_connect(Client* src, const char* host, uint16_t port)
{

}
uint8_t Client_connected(Client* src)
{

}

int Client_available(Client* src)
{

}

void Client_stop(Client* src) {
    if (src->sock == 255)
        return;
}

bool checkStringLength(PubSubClient* src, int l, const char* s) {
    if (l + 2 + strnlen(s, src->bufferSize) > src->bufferSize) {
        Client_stop(src->_client);
        return false;
    }
    return true;
}
uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos) {
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);    // high Byte
    buf[pos-i-1] = (i & 0xFF);  // low Byte
    return pos;
}
bool write(PubSubClient* src,uint8_t header, uint16_t length) {
    uint16_t rc;
    uint8_t hlen = buildHeader(header, &src->buffer, length);

// #ifdef MQTT_MAX_TRANSFER_SIZE
//     uint8_t* writeBuf = buf+(MQTT_MAX_HEADER_SIZE-hlen);
//     uint16_t bytesRemaining = length+hlen;  //Match the length type
//     uint8_t bytesToWrite;
//     boolean result = true;
//     while((bytesRemaining > 0) && result) {
//         bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE)?MQTT_MAX_TRANSFER_SIZE:bytesRemaining;
//         rc = _client->write(writeBuf,bytesToWrite);
//         result = (rc == bytesToWrite);
//         bytesRemaining -= rc;
//         writeBuf += rc;
//     }
//     return result;
// #else
    rc = Client_write(&src->buffer+(MQTT_MAX_HEADER_SIZE-hlen),length+hlen);
    src->lastOutActivity = millis();
    return (rc == hlen+length);
// #endif
}

bool Client_write(const uint8_t* buf, uint16_t len) {
    printf("Pseudo Write Funktion: ");
    for (int i = 0; i < len; i++) {
        printf("%u ", buf[i]);  // Use %u for uint8_t type
    }
    printf("\n");
    return true;
}

// Header wird in die ersten 4 Stellen des Buffers geschrieben 
size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length) {
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint16_t len = length;
    do {

        digit = len  & 127; //digit = len %128
        len >>= 7; //len = len / 128
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while(len>0);

    buf[4-llen] = header;
    for (int i=0;i<llen;i++) {
        buf[MQTT_MAX_HEADER_SIZE-llen+i] = lenBuf[i];
    }
    return llen+1; // Full header size is variable length bit plus the 1-byte fixed header
}

#include <sys/time.h>   // das ist ein VSCode Problem: If you're using Visual Studio Code, ensure that the c_cpp_properties.json file in your project or workspace is configured correctly. Open the file and check the includePath setting. Replace "C:/Path/To/Your/Includes" with the actual path to your include directories.
unsigned long millis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    return currentTime.tv_sec * 1000 + currentTime.tv_usec / 1000;
}