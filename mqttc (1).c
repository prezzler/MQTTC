
#include "fwf_defs.h"
#include "fwf_typ_defines.h"
#include "fwf_dbg.h"

#include "fwf_api_com.h"          	// FWF_COM_CMD MULTICAST_JOIN_LEAVE_STATUS
#include "fwf_api_flash.h"          // CONTROLLER_NAME
#include "fwf_dhcp.h"				// DHCP_STATE
#include "fwf_uc_status.h"

#include "mqttc.h"

#if USE_MQTT_SERVER

//---------------------------------Definition der Funktionen-------------------------------------------------------

UINT8 checkStringLength(PubSubClient* src, int len, const char* s) {
    if (len + 2 + strnlen(s, src->wBufferSize) > src->wBufferSize) {
        // Client_stop(src->_client);
        return 0;
    }
    return 1;
}

UINT32 millis() {    // Millisekunden seit POR
    return uC.timer_1ms;
}

MqttConnect* MqttConnectKonstruktor(PubSubClient* src) //setzt Standartwerte
{
static  MqttConnect tmpConnect;
    tmpConnect.PSCsrc 		= src;
    tmpConnect.id 			= NULL;
    tmpConnect.pass 		= NULL;
    tmpConnect.user 		= NULL;
    tmpConnect.willMessage 	= NULL;
    tmpConnect.willQos 		= MQTTQOS0; // Ka
    tmpConnect.willRetain 	= NULL;
    tmpConnect.willTopic 	= NULL;

    return &tmpConnect;
}

//_______________________________Beginn-PubSubClient-Funktionen_____________________________________________
#if 0
//void setServer(PubSubClient* src, IPAddress ip, uint16_t port)
//{
//
//}

void setServer(PubSubClient* src, uint8_t* ip, uint16_t port) {
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
// personalisierte Callback Funktion uebergeben werden!

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
#endif
void setSocketTimeout(PubSubClient* src, uint16_t timeout)
{
	src->socketTimeout = timeout;
}

void setKeepAlive(PubSubClient* src, uint16_t keepAlive)
{
	src->keepAlive = keepAlive;
}

uint16_t getBufferSize(PubSubClient* src) {
	return src->wBufferSize;
}

PubSubClient* PubSubConstructor(TCP_MQTT_CLIENT_CONTEXT* MQTTConnection) {
static	PubSubClient tmpPubSubClient;
	tmpPubSubClient._state  = MQTT_DISCONNECTED;
    tmpPubSubClient.mqtt_connection = MQTTConnection;
	// tmpPubSubClient._client = NULL;
	// tmpPubSubClient.stream = NULL;
	// setCallback(&tmpPubSubClient, NULL);
	tmpPubSubClient.wBufferSize = 0;
	setKeepAlive    (&tmpPubSubClient, MQTT_KEEPALIVE);
	setSocketTimeout(&tmpPubSubClient, MQTT_SOCKET_TIMEOUT);

	return &tmpPubSubClient;
}

 // TODO:
 // PubSubClient.connected() und Client.connected() 
 // und Client.connect() (mit Domain bzw. IP und Port) muss gemacht werden
char connectStart(PubSubClient* src, MqttConnect* con){
int result = 0;
UINT32 time_ms;
uint16_t llen;
uint32_t len;

    if(!Client_connected(src->mqtt_connection)){
            result = 1;
        } else {
  //              result = Client_connectIp(src,&src->ip, src->port);
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
                src->wBuffer[length++] = d[j];       // schreiben ab Index 5 das 0x00,0x04,'M','Q','T','T',MQTT_VERSION hin und landen an stelle 12
            }
// Als naechstes werden die Connect Flags (8Bit) gesetzt ---------------
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
            src->wBuffer[length++] = v;

            // keepAlive wird an den Buffer angefuegt. keepAlive ist 16 bit daher obere 8 Bits als erstes und dann untere 8+8=16
            src->wBuffer[length++] = ((src->keepAlive) >> 8);
            src->wBuffer[length++] = ((src->keepAlive) & 0xFF);

            // ID Größe wird geprueft und in Buffer hinzugefuegt
            checkStringLength(src, length, con->id); 	// bei &con->id ist es wichtig die Adresse also mit & weiterzugeben, da con->id
            length = writeString( con->id, src->wBuffer,length);
            
            // WillTopic wird geprueft und in Buffer hinzugefuegt
            if (con->willTopic) {
                checkStringLength(src,length,con->willTopic);
                length = writeString(con->willTopic, src->wBuffer,length);
                checkStringLength(src,length, con->willMessage);
                length = writeString(con->willMessage, src->wBuffer,length);
            }

            // Username und Password wird geprueft und in Buffer hinzugefuegt
            if(con->user != NULL) {
                checkStringLength(src, length, con->user);
                length = writeString( con->user, src->wBuffer,length);
                if(con->pass != NULL) {
                    checkStringLength(src,length, con->pass);
                    length = writeString( con->pass, src->wBuffer, length);
                }
            }

            write(src,MQTTCONNECT,length-MQTT_MAX_HEADER_SIZE);

            src->lastInActivity = src->lastOutActivity = millis();

            while (!Client_available(src->mqtt_connection)) {    // while (!_client->available())
            // die Schleife wird wiederholt, bis Information vom Client verfuegbar ist
            // wenn der Rueckgabewert true ist, dann heißt es dass eine Nachricht angekommen ist
            	time_ms = millis();
                // Zeit wird gestoppt
#if 0
                if (time_ms - src->lastInActivity >= ((int32_t)src->socketTimeout * 1000UL)) {
                    src->_state = MQTT_CONNECTION_TIMEOUT;
                    //Client_stop( src->_client);
                    return false;
                }
#endif
            }
            // weil die while Schleife verlassen wurde, gehen wir davon aus, dass eine einkommende Nachricht vorliegt
            len = readPacket(src,&llen);

            if (len == 4) { // wenn die Länge des angekommenen Pakets 4 ist, handelt es sich um ein CONACK
                if (src->wBuffer[3] == 0) {  // letzter Byte eines CONACK ist der RC, wenn der 0 ist --> erfolgreiche Verbindung
                    src->lastInActivity = millis();
                    src->pingOutstanding = false;
                    src->_state = MQTT_CONNECTED;
                    return true;
                }
                else {
                    src->_state = src->wBuffer[3];   // sonst wird der RC Wert des CONACKS in State geschrieben
                }
            }
            //Client_stop(src->_client);
        } else {
            src->_state = MQTT_CONNECT_FAILED;
        }
        return false;
}

UINT8 *disconnect(PubSubClient* src) {
	src->wBuffer[0] = MQTTDISCONNECT;
	src->wBuffer[1] = 0;
	src->lastInActivity = src->lastOutActivity = millis(); //millis ist  fuer arduino, kann man aber auch mit time.h bib realisieren
	return &src->wBuffer[0];
    /*_client->write(this->buffer, 2);
	_state = MQTT_DISCONNECTED;
	_client->flush();
	_client->stop();*/
}

char publish(PubSubClient* src, const char* topic, const char* payload) {
	return 2;
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
        len >>= 7; 			//len = len / 128
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
char readByteIntoBuff(PubSubClient* src, uint16_t* index){
  uint16_t current_index = *index;
  uint8_t * write_address = &(src->wBuffer[current_index]);

  if(readByte(src, write_address)){
    *index = current_index + 1;
    return true;
  }
  return false;
}

#if 0
// reads a byte into result
bool readByte(PubSubClient* src, uint8_t * result) {
   uint32_t previousMillis = millis();
   while(!Client_available(src->_client) ) {  // solange eine einkommende Nachricht verfuegbar ist
     uint32_t currentMillis = millis();
     if(currentMillis - previousMillis >= ((int32_t) src->socketTimeout * 1000)){
       return false;
     }
   }
   *result = Client_read(src->_client);
   return true;
}
#endif

uint32_t readPacket(PubSubClient* src, uint16_t* lengthLength) {
    uint16_t len = 0;
    if(!readByteIntoBuff(src, &len)) return 0;
   // char isPublish = (src->buffer[0]&0xF0) == MQTTPUBLISH; noch nicht notwendig
    uint32_t multiplier = 1;
    uint32_t length = 0;
    uint8_t  digit 	= 0;
    //uint16_t skip 	= 0;
    uint32_t start 	= 0;

    do {
        if (len == 5) {
            // Incoming remaining length encoding - kill the connection
            src->_state = MQTT_DISCONNECTED;
            Client_stop(src->mqtt_connection);
            return 0;
        }
        if(!readByte(src,&digit)) return 0;
        src->wBuffer[len++] = digit;
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

        if (len < src->wBufferSize) {
            src->wBuffer[len] = digit;
            len++;
        }
        idx++;
    }
#if 0
    if (!src->stream && idx > src->bufferSize) {
        len = 0; // This will cause the packet to be ignored.
    }
#endif
    return len;
}

    //_______________________________Ende-PubSubClient-Funktionen__________________________________________________

    //_______________________________Beginn-Client-Funktionen_____________________________________________
void Client_stop(TCP_MQTT_CLIENT_CONTEXT* mqtt_connection) {
	mqtt_connection->state = MQTT_FSM_NONE;
    // if (src->sock == 255)
    //     return;
}

int Client_available(TCP_MQTT_CLIENT_CONTEXT* mqtt_connection){ //Client_available muesste von rossendorf uebernommen werden
	return ( 0 == mqtt_connection )? 0 : 1;
}
UINT8 Client_connected(TCP_MQTT_CLIENT_CONTEXT* mqtt_connection){
	return ( MQTT_FSM_Connected_WORKING == mqtt_connection->state )? 1 : 0;
}

#if 0
UINT8 Client_read(Client* src){
	return 0;
}
#endif

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

char write(PubSubClient* src, uint8_t header, uint16_t length) {
    uint16_t rc;
    uint8_t  hlen = buildHeader(header, src->wBuffer, length);

// #ifdef MQTT_MAX_TRANSFER_SIZE
//     uint8_t* writeBuf = buf+(MQTT_MAX_HEADER_SIZE-hlen);
//     uint16_t bytesRemaining = length+hlen;  //Match the length type
//     uint8_t bytesToWrite;
//     boolean result = true;
//     while((bytesRemaining > 0) && result) {
//         bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE) ? MQTT_MAX_TRANSFER_SIZE:bytesRemaining;
//         rc = _client->write(writeBuf,bytesToWrite);
//         result = (rc == bytesToWrite);
//         bytesRemaining -= rc;
//         writeBuf += rc;
//     }
//     return result;
// #else
    rc = Client_write(&(src->wBuffer[MQTT_MAX_HEADER_SIZE-hlen] ), length+hlen);
    src->lastOutActivity = millis();
    return (rc == hlen+length);
// #endif
}

char Client_write(const uint8_t* buf, uint16_t len) {
	FWF_DBG1_PRINTFv("Pseudo Write Funktion: ");
    for (int i = 0; i < len; i++) {
    	FWF_DBG1_PRINTFv("%u ", buf[i]);  // Use %u for uint8_t type
    }
    FWF_DBG1_PRINTFv("\n");
    return true;
}

struct {
char buffer;
} this;


//_______________________________Ende-Client-Funktionen__________________________________________________

// Return value: erklaeren! // Kaever: erklaeren
UINT8 MQTT_loop(PubSubClient* pub, MqttConnect* con) {
uint16_t llen;
uint16_t len ;
uint16_t msgId ;

if (Client_connected(pub->mqtt_connection)) {
        unsigned long t = millis();
        if ((t - pub->lastInActivity > pub->keepAlive*1000UL) || (t - pub->lastOutActivity > pub->keepAlive*1000UL)) {
            if (pub->pingOutstanding) {
                pub->_state = MQTT_CONNECTION_TIMEOUT;
                Client_stop(pub->mqtt_connection); // client stop funktion
                return false;
            } else {
                pub->wBuffer[0] = MQTTPINGREQ;
                pub->wBuffer[1] = 0;
                write(pub, this.buffer, 2); //client write function
                pub->lastOutActivity = t;
                pub->lastInActivity  = t;
                pub->pingOutstanding = true;
            }
        }

        if (Client_available(pub->mqtt_connection)) { 	//
            len 	= readPacket(pub, &llen);
            msgId 	= 0;
            if (len > 0) {
                pub->lastInActivity = t;
                uint8_t type = pub->wBuffer[0] & 0xF0;
                if (type == MQTTPUBLISH)
                {
#if 0
                    if (pub->callback)  {
                        uint16_t tl = (pub->wBuffer[llen+1]<<8)+pub->wBuffer[llen+2]; /* topic length in bytes */
                        memmove(pub->wBuffer+llen+2, pub->wBuffer+llen+3,tl); 		/* move topic inside buffer 1 byte to front */
                        pub->wBuffer[llen+2+tl] = 0; 								/* end the topic as a 'C' string with \x00 */
                        char *topic = (char*) pub->wBuffer +llen +2;
                        // msgId only present for QOS>0
                        if ((pub->wBuffer[0]&0x06) == MQTTQOS1)
                        {
                            msgId   = (pub->wBuffer[llen+3+tl]<<8)+pub->wBuffer[llen+3+tl+1];
                            payload =  pub->wBuffer+llen+3+tl+2;
                            pub->callback(topic, payload,len-llen-3-tl-2);

                            pub->wBuffer[0] = MQTTPUBACK;
                            pub->wBuffer[1] = 2;
                            pub->wBuffer[2] = (msgId >> 8);
                            pub->wBuffer[3] = (msgId & 0xFF);
                            write(pub, this.buffer,4);
                            pub->lastOutActivity = t;

                        }  else    {
                            payload = this.buffer +llen +3 +tl;
                            pub->callback(topic, payload, len-llen-3-tl);
                        }
                    }
#endif
                } 
                else if (type == MQTTPINGREQ) 
                {
                    pub->wBuffer[0] = MQTTPINGRESP;
                    pub->wBuffer[1] = 0;
                    write(pub, this.buffer,2);
                } 
                else if (type == MQTTPINGRESP) 
                {
                    pub->pingOutstanding = false;
                }
            } 
            else if (!Client_connected(pub->mqtt_connection) )
            {
                // readPacket has closed the connection
                return false;
            }
        } // Client_available
        return true;
    } // if (connected())
    return false;
}

//---------------------------------Ende Definition der Funktionen-------------------------------------------------------

#endif // USE_MQTT_SERVER
