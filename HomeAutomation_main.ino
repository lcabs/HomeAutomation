/* ARDUINO -> HIVEMQ
   Author: Lazaro Borges Silva

   PIR01      ->  MQTT    publishes timestamp
   BUZZER     <-  MQTT    activates buzzer on command
   PUSHBUTTON ->  MQTT    publishes timestamp (may activate something later idk)
   LED01      <-  MQTT    activates a lamp on command
   mqTTheartbeat working properly

*/


#include      <SPI.h>
#include      <Wire.h>
#include      <stdlib.h>
#include      <Ethernet.h>
#include      <PubSubClient.h>
#include      <EthernetUdp.h>
//#include      <Bounce.h>
#include      <elapsedMillis.h>
//#include    <dht.h>
#include      <Adafruit_GFX.h>
#include      <Adafruit_SSD1306.h>

//dht         DHT;


#define       CLIENT_ID             "Arduino"
#define       MQTT_HEARTBEAT_TIME1  5000                         // in milliseconds
#define       MQTT_HEARTBEAT_TIME2  500                          // in milliseconds
#define       MQTT_HEARTBEAT_MSG1   "( (♥) )"
#define       MQTT_HEARTBEAT_MSG2   "   ♥   "
#define       BUZZER_PIN            31
#define       LED_PIN               35
//#define     DHT11_PIN             49
#define       PUSHBUTTON_PIN        33
#define       PIR01_VCC             5
#define       PIR01_PIN             6
#define       PIR01_GND             7
#define       LAMP_PIN              37
#define       SCREEN_WIDTH 128 // OLED display width, in pixels
#define       SCREEN_HEIGHT 64 // OLED display height, in pixels

// DISPLAY-RELATED DEFINES //

#define       SCREEN_WIDTH 128 // OLED display width, in pixels
#define       SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

// endDisplay




//int         lasttemp = 11;                                    //initialize variable for last temperature
//int         lasthumi = 10;                                    //initialize variable for last humidity
//int         temp = 1;
//int         humi = 2;
int           HOST_PORT   = 80;
char          HTTP_METHOD[] = "GET";
const char    HOST_NAME[] = "maker.ifttt.com";
//String        PATH_NAME   = "/trigger/EVENT-NAME/with/key/YOUR-KEY";
char         PATH_NAME[]   = "/trigger/PIR01_on/with/key/kfqX_IGsvoojtN7eoFZWRDtXiQqhc_LygmLnv6f1Wib";
char         queryString[] = "?value1=localPort&value2=1808";

char         queryString1[] = "?value1=";
char         queryString2[] = "&value2=";
char         queryString3[] = "&value3=";


 
      
int           y;
long          lastReconnectAttempt = 0;                         // for the mqTT watchdog
const int     buttonPin = PUSHBUTTON_PIN;                       // the pin that the pushbutton is attached to
int           buttonPushCounter = 0;                            // counter for the number of button presses
int           buttonState = 0;                                  // current state of the button
int           lastButtonState = 0;                              // previous state of the button
const int     PIR01Pin = PIR01_PIN;                             // the pin that the pushbutton is attached to
unsigned int  PIR01Counter = 0;                                 // counter for the number of button presses
int           PIR01State = 0;                                   // current state of the button
int           lastPIR01State = 0;                               // previous state of the button
unsigned int  localPort = 8888; 
const char    timeServer[] = "time.nist.gov";                   // time.nist.gov NTP server
const char    apiServer[] = "api.pushingbox.com";               //YOUR SERVER
const char    webhooksServer[] = "https://maker.ifttt.com/trigger/PIR01_on/with/key/kfqX_IGsvoojtN7eoFZWRDtXiQqhc_LygmLnv6f1Wib";               //YOUR SERVER
const int     NTP_PACKET_SIZE = 48;                             // NTP time stamp is in the first 48 bytes of the message
byte          packetBuffer[NTP_PACKET_SIZE];                    // buffer to hold incoming and outgoing packets
elapsedMillis mqTT_TimeBetweenHeartbeats1;                      // counts time between "systole"  (in milliseconds)
elapsedMillis mqTT_TimeBetweenHeartbeats2;                      // counts time between "diastole" (in milliseconds)

// Function prototypes
void          callback(char* topic, byte* payload, unsigned int length);
void          setupBuzzer(int pin);
void          setBuzzer(bool x);
void          setupLED(int pin);
void          subscribeToAll();
//void        readDHT11();
void          setupPushbutton(int pin);
void          pubPushbutton();
void          readBuzzer();
void          readPIR01();
void          pubPIR01();
void          mqTTheartbeat();                                  // function to track and publish heartbeats
void          updateJSON();
boolean       reconnect();

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

class PIR{
    public:
    unsigned int     Counter = 0;          // how many times has this sensor been activated
    int     Position;             // there might be many PIR sensors involved
    bool    PIRstatus;            // on or off
  //  int     timestamp[1][6] ;     // logs the timestamp of all events
    
};

class Button{
    public:
    int     Counter = 0;          // how many times has this sensor been activated
    int     Position;             // there might be many PIR sensors involved
    bool    Pushbuttonstatus;            // on or off
  //  int     timestamp[1][6] ;     // logs the timestamp of all events
    
};


class myClass {
  public:
    int     hourminsec[3];
    int     inthour;
    int     intmin;
    int     intsec;
    char    strhour;
    char    strmin;
    char    strsec;
    String  payload;                       // saves the payload received
    String  lamp;
    String  lastLamp = "lampoff";
    String  buzzer;                        // saves the buzzer state
    String  lastBuzzer = "buzzeroff";      // initializes the buzzer state
    String  PIR01;                         // saves the PIR01 state
    String  temp;                          // saves the temperature
    String  humi;                          // saves the humidity
    String  LED01;                         // saves the LED01 state
    String  pushbutton;                    // saves the pushbutton state
    String  timestamp[8];                  // saves the timestamp
    char    chartimestamp[8];
    int     callbackread;                     // ?

    //Setters
    void setBuzzer(int buzzerStatus) {
      buzzer = buzzerStatus;
    }
    void setLED01(int LED01Status) {
      LED01 = LED01Status;
    }
    //Getters
    String getBuzzer() {
      return buzzer;
    }
    String getPIR01() {
      return PIR01;
    }
};

myClass         myObj;
EthernetUDP     Udp;
PIR             PIR01;
Button          Pushbutton;
EthernetClient  ethClient;                                              // Ethernet and MQTT related objects
EthernetClient  apiClient;
EthernetClient  webhooksClient;
PubSubClient    mqttClient;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("HiveMq: [");                                            // prints the topic of the received payload
  Serial.print(topic);
  Serial.print("] ");
  char    payload_char[length];                                         // for reading the payload
  for (int i = 0; i < (length); i++) {                                  // reads and prints the payload
    Serial.print((char)payload[i]);
    payload_char[i]     = payload[i];
//    displaywrite(payload[i]);
  }
  payload_char[length] = 0;                                             // it needs a null to end the string
  Serial.println();
  myObj.payload = payload_char;                                         // saves the payload
//  displaywrite(payload_char);
//  displaywrite("\n");
  //BUZZER
  readBuzzer(); 
  //PIR01
  readPIR01();
  //LAMP
  readLamp();
  //UPDATE
  readUpdate();
}

////////////////////////////////////////////////////////////////////////////////SETUP/////
void setup() {

/////////// DISPLAY

setupDisplay();


//////////// endDISPLAY
  
  Serial.begin(9600);                                                           // Starts serial port
  Serial.println();                                                             // To keep it neat
  Serial.println("----------------------------------");                         // prints stuff
  displaywrite("---------------------");
  Serial.println("__________________________________");
  displaywrite("SERIAL: <Port ON>\n");
  Serial.print("SERIAL: "); Serial.println("<Port ON>");
  pinMode(PIR01_VCC, OUTPUT);                                                   // sets some pinModes on the board
  pinMode(PIR01_GND, OUTPUT);
  digitalWrite(PIR01_VCC, HIGH);
  digitalWrite(PIR01_GND, LOW);
  pinMode(PIR01_PIN, INPUT);
  lastReconnectAttempt = 0;
  ////////////////////////////////////////////
  mqttClient.setClient(ethClient);                                                // starts the client
  mqttClient.setServer("broker.hivemq.com", 1883);                                // connects to the server
  mqttClient.setCallback(callback);                                               // defines the callback function
  ////////////////////////////////////////////
  Serial.print("SERIAL: ");                                                       // starts the Ethernet connection
  displaywrite("Initializing Ethernet with DHCP...\n");
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {                                                 // TODO: Iterate this to keep trying to connect
    Serial.print("SERIAL: ");
    displaywrite("Failed to configure Ethernet using DHCP");
    Serial.println("Failed to configure Ethernet using DHCP");                    // TODO: Add a hardware watchdog
    buzzer_fail();
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.print("SERIAL: ");
      displaywrite("Ethernet shield was not found.\n");
      Serial.println("Ethernet shield was not found.");
      buzzer_fail();
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.print("SERIAL: ");
      displaywrite("Ethernet cable is not connected.\n");
      Serial.println("Ethernet cable is not connected.");
      buzzer_fail();
    }
    while (true) {
    delay(1);                                                                     // TODO: add hardware watchdog
    }
  }
  Udp.begin(localPort);
 // getTime();
  Serial.print("SERIAL: ");
  displaywrite("Ethernet ON\n");
  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());                                             // prints local IP address
//  displaywrite(*Ethernet.localIP());
  ///////////////////////////////////////////////
  setupBuzzer(BUZZER_PIN);
  setupLamp(LAMP_PIN);
  setupLED(LED_PIN);
  subscribeToAll();
//  testdrawrect();
//  delay(1000);

}
/////////////////////////////////////////////////////////////////////////////////LOOP/////

void loop() {
  mqttClient.loop();
  mqTTheartbeat();
  //readDHT11();
  pubPushbutton();                                           //TODO: se mudou, printa nova timestamp
  pubPIR01();
  //atualizaOLED();                                          //TODO: atualiza OLED com novos valores

  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      Serial.println("// MQTT FAILED! ");
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        Serial.print("SERIAL: ");
        Serial.println("mqTT status: ON ");
        display.write("mqTT status: ON \n");
        mqttClient.publish("lcabs1993/arduino", "Reconnected!");
        display.write("Reconnected!\n");
        display.clearDisplay();
        display.display();
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqttClient.loop();      // Client connected
  }
}

/*void readDHT11(){
  int dht[2] ;
  int chk = DHT.read11(DHT11_PIN);  //reads DHT11
  temp = DHT.temperature;     // reads current state
  humi = DHT.humidity;
  if (temp == lasttemp){
    } else {
  char msgbuffer[10];         //initializes a message buffer
  mqttClient.publish("lcabs1993/arduino/dht11/temp",itoa(temp, msgbuffer, 10));   //publishes DHT11 data to hiveMQ
  Serial.print("Temp: ");     //prints to serial
  Serial.println(temp);
  Serial.print("Last Temp: ");     //prints to serial
  Serial.println(lasttemp);
      }
  if (humi == lasthumi){
    } else {
  char msgbuffer[10];         //initializes a message buffer
  mqttClient.publish("lcabs1993/arduino/dht11/humidade",itoa(humi, msgbuffer, 10));
  //  Serial.print("Humi: ");
  //  Serial.println(humi);
    }
  lasttemp = temp;
  lasthumi = humi;
  }

*/

void setupBuzzer(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void setupLamp(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void setupLED(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void setBuzzer(bool x) {
  digitalWrite(BUZZER_PIN, x);
}

void setLamp(bool x) {
  digitalWrite(LAMP_PIN, x);
}

void setLED(bool x) {
  digitalWrite(LED_PIN, x);
}

void pubPushbutton() {
  buttonState = digitalRead(buttonPin);                             // read state
  if (buttonState != lastButtonState) {                             // has it changed?
    if (buttonState == HIGH) {                                      // if changed to on
      getTime();
      setBuzzer(true);                                              // turns buzzer on
      Pushbutton.Counter = Pushbutton.Counter + 1;                  // increment counter
      Serial.print("SERIAL: ");                                     // print stuff
      Serial.print("Pushbutton: ON ");
      Serial.print("  |  ");
      Serial.print("Pushbutton.Counter: ");
      Serial.println(Pushbutton.Counter);
      pubTimestamp("lcabs1993/arduino/pushbutton");                  // publishes timestamp
    } else {                                                        // when it goes low
      setBuzzer(false);                                             // turns buzzer off
      Serial.print("SERIAL: ");                                     // print stuff
      Serial.print("Pushbutton: OFF");
      Serial.print("  |  ");
      Serial.print("Pushbutton.Counter: ");
      Serial.println(Pushbutton.Counter);
    }
  }
  lastButtonState = buttonState;                                    // updates last state
}

void pubPIR01() {
  PIR01State = digitalRead(PIR01_PIN);                              //  read state
  if (PIR01State != lastPIR01State) {                               //  has it changed?
    if (PIR01State == HIGH) {                                       //  if changed to on
      setLED(true);                                                 //  turns LED on
      getTime();
    //  buzzer_success();
      PIR01.Counter = PIR01.Counter + 1;                            //  increment counter
      Serial.print("SERIAL: ");                                     //  print stuff
      Serial.print("PIR Sensor: ON ");
      Serial.print("  |  ");
      Serial.print("PIR01 Counter: ");
      Serial.println(PIR01.Counter);
      pubTimestamp("lcabs1993/arduino/PIR01");                      //  publishes timestamp
      send_to_spreadsheet();
      send_to_webhooks();
    } else {                                                        //  when it goes low
      setLED(false);                                                //  turns LED off
      Serial.print("SERIAL: ");                                     //  print stuff
      Serial.print("PIR Sensor: OFF");
      Serial.print("  |  ");
      Serial.print("PIR01 Counter: ");
      Serial.println(PIR01.Counter);
    }
  }
  lastPIR01State = PIR01State;                                      //  updates last state
}

boolean reconnect() {
  if (mqttClient.connect(CLIENT_ID)) {
    mqttClient.publish("lcabs1993/arduino", "Hello world!");        // Once connected, publish an announcement...
    subscribeToAll();                                               // ... and subscribe
  }
  return mqttClient.connected();
}

void mqTTheartbeat(){                                               // Publishes a heartbeat every MQTT_HEARTBEAT_TIME milliseconds
  if (mqTT_TimeBetweenHeartbeats1 >= MQTT_HEARTBEAT_TIME1){
    mqttClient.publish("lcabs1993/arduino/heartbeat", MQTT_HEARTBEAT_MSG1);
      if (mqTT_TimeBetweenHeartbeats2 >= (MQTT_HEARTBEAT_TIME1 + MQTT_HEARTBEAT_TIME2)){
    mqttClient.publish("lcabs1993/arduino/heartbeat", MQTT_HEARTBEAT_MSG2);
    mqTT_TimeBetweenHeartbeats1 = mqTT_TimeBetweenHeartbeats1 - (MQTT_HEARTBEAT_TIME1 + MQTT_HEARTBEAT_TIME2);
    mqTT_TimeBetweenHeartbeats2 = mqTT_TimeBetweenHeartbeats2 - (MQTT_HEARTBEAT_TIME1 + MQTT_HEARTBEAT_TIME2);
    }  
    }

}


void subscribeToAll() {
  mqttClient.subscribe("lcabs1993");
  mqttClient.subscribe("lcabs1993/arduino");
  mqttClient.subscribe("lcabs1993/arduino/buzzer");
  mqttClient.subscribe("lcabs1993/arduino/pushbutton");
  mqttClient.subscribe("lcabs1993/arduino/led");
  mqttClient.subscribe("lcabs1993/arduino/lamp");
  mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
  mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
  mqttClient.subscribe("lcabs1993/arduino/PIR01");
  mqttClient.subscribe("lcabs1993/arduino/requestupdate");
}

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}


void readBuzzer() {
  if ((((myObj.payload) == "buzzeron"))) {              // if a command is received to turn the buzzer on
    myObj.buzzer = (myObj.payload);                     // saves the new command
  }
  if ((((myObj.payload) == "buzzeroff"))) {             // if a command is received to turn the buzzer off
    myObj.buzzer = (myObj.payload);                     // saves the new command
  }
  if (myObj.buzzer != myObj.lastBuzzer) {               // if the state has changed
    if (myObj.buzzer == "buzzeron") {
      setBuzzer(true);                                  // turns buzzer on
    } else {
      setBuzzer(false);                                 // or off
    }
  }
  myObj.lastBuzzer = myObj.buzzer;                      // and update the last state
}

void readLamp() {
  if ((((myObj.payload) == "lampon"))) {                // if a command is received to turn the buzzer on
    myObj.lamp = (myObj.payload);                       // saves the new command
  }
  if ((((myObj.payload) == "lampoff"))) {               // if a command is received to turn the buzzer off
    myObj.lamp = (myObj.payload);                       // saves the new command
  }
  if (myObj.lamp != myObj.lastLamp) {                   // if the state has changed
    if (myObj.lamp == "lampon") {
      setLamp(false);                                   // turns lamp on (inverted logic)
    } else {
      setLamp(true);                                    // or off
    }
  }
  myObj.lastLamp = myObj.lamp;                          // and update the last state
}

void readPIR01(){
  if ((((myObj.payload) == "PIR01on"))) {               // if a command is received to turn the buzzer on
    myObj.PIR01 = (myObj.payload);                      // saves the new command
  }
  if ((((myObj.payload) == "PIR01off"))) {              // if a command is received to turn the buzzer on
    myObj.PIR01 = (myObj.payload);                      // saves the new command
  }
}

void readUpdate(){

//  testdrawline();      // Draw many lines
//  testdrawrect();      // Draw rectangles (outlines)
//  testfillrect();      // Draw rectangles (filled)
//  testdrawcircle();    // Draw circles (outlines)
//  testfillcircle();    // Draw circles (filled)
//  testdrawroundrect(); // Draw rounded rectangles (outlines)
//  testfillroundrect(); // Draw rounded rectangles (filled)
//  testdrawtriangle();  // Draw triangles (outlines)
//  testfilltriangle();  // Draw triangles (filled)
//  testdrawchar();      // Draw characters of the default font
//  testdrawstyles();    // Draw 'stylized' characters
//  testscrolltext();    // Draw scrolling text
//  testdrawbitmap();    // Draw a small bitmap image
  
  if ((((myObj.payload) == "updateall"))) {              
      mqTTupdateall();                                   // updates all topics on myObj
      displaywrite(myObj.chartimestamp);
      displaywrite("\n");
  }  
      if ((((myObj.payload) == "drawline"))) {              
      testdrawline();                                   
  } 
    if ((((myObj.payload) == "drawrect"))) {              
      testdrawrect();                                   
  }  
      if ((((myObj.payload) == "fillrect"))) {              
      testfillrect();                                   
  }
      if ((((myObj.payload) == "drawcircle"))) {              
      testdrawcircle();                                   
  }
        if ((((myObj.payload) == "fillcircle"))) {              
      testfillcircle();                                   
  }
        if ((((myObj.payload) == "drawroundrect"))) {              
      testdrawroundrect();                                   
  }
        if ((((myObj.payload) == "drawtriangle"))) {              
      testdrawtriangle();                                   
  }
        if ((((myObj.payload) == "filltriangle"))) {              
      testfilltriangle();                                   
  }
          if ((((myObj.payload) == "drawchar"))) {              
      testdrawchar();                                   
  }
          if ((((myObj.payload) == "drawstyles"))) {              
      testdrawstyles();                                   
  }
          if ((((myObj.payload) == "scrolltext"))) {              
      testscrolltext();                                   
  }
          if ((((myObj.payload) == "drawbitmap"))) {              
      testdrawbitmap();                                   
  }
  
}

void mqTTupdateall(){
    pubTimestamp("lcabs1993/arduino/PIR01");
//  mqttClient.publish("lcabs1993/arduino/buzzer", "UpdateBuzzer");
//  mqttClient.publish("lcabs1993/arduino/heartbeat", myObj.);
//  mqttClient.publish("lcabs1993/arduino/heartbeat", myObj.);
//  mqttClient.publish("lcabs1993/arduino/heartbeat", myObj.);
}

void getTime(){
    sendNTPpacket(timeServer);                            // send an NTP packet to a time server
    delay(1000);                                          // TODO : implement non blocking delay
    if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears - 10800;
       
    Serial.println();    
    Serial.print("Event at: ");                           // UTC is the time at Greenwich Meridian (GMT)
    myObj.inthour = ((epoch  % 86400L) / 3600);
    Serial.print(myObj.inthour);                          // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {                     // In the first 10 minutes of each hour, we'll want a leading '0'
    }
    myObj.intmin = ((epoch  % 3600) / 60);
    Serial.print(myObj.intmin);                           // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {                              // In the first 10 seconds of each minute, we'll want a leading '0'
    }
    myObj.intsec = (epoch % 60);

    Serial.println(myObj.intsec); 
  }
}

void pubTimestamp(char* outTopic){
      char charforhour[4];
      char charformin[4];
      char charforsec[4];
      char timestamp[8];
      itoa(myObj.inthour, charforhour, 10);
      itoa(myObj.intmin, charformin, 10);
      itoa(myObj.intsec, charforsec, 10);
      timestamp[0] =  charforhour[0];
      myObj.chartimestamp[0] = timestamp[0];    
      timestamp[1] =  charforhour[1];    
      myObj.chartimestamp[1] = timestamp[1];    
      timestamp[2] =  ':';       
      myObj.chartimestamp[2] = timestamp[2];    
      if (myObj.intmin < 10){
      timestamp[3] =  '0';
      myObj.chartimestamp[3] = timestamp[3];
      timestamp[4] =  charformin[0];
      myObj.chartimestamp[4] = timestamp[4];
      } else {
      timestamp[3] =  charformin[0];
      myObj.chartimestamp[3] = timestamp[3];
      timestamp[4] =  charformin[1];
      myObj.chartimestamp[4] = timestamp[4];
      }          
      timestamp[5] =  ':'; 
      myObj.chartimestamp[5] = timestamp[5];   
      if (myObj.intsec < 10){
      timestamp[6] =  '0';
      myObj.chartimestamp[6] = timestamp[6];
      timestamp[7] =  charforsec[0];
      myObj.chartimestamp[7] = timestamp[7];
      } else {
      timestamp[6] =  charforsec[0];
      myObj.chartimestamp[6] = timestamp[6];
      timestamp[7] =  charforsec[1];
      myObj.chartimestamp[7] = timestamp[7];
      }     
      myObj.chartimestamp[8] = timestamp[8];

      y = y  + (1*10);
      if (y >= (6*10)){  
      y = 10;
      display.clearDisplay();
      display.display();
      }      
      display.setCursor(0,0);
      display.write("Ultimos movimentos: ");
      display.display();
      char cont[5];
      itoa(PIR01.Counter,cont,10);
      cont[5] = 0;
      display.setCursor(0,y);
      display.write("#");
     // display.write(cont);
      display.write(" ");
      timestamp[8] =  '\0';
   /*   for (int i=0; i=8; i++){
      Serial.print("timestamp[");
      Serial.print(i);
      Serial.print("]: ");      

      }*/
      Serial.print("timestamp[8]: ");
      Serial.println(timestamp[8]);
      display.write(timestamp);
      display.display();
      mqttClient.publish(outTopic, timestamp);      
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//OLED DISPLAY STUFF

/*
  Hardware Connections (Breakoutboard to Arduino):
  -5V = 5V (3.3V is allowed)
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected
 
  The MAX30105 Breakout can handle 5V or 3.3V I2C logic. We recommend powering the board with 5V
  but it will also run at 3.3V.
*/

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

 void setupDisplay(){     //// CLEAN THIS PULL

   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //Ive changed the address //already chill
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
 // display.display();
//  delay(2000); // Pause for 2 seconds

  // Clear the buffer


  // Draw a single pixel in white
//  display.drawPixel(10, 10, SSD1306_WHITE);
  loadingscreen();
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
//  display.display();
//  delay(2000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

//  testdrawline();      // Draw many lines

//  testdrawrect();      // Draw rectangles (outlines)

//  testfillrect();      // Draw rectangles (filled)

//  testdrawcircle();    // Draw circles (outlines)

//  testfillcircle();    // Draw circles (filled)

//  testdrawroundrect(); // Draw rounded rectangles (outlines)

//  testfillroundrect(); // Draw rounded rectangles (filled)

//  testdrawtriangle();  // Draw triangles (outlines)

//  testfilltriangle();  // Draw triangles (filled)

//  testdrawchar();      // Draw characters of the default font

//  testdrawstyles();    // Draw 'stylized' characters

//  testscrolltext();    // Draw scrolling text

//  testdrawbitmap();    // Draw a small bitmap image

  // Invert and restore display, pausing in-between
//  display.invertDisplay(true);
//  delay(1000);
//  display.invertDisplay(false);
// delay(1000);

//  testanimate(logo_bmp, LOGO_WIDTH, LOGO_HEIGHT); // Animate bitmaps

  
 }

void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000); // Pause for 2 seconds
}

void testdrawrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  display.clearDisplay();
  display.display();
}

void testfillrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=3) {
    // The INVERSE color is used so rectangles alternate white/black
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, SSD1306_INVERSE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }

  delay(2000);
}

void testdrawcircle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfillcircle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=3) {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, SSD1306_INVERSE);
    display.display(); // Update screen with each newly-drawn circle
    delay(1);
  }

  delay(2000);
}

void testdrawroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfillroundrect(void) {
  display.clearDisplay();

  for(int16_t i=0; i<display.height()/2-2; i+=2) {
    // The INVERSE color is used so round-rects alternate white/black
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i,
      display.height()/4, SSD1306_INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testdrawtriangle(void) {
  display.clearDisplay();

  for(int16_t i=0; i<max(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void testfilltriangle(void) {
  display.clearDisplay();

  for(int16_t i=max(display.width(),display.height())/2; i>0; i-=5) {
    // The INVERSE color is used so triangles alternate white/black
    display.fillTriangle(
      display.width()/2  , display.height()/2-i,
      display.width()/2-i, display.height()/2+i,
      display.width()/2+i, display.height()/2+i, SSD1306_INVERSE);
    display.display();
    delay(1);
  }

  delay(2000);
}

void loadingscreen(){
//  char txt[];      
  display.clearDisplay();
  display.setTextSize(1);               // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.cp437(true);                  // Use full 256 char 'Code Page 437' font
//  display.write(txt);
//  display.display();
//  delay(3000);
//  display.clearDisplay();
//  display.display();
}

void displaywrite(char txt[])
{
  display.write(txt);
  display.display();  
}


void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}

void testdrawstyles(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));

  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.println(3.141592);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(2000);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}

void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
}

void send_to_spreadsheet(){

     if (apiClient.connect(apiServer, 80)) {
    // Make a HTTP request:
    apiClient.print("GET /pushingbox?devid=vD5E764B0EE806EA&allowed_members=");     //YOUR URL
    apiClient.print(PIR01.Counter);
    Serial.println("HTTP request sent.");
    if(false)
    {
//      client.print('1');
//      Serial.print('1');
    }
    else
    {
//      client.print('0');
    }
    
//    apiClient.print("&Member_ID=");
//    for(int s=0;s<4;s++)
//                  {
//                    client.print(rfid.uid.uidByte[s]);
//                                  
//                  }
    apiClient.print(" ");      //SPACE BEFORE HTTP/1.1
    apiClient.print("HTTP/1.1");
    apiClient.println();
    apiClient.println("Host: api.pushingbox.com");
    apiClient.println("Connection: close");
    apiClient.println();
  } 
   else {
    // if you didn't get a connection to the server:
    Serial.println("API connection failed");
  }
  }


  void send_to_webhooks()   //CONNECTING WITH WEBHOOKS
 {
  // connect to web server on port 80:
  if(webhooksClient.connect(HOST_NAME, HOST_PORT)) {
    // if connected:
    Serial.println("WEBHOOKS CONNECTED");
    // make a HTTP request:
    // send HTTP header
  //  updateJSON();
    webhooksClient.print("GET ");
    webhooksClient.print(PATH_NAME);
//    webhooksClient.print(queryString);
    webhooksClient.print(queryString1);      
    webhooksClient.print(PIR01.Counter);
    webhooksClient.print(queryString2);
    webhooksClient.print(localPort);
    webhooksClient.print(queryString3);
    webhooksClient.print(localPort);

    webhooksClient.println(" HTTP/1.1");
    webhooksClient.print("Host: ");
    webhooksClient.println(String(HOST_NAME));
    webhooksClient.println("Connection: close");
    webhooksClient.println(); // end HTTP header

    while(webhooksClient.connected()) {
      if(webhooksClient.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = webhooksClient.read();
//        Serial.println(")-----  WEBHOOKS -----(");
        Serial.print(c);
        
 }
    }
  } else {
    Serial.println("WEBHOOKS CONNECTION FAILED.");
    }
 }

void buzzer_fail(){
for (int i=0; i<3; i++){
setBuzzer(true);
delay(500);
setBuzzer(false);
delay(500);
}
}

void buzzer_success(){
for (int i=0; i<3; i++){
setBuzzer(true);
delay(50);
setBuzzer(false);
delay(50);
}
}






