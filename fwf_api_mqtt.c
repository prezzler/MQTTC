/*
author HZDR FWF Kaever
MQTT

version 1.0
date    11.2020
*/

/*-----------------------------------------------------------------------------------------
------	Includes
-----------------------------------------------------------------------------------------*/
// ARM-Strukturen ==============================
#include <misc.h>
#include <stm32f4xx.h>
#include "stm32f4xx_gpio.h"         // BitAction
#include <stm32f4xx_tim.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_wwdg.h>
#include <stm32f4xx_adc.h>

#include <string.h>                 // memcpy

// FWF-Strukturen fuer Anwender ==============================

// Lokale Einstellungen zum Ablauf
//=========================================================================================
#define _FWF_APP_MQTT_MODULE_C 			1
#define USE_MEASURE_CMD_EXECUTION_TIMER			1

#include "fwf_defs.h"
#include "fwf_typ_defines.h"

#undef  DEBUG_LEVEL_FWF   			// Lokale Anpassung Debug Level fuer dieses Modul ===================================
#define DEBUG_LEVEL_FWF         2

#include "fwf_dbg.h"      			// dbg_toggle
#include "fwf_api_categories.h" 	// FWF_CAT_xxx CATEGORIES_FOR_COMMUNICATION

#include "fwf_test_struct_uc.h"    	// SETGET_TEST_STRUCT_INDEX ..

#include "fwf_lib.h"				// msg2uint_be
#include "fwf_flash.h"          	// Datenstrukturen + fwf_api_com

#include "fwf_dhcp.h"				// DHCP_STATE
#include "fwf_uc_status.h"          // uC
#include "fwf_hw_dio.h"             // lax,..

#include "HW_pinfkt.h"				// GPIO_LOW_
#include "HW_devices.h" 			// SPIx_HWTYPE_ADC_ADS8588S

#include "fwf_udp_interpreter.h"    // generate_err_msg()
// #include "fwf_tcp_server.h"         // tcp_server_vars
#include "mac_dma.h"                // rx_tcp_appdata
#include "fwf_string.h"             // check_string
#include "fwf_api_mqtt.h"			// 	MQTT_interlock_timer_reset ..
#include "fwf_api_uC_headers.h" 	// alle Applikationsheader

#include "uip.h"					// command_execution_time


//#if  USE_MQTT_SERVER
#include "mqttc.h"

// VORSICHT : Fuer Strukturgleichheit mit struct http_state im allgemeinen Teil sorgen!!!!
typedef struct tcp_MQTT_Server_hasMsg_vars {
  // allgemeiner Teil der Datenstruktur
  MQTT_state  state;			// Aktueller Zustand des Servers
  UINT16 countSendData;		// Gibt an, ob zu versendende Daten vorhanden sind (wenn ungleich 0)
  UINT8* pSendData;			// Zeiger auf die zu versendenden Daten
  // Applikationsspezifischer Teil der Serverstruktur
  UINT8  server_index;   		// Index zur Individualisierung der Server Kaever
} TCP_MQTT_SERVER_CONTEXT;


// Statische Variable
static TCP_MQTT_SERVER_CONTEXT	    *tcp_server_var; // Datenkontext des Servers
static TCP_MQTT_CLIENT_CONTEXT		*mqtt_client_var; // Datenkontext des Client
static UINT8 *rx_MQTT_msg;   		/* The uip_appdata pointer points to received application data. */
static UINT8 MQTT_response[1500]; 	// Buffer zur Generierung von Antworten

static MQTT_PUBLISH_IO_STATUS   io_status_struct;
static MQTT_FUNCTION_BLOCK 	   	MQTT_function_block;
static UINT16 					switch_event_msg_switch_event 	= 0;

#if 0
// Variablen fuer tcp Abwicklung
FWF_COM_CMD       fwf_com_tcp_cmd;
FWF_COM_RESPONSE  fwf_com_tcp_resp;
#endif


// Lokale Funktionen
static void   MQTT_Server_on_disconnect(void);
static UINT16 MQTT_Server_on_connect(void);

// Lokale Strukturen
static PubSubClient* pPubSubClient;
//static MqttConnect*  pMqttConnect;
static Subscription  aSubscriptions   [MaxSubscriptions_of_this_Node];
static int SubCount = 0;
static Unsubscription aUnsubscriptions[MaxSubscriptions_of_this_Node];
static int UnsubCount = 0;
static PublishContext 	 aPublish[MaxPublishStructs_of_this_Node];
static UINT8 PublishCount = 0;

static struct uip_TCP_conn* MQTT_Client_uip_TCP_conn; // Die Connenction-Struktur des MQTT_Client

UINT8 mqtt_PublishStructALLInit(void);
UINT8 mqtt_PublishStructInit(PublishContext* aPublish, UINT16 msgId, char* name, UINT8 headerFlags, UINT32 publishPeriod);

UINT8 mqtt_SubscribeStructALLInit(void);
UINT8 mqtt_SubscribeStructInit(Subscription* aSubscriptions, UINT16 msgId, char* name, UINT8 QoS);

void MQTT_init(void){
	MQTT_function_block.app_status.msg_version = FWF_APP_MQTT_INTERFACEVERSION;

	MQTT_function_block.msg_publish_event	= 0;

    io_status_struct.msg_version = FWF_APP_MQTT_INTERFACEVERSION;
    io_status_struct.led 			= 0x0;
    io_status_struct.voltage_ADC1 	= 0.0;
    io_status_struct.voltage_ADC2 	= 0.0;

	switch_event_msg_switch_event 	= 0;
}

void MQTT_Client_init(void){
Union32 ip;
	ip.U8[0] = 10;
	ip.U8[1] = 0;
	ip.U8[2] = 0;
	ip.U8[3] = 1;

    //TCP-Connection for MQTT_Client
    MQTT_Client_uip_TCP_conn 		= uip_tcp_connect(ip.U16, htons(MQTT_PORT), MQTT_Client);
	mqtt_client_var					= (TCP_MQTT_CLIENT_CONTEXT *) MQTT_Client_uip_TCP_conn->appstate;
	mqtt_client_var->state 			= MQTT_FSM_TCP_SYN_SENT;
	mqtt_client_var->pSendData		= MQTT_response;
	mqtt_client_var->countSendData  = 0;	// kein Sendebedarf
	mqtt_client_var->TCP_connection = MQTT_Client_uip_TCP_conn;
	// MQTT Client
    pPubSubClient 	= PubSubConstructor(mqtt_client_var);
    pPubSubClient->connected = 0;

    PublishCount = mqtt_PublishStructALLInit();
}

// PUBLISH ===============================================================================================

UINT8 mqtt_PublishStructALLInit(void){
UINT8 index = 0;
	mqtt_PublishStructInit(&aPublish[index++], 1, "/analog1", MQTTQOS0 | MQTT___RETAIN, 4500 );
	mqtt_PublishStructInit(&aPublish[index++], 1, "/AnaIn2", MQTTQOS0 | MQTT_NoRETAIN, 6000 );
	return index;
}

UINT8 mqtt_SubscribeStructALLInit(void){
UINT8 index = 0;
	mqtt_SubscribeStructInit(&aSubscriptions[index++], 1, "/analog1", 0 );
	mqtt_SubscribeStructInit(&aSubscriptions[index++], 1, "/AnaIn2", 0);
	return index;
}

// Alle relevanten PublishStruct's (topicLen) auf MQTT_PUBLISH_Scheduled setzen
void mqtt_PublishStructALL_SetPublishScheduled(void){
UINT8 index = 0;
	for (index = 0 ; index < MaxPublishStructs_of_this_Node; index++){
		if(  aPublish[index].topicLen &&
			( MQTT_PUBLISH_SENT == aPublish[index].state)
				){
			aPublish[index].state = MQTT_PUBLISH_Scheduled;
		}
	}
}

// gibt es eine erforderliche Aktivitaet bei einer Publish-Struktur?
// array aPublish[] durchsuchen
// Bei timeout alle aPublish durch mqtt_PublishStructALL_SetPublishScheduled()
PublishContext* mqtt_find_PublishStruct_which_are_Scheduled (void){
UINT8 index = 0;
	for (index = 0 ; index < MaxPublishStructs_of_this_Node; index++){
		if(!(aPublish[index].topicLen)) break;
		if( (MQTT_PUBLISH_FirstPublishRequired == aPublish[index].state) ){			// first publish required?
			aPublish[index].state = MQTT_PUBLISH_Scheduled;
			return &aPublish[index];
		}
		if(	uC.timer_1ms >	(aPublish[index].lastPublishActivity + aPublish[index].PublishPeriod )  ){ // Timeout exceeded?
			aPublish[index].state = MQTT_PUBLISH_Scheduled;
			mqtt_PublishStructALL_SetPublishScheduled(); 		// alle auf MQTT_PUBLISH_Scheduled setzen
			break; // Array von Anfang an bearbeiten
		}
	}
	for (index = 0 ; index < MaxPublishStructs_of_this_Node; index++){
		if(!(aPublish[index].topicLen)) break;
		if( (MQTT_PUBLISH_Scheduled == aPublish[index].state) ){			// first publish required?
			aPublish[index].state = MQTT_PUBLISH_Scheduled;
			return &aPublish[index];
		}
	}
	return 0; // Keine Aktivitaet erforderlich
}

Subscriptions* mqtt_find_SubscriptionStruct_which_are_Scheduled(void){
	UINT8 index = 0;
	
	for (index = 0 ; index < MaxSubscriptions_of_this_Node; index++){
		if(!(aSubscriptions[index].topicLen)) break;
		if( (MQTT_SUBSCRIBE_FirstSubscribeRequired == aSubscriptions[index].state) ){			// first publish required?
			aSubscriptions[index].state = MQTT_SUBSCRIBE_Scheduled;
			return &aSubscriptions[index];
		}
	}
		for (index = 0 ; index < MaxSubscriptions_of_this_Node; index++){
		if(!(aSubscriptions[index].topicLen)) break;
		if( (MQTT_SUBSCRIBE_Scheduled == aSubscriptions[index].state) ){			// first publish required?
			aSubscriptions[index].state = MQTT_SUBSCRIBE_Scheduled;
			return &aSubscriptions[index];
		}
	}
	return 0; // Keine Aktivitaet erforderlich
}


// Return: Laenge der TxMsg MQTT_Msg_Length +2
UINT16 createPublishMsg(PublishContext* pPubCon, UINT8 *TxBuf){
UINT16 len ;
UINT16 len_val;
float  value = 0.123456;
	if( MQTT_PUBLISH_Scheduled == pPubCon->state){
		if(!(pPubCon->topicLen)) return 0;
		TxBuf[0] = pPubCon->headerflags.U8 ;  							// 0: 	Header
		len = sprintf((char*)&TxBuf[4],"%s%s",uC.controller_name, pPubCon->topic_name);
		TxBuf[2] = (UINT8)(len >> 8);									// 2:3	Topic_Length
		TxBuf[3] = (UINT8)(len );
		len_val = sprintf((char*)&TxBuf[4 + len],"%f",value);
		TxBuf[1] = len + len_val; 										// 1: 	MQTT_Msg_Length
		pPubCon-> state =  MQTT_PUBLISH_SENT;
		pPubCon->lastPublishActivity = uC.timer_1ms;
	    return TxBuf[1] +2;												// Laenge der Nachricht ist MQTT_Msg_Length +2
	}
	return 0;
}

UINT16 createSubscribeReqMsg(Subscriptions* pSubscribe, UINT8 *TxBuff) {
    UINT16 len;
    if (pSubscribe->state == MQTT_SUBSCRIBE_Scheduled) {
        if (!(pSubscribe->topicLen)) return 0;
        TxBuf[0] = pSubscribe->headerflags.U8; // Header
        len = sprintf((char*)&TxBuf[4], "%s", pSubscribe->topic_name);
        TxBuf[2] = (UINT8)(len >> 8);  // Topic_Length
        TxBuf[3] = (UINT8)(len);
        TxBuf[1] = 2 + len + 1;  // MQTT_Msg_Length = 2 Bytes Topic_Length + len for topics + Qos
        TxBuf[4 + len] = pSubscribe->QoS; // QoS level
        pSubscribe->state = SUBSCRIBE_SENT;
        return TxBuf[1] + 2;  // Length of the message is MQTT_Msg_Length + 2
    }
    return 0;
}

}



UINT8 mqtt_PublishStructInit(PublishContext* aPublish, UINT16 msgId, char* name, UINT8 HeaderFlags, UINT32 publishPeriod){
	aPublish->state = MQTT_PUBLISH_FirstPublishRequired;
	aPublish->headerflags.U8 = MQTTPUBLISH | HeaderFlags;
	aPublish->message_id = msgId;
	strcpy(aPublish->topic_name, name);
	aPublish->topicLen  = strlen(name);
	aPublish->remainingLenght = 5;
	aPublish->CleanSession = 0;
	aPublish->PublishPeriod = publishPeriod;
	return 0;
}

UINT8 mqtt_SubscribeStructInit(Subscription* aSubscriptions, UINT16 msgId, char* name, UINT8 QoS){
	aSubscriptions->state = MQTT_SUBSCRIBE_FirstSubscribeRequired;
	aSubscriptions->headerflags.U8 = MQTTSUBSCRIBE | 2; // 2 für reserved flags
	aSubscriptions->message_id = msgId;
	strcpy(aSubscriptions->topic_name, name);
	aSubscriptions->topicLen  = strlen(name);
	aSubscriptions->remainingLenght = 5;
	aSubscriptions->QoS = Qos;
	return 0;
}


//===============================================================================================


// Aufruf bei Verbindungsaufbau des Servers mit einem externen Client
UINT16  MQTT_Client_create_connect(UINT8*  buffer){
UINT16 var;
UINT8  index = 2; // Laenge der zu sendenden Nachricht
		buffer[0] = MQTTCONNECT;
		var = 4; 						// Protocol name Length
		buffer[index++] = (UINT8)(var >> 8);
		buffer[index++] = (UINT8)(var );
		buffer[index++] = 'M';			// Protocol name
		buffer[index++] = 'Q';
		buffer[index++] = 'T';
		buffer[index++] = 'T';
		buffer[index++] =  MQTT_VERSION_3_1_1	;		// MQTT Version 4 = v3.1.1
		buffer[index++] =  2	;		// connect flags QoS
		var = 60; 						// Keep Alive
		buffer[index++] = (UINT8)(var >> 8);
		buffer[index++] = (UINT8)(var );

		var = sprintf((char*)&buffer[index+2],uC.controller_name);
		buffer[index  ] = (UINT8)(var >> 8);
		buffer[index+1] = (UINT8)(var );
		index += var+2;
		//
		buffer[1] = index -2; 			// MQTT-MsgLen = Laenge der zu sendenden Nachricht -2
		return index;
}


// Aufruf bei Verbindungsaufbau des Servers mit einem externen Client
UINT16  MQTT_Server_on_connect(void){
    tcp_server_var 					= (TCP_MQTT_SERVER_CONTEXT *)(uip_tcp_conn->appstate);
	tcp_server_var->state 			= MQTT_FSM_AWAIT_INIT;
    tcp_server_var->pSendData 		= MQTT_response;
	tcp_server_var->countSendData 	= 0; // kein Sendebedarf

#if 0 // Server = Broker => nicht implementiert
    // MQTT-Library initialisieren
    pPubSubClient 	= PubSubConstructor(uip_tcp_conn);
    pMqttConnect  	= MqttConnectKonstruktor(pPubSubClient);
#endif
    return 0;	// kein Sendebedarf
}


// UINT8 createMqttDisconnect(UINT8* buffer) {
//     buffer[0] = MQTTDISCONNECT;
//     buffer[1] = 0;

//     return 2;
// }

void MQTT_Server_on_disconnect(void){
    // createMqttDisconnect(pPubSubClient);
    tcp_server_var 				= (TCP_MQTT_SERVER_CONTEXT *)(uip_tcp_conn->appstate);
	tcp_server_var->state 		= MQTT_FSM_END; 	//welcher State?
    tcp_server_var->pSendData 	= (UINT8*)disconnect(pPubSubClient);
    tcp_server_var->countSendData = 2; // Laenge eines Disconnect-Pakets
}


// Anlegen von Subscription-Requests in aSubscriptions[]
UINT8 isMqttSubscribeRequest(UINT8 *buffer, UINT16 msg_length){
UINT8 	mqtt_msg_len;
Subscription* subscript ;
UINT8   buf_index 			= 0;
UINT8 	subscription_index 	= 0;

	while(buf_index < msg_length) {
		if( (buffer[buf_index] & 0xf0) != MQTTSUBSCRIBE)  return 0; 	// Client Subscribe request + Reserved
		mqtt_msg_len = buffer[buf_index + 1];  					// Subscr. Message Length
		subscript = &aSubscriptions[subscription_index];		// Take a subscription
		subscript->message_id   = (buffer[buf_index+2] << 8) + buffer[buf_index+3]; 				// Message ID
		subscript->topic_length = (buffer[buf_index+4] << 8) + buffer[buf_index+5];
		strncpy( subscript->topic_name, (char*) &buffer[buf_index+6],  subscript->topic_length);  	// topic_name
		subscript->Qos 		= buffer[buf_index+ 6 + subscript->topic_length]; 						// Quality of service
		subscript->acked 	= 0;

		buf_index += 2 + mqtt_msg_len;
		subscription_index ++;
	}
	return 1; // case MQTTSUBSCRIBE
}

// Return #bytes to send
UINT16 createMqttSuback(UINT8 *buffer, UINT8 subscription_index) {	// Subscribe ack anlegen
#if 0
	UINT8   buf_index ;
	// Take the first subscription
	buffer[0] 	= MQTTSUBACK ;//+ 3;     	// Packet Type
	buffer[2]   = aSubscriptions[subscription_index].message_id  >> 8; 	// set MsgID
	buffer[3]   = aSubscriptions[subscription_index].message_id;
    buffer[4] 	= aSubscriptions[subscription_index].Qos; 					// Maximaler QoS, der etabliert wird, derzeit alles auf QoS 0
	buf_index 	= 4;
	subscription_index ++;
	while(subscription_index < MaxSubscriptions_of_this_Node) {
		if( !aSubscriptions[subscription_index].message_id ) break;			// Naechste aSubscription valide?
	    buffer[buf_index] = aSubscriptions[subscription_index].Qos;			// Maximaler QoS, der etabliert wird, derzeit alles auf QoS 0
	    aSubscriptions[subscription_index].acked = 1;
	    buf_index ++ ;
	    subscription_index ++;
	}
	buffer[1] 	= buf_index - 1;			    // Remaining Length
    return buf_index +1;
#else // 1 Suback per Message
    aSubscriptions[subscription_index].acked = 1;
    buffer[0] = MQTTSUBACK ;	       // Packet Type
    buffer[1] = 3;     				   // Remaining Length
    buffer[2] = aSubscriptions[subscription_index].message_id  >> 8; // set MsgID
    buffer[3] = aSubscriptions[subscription_index].message_id;
    buffer[4] = aSubscriptions[subscription_index].Qos;				// Maximaler QoS, der etabliert wird, derzeit alles auf QoS 0
    return 5;
#endif
}

UINT8 isMqttUnsubscribeRequest(UINT8* buffer, UINT16 msg_length) {   //  1 Byte Control Packet & Flags, 2 Bytes Rem Len, X Bytes Topic
	//ka
	UINT8 			mqtt_msg_len;
	Unsubscription* unSubscript ;
	UINT8   buf_index 				= 0;
	UINT8 	unSubscription_index 	= 0;

		while(buf_index < msg_length) {
			if( (buffer[buf_index] & 0xf0) != MQTTUNSUBSCRIBE)  return 0; 	// Client Unsubscribe request + Reserved
			mqtt_msg_len = buffer[buf_index + 1];  							// UnSubscr. Message Length
			unSubscript = &aUnsubscriptions[unSubscription_index];			// Take an unSubscription_index
			unSubscript->message_id   = (buffer[buf_index+2] << 8) + buffer[buf_index+3]; 					// Message ID
			unSubscript->topic_length = (buffer[buf_index+4] << 8) + buffer[buf_index+5];
			strncpy( unSubscript->topic_name, (char*) &buffer[buf_index+6],  unSubscript->topic_length);  	// topic_name
			unSubscript->acked 	= 0;

			buf_index += 2 + mqtt_msg_len;
			unSubscription_index ++;
		}
		return 1; // case MQTTSUBSCRIBE
	//ka

	return 1;
}

UINT8 UnSubscription_MsgID_valid(Unsubscription* unsubscript){
UINT8 index;

	for (index = 0; index < MaxSubscriptions_of_this_Node; index++){
			if( aSubscriptions[index].message_id == unsubscript->message_id ) return 1; // found
	}
	return 0;
}

UINT8 createMqttUnsuback(UINT8* buffer, Unsubscription* unsubscript, bool valid){
	unsubscript->acked = 1;
    buffer[0] = MQTTUNSUBACK ;	       // Packet Type
    buffer[1] = 3;     				   // Remaining Length
    buffer[2] = unsubscript->message_id  >> 8; 	// set MsgID
    buffer[3] = unsubscript->message_id;
    if(UnSubscription_MsgID_valid(unsubscript)) // Return Code (Reason String)
        buffer[4] = 0;
    else 
        buffer[4] = 17; // no topic matched
    return 5;
}

// Ueberprueft ob es ein Subscription auf ein bestimmtes Topic gibt. Ja: geben den Index der Subscription zurueck; Nein: gibt Fehler zurueck -1;
int CheckTopicUn(Unsubscription* unsubscript){
int i,j;
    for(i = 0; i < MaxSubscriptions_of_this_Node; i++){
    	// Eine Subscription absuchen
    	for(j = 0; j < aSubscriptions[i].topic_length ; j++){
    		if( aSubscriptions[i].topic_name[j] != unsubscript->topic_name[j]){
    			break; // naechste subscription
    		}
    		if (j == unsubscript->topic_length) return -1;
    	}
    }
    perror("Unsubscribe Topic didn't match a Subscription");
    return -1;
} 

// setzt alle Werte der zu loeschenden Subscription auf null
void deleteMqttSubscription(Subscription* subscript){
    memset(subscript->topic_name, 0, sizeof(subscript->topic_name));
    subscript->topic_length = 0;
    subscript->message_id = 0;
    subscript->Qos = 0;
    SubCount--;
}

UINT8 isMqttPublishRequest(UINT8 *buffer, PublishContext* publ){
    UINT16 len, remaining_length;

    if((buffer[0]& 0xf0)!= MQTTPUBLISH) return 0; //bit 4-7
        //Flags befinden sich im ersten byte. bit 0-3

        // Bit:   | 7 | 6 | 5 | 4 | 3 | 2 : 1 | 0    |
        //        +---+---+---+---+---+---+---+------+
        // Value: |MQTTPACKET TYPE|DUP| QoS   |Retain|
        //        +---+---+---+---+---+---+---+------+
   
    int index = 0; //index of buffer

    publ->headerflags.BA.DUP    = (buffer[0] & 0x08) >> 3; 		// Bit 3 , DUP bei QoS 0 immer 0
    publ->headerflags.BA.QoS    = (buffer[0] & 0x06) >> 1; 		// Bit 2 und 1
    publ->headerflags.BA.Retain = (buffer[0] & 0x01);    		// Bit 0 , wenn true (1) dann speichert der Server die Nachricht um auch an alle zukünfitgen Subs. des Topics zu schicken
    remaining_length = buffer[1]; 				// index 1
    publ->remainingLenght = remaining_length;

    publ->topicLen = len = ( buffer[2] << 8 ) + buffer[3]; 		//topic length // index 2,3

    if(len > TOPIC_LENGTH){
    	FWF_DBG1_PRINTFv("Length > TOPIC_LENGHT");
        return 0;
    }
    memset(publ->topic_name, 0, len); 		//publ->topic_name[0]= '\0';
    strncpy(publ->topic_name, (char*)&buffer[4], publ->topicLen);	// index 4
    publ->topic_name[len] = '\0'; // TODO: notwendig?
    index = 4 + len;

    if(publ->headerflags.BA.QoS > 0){
        publ->message_id = (buffer[index] << 8) + buffer[index+1]; 	// Ist nur bei QoS > 0 enthalten
    }
    index += 2;
#if 0 // publ->payload existiert nicht
    int payloadLen = remainingLenght - (index - 1);   // remLen - länge seit remLen (remLen fängt bei 1 an)
    publ->payloadLen = payloadLen;

    strncpy(publ->payload ,buffer[index++], payloadLen);
#endif
    return 1;
}

int CheckTopicPub(PublishContext* publ){
    for(int i = 0; i < MaxPublishStructs_of_this_Node; i++){
        if( aPublish[i].topic_name == publ->topic_name){ // ToDo strcmp()
            return i;
        }
    }
    FWF_DBG1_PRINTFv("Publish Topic didn't match a Publish Context");
    return -1;
}

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
UINT8 isMqttPingResp(UINT8 *buffer){
    if((buffer[0]& 0xf0) != MQTTPINGRESP) return 0;
    pPubSubClient->pingOutstanding = false;
    return 1;
}

//======================================================================================================
//======================================================================================================
#define MQTT_CLIENT_PING_INTERVAL	10000 // Intervall zwischen Ping-Aufrufen des Client [ms]


// Rolle eines MQTT Node
// Call fuer TCP_connection SYN erfolgt ausserhalb von MQTT_Client
void MQTT_Client(u8_t uip_flags){
    if(uip_tcp_conn->lport == 0) uip_abort();	// Kein Serverport

	switch (	mqtt_client_var->state){
		case  MQTT_FSM_TCP_SYN_SENT:
			if( (UIP_CONNECTED | UIP_NEWDATA) == uip_flags ) { //  1. event TCP Server sends SYN ACK -> Reaction: send MQTT_connect request
				rx_MQTT_msg  = (UINT8 *) &uip_rxbuf[TCP_HEADER_SIZE + UIP_LLH_LEN]; 	/* The uip_appdata pointer points to received application data. */
				mqtt_client_var->countSendData = MQTT_Client_create_connect(mqtt_client_var->pSendData);
				mqtt_client_var->state = MQTT_FSM_AWAIT_MQTTConAck;
				break;
			}
			if( (UIP_ACKDATA) & uip_flags ) { //  1. event TCP Server sends SYN ACK -> Reaction: send MQTT_connect request
				mqtt_client_var->countSendData = MQTT_Client_create_connect(mqtt_client_var->pSendData);
				mqtt_client_var->state = MQTT_FSM_AWAIT_MQTTConAck;
				break;
			}
			if( (UIP_ABORT) == uip_flags ) return ;	 // Reaktion auf Ebene des Uip_Stack
			break;

		case  MQTT_FSM_AWAIT_MQTTConAck:	// Wait for MQTT Connect Acknowledge
			if ( (UIP_NEWDATA) & uip_flags ){
				rx_MQTT_msg  = (UINT8 *) &uip_rxbuf[TCP_HEADER_SIZE + UIP_LLH_LEN]; 	/* The uip_appdata pointer points to received application data. */
				if( isMqttConnectACK(rx_MQTT_msg) ){
					mqtt_client_var->state = MQTT_FSM_Connected_WORKING;
					mqtt_client_var->mqtt_ping_time = uC.timer_1ms;
					return ;
				}
				if(!uip_len){
					return; // Wait longer
				}
			}
			break;

		case  MQTT_FSM_Connected_WORKING: // MQTT WORKING state
		    if( uip_poll() ){
		    	mqtt_client_var->countSendData = MQTT_Client_MsgOutstanding(mqtt_client_var->pSendData);
		    }
		    if( (UIP_NEWDATA) & uip_flags ){ // Response einarbeiten
		    	mqtt_client_var->countSendData = MQTT_Client_Response_Processing(&uip_rxbuf[TCP_HEADER_SIZE + UIP_LLH_LEN], uip_len);
		    }
			break;

		case MQTT_FSM_NONE:
			break;

		default:
			return ;

	}
	// Ggf. Senden vorbereiten
	uip_slen = tcp_app_transfer_to_tx_tcp_buffer((u8_t*)mqtt_client_var->pSendData, mqtt_client_var->countSendData);
	mqtt_client_var->countSendData = 0;
}



// TCP Interpreter fuer MQTT - Rolle des BROKERs ===========================================================
void fwf_MQTT_server(u8_t flag) {
int i;
char tcp_cmd  [TCP_CMD_SIZE]  = "";
char tcp_par1 [TCP_PAR1_SIZE] = "";
char tcp_par2 [TCP_PAR2_SIZE] = "";
uint8_t tcp_cmd_pos = 0;
uint8_t tcp_par1_pos = 0;
uint8_t tcp_par2_pos = 0;
UINT8 channel = 0;          // from received message extracted channel (PWM-,ADC-, LED-Number)

    if(uip_tcp_conn->lport == 0) uip_abort();	// Kein Serverport
    
    if( uip_poll() ){  // Timergesteuerter Aufruf UIP_POLL 
    	tcp_server_var->countSendData = MQTT_Server_MsgOutstanding(tcp_server_var->pSendData);
        uip_slen = tcp_app_transfer_to_tx_tcp_buffer((u8_t*)tcp_server_var->pSendData, tcp_server_var->countSendData);
        tcp_server_var->countSendData = 0;
    }

    if(uip_connected()) {  //  1. event TCP connected=ESTABLISHED -> Reaction: reset the MQTT_state for this connection.
    	tcp_server_var->countSendData = MQTT_Server_on_connect(); // Kontext setzen ( z.B. tcp_server_var->state )
        tcp_server_var->state = MQTT_FSM_AWAIT_INIT;
        FWF_DBG2_PRINTFv("TCP:->FSM_AWAIT_INIT");
        return;
    }
    // Behandeln der TCP-Transfers Timeout - ausserhalb der Server-States
    if (uip_timedout()) { // UIP_TIMEDOUT
        FWF_DBG2_PRINTFv("\nTCP-SRV TIMEDout");
        uip_close();
        return;
     }
    if (uip_closed()){  // event close
        FWF_DBG2_PRINTFv("TCP:->CLOSE\n");
        MQTT_Server_on_disconnect();
        tcp_server_var->state = MQTT_FSM_END;
        uip_close();
        return;
    }
    if (uip_aborted() ) { // event UIP_ABORT [PC:RST]  [PC:FIN]
        FWF_DBG2_PRINTFv("TCP:->ABORT\n");
        uip_abort();      // Reaktion: abort der Verbindung
        return;
    }
    if (uip_poll()) return;  // Nur Polling Mode: uip_flags & UIP_POLL
    rx_MQTT_msg  = (UINT8 *) &uip_rxbuf[TCP_HEADER_SIZE + UIP_LLH_LEN]; 	/* The uip_appdata pointer points to received application data. */

#if DEBUG
    if (!uip_poll()) FWF_DBG3_PRINTFv("\tTCP-FSM 0x%x",tcp_server_var->state);
#endif // DEBUG

    switch( tcp_server_var->state ){
        case MQTT_FSM_AWAIT_INIT:  // 2. TCP Connect des Client eingetroffen + INIT
            if (!uip_newdata()) return;
            // Is it a MQTT connect =======================================================================
            if(isMqttConnect(rx_MQTT_msg)){
            	tcp_server_var->countSendData = createMqttConnectACK( (UINT8*) tcp_server_var->pSendData);
				pPubSubClient->_state = MQTT_CONNECTED;
				pPubSubClient->connected = 1;
				tcp_server_var->state = MQTT_FSM_WORK;
            }
            break;

        case MQTT_FSM_INIT_NOACK:  	// 3. Vorstufe zu INIT (ohne Ack des client)
            if(uip_acked()) { 		// uip_flags & UIP_ACKDATA
                    FWF_DBG1_PRINTFv("FSM_INIT_NOACK:ack->FSM_WORK");
                    tcp_server_var->state = MQTT_FSM_WORK;
            } else {
                    FWF_DBG1_PRINTFv("ERR-TCP-state-%x",tcp_server_var->state);
            }
            break;

        case MQTT_FSM_WORK: 	// 4. nach MQTT_FSM_AWAIT_INIT mit TCP-Ack des Client
            if ( !uip_newdata()) return;
            FWF_DBG2_PRINTFv("FSM@WORK Process requests");
            tcp_server_var->countSendData = MQTT_ServerRequest_to_Response(rx_MQTT_msg, uip_len);
            break;


            tcp_cmd_pos = 0;
            rx_MQTT_msg[uip_len] = 0;       // terminate string for scanning
            MQTT_response[0] = 0;     		// erase datapointer for the data that will be sent back

#if USE_MQTT_SERVER
#if FWF_APP_STEPPER_STMdrv
            STEPPER_STM_MQTT_Interpreter(rx_MQTT_msg);
#endif // FWF_APP_STEPPER_STMdrv

#endif // USE_MQTT_SERVER

            // extract tcp_cmd, tcp_par1, tcp_par2 from received data
            // nicht effizient, fuer die Anwendung aber unkritisch und einfacher zu debuggen
            if ( (i = SEARCH_STRING((char*)rx_MQTT_msg, "." ,uip_len)) <= uip_len){       // find first dot
                tcp_par1_pos = tcp_cmd_pos+i+1;
                strncpy(tcp_cmd, (char*)&rx_MQTT_msg[tcp_cmd_pos], i);
                if ( (i = SEARCH_STRING((char*)&rx_MQTT_msg[i+1], "." ,uip_len)) <= uip_len){
                    tcp_par2_pos = tcp_par1_pos+i+1;
                    strncpy(tcp_par1, (char*)&rx_MQTT_msg[tcp_par1_pos], i);
                    strncpy(tcp_par2, (char*)&rx_MQTT_msg[tcp_par2_pos], uip_len-i);
                } else {    // no second dot found
                    strncpy(tcp_par1, (char*)&rx_MQTT_msg[tcp_par1_pos], uip_len-tcp_par1_pos);
                    tcp_par1[TCP_PAR1_SIZE-1] = 0;
                }
            } else { // kein dot enthalten
                strcpy(tcp_cmd, (char*)&rx_MQTT_msg[tcp_cmd_pos]);
            }

            channel = fwf_atoi((UINT8*)tcp_par1,strlen(tcp_par1),10);
            fwf_atoi((UINT8*)tcp_par2,strlen(tcp_par2),10);
            FWF_DBG2_PRINTFv("FSM: interpreter received: >%s >%s >%s", tcp_cmd, tcp_par1, tcp_par2);


            //** cases to response to:
            // LED
            if(!SEARCH_STRING(tcp_cmd, "LED",TCP_CMD_SIZE)){   // if string found on pos 0
                if(!SEARCH_STRING(tcp_par2, "ON", TCP_PAR2_SIZE)){
                    sprintf((char*)MQTT_response,"LED.%d.ON.OK\n", channel);
                }
                if(!SEARCH_STRING(tcp_par2, "OFF", TCP_PAR2_SIZE)){
                    sprintf((char*)MQTT_response,"LED.%d.OFF.OK\n", channel);
                }
            }

            // PWM
            if(!SEARCH_STRING(tcp_cmd, "SETPWM", TCP_CMD_SIZE)){
            }

            // ADC
            if(!SEARCH_STRING(tcp_cmd, "GETADC", TCP_CMD_SIZE)){
                sprintf((char*)MQTT_response,"ADC.%d.%d\n", channel, 42);
            }

            //** fill TCP output buffer
            tcp_server_var->pSendData = MQTT_response;
            for(i = 0; tcp_server_var->pSendData[i] != '\n'; ++i);    /* Calculate length of string. */
            tcp_server_var->countSendData = i;
            FWF_DBG2_PRINTFv(" MSG %s %d",MQTT_response,i);

            break;

        case MQTT_FSM_TCP_SYN_SENT:
            FWF_DBG2_PRINTFv("FSM:ERR-TCP-FSM Noinit");
            break;

        default:
            FWF_DBG2_PRINTFv("FSM:Err-TCP-FSM");
            break;
    }

    // Send Data - dies ist aus verschiedenen Zustaenden heraus moeglich
    if(        (tcp_server_var->state & (MQTT_FSM_AWAIT_INIT | MQTT_FSM_WORK ) )
           &&   tcp_server_var->countSendData
      ) {  /* Send data, but not more than the MSS of the connection. uip_sappdata = (hs->pSendData); uip_slen = (hs->count) */
            uip_slen = tcp_app_transfer_to_tx_tcp_buffer((u8_t*)tcp_server_var->pSendData, tcp_server_var->countSendData);
            tcp_server_var->countSendData = 0;
    }
	if(!uC.uC_IP[0]){
		FWF_DBG1_PRINTFv("ERROR uC_IP");
	}
}

// Search aSubscriptions that has not been acknowledged
UINT8 NextOpenSuback(void){
	UINT8 index = 0;
	for (index = 0; index < MaxSubscriptions_of_this_Node; index++){
		if( (aSubscriptions[index].message_id) && (!aSubscriptions[index].acked) ) return index;
	}
	return MaxSubscriptions_of_this_Node; // not found
}


// Erzeugen von spontanen Nachrichten des MQTT Servers
// pMsg: Buffer fuer ausgehende Nachricht
// Return : Laenge der Nachricht
UINT16 MQTT_Server_MsgOutstanding(UINT8 * pMsg){
UINT8 index = 0;

	index = NextOpenSuback();
	if( MaxSubscriptions_of_this_Node == index ){ // kein offener Suback
		return 0;
	} else {
		return createMqttSuback((UINT8*)pMsg, index);
	}
}

char* isMqttSubscribeAck(UINT* RxBuffer){
	char topic[20];
	return topic;
	//TODO: schreiben
}

bool match_topics(char* topic, char* sub_topic) {
    int size_topic = strlen(topic); 
    int size_sub_topic = strlen(sub_topic);

    if (size_topic != size_sub_topic)
        return false;

    for (int i = 0; i < size_sub_topic; i++) {
        if (topic[i] != sub_topic[i]) 
            return false;
    }

    return true;
}


Subscriptions* search_Subscription_topic_match(char* topic){
	UINT8 index = 0;
	for (index = 0; index < MaxSubscriptions_of_this_Node; index++){
		if(match_topics(topic, aSubscriptions[index].topic))
			return aSubscriptions[index];
	}
	return NULL;
}

// Bearbeiten von Requests an MQTT Client und generieren einer Antwort in mqtt_client_var->pSendData
// RxBuffer: Buffer der eingehenden MQTT-Response
// Return: Laenge der TxMsg
UINT16 MQTT_Client_Response_Processing(UINT8 *RxBuffer, UINT16 msg_Rx_len){
	if( isMqttPingResp(RxBuffer) ){
		pPubSubClient->pingOutstanding = false;
	}
	if( char* topic = isMqttSubscribeAck(RxBuffer)){
		Subscriptions* pSubscriptionStruct;
		if(search_Subscription_topic_match(topic) != NULL)
		pSubscriptionStruct = search_Subscription_topic_match(topic);
	}
	return 0;
}
// Bearbeiten von Messages, die der MQTT Client absetzen soll - und generieren einer Antwort in TxBuffer
// Return: Laenge der TxMsg
UINT16 MQTT_Client_MsgOutstanding(UINT8 * TxBuffer){
PublishContext* pPublishStruct;
Subscriptions* pSubscriptionStruct;
UINT16 bytes_to_send = 0;

	if( (uC.timer_1ms - mqtt_client_var->mqtt_ping_time ) > MQTT_CLIENT_PING_INTERVAL){ // Ping erzeugen
		if(true == pPubSubClient->pingOutstanding){ // Server antwortete nicht
			uip_abort();
		}
		mqtt_client_var->mqtt_ping_time = uC.timer_1ms ;
		pPubSubClient->pingOutstanding = true;
		return createMqttPingReq(TxBuffer);
	}

	if ( (pSubscriptionStruct = mqtt_find_SubscriptionStruct_which_are_Scheduled() )){
		do {
			bytes_to_send += createSubscribeReqMsg( pPublishStruct , &TxBuffer[bytes_to_send]);	
		}
		while ( (pSubscriptionStruct = mqtt_find_SubscriptionStruct_which_are_Scheduled()))
		return bytes_to_send;
	}

	if ( (pPublishStruct = mqtt_find_PublishStruct_which_are_Scheduled() ) ){
		do {
			bytes_to_send += createPublishMsg( pPublishStruct , &TxBuffer[bytes_to_send]);	
		}
		while ( (pPublishStruct = mqtt_find_PublishStruct_which_are_Scheduled ()) ){ // Processing einer PublishStruct
		return bytes_to_send;
	}


	// while ( (pPublishStruct = mqtt_find_PublishStruct_which_are_Scheduled ()) ){ // Processing einer PublishStruct
	// 	bytes_to_send += createPublishMsg( pPublishStruct , &TxBuffer[bytes_to_send]);
	// }
// return bytes_to_send;
	
	
}

// Bearbeiten von Requests an MQTT Server und generieren einer Antwort in tcp_server_var->pSendData
// Return: Laenge der TxMsg
UINT16 MQTT_ServerRequest_to_Response(UINT8 *rx_MQTT_msg, UINT16 msg_len){
UINT8 ret_index;
	if( isMqttPingRequest(rx_MQTT_msg)){
		return createMqttPingResp((UINT8*)tcp_server_var->pSendData);
	}
    if( isMqttSubscribeRequest(rx_MQTT_msg, msg_len)){ 							//  Subscription ? for Broker Role
    	return createMqttSuback((UINT8*)tcp_server_var->pSendData, NextOpenSuback() );
    }
    if(isMqttUnsubscribeRequest(rx_MQTT_msg, msg_len)){ 			// UnSubscription ? for Broker Role
    	ret_index = CheckTopicUn(&aUnsubscriptions[UnsubCount]); 	// valide Unsubscription?
        if ( ret_index >= 0 || ret_index < 10){
            // send / create Unsuback
            deleteMqttSubscription(&aSubscriptions[ret_index]);
            UnsubCount++;
        	return createMqttUnsuback((UINT8*)tcp_server_var->pSendData, &aUnsubscriptions[UnsubCount], true);
        } else {
            // MqttUnsubAck mit Fehler RC;
        	return createMqttUnsuback((UINT8*)tcp_server_var->pSendData, &aUnsubscriptions[UnsubCount], false);
        }
    }
    if( isMqttPublishRequest(rx_MQTT_msg, aPublish)){
    	ret_index = CheckTopicPub(aPublish);
        if ( (ret_index >= 0) && (ret_index < MaxSubscriptions_of_this_Node) ){
            // TODO:
            // send publish to the Client that is subscribed
            // send to Client with aSubscriptions[ret]
        }
    }
    return 0;
}



// TEST SUPPORT
#define FWF_DBG_CYCLE_PRINTFv	FWF_DBG1_PRINTFv



#endif // USE_MQTT_SERVER
