#define MQTTCLIENT_QOS2 1
#define IAP_LOCATION 0x1FFF1FF1
#define COMMAND 54

#include "TextLCD.h"
#include "ESP8266Interface.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "TCPSocket.h"
#include "mbed.h"
#include "rtos.h"
#include <string>

unsigned long command[5];
unsigned long output[5];
unsigned long command2[5];
unsigned long output2[5];

typedef void (*IAP)(unsigned long[],unsigned long[]);
IAP iap_entry;

ESP8266Interface wifi(p28, p27);
Serial pc(USBTX, USBRX);
Thread thread;
TextLCD lcd(p15, p16, p17, p18, p19, p20, TextLCD::LCD16x2);


//Message Arrived from Subscription
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    pc.printf("Recieved: %s\n\n", (char*)message.payload);
    return;
}

//MQTT Pacemaker Method
int mqt(NetworkInterface *myfi) {
    //Basic Setup
    MQTTNetwork mqttNetwork(myfi);
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    const char* broker = "35.188.242.1";
    int port = 1883;
    int rc = mqttNetwork.connect(broker, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\r\n", rc);
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;   
    
    //Set channels and info
    data.username.cstring = "mbed";
    data.password.cstring = "homework";
    char* send_topic = "Group3Pace";
    char* pub_topic = "Group3Heart";
    
    //Connect to channels
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\r\n", rc);
    if ((rc = client.subscribe(pub_topic, MQTT::QOS0, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
        
    //Initialize message
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    
    Countdown countdown;
    
    //Main loop
    while (true) {
        
        countdown.countdown(1);
        while(countdown.expired() != true) {
            //countdown.countdown(5);
            //pc.printf("counted down\r\n");
            
        }
        
        //Generate random and load message
        int bpm = rand()%75;
        char buf[100];
        sprintf(buf,"%d",bpm);
        message.payload = (void*)buf;
        message.payloadlen = strlen(buf);
        
        //Publish to channel
        if ((rc = client.publish(send_topic, message)) != 0) {
            printf("Publish Error");
            return -1;
        }
        pc.printf("%d\n\n", bpm);
        client.yield(10);
    }
    wifi.disconnect();
    return 0;   
}

//Network GET request
void printhttp(NetworkInterface *myfi) {
    TCPSocket socket;
    nsapi_error_t response;
    socket.open(myfi);
    response = socket.connect("35.188.242.1", 80);
    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        return;
    }
    // Send a simple http request
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: 35.188.242.1\r\n\r\n";
    nsapi_size_t size = strlen(sbuffer);
    while(size)
    {
        response = socket.send(sbuffer+response, size);
        if (response < 0) {
            printf("Error sending data: %d\n", response);
            socket.close();
            return;
        } else {
            size -= response;
            //printf("sent %d [%.*s]\n", response, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);
        }
    }
    // Recieve a simple http response and print out the response line
    char rbuffer[128];
    response = socket.recv(rbuffer, sizeof rbuffer);
    if (response < 0) {
        printf("Error receiving data: %d\n", response);
    } else {
        while (!(response < 0)) {
            pc.printf("%s", rbuffer);
            response = socket.recv(rbuffer, sizeof rbuffer);
        }
    }
    socket.close();
}


int main(int argc, char* argv[]) {
    pc.printf("Start\r\n");
    
    //Basic Setup
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD);
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
    pc.printf("Connected\r\n");
    
    //Run MQTT method
    mqt(&wifi);
    wifi.disconnect();
    return 0;
}