#include "SIM900.h"
#include "Streaming.h"

#define _GSM_CONNECTION_TOUT_ 5
#define _TCP_CONNECTION_TOUT_ 20
#define _GSM_DATA_TOUT_ 10

//#define RESETPIN 7

SIMCOM900 gsm;
SIMCOM900::SIMCOM900() {};
SIMCOM900::~SIMCOM900() {};

/**********************************************************
Function: 	This function premits to wake up the module
			(only for SIM908) when it goes in energy saving
			mode.

Author:		Marco Martines
Created:	unknown
Modified:	18/02/2014

Output:		none

Comments:	It would be nice to call this function
 			automatically when gsm.begin is called (of course
 			only if a SIM908 is used). 
**********************************************************/

char SIMCOM900::forceON() {
     char ret_val=0;
     char *p_char;
     char *p_char1;

     SimpleWriteln(F("AT+CREG?"));
     WaitResp(5000, 100, str_ok);
     if(IsStringReceived(str_ok)) {
          ret_val=1;
     }
     
     p_char = strchr((char *)(gsm.comm_buf),',');
     p_char1 = p_char+1;
     *(p_char1+2)=0;
     p_char = strchr((char *)(p_char1), ',');
     if (p_char != NULL) {
          *p_char = 0;
     }

     if((*p_char1)=='4') {
          digitalWrite(GSM_ON, HIGH);
          delay(1200);
          digitalWrite(GSM_ON, LOW);
          delay(10000);
          ret_val=2;
     }

     return ret_val;
}

int SIMCOM900::configandwait(char* pin)
{
     int connCode;
     //_tf.setTimeout(_GSM_CONNECTION_TOUT_);

     if(pin) setPIN(pin); //syv

     // Try 10 times to register in the network. Note this can take some time!
     for(int i=0; i<10; i++) {
          //Ask for register status to GPRS network.
          SimpleWriteln(F("AT+CGREG?"));

          //Se espera la unsolicited response de registered to network.
          while(gsm.WaitResp(5000, 50, "+CGREG: 0,")!=RX_FINISHED_STR_RECV) {
               //while (_tf.find("+CGREG: 0,"))  // CHANGE!!!!
          
               //connCode=_tf.getValue();
               connCode=_cell.read();
               if((connCode==1)||(connCode==5)) {
                    setStatus(READY);

                    SimpleWriteln(F("AT+CMGF=1")); //SMS text mode.
                    delay(200);
                    // Buah, we should take this to readCall()
                    SimpleWriteln(F("AT+CLIP=1")); //SMS text mode.
                    delay(200);
                    //_cell << "AT+QIDEACT" <<  _DEC(cr) << endl; //To make sure not pending connection.
                    //delay(1000);

                    return 1;
               }
          }
     }
     return 0;
}

/**
 * SIMCOM900::read(char* buffer, int buffersize)
 *
 * Waits for data to be readable from the gsm module, reads data until
 * no more is available or the buffer has been filled
 *
 * returns number of bytes read
 *
 */
int SIMCOM900::read(char* result, int resultlength)
{
     char temp;
     int i=0;

     #ifdef DEBUG_ON
          Serial.print(F("Starting read..\nWaiting for Data.."));
     #endif
     // Wait until we start receiving data
     while(gsm.available()<1) {
          delay(100);
          #ifdef DEBUG_ON
               Serial.print(F("."));
          #endif
     }

     while(gsm.available()>0 && i<(resultlength-1)) {
          temp=_cell.read();
          if(temp>0) {
               #ifdef DEBUG_ON
                    Serial.print(temp);
          #endif
          result[i]=temp;
          i++;
          }
          delay(1);
     }

     // Terminate the string
     result[resultlength-1]='\0';

     #ifdef DEBUG_ON
          Serial.println(F("\nDone.."));
     #endif
     return i;
}

int SIMCOM900::readCellData(int &mcc, int &mnc, long &lac, long &cellid)
{
     if (getStatus()==IDLE)
          return 0;

     //_tf.setTimeout(_GSM_DATA_TOUT_);
     //_cell.flush();
     SimpleWriteln(F("AT+QENG=1,0"));
     SimpleWriteln(F("AT+QENG?"));
     if(gsm.WaitResp(5000, 50, "+QENG")!=RX_FINISHED_STR_NOT_RECV)
          return 0;

     //mcc=_tf.getValue(); // The first one is 0
     mcc=_cell.read();
     //mcc=_tf.getValue();
     mcc=_cell.read();
     //mnc=_tf.getValue();
     mnc=_cell.read();
     //lac=_tf.getValue();
     lac=_cell.read();
     //cellid=_tf.getValue();
     cellid=_cell.read();

     gsm.WaitResp(5000, 50, "+OK");
     SimpleWriteln(F("AT+QENG=1,0"));
     gsm.WaitResp(5000, 50, "+OK");
     return 1;
}

int SIMCOM900::setPIN(char *pin)
{
     //Status = READY or ATTACHED.
     if((getStatus() != IDLE))
          return 2;

     //_tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.

     //_cell.flush();

     //AT command to set PIN.
     SimpleWrite(F("AT+CPIN="));
     SimpleWriteln(pin);

     //Expect str_ok.

     if(gsm.WaitResp(5000, 50, str_ok)!=RX_FINISHED_STR_NOT_RECV)
          return 0;
     else
          return 1;
}

int SIMCOM900::changeNSIPmode(char mode)
{
     //_tf.setTimeout(_TCP_CONNECTION_TOUT_);

     //if (getStatus()!=ATTACHED)
     //    return 0;

     //_cell.flush();

     SimpleWrite(F("AT+QIDNSIP="));
     SimpleWriteln(mode);
     if(gsm.WaitResp(5000, 50, str_ok)!=RX_FINISHED_STR_NOT_RECV) return 0;
     //if(!_tf.find(str_ok)) return 0;

     return 1;
}

int SIMCOM900::getCCI(char *cci)
{
     //Status must be READY
     if((getStatus() != READY))
          return 2;

     //_tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.

     //_cell.flush();

     //AT command to get CCID.
     SimpleWriteln(F("AT+QCCID"));

     //Read response from modem
#ifdef UNO
     _tf.getString("AT+QCCID\r\r\r\n","\r\n",cci, 21);
#endif
#ifdef MEGA
     _cell.getString("AT+QCCID\r\r\r\n","\r\n",cci, 21);
#endif

     //Expect str_ok.
     if(gsm.WaitResp(5000, 50, str_ok)!=RX_FINISHED_STR_NOT_RECV)
          return 0;
     else
          return 1;
}

int SIMCOM900::getIMEI(char *imei)
{

     //_tf.setTimeout(_GSM_DATA_TOUT_);	//Timeout for expecting modem responses.

     //_cell.flush();

     //AT command to get IMEI.
     SimpleWriteln(F("AT+GSN"));

     //Read response from modem
#ifdef UNO
     _tf.getString("\r\n","\r\n",imei, 16);
#endif
#ifdef MEGA
     _cell.getString("\r\n","\r\n",imei, 16);
#endif

     //Expect str_ok.
     if(gsm.WaitResp(5000, 50, str_ok)!=RX_FINISHED_STR_NOT_RECV)
          return 0;
     else
          return 1;
}

int SIMCOM900::available()
{
     return _cell.available();
}

uint8_t SIMCOM900::read()
{
     return _cell.read();
}

void SIMCOM900::SimpleRead()
{
     char datain;
     if(_cell.available()>0) {
          datain=_cell.read();
          if(datain>0) {
               Serial.print(datain);
          }
     }
}

void SIMCOM900::SimpleWrite(char *comm)
{
     _cell.print(comm);
}

void SIMCOM900::SimpleWrite(const char *comm)
{
     _cell.print(comm);
}

void SIMCOM900::SimpleWrite(int comm)
{
     _cell.print(comm);
}

void SIMCOM900::SimpleWrite(const __FlashStringHelper *pgmstr)
{
     _cell.print(pgmstr);
}

void SIMCOM900::SimpleWriteln(char *comm)
{
     _cell.println(comm);
}

void SIMCOM900::SimpleWriteln(const __FlashStringHelper *pgmstr)
{
     _cell.println(pgmstr);
}

void SIMCOM900::SimpleWriteln(char const *comm)
{
     _cell.println(comm);
}

void SIMCOM900::SimpleWriteln(int comm)
{
     _cell.println(comm);
}

void SIMCOM900::WhileSimpleRead()
{
     char datain;
     while(_cell.available()>0) {
          datain=_cell.read();
          if(datain>0) {
               Serial.print(datain);
          }
     }
}

byte GSM::IsRegistered(void)
{
     return (module_status & STATUS_REGISTERED);
}

byte GSM::IsInitialized(void)
{
     return (module_status & STATUS_INITIALIZED);
}


/**********************************************************
Method checks if the GSM module is registered in the GSM net
- this method communicates directly with the GSM module
  in contrast to the method IsRegistered() which reads the
  flag from the module_status (this flag is set inside this method)

- must be called regularly - from 1sec. to cca. 10 sec.

return values:
      REG_NOT_REGISTERED  - not registered
      REG_REGISTERED      - GSM module is registered
      REG_NO_RESPONSE     - GSM doesn't response
      REG_COMM_LINE_BUSY  - comm line between GSM module and Arduino is not free
                            for communication
**********************************************************/
byte GSM::CheckRegistration(void)
{
     byte status;
     byte ret_val = REG_NOT_REGISTERED;

     if (CLS_FREE != GetCommLineStatus()) return (REG_COMM_LINE_BUSY);
     SetCommLineStatus(CLS_ATCMD);
     _cell.println(F("AT+CREG?"));
     // 5 sec. for initial comm tmout
     // 50 msec. for inter character timeout
     status = WaitResp(5000, 50);

     if (status == RX_FINISHED) {
          // something was received but what was received?
          // ---------------------------------------------
          if(IsStringReceived("+CREG: 0,1")
                    || IsStringReceived("+CREG: 0,5")) {
               // it means module is registered
               // ----------------------------
               module_status |= STATUS_REGISTERED;


               // in case GSM module is registered first time after reset
               // sets flag STATUS_INITIALIZED
               // it is used for sending some init commands which
               // must be sent only after registration
               // --------------------------------------------
               if (!IsInitialized()) {
                    module_status |= STATUS_INITIALIZED;
                    SetCommLineStatus(CLS_FREE);
                    InitParam(PARAM_SET_1);
               }
               ret_val = REG_REGISTERED;
          } else {
               // NOT registered
               // --------------
               module_status &= ~STATUS_REGISTERED;
               ret_val = REG_NOT_REGISTERED;
          }
     } else {
          // nothing was received
          // --------------------
          ret_val = REG_NO_RESPONSE;
     }
     SetCommLineStatus(CLS_FREE);


     return (ret_val);
}


