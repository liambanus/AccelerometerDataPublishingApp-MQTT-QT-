#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include "MQTTClient.h"
#include <thread>
#include <time.h>
#include"I2CDevice.h"
#include "ADXL345.h"
#include<iostream>
#include<sstream>
#include<fcntl.h>
#include<stdio.h>
#include<iomanip>
#include<unistd.h>
#include<sys/ioctl.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
using namespace std;


#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)
//g++ I2CDevice.cpp ADXL345.cpp publish.cpp -o publish  -lpaho-mqtt3c


#include"I2CDevice.h"
#include<iostream>
#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;
using namespace exploringRPi;

//Please replace the following address with the address of your server
//#define ADDRESS    "tcp://10.42.0.152:1883"//works with or without port number
#define ADDRESS    "tcp://192.168.0.10:1883"
//192.168.0.10 
#define CLIENTID   "rpi1"
#define AUTHMETHOD "liambanus"
#define AUTHTOKEN  "pw"
#define TOPIC_FOLDER "ee513/"
#define TOPIC_tem     "ee513/1CPUTem"
#define TOPIC_pitch      "ee513/2Ptch"
#define TOPIC_roll      "ee513/3Roll"
#define TIMEOUT    10000L
#define QOS_3Roll        2
#define QOS_2Ptch        1
#define QOS_1CPUTem        0

float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
 fstream fs;
   fs.open(CPU_TEMP, fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}

int main(int argc, char* argv[]) {
   bool running = true;
   while (running){
   ADXL345 adx(2, 0x53);
   //adx.displayPitchAndRoll(2);
   
   char str_payload[200];          // Set your max message size here
   MQTTClient client;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_willOptions will_opts = MQTTClient_willOptions_initializer;

   //MQTTCLIENT_TRACE = ON;
   //MQTTCLIENT_TRACE_LEVEL = PROTOCOL;
  //S2.ii
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   //MQTTClient_willOptions =
   MQTTClient_deliveryToken token;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
   opts.cleansession = 0;//chd maybe this will keep a record between cons, was 1

   //When the clean session flag is set to true, the client does not want a persistent session. If the client disconnects for any reason, 
   //all information and messages that are queued from a previous persistent session are lost.
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   will_opts.topicName = TOPIC_tem;
    //will_opts.topicName = TOPIC;
   will_opts.message = "I told you I was ill";
   will_opts.qos = QOS_1CPUTem;
   opts.will = &will_opts;
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }
  printf ("Current local time and date: %s", asctime(adx.get_time()));
    adx.readSensorState();

   //sprintf is like a string buffer, prepares strings for output but doesn't directly output them
   
   //sprintf(str_payload, "{\"d \":{\"", TOPIC, TOPIC_FOLDER,": %f and pitch %f and roll %f }}", getCPUTemperature(), adx.getPitch(), adx.getRoll());
   //S2.i CPUTemp Topic:
   sprintf(str_payload, "{\"d\":{\"1CPUTem\": %f  }}", getCPUTemperature());
   pubmsg.payload = str_payload;//multiple payloads/publish callsMAN
   pubmsg.payloadlen = strlen(str_payload);
   //pubmsg.qos = QOS;
   pubmsg.qos = QOS_1CPUTem; 
   pubmsg.retained = 0;
   
   MQTTClient_publishMessage(client, TOPIC_tem, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload <<
        " \non topic " << TOPIC_tem << " for ClientID: " << CLIENTID << endl;
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;
    std::this_thread::sleep_for (std::chrono::seconds(5));
   //MQTTClient_disconnect(client, 1000);
   //c&p:

   sprintf(str_payload, "{\"d\":{\"2Ptch\": %f  }}", adx.getPitch());

   //sprintf(str_payload, "{\"d\":{\"2Ptch\": %f  }}", adx.getPitch());
   //sprintf(str_payload, "{\"d\":{\"CPUTemp\": %f }}", getCPUTemperature());
   pubmsg.payload = str_payload;//multiple payloads/publish callsMAN
   pubmsg.payloadlen = strlen(str_payload);
   pubmsg.qos = QOS_2Ptch;//chd
   pubmsg.retained = 0;
   
  
   adx.readSensorState();
   MQTTClient_publishMessage(client, TOPIC_pitch, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload <<
        " \non topic " << TOPIC_pitch << " for ClientID: " << CLIENTID << endl;
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;
   std::this_thread::sleep_for (std::chrono::seconds(1));
   //MQTTClient_disconnect(client, 1000);
   //MQTTClient_destroy(&client);
   //return rc;

sprintf(str_payload, "{\"d\":{\"3Roll\": %f  }}", adx.getRoll());

   //sprintf(str_payload, "{\"d\":{\"2Ptch\": %f  }}", adx.getPitch());
   //sprintf(str_payload, "{\"d\":{\"CPUTemp\": %f }}", getCPUTemperature());
   pubmsg.payload = str_payload;//multiple payloads/publish callsMAN
   pubmsg.payloadlen = strlen(str_payload);
   pubmsg.qos = QOS_3Roll;//chd
   pubmsg.retained = 0;
   
  
   //adx.readSensorState();
   MQTTClient_publishMessage(client, TOPIC_roll, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << str_payload <<
        " \non topic " << TOPIC_roll << " for ClientID: " << CLIENTID << endl;
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered." << endl;
   std::this_thread::sleep_for (std::chrono::seconds(1));
   //MQTTClient_disconnect(client, 1000);
   //MQTTClient_destroy(&client);
   //return rc;


   }
   
   
}



//wrap the time stuff into a function I'd say
  
 
   //and pitch %f and roll %f
   //strcat(TOPIC_FOLDER, TOPIC);
   //TOPIC_FOLDER.append(TOPIC);
   //TOPIC = TOPIC_FOLDER + std::string (TOPIC);


  //struct tm y2k = {0};
  //double seconds;
  //y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
  //y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
  //seconds = difftime(timer,mktime(&y2k));
  
  /*
  time_t timer;
  time(&timer);  // get current time; same as: timer = time(NULL)  


  struct tm * timeinfo;

  time (&timer);
  timeinfo = localtime (&timer);
  //printf ("Current local time and date: %s", asctime(timeinfo));

  */