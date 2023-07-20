# HomeAutomation
Project for online monitoring and control of sensors and actuators in a house.

///////// PINS
#define BUZZER_PIN 53
#define LED_PIN 51
//#define DHT11_PIN 49
#define PUSHBUTTON_PIN 47
#define PIR01_VCC 7
#define PIR01_PIN 6
#define PIR01_GND 5

1 - Estabelecendo o servidor
    No cmd, **net start mosquitto**
            **mosquitto_sub -h localhost -t "MakerIOTopic"**
    
    
    
2 - Conectando arduino à internet

3 - Enviando mensagem do arduino para o servidor

4 - Monitorando o primeiro sensor

