/* ARDUINO -> HIVEMQ
   Author: Lazaro Borges Silva

   PIR01      ->  MQTT    publishes timestamp
   BUZZER     <-  MQTT    activates buzzer on command
   PUSHBUTTON ->  MQTT    publishes timestamp (may activate something later idk)
   LED01      <-  MQTT    activates a lamp on command

*/

#include <SPI.h>
#include <stdlib.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <EthernetUdp.h>
//#include <dht.h>

//dht DHT;
EthernetUDP Udp;

#define     CLIENT_ID "Arduino"
#define     BUZZER_PIN 31
#define     LED_PIN 35
//#define   DHT11_PIN 49
#define     PUSHBUTTON_PIN 33
#define     PIR01_VCC 7
#define     PIR01_PIN 6
#define     PIR01_GND 5

//int       lasttemp = 11;                                    //initialize variable for last temperature
//int       lasthumi = 10;                                    //initialize variable for last humidity
//int       temp = 1;
//int       humi = 2;
long        lastReconnectAttempt = 0;                         // for the mqTT watchdog
const int   buttonPin = PUSHBUTTON_PIN;                       // the pin that the pushbutton is attached to
int         buttonPushCounter = 0;                            // counter for the number of button presses
int         buttonState = 0;                                  // current state of the button
int         lastButtonState = 0;                              // previous state of the button
const int   PIR01Pin = PIR01_PIN;                             // the pin that the pushbutton is attached to
int         PIR01Counter = 0;                                 // counter for the number of button presses
int         PIR01State = 0;                                   // current state of the button
int         lastPIR01State = 0;                               // previous state of the button

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
void readPIR01();
void pubPIR01();
boolean reconnect();

byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
IPAddress ip(192, 168, 0, 105);

class myClass {
  public:
    String payload;                       // reads the payload received
    String buzzer;                        // reads the buzzer state
    String lastBuzzer = "buzzeroff";      // initializes the buzzer state
    String PIR01;                         // reads the PIR01 state
    String temp;                          // reads the temperature
    String humi;                          // reads the humidity
    String LED01;                         // reads the LED01 state
    String pushbutton;                    // reads the pushbutton state
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

myClass myObj;

EthernetClient ethClient;                 // Ethernet and MQTT related objects
PubSubClient mqttClient;

String myString;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("HiveMq: [");                                            // prints the topic of the received payload
  Serial.print(topic);
  Serial.print("] ");
  char    payload_char[length];                                         // reads the payload
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
  Serial.begin(9600);                                                           // Start serial port
  Serial.println("----------------------------------");                         // print stuff
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
  if (Ethernet.begin(mac) == 0) {
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
