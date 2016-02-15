// Hue variables
TCPClient client;
const char hueHubIP[] = "192.168.1.16";     // local HUE bridge
const int hueHubPort = 80;                  // local HUE bridge port
const char hueUsername[] = "newdeveloper";  // hue username
int lightNum = 3;                           // which HUE light we want to control
unsigned int len;                           // length of string sent to HUE
boolean hueOn;                              // store returned on/off info
int hueBri;                                 // store returned brightness value
long hueHue;                                // store returned hue value
bool hueOffValue = 0;                       // when off, getHue will return 0 in hueOn
bool hueOnValue = 1;                        // when on, getHue will return 1 in hueOn

// read the buzzer value
int buzzerPin = D6;                         // pin the buzzer is connected to
int buzzerState;                            // current reading from the input pin

// debouncing variables to check pin state twice before doing anything
long lastDebounceTime = 0;                  // the last time the output pin was toggled
long debounceDelay = 20;                    // the debounce time; increase if multiple texts are sent
bool buttonWasPressed = false;

// keep tabs on elapsed time
unsigned long previousMillis = 0;           // start counting time from zero
long interval = 5000;                       // gonna want to turn the HUEs off every 5 seconds

// use internal led for visual event notification
int led = D7;

void setup() {
    // read buzzerPin
    pinMode(buzzerPin, INPUT_PULLDOWN);
    attachInterrupt(D6, interrupt, RISING);
    
    // initialize D7 pin as output
    pinMode(led, OUTPUT);
    
    // turn on serial communication
    Serial.begin(9600);
}

void interrupt() {
    if ((millis() - lastDebounceTime) > debounceDelay) {
        buttonWasPressed = true;
    }
}

void loop() {                               
    unsigned long currentMillis = millis();
    // if the button was pressed, send a text via Twilio + do stuff with the lights
    if (buttonWasPressed == true) {
        digitalWrite(led, HIGH);
        
        // use a webhook to send json to Twilio
        Particle.publish("plivo_sms", "Heads up: Someone just buzzed your intercom!", 60, PRIVATE);
        
        if (client.connect(hueHubIP, hueHubPort)) {
            String command = "{\"on\": true,\"hue\":50000,\"sat\":254,\"bri\":180,\"alert\":\"none\",\"transitiontime\":40}";
            setHue(lightNum, command);
            delay(1000);                             // give the data time to reach HUE
            getHue(lightNum);
            delay(1600);                             // give the data time to come in
            if (hueOn == 0) {
                String command = "{\"on\": true,\"hue\":50000,\"sat\":254,\"bri\":180,\"alert\":\"none\",\"transitiontime\":40}";
                setHue(lightNum, command);
                Serial.println("Hue wasn't on, turning it on!");
                getHue(lightNum);
            }
            delay(10000);                           // leave lights on for 10 seconds
            getHue(lightNum);
            // turning HUE off after alerting produces nicer gradual off than by relying on 'else' to turn it off
            command = "{\"on\": false}";
            setHue(lightNum, command);
        }
        digitalWrite(led, LOW);
        buttonWasPressed = false;
    }
    else {
        if (currentMillis - previousMillis > interval) {
            previousMillis = currentMillis;
            String command = "{\"on\": false}";
            setHue(lightNum, command);
        }
    }
}

boolean setHue(int lightNum, String command) {      // moving this out of the loop
    unsigned int len = command.length();            // get length of command (will need it later)
    if (client.connect(hueHubIP, hueHubPort)) {     // for some reason the bulbs do not react reliably
        boolean currentLineIsBlank = true;
        while (client.connected()) {                // unless this is done done twice?
            client.print("PUT /api/");
            client.print(hueUsername);
            client.print("/lights/");
            client.print(lightNum);  // hueLight zero based, add 1
            client.println("/state HTTP/1.1");
            client.println("Connection: keep-alive");
            client.print("Host: ");
            client.println(hueHubIP);
            client.println("Content-Type: text/plain;charset=UTF-8");
            client.print("Content-Length: ");
            client.println(len);                        // brightness string + val length
            client.println();                           // blank line before body
            client.print(command);
            delay(100);                                 // slight delay IMPORTANT
            client.stop();
        }
    }
    else if (!client.connected()) {
        Serial.println("hue bridge not connected");
    }
}

boolean getHue(int lightNum) {
    if (client.connect(hueHubIP, hueHubPort)) {
        client.print("GET /api/");
        client.print(hueUsername);
        client.print("/lights/");
        client.print(lightNum);  
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(hueHubIP);
        client.println("Content-type: application/json");
        client.println("keep-alive");
        client.println();
        while (client.connected()) {
            if (client.available()) {
                client.findUntil("\"on\":", "\0");
                hueOn = (client.readStringUntil(',') == "true");  // if light is on, set variable to true
 
                client.findUntil("\"bri\":", "\0");
                hueBri = client.readStringUntil(',').toInt();  // set variable to brightness value
 
                client.findUntil("\"hue\":", "\0");
                hueHue = client.readStringUntil(',').toInt();  // set variable to hue value
        
                break;  // not capturing other light attributes yet
            }
        }
    client.stop();
    }
    else if (!client.connected()) {
        Serial.println("hue bridge not connected");
    }
}
