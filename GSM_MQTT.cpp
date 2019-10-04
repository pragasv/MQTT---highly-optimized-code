/*
  MQTT.h - Library for GSM MQTT Client.
  Created by Nithin K. Kurian, Dhanish Vijayan, Elementz Engineers Guild Pvt. Ltd, July 2, 2016.
  Released into the public domain.

  Obtimized by V.Pragatheeswaran 28th Aug 2018.
*/

#include "GSM_MQTT.h"
#include "Arduino.h"
//#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
extern uint8_t GSM_Response;
//extern SoftwareSerial mySerial;

extern GSM_MQTT MQTT;
uint8_t GSM_Response = 0;
unsigned long previousMillis = 0;
//char inputString[UART_BUFFER_LENGTH];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
void serialEvent();
int TextCheck(char*A, int B);

//new parts of the progmem for string check
const char INITIAL[] PROGMEM = "INITIAL";
const char START[] PROGMEM = "START" ;
const char IP_CONFIG[] PROGMEM = "IP CONFIG";
const char GPRSACT[] PROGMEM = "GPRSACT";
const char STATUS[] PROGMEM = "STATUS";
const char TCP_CLOSED[] PROGMEM = "TCP CLOSED";
const char TCP_CONNECTING[] PROGMEM = "TCP CONNECTING";
const char CONNECT_OK[] PROGMEM = "CONNECT OK";
const char CONNECT_FAIL[] PROGMEM = "CONNECT FAIL";
const char PDP_DEACT[] PROGMEM = "PDP DEACT";
const char OK[] PROGMEM = "OK";
const char error[] PROGMEM = "ERROR";
const char dot[] PROGMEM = ".";
//const char CONNECT_FAIL[] PROGMEM = "CONNECT FAIL";
const char CONNECT_str[] PROGMEM = "CONNECT";
const char CLOSED[] PROGMEM = "CLOSED";

const char * const string_table[] PROGMEM =
{
  INITIAL,
  START,
  IP_CONFIG,
  GPRSACT,
  STATUS,
  TCP_CLOSED,
  TCP_CONNECTING,
  CONNECT_OK,
  CONNECT_FAIL,
  PDP_DEACT,
  OK,
  error,
  dot,
  CONNECT_FAIL,
  CONNECT_str,
  CLOSED
};

GSM_MQTT::GSM_MQTT(unsigned long KeepAlive)
{
  _KeepAliveTimeOut = KeepAlive;
}

void GSM_MQTT::begin(void)
{
  //mySerial.begin(9600);
  Serial.begin(9600);
  Serial.write("AT\r\n");
  delay(1000);
  _tcpInit();
}
char GSM_MQTT::_sendAT(int command, unsigned long waitms)
{

  unsigned long PrevMillis = millis();
  strcpy(reply, "none");
  GSM_Response = 0;
  switch (command) {
    case 0:
      {
        Serial.print(F("AT\r\n"));
        ////Serial.flush();
        break;
      }
    case 1:
      {
        Serial.print(F("ATE1\r\n"));
        ////Serial.flush();
        break;
      }
    case 2:
      {
        Serial.print(F("AT+CIPMUX=0\r\n"));
        ////Serial.flush();
        break;
      }
    case 3:
      {
        Serial.print(F("AT+CIPMODE=1\r\n"));
        //Serial.flush();
        break;
      }
    case 4:
      {
        Serial.print(F("AT+CGATT=1\r\n"));
        //Serial.flush();
        break;
      }
    case 5:
      {
        //"AT+CSTT=\"ppwap,"",""\"\r\n"
        Serial.print(F("AT+CSTT=\"ppwap\",\"\",\"\"\r\n"));
        //Serial.flush();
        break;
      }
    case 6:
      {
        Serial.print(F("AT+CIICR\r\n"));
        //Serial.flush();
        break;
      }
    case 7:
      {
        Serial.print(F("\"\r\n"));
        //Serial.flush();
        break;
      }

  }
  delay(1000);
  unsigned long currentMillis = millis();
  //  //mySerial.println(PrevMillis);
  //  //mySerial.println(currentMillis);
  while ( (GSM_Response == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    //    delay(1);
    serialEvent();
    currentMillis = millis();
  }
  return GSM_Response;
}
char GSM_MQTT::sendATreply(int command, char *replystr, unsigned long waitms)
{
  strcpy(reply, replystr);
  unsigned long PrevMillis = millis();
  GSM_ReplyFlag = 0;
  switch (command) {
    case 0:
      {
        Serial.print(F("AT+CREG?\r\n"));
        //Serial.flush();
        break;
      }
    case 1:
      {
        Serial.print(F("AT+CGATT?\r\n"));
        //Serial.flush();
        break;
      }
    case 2:
      {
        Serial.print(F("AT+CIPSTATUS\r\n"));
        //Serial.flush();
        break;
      }
    case 3:
      {
        Serial.print(F("AT+CIFSR\r\n"));
        //Serial.flush();
        break;
      }
    case 4:
      {
        Serial.print(F("AT+CIPSHUT\r\n"));
        //Serial.flush();
        break;
      }
  }
  delay(1000);
  unsigned long currentMillis = millis();

  //  //mySerial.println(PrevMillis);
  //  //mySerial.println(currentMillis);
  while ( (GSM_ReplyFlag == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    //    delay(1);
    serialEvent();
    currentMillis = millis();
  }
  return GSM_ReplyFlag;
}
void GSM_MQTT::_tcpInit(void)
{
  switch (modemStatus)
  {
    case 0:
      {
        delay(1000);
        Serial.print(F("+++"));
        ////Serial.flush();
        delay(500);

        //softSerial.println(F("+++"));
        //"AT\r\n"
        if (_sendAT(0, 5000) == 1)
        {
          //softSerial.println("AT");
          //softSerial.println("OK");
          modemStatus = 1;
        }
        else
        {
          modemStatus = 0;
          //          //softSerial.println("AT");

          break;
        }
      }
    case 1:
      {
        //"ATE1\r\n"
        if (_sendAT(1, 2000) == 1)
        {
          //softSerial.println("ATE1");
          //softSerial.println("Echo mode on ");
          modemStatus = 2;
        }
        else
        {
          //softSerial.println("ATE1");
          //softSerial.println("Echo mode off ");
          modemStatus = 1;
          break;
        }
      }
    case 2:
      {
        //softSerial.println("AT+CREG?\r\n");
        //"AT+CREG?\r\n"
        if (sendATreply(0, "0,1", 5000) == 1)
        {
          //AT+CIPMUX=0\r\n
          _sendAT(2, 2000);
          //AT+CIPMODE=1\r\n
          _sendAT(3, 2000);
          //"AT+CGATT?\r\n"
          if (sendATreply(1, ": 1", 4000) != 1)
          {
            //AT+CGATT=1\r\n
            _sendAT(4, 2000);
          }
          modemStatus = 3;
          _tcpStatus = 2;
        }
        else
        {
          modemStatus = 2;
          break;
        }
      }
    case 3:
      {
        if (GSM_ReplyFlag != 7)
        {
          //"AT+CIPSTATUS\r\n"
          _tcpStatus = sendATreply(2, "STATE", 4000);
          if (_tcpStatusPrev == _tcpStatus)
          {
            tcpATerrorcount++;
            if (tcpATerrorcount >= 10)
            {
              tcpATerrorcount = 0;
              _tcpStatus = 7;
            }

          }
          else
          {
            _tcpStatusPrev = _tcpStatus;
            tcpATerrorcount = 0;
          }
        }
        _tcpStatusPrev = _tcpStatus;
        ////softSerial.print(_tcpStatus);
        switch (_tcpStatus)
        {
          case 2:
            {
              //"AT+CSTT=\"ppwap\",\"\",\"\"\r\n"
              _sendAT(5, 5000);
              break;
            }
          case 3:
            {
              //"AT+CIICR\r\n"
              _sendAT(6, 5000);
              break;
            }
          case 4:
            {
              //"AT+CIFSR\r\n"
              sendATreply(3, ".", 4000);
              break;
            }
          case 5:
            {
              Serial.print(F("AT+CIPSTART=\"TCP\",\""));
              //Serial.flush();
              Serial.print(F("mmqtt.senzmate.com"));
              //Serial.flush();
              Serial.print(F("\",\""));
              //Serial.flush();
              Serial.print(F("1883"));
              //Serial.flush();
              if (_sendAT(7, 5000) == 1)
              {
                unsigned long PrevMillis = millis();
                unsigned long currentMillis = millis();
                while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
                {
                  //    delay(1);
                  serialEvent();
                  currentMillis = millis();
                }
              }
              break;
            }
          case 6:
            {
              unsigned long PrevMillis = millis();
              unsigned long currentMillis = millis();
              while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
              {
                //    delay(1);
                serialEvent();
                currentMillis = millis();
              }
              break;
            }
          case 7:
            {
              //"AT+CIPSHUT\r\n"
              sendATreply(4, "SHUT OK", 4000) ;
              modemStatus = 0;
              _tcpStatus = 2;
              break;
            }

        }
      }
  }
}

void GSM_MQTT::_ping(void)
{

  if (pingFlag == true)
  {
    unsigned long currentMillis = millis();
    if ((currentMillis - _PingPrevMillis ) >= _KeepAliveTimeOut * 1000)
    {
      // save the last time you blinked the LED
      _PingPrevMillis = currentMillis;
      Serial.print(char(PINGREQ * 16));
      _sendLength(0);
    }
  }
}
void GSM_MQTT::_sendUTFString(char *string)
{
  int localLength = strlen(string);
  Serial.print(char(localLength / 256));
  Serial.print(char(localLength % 256));
  Serial.print(string);
}
void GSM_MQTT::_sendLength(int len)
{
  bool  length_flag = false;
  while (length_flag == false)
  {
    if ((len / 128) > 0)
    {
      Serial.print(char(len % 128 + 128));
      len /= 128;
    }
    else
    {
      length_flag = true;
      Serial.print(char(len));
    }
  }
}
void GSM_MQTT::connect(char *ClientIdentifier, char UserNameFlag, char PasswordFlag, char *UserName, char *Password, char CleanSession, char WillFlag, char WillQoS, char WillRetain, char *WillTopic, char *WillMessage)
{
  ConnectionAcknowledgement = NO_ACKNOWLEDGEMENT ;
  Serial.print(char(CONNECT * 16 ));
  char ProtocolName[7] = "MQIsdp";
  int localLength = (2 + strlen(ProtocolName)) + 1 + 3 + (2 + strlen(ClientIdentifier));
  if (WillFlag != 0)
  {
    localLength = localLength + 2 + strlen(WillTopic) + 2 + strlen(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    localLength = localLength + 2 + strlen(UserName);

    if (PasswordFlag != 0)
    {
      localLength = localLength + 2 + strlen(Password);
    }
  }
  _sendLength(localLength);
  _sendUTFString(ProtocolName);
  Serial.print(char(_ProtocolVersion));
  Serial.print(char(UserNameFlag * User_Name_Flag_Mask + PasswordFlag * Password_Flag_Mask + WillRetain * Will_Retain_Mask + WillQoS * Will_QoS_Scale + WillFlag * Will_Flag_Mask + CleanSession * Clean_Session_Mask));
  Serial.print(char(_KeepAliveTimeOut / 256));
  Serial.print(char(_KeepAliveTimeOut % 256));
  _sendUTFString(ClientIdentifier);
  if (WillFlag != 0)
  {
    _sendUTFString(WillTopic);
    _sendUTFString(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    _sendUTFString(UserName);
    if (PasswordFlag != 0)
    {
      _sendUTFString(Password);
    }
  }
}
void GSM_MQTT::publish(char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message)
{
  Serial.print(char(PUBLISH * 16 + DUP * DUP_Mask + Qos * QoS_Scale + RETAIN));
  int localLength = (2 + strlen(Topic));
  if (Qos > 0)
  {
    localLength += 2;
  }
  localLength += strlen(Message);
  _sendLength(localLength);
  _sendUTFString(Topic);
  if (Qos > 0)
  {
    Serial.print(char(MessageID / 256));
    Serial.print(char(MessageID % 256));
  }
  Serial.print(Message);
}
void GSM_MQTT::publishACK(unsigned int MessageID)
{
  Serial.print(char(PUBACK * 16));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}
void GSM_MQTT::publishREC(unsigned int MessageID)
{
  Serial.print(char(PUBREC * 16));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}
void GSM_MQTT::publishREL(char DUP, unsigned int MessageID)
{
  Serial.print(char(PUBREL * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}

void GSM_MQTT::publishCOMP(unsigned int MessageID)
{
  Serial.print(char(PUBCOMP * 16));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}
void GSM_MQTT::subscribe(char DUP, unsigned int MessageID, char *SubTopic, char SubQoS)
{
  Serial.print(char(SUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = 2 + (2 + strlen(SubTopic)) + 1;
  _sendLength(localLength);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
  _sendUTFString(SubTopic);
  Serial.print(SubQoS);

}
void GSM_MQTT::unsubscribe(char DUP, unsigned int MessageID, char *SubTopic)
{
  Serial.print(char(UNSUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = (2 + strlen(SubTopic)) + 2;
  _sendLength(localLength);

  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));

  _sendUTFString(SubTopic);
}
void GSM_MQTT::disconnect(void)
{
  Serial.print(char(DISCONNECT * 16));
  _sendLength(0);
  pingFlag = false;
}
//Messages
const char CONNECTMessage[] PROGMEM  = {"Client request to connect to Server\r\n"};
const char CONNACKMessage[] PROGMEM  = {"Connect Acknowledgment\r\n"};
const char PUBLISHMessage[] PROGMEM  = {"Publish message\r\n"};
const char PUBACKMessage[] PROGMEM  = {"Publish Acknowledgment\r\n"};
const char PUBRECMessage[] PROGMEM  = {"Publish Received (assured delivery part 1)\r\n"};
const char PUBRELMessage[] PROGMEM  = {"Publish Release (assured delivery part 2)\r\n"};
const char PUBCOMPMessage[] PROGMEM  = {"Publish Complete (assured delivery part 3)\r\n"};
const char SUBSCRIBEMessage[] PROGMEM  = {"Client Subscribe request\r\n"};
const char SUBACKMessage[] PROGMEM  = {"Subscribe Acknowledgment\r\n"};
const char UNSUBSCRIBEMessage[] PROGMEM  = {"Client Unsubscribe request\r\n"};
const char UNSUBACKMessage[] PROGMEM  = {"Unsubscribe Acknowledgment\r\n"};
const char PINGREQMessage[] PROGMEM  = {"PING Request\r\n"};
const char PINGRESPMessage[] PROGMEM  = {"PING Response\r\n"};
const char DISCONNECTMessage[] PROGMEM  = {"Client is Disconnecting\r\n"};

void GSM_MQTT::printMessageType(uint8_t Message)
{
  switch (Message)
  {
    case CONNECT:
      {
        int k, len = strlen_P(CONNECTMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(CONNECTMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case CONNACK:
      {
        int k, len = strlen_P(CONNACKMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(CONNACKMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case PUBLISH:
      {
        int k, len = strlen_P(PUBLISHMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBLISHMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case PUBACK:
      {
        int k, len = strlen_P(PUBACKMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBACKMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case  PUBREC:
      {
        int k, len = strlen_P(PUBRECMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBRECMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case PUBREL:
      {
        int k, len = strlen_P(PUBRELMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBRELMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case PUBCOMP:
      {
        int k, len = strlen_P(PUBCOMPMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBCOMPMessage  + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case SUBSCRIBE:
      {
        int k, len = strlen_P(SUBSCRIBEMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(SUBSCRIBEMessage  + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case SUBACK:
      {
        int k, len = strlen_P(SUBACKMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(SUBACKMessage  + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case UNSUBSCRIBE:
      {
        int k, len = strlen_P(UNSUBSCRIBEMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(UNSUBSCRIBEMessage  + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case UNSUBACK:
      {
        int k, len = strlen_P(UNSUBACKMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(UNSUBACKMessage  + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case PINGREQ:
      {
        int k, len = strlen_P(PINGREQMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PINGREQMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case PINGRESP:
      {
        int k, len = strlen_P(PINGRESPMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PINGRESPMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case DISCONNECT:
      {
        int k, len = strlen_P(DISCONNECTMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(DISCONNECTMessage + k);
          //mySerial.print(myChar);
        }
        break;
      }
  }
}

//Connect Ack
const char ConnectAck0[] PROGMEM  = {"Connection Accepted\r\n"};
const char ConnectAck1[] PROGMEM  = {"Connection Refused: unacceptable protocol version\r\n"};
const char ConnectAck2[] PROGMEM  = {"Connection Refused: identifier rejected\r\n"};
const char ConnectAck3[] PROGMEM  = {"Connection Refused: server unavailable\r\n"};
const char ConnectAck4[] PROGMEM  = {"Connection Refused: bad user name or password\r\n"};
const char ConnectAck5[] PROGMEM  = {"Connection Refused: not authorized\r\n"};
void GSM_MQTT::printConnectAck(uint8_t Ack)
{
  switch (Ack)
  {
    case 0:
      {
        int k, len = strlen_P(ConnectAck0);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck0 + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case 1:
      {
        int k, len = strlen_P(ConnectAck1);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck1 + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case 2:
      {
        int k, len = strlen_P(ConnectAck2);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck2 + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case 3:
      {
        int k, len = strlen_P(ConnectAck3);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck3 + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case 4:
      {
        int k, len = strlen_P(ConnectAck4);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck4 + k);
          //mySerial.print(myChar);
        }
        break;
      }
    case 5:
      {
        int k, len = strlen_P(ConnectAck5);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck5 + k);
          //mySerial.print(myChar);
        }
        break;
      }
  }
}
unsigned int GSM_MQTT::_generateMessageID(void)
{
  if (_LastMessaseID < 65535)
  {
    return ++_LastMessaseID;
  }
  else
  {
    _LastMessaseID = 0;
    return _LastMessaseID;
  }
}
void GSM_MQTT::processing(void)
{
  if (TCP_Flag == false)
  {
    MQTT_Flag = false;
    _tcpInit();
  }
  _ping();
}
bool GSM_MQTT::available(void)
{
  return MQTT_Flag;
}
void serialEvent()
{

  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    if (MQTT.TCP_Flag == false)
    {
      if (MQTT.index < 200)
      {
        MQTT.inputString[MQTT.index++] = inChar;
      }
      if (inChar == '\n')
      {
        MQTT.inputString[MQTT.index] = 0;
        stringComplete = true;
        //mySerial.print(MQTT.inputString);
        if (strstr(MQTT.inputString, MQTT.reply) != NULL)
        {
          MQTT.GSM_ReplyFlag = 1;
          if (TextCheck(MQTT.inputString, 0) != 0) //" INITIAL"
          {
            MQTT.GSM_ReplyFlag = 2; //
          }
          else if (TextCheck(MQTT.inputString, 1) != 0) //" START"
          {
            MQTT.GSM_ReplyFlag = 3; //
          }
          else if (TextCheck(MQTT.inputString, 2) != 0) //"IP CONFIG"
          {
            _delay_us(10);
            MQTT.GSM_ReplyFlag = 4;
          }
          else if (TextCheck(MQTT.inputString, 3) != 0) //" GPRSACT"
          {
            MQTT.GSM_ReplyFlag = 4; //
          }
          else if ((TextCheck(MQTT.inputString, 4) != 0) || (TextCheck(MQTT.inputString, 5) != 0)) //" STATUS", "TCP CLOSED"
          {
            MQTT.GSM_ReplyFlag = 5; //
          }
          else if (TextCheck(MQTT.inputString, 6) != 0) //" TCP CONNECTING"
          {
            MQTT.GSM_ReplyFlag = 6; //
          }
          else if ((TextCheck(MQTT.inputString, 7) != 0) || (TextCheck(MQTT.inputString, 8) != NULL) || (TextCheck(MQTT.inputString, 9) != 0))//" CONNECT OK","CONNECT FAIL","PDP DEACT"
          {
            MQTT.GSM_ReplyFlag = 7;
          }
        }
        else if (TextCheck(MQTT.inputString, 10) != NULL)//"OK"
        {
          GSM_Response = 1;
        }
        else if (TextCheck(MQTT.inputString, 11) != NULL)//"ERROR"
        {
          GSM_Response = 2;
        }
        else if (TextCheck(MQTT.inputString, 12) != NULL)//"."
        {
          GSM_Response = 3;
        }
        else if (TextCheck(MQTT.inputString, 13) != NULL)//"CONNECT FAIL"
        {
          GSM_Response = 5;
        }
        else if (TextCheck(MQTT.inputString, 14) != NULL)//"CONNECT"
        {
          GSM_Response = 4;
          MQTT.TCP_Flag = true;
          //mySerial.println("MQTT.TCP_Flag = True");
          MQTT.AutoConnect();
          MQTT.pingFlag = true;
          MQTT.tcpATerrorcount = 0;
        }
        else if (TextCheck(MQTT.inputString, 15) != NULL)// "CLOSED"
        {
          GSM_Response = 4;
          MQTT.TCP_Flag = false;
          MQTT.MQTT_Flag = false;
        }
        MQTT.index = 0;
        MQTT.inputString[0] = 0;
      }
    }
    else
    {
      uint8_t ReceivedMessageType = (inChar / 16) & 0x0F;
      uint8_t DUP = (inChar & DUP_Mask) / DUP_Mask;
      uint8_t QoS = (inChar & QoS_Mask) / QoS_Scale;
      uint8_t RETAIN = (inChar & RETAIN_Mask);
      if ((ReceivedMessageType >= CONNECT) && (ReceivedMessageType <= DISCONNECT))
      {
        bool NextLengthByte = true;
        MQTT.length = 0;
        MQTT.lengthLocal = 0;
        uint32_t multiplier = 1;
        delay(2);
        char Cchar = inChar;
        while ( (NextLengthByte == true) && (MQTT.TCP_Flag == true))
        {
          if (Serial.available())
          {
            inChar = (char)Serial.read();
            //mySerial.println(inChar, DEC);
            if ((((Cchar & 0xFF) == 'C') && ((inChar & 0xFF) == 'L') && (MQTT.length == 0)) || (((Cchar & 0xFF) == '+') && ((inChar & 0xFF) == 'P') && (MQTT.length == 0)))
            {
              MQTT.index = 0;
              MQTT.inputString[MQTT.index++] = Cchar;
              MQTT.inputString[MQTT.index++] = inChar;
              MQTT.TCP_Flag = false;
              MQTT.MQTT_Flag = false;
              MQTT.pingFlag = false;
              //mySerial.println("Disconnecting");
            }
            else
            {
              if ((inChar & 128) == 128)
              {
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
                //mySerial.println("More");
              }
              else
              {
                NextLengthByte = false;
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
              }
            }
          }
        }
        MQTT.lengthLocal = MQTT.length;
        //mySerial.println(MQTT.length);
        if (MQTT.TCP_Flag == true)
        {
          MQTT.printMessageType(ReceivedMessageType);
          MQTT.index = 0L;
          uint32_t a = 0;
          while ((MQTT.length-- > 0) && (Serial.available()))
          {
            MQTT.inputString[uint32_t(MQTT.index++)] = (char)Serial.read();

            delay(1);

          }
          //mySerial.println(" ");
          if (ReceivedMessageType == CONNACK)
          {
            MQTT.ConnectionAcknowledgement = MQTT.inputString[0] * 256 + MQTT.inputString[1];
            if (MQTT.ConnectionAcknowledgement == 0)
            {
              MQTT.MQTT_Flag = true;
              MQTT.OnConnect();
              digitalWrite(4, HIGH);


            }

            MQTT.printConnectAck(MQTT.ConnectionAcknowledgement);
            // MQTT.OnConnect();
          }
          else if (ReceivedMessageType == PUBLISH)
          {
            uint32_t TopicLength = (MQTT.inputString[0]) * 256 + (MQTT.inputString[1]);
            //mySerial.print("Topic : '");
            MQTT.PublishIndex = 0;
            for (uint32_t iter = 2; iter < TopicLength + 2; iter++)
            {
              //mySerial.print(MQTT.inputString[iter]);
              MQTT.Topic[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Topic[MQTT.PublishIndex] = 0;
            //mySerial.print("' Message :'");
            MQTT.TopicLength = MQTT.PublishIndex;

            MQTT.PublishIndex = 0;
            uint32_t MessageSTART = TopicLength + 2UL;
            int MessageID = 0;
            if (QoS != 0)
            {
              MessageSTART += 2;
              MessageID = MQTT.inputString[TopicLength + 2UL] * 256 + MQTT.inputString[TopicLength + 3UL];
            }
            for (uint32_t iter = (MessageSTART); iter < (MQTT.lengthLocal); iter++)
            {
              //mySerial.print(MQTT.inputString[iter]);
              MQTT.Message[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Message[MQTT.PublishIndex] = 0;
            //mySerial.println("'");
            MQTT.MessageLength = MQTT.PublishIndex;
            if (QoS == 1)
            {
              MQTT.publishACK(MessageID);
            }
            else if (QoS == 2)
            {
              MQTT.publishREC(MessageID);
            }
            MQTT.OnMessage(MQTT.Topic, MQTT.TopicLength, MQTT.Message, MQTT.MessageLength);
            MQTT.MessageFlag = true;
          }
          else if (ReceivedMessageType == PUBREC)
          {
            //mySerial.print("Message ID :");
            MQTT.publishREL(0, MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            //mySerial.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if (ReceivedMessageType == PUBREL)
          {
            //mySerial.print("Message ID :");
            MQTT.publishCOMP(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            //mySerial.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if ((ReceivedMessageType == PUBACK) || (ReceivedMessageType == PUBCOMP) || (ReceivedMessageType == SUBACK) || (ReceivedMessageType == UNSUBACK))
          {
            //mySerial.print("Message ID :");
            //mySerial.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
          }
          else if (ReceivedMessageType == PINGREQ) // pingreq is sent by the clinet to the server then how come it is a Recieved Message?
          {
            MQTT.TCP_Flag = false;
            MQTT.pingFlag = false;
            //mySerial.println("Disconnecting");
            //"AT+CIPSHUT\r\n"
            MQTT.sendATreply(4, ".", 4000) ;
            MQTT.modemStatus = 0;
          }
        }
      }
      else if ((inChar = 13) || (inChar == 10))
      {
      }
      else
      {
        //mySerial.print("Received :Unknown Message Type :");
        //mySerial.println(inChar);
      }
    }
  }
}

int TextCheck(char*A, int B) {
  char myChar[15];
  int result = strstr(A, strcpy_P(myChar, (char*)pgm_read_word(&(string_table[B]))));
  return result;
}
