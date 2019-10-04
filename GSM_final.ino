#include "SparkFunLSM6DS3.h"
#include "SPI.h"
#include "GSM_MQTT.h"
#include <avr/pgmspace.h>
#include <avr/wdt.h>

void GSM_MQTT::AutoConnect(void)
{
  connect("ubi0010", 1, 1, "sammy", "sammy@SenzMate", 1, 1, 1, 0, "SenzMate/D2S/ubi00010", "client disconnected");
}


String msg ="IMU:-0.0,0.2,-1.0,3.6,-3.8,-0.5";
GSM_MQTT MQTT(20);


//***********************************************************************************************************************************
String AccelData(String msg);
void publishData(String msg);
//***********************************************************************************************************************************
unsigned long prevMillis,currMillis = 0; 
bool wdtFlag = true;

void setup()
{

  prevMillis = millis();
  currMillis = millis();
  
  
  pinMode(7,OUTPUT);
  pinMode(15,OUTPUT);
  
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
 
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(15, HIGH);
  digitalWrite(7, LOW); 
  
  wdt_disable();

  MQTT.begin(); //takes a bit of time to do this 

}


void loop()
{
  digitalWrite(7, LOW);
 
  digitalWrite(4,HIGH); //light up the LED
  currMillis = millis();
  
  MQTT.processing();
  delay(10);
  //msg ="";
  if(MQTT.available()&&(currMillis - prevMillis) > 100){
    delay(10);
    publishData(msg);
  }
  wdt_reset();
}
//################################################################################################################

void publishData(String msg){
    if (wdtFlag){
      wdt_enable(WDTO_8S);
      wdtFlag = false;
    }
  //if(MQTT.available()){
    char sendBuff[50];     
    msg.toCharArray(sendBuff,50);
    MQTT.publish(0, 1, 1,100, "SenzMate/D2S/ubi00010",sendBuff);  
    prevMillis = millis();
  //  wdt_reset();
 // } 
}

//################################################################################################################
void GSM_MQTT::OnConnect(void)
{
   publish(0, 1, 1, 0, "SenzMate/D2S/ubi00010/onConnect", "Hello, I am connected");
}

//################################################################################################################
void GSM_MQTT::OnMessage(char *Topic, int TopicLength, char *Message, int MessageLength)
{
}


