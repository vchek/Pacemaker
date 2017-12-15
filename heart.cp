#define MQTTCLIENT_QOS2 1
#define IAP_LOCATION 0x1FFF1FF1

#include "ESP8266Interface.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "mbed.h"

Serial pc(USBTX, USBRX);
Serial master(p9, p10);
Countdown cd;
Thread receiving;
int duration;

void receivingThread() {
    while(true) {
        if (master.readable()) {
            printf("durationinthread%d\r\n", duration);
            duration = 0;    
        }    
    }
} 

int main() {
    receiving.start((callback(receivingThread)));
    while (true) {
        duration = 300 + (rand() % 2000);
        printf("duration%d\r\n", duration);
        while (duration > 0) {
            duration -= 1;
            cd.countdown_ms(1);
            while(!cd.expired());    
        }
        master.printf("tick");
        pc.printf("printed something\r\n");
    }
    return 0;
}
