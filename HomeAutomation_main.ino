#include <SPI.h>
#include <stdlib.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define CLIENT_ID "Arduino"
#define BUZZER_PIN 53
#define LED_PIN 51

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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  }
  


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

setupBuzzer(BUZZER_PIN);
setupLED();
sendData();


 mqttClient.subscribe("lcabs1993");
 mqttClient.subscribe("lcabs1993/arduino");
 mqttClient.subscribe("lcabs1993/arduino/dht11/temp");
 mqttClient.subscribe("lcabs1993/arduino/dht11/humidade");
 mqttClient.subscribe("lcabs1993/led");
}
////////////////////////////////////////////

void loop() {
mqttClient.loop();
readPushbutton(); //faz a leitura do estado do botão
//pubPushbutton(); //TODO: se mudou, printa nova timestamp
//readDHT11(); //TODO: checa se mudou temperatura e humidade
//pubDHT11(); //TODO: se mudou, publica nova temperatura e nova humidade
//atualizaOLED(); //TODO: atualiza OLED com novos valores

 delay(1000);
}

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

void readPushbutton(){
const int  buttonPin = 7;    // the pin that the pushbutton is attached to
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      Serial.println("on");
      mqttClient.publish("lcabs1993/arduino","on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("off");
      mqttClient.publish("lcabs1993/arduino","off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
    lastButtonState = buttonState;
}

void setupLED(){
  int pin;
  pinMode(pin, OUTPUT);
}
