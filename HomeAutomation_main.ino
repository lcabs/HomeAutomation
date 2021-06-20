#include <SPI.h>
#include <stdlib.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <dht.h>

dht DHT;

#define CLIENT_ID "Arduino"
#define BUZZER_PIN 53
#define LED_PIN 51
#define DHT11_PIN 49

int temp = 1;
int humi = 2; 
int pin = LED_PIN;
long lastReconnectAttempt = 0;
  int lasttemp =3;              //initialize buffers for last state
  int lasthumi =4;
///////////////////////////////////////////////////////////////////////DEFINES/////////

// Function prototypes
void callback(char* topic, byte* payload, unsigned int length);



void setupPushbutton(int pin);

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient;

/////////////////////////////////////////////////////////START CALLBACK
/////////////////////////////////////////////////////////--------------

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

/////////////////////////////////////ACENDE O LED///////
if      (payload[0]=="1"){
            digitalWrite(LED_PIN,HIGH);
            Serial.println("LED:1");
}
if (payload[0]=="0"){
            digitalWrite(LED_PIN,LOW);
            Serial.println("LED:0");
     }
///////////////////////////////////////PRINTA TEMP E HUMI do broker//////

//if (topic == "lcabs1993/arduino/dht11/temp"){ //SE ALGO FOR POSTADO NO TOPICO
//  Serial.println("Temp: ");     //prints to serial
//  Serial.println(temp);
//  Serial.println("Humi: "); 
//  Serial.println(humi);
  
  
}
////////////////////////////////////////////////////////-------------
/////////////////////////////////////////////////////////END CALLBACK

void setupLED(int pin){
  pinMode(pin, OUTPUT);
}

void turnsLEDon(int pin){
digitalWrite(pin,HIGH);
}

void turnsLEDoff(int pin){
digitalWrite(pin,LOW);
}

boolean reconnect() {
  if (mqttClient.connect(CLIENT_ID)) {
    // Once connected, publish an announcement...
    mqttClient.publish("lcabs1993/arduino","Reconnected");
    // ... and resubscribe
  //  mqttClient.subscribe("lcabs1993");
  }
  return mqttClient.connected();
}

////////////////////////////////////////////////////////////////////////////////SETUP/////
void setup() {
      Serial.begin(9600); // Start serial port 
////////////////////////////////////////////
 mqttClient.setClient(ethClient);
 mqttClient.setServer("broker.hivemq.com",1883);
 mqttClient.setCallback(callback);
////////////////////////////////////////////
  // start the Ethernet connection:
  Serial.println("Initializing Ethernet with DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    //TODO: Include watchdog
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
///////////////////////////////////////////////
//setupBuzzer(BUZZER_PIN);
setupLED(LED_PIN);
//sendData();
resubscribe();
// mqttClient.subscribe("lcabs1993");
// mqttClient.subscribe("lcabs1993/arduino");
// mqttClient.subscribe("lcabs1993/arduino/buzzer");
// mqttClient.subscribe("lcabs1993/arduino/led");
// mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
// mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");

}
/////////////////////////////////////////////////////////////////////////////////LOOP/////

void loop() {
delay(1000);
//readPushbutton(); //reads Pushbutton
int dht[2] ;
//readDHT11(temp,humi); // faz a leitura do DHT11
 int chk = DHT.read11(DHT11_PIN);  //reads DHT11
int  temp = DHT.temperature;     // reads current state
int  humi = DHT.humidity;  
//  Serial.print("Temp: ");     //prints to serial
 // Serial.println(temp);
//  Serial.print("Humi: "); 
//  Serial.println(humi);
if (temp == lasttemp)
{
  }else {
          char msgbuffer[20];
 //       Serial.print("temp: "); Serial.println(temp);
 //       Serial.print("lasttemp: "); Serial.println(lasttemp);        
          mqttClient.publish("lcabs1993/arduino/dht11/temp",itoa(temp, msgbuffer, 10));   //publishes DHT11 data to hiveMQ
          lasttemp= temp;
  }


if (humi != lasthumi){
}    
else {
          char msgbuffer[20];
//          Serial.print("humi: "); Serial.println(humi);
//          Serial.print("lasthumi: "); Serial.println(lasthumi); Serial.println("---");    
          mqttClient.publish("lcabs1993/arduino/dht11/humidade",itoa(humi, msgbuffer, 10));   //publishes DHT11 data to hiveMQ
          lasthumi= humi;
  }

  char msgbuffer[4];         //initializes a message buffer
 // mqttClient.publish("lcabs1993/arduino/dht11/temp",itoa(temp, msgbuffer, 10));   //publishes DHT11 data to hiveMQ
 // mqttClient.publish("lcabs1993/arduino/dht11/humidade",itoa(humi, msgbuffer, 10));

  
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        resubscribe();
 //mqttClient.subscribe("lcabs1993");
 //mqttClient.subscribe("lcabs1993/arduino");
 //mqttClient.subscribe("lcabs1993/arduino/buzzer");
 //mqttClient.subscribe("lcabs1993/arduino/led");
 //mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
 //mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
        lastReconnectAttempt = 0;
      }
    }
  } else {
    // Client connected
mqttClient.loop();
  }
}

//pubPushbutton(); //TODO: se mudou, printa nova timestamp
//readDHT11(); //TODO: checa se mudou temperatura e humidade
//pubDHT11(); //TODO: se mudou, publica nova temperatura e nova humidade
//atualizaOLED(); //TODO: atualiza OLED com novos valores


void sendData(){ 
    char msgBuffer[20];
   if(mqttClient.connect(CLIENT_ID)) {
   mqttClient.publish("lcabs1993/arduino", "Arduino on!");
   mqttClient.publish("lcabs1993/arduino/dht11/temp", "XX °C");
   mqttClient.publish("lcabs1993/arduino/dht11/humidade", "XX %");
 }
  }

void setupBuzzer(int pin){
  pinMode(pin, OUTPUT);
}

//void readPushbutton(){
//const int  buttonPin = 7;    // the pin that the pushbutton is attached to
//int buttonPushCounter = 0;   // counter for the number of button presses
//int buttonState = 0;         // current state of the button
//int lastButtonState = 0;     // previous state of the button
  // read the pushbutton input pin:
//  buttonState = digitalRead(buttonPin);
  // compare the buttonState to its previous state
//  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
//    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
//      buttonPushCounter++;
  //    Serial.println("on");
//      mqttClient.publish("lcabs1993/arduino","on");
  //    Serial.print("number of button pushes: ");
   //   Serial.println(buttonPushCounter);
 //   } else {
      // if the current state is LOW then the button went from on to off:
    //  Serial.println("off");
  //    mqttClient.publish("lcabs1993/arduino","off");
  //  }
    // Delay a little bit to avoid bouncing
  //  delay(500);
 // }
 //   lastButtonState = buttonState;

void resubscribe(){
 mqttClient.subscribe("lcabs1993");
 mqttClient.subscribe("lcabs1993/arduino");
 mqttClient.subscribe("lcabs1993/arduino/buzzer");
 mqttClient.subscribe("lcabs1993/arduino/led");
 mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
 mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
}
