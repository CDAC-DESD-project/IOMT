
#include <SoftwareSerial.h>//to convert the pin 2 and 3 as serial connector


const byte LDR = 2;
volatile int val =10;     //data to be send on thingspeak
String value = "{\"value\": "+(String)val+"}";

String _SSID = "\"YOUR SSID\"";//Enter your wifi ssid 
String PWD = "\"YOUR WIFI PASSWORD\"";//Enter your wifi password
String varId = "YOUR VARIABLE ID ON UBIDOTS"; // Your variable ID created on ubidots
String token = "YOUR TOKEN NUMBER IN API CREDENTIALS" ;// Your Token under api credentials

SoftwareSerial ser(4,5); // RX, TX

// this runs once
void setup() {

      pinMode(LDR,INPUT);

      Serial.begin(9600);// enable debug serial

      ser.begin(115200);// enable software serial

      ser.println("AT");//check if esp8266 set

      if(ser.find("OK"))//check for response

        Serial.println("Done");

      while(!ser.find("OK"))
      {

          ser.println("AT+CWJAP="+_SSID+","+PWD);
        
        }

      Serial.println("Connected to wifi");

      func(value);
}

// the loop
void loop() {

  if(digitalRead(LDR)==HIGH)     //connected ldr, when ldr is blocked and value is decremented and sent to cloud.
    {
      val--;//decrement the count value if Ldr input is low.
      value = "{\"value\": "+(String)val+"}";
      func(value);
    }

}

void func(String value){

    Serial.println(val);

    atRST("AT+RST");

    connectUBIDOTS("AT+CIPSTART=\"TCP\",\"50.23.124.68\",80"); // IP ADDRESS OF UBIDOTS IS 50.23.124.68 

    String cmd = "POST /api/v1.6/variables/" + varId + "/values HTTP/1.1\r\n" /*POST request  of ubidots to send the data*/
               + "X-Auth-Token: " + token + "\r\n"
               + "Host: things.ubidots.com\r\n"
               + "Connection: close\r\n"
               + "Content-Type: application/json\r\n"
               + "Content-Length:"+14+"\r\n\n"
               + value + "\r\n";

    cipSEND(String(cmd.length()));

    ser.print(cmd);

    delay(5000);

    Serial.println("data successfully send ");
}

void atRST(String rst){       /*this function checks that esp8266 is working or not it returns OK when everything is fine :)*/

  while(!ser.find("OK"))
    ser.println(rst);

}

void connectUBIDOTS(String website){ /*connection to the website of ubidots*/
  
  while(!ser.find("OK"))
    ser.println(website);

  Serial.println("Connection established");

}

void cipSEND(String length){  /*it request to send to number of charcter to be send (length of POST request)*/

  String cipsend = "AT+CIPSEND="+length;

  while(!ser.find(">"))
    ser.println(cipsend);

  Serial.println("LENGTH OF DATA TO BE SEND IS :"+length);

}
