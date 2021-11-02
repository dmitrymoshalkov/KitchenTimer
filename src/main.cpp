#include <Arduino.h>
#include <Ethernet.h>
#include <avr/wdt.h>



#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient

#include "timer-api.h"



//#define NDEBUG

#define CHECK_PULSES_INTERVAL 4
#define ALARM_PIN 2
#define ETHERNET_RESET_PIN  4
#define ARDUINO_RESET_PIN  7
#define MQTTUPDATEINTERVAL 300

int volatile newco = -1;
uint16_t interruptCounter = 0;

bool currAlarmstate = false;


uint8_t mqttUnaviable = 0;

byte mac[] = {
  0x00, 0xAB, 0x1C, 0xAC, 0x12, 0x07
};
//byte ip[] = { 10, 11, 13, 11 };
//byte gateway[] = { 10, 11, 13, 1 };
//byte subnet[] = { 255, 255, 255, 0 };


String MQTTserver = "192.168.100.124";
String MQTTuser = "openhabian";
String MQTTpwd = "damp19ax";
String MQTTtopic = "home/kitchen/timer";
String MQTTClientID = "kitchentimer";


long Day=0;
int Hour =0;
int Minute=0;
int Second=0;
int SecondStamp=0;
int Once=0;


#include "services.h"

void uptime();

// 750 - 800

void pop ()
{
  newco++;
}

void timer_handle_interrupts(int timer) {

        #ifdef NDEBUG
          Serial.print("Frequency: ");
          Serial.println(newco, DEC);
         
        #endif



        if ( newco >= 500 &&  newco <= 800 )
        { 
            if (!currAlarmstate)
            {
              currAlarmstate = true;
              sendDataToMQTT();

                        #ifdef NDEBUG
                        Serial.println("Срабатывание таймера.");
                        #endif
            }
        }
        else
        {
            if (currAlarmstate)
            {
              currAlarmstate = false;
              sendDataToMQTT();

                        #ifdef NDEBUG
                        Serial.println("Таймер выключен.");
                        #endif
            }
        }
               
      newco = 0;


    interruptCounter++;

        #ifdef NDEBUG
          Serial.print("interruptCounter: ");
          Serial.println(interruptCounter, DEC);
         
        #endif

    if( interruptCounter == MQTTUPDATEINTERVAL )
    {
 
        sendDataToMQTT();

       interruptCounter = 0;
    }

}



void setup() {

digitalWrite(ARDUINO_RESET_PIN,HIGH);
delay(200);   
pinMode(ARDUINO_RESET_PIN, OUTPUT);     


    #ifdef NDEBUG
        Serial.begin(9600);
    //    Serial.println(co,DEC);
    #endif


    //set MQTT connect state
    digitalWrite(6, LOW);


    for (uint8_t i=0; i<=4; i++)
    {

      for (int y=0; y<=255; y++)
      {
        analogWrite(5, y);
        delay (5);
      }
      delay (5);

      for (int z=255; z>=0; z--)
      {
        analogWrite(5, z);
        delay (5);
      }
      delay (5);
    }


// Reset the W5500 module
  pinMode(ETHERNET_RESET_PIN, OUTPUT);
  digitalWrite(ETHERNET_RESET_PIN, LOW);
  delay(800); //100
  digitalWrite(ETHERNET_RESET_PIN, HIGH);
  delay(800); //100



if ( Ethernet.begin(mac) == 0)
{

  //Enable watchdog timer
  // wdt_enable(WDTO_8S);


#ifdef NDEBUG

    Serial.println("Resetting");
#endif

  resetBoard();


}

#ifdef NDEBUG

    Serial.println(DisplayAddress(Ethernet.localIP()));
#endif


  pinMode(ALARM_PIN, INPUT_PULLUP);


  attachInterrupt(digitalPinToInterrupt(ALARM_PIN),pop,FALLING);

timer_init_ISR_1Hz(TIMER_DEFAULT);

sendDataToMQTT();

}

void loop() {
  switch (Ethernet.maintain()) {
      case 1:
        //renewed fail

             resetBoard();
        break;

      case 2:
        //renewed success

        break;

      case 3:
        //rebind fail

             resetBoard();
        break;

      case 4:
        //rebind success

        break;

      default:
        //nothing happened
        break;
    }


    uptime();
}



//************************ Uptime Code - Makes a count of the total up time since last start ****************//
//It will work for any main loop's, that loop moret han twice a second: not good for long delays etc
void uptime(){
//** Checks For a Second Change *****//
if(millis()%1000<=500&&Once==0){
SecondStamp=1;
Once=1;
}
//** Makes Sure Second Count doesnt happen more than once a Second **//
if(millis()%1000>500){
Once=0;
}




                         if(SecondStamp==1){
                           Second++;
                           SecondStamp=0;
                           //print_Uptime();

                         if (Second==60){
                          Minute++;
                          Second=0;
                          if (Minute==60){
                          Minute=0;
                          Hour++;

                         if (Hour==24){
                          Hour=0;
                          Day++;
                         }
                         }
                         }
                         }


}

