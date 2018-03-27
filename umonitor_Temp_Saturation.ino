#include <SoftwareSerial.h>

//libs for temp
#include <OneWire.h>
#include <DallasTemperature.h>

//Inserting libraries for pulse oximeter
#include <Arduino.h>
#include <math.h>
#include <Wire.h>

#include "MAX30100.h"



MAX30100* pulseOxymeter;



//#define esp FALSE //comment out to remove esp msgs

//*-- Hardware Serial
#define _baudrate 9600

//*-- Software Serial
//
#define _rxpin      9
#define _txpin      10

//variables for temp
// Data wire is plugged into pin 3 on the Arduino
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);


SoftwareSerial esp( _rxpin, _txpin ); // RX, TX

//*-- IoT Information
#define SSID "########"
#define PASS "########"
#define server "server url" // Localhost IP Address: 184.106.153.149


String GET;

void reset() {

esp.println("AT+CWMODE=3");

delay(500);

if(esp.find("OK") ) {
  Serial.println("Module Reset");
  }

   esp.println("AT+RST");
  delay(2000);
    if(esp.find("OK") ) {
      Serial.println("Module Reset");

      esp.println("AT+CIPMUX=1");
      if(esp.find("OK") ) {
        Serial.println("Module Mul");

     
      }else{
        Serial.println("Module Mul failed");
      }
    }else{
        Serial.println("Module Reset failed");
    }
}
boolean connectWiFi()
{
  //Connect to Router with AT+CWJAP="SSID","Password";
  // Check if connected with AT+CWJAP?
  String cmd="AT+CWJAP=\""; // Join accespoint
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  sendesp(cmd);
  delay(4000);
  if(esp.find("OK"))
  {
    Serial.println("RECEIVED: OK");
    return true;
  }
  else
  {
    Serial.println("RECEIVED: Error CWJAP");
    return false;
  }

 /* cmd = "AT+CIPMUX=0";// Set Single connection
  sendesp( cmd );
  if( esp.find( "Error") )
  {
    Serial.print( "RECEIVED: Error" );
    return false;
  }*/
}

void sendesp(String cmd)
{
  Serial.print("SEND: ");
  esp.println(cmd);
  Serial.println(cmd);
}


void setup() {
  Wire.begin();
  
  //pulseOxymeter = new MAX30100( DEFAULT_OPERATING_MODE, DEFAULT_SAMPLING_RATE, DEFAULT_LED_PULSE_WIDTH, DEFAULT_IR_LED_CURRENT, true, true );
  pulseOxymeter = new MAX30100();
  
  //Serial.begin(115200);
   Serial.begin( _baudrate );
  esp.begin( _baudrate );
    pinMode(2, OUTPUT);
    sensors.begin();
  sendesp("AT");
  delay(500);
  if(esp.find("OK"))
  {
    Serial.println("RECEIVED: OK\nData ready to sent!");
}

}
void loop() {
float value_sa;
  pulseoxymeter_t result = pulseOxymeter->update();

    while(true){
      delay(5);
      result = pulseOxymeter->update();
  

      if( result.pulseDetected == true )
      {
        Serial.println("BEAT");
    
        Serial.print( "BPM: " );
        Serial.print( result.heartBPM );
        Serial.print( " | " );

  
        Serial.print( "SaO2: " );
        value_sa=result.SaO2;
        Serial.print(value_sa );
        Serial.println( "%" );

        Serial.print("{P2|BPM|255,40,0|");
        Serial.print(result.heartBPM);
        Serial.print("|SaO2|0,0,255|");
        Serial.print(result.SaO2);
        Serial.println("}");
        break;
      }
    }
 

  reset();
  if(connectWiFi()){
    Serial.println("Connected");
   }else{
    Serial.println("Connect failed");
   }

 int value_bed = 9;
 float value_temp;
 
  //value_humid = analogRead(sensor_humid);
 
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  value_temp=sensors.getTempCByIndex(0);
  Serial.println(value_temp);
 String temp =String(value_temp);// turn integer to string
 //String temp ="28.00";
 //String sa="92.00";
  String bed= String(value_bed);// turn integer to string
  String sa= String(value_sa);// turn float to string
  updateTS(bed,temp,sa);
  delay(3000); //
}
//----- update the  Thingspeak string with 3 values
void updateTS( String B,String T,String S)
{
  // ESP8266 Client
  String cmd = "AT+CIPSTART=4,\"TCP\",\"";// Setup TCP connection
  cmd += server;
  cmd += "\",80";
  sendesp(cmd);
  //delay(2000);
  if( esp.find( "Error" ) )
  {
    Serial.print( "RECEIVED: Error\nExit1" );
    return;
  }
   GET= "GET /firebaseTest.php";
  cmd = GET + "?b=" + B +"&a="+ T +"&s="+ S +" HTTP/1.1\r\nHost: "+server+"\r\n\r\n\r\n";
  esp.print( "AT+CIPSEND=4," );
  esp.println( cmd.length() );
  if(esp.find( ">" ) )
  {
    //esp.print(">");
    esp.print(cmd);
    Serial.print(cmd);

    if( esp.find("OK") )
      {
        Serial.println( "RECEIVED IN: OK" );
         while (esp.available()) {

            String tmpResp = esp.readString();

            Serial.println(tmpResp);

        }
      }
   else
       {
    Serial.println( "RECEIVED: Error\nExit2" );
      }
  }
  else
  {
    sendesp( "AT+CIPCLOSE" );//close TCP connection
  }
 
}




