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

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    lcd.printf("%s\n\n", (char*)message.payload);
    //return;
}

void my_thread(MQTT::Client<MQTTNetwork, Countdown> client) {
    while (true) {
        client.yield(1000);
    }
}

void mqt(NetworkInterface *myfi) {
    
    MQTTNetwork mqttNetwork(myfi);
 
    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);
    const char* broker = "35.188.242.1";
    int port = 1883;
    //pc.printf("Connecting to %s:%d\r\n", broker, port);
    int rc = mqttNetwork.connect(broker, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\r\n", rc);
    
    const int len = snprintf(NULL, 0, "%lx-%lx%lx%lx%lx", output2[1], output[1], output[2], output[3], output[4]);
    char buf[len+1];
    snprintf(buf, len+1, "%lx-%lx%lx%lx%lx", output2[1], output[1], output[2], output[3], output[4]);
    
    //pc.printf("%s\r\n", buf);
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    Countdown countdown;
    data.clientID.cstring = buf;
    //strcat(strcat("<",buf),">");
    data.username.cstring = "mbed";
    data.password.cstring = "homework";
    //data.keepAlive = 10;
    
    const int t = snprintf(NULL, 0, "cis541/hw-mqtt/%s/data", buf);
    char send_topic[t+1];
    char pub_topic[t+1];
    snprintf(send_topic, t+1, "cis541/hw-mqtt/%s/data", buf);
    snprintf(pub_topic, t+1, "cis541/hw-mqtt/%s/echo", buf);

    //pc.printf("%s, %s\r\n", data.clientID.cstring, topic);
    
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\r\n", rc);
 
    if ((rc = client.subscribe(pub_topic, MQTT::QOS0, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);
    //thread.start(my_thread(client));
    //countdown.countdown(5);
    pc.printf("message\r\n");
    MQTT::Message message;
    char arr[100];
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    while(true) {
        countdown.countdown(5);
        while(countdown.expired() != true) {
            //countdown.countdown(5);
            //pc.printf("counted down\r\n");
            
        }
        int mess = rand()%100;
        sprintf(arr, "%d", mess);
        message.payload = (void*)arr;
        message.payloadlen = strlen(arr);
        //pc.printf("sending");
        rc = client.publish(send_topic, message);
        //pc.printf("%d", rc);
        client.yield(1);
        //pc.printf("%s",arr);
        //client.yield(1);
    }   
}
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
    //pc.printf("%s",wifi.get_mac_address());
    pc.printf("Start\r\n");
    command[0] = 58;
    command2[0] = 54;
    iap_entry = (IAP) IAP_LOCATION;
    iap_entry (command, output);
    iap_entry (command2, output2);
    int ret = wifi.connect("goHAMCO!", "whatisthepassword");
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }
    lcd.printf("Connected\r\n");
    lcd.printf("HTTP = 1 \nMQTT = 2\r\n");
    int command = pc.getc();
    if (command == '1') {
        lcd.printf("Milestone 1\n\n");
        printhttp(&wifi);
    } else {
        pc.printf("Milestone 2\r\n");
        mqt(&wifi);
        //pc.printf("DONEZO");
        return 0;
    }
    
    // Close the socket to return its memory and bring down the network interface
    //socket.close();
    wifi.disconnect();
    //pc.printf("DONEZO");
    return 0;
}