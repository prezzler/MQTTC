////./*
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
typedef struct tcp_MQTT_server_vars {
  // allgemeiner Teil der Datenstruktur
  UINT8  state;					// Aktueller Zustand des Servers
  UINT16 countSendDdata;		// Gibt an, ob zu versendende Daten vorhanden sind (wenn ungleich 0)
  char  *pSendDdata;			// Zeiger auf die zu versendenden Daten
  // Applikationsspezifischer Teil der Serverstruktur
  UINT8  server_index;   		// Index zur Individualisierung der Server Kaever
} TCP_MQTT_SERVER_CONTEXT;


// Statische Variable
static TCP_MQTT_SERVER_CONTEXT	    *tcp_server_var; // Datenkontext des Servers
static UINT8 *rx_MQTT_msg;   		/* The uip_appdata pointer points to received application data. */
static char MQTT_response[1500]; 	// Buffer zur Generierung von Antworten

static MQTT_PUBLISH_IO_STATUS   io_status_struct;
static MQTT_FUNCTION_BLOCK 	   	MQTT_function_block;
static UINT16 					switch_event_msg_switch_event 	= 0;

#if 0
// Variablen fuer tcp Abwicklung
FWF_COM_CMD       fwf_com_tcp_cmd;
FWF_COM_RESPONSE  fwf_com_tcp_resp;
#endif


// Lokale Funktionen
static void MQTT_on_disconnect(void);
static UINT16 MQTT_on_connect(void);

// Lokale Strukturen
static PubSubClient* pPubSubClient;
static MqttConnect*  pMqttConnect;
static Subscription* aSubscriptions[10];
static int SubCount = 0;
static Unsubscription* aUnsubscriptions[10];
static int UnsubCount = 0;
static Publish* aPublish


void MQTT_init(void){
	MQTT_function_block.app_status.msg_version = FWF_APP_MQTT_INTERFACEVERSION;

	MQTT_function_block.msg_publish_event	= 0;

    io_status_struct.msg_version = FWF_APP_MQTT_INTERFACEVERSION;
    io_status_struct.led 			= 0x0;
    io_status_struct.voltage_ADC1 	= 0.0;
    io_status_struct.voltage_ADC2 	= 0.0;

	switch_event_msg_switch_event 	= 0;
}

// Aufruf bei Verbindungsaufbau
UINT16  MQTT_on_connect(void){
    tcp_server_var 				= (TCP_MQTT_SERVER_CONTEXT *)(uip_tcp_conn->appstate);
	tcp_server_var->state 		= MQTT_FSM_AWAIT_INIT;
    tcp_server_var->pSendDdata 	= MQTT_response;
    tcp_server_var->countSendDdata = 0; // kein Sendebedarf
    // MQTT-Library
    pPubSubClient = PubSubConstructor();
    pMqttConnect  = MqttConnectKonstruktor(pPubSubClient);

    return 0;
}

// UINT8 createMqttDisconnect(UINT8* buffer) {
//     buffer[0] = MQTTDISCONNECT;
//     buffer[1] = 0;

//     return 2;
// }

void MQTT_on_disconnect(void){
    // createMqttDisconnect(pPubSubClient);

    tcp_server_var 				= (TCP_MQTT_SERVER_CONTEXT *)(uip_tcp_conn->appstate);
	tcp_server_var->state 		= //welcher State?
    tcp_server_var->pSendDdata 	= disconnect(pPubSubClient);
    tcp_server_var->countSendDdata = 2; // Laenge eines Disconnect-Pakets

    free(pPubSubClient);
    free(pMqttConnect);
}

UINT8 isMqttConnect(UINT8 *buffer){
	if(buffer[2] !=  0)  return 0; // MQTT-Laenge
	if(buffer[3] !=  4)  return 0; // MQTT-Laenge
	if(buffer[4] != 'M') return 0;
	if(buffer[5] != 'Q') return 0;
	if(buffer[6] != 'T') return 0;
	if(buffer[7] != 'T') return 0;

	if(buffer[0] != MQTTCONNECT ) return 0;

	return 1;
}
UINT8 createMqttConnectACK(UINT8 *buffer){
	buffer[0] = MQTTCONNACK;
	buffer[1] = 2;				// Length
	buffer[2] = 0;				// Acknowledge Flags
	buffer[3] = 0;				// Connection accepted

	return 4;
}
UINT8 isMqttSubscribeRequest(UINT8 *buffer, Subscription* subscript){
    UINT16 len;
	if((buffer[0] & 0xf0) != MQTTSUBSCRIBE))  return 0; // Client Subscribe request + Reserved

	// buffer[0] // Message Length
	subscript->message_id = (buffer[1] << 8) + buffer[2]; 	// Message ID
	len = strncpy(subscript->topic_name, &buffer[5],  (buffer[3 << 8) + buffer[4]);  // topic_name
	subscript->Qos = buffer[6];

	return 1;
}
UINT8 createMqttSuback(UINT8 *buffer, Subscription* subscript) {
    buffer[0] = MQTTSUBACK + 3;     // Packet Type + Remaining Length
    buffer[1] = subscript->message_id  >> 8; // set MsgID 
    buffer[2] = subscript->message_id;
    switch (subscript->Qos) //Maximaler QoS, der etabliert wird, derzeit alles auf QoS 0
    {
    case 0:
        buffer[3] = 0;  //max. Qos 
        break;
    
    case 1: 
        buffer[3] = 0; //max. Qos
            break;
    
    case 2:
        buffer[3] = 0; // max. Qos
        break;

    default:
        buffer[3] = 0x80; // Error
        break;
    }
    
    return 4;
}

UINT8 isMqttUnsubscribe(UINT8* buffer, Unsubscription* unsubscript) {   //  1 Byte Control Packet & Flags, 2 Bytes Rem Len, X Bytes Topic
	if((buffer[0] & 0xf0) != MQTTUNSUBSCRIBE))  return 0; // Client Unsubscribe request + Reserved 
// Vorerst die Lösung
    UINT16 len;
	if((buffer[0] & 0xf0) != MQTTUNSUBSCRIBE))  return 0; // Client Subscribe request + Reserved

	// buffer[0] // Message Length
	unsubscript->message_id = (buffer[1] << 8) + buffer[2]; 	// Message ID
	len = strncpy(unsubscript->topic_name, &buffer[5],  (buffer[3 << 8) + buffer[4]);  // topic_name

	return 1;
}

UINT8 createMqttUnsuback(UINT8* buffer, Unsubscription* unsubscript, bool valid){
    buffer[0] = MQTTUNSUBACK;     // Packet Type 
    buffer[1] = 3;  // Remaining Length
    buffer[2] = unsubscript->message_id  >> 8; // set MsgID 
    buffer[3] = unsubscript->message_id;
    if(valid)   // Return Code (Reason String)
        buffer[4] = 0;
    else 
        buffer[4] = 17; // no topic matched
    return 4;
  
}

//Überprüft ob es ein Subscription auf ein bestimmtes Topic gibt. Ja: geben den Index der Subscription zurück; Nein: gibt Fehler zurück -1;
int CheckTopic(Unsubscription* unsubscript){
    for(int i = 0; i < 10; i++){
        if(aSubscriptions[i]->topic_name == unsubscript->topic_name){
            return i;
        }
    }

    return -1;
} 

// setzt alle Werte von der löschenden Subscription auf null
void deleteMqttSubscription(Subscription* subscript){
    memset(subscript->topic_name, 0, sizeof(subscript->topic_name));
    subscript->topic_length = 0;
    subscript->message_id = 0;
    subscript->Qos = 0;
    SubCount--;
}

UINT8 isMqttPublishRequest(UINT8 *buffer, Publish* publ){
    UINT16 len;

    if((buffer[0]& 0xf0)!= MQTTPUBLISH) return 0; //bit 0-3
    len= (buffer[3] << 8) + buffer[4];
    publ->topic_name[0]= '\0';

    if(len > TOPIC_LENGTH){
        perror("Lenght > TOPIC_LENGHT");
        return 0;
    }
    
    strncpy(publ->topic_name,(char*)&buffer[5],len);
    publ->topic_name[len]='\0';

    publ->message_id=(buffer[5+len]<<8)+ buffer[6+len]; //Ist message ID immer vorhanden?

    publ->remainingLenght= buffer[1];




        //Flags befinden sich im ersten byte. bit 4-7

       // Bit:   | 7 | 6 : 5 | 4 |
       //        +---+---+---+---+
       // Value: |DUP| QoS  |Retain|
       //        +---+---+---+---+

    publ->QoS =(buffer[0]& 0x06) >> 1; //Bit 5 und 6
    publ->DUP=(buffer[0] & 0x08) >> 3; //Bit 7
    publ->Retain=(buffer[0] & 0x01);    //Bit 4


    return 1;
}

UINT8 isMqttPingreq(UINT8 *buffer){ 
    if((buffer[0]& 0xf0) != MQTTPINGREQ) return 0; //Fixed Header nur 1 byte
    return 1;
}

UINT8 createMqttPingResp(UINT8 *buffer){
    buffer[0]=MQTTPINGRESP; //Fixed Header nur 1 byte
    return 4; //warum 4?
}

UINT8 isMqttPingResp(UINT8 *buffer){
    if((buffer[0]& 0xf0) != MQTTPINGRESP) return 0;
    pPubSubClient->pingOutstanding=false;
    return 1;
}

//======================================================================================================
//======================================================================================================
// HTW Uebung Kommandointerpreter fuer TCP-Datagramme
//
// flag beinhaltet den Kontext des Aufrufs aus der uip_main
// uip_flags beinhaltet den Status des uip-Stacks

/*-----------------------------------------------------------------------------------*/

#if 0
static const UINT8 *tx_MQTT_appdata   /* The uip_appdata pointer points to transmit application data. */
= &uip_txbuf[TCP_HEADER_SIZE + UIP_LLH_LEN];
#endif


// TCP Interpreter fuer MQTT
void fwf_tcp_MQTT_server(u8_t flag) {
int i;
char tcp_cmd  [TCP_CMD_SIZE]  = "";
char tcp_par1 [TCP_PAR1_SIZE] = "";
char tcp_par2 [TCP_PAR2_SIZE] = "";
uint8_t tcp_cmd_pos = 0;
uint8_t tcp_par1_pos = 0;
uint8_t tcp_par2_pos = 0;
UINT8 channel = 0;          // from received message extracted channel (PWM-,ADC-, LED-Number)

    if(uip_tcp_conn->lport == 0) uip_abort();	// Kein Serverport
   //tcp_debugout(flag, tcp_server_var->state, tcp_server_var->server_index);

    if(uip_connected()) {  //  1. event TCP connected -> Reaction: reset the MQTT_state for this connection.
    	tcp_server_var->countSendDdata = MQTT_on_connect(); // Kontext setzen ( z.B. tcp_server_var->state )
        tcp_server_var->state = MQTT_FSM_AWAIT_INIT;
        FWF_DBG2_PRINTFv("TCP:->FSM_AWAIT_INIT");
        return;
    }
    // Behandeln der TCP-Transfers Timeout - ausserhalb der Server-States
    if (uip_timedout()) { // event timeout
        FWF_DBG2_PRINTFv("\nTCP-SRV TIMEDout");
        uip_close();
        return;
     }

    if (uip_closed()){  // event close
        FWF_DBG2_PRINTFv("TCP:->CLOSE\n");
        MQTT_on_disconnect();
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
            // Is it a MQTT connect
            if(isMqttConnect(rx_MQTT_msg)){
            	tcp_server_var->countSendDdata = createMqttConnectACK(tcp_server_var->pSendDdata);
            }
            pPubSubClient->_state = MQTT_CONNECTED;
            tcp_server_var->state = MQTT_FSM_WORK;
#if 0
            // if ( (i = search_string( (char *)rx_MQTT_msg, "MQTT", 5 , uip_len)) <= uip_len) {
            if ( (i = SEARCH_STRING( rx_MQTT_msg, "mqtt", uip_len)) <= uip_len) {
                FWF_DBG1_PRINTFv("FSM_AWAIT_INIT:->FSM_INIT_NOACK %c%c%c",RX_TCP_MSG(i),RX_TCP_MSG(i+1),RX_TCP_MSG(i+2));
                //FWF_DBG2_PRINTFv("%c",RX_TCP_MSG(i+3));
                tcp_server_var->state = MQTT_FSM_INIT_NOACK;
                tcp_server_var->countSendDdata = sprintf((char*)MQTT_response,"MQTT.OK");
                FWF_DBG2_PRINTFv("FSM: MSG %s %d",tcp_server_var->pSendDdata , tcp_server_var->countSendDdata);
            }
#endif
            break;

        case MQTT_FSM_INIT_NOACK:  // 3. Vorstufe zu INIT (ohne Ack des client)
            if(uip_acked()) { // uip_flags & UIP_ACKDATA
                    FWF_DBG1_PRINTFv("FSM_INIT_NOACK:ack->FSM_WORK");
                    tcp_server_var->state = MQTT_FSM_WORK;
            } else {
                    FWF_DBG1_PRINTFv("ERR-TCP-state-%x",tcp_server_var->state);
            }
            break;

        case MQTT_FSM_WORK: 	// 4. nach INIT mit Ack des Client
            if ( !uip_newdata()) return;
            FWF_DBG2_PRINTFv("FSM@WORK ");
            if(isMqttSubscribeRequest(rx_MQTT_msg, &aSubscriptions[SubCount])){
               tcp_server_var->countSendDdata = createMqttSuback(tcp_server_var->pSendDdata, &aSubscriptions[SubCount]);
               SubCount++;
            }
            if(isMqttUnsubscribe(rx_MQTT_msg, &aUnsubscriptions[UnsubCount])){
                int ret = CheckTopic(&aUnsubscriptions[UnsubCount]); //valide Unsubscription?
                if ( ret >= 0 || ret < 10){
                    // send / create Unsuback
                    tcp_server_var->countSendDdata = createMqttUnsuback(tcp_server_var->pSendDdata, &aUnsubscriptions[UnsubCount], true);
                    deleteMqttSubscription(&aSubscriptions[ret]);
                    UnsubCount++;
                }

                else {
                    //MqttUnsubAck mit Fehler RC;
                    tcp_server_var->countSendDdata = createMqttUnsuback(tcp_server_var->pSendDdata, &aUnsubscriptions[UnsubCount], false);
                }

            }
            if( ){

            }
            if(isMqttPublishRequest(rx_MQTT_msg, aPublish)){
                
            }
            if(isMqttPingreq(rx_MQTT_msg)){
                tcp_server_var->countSendDdata = createMqttPingResp(tcp_server_var->pSendDdata);
                //TODO: lastOut und lastInActivity aktualisieren 
                //TODO: pingOutstanding?
            }
            if(isMqttPingResp(rx_MQTT_msg)){
                //TODO: lastOut und lastInActivity aktualisieren 
                //TODO: pingOutstanding?
            }
            tcp_cmd_pos = 0;
            rx_MQTT_msg[uip_len] = 0;      // terminate string for scanning
            MQTT_response[0] = 0;     		// erase datapointer for the data that will be sent back

#if USE_MQTT_SERVER
#if FWF_APP_STEPPER_STMdrv
            STEPPER_STM_MQTT_Interpreter(rx_MQTT_msg);
#endif // FWF_APP_STEPPER_STMdrv

#endif // USE_MQTT_SERVER

            // extract tcp_cmd, tcp_par1, tcp_par2 from received data
            // nicht effizient, fuer die Anwendung aber unkritisch und einfacher zu debuggen
            if ( (i = SEARCH_STRING(rx_MQTT_msg, "." ,uip_len)) <= uip_len){       // find first dot
                tcp_par1_pos = tcp_cmd_pos+i+1;
                strncpy(tcp_cmd, &rx_MQTT_msg[tcp_cmd_pos], i);
                if ( (i = SEARCH_STRING(&rx_MQTT_msg[i+1], "." ,uip_len)) <= uip_len){
                    tcp_par2_pos = tcp_par1_pos+i+1;
                    strncpy(tcp_par1, &rx_MQTT_msg[tcp_par1_pos], i);
                    strncpy(tcp_par2, &rx_MQTT_msg[tcp_par2_pos], uip_len-i);
                } else {    // no second dot found
                    strncpy(tcp_par1, &rx_MQTT_msg[tcp_par1_pos], uip_len-tcp_par1_pos);
                    tcp_par1[TCP_PAR1_SIZE-1] = 0;
                }
            } else { // kein dot enthalten
                strcpy(tcp_cmd, &rx_MQTT_msg[tcp_cmd_pos]);
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
            tcp_server_var->pSendDdata = MQTT_response;
            for(i = 0; tcp_server_var->pSendDdata[i] != '\n'; ++i);    /* Calculate length of string. */
            tcp_server_var->countSendDdata = i;
            FWF_DBG2_PRINTFv(" MSG %s %d",MQTT_response,i);

            break;

        case MQTT_FSM_NOINIT:
            FWF_DBG2_PRINTFv("FSM:ERR-TCP-FSM Noinit");
            break;

        default:
            FWF_DBG2_PRINTFv("FSM:Err-TCP-FSM");
            break;
    }

    // Send Data - dies ist aus verschiedenen Zustaenden heraus moeglich
    if(        (tcp_server_var->state & (MQTT_FSM_AWAIT_INIT | MQTT_FSM_WORK ) )
           &&   tcp_server_var->countSendDdata
      ) {  /* Send data, but not more than the MSS of the connection. uip_sappdata = (hs->pSendDdata); uip_slen = (hs->count) */
            uip_slen = tcp_app_transfer_to_tx_tcp_buffer((u8_t*)tcp_server_var->pSendDdata, tcp_server_var->countSendDdata);
            tcp_server_var->countSendDdata = 0;
    }
}


// TEST SUPPORT
#define FWF_DBG_CYCLE_PRINTFv	FWF_DBG1_PRINTFv



#endif // USE_MQTT_SERVER
