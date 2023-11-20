#include "MQTTC.h"



//int main(){
//	//PubSubClient *original = Constructor();
//	return 0;
//}

//---------------------------------Definition der Funktionen-------------------------------------------------------


bool checkStringLength(PubSubClient* src, int l, const char* s) {
    if (l + 2 + strnlen(s, src->bufferSize) > src->bufferSize) {
        Client_stop(&src->_client);
        return false;
    }
    return true;
}

unsigned long millis() {    // nicht die Beste Lösung, da hier keine präzise Zeit gezählt sondern CPU Clocks pro Sekunde
    return (unsigned long)(clock() * 1000 / CLOCKS_PER_SEC);
    // clock() gibt den Wert für die Anzahl an Ticks der CPU seit Start des Programms
}

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

    //_______________________________Beginn-PubSubClient-Funktionen_____________________________________________

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
	src->bufferSize  = size;
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

 // TODO:
 // PubSubClient.connected() und Client.connected() 
 // und Client.connect() (mit Domain bzw. IP und Port) muss gemacht werden
bool connectStart(PubSubClient* src, Connect* con){ 
    if (!connected()) {
        int result = 0;
        if(Client_connected(&src->_client)){
            result = 1;
        } else {
            if (src->domain != NULL) {
                result = Client_connectDomain(src,&src->domain, src->port);
            }
            else  {
                result = Client_connectIp(src,&src->ip, src->port);
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
            checkStringLength(src, length, &con->id); //bei &con->id ist es wichtig die Adresse also mit & weiterzugeben, da con->id
            length = writeString(&con->id,&src->buffer,length);
            
            // WillTopic wird geprüft und in Buffer hinzugefügt
            if (con->willTopic) {
                checkStringLength(src,length,&con->willTopic);
                length = writeString(&con->willTopic,src->buffer,length);
                checkStringLength(src,length,&con->willMessage);
                length = writeString(&con->willMessage,src->buffer,length);
            }

            // Username und Password wird geprüft und in Buffer hinzugefügt
            if(con->user != NULL) {
                checkStringLength(src,length,&con->user);
                length = writeString(&con->user,&src->buffer,length);
                if(con->pass != NULL) {
                    checkStringLength(src,length,&con->pass);
                    length = writeString(&con->pass,&src->buffer,length);
                }
            }

            write(src,MQTTCONNECT,length-MQTT_MAX_HEADER_SIZE);

            src->lastInActivity = src->lastOutActivity = millis();

            while (!Client_available(&src->_client)) {    // while (!_client->available())
            // die Schleife wird wiederholt, bis Information vom Client verfügbar ist 
            // wenn der Rückgabewert true ist, dann heißt es dass eine Nachricht angekommen ist   
                unsigned long t = millis();
                //Zeit wird gestoppt
                if (t - src->lastInActivity >= ((int32_t)src->socketTimeout * 1000UL)) {
                    src->_state = MQTT_CONNECTION_TIMEOUT;
                    Client_stop(&src->_client);
                    return false;
                }
            }
            // weil, die while Schleife verlassen wurde, gehen wir davon aus, dass eine einkommende Nachricht vorliegt
            uint8_t llen;
            uint32_t len = readPacket(src,&llen);

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
            Client_stop(&src->_client);
        }
        else {
            src->_state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
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

// reads a byte into src->buffer and increments index
bool readByteIntoBuff(PubSubClient* src, uint16_t* index){
  uint16_t current_index = *index;
  uint8_t * write_address = &(src->buffer[current_index]);
  if(readByte(src,write_address)){
    *index = current_index + 1;
    return true;
  }
  return false;
}

// reads a byte into result
bool readByte(PubSubClient* src, uint8_t * result) {
   uint32_t previousMillis = millis();
   while(!Client_available(&src->_client)) {  // solange eine einkommende Nachricht verfügbar ist 
     yield();   // gibt dem Prozessor die Möglichkeit andere Prozesse vorzuziehen (Multithreading)
     uint32_t currentMillis = millis();
     if(currentMillis - previousMillis >= ((int32_t) src->socketTimeout * 1000)){
       return false;
     }
   }
   *result = Client_read(&src->_client);
   return true;
}

uint32_t readPacket(PubSubClient* src, uint16_t* lengthLength) {
    uint16_t len = 0;
    if(!readByteIntoBuff(src, &len)) return 0;
   // bool isPublish = (src->buffer[0]&0xF0) == MQTTPUBLISH; noch nicht notwendig
    uint32_t multiplier = 1;
    uint32_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint32_t start = 0;

    do {
        if (len == 5) {
            // Inconid remaining length encoding - kill the connection
            src->_state = MQTT_DISCONNECTED;
            Client_stop(&(src->_client));
            return 0;
        }
        if(!readByte(src,&digit)) return 0;
        src->buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier <<=7; //multiplier *= 128
    } while ((digit & 128) != 0);
    *lengthLength = len-1;

    // if (isPublish) {
    //     // Read in topic length to calculate bytes to skip over for Stream writing
    //     if(!readByte(this->buffer, &len)) return 0;
    //     if(!readByte(this->buffer, &len)) return 0;
    //     skip = (this->buffer[*lengthLength+1]<<8)+this->buffer[*lengthLength+2];
    //     start = 2;
    //     if (this->buffer[0]&MQTTQOS1) {
    //         // skip message id
    //         skip += 2;
    //     }
    // }
    uint32_t idx = len;

    for (uint32_t i = start;i<length;i++) {
        if(!readByte(src,&digit)) return 0;
        // if (src->stream) {
        //     if (isPublish && idx-*lengthLength-2>skip) {
        //         this->stream->write(digit);
        //     }
        // }

        if (len < src->bufferSize) {
            src->buffer[len] = digit;
            len++;
        }
        idx++;
    }

    if (!src->stream && idx > src->bufferSize) {
        len = 0; // This will cause the packet to be ignored.
    }
    return len;
}

    //_______________________________Ende-PubSubClient-Funktionen__________________________________________________

    //_______________________________Beginn-Client-Funktionen_____________________________________________

void Client_stop(Client* src) {
    // if (src->sock == 255)
    //     return;
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

    //_______________________________Ende-Client-Funktionen__________________________________________________

//---------------------------------Ende Definition der Funktionen-------------------------------------------------------