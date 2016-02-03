// Hue network ip and access info
TCPClient client;
const char hueHubIP[] = "192.168.1.20";     // Local HUE bridge
const char hueUsername[] = "newdeveloper";  // Hue username

// read the buzzer value
int buzzerPin = D3;                         // pin the buzzer is connected to
int buzzerState;                            // current reading from the input pin

// debouncing variables to check button twice before doing anything
long lastDebounceTime = 0;                  // the last time the output pin was toggled
long debounceDelay = 120;                   // the debounce time; increase if multiple texts are sent
int lastBuzzerState = HIGH;                 // store the previous reading from the buzzerPin

// Hue variables
int lightNum;                               // which HUE light we want to control
unsigned int len;                           // to store length of String command sent to HUE

void setup() {
    // read buzzerPin
    pinMode(buzzerPin, INPUT_PULLUP);
    
    // set up connection to HUE bridge
    Serial.begin(9600);
    delay(1000);
    client.connect(hueHubIP, 80);
    delay(1000);
    if (client.connected()){
        Serial.println("connection ready.");
    }
    else{
        Serial.println("connection failed");
    }
}

void loop() {
    // put the state of the switch into a local variable, 'reading':
    int reading = digitalRead(buzzerPin);
    
    // check to see if the buzzer has went off
    // (i.e. the input went from LOW to HIGH),  and you've waited
    // long enough to ignore any noise:
    if (reading != lastBuzzerState) {
        // reset the debouncing timer
        lastDebounceTime = millis();
    }
    
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    if ((millis() - lastDebounceTime) > debounceDelay) {
        
        // if the actual current state of the buzzer has changed,
        // save it to the buzzerState variable
        if (reading != buzzerState) {
            buzzerState = reading;
            
            // if the buzzerState is LOW, send a text via Twilio    
            if (buzzerState == LOW);
                // use a webhook to send json to Twilio
                Particle.publish("twilio", "Heads up: Someone just buzzed your intercom!", 60, PRIVATE);
                
                // if a connection to the HUE bridge exists then do stuff with the lights
                if (client.connect(hueHubIP, 80)) {
                    while (client.connected())
                    {
                        String command = "{\"on\": true,\"hue\":50000,\"sat\":254,\"bri\":180,\"alert\":\"none\",\"transitiontime\":"+String(random(40,120))+"}";
                        setHue(lightNum, command);
                        delay(8000);
                    }
                }
        }
        else {
            // turn HUE off
            String command = "{\"on\": false}";
            setHue(lightNum, command);
        }
    }
    // Revert buzzer state 
    lastBuzzerState = reading;
}

boolean setHue(int lightNum, String command)        // Moving this out of the loop
{
    if (client.connect(hueHubIP, 80))
    {
        if(client.connected())
        {
            unsigned int len = command.length();    //get length
            client.println("PUT /api/newdeveloper/lights/3/state HTTP/1.1");
            client.println("Connection: keep-alive");
            client.print("Host: ");
            client.println(hueHubIP);
            client.println("Content-Type: text/plain;charset=UTF-8"); //
            client.print("Content-Length: ");
            client.println(len);                    //brightness string + val length
            client.println();                       // blank line before body
            client.print(command);
            Serial.print("sent");                   // command executed
            delay(100);                             // slight delay IMPORTANT
        }
        client.stop();
        return true;                                // command executed
    }
    else
    return false;                                   // command failed
}
