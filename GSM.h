#ifndef GSM_H
     #define GSM_H
     
     
     #define MEGA
     
     #include <inttypes.h>
     
     
     #define ctrlz 26 //Ascii character for ctr+z. End of a SMS.
     #define cr    13 //Ascii character for carriage return. 
     #define lf    10 //Ascii character for line feed. 
     #define ctrlz 26 //Ascii character for ctr+z. End of a SMS.
     #define cr    13 //Ascii character for carriage return. 
     #define lf    10 //Ascii character for line feed.
     #define GSM_LIB_VERSION 308 // library version X.YY (e.g. 1.00)
     
     #define DEBUG_ON
     
     
     #include "HWSerial.h"
     
     // if defined - debug print is enabled with possibility to print out
     // debug texts to the terminal program
     //#define DEBUG_PRINT
     
     // if defined - debug print is enabled with possibility to print out
     // the data recived from gsm module
     //#define DEBUG_GSMRX
     
     // if defined - debug LED is enabled, otherwise debug LED is disabled
     //#define DEBUG_LED_ENABLED
     
     // if defined - SMSs are not send(are finished by the character 0x1b
     // which causes that SMS are not send)
     // by this way it is possible to develop program without paying for the SMSs
     //#define DEBUG_SMS_ENABLED
     
     
     // pins definition
     #define GSM_ON              8 // connect GSM Module turn ON to pin 77 
     #define GSM_RESET           9 // connect GSM Module RESET to pin 35
     //#define DTMF_OUTPUT_ENABLE  71 // connect DTMF Output Enable not used
     #define DTMF_DATA_VALID     14 // connect DTMF Data Valid to pin 14
     #define DTMF_DATA0          72 // connect DTMF Data0 to pin 72
     #define DTMF_DATA1          73 // connect DTMF Data1 to pin 73
     #define DTMF_DATA2          74 // connect DTMF Data2 to pin 74
     #define DTMF_DATA3          75 // connect DTMF Data3 to pin 75
     
     // length for the internal communication buffer
     #define COMM_BUF_LEN        200
     
     // some constants for the IsRxFinished() method
     #define RX_NOT_STARTED      0
     #define RX_ALREADY_STARTED  1
     
     // some constants for the InitParam() method
     #define PARAM_SET_0   0
     #define PARAM_SET_1   1
     
     // DTMF signal is NOT valid
     //#define DTMF_NOT_VALID      0x10
     
     
     // status bits definition
     #define STATUS_NONE                 0
     #define STATUS_INITIALIZED          1
     #define STATUS_REGISTERED           2
     #define STATUS_USER_BUTTON_ENABLE   4
     
     // GPRS status
     #define CHECK_AND_OPEN    0
     #define CLOSE_AND_REOPEN  1
     
     // Common string used
     #define str_ok 		"OK"			//string to reduce stack usage
     #define str_at		"AT"			//string to reduce stack usage
     
     
     enum comm_line_status_enum {
          // CLS like CommunicationLineStatus
          CLS_FREE,   // line is free - not used by the communication and can be used
          CLS_ATCMD,  // line is used by AT commands, includes also time for response
          CLS_DATA,   // for the future - line is used in the CSD or GPRS communication
          CLS_LAST_ITEM
     };
     
     enum rx_state_enum {
          RX_NOT_FINISHED = 0,      // not finished yet
          RX_FINISHED,              // finished, some character was received
          RX_FINISHED_STR_RECV,     // finished and expected string received
          RX_FINISHED_STR_NOT_RECV, // finished, but expected string not received
          RX_TMOUT_ERR,             // finished, no character received
          // initial communication tmout occurred
          RX_LAST_ITEM
     };
     
     
     enum at_resp_enum {
          AT_RESP_ERR_NO_RESP = -1,   // nothing received
          AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
          AT_RESP_OK = 1,             // response_string was included in the response
     
          AT_RESP_LAST_ITEM
     };
     
     enum registration_ret_val_enum {
          REG_NOT_REGISTERED = 0,
          REG_REGISTERED,
          REG_NO_RESPONSE,
          REG_COMM_LINE_BUSY,
     
          REG_LAST_ITEM
     };
     
     
     class GSM {
          public:
               enum GSM_st_e { ERROR, IDLE, READY, ATTACHED, TCPSERVERWAIT, TCPCONNECTEDSERVER, TCPCONNECTEDCLIENT };
               byte comm_buf[COMM_BUF_LEN+1];  // communication buffer +1 for 0x00 termination
               void InitParam (byte group);
     
          private:
               int _status;
               byte comm_line_status;
          
               // global status - bits are used for representation of states
               byte module_status;
          
               // variables connected with communication buffer
          
               byte *p_comm_buf;               // pointer to the communication buffer
               byte comm_buf_len;              // num. of characters in the buffer
               byte rx_state;                  // internal state of rx state machine
               uint16_t start_reception_tmout; // max tmout for starting reception
               uint16_t interchar_tmout;       // previous time in msec.
               unsigned long prev_time;        // previous time in msec.
     
          protected:
               HWSerial _cell;
          
               int isIP(const char* cadena);
     
          public:
               inline void setStatus(GSM_st_e status) {
                    _status = status;
               };
               GSM();
               inline int getStatus() {
                    return _status;
               };
               virtual int begin(long baud_rate);
               inline void SetCommLineStatus(byte new_status) {
                    comm_line_status = new_status;
               };
               inline byte GetCommLineStatus(void) {
                    return comm_line_status;
               };
               void RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout);
               byte IsRxFinished(void);
               byte IsStringReceived(char const *compare_string);
               byte WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout);
               byte WaitResp(uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                             char const *expected_resp_string);
               char SendATCmdWaitResp(char const *AT_cmd_string,
                                      uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                                      char const *response_string,
                                      byte no_of_attempts);
               char SendATCmdWaitResp(const __FlashStringHelper *AT_cmd_string,
                                      uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                                      char const *response_string,
                                      byte no_of_attempts);
               void Echo(byte state);
          
               // checks if module is registered in the GSM network
               // must be called regularly
               byte CheckRegistration(void);
               // returns registration state
               byte IsRegistered(void);
               // returns whether complete initialization was made
               byte IsInitialized(void);
               //-----------------------
     
          #ifdef DEBUG_PRINT
               void DebugPrint(const char *string_to_print, byte last_debug_print);
               void DebugPrint(int number_to_print, byte last_debug_print);
          #endif
     };
     
#endif
