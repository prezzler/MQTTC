
#include "fwf_defs.h"
#include "fwf_typ_defines.h"
#include "fwf_dbg.h"

#include "fwf_api_com.h"          	// FWF_COM_CMD MULTICAST_JOIN_LEAVE_STATUS
#include "fwf_api_flash.h"          // CONTROLLER_NAME
#include "fwf_dhcp.h"				// DHCP_STATE
#include "fwf_uc_status.h"

#include "mqttc.h"

#if USE_MQTT_NODE





// COMMUNICATION FUNCTIONS ======================================================================================
// COMMUNICATION FUNCTIONS ======================================================================================
// COMMUNICATION FUNCTIONS ======================================================================================

// MQTT Connect ==============================================
UINT8 isMqttConnect(UINT8 *buffer){
	if(buffer[0] != MQTTCONNECT ) return 0;
	if(buffer[2] !=  0)  return 0; // MQTT-Laenge
	if(buffer[3] !=  4)  return 0; // MQTT-Laenge
	if(buffer[4] != 'M') return 0;
	if(buffer[5] != 'Q') return 0;
	if(buffer[6] != 'T') return 0;
	if(buffer[7] != 'T') return 0;
	return 1;
}
UINT8 createMqttConnectACK(UINT8 *buffer){
	buffer[0] = MQTTCONNACK;
	buffer[1] = 2;				// Length
	buffer[2] = 0;				// Acknowledge Flags
	buffer[3] = 0;				// Connection accepted
	return 4;
}

UINT8 isMqttConnectACK(UINT8 *buffer){
	if(buffer[0] != MQTTCONNACK ) return 0;
	if(buffer[1] !=  2)  return 0; // MQTT-Laenge
	if(buffer[2] !=  0)  return 0; // MQTT Ack Flag
	if(buffer[3] !=  0)  return 0; // MQTT Return Code
	return 1;
}


// PING ===========================================================
UINT8 createMqttPingReq(UINT8 *buffer){
    buffer[0] = MQTTPINGREQ;  	// Fixed Header nur 1 byte
    buffer[1] = 0;  			// Fixed Header nur 1 byte
    return 2; //
}
UINT8 isMqttPingRequest(UINT8 *buffer){
    if((buffer[0] & 0xf0) != MQTTPINGREQ) return 0; //Fixed Header nur 1 byte
    return 1;
}

UINT8 createMqttPingResp(UINT8 *buffer){
    buffer[0] = MQTTPINGRESP;  // Fixed Header nur 1 byte
    buffer[1] = 0;  // Fixed Header nur 1 byte
    return 2; //warum 4?
}
UINT8 isMqttPingResp( PubSubClient* pPubSubClient, UINT8 *buffer){
    if((buffer[0]& 0xf0) != MQTTPINGRESP) return 0;
    pPubSubClient->pingOutstanding = false;
    return 1;
}

// SUBSCRIPTION ===========================================================

Subscription* isMqttSubscribeAck(Subscription aSubscripts[], UINT8 *buffer, UINT16 msg_length, UINT8 MaxSubscriptions ){
	UINT8 	mqtt_msg_len;
	UINT8   buf_index 			= 0;
	UINT8 	subs_index 	= 0;
	UINT16  message_id;
	UINT8   return_code;

    if( (buffer[buf_index] & 0xf0) != MQTTSUBACK)  return 0; 	// Client Subscribe Acknowledgement + Reserved
    mqtt_msg_len = buffer[buf_index + 1];  					// Subscr. Message Length
    message_id   = (buffer[buf_index+2] << 8) + buffer[buf_index+3]; 				// Message ID
    return_code  = buffer[buf_index+4]; 						// Return Code

    // Check if the message ID matches any subscription in the array
    for(subs_index = 0; subs_index < MaxSubscriptions; subs_index++) {
        if(aSubscripts[subs_index].message_id == message_id ) {
            aSubscripts[subs_index].QoS = return_code;
            aSubscripts[subs_index].state = MQTT_SUBSCRIBE_ACKNOWLEDGED; // Set the subscription state to ACKNOWLEDGED
            return &aSubscripts[subs_index]; // Found a matching subscription
        }
    }

	return 0; // No matching subscription found
}

Subscription* search_Subscription_topic_match(Subscription aSubscripts[] , char* topic, UINT8 MaxSubscriptions){
	UINT8 index = 0;
	for (index = 0; index < MaxSubscriptions; index++){
		if(strcmp(topic, aSubscripts[index].topic_name) == 0)
			return &aSubscripts[index];
	}
	return NULL;
}

UINT8 mqtt_SubscribeStructInit(Subscription* pSubscript, UINT16 msgId, char* name, UINT8 QoS, PublishBrokerContext* rxPublish){
	pSubscript->state 			= MQTT_SUBSCRIBE_FirstSubscribeRequired;
	pSubscript->headerflags.U8 	= MQTTSUBSCRIBE | 2; 		// 2 fuer reserved flags
	pSubscript->message_id 		= msgId;
	strcpy(pSubscript->topic_name, name);
	pSubscript->topic_length	= strlen(name);
	pSubscript->remainingLength = 5; // ToDo
	pSubscript->QoS 			= QoS;
    pSubscript->RxPublish = rxPublish; // übernehmen des zugewiesenen PublishBrokerContext;
	return 0;
}

Subscription* mqtt_find_SubscriptionStruct_which_are_Scheduled(Subscription aSubscripts[], UINT8 MaxSubscriptions){
	UINT8 index = 0;

	for (index = 0 ; index < MaxSubscriptions; index++){
		if(!(aSubscripts[index].topic_length)) break;
		if( (MQTT_SUBSCRIBE_FirstSubscribeRequired == aSubscripts[index].state) ){			// first publish required?
			aSubscripts[index].state = MQTT_SUBSCRIBE_Scheduled;
			return &aSubscripts[index];
		}
	}
	for (index = 0 ; index < MaxSubscriptions; index++){
		if(!(aSubscripts[index].topic_length)) break;
		if( (MQTT_SUBSCRIBE_Scheduled == aSubscripts[index].state) ){						// first publish required?
			aSubscripts[index].state = MQTT_SUBSCRIBE_Scheduled;
			return &aSubscripts[index];
		}
	}
	return 0; // Keine Aktivitaet erforderlich
}

UINT16 createSubscribeReqMsg(Subscription* pSubscript, UINT8 *TxBuf) {
    UINT16 len;
    if (pSubscript->state == MQTT_SUBSCRIBE_Scheduled) {
        if (!(pSubscript->topic_length)) return 0;
        TxBuf[0] = pSubscript->headerflags.U8; // Header
        len = sprintf((char*)&TxBuf[6], "%s", pSubscript->topic_name);
        TxBuf[2] = 	pSubscript->message_id >> 8;		// Message ID MSB
        TxBuf[3] = 	pSubscript->message_id;		// Message ID LSB
        TxBuf[4] = 	(UINT8)(len >> 8);
        TxBuf[5] =	(UINT8)len ;  // Topic_Length
        TxBuf[1] = 2 + 2 + len + 1;  // MQTT_Msg_Length = 2 Bytes Message ID + 2 Bytes Topic_Length + len for topics + QoS
        TxBuf[6 + len] = pSubscript->QoS; // QoS level
        pSubscript->state = MQTT_SUBSCRIBE_SENT;
        return TxBuf[1] + 2;  // Length of the message is MQTT_Msg_Length + 2
    }
    return 0;
}

// Prüft ob der eingehende Buffer ein Publish ist und speichert die Daten in publ
// checkt ob der publish topic auch in einer Subscription mit state = MQTT_SUBSCRIBE_ACKED ist
UINT8 isMqttRxPublish(UINT8 *buffer, PublishBrokerContext* publ){
	UINT16 len;
	int payloadLen;
	int index = 0; //index of buffer

	if((buffer[0]& 0xf0)!= MQTTPUBLISH) return 0; //bit 4-7

	publ->headerflags.BA.DUP    = (buffer[0] & 0x08) >> 3; 		// Bit 3 , DUP bei QoS 0 immer 0
	publ->headerflags.BA.QoS    = (buffer[0] & 0x06) >> 1; 		// Bit 2 und 1
	publ->headerflags.BA.Retain = (buffer[0] & 0x01);    		// Bit 0 , wenn true (1) dann speichert der Server die Nachricht um auch an alle zukünftigen Subs. des Topics zu schicken
	publ->remainingLength 		= buffer[1]; 					// MQTT_Msg_Length index 1

	len = ( (UINT16)buffer[2] << 8 ) + buffer[3]; 		//topic length: index 2,3
	publ->topicLen = len;

	if(len > TOPIC_LENGTH){
		FWF_DBG1_PRINTFv("Length > TOPIC_LENGHT");
		return 0;
	}

	strncpy(publ->topic_name, (char*)&buffer[4], publ->topicLen);	// index 4
	publ->topic_name[len] = '\0'; 								// TODO: notwendig?
	index = 4 + len;

    // Wenn QoS > 0 dann ist die Message ID enthalten
	if(publ->headerflags.BA.QoS > 0){
		publ->message_id = (buffer[index] << 8) + buffer[index+1]; 	// Ist nur bei QoS > 0 enthalten
	}
	index += 2;

    payloadLen = publ->remainingLength - (index - 1);   			// remLen - länge seit remLen (remLen fängt bei 1 an)
    publ->payloadLen = payloadLen;

	strncpy((char*) publ->payload , (char*)&buffer[index], payloadLen);		// Wozu das?
	return 1;
}


// Speichere die empfangene Publish Nachricht in der zugehoerigen Subscription (Uebertragen aller Daten aus der temporaere Publish zur Subscription)
void copyPublishToSubscription(Subscription* sub, PublishBrokerContext* pub, int index) {
     if (sub->RxPublish == NULL) {
        FWF_DBG1_PRINTFv("RxPublish is NULL");  //optional
        return;
    }
    sub->RxPublish->headerflags.U8 	= pub->headerflags.U8;
    sub->RxPublish->message_id 		= pub->message_id;
    sub->RxPublish->topicLen 		= pub->topicLen;
    strcpy(sub->RxPublish->topic_name, pub->topic_name);

    sub->RxPublish->remainingLength = pub->remainingLength;
    sub->RxPublish->payloadLen 		= pub->payloadLen;
    strcpy((char*) sub->RxPublish->payload,(char*) pub->payload);
    sub->RxPublish->Subscription_index = index;
    sub->state = MQTT_SUBSCRIBE_PUBLISH_RECEIVED;
}

// Welche Subscription hat das selbe Topic wie die empfangene Publish Nachricht?
// Rueckgabe: Index der Subscription
int CheckTopicRxPub(Subscription aSubscripts[], PublishBrokerContext* aPublish, UINT8 MaxSubscriptions){
	for(int i = 0; i < MaxSubscriptions; i++){
        if(&aSubscripts[i] != NULL){    // wenn Subscription existiert

            if( strcmp(aSubscripts[i].topic_name, aPublish->topic_name) == 0    // richtige subscription gefunden 
            && aSubscripts[i].state == MQTT_SUBSCRIBE_ACKNOWLEDGED){
                copyPublishToSubscription(&aSubscripts[i], aPublish, i);
                return i;
            }
        }
	}
	FWF_DBG1_PRINTFv("Publish Topic didn't match a Publish Context");
	return -1;
}


// Entspricht Message ID von empfanganem Pub Rel der Message ID von Publish?
int CheckTopicPubRel(Subscription aSubscripts[], PublishBrokerContext* aPublish, UINT8 MaxSubscriptions){
    for (int i = 0; i<MaxSubscriptions; i++){
        if(&aSubscripts[i] != NULL ){

            if((aSubscripts[i].state == MQTT_SUBSCRIBE_PUBLISH_REC) 
            && (aSubscripts[i].RxPublish->message_id == aPublish.message_id)){
                aSubscripts[i].RxPublish->state = MQTT_SUBSCRIBE_PUBLISH_REL;    // Publish Rel has been received
                return i;
            }
        }
    }
    return 0;

}


// PUBLISH ===========================================================

// Alle relevanten PublishStruct's (topicLen und MQTT_PUBLISH_SENT) auf MQTT_PUBLISH_Scheduled setzen
void mqtt_PublishStructALL_SetPublishScheduled(PublishNodeContext aPublishs[], UINT8 MaxPublishStructs){
UINT8 index = 0;
	for (index = 0 ; index < MaxPublishStructs; index++){
		if(!(aPublishs[index].topicLen)) continue;
		if( MQTT_PUBLISH_SENT == aPublishs[index].state ){
			aPublishs[index].state = MQTT_PUBLISH_Scheduled;
		}
	}
}

// gibt es eine erforderliche Aktivitaet bei einer Publish-Struktur?
// array aPublish[] durchsuchen
// Bei timeout alle aPublish durch mqtt_PublishStructALL_SetPublishScheduled() aktualisieren
PublishNodeContext* mqtt_find_PublishStruct_which_are_Scheduled (PublishNodeContext aPublishs[], UINT8 MaxPublishStructs){
UINT8 index = 0;
	for (index = 0 ; index < MaxPublishStructs; index++){ 	// first publish required?
		if(!(aPublishs[index].topicLen)) continue;
		if( (MQTT_PUBLISH_FirstPublishRequired == aPublishs[index].state) ){			// first publish required?
			aPublishs[index].state = MQTT_PUBLISH_Scheduled;
		}
	}
	for (index = 0 ; index < MaxPublishStructs; index++){ 	// Timeout exceeded?
		if(!(aPublishs[index].topicLen)) continue;
		if(	uC.timer_1ms >	(aPublishs[index].lastPublishActivity + aPublishs[index].PublishPeriod )  ){ // Timeout exceeded?
			aPublishs[index].state = MQTT_PUBLISH_Scheduled;
			mqtt_PublishStructALL_SetPublishScheduled(aPublishs,MaxPublishStructs ); 				// bei Bedarf auf MQTT_PUBLISH_Scheduled setzen
			break; // Array spaeter von Anfang an bearbeiten
		}
	}
	for (index = 0 ; index < MaxPublishStructs; index++){ // return first aPublish to be scheduled
		if(!(aPublishs[index].topicLen)) continue;
		if( (MQTT_PUBLISH_Scheduled == aPublishs[index].state) ){
			return &aPublishs[index];
		}
	}
	return 0; // Keine Aktivitaet erforderlich
}



// Return: Laenge der TxMsg MQTT_Msg_Length +2
UINT16 createPublishMsg(PublishNodeContext* pPubCon, UINT8 *TxBuf){
UINT16 len ;
UINT16 len_val;
float  value = 0.123456;
	if( MQTT_PUBLISH_Scheduled == pPubCon->state){
		if(!(pPubCon->topicLen)) return 0;
		TxBuf[0] = pPubCon->headerflags.U8 ;  							// 0: 	Header
		len = sprintf((char*)&TxBuf[4],"%s%s",uC.controller_name, pPubCon->topic_name);
		TxBuf[2] = (UINT8)(len >> 8);									// 2,3	Topic_Length
		TxBuf[3] = (UINT8)(len );
		len_val = sprintf((char*)&TxBuf[4 + len],"%f",value);
		TxBuf[1] = len + len_val; 										// 1: 	MQTT_Msg_Length
		pPubCon-> state =  MQTT_PUBLISH_SENT;
		pPubCon->lastPublishActivity = uC.timer_1ms;
	    return TxBuf[1] +2;												// Laenge der Nachricht ist MQTT_Msg_Length +2
	}
	return 0;
}


UINT8 mqtt_PublishStructInit(PublishNodeContext* aPublish, UINT16 msgId, char* name, UINT8 HeaderFlags, UINT32 publishPeriod){
	aPublish->state = MQTT_PUBLISH_FirstPublishRequired;
	aPublish->headerflags.U8 	= MQTTPUBLISH | HeaderFlags;
	aPublish->message_id 		= msgId;
	strcpy(aPublish->topic_name, name);
	aPublish->topicLen  		= strlen(name);
	aPublish->remainingLenght 	= 5;		// ToDo: ueberarbeiten
	aPublish->CleanSession 		= 0;
	aPublish->PublishPeriod = publishPeriod;
	return 0;
}

UINT8 isMqttPubRel(UINT8 *buffer, PublishBrokerContext* publ) {
	if((buffer[0]& 0xf0)!= MQTTPUBREL) return 0; 		//bit 4-7
	publ->message_id = (buffer[2] << 8) + buffer[3]; 	// message ID wird von Pubrel übertragen
	return 1;
}

UINT16 createMqttPuback(UINT8 *buffer, PublishBrokerContext* publ) {
	buffer[0] = MQTTPUBACK ;	       // Packet Type
	buffer[1] = 3;     				   // Remaining Length
	buffer[2] = publ->message_id  >> 8; 	// set MsgID
	buffer[3] = publ->message_id;
	return 4;
}

UINT16 createMqttPubrec(UINT8 *buffer, PublishBrokerContext* publ) {
	buffer[0] = MQTTPUBREC ;	       // Packet Type
	buffer[1] = 3;     				   // Remaining Length
	buffer[2] = publ->message_id  >> 8; 	// set MsgID
	buffer[3] = publ->message_id;
	return 4;
}

UINT16 createMqttPubComp(UINT8 *buffer, PublishBrokerContext* publ) {
    buffer[0] = MQTTPUBCOMP ;	       // Packet Type
	buffer[1] = 3;     				   // Remaining Length
	buffer[2] = publ->message_id  >> 8; 	// set MsgID
	buffer[3] = publ->message_id;
	return 4;
}

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

UINT8 Client_read(TCP_MQTT_CLIENT_CONTEXT* src){
	return 0;
}



//_______________________________Beginn-PubSubClient-Funktionen_____________________________________________
#if 0


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

// reads a byte into result
bool readByte(PubSubClient* src, uint8_t * result) {
   uint32_t previousMillis = millis();
   while(!Client_available(src->mqtt_connection) ) {  // solange eine einkommende Nachricht verfuegbar ist
     uint32_t currentMillis = millis();
     if(currentMillis - previousMillis >= ((int32_t) src->socketTimeout * 1000)){
       return false;
     }
   }
   *result = Client_read(src->mqtt_connection);
   return true;
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
    return len;
}

    //_______________________________Ende-PubSubClient-Funktionen__________________________________________________



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

#endif // USE_MQTT_NODE
