
// Include Libraries
    #include <PubSubClient.h>;
    #include <ESP8266WiFi.h>;
    #include <ESP8266mDNS.h>;
    #include <WiFiUdp.h>;
    #include <SimpleTimer.h>;
    #include <Arduino.h>;
    #include <dht.h>
    
    dht DHT;

// Wifi Settings:
    const char*  wifi_ssid ="YOUR WIFI SSID"; 
    const char*  wifi_password ="YOUR WIFI Password"; 

 // MQTT Settings: 
    const char* mqtt_broker = "YOUR MQTT BROKER";
    const int mqtt_port = 1883;
    const char* mqtt_username = "YOUR MQTT USER";
    const char* mqtt_password = "YOUR MQTT PASSWORD";
    const char* mqtt_client = "YOUR CLIENT NAME";

// initialise WiFiClient for espClient type
    WiFiClient espClient;

// initialise PubSubClient for espClient type
    PubSubClient client(espClient);

//initialise SimpleTimer as timer
    SimpleTimer timer;

// Variabeles
    //Set the Bootstate initially on true
        bool boot = true; 

    //Set DHT pin
        #define DHT11_PIN 2

    //Set Temperature Variables
        String temp_str;
        char temp[50];

    // Set Humidity Variables
        String hum_str;
        char hum[50];

//Functions


// Function for the initial wifi connection 
    void setup_wifi ()
    {
        // we'll start with connecting to the wifi network and print the status to the serial console
            Serial.println();
            Serial.print("Connecting to SSID: ");
            Serial.print(wifi_ssid);

        // start the connection
            WiFi.begin (wifi_ssid,wifi_password);

        // until the client is connected, print a dot every 500ms

            while (WiFi.status() !=WL_CONNECTED)
            {
                delay(5000);
                Serial.print(".");
            }

        // Here we are connected: so we serial print
        // our Status, Ip address, and Mac address

            Serial.println (" ");
            Serial.println ("Wifi client is connected");
            Serial.println ("Ip address: ");
            Serial.println (WiFi.localIP());
        
    }

// Function to reconnect
    
    void reconnect ()
    {
        // create a loop until we're reconnected with m
        int retries = 0;
        
        while (!client.connected())
        {
            if (retries < 15)
            {
                //write update to the console:
                Serial.println("Reconnecting to mqtt broker: ");
                Serial.println (mqtt_broker);

                //attempt to reconnect to the mqtt broker
                if (client.connect(mqtt_client, mqtt_username,mqtt_password))
                {
                    Serial.println ("connected");
                    //now that we are connected publish a message to the broker letting it know we're here. 
                    //can be used for future automations + monitoring

                    if (boot == true)
                    {
                        //Perform Check-in
                        client.publish("checkin/YOUR CLIENT NAME","rebooted");
                        boot = false; 
                        
                    }

                    if (boot == false)
                    {
                        //Perform Check-in
                        client.publish ("checkin/YOUR CLIENT NAME","reconnected");

                    }
                
                // resubscribe to a command topic for incomming commands from hass or node-red
                client.subscribe ("YOUR CLIENT NAME/commands");
                    
                }

                else
                {
                    //when we're unable to connect wait 5 seconds and try again 
                    Serial.print("Failed, reconnect=");
                    Serial.print(client.state());
                    Serial.println ("retry in 5 seconds");
                    retries++;
                    // retry in 5 seconds
                    delay(5000);     

                }

                if(retries > 14)
                {
                    //When we we are past max retries, reboot the board
                    ESP.restart();
                    
                }

            }
        }
    }

// Function for the checkin
    void checkin()
    {
        client.publish ("checkin/YOUR CLIENT NAME","ok");
    }


// Function to listen for commands
void callback(char* topic, byte* payload, unsigned int length) 
    {
         Serial.print("Message arrived [");
        String newTopic = topic;
        Serial.print(topic);
        Serial.print("] ");
        payload[length] = '\0';
        String newPayload = String((char *)payload);
        Serial.println(newPayload);
        Serial.println();

        // Commands go here:
        if (newTopic == "YOUR CLIENT NAME/commands") 
        {
            if (newPayload == "reboot")
            {
            Serial.println("Reboot command received");
            client.publish ("checkin/YOUR CLIENT NAME","Reboot Command received");
            pinMode(2,OUTPUT);
            digitalWrite(2,LOW);
            delay(5000);
            ESP.restart();
            }

            if (newPayload == "LED")
            {
            Serial.println("LED command received");
            client.publish ("checkin/YOUR CLIENT NAME","LED Command received");
            pinMode(2,OUTPUT);
            digitalWrite(2,LOW);
            delay(5000);
            digitalWrite(2,HIGH);
            
            }
        
        }

    }

// SETUP STARTS HERE, now we will use the functions above

void setup()
{
  //set the baudrate to 115200
    Serial.begin(115200);

  // Setup the wifi
    setup_wifi();

  // connect to the mqtt_broker
    client.setServer(mqtt_broker,mqtt_port);

  // start the client callback
    client.setCallback(callback);

  // set interval for the checkin  
    timer.setInterval(120000,checkin);


}


void loop()
{
    // if Client is not connected, call the reconnect function
        if (!client.connected()) 
        {
            reconnect();    
        }

    // Client.loop makes sure that we're listening for commands
        client.loop();
    
    // start the timer
        timer.run();


    // Get Data from the DHT11n Sensor 
        int chk = DHT.read11(DHT11_PIN);

    // Print the Data to the serial console (for trouble shooting)
        Serial.print("Temperature = ");
        Serial.println(DHT.temperature);

        Serial.print("Humidity = ");
        Serial.println(DHT.humidity);

    // Prepare the data to be transmited with mqtt

        //converting ftemp (the float variable above) to a string 
            temp_str = String(DHT.temperature); 

        //packaging up the data to publish to mqtt
            temp_str.toCharArray(temp, temp_str.length() + 1);
        
        //converting Humidity (the float variable above) to a string
            hum_str = String(DHT.humidity); 

        //packaging up the data to publish to mqtt
            hum_str.toCharArray(hum, hum_str.length() + 1);

    // Send the data to the MQTT broker
        client.publish("LivingRoom/temp",temp);
        client.publish("LivingRoom/hum",hum);

    // Set delay of 10 seconds so we don't flood the server when we wire every room
        delay(10000);


  
}
