/* ========================================
fwf_uc_status.h
 * Author  		Kaever
 * Statusstruktur des uC
=========================================== */

#ifndef _FWF_uc_status_H_
#define _FWF_uc_status_H_

#ifdef _FWF_MAIN_MODULE_C
#define extern
#endif

#include "HW_devices.h"

// STREAMEN =============================================================
typedef enum CPP_ENUM_SIZE_8Bit {
	TAKE_NONE =         0,
	TAKE_SlowCtlPC_ARP   ,
	TAKE_StreamPC_ARP    ,
	TAKE_ROUTER_ARP      ,
	TAKE_DHCP_BROADCAST  ,
	TAKE_DHCP_SERVER     ,
	TAKE_STREAMING_HOST_IP_PORT_MAC   /// Streaming mode: IP, Port einsetzen, ggf. MAC (bei Multicast Gruppen-ID)
} TARGET_MODE;


#define DHCP_GER_LEN          32  // maximale Geraetenamenslaenge

#define CM4_TIMER_NUM 		  15  // Anzahl der Timer im CM4 + 1 wg. Systick
#define CM4_USART_NUM		  4	  // verwendete USART

#define STARTUP_RESET		  	  0x10

#define PORT_USE_FUNCTION_NAMELEN  64
typedef  char PORT_USE_FUNCTION_STRING[PORT_USE_FUNCTION_NAMELEN];

typedef UINT8 MACADDR[6];    			// MAC-Addr

typedef struct {
  UC_DEVICE_SIG devsig;					// device signature
  UINT32 sysstart_csr;
  UINT8  APB1_prescaler;
  UINT8  APB2_prescaler;
  APP_FLASH_HWINFO AppFlashHW;
  UINT8 Status_MSG_check;

  // Netzwerk
  UINT16 uC_IP[2] __attribute__ ((aligned (2)));                     // IP_Adresse des uC
  UINT16 uC_RxPort;
  UINT16 PC_SlowCtl_port;
  UINT16 PC_Stream_port;
  UINT16 PC_Stream_ip[2];
  UINT16 PC_SlowCtl_ip[2];
#if FWF_DEBUG_IP
  Union32 DBG_IP;						// IP fuer Debugging-Zwecke
#endif // FWF_DEBUG_IP

  UINT16 default_RouterIP[2];           // Default ROUTER
  UINT16 ARP_Netmask[2];				// Netmask fuer ARP-Layer

  MACADDR MACaddr __attribute__ ((aligned (2)));          			// MAC-Addr
  MACADDR PC_Stream_mac;				// Ethernet-Adresse fuer Streamen, falls kein Multicast-Mode
  MACADDR MultiCast_mac;				// Ethernet-Adresse fuer Streamen, falls Multicast-Mode

  UINT8 udp_conns_cnt;					// Anzahl der offenen UDP-Verbindungen
  UINT8 tcp_conns_cnt;					// Anzahl der offenen TCP-Verbindungen
  UINT8 udp_conns_initialized;			// Flag zur Anzeige der Initialisierung offener UDP-Verbindungen
  UINT8 tcp_conns_initialized;			// Flag zur Anzeige der Initialisierung offenerTCP-Verbindungen

  // Startup im Netzwerk
  UINT8 new_arp_entry_event		:1;	 	// Neuer ARP-Eintrag
  UINT8 eth_link_retries;                 // Versuche zum Initialisieren des Links 0: Dauerversuch; ansonsten Anzahl
  UINT8 udp_startup_msg_reqired :1;       // Melden des Startupvorgangs erforderlich

 // UINT8 uC_only_in_subnet  		:1;       // uC soll nur im Subnet sichtbar und aktiv sein, wenn der SlowCtlPC auch im Subnet ist
  UINT8 SlowctlPC_in_subnet     :1;       // Host befindet sich im gleichen Subnet
  UINT8 StreamPC_in_subnet      :1;       // Slowctl-Pc befindet sich im gleichen Subnet
  UINT8 router_in_subnet        :1;       // Der default_RouterIP befindet sich im Subnet
  UINT8 dhcp_in_subnet          :1;       // DHCP Server befindet sich im gleichen Subnet

  UINT8	use_dhcp                :1;       // Abfrage der eigenen IP-Adresse vom DHCP_Server
  UINT8 arp_got_fromRouter      :1;       // Status: erhaltener ARP-Request
  UINT8 arp_got_fromDhcp        :1;       // Status: erhaltener ARP-Request
  UINT8 arp_got_fromSlowCtlPC   :1;       // Status: erhaltener ARP-Request vom SlowCtlPC
  UINT8 arp_got_fromStreamPC    :1;       // Status: erhaltener ARP-Request vom StreamPC

  UINT8 send1msg_arp_dhcp_stream:1;      // cmd: 1 ARP oder DHCP anfragen oder stream-eventbasiertes Senden => 1msg zu senden
  UINT8 incoming_arp            :1;       // cmd: ARP - Anfrage eingegangen
  UINT8 incoming_dhcp           :1;       // DHCP-Antwort eingegangen
  UINT8 incoming_dhcp_offer     :1;       // DHCP-Offer ist eingegangen
  UINT8 incoming_dhcp_ack       :1;       // DHCP-ACK ist eingegangen

  UINT8 uC_dhcp_registered      :1;       // DHCP-REGISTERED - dhcp Prozess erfolgreich durchlaufen
  UINT8 uC_init_udpcon          :1;       // Finale Initialisierung der uip_udp_con nach startup

  UINT8 udp_connected_to_host           :1; // explizit mit PC ueber set Port verbunden
  UINT8 udp_disconnected_from_host      :1;	// Aufforderung zum Disconnect ist eingegangen
  UINT8 udp_host_reset_request          :1;	// Aufforderung zum Reset ist eingegangen

  UINT8 PC_Streaming_in_multicast_mode  :1; // Streaming Pakete werden als Multicast gesendet
  UINT8 StreamPC_SlowCtlPC_identical	:1; // SlowCtlPC und StreamPC sind identisch
  UINT8 stream_mac_valid				:1; // Stream Mac durch ARP valide
//
  TARGET_MODE		 sendto_HoRoNo;       // HOST, Router, None fuer Anfragen und eventbasierte oder spontane Nachrichten
  UINT8	retry_dhcp;                       // Anzahl der Abfragewiederholgungen beim DHCP_Server
  UINT8 msg_arp_forever;                  // FWF_ARP_FOREVER=1:Anzahl der Wiederholungen von ARP Requests ist unlimitiert
  UINT8 msg_retry_dhcp_cnt;                    // Anzahl der Wiederholungen von ARP Requests FWF_ARP_RQ_RETRIES
  UINT8 arps_on_startup_retry_count;      // Versuche zur Arp-Anfrage an Host-PC   0: Dauerversuch; ansonsten Anzahl
  UINT8 arps_on_startup_retries_act;      // Versuche zur Arp-Anfrage an Host-PC   0: Dauerversuch; ansonsten Anzahl
  UINT8 fill1;
  UINT16 msg_wait_delay;                  // Wartezeit fuer Wiederholung von ARP Requests
  // DHCP
  UINT8 dhcp_netmask[4];                  // DHCP_OPTION_SUBNET_MASK
  UINT32 XID;                             // DHCP Transaction identifier
  UINT16 dhcp_uc_ip[2];                   // IP Adresse vom DHCP-Server fuer uC-Client
  UINT16 dhcp_serverip[2];                // IP Adresse des DHCP-Servers DHCP_OPTION_SERVER_ID
  UINT8  dhcp_broadcast_ip[4];            // DHCP_OPTION_BROADCAST_ADDRESS
  UINT32 dhcp_lease_time_sec;             // DHCP_OPTION_LEASE_TIME
  UINT8 dhcp_dnsaddr[4];                  // DHCP_OPTION_DNS_SERVER
  DHCP_STATE connect_startup_state;       // Zustand fuer das Hochstarten im Netz mit DHCP / ARP

  // Webseiten-Information
  UINT8 patch_html;                       // zeigt an, ob die Webseite gepatched wurde

  // Events fuer UDP , TCP
  UINT8 tcp_event_flags;


  // Strukturen zur Zeitsteuerung
  UINT16 uart0_wait4result_1ms;      	  // Warte-Timer fuer Antworten auf uart0 im 1 ms - Raster
  UINT16 uart1_wait4result_1ms;      	  // Warte-Timer fuer Antworten auf uart1 im 1 ms - Raster
volatile  UINT32 timer_1ms;			  	  // Allgemeiner Zaehler fuer ms-Wartezeiten - NUR LESEN (Wird in ISR inkrementiert); ggf. in Applikationen rueckgesetzt
  UINT32 cycle;

  UINT8 						udp_send_EventMessages_enabled;		// Senden von Nachrichten bei Events im uC
  CMD_PHYSICAL_VALUES 			send_physical_values;
  STREAMING_CMD_STRUCT 			streaming_cmd;           		/// Client-Command via msg
  STREAMING_STATUS_STRUCT		streaming_status;               /// uC-Status fuer Umsetzung des Streaming-Zustandes
  UINT32						streaming_msg_number; 		    // Anzahl der Stream-messages, die testweise gestreamt werden
  MULTICAST_JOIN_LEAVE_CMD 		multicast_join_on_off_cmd;		// ToDo ggf. entfernen
  MULTICAST_JOIN_LEAVE_STATUS 	multicast_join_on_off_status;	// ToDo ggf. entfernen
  UINT16 multicast_join_leave_ip[2];  	// multicast ip-Adresse des Routers


  // ID- und Versionsinformation
  CONTROLLER_NAME	 controller_name;
  COM_VERSION_NAME	 com_version_name;					// Versionsstring der COM-Category
  COM_VERSION_NAME	 flash_version_name;				// Versionsstring der Flash-Category
  COMPILE_INFO		 compile_date;						// compile Date
  PIT_ID_NAME	   	 pit_id;							// PIT-Identifikation
  char dhcp_ucname[DHCP_GER_LEN];        				// DHCP-Name fuer das Geraet
  APPLICATION_NAME 		application_name;			// Application
  APPLICATION_NAME 		com_config_name;			// Ethernet & common
  APP_FLASH_PARAM_eSTATUS 	app_params_fit_to_flash;	// Validitaetsstatus der Applikationsparameter
  COM_FLASH_PARAM_eSTATUS	com_params_fit_to_flash;	// Validitaetsstatus der Kommunikationsparameter
  UINT8 					con_name_valid;


  UINT8 dbgflags_port6;
  UINT8 dbgflags_uart;
  UINT8 dbgflags_level;
  UINT8 dbgflags_msg;


#define  HW_DESC_LEN	128
  // HARDWARE-Information
  UINT8 spi1_cnt;		/// Anzahl der SPI-Devices
  UINT8 spi2_cnt;		/// Anzahl der SPI-Devices
  UINT8 spi3_cnt;		/// Anzahl der SPI-Devices
  SPI1_HW_TYPE spi1config;				/// Aktuell eingestellte SPI-Konfiguration
  SPI2_HW_TYPE spi2config;				/// Aktuell eingestellte SPI-Konfiguration
  SPI3_HW_TYPE spi3config;				/// Aktuell eingestellte SPI-Konfiguration
  SPI1_PINNING spi1_pinning;			/// Konfiguration des SPI-Pinnings
  SPI2_PINNING spi2_pinning;			/// Konfiguration des SPI-Pinnings
  SPI3_PINNING spi3_pinning;			/// Konfiguration des SPI-Pinnings

  char spi1[HW_DESC_LEN];
  char spi2[HW_DESC_LEN];
  char spi3[HW_DESC_LEN];

  UINT8 i2c1_cnt;
  UINT8 i2c2_cnt;
  UINT8 i2c3_cnt;
  char i2c1[HW_DESC_LEN];
  char i2c2[HW_DESC_LEN];
  char i2c3[HW_DESC_LEN];

  UINT32 use_port_a;
  UINT32 use_port_b;
  UINT32 use_port_c;
  UINT32 use_port_d;
  UINT32 use_port_e;
  UINT32 use_port_f;
  UINT32 use_port_g;
  PORT_USE_FUNCTION_STRING port_a_functions; // enthaelt die zuvor am Port aktiven Funktionsnamen
  PORT_USE_FUNCTION_STRING port_b_functions;
  PORT_USE_FUNCTION_STRING port_c_functions;
  PORT_USE_FUNCTION_STRING port_d_functions;
  PORT_USE_FUNCTION_STRING port_e_functions;
  PORT_USE_FUNCTION_STRING port_f_functions;
  PORT_USE_FUNCTION_STRING port_g_functions;
  int port_a_function_line;
  int port_b_function_line;
  int port_c_function_line;
  int port_d_function_line;
  int port_e_function_line;
  int port_f_function_line;
  int port_g_function_line;

  // Hardwarebelegung
	char * tim_use_by_fkt[CM4_TIMER_NUM];
	char * tim_name[CM4_TIMER_NUM];
  	char * usart_use[CM4_USART_NUM];
  // Diagnose
  UINT32 irq_tim2_cnt;
  UINT32 irq_tim3_cnt;
  UINT32 irq_tim4_cnt;

  UC_PARAMETER uc_params;   // Mikrocontrollerspezifische Parameter

 #define uC_DIAGNOSE_LOG_LEN 64
 #define uC_DIAGNOSE_LOG_NUM 10
  UINT8 diagnose_log_nr;

} uC_CONFIG;

extern uC_CONFIG uC;


#undef extern

#endif // _FWF_uc_status_H_
