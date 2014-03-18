
#include <SPI.h>       
#include <DMD.h>       
#include <TimerOne.h>  
#include "Arial_black_16.h"
#include <stdlib.h>
#include <Ethernet.h>
#include <Wire.h>
#include "RTClib.h"
#include "DHT.h"

RTC_DS1307 rtc;

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
DMD dmd(DISPLAYS_ACROSS, DISPLAYS_DOWN);

#define DHTTYPE DHT22 
#define DHTPIN 2
DHT dht(DHTPIN, DHTTYPE);

#define HW_ID        "1"
#define WEBSITE      "alert.arduinosbs.com"
#define WEBPAGE      "/get_message/"
#define IDLE_TIMEOUT_MS  3000 

byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
IPAddress ip;
EthernetClient client;

char text_buffer[10];

boolean        reading      = false;  //TRUE while the GET request is being received
String         get_request  = "";     //Holds the GET request

void ScanDMD()
{ 
  dmd.scanDisplayBySPI();
}

void setup(void)
{
   Serial.begin(9600);
   //initialize TimerOne's interrupt/CPU usage used to scan and refresh the display
   Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
   Timer1.attachInterrupt( ScanDMD );   //attach the Timer1 interrupt to ScanDMD which goes to dmd.scanDisplayBySPI()
   //clear/init the DMD pixels held in RAM
   dmd.clearScreen( true );   //true is normal (all pixels off), false is negative (all pixels on)
   
   dht.begin();   
   Wire.begin();
   rtc.begin();

   if (! rtc.isrunning()) {
      rtc.adjust(DateTime(__DATE__, __TIME__));
   }
  
   if (Ethernet.begin(mac) == 0) {
   }
   // print your local IP address:
   Serial.print("My IP address: ");
   ip = Ethernet.localIP();
   for (byte thisByte = 0; thisByte < 4; thisByte++) {
     // print the value of each byte of the IP address:
     Serial.print(ip[thisByte], DEC);
     Serial.print("."); 
   }
   Serial.println();
}

void loop(void)
{  
   setup_DMD();  
   make_get_request();
   process_response();
   display_environmental_info();
   show_time();
}

void setup_DMD(){
     dmd.clearScreen( true );
   dmd.selectFont(Arial_Black_16);
}

void display_marquee(String &message)
{
     char buffer[message.length()+1];
     message.toCharArray(buffer, message.length()+1); 
     dmd.clearScreen( true );
     dmd.drawMarquee(buffer,message.length(),(32*DISPLAYS_ACROSS)-1,0);
     long start=millis();
     long timer=start;
     boolean ret=false;
     while(!ret){
       if ((timer+100) < millis()) {
         ret=dmd.stepMarquee(-1,0);
         timer=millis();
       }
     }
}

void show_time(){
   DateTime now = rtc.now();   
   String hour; 
   String minute;
   byte byteHour = now.hour();
   byte byteMinute = now.minute();

   if (byteHour < 10)
   {
     hour = "0";
     hour += String(byteHour,DEC);
   } else
   {
     hour = String(byteHour,DEC);
   }
     
  if (byteMinute < 10)
  {
    minute = "0";
    minute += String(byteMinute,DEC); 
  } else
  {
     minute = String(byteMinute,DEC); 
  }
  
  dmd.drawChar(  0,  0, hour[0], GRAPHICS_NORMAL );
  dmd.drawChar(  7,  0, hour[1], GRAPHICS_NORMAL );
  dmd.drawChar( 17,  0, minute[0], GRAPHICS_NORMAL );
  dmd.drawChar( 25,  0, minute[1], GRAPHICS_NORMAL );
   
  for (int i=0;i<31;i++)
  {      
    dmd.drawChar( 15,  3, '.', GRAPHICS_OR     );   // clock colon overlay on
    delay( 1000 );
    dmd.drawChar( 15,  3, '.', GRAPHICS_NOR    );   // clock colon overlay off
    delay( 1000 );
  }
}

void display_environmental_info(){
   byte b;
   DateTime now = rtc.now();

   String month;
   String day;
   String year;
   byte byteMonth = now.month();
   byte byteDay = now.day();
   
  year = String(now.year(),DEC);

  if (byteMonth < 10)
  {
    month = "0";
    month += String(byteMonth,DEC); 
  } else
  {
    month = String(byteMonth,DEC); 
  }
  
  if (byteDay < 10)
  {
    day = "0";
    day += String(byteDay,DEC); 
  } else
  {
    day = String(byteDay,DEC); 
  }

  String marqueeText;
  marqueeText = day;
  marqueeText += '/';
  marqueeText += month;
  marqueeText += '/';
  marqueeText += year;
  marqueeText += ' ';
  marqueeText += dtostrf(dht.readTemperature(), 2, 2, text_buffer);
  marqueeText += '*';
  marqueeText += 'C';
  marqueeText += ' ';

  marqueeText += ' ';
  marqueeText += 'H';
  marqueeText += ':';
  marqueeText += dtostrf(dht.readHumidity(), 2, 0, text_buffer);
  marqueeText += '%';
  display_marquee(marqueeText);
}

void process_response(){
         /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (client.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    boolean currentLineIsBlank = false;
    get_request           = "";     
    while (client.available()) {   
       char c = client.read();
       if(reading && c == '\n') 
        { Serial.println("1");
          reading = false;  
          parseGetRequest(get_request);
          break;
        }        

        if(reading){ 
          get_request += c;
         }      
       if (reading && c=='\n')
       {
          break; 
       }       
       if (c == '\n' && currentLineIsBlank)  {
         reading = true; // Found the body of the server response, start reading
       }
       if (c == '\n') {
         currentLineIsBlank = true;
       } 
       else if (c != '\r') {
         currentLineIsBlank = false;
       }
    }
  }
  client.stop();
}
  

void parseGetRequest(String &str) {
  Serial.print(F("Parsing this string:"));
  Serial.println(str);  
  int   buzzer_state    = str[0] - '0';
  
  if (buzzer_state == 1)
      tone(3, 1000, 1000);
      
  String new_str = str.substring(1);
  display_marquee(new_str);
}


void make_get_request(){
    if (client.connect(WEBSITE, 80)) {
    Serial.println(F("connected"));
    client.print(F("GET ")); 
    client.print(WEBPAGE);
    client.print(HW_ID);
    client.print(F(" HTTP/1.1\r\n"));
    client.print("Host: ");
    client.print(WEBSITE);
    client.print(F("\r\n"));
    client.println();
    } else {
    // If you didn't get a connection to the server:
    Serial.println(F("connection failed"));
    }
  }


