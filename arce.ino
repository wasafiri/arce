// Hue variables
TCPClient client;
const char hueHubIP[] = "192.168.1.16";     // local HUE bridge
const int hueHubPort = 80;                  // local HUE bridge port
const char hueUsername[] = "newdeveloper";  // hue username
int lightNum;                               // which HUE light we want to control
unsigned int len;                           // to store length of String command sent to HUE

// read the buzzer value
int buzzerPin = D3;                         // pin the buzzer is connected to
int buzzerState;                            // current reading from the input pin

// debouncing variables to check button twice before doing anything
long lastDebounceTime = 0;                  // the last time the output pin was toggled
long debounceDelay = 120;                   // the debounce time; increase if multiple texts are sent
bool buttonWasPressed = false;

void setup() {
    // read buzzerPin
    pinMode(buzzerPin, INPUT_PULLUP);
    attachInterrupt(D3, interrupt, FALLING);
    
    // turn on serial communication
    Serial.begin(9600);
    delay(200);
}

void interrupt() {
    if ((millis() - lastDebounceTime) > debounceDelay) {
        buttonWasPressed = true;
    }
}

void loop() {
    // if the button was pressed, send a text via Twilio + do stuff with the lights
    if (buttonWasPressed == true) {

        String command = "{\"on\": true,\"hue\":50000,\"sat\":254,\"bri\":180,\"alert\":\"none\",\"transitiontime\":40}";
        setHue(lightNum, command);
        delay(10000);
        // turning HUE off after alert produces nicer gradual off than by relying on 'else' to turn it off
        command = "{\"on\": false}";
        setHue(lightNum, command);
        Serial.println("command sent to HUE ");     // command executed
        
        // use a webhook to send json to Twilio
        Particle.publish("twilio", "Heads up: Someone just buzzed your intercom!", 60, PRIVATE);
        delay(20);
        buttonWasPressed = false;
    }
    else {
        // turn HUE off if for some reason it's on, else it will stay on until someone pushes the buzzer
        String command = "{\"on\": false}";
        setHue(lightNum, command);
    }
}

boolean setHue(int lightNum, String command) {      // moving this out of the loop
    if (client.connect(hueHubIP, hueHubPort)) {
        unsigned int len = command.length();        // get length
        client.println("PUT /api/newdeveloper/lights/3/state HTTP/1.1");
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
    else {
        Serial.println("hue bridge not connected");
    }
}
