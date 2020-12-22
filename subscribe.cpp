#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
//works out of the box; just ssh with IP add 151 using Putty
//#define ADDRESS     "tcp://10.42.0.152"
#define ADDRESS    "tcp://192.168.0.10:1883"

#define CLIENTID    "rpi5"
#define AUTHMETHOD  "liambanus"
#define AUTHTOKEN   "pw"
#define TOPIC       "ee513/1CPUTem"
#define TOPIC_pitch       "ee513/2Ptch"
#define TOPIC_roll       "ee513/3Roll"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#include<iostream>

//g++ subscribe.cpp -o subscribe -lpaho-mqtt3c -std=c++11


#include<iostream>
#include<fstream>
#include<string>
#include<unistd.h>         // for the microsecond sleep function
using namespace std;
#define GPIO         "/sys/class/gpio/"
#define FLASH_DELAY  50000 // 50 milliseconds


using namespace std;


class LED{
   private:                // the following is part of the implementation
      string gpioPath;     // private states
      int    gpioNumber;
      void writeSysfs(string path, string filename, string value);
   public:                 // part of the public interface
      LED(int gpioNumber); // the constructor -- create the object
      virtual void turnOn();
      virtual void turnOff();
      virtual void displayState();
      virtual ~LED();      // the destructor -- called automatically
};

LED::LED(int gpioNumber){  // constructor implementation
   this->gpioNumber = gpioNumber;
   gpioPath = string(GPIO "gpio") + to_string(gpioNumber) + string("/");
   writeSysfs(string(GPIO), "export", to_string(gpioNumber));
   usleep(100000);         // ensure GPIO is exported
   writeSysfs(gpioPath, "direction", "out");
}

// This implementation function is "hidden" outside of the class
void LED::writeSysfs(string path, string filename, string value){
   ofstream fs;
   fs.open((path+filename).c_str());
   fs << value;
   fs.close();
}

void LED::turnOn(){
   writeSysfs(gpioPath, "value", "1");
}

void LED::turnOff(){
   writeSysfs(gpioPath, "value", "0");
}

void LED::displayState(){
   cout << "The current LED state is ";   
   ifstream fs;
   fs.open((gpioPath + "value").c_str());
   string line;
   cout << "The current LED state is ";
   while(getline(fs,line)) cout << line << endl;
   fs.close();
}

LED::~LED(){  // The destructor unexports the sysfs GPIO entries
   usleep(2000);
   //writeSysfs(string(GPIO), "unexport", to_string(gpioNumber));
}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;

}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = (char*) message->payload;
    LED gled12(12); 
    LED lled18(18);
    LED rled23(23);
    LED dled21(21);
    //https://stackoverflow.com/questions/4329324/c-error-array-must-be-initialized-with-a-brace-enclosed-initializer

    char topic_identifier[1] = {topicName[6]}; //topics coded 1-9
    cout << "top name char[6] :" << topic_identifier[0] << endl;
    
    if (topic_identifier[0] == '1'){       

        float temp16 = atof(payloadptr+16);
         cout << "temp 16 is "<< temp16 << endl;
        if (temp16 > 36.5) {
            cout << "Flashing LED for danger" << endl;
            dled21.turnOn();            
            dled21.turnOff(); 
        }
        else {
            cout << "Temperature Safe" << endl;
           //gled12.turnOff();
            //usleep(FLASH_DELAY);
            gled12.turnOn();}
    }
    if (topic_identifier[0] == '2'){
        float pitch14 = atof(payloadptr+14);
        //cout << "pitch 14 is "<< pitch14 << endl;
        if(pitch14 > 10.0) {
            cout << "Forward tilt alert" << endl;
            //gled12.turnOff();
            //usleep(FLASH_DELAY);
            gled12.turnOn();}
        else if(pitch14 < -10.0) {
            cout << "Backward tilt alert" << endl;
            dled21.turnOn();
            //usleep(FLASH_DELAY);
           // dled12.turnOff();            
            //led12.turnOn();   } 
        }
        else {
            dled21.turnOff();
            gled12.turnOff();
        }
        }
    if (topic_identifier[0] == '3'){
        cout << "roll id'd" << endl;
        float roll14 = atof(payloadptr+14);
        cout << "roll 14 is "<< roll14 << endl; 
         if(roll14 > 1.5) {
            cout << "Left Turn" << endl;
            lled18.turnOn();
            //usleep(FLASH_DELAY);
            //usleep(FLASH_DELAY);
            //led23.turnOff();  
            }  
        else if(roll14 < -1.5) {
            cout << "Right Turn" << endl;
            rled23.turnOn();
            //usleep(FLASH_DELAY);
            //usleep(FLASH_DELAY);
            //led16.turnOff();  
            }  
        else {
            lled18.turnOff();
            rled23.turnOff();

        
         
    }}

    for(i=0; i<message->payloadlen; i++) {
        //float temper = atof(payloadptr+i);
        //cout << "temper: "<< i << "is: " <<temper << endl;
        putchar(*payloadptr++);//putchar just prints straight to ostream
    }
    putchar('\n');

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

//g++ subscribe.cpp -o subscribe -lpaho-mqtt3c -std=c++11

int main(int argc, char* argv[]) {

    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
    const char* CLIENTID2 = argv[2];
    MQTTClient_create(&client, ADDRESS, CLIENTID2, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 0;//changed from 1 to disable clean session
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;    
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    //printf("argv[1]: %s ", argv[1]);

    //  Processing command line argument
    if (argv[1] == std::string("temp")){
        printf("Subbing to temp\n");
        printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID2, QOS);
        MQTTClient_subscribe(client, TOPIC, QOS);

    }
    else if (argv[1] == std::string("adxl")){
                printf("Subbing to adxl\n");

         printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC_pitch, CLIENTID2, QOS);
        MQTTClient_subscribe(client, TOPIC_pitch, QOS);

         printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC_roll, CLIENTID2, QOS);
         MQTTClient_subscribe(client, TOPIC_roll, QOS);
    }

    else if (argv[1] == std::string("both")){
        printf("Subbing to temp and adxl \n");
        printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
         MQTTClient_subscribe(client, TOPIC, QOS);
         printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC_pitch, CLIENTID, QOS);
         MQTTClient_subscribe(client, TOPIC_pitch, QOS);

        printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC_roll, CLIENTID, QOS);
         MQTTClient_subscribe(client, TOPIC_roll, QOS);}
    else {printf( "No argument supplied, please enter 'temp', 'adxl' or 'both' as the first string argument and desired clientid as the second");}  
    
    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}