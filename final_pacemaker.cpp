#define MQTTCLIENT_QOS2 1
#define IAP_LOCATION 0x1FFF1FF1

#include "ESP8266Interface.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "mbed.h"

Serial pc(USBTX, USBRX);
Serial slave(p9, p10);
Countdown cd;
Thread receiving;
int duration;

void receivingThread() {
    while(true) {
        if (slave.readable() && slave.getc() == 't') {
            printf("heartbeat duration: %d\r\n", 1480 - duration);
            printf("reads from heart\n");
            //if heart beats too fast
            pc.printf("FASTER THAN 1480");    

            if (duration > 900) {
                //publish to broker "FAST"
                pc.printf("TOO FAST");
            }
            duration = 1480;
        }  
    }
} 

int main() {
    receiving.start((callback(receivingThread)));
    //MQTT BS
    
    
    while (true) {
        duration = 1480;
        while (duration > 0) {
            duration -= 1;
            cd.countdown_ms(1);
            while(!cd.expired());
        }
        pc.printf("SLOWER THAN 1480");    //force a tick
        slave.printf("t");
        //publish to broker "SLOW"
    }
    return 1;
}
