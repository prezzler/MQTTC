#ifndef  __FWF_TYP_DEFINES_H__
#define  __FWF_TYP_DEFINES_H__
/* ========================================
Author HZDR FWF Kaever 

version 1.2  Typenpruefung, reduzierte Typenzahl

date    13.02.2015
=========================================== */

/* length specific types recommended by MISRA rule 13 */
typedef signed char     SINT8;
typedef unsigned char   UINT8;
typedef signed short    SINT16;
typedef unsigned short  UINT16;


	typedef unsigned char   u8_t;     // uip
	typedef unsigned short  u16_t;    // uip
	typedef unsigned long   u32_t;    // uip
	typedef unsigned long   UINT32;
	typedef   signed long   SINT32;

	typedef union {
		UINT8 	U8[4];
		UINT16 	U16[2];
		UINT32 	U32;
		SINT32  S32;
	} Union32;




#define FWF_APP_NAME_LEN	  64  // maximale Softwareconfigurationsnamenslaenge
typedef char APPLICATION_NAME[FWF_APP_NAME_LEN];

#define CONTROLLER_NAME_LEN		64
typedef char CONTROLLER_NAME[CONTROLLER_NAME_LEN];

#define PIT_ID_LEN				16
typedef char PIT_ID_NAME[PIT_ID_LEN];

#define PCB_ID_LEN				32
typedef char PCB_NAME[PCB_ID_LEN];

#define GIT_HASH_NAMELEN		32
typedef char GIT_HASH_NAME[GIT_HASH_NAMELEN];

#define GCC_VERSION_STRING_NAMELEN		128
typedef char GCC_VERSION_STRING[GCC_VERSION_STRING_NAMELEN];

#define APPLICATION_COMMENT_STRING_NAMELEN		128
typedef char APPLICATION_COMMENT_STRING[APPLICATION_COMMENT_STRING_NAMELEN];

#define VALID_CHAR_IN_NAME(x)   (((x) > 48) && ((x) < 122))

#pragma pack(1)
typedef struct {
	UINT32 					ManufacturerCode;
	UINT32 					DeviceCode;
	UINT32 					DieRevisionCode;
} UC_DEVICE_SIG;			// Mikrocontrollerspezifische Parameter
#pragma pack()

typedef enum CPP_ENUM_SIZE_8Bit {
	COMMMON_FLASH_PARAMS_UNCLEAR = 0,	// Status unklar
	COMMMON_FLASH_PARAMS_CLEARED,		// Flash geloescht
	COMMMON_FLASH_PARAMS_NAMEnok,		// Parameter ungeschrieben - aber auch nicht korrekt
	COMMMON_FLASH_PARAMS_NAMEok			// Name passt - alles ok
} COM_FLASH_PARAM_eSTATUS;				// Qualification status for Common (and Ethernet) Parameters

typedef enum CPP_ENUM_SIZE_8Bit {
	APPLICATION_FLASH_PARAMS_UNCLEAR = 0,	// Status unklar
	APPLICATION_FLASH_PARAMS_CLEARED,		// Flash geloescht
	APPLICATION_FLASH_PARAMS_UNNAMED,		// Parameter ungeschrieben - aber auch nicht verwendet
	APPLICATION_FLASH_PARAMS_NOT_NEEDED,	// Parameter unbenutzt und ungeschrieben
	APPLICATION_FLASH_PARAMS_NAMED			// Name passt - alles ok
} APP_FLASH_PARAM_eSTATUS;			// Qualification status for application Parameters


typedef enum CPP_ENUM_SIZE_8Bit {					/// Streaming status
	STREAMING_STAT_NONE 							= 0x00, /// Befehl falsch
	STREAMING_STAT_OFF 								= 0x01, /// Kein Streamen
	STREAMING_STAT_IOdata_OnCLK						= 0x02, /// clock-getriebenes Senden von IO-Daten
	STREAMING_STAT_IOdata_OnChange 					= 0x04, /// Von Signalaenderunggen getriebenes Senden von IO-Daten
	STREAMING_STAT_Testdata_OnCLK					= 0x08,	/// Senden von TESTDATA dauerhaft
	STREAMING_STAT_Testdata_OnChange				= 0x40,	/// Send TESTDATA on IO-changes
	STREAMING_STAT_IOdata_FixedNumber_OnCLK 		= 0x10, /// Senden von IODATA - begrenzte Anzahl STREAMING_TEST_NUMBER_OF_STREAMMSG
	STREAMING_STAT_Testdata_FixedNumber_OnCLK 		= 0x20 	/// Senden von TESTDATA - begrenzte Anzahl STREAMING_TEST_NUMBER_OF_STREAMMSG
}  STREAMING_eSTATUS;
#define ANY_STREAMING_STAT (STREAMING_STAT_IOdata_OnCLK | STREAMING_STAT_IOdata_OnChange | STREAMING_STAT_IOdata_FixedNumber_OnCLK | STREAMING_STAT_Testdata_OnCLK | STREAMING_STAT_Testdata_OnChange | STREAMING_STAT_Testdata_FixedNumber_OnCLK)
#define NO__STREAMING_STAT (STREAMING_STAT_NONE | STREAMING_STAT_OFF)

#endif // FWF_TYPES_USED


#endif /* __FWF_TYP_DEFINES_H__ */

/************************************ EOF ***********************************/
