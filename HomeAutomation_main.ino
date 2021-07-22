#include <SPI.h>
#include <stdlib.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EthernetUdp.h>
//#include <dht.h>

//dht DHT;
EthernetUDP Udp;

#define CLIENT_ID "Arduino"
#define BUZZER_PIN 31
#define LED_PIN 35
//#define DHT11_PIN 49
#define PUSHBUTTON_PIN 33
#define PIR01_VCC 7
#define PIR01_PIN 6
#define PIR01_GND 5

//int lasttemp = 11;              //initialize variable for last temperature
//int lasthumi = 10;              //initialize variable for last humidity
//int temp = 1;
//int humi = 2; 
long lastReconnectAttempt = 0;
const int  buttonPin = PUSHBUTTON_PIN;    // the pin that the pushbutton is attached to
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

const int  PIR01Pin = PIR01_PIN;    // the pin that the pushbutton is attached to
int PIR01Counter = 0;   // counter for the number of button presses
int PIR01State = 0;         // current state of the button
int lastPIR01State = 0;     // previous state of the button
///////////////////////////////////////////////////////////////////////DEFINES/////////

// Function prototypes
void callback(char* topic, byte* payload, unsigned int length);
void setupBuzzer(int pin);
void setBuzzer(bool x);
void setupLED(int pin);
void subscribeToAll();
//void readDHT11();
void setupPushbutton(int pin);
void pubPushbutton();
void readBuzzer();
void pubPIR01();
boolean reconnect();

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

class myClass{
  public:
    String buzzer;
    String lastBuzzer = "buzzeroff";
    String PIR01;
    String temp;
    String humi;
    String LED01;
    String pushbutton;
    int callbackread;
    
    //Setters
    void setBuzzer(int buzzerStatus){
    buzzer = buzzerStatus;
    }
    void setLED01(int LED01Status){
    LED01 = LED01Status;
    }
    
    //Getters
    String getBuzzer(){
    return buzzer;  
    }
    String getPIR01(){
    return PIR01; 
    }
};

myClass myObj;

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient;

String myString;

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("HiveMq: [");
  Serial.print(topic);
  Serial.print("] ");
  
  char    payload_char[length];
  char    topic_char;
  int     msg_int[length];
  String  msg_string[length];
  byte    msg_byte[length];
  
  for (int i=0;i<(length);i++) {
    Serial.print((char)payload[i]);
    payload_char[i]     = payload[i];
  }     
//    Serial.println(topic);
    topic_char = topic;
    payload_char[length] = 0;
    Serial.println();
    Serial.print("CALLBACK: payload_char: ");
    Serial.println(payload_char);
    myObj.PIR01 = payload_char;
    Serial.print("CALLBACK: myObj.PIR01: ");
    Serial.println(myObj.PIR01);

    if ((myObj.PIR01) == "on"){
      Serial.print("topic//|: ");
      Serial.println("CALLBACK: PASSOU ALGUEM AÊW!");
    }

    Serial.print("CALLBACK before test: myObj.buzzer: ");
    Serial.println(myObj.buzzer);

                                  ////////////IFS FUNCIONANDO ABAIXO
      if ((((myObj.PIR01) == "buzzeron"))){    // if a command is received to turn the buzzer on
      myObj.buzzer = (myObj.PIR01);        // saves the new command
 //     setBuzzer(1);             // turns buzzer on/off        - NOT WORKING
} 
      if ((((myObj.PIR01) == "buzzeroff"))){    // if a command is received to turn the buzzer on
      myObj.buzzer = (myObj.PIR01);        // saves the new command
   //   setBuzzer(0);             // turns buzzer on/off        - NOT WORKING
} 
    Serial.print("CALLBACK after test:  myObj.buzzer: ");
    Serial.println(myObj.buzzer);
  Serial.println();
  }
  

////////////////////////////////////////////////////////////////////////////////SETUP/////
void setup() {
      Serial.begin(9600);                                                       // Start serial port 
      Serial.println("----------------------------------");                     // print stuff
      Serial.println("__________________________________");
      Serial.print("SERIAL: "); Serial.println("<Port ON>");
      
      pinMode(PIR01_VCC,OUTPUT);                                                // sets some pinModes on the board
      pinMode(PIR01_GND,OUTPUT);
      digitalWrite(PIR01_VCC,HIGH);
      digitalWrite(PIR01_GND,LOW);
      pinMode(PIR01_PIN,INPUT);
      lastReconnectAttempt = 0;
////////////////////////////////////////////
 mqttClient.setClient(ethClient);                                               // starts the client
 mqttClient.setServer("broker.hivemq.com",1883);                                // connects to the server
 mqttClient.setCallback(callback);                                              // defines the callback function
////////////////////////////////////////////
  // start the Ethernet connection:  
  Serial.print("SERIAL: ");
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.print("SERIAL: ");
    Serial.println("Failed to configure Ethernet using DHCP");  // TODO: Add a hardware watchdog
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.print("SERIAL: ");
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.print("SERIAL: ");
      Serial.println("Ethernet cable is not connected.");
    }
    while (true) {
      delay(1);                                               // no point in carrying on, so do nothing forevermore:
    }
  }
  Serial.print("SERIAL: ");
  Serial.print("Ethernet IP: ");  
  Serial.println(Ethernet.localIP());                         // prints local IP address
///////////////////////////////////////////////
setupBuzzer(BUZZER_PIN);
setupLED(LED_PIN);
subscribeToAll();
}
/////////////////////////////////////////////////////////////////////////////////LOOP/////

void loop() {
mqttClient.loop();
//readDHT11();
pubPushbutton(); //TODO: se mudou, printa nova timestamp
pubPIR01();
readBuzzer();
//atualizaOLED(); //TODO: atualiza OLED com novos valores

  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        Serial.print("SERIAL: ");
        Serial.println("mqTT status: ON ");
        mqttClient.publish("lcabs1993/arduino","Reconnected!");
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

void setupBuzzer(int pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin,LOW);
}

void setupLED(int pin){
  pinMode(pin, OUTPUT);
  digitalWrite(pin,LOW);
}

void setBuzzer(bool x){
  digitalWrite(BUZZER_PIN,x);
}

void setLED(bool x){
  digitalWrite(LED_PIN,x);
}

void pubPushbutton(){   
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
      mqttClient.publish("lcabs1993/arduino/pushbutton","on");      // TODO: publish timestamp
    } else {                                                        // when it goes low
      setBuzzer(false);                                             // turns buzzer off
      Serial.print("SERIAL: ");                                     // print stuff
      Serial.print("Pushbutton: OFF");                  
      Serial.print("  |  ");
      Serial.print("Pushbutton Counter: ");
      Serial.println(buttonPushCounter);
      mqttClient.publish("lcabs1993/arduino/pushbutton","off");     // maybe delete this?
    }
  }
    lastButtonState = buttonState;                                  // updates last state
}                                

void pubPIR01(){
  PIR01State = digitalRead(PIR01_PIN);                              //  read state
  if (PIR01State != lastPIR01State) {                               //  has it changed?
    if (PIR01State == HIGH) {                                       //  if changed to on
      setLED(true);                                                 //  turns LED on
      PIR01Counter++;                                               //  increment counter
      Serial.print("SERIAL: ");                                     //  print stuff
      Serial.print("PIR Sensor: ON ");
      Serial.print("  |  ");
      Serial.print("PIR01 Counter: ");
      Serial.print(PIR01Counter);
      Serial.print("  |  ");
      Serial.print("DEBUG: myObj.PIR01: ");
      Serial.println(myObj.PIR01);
 //     debug();
      mqttClient.publish("lcabs1993/arduino/PIR01","on");           //  TODO: publish timestamp
    } else {                                                        //  when it goes low
      setLED(false);                                                //  turns LED off
      Serial.print("SERIAL: ");                                     //  print stuff
      Serial.print("PIR Sensor: OFF");  
      Serial.print("  |  ");
      Serial.print("PIR01 Counter: ");
      Serial.println(PIR01Counter);
      mqttClient.publish("lcabs1993/arduino/PIR01","off");          //  maybe delete this later?
    }
  }
    lastPIR01State = PIR01State;                                    //  updates last state
  }
/*
void debug(){
    Serial.print("infunctionDEBUG: myObj.PIR01: ");
    Serial.println(myObj.PIR01);
    if ((myObj.PIR01) == "on"){
      Serial.println("infunctionDEBUG: PASSOU ALGUEM AÊ!");
    }
  }
*/
boolean reconnect() {
  if (mqttClient.connect(CLIENT_ID)) {
    mqttClient.publish("lcabs1993/arduino","Hello world!");     // Once connected, publish an announcement...
    subscribeToAll();                                           // ... and subscribe
  }
  return mqttClient.connected();
}

void subscribeToAll(){
 mqttClient.subscribe("lcabs1993");
 mqttClient.subscribe("lcabs1993/arduino");
 mqttClient.subscribe("lcabs1993/arduino/buzzer");
 mqttClient.subscribe("lcabs1993/arduino/pushbutton");
 mqttClient.subscribe("lcabs1993/arduino/led");
 mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
 mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
 mqttClient.subscribe("lcabs1993/arduino/PIR01");
}

void readBuzzer(){
  if (myObj.buzzer != myObj.lastBuzzer) {           // if the state has changed
if (myObj.buzzer == "buzzeron") {                   
      setBuzzer(true);                              // turns buzzer on
    } else {                                        
setBuzzer(false);                                   // or off
    }
  }
    myObj.lastBuzzer = myObj.buzzer;                // and update the last state
}
