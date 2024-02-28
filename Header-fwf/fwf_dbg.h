/*-----------------------------------------------------------------------------------------
File Name : fwf_dbg.c
Author    : Kaever
Version   : V1.0
Date      : 10.03.2013
-----------------------------------------------------------------------------------------*/
#ifndef __FWF_DBG
#define __FWF_DBG

#ifndef __INC_STDARG_H
#define __INC_STDARG_H
#include <stdarg.h>
#endif

#if __FWF_DBG_MODULE_
#define extern
#endif


// Diagnose

#define ENTER_DIAGNOSE(string) { uC.diagnose_log_nr++; if(uC.diagnose_log_nr >= uC_DIAGNOSE_LOG_NUM) uC.diagnose_log_nr = 0 ; sprintf((char*)uC.diagnose_log[uC.diagnose_log_nr],string); }


#define MSG_IN_DBG0 0x01
#define MSG_IN_DBG1 0x02
#define MSG_IN_DBG2 0x04
#define MSG_IN_DBG3 0x08
#define MSG_IN_DBG4 0x10
#define MSG_IN_DBG5 0x20
#define MSG_IN_DBG6 0x40
#define MSG_IN_DBG7 0x80


#if FWF_DEBUG_IP
	#define  CHECK_DEBUG_IP(DBG,IP)	check_debug_ip(DBG,IP)
#else
	#define  CHECK_DEBUG_IP(DBG,IP)
#endif //


#if (DEBUG_LEVEL_FWF > 0) && DEBUG_GLOBAL_ENABLE
  #define  FWF_DBG1_PRINTFv(format, ...) dbg_printfv(1,__FILE__ ,__func__ ,__LINE__, format, ##__VA_ARGS__)
#else
  #define  FWF_DBG1_PRINTFv(format, ...)
#endif

#if (DEBUG_LEVEL_FWF > 1) && DEBUG_GLOBAL_ENABLE
  #define  FWF_DBG2_PRINTFv(format, ...) dbg_printfv(1,__FILE__ ,__func__ ,__LINE__, format, ##__VA_ARGS__)
#else // Release
  #define  FWF_DBG2_PRINTFv(format, ...)
#endif

#if (DEBUG_LEVEL_FWF > 2) && DEBUG_GLOBAL_ENABLE
  #define  FWF_DBG3_PRINTFv(format, ...) dbg_printfv(1,__FILE__ ,__func__ ,__LINE__, format, ##__VA_ARGS__)
#else
  #define  FWF_DBG3_PRINTFv(format, ...)
#endif

#if (DEBUG_LEVEL_FWF > 3) && DEBUG_GLOBAL_ENABLE
  #define  FWF_DBG4_PRINTFv(format, ...) dbg_printfv(1,__FILE__ ,__func__ ,__LINE__, format, ##__VA_ARGS__)
#else
  #define  FWF_DBG4_PRINTFv(format, ...)
#endif


// Funktionsaufruf der Debugausgabe mit vollstaendigen Parametern __FILE__ ,__func__ ,__LINE__,....
int dbg_printfv(int verb, char *file, const char *func, int line, const char *format, ...);


#ifdef FWF_DBG_HW_SUPPORT
  #include <stdio.h>
  void err_isr(void);
  void break_dbg(void);
#else /* FWF_DEBUG */
  #define err_isr()
  #define break_dbg()
#endif /* FWF_DEBUG */

void TEST_ERROR(void);

// Debug-Funktionen vor Debug-Makros, damit die Makros nicht auf Funktionsprototypen angewendet werden

#ifndef __FWF_DBG_MODULE_

#if HW_DEBUG_PORT6_0
#define dbg_p6_set_0()   GPIO6_0_set()        // Set
#define dbg_p6_reset_0() GPIO6_0_reset()      // Reset
void    dbg_p6_toggle_0(void);
#else
#define dbg_p6_set_0()
#define dbg_p6_reset_0()
#define dbg_p6_toggle_0()
#endif

#if HW_DEBUG_PORT6_1
#define dbg_p6_set_1()   GPIO6_1_set()        // Set
#define dbg_p6_reset_1() GPIO6_1_reset()      // Reset
void    dbg_p6_toggle_1(void);
#else
#define dbg_p6_set_1()
#define dbg_p6_reset_1()
#define dbg_p6_toggle_1()
#endif

#if HW_DEBUG_PORT6_2
#define dbg_p6_set_2()   GPIO6_2_set()        // Set
#define dbg_p6_reset_2() GPIO6_2_reset()      // Reset
void    dbg_p6_toggle_2(void);
#else
#define dbg_p6_set_2()
#define dbg_p6_reset_2()
#define dbg_p6_toggle_2()
#endif

#if HW_DEBUG_PORT6_3
#define dbg_p6_set_3()   GPIO6_3_set()        // Set
#define dbg_p6_reset_3() GPIO6_3_reset()      // Reset
void    dbg_p6_toggle_3(void);
#else
#define dbg_p6_set_3()
#define dbg_p6_reset_3()
#define dbg_p6_toggle_3()
#endif

#if HW_DEBUG_PORT6_4
#define dbg_p6_set_4()   GPIO6_4_set()        // Set
#define dbg_p6_reset_4() GPIO6_4_reset()      // Reset
void    dbg_p6_toggle_4(void);
#else
#define dbg_p6_set_4()
#define dbg_p6_reset_4()
#define dbg_p6_toggle_4()
#endif


#if HW_DEBUG_PORT6_5
#define dbg_p6_set_5()   GPIO6_5_set()        // Set
#define dbg_p6_reset_5() GPIO6_5_reset()      // Reset
void    dbg_p6_toggle_5(void);
#else
#define dbg_p6_set_5()
#define dbg_p6_reset_5()
#define dbg_p6_toggle_5()
#endif

#if HW_DEBUG_PORT6_6
#define dbg_p6_set_6()   GPIO6_6_set()        // Set
#define dbg_p6_reset_6() GPIO6_6_reset()      // Reset
void    dbg_p6_toggle_6(void);
#else
#define dbg_p6_set_6()
#define dbg_p6_reset_6()
#define dbg_p6_toggle_6()
#endif


#endif // __FWF_DBG_MODULE_

#define FWF_DBG_STRUCT_NUM 500

#ifdef DEBUG
extern
UINT16 fwf_dbg_num_samples;
#endif


void GPIO_UART_dbg_init(void);
void check_debug_ip(Union32 *DBG_IP, const UINT16 *uC_IP);

void dbg_out_sysconfig(void);	        // Debug-Ausgabe zentraler Kenngroessen der Software
void dbg_print_clock_distribution(void); // Debug-Ausgabe der Clock-Konfiguration
void dbg_print_mem_organization(void);

UINT16 uCdbg_logv_entry(const char *format, ...);	// Eintrag von Messages ins Startup-Log
UINT16 uCdbg_log_clear(void);				// Loeschen des Startup-Log
UINT16 uCdbg_log_msginsert(UINT8 * target);	// Uebertrag des Startup-Log in target Message

UINT32 fwf_dbg_message_buf_out(UINT8 * BUF);


#undef extern
#endif /* __FWF_DBG */





