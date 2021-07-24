/* ARDUINO -> HIVEMQ
   Author: Lazaro Borges Silva

   PIR01      ->  MQTT    publishes timestamp
   BUZZER     <-  MQTT    activates buzzer on command
   PUSHBUTTON ->  MQTT    publishes timestamp (may activate something later idk)
   LED01      <-  MQTT    activates a lamp on command
   mqTTheartbeat working properly



*/


#include      <SPI.h>
#include      <stdlib.h>
#include      <Ethernet.h>
#include      <PubSubClient.h>
#include      <EthernetUdp.h>
//#include      <Bounce.h>
#include      <elapsedMillis.h>
//#include    <dht.h>

//dht         DHT;
EthernetUDP   Udp;

#define       CLIENT_ID             "Arduino"
#define       MQTT_HEARTBEAT_TIME1  5000                         // in milliseconds
#define       MQTT_HEARTBEAT_TIME2  500                          // in milliseconds
#define       MQTT_HEARTBEAT_MSG1   "( (♥) )"
#define       MQTT_HEARTBEAT_MSG2   "   ♥   "
#define       BUZZER_PIN            31
#define       LED_PIN               35
//#define     DHT11_PIN             49
#define       PUSHBUTTON_PIN        33
#define       PIR01_VCC             7
#define       PIR01_PIN             6
#define       PIR01_GND             5

//int         lasttemp = 11;                                    //initialize variable for last temperature
//int         lasthumi = 10;                                    //initialize variable for last humidity
//int         temp = 1;
//int         humi = 2;
long          lastReconnectAttempt = 0;                         // for the mqTT watchdog
const int     buttonPin = PUSHBUTTON_PIN;                       // the pin that the pushbutton is attached to
int           buttonPushCounter = 0;                            // counter for the number of button presses
int           buttonState = 0;                                  // current state of the button
int           lastButtonState = 0;                              // previous state of the button
const int     PIR01Pin = PIR01_PIN;                             // the pin that the pushbutton is attached to
int           PIR01Counter = 0;                                 // counter for the number of button presses
int           PIR01State = 0;                                   // current state of the button
int           lastPIR01State = 0;                               // previous state of the button
unsigned int  localPort = 8888; 
const char    timeServer[] = "time.nist.gov";                   // time.nist.gov NTP server
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
boolean       reconnect();

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

class myClass {
  public:
    int    inthour;
    int    intmin;
    int    intsec;
    char strhour;
    char strmin;
    char strsec;
    String payload;                       // saves the payload received
    String buzzer;                        // saves the buzzer state
    String lastBuzzer = "buzzeroff";      // initializes the buzzer state
    String PIR01;                         // saves the PIR01 state
    String temp;                          // saves the temperature
    String humi;                          // saves the humidity
    String LED01;                         // saves the LED01 state
    String pushbutton;                    // saves the pushbutton state
    String timestamp[8];                  // saves the timestamp
    int callbackread;                     // ?

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
EthernetClient  ethClient;                                              // Ethernet and MQTT related objects
PubSubClient    mqttClient;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("HiveMq: [");                                            // prints the topic of the received payload
  Serial.print(topic);
  Serial.print("] ");
  char    payload_char[length];                                         // for reading the payload
  for (int i = 0; i < (length); i++) {                                  // reads and prints the payload
    Serial.print((char)payload[i]);
    payload_char[i]     = payload[i];
  }
  payload_char[length] = 0;                                             // it needs a null to end the string
  Serial.println();
  myObj.payload = payload_char;                                         // saves the payload
  //BUZZER
  readBuzzer(); 
  //PIR01
  readPIR01();
}

////////////////////////////////////////////////////////////////////////////////SETUP/////
void setup() {
  Serial.begin(9600);                                                           // Starts serial port
  Serial.println();                                                             // To keep it neat
  Serial.println("----------------------------------");                         // prints stuff
  Serial.println("__________________________________");
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
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {                                                 // TODO: Iterate this to keep trying to connect
    Serial.print("SERIAL: ");
    Serial.println("Failed to configure Ethernet using DHCP");                    // TODO: Add a hardware watchdog
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.print("SERIAL: ");
      Serial.println("Ethernet shield was not found.");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.print("SERIAL: ");
      Serial.println("Ethernet cable is not connected.");
    }
    while (true) {
    delay(1);                                                                     // TODO: add hardware watchdog
    }
  }
  Udp.begin(localPort);
 // getTime();
  Serial.print("SERIAL: ");
  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());                                             // prints local IP address
  ///////////////////////////////////////////////
  setupBuzzer(BUZZER_PIN);
  setupLED(LED_PIN);
  subscribeToAll();
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
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        Serial.print("SERIAL: ");
        Serial.println("mqTT status: ON ");
        mqttClient.publish("lcabs1993/arduino", "Reconnected!");
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

void setupLED(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void setBuzzer(bool x) {
  digitalWrite(BUZZER_PIN, x);
}

void setLED(bool x) {
  digitalWrite(LED_PIN, x);
}

void pubPushbutton() {
  buttonState = digitalRead(buttonPin);                             // read state
  if (buttonState != lastButtonState) {                             // has it changed?
    if (buttonState == HIGH) {                                      // if changed to on
      setBuzzer(true);                                              // turns buzzer on
      buttonPushCounter++;                                          // increment counter
      Serial.print("SERIAL: ");                                     // print stuff
      Serial.print("Pushbutton: ON ");
      Serial.print("  |  ");
      Serial.print("Pushbutton Counter: ");
      Serial.println(buttonPushCounter);
      mqttClient.publish("lcabs1993/arduino/pushbutton", "on");     // TODO: publish timestamp
    } else {                                                        // when it goes low
      setBuzzer(false);                                             // turns buzzer off
      Serial.print("SERIAL: ");                                     // print stuff
      Serial.print("Pushbutton: OFF");
      Serial.print("  |  ");
      Serial.print("Pushbutton Counter: ");
      Serial.println(buttonPushCounter);
      mqttClient.publish("lcabs1993/arduino/pushbutton", "off");    // maybe delete this?
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
      PIR01Counter++;                                               //  increment counter
      Serial.print("SERIAL: ");                                     //  print stuff
      Serial.print("PIR Sensor: ON ");
      Serial.print("  |  ");
      Serial.print("PIR01 Counter: ");
      Serial.println(PIR01Counter);
      mqttClient.publish("lcabs1993/arduino/PIR01", "PIR01on");     //  TODO: publish timestamp
    } else {                                                        //  when it goes low
      setLED(false);                                                //  turns LED off
      Serial.print("SERIAL: ");                                     //  print stuff
      Serial.print("PIR Sensor: OFF");
      Serial.print("  |  ");
      Serial.print("PIR01 Counter: ");
      Serial.println(PIR01Counter);
      mqttClient.publish("lcabs1993/arduino/PIR01", "PIR01off");    //  maybe delete this later?
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
  mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
  mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
  mqttClient.subscribe("lcabs1993/arduino/PIR01");
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

void readPIR01(){
  if ((((myObj.payload) == "PIR01on"))) {               // if a command is received to turn the buzzer on
    myObj.PIR01 = (myObj.payload);                      // saves the new command
  }
  if ((((myObj.payload) == "PIR01off"))) {              // if a command is received to turn the buzzer on
    myObj.PIR01 = (myObj.payload);                      // saves the new command
  }
}

void getTime(){
    sendNTPpacket(timeServer);                            // send an NTP packet to a time server
    delay(1000);                                          // wait to see if a reply is available
    if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears - 10800;
       
    Serial.println();    
    Serial.print("int timestamp:  ");                     // UTC is the time at Greenwich Meridian (GMT)
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
    Serial.print("String timestamp: ");
    Serial.print(myObj.strhour);
    Serial.print(":");
    Serial.print(myObj.strmin);
    Serial.print(":");
    Serial.print(myObj.strsec);
    Serial.println();
  }
}
