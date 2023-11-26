//#include "MQTTC.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <Windows.h>

bool debug = 1;

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

//typedef void (*MQTT_CALLBACK_SIGNATURE)(char*, byte*, unsigned int);

struct Client {

    uint8_t ip;
    uint16_t port;
    uint8_t sock;

};
typedef struct Client Client;


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
  //  MQTT_CALLBACK_SIGNATURE callback;

    //uint8_t ip; // geht nicht da ipv4 Addressen aus 4 8Bit Werten besteht!
    uint8_t ip[4];
    const char* domain;
    uint16_t port;
    // FILE* stream;
    int _state;


};
typedef struct PubSubClient PubSubClient;

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



//---------------------------------Definition der Funktionen-------------------------------------------------------
void copyString(char *dest, const char *src) {
    while (*src != '\0') {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0'; // Don't forget to null-terminate the destination string
}

void printBinary(char ch) {
    // Size of a char in bits (assuming 8 bits per byte)
    int size = sizeof(char) * 8;

    // Loop through each bit from the most significant bit to the least significant bit
    for (int i = size - 1; i >= 0; i--) {
        // Use bitwise AND to check the value of the current bit
        int bit = (ch & (1 << i)) ? 1 : 0;

        // Print the bit
        printf("%d", bit);
    }

    // Add a newline for clarity
    printf("\n");
}


// unsigned long millis() {
//     struct timespec currentTime;
//     clock_gettime(CLOCK_MONOTONIC, &currentTime);

//     // Convert seconds to milliseconds and add the remaining milliseconds
//     return currentTime.tv_sec * 1000 + currentTime.tv_nsec / 1000000;
// }

unsigned long millis() {
    if(debug)
        printf("Debug: millis() hole milis\n");
    return GetTickCount();
}

Connect* ConnectConstructor(PubSubClient* src) //setzt Standartwerte
{
    if(debug)
        printf("Debug: ConnectConstructor()\n");
   Connect* tmp = (Connect*)malloc(sizeof(Connect));

    if (tmp != NULL) {
        // Set default values
        tmp->PSCsrc = src;
        // const char tmpid[9] = "Standart";
       // copyString(tmp->id,"Standart");
        tmp->id = "Standart";
        tmp->pass = NULL;
        tmp->user = NULL;
        tmp->willMessage = NULL;
        tmp->willQos = 0;
        tmp->willRetain = NULL;
        tmp->willTopic = NULL;
    }

    return tmp;
}


    //_______________________________Beginn-PubSubClient-Funktionen_____________________________________________

//void setServer(PubSubClient* src, IPAddress ip, uint16_t port)
//{
//
//}

// void setServer(PubSubClient* src, uint8_t* ip, uint16_t port)
// {
//     if( ip != NULL && port != NULL){
//         for(int i = 0; i<4; i++){ // ip size is 4 Byte
//             src->ip[i] = ip[i];
//         }

//         src->port = port;
//     }
// }

// void setServer(PubSubClient* src, const char* domain, uint16_t port)
// {
    
// }

// void setCallback(PubSubClient* src, MQTT_CALLBACK_SIGNATURE callback)
// {
// 	src->callback = callback;
// }
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

// void setStream(PubSubClient* src, FILE* stream)
// {
// 	src->stream = &stream;
// }

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
	src->bufferSize  = size;
	return (src->buffer != NULL);
}

uint16_t getBufferSize(PubSubClient* src) {
	return src->bufferSize;
}

PubSubClient* Constructor() {
	PubSubClient* tmp = (PubSubClient*)malloc(sizeof(PubSubClient));
    if (debug)
        printf("Debug: Constructor() PubSubClient initialisiert Werte\n");

    if (tmp != NULL) {
        tmp->_state = MQTT_DISCONNECTED;
        setClient(tmp, NULL);
        tmp->bufferSize = 0;
        setBufferSize(tmp, MQTT_MAX_PACKET_SIZE);
        setKeepAlive(tmp, MQTT_KEEPALIVE);
        setSocketTimeout(tmp, MQTT_SOCKET_TIMEOUT);
        tmp->port = 0;
    }

    return tmp;
}


 // TODO:
 // PubSubClient.connected() und Client.connected() 
 // und Client.connect() (mit Domain bzw. IP und Port) muss gemacht werden



    //_______________________________Ende-PubSubClient-Funktionen__________________________________________________

    //_______________________________Beginn-Client-Funktionen_____________________________________________




bool Client_write(const uint8_t* buf, uint16_t len, uint8_t PacketType) {
    // Hier muss in eine Datei geschrieben werden
    char* filename = (char*)malloc(8*sizeof(char));
    if(debug){
        printf("Debug: Client_write() Schreibe in neue Datei\n");
        printf("    Inhalt des Buffers: \n");
        for (int i = 0; i < len; i++) {
            printBinary(buf[i]);
        }
    }
    if(PacketType == MQTTCONNECT){
        strcpy(filename, "CNN.hex");
    }
    if(PacketType == MQTTDISCONNECT){
        strcpy(filename, "DSC.hex");
    }
    FILE* file = fopen(filename , "wb");
    if (file == NULL) {
        perror("    Error opening file");
        return 1;
    }
    fwrite(buf, sizeof(unsigned char), len, file);
    fclose(file);
    /////////////////////////////////////////////////

    printf("\n");
    return true;
}



// Header wird in die ersten 4 Stellen des Buffers geschrieben 
size_t buildHeader(uint8_t header, uint8_t* buf, uint16_t length) {
    if(debug)
        printf("Debug: buildHeader() Erstelle Header\n");
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


bool write(PubSubClient* src, uint8_t header, uint16_t length) {
    if(debug)
        printf("Debug: write()\n");
    uint16_t rc;
    uint8_t hlen = buildHeader(header, src->buffer, length);

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
    rc = Client_write(src->buffer+(MQTT_MAX_HEADER_SIZE-hlen),length+hlen, header);
    src->lastOutActivity = millis();
    return (rc == hlen+length);
// #endif
}


uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos) {
    if(debug)
        printf("Debug: writeString() Schreibe String inkl. Laenge in Buffer\n");
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);    // store the high Byte of the length of the string
    buf[pos-i-1] = (i & 0xFF);  // low Byte
    return pos;
}


int Client_available(){ // hier wird eig im echten geprüft ob vom Socket neue Daten vorliegen 
// der return-wert zeigt normalerweise die Anzahl an Bytes die dem Socket zur verfügung stehen (bzw. wieviele Bytes angekommen sind)

// aus Testgründen verändern wir diese Funktion, dass sie immer available sagt
// da bisher immer auf einen Connect ein ConnectAck folgt 
return 1;
}

void Client_stop() {
    // TODO
    if(debug)
        printf("******Stoppe den Client******\n");
}

bool checkStringLength(PubSubClient* src, int l, const char* s) {
    if(debug){
        printf("Debug: checkStringLenght()\n");
        printf("    %d %d %d\n", l, strnlen(s, src->bufferSize), src->bufferSize);
    }
    if (l + 2 + strnlen(s, src->bufferSize) > src->bufferSize) {
        Client_stop();
        return false;
    }
    return true;
}

// Dient dazu ein Byte aus der Datei in den Buffer zu schreiben
bool readByteOfFileIntoBuff(PubSubClient* src, uint16_t* index, FILE* file){
    // eine weitere Client.h Funktion, die dazu dient einen Byte vom network socket zu lesen 
    // gibt normalerweise den gelesenen byte zurück und sons -1
    if(debug)
        printf("Debug: readByteOfFileIntoBuff()\n");
    // Hier muss in eine Datei geschrieben werden

        // Read and print the contents of the file
        char buffer=0; // Buffer to store read data

    if(fread(&buffer, 1, 1, file) != 0){
        if(debug){
            printf("    Erstes Byte der Datei: ");
            printBinary(buffer);
        }
        src->buffer[*index] = buffer;
        (*index)++;
        return true; // Return true to indicate successful execution
    }
    else 
        return false;
}

uint32_t readPacket(PubSubClient* src, uint8_t* lengthLength,int PacketType ) {
    if(debug)
        printf("Debug: readPacket()\n");
    uint16_t len = 0;
        // File pointer
        FILE* file;
        const char* filePath;
        if(PacketType == MQTTCONNACK){
        // File path (change this to your file's path)
            filePath = "CONNACK.hex";
                if(debug)
                    printf("    Nutze Datei: %s\n", filePath);
        }
        else {
            filePath = "nothing.txt";
                if(debug)
                    printf("    Nutze Datei: %s\n", filePath);
        }
        // Open file for reading ("r" stands for read mode)
        file = fopen(filePath, "r");

        // Check if the file was opened successfully
        if (file == NULL) {
            fprintf(stderr, "   Error opening file %s\n", filePath);
            return 1; // Return an error code
        }

    if(!readByteOfFileIntoBuff(src, &len, file)) return 0;

    uint32_t multiplier = 1;
    uint32_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint32_t start = 0;

    do {
        if (len == 5) {
            // Inconid remaining length encoding - kill the connection
            src->_state = MQTT_DISCONNECTED;
            Client_stop();
            return 0;
        }
        if(fread(&digit, 1, 1, file) == 0) return 0;
        src->buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier <<=7; //multiplier *= 128
    } while ((digit & 128) != 0);
    *lengthLength = len-1;

    for (uint32_t i = start;i<length;i++) {
        if(fread(&digit, 1, 1, file) == 0) return 0;

        if (len < src->bufferSize) {
            src->buffer[len] = digit;
            len++;
        }
    }
    
    return len;
}

bool connectStart(PubSubClient* src, Connect* con){ 
if(debug)
        printf("Debug: connectStart() Verbinde...\n");
//     if (!connected()) {
//         int result = 0;
//   //      if(Client_connected(&src->_client)){														// Gehen davon aus dass die Verbindung steht
//             result = 1;
//         } else {
//             if (src->domain != NULL) {
//  //               result = Client_connectDomain(src,&src->domain, src->port);
//             }
//             else  {
//   //              result = Client_connectIp(src,&src->ip, src->port);
//             }
//         }

//       //  if (result == 1) {
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
            if (con->willTopic != NULL) {
                v = 0x04|(con->willQos<<3)|(con->willRetain<<5);
            }else {
                v = 0x00;
            }
            if (con->cleanSession) {
                v = v|0x02;
            }

            if(con->user != NULL) {
                v = v|0x80;

                if(con->pass != NULL) {
                    v = v|(0x80>>1);
                }
            }
            // die Connect Flags werden als ein 1Byte Element mit an den Buffer gehangen
            src->buffer[length++] = v;

            // keepAlive wird an den Buffer angefügt. keepAlive ist 16 bit daher obere 8 Bits als erstes und dann untere 8+8=16
            src->buffer[length++] = ((src->keepAlive) >> 8);
            src->buffer[length++] = ((src->keepAlive) & 0xFF);

            // ID Größe wird geprüft und in Buffer hinzugefügt
            checkStringLength(src, length, con->id); 
            length = writeString(con->id,src->buffer,length);
            
            // WillTopic wird geprüft und in Buffer hinzugefügt
            if (con->willTopic) {
                checkStringLength(src,length,con->willTopic);
                length = writeString(con->willTopic,src->buffer,length);
                checkStringLength(src,length,con->willMessage);
                length = writeString(con->willMessage,src->buffer,length);
            }

            // Username und Password wird geprüft und in Buffer hinzugefügt
            if(con->user != NULL) {
                checkStringLength(src,length,con->user);
                length = writeString(con->user,src->buffer,length);
                if(con->pass != NULL) {
                    checkStringLength(src,length,con->pass);
                    length = writeString(con->pass,src->buffer,length);
                }
            }

            write(src,MQTTCONNECT,length-MQTT_MAX_HEADER_SIZE);

            src->lastInActivity = src->lastOutActivity = millis();

            while (!Client_available()) {    // while (!_client->available())
            // die Schleife wird wiederholt, bis Information vom Client verfügbar ist 
            // wenn der Rückgabewert true ist, dann heißt es dass eine Nachricht angekommen ist   
                unsigned long t = millis();
                //Zeit wird gestoppt
                if (t - src->lastInActivity >= ((int32_t)src->socketTimeout * 1000UL)) {
                    src->_state = MQTT_CONNECTION_TIMEOUT;
                    Client_stop();
                    return false;
                }
                // wenn es länger als vorgeschrieben dauert eine Antwort von dem Broker zu kriegen, dann wird gestoptt
            }
            // weil, die while Schleife verlassen wurde, gehen wir davon aus, dass eine einkommende Nachricht vorliegt
            uint8_t llen;
            uint32_t len = readPacket(src,&llen,MQTTCONNACK);
            if(debug)
                printf("ReturnCode von readPacket(): Laenge des gelesenen Pakets %d\n", len);
            if (len == 4) { // wenn die Länge des angekommenen Pakets 4 ist, handelt es sich um ein CONACK
               
                if (src->buffer[3] == 0) {  // letzter Byte eines CONACK ist der RC, wenn der 0 ist --> erfolgreiche Verbindung
                    if(debug)
                        printf("CONNACK genehmigt :)\n");
                    src->lastInActivity = millis();
                    src->pingOutstanding = false;
                    src->_state = MQTT_CONNECTED;
                    return true;
                }
                else {
                    src->_state = src->buffer[3];   // sonst wird der RC Wert des CONACKS in State geschrieben
                    if(debug)
                        printf("CONNACK nicht genehmigt :( rc:%b\n", src->buffer[3]);
                }
            }
            Client_stop();
//        }
//        else {
//            src->_state = MQTT_CONNECT_FAILED;
//        }
//        return false;
//    }
    return true;
}

void disconnect(PubSubClient* src, Connect* Con) {
	if(debug) 
        printf("Debug: disconnect()\n");
    src->buffer[0] = MQTTDISCONNECT;
	src->buffer[1] = 0;
	//_client->write(this->buffer, 2);
    Client_write(src->buffer,2,MQTTDISCONNECT);
	src->_state = MQTT_DISCONNECTED;
	//_client->flush(); muss von Rossendorf implementiert werden
	Client_stop();
	src->lastInActivity = src->lastOutActivity = millis(); //millis ist f�r arduino, kann man aber auch mit time.h bib realisieren
    if(debug)
        printf("Gebe Speicher Frei\n");
    free(src);
    free(Con);
}

int main(void) {    
    printf("START\n");
    
    if(debug)
        printf("Debug: main() Haupt\n");
    PubSubClient* Client = Constructor();
    Connect* Con = ConnectConstructor(Client);
    if(debug){
        if (connectStart(Client, Con)) {
            printf("ConnectStart() erfolgreich\n");
        } else {
            printf("ConnectStart() fehlgeschlagen\n");
        }
    }
    disconnect(Client,Con);
    printf("ENDE");
    return 0;
}
