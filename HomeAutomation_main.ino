#include <SPI.h>
#include <stdlib.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EthernetUdp.h>
//#include <dht.h>

//dht DHT;
EthernetUDP Udp;

#define CLIENT_ID "Arduino"
#define BUZZER_PIN 51
#define LED_PIN 45
//#define DHT11_PIN 49
#define PUSHBUTTON_PIN 47
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
void subscribeToAll();
//void readDHT11();
void setupPushbutton(int pin);
void pubPushbutton();
void pubPIR01();
boolean reconnect();
void copyA(int* src, int* dst, int len);
void copyB(int* src, int* dst, int len);
void copyC(int* src, int* dst, int len);





byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

class myClass{
  public:
    String buzzer;
    String PIR01;
    String temp;
    String humi;
    String LED01;
    
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
  //  char    msg_char = payload;
  //  int     msg_int = payload;
  //  String  msg_string = payload;  
  
  char    msg_char[length];
  int     msg_int[length];
  String  msg_string[length];
  byte    msg_byte[length];
  
  for (int i=0;i<(length);i++) {
    Serial.print((char)payload[i]);
    msg_char[i]     = payload[i];
 /*   Serial.println(); 
    Serial.print("|msg_char[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(msg_char[i]);
    Serial.print("   |   ");
    Serial.print("payload[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.print(payload[i]);
    Serial.print("| length: ");
    Serial.print(length);
    Serial.println();*/
  }     
    msg_char[length] = 0;
    Serial.println();
/*
    Serial.print("after loop -> msg_char: ");
    Serial.println(msg_char);*/
    
/* if (topic == "lcabs1993/arduino/temp"){
      myObj.temp = "1";//msg_char;
 }
 if (topic == "lcabs1993/arduino/humi"){
      myObj.humi = "2";//msg_char;
 }

 if (topic == "lcabs1993/arduino/PIR01"){
  myObj.PIR01 = "3";//msg_char;
 }

 if (topic == "lcabs1993/arduino/buzzer"){
      myObj.buzzer = "4";//msg_char;
 }*/

  

  
/*
  Serial.print("/* debug");
  Serial.println();

  Serial.print("| myString.String(): ");
  Serial.print(myString.charAt(1));
  Serial.println();

  Serial.print("| msg_char: ");
  Serial.print(msg_char);
  Serial.println();

  Serial.print("| msg_int: ");
//  Serial.print(msg_int);
  Serial.println();

  Serial.print("| msg_string: ");
 // Serial.print(msg_string);
  Serial.println();

  Serial.print("| msg_byte: ");
//  Serial.print(msg_byte);
  Serial.println();
  */
  
  }
  
void setupLED(){
  int pin;
  pinMode(pin, OUTPUT);
}

////////////////////////////////////////////////////////////////////////////////SETUP/////
void setup() {
      Serial.begin(9600); // Start serial port 
      Serial.println("----------------------------------");
      Serial.println("__________________________________");
      Serial.print("SERIAL: "); Serial.println("<Port ON>");
      pinMode(PIR01_VCC,OUTPUT);
      pinMode(PIR01_GND,OUTPUT);
      digitalWrite(PIR01_VCC,HIGH);
      digitalWrite(PIR01_GND,LOW);
      pinMode(PIR01_PIN,INPUT);
      lastReconnectAttempt = 0;
////////////////////////////////////////////
 mqttClient.setClient(ethClient);
 mqttClient.setServer("broker.hivemq.com",1883);
 mqttClient.setCallback(callback);
////////////////////////////////////////////
  // start the Ethernet connection:  
  Serial.print("SERIAL: ");
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.print("SERIAL: ");
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.print("SERIAL: ");
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.print("SERIAL: ");
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("SERIAL: ");
  Serial.print("Ethernet IP: ");
  Serial.println(Ethernet.localIP());
///////////////////////////////////////////////
setupBuzzer(BUZZER_PIN);
setupLED();
subscribeToAll();

}
/////////////////////////////////////////////////////////////////////////////////LOOP/////

void loop() {
mqttClient.loop();
//readDHT11();
pubPushbutton(); //TODO: se mudou, printa nova timestamp
pubPIR01();

//atualizaOLED(); //TODO: atualiza OLED com novos valores







//delay(500);
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
}

void pubPushbutton(){
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
  digitalWrite(BUZZER_PIN,buttonState);
  digitalWrite(LED_PIN,buttonState);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      Serial.print("SERIAL: ");
      Serial.print("Pushbutton: ON ");
      Serial.print("  |  ");
      Serial.print("Counter: ");
      Serial.print(buttonPushCounter);
      Serial.print("  |  ");
      Serial.print("myObj.temp: ");
      Serial.println(myObj.temp);
      mqttClient.publish("lcabs1993/arduino","on");






      
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.print("SERIAL: ");
      Serial.print("Pushbutton: OFF");
      Serial.print("  |  ");
      Serial.print("Counter: ");
      Serial.println(buttonPushCounter);
      mqttClient.publish("lcabs1993/arduino","off");
    }

  }
    lastButtonState = buttonState;
}

void pubPIR01(){
    // read the pushbutton input pin:
  PIR01State = digitalRead(PIR01_PIN);
  // compare the buttonState to its previous state
  if (PIR01State != lastPIR01State) {
    // if the state has changed, increment the counter
    if (PIR01State == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      PIR01Counter++;
  //    debug();
      Serial.print("SERIAL: ");
      Serial.print("PIR Sensor: ON ");
      Serial.print("  |  ");
      Serial.print("Counter: ");
      Serial.println(PIR01Counter);
  
      mqttClient.publish("lcabs1993/arduino/PIR01","on");

    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.print("SERIAL: ");
      Serial.print("PIR Sensor: OFF");
      Serial.print("  |  ");
      Serial.print("Counter: ");
      Serial.println(PIR01Counter);
      mqttClient.publish("lcabs1993/arduino/PIR01","off");
    }
  }
    lastPIR01State = PIR01State;
  }
  
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
 mqttClient.subscribe("lcabs1993/arduino/led");
 mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
 mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
 mqttClient.subscribe("lcabs1993/arduino/PIR01");
}


