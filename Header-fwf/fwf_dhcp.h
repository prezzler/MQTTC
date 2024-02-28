/*-----------------------------------------------------------------------------------------
File Name : fwf_dhcp.h
Author    : Kaever
Version   : V1.0
Date      : 10.03.2013
-----------------------------------------------------------------------------------------*/
#ifndef __FWF_DHCP
#define __FWF_DHCP

#define FWF_ARP_RETRY_LIMIT_DEFAULT		10	 // Limit zum Senden von ARP-Paketen bei der Verbindungsaufnahme

#if DEBUG
#define FWF_ARP_REQUEST_DELAY           2000 // Wartezeit zwischen 2 ARP-Requests bei Startup uC
#define FWF_DHCP_REQUEST_DELAY          2000 // Wartezeit zwischen 2 ARP-Requests bei Startup uC
#define FWF_ARP_REQUEST_MINDELAY		100  // Minimale Wartezeit
#define FWF_ARP_REQUEST_RETRIES         20   // Anzahl der Wiederholungen von ARP Requests fuer Router
#define FWF_DHCP_RETRIES                6    // Anzahl der Wiederholungen von DHCP Requests
#else
#define FWF_ARP_REQUEST_DELAY           2000 // Wartezeit zwischen 2 ARP-Requests bei Startup uC
#define FWF_DHCP_REQUEST_DELAY          2000 // Wartezeit zwischen 2 ARP-Requests bei Startup uC
#define FWF_ARP_REQUEST_MINDELAY		100  // Minimale Wartezeit
#define FWF_ARP_REQUEST_RETRIES         10   // Anzahl der Wiederholungen von ARP Requests fuer Router
#define FWF_DHCP_RETRIES                5    // Anzahl der Wiederholungen von ARP Requests fuer Router
#endif



// connect_startup_states
#define uCSS_HANDLE_SM        9   // Schwelle, unterhalb der stets die StateMachine ausgefuehrt wird
#define uCSS_START            1
#define uCSS_ARP_START        2
#define uCSS_DHCP_OFFERED     3
#define uCSS_ARP_SETPORT      4
#define uCSS_DHCP_WAIT_OFFER  16
#define uCSS_DHCP_REQUESTED   18
#define uCSS_DHCP_REQACK      19
#define uCSS_ARP_ROUTER       20
#define uCSS_ARP_HOST         21
#define uCSS_WAIT_ENDLEASE    23
#define uCSS_ARP_NOCONNECT    26
#define uCSS_StartupMSG_SlowCtlPC       22
#define uCSS_ARP_FIN_SlowCtlPC          25
#define uCSS_ARP_AWAIT_SlowCtlPC_MAC    30
#define uCSS_ARP_SlowCtlPC_MAC_FIN      31
#define uCSS_ARP_FIN_ALL                33

typedef enum {
	START 		    = uCSS_START,
	ARP_START       = uCSS_ARP_START,
	DHCP_OFFERED    = uCSS_DHCP_OFFERED,
	ARP_SETPORT     = uCSS_ARP_SETPORT,
	DHCP_WAIT_OFFER = uCSS_DHCP_WAIT_OFFER,
	DHCP_REQUESTED  = uCSS_DHCP_REQUESTED,
	DHCP_REQACK     = uCSS_DHCP_REQACK,
    WAIT_ENDLEASE   	 = uCSS_WAIT_ENDLEASE,
	ARP_ROUTER      	 = uCSS_ARP_ROUTER,
	ARP_WaitSlowCtlPC    = uCSS_ARP_HOST,
    ARP_FIN_SlowCtlPC    = uCSS_ARP_FIN_SlowCtlPC,
	StartupMSG_SlowCtlPC = uCSS_StartupMSG_SlowCtlPC,
    ARP_WaitStreamPC	 = uCSS_ARP_AWAIT_SlowCtlPC_MAC,
    ARP_FIN_StreamPC 	 = uCSS_ARP_SlowCtlPC_MAC_FIN,
    ARP_FIN_ALL        	 = uCSS_ARP_FIN_ALL,
	ARP_NOCONNECT      	 = uCSS_ARP_NOCONNECT
} DHCP_STATE;


// OPCODES
#define DHCPDiscover 	1 // DHCP Discover message (boot request)
#define DHCPOffer 	    2 // DHCP Offer message
#define DHCPRequest    	3 // DHCP Request message
#define DHCPDecline    	4 // DHCP Decline message
#define DHCPAck    	    5 // DHCP Acknowledgment message
#define DHCPNak    	    6 // DHCP Negative Acknowledgment message
#define DHCPRelease    	7 // DHCP Release message
#define DHCPInform     	8 // DHCP Informational message



#define DHCP_OPTION_SUBNET_MASK         1
#define DHCP_OPTION_ROUTER              3
#define DHCP_OPTION_DNS_SERVER          6
#define DHCP_OPTION_HOSTNAME            12
#define DHCP_OPTION_DOMAIN_NAME         15
#define DHCP_OPTION_BROADCAST_ADDRESS   28
#define DHCP_OPTION_REQ_IPADDR          50
#define DHCP_OPTION_LEASE_TIME          51
#define DHCP_OPTION_MSG_TYPE            53
#define DHCP_OPTION_SERVER_IP           54
#define DHCP_OPTION_REQ_LIST            55
#define DHCP_OPTION_MSG_SIZE            57
#define DHCP_OPTION_WWW_SERVER          72
#define DHCP_OPTION_DOMAIN_SEARCH       119
#define DHCP_OPTION_END                 255


#pragma pack(1)
typedef struct  msg_dhcp_hdr {
  UINT8		opcode;	// DHCP Opcode constants
  UINT8		HType;	// hardware type :6 for IEEE 802
  UINT8		HLen;	// hardware address len : 6 for ethernet
  UINT8		Hops;	// set 0 by client
  UINT32	XID;	// Transaction identifier
  UINT16	secs;	// wait time on dhcp server
  UINT16	flags;	// broadcast: 1 by client
  UINT16	CIaddr[2];  // client IP address
  UINT16	YIaddr[2];	// IP address assigned by dhcp server
  UINT16	SIaddr[2];	// Next Server IP Address
  UINT16	GIaddr[2];	// Relay Agent IP Adress
  UINT8		CHaddr[16];	// Client Hardware MAC address
  UINT8		Sname[64];	// Server Name
  UINT8		file[128];	// Boot filename
  UINT32	maco;		// magic cookie
  UINT8		optcodeDHCP;	// option code
  UINT8		oplenDHCP;	// options
  UINT8		optionDHCP;	// options used by DHCPDISCOVER

#if 1
  UINT8 options[312];
#else
  UINT8		optcode2;	// option code
  UINT8		oplen2;		// options
  UINT8         option2[7]; 	//

  UINT8		optcode3;	// option code
  UINT8		oplen3;		// options
  UINT8         option3[10];	// options used by DHCPDISCOVER

  UINT8		reqlcode;	// parameter request list
  UINT8		reqllen;	// parameter list length
  UINT8         reqlit[6]; 	// item
  UINT8		end;		// 0xff bei Ende
  //UINT8         reserve[14]; 	// padding
#endif
} msgDHCP;
#pragma pack()

#pragma pack(1)
typedef struct  msg_offer_hdr {
  UINT8		opcode;	// DHCP Opcode constants
  UINT8		HType;	// hardware type :6 for IEEE 802
  UINT8		HLen;	// hardware address len : 6 for ethernet
  UINT8		Hops;	// set 0 by client
  UINT32	XID;	// Transaction identifier
  UINT16	secs;	// wait time on dhcp server
  UINT16	flags;	// broadcast: 1 by client
  UINT16	CIaddr[2];  // client IP address
  UINT16	YIaddr[2];	// IP address assigned by dhcp server
  UINT16	SIaddr[2];	// Next Server IP Address
  UINT16	GIaddr[2];	// Relay Agent IP Adress
  UINT8		CHaddr[16];	// Client Hardware MAC address
  UINT32	maco;		// magic cookie
  UINT8		optcodeDHCP;	// option code
  UINT8		oplenDHCP;	// options
  UINT8		optionDHCP;	// options used by DHCPDISCOVER
} msgOFFER;
#pragma pack()

struct dhcpc_state {
    u16_t ticks;
    const void *mac_addr;
    int mac_len;

    u8_t serverid[4];

    u16_t lease_time[2];
    u16_t ipaddr[2];
    u16_t netmask[2];
    u16_t dnsaddr[2];
    u16_t default_router[2];
};

UINT8 handle_incoming_dhcp(UINT8 *resp);
void fwf_Startup_dhcp_client(void);
UINT8 fwf_uC_netstart_init(UINT32 init);
#undef extern
#endif /* __FWF_DHCP */





