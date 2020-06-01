/*
  Paradigm_controller
  (Arduino controller for behavior paradigm)
  Ver 0.1

  <Functionarities>
  Provide the following output:
  1. Exposure timing for up to three camera (3.3V).
  2. nversed TTL signal for shocker.
  3. information on LED display for frame number, time, shock and tone.
  4. Digital output to recording system, like Open-Ephys.
  5. Analog modulation signal to LED driver, up to two.
  6. Drive speaker.

  <Digital pin connection

  07  Sound_B_VOL+  digital out
  08  Sound_B_VOL-  digital out
  09  Sound_B_UG    digital out
  10  Sound_B_RX    digital out
  11  Sound_B_TX    digital in
  12  Sound_B_RST   digital out
  13  Sound_B_ACT   ditital in

  30  Button_#1     digital in  blue  OK
  32  Button_LED_#1 digital out blue  OK
  34  Button_#2     digital in  red   OK
  36  Button_LED_#2 digital out red   OK

  38  Sound_B_#3    digital out (pull up) OK
  40  Sound_B_#4    digital out (pull up) OK
  42  Sound_B_#5    digital out (pull up) OK
  44  Sound_B_#6    digital out (pull up) OK
  46  Sound_B_#7    digital out (pull up) OK
  48  Sound_B_#8    digital out (pull up) OK
  50  Sound_B_#9    digital out (pull up) OK
  52  Sound B #10   digital out (pull up) OK

  31  Camera_#1     digital out (3.3V)  OK
  33  Camera_#2     digital out (3.3V)  OK
  35  Camera_#3     digital out (3.3V)  OK

  37  Shocker       digital out (inverse) OK

  39  OE_IO_#1      digital out OK
  41  OE_IO_#2      digital out
  43  OE_IO_#3      digital out
  45  OE_IO_#4      digital out
  47  OE_IO_#5      digital out
  49  OE_IO_#6      digital out
  51  OE_IO_#7      digital out
  53  OE_IO_#8      digital out
*/
//############################################################################
// Initialize libraries
//
//  Confirm using Include Library/Manage Libraries
//    Adafruit_LED_Backpack       1.16
//    Adafruit-GFX-Library        1.15
//    Adafruit Soundboard library 1.0.1
//    SimpleCLI                   1.0.9
//############################################################################
// Alphanumeric display LED backpack kits
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
Adafruit_AlphaNum4 alpha4[3] = {Adafruit_AlphaNum4(), Adafruit_AlphaNum4(), Adafruit_AlphaNum4()};

//####################################
// Initialize to use UART to communicate to Adafruit Audio FX Sound Board
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 11
#define SFX_RX 10
// Connect to the RST pin on the Sound Board
#define SFX_RST 12
// we'll be using software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

//####################################
// Initialize SimpleCLI command line interface
//    See Github spacehuhn/SimpleCLI
//    Need to install SimpleCLI library
#include <SimpleCLI.h>
SimpleCLI cli;
Command cmdSetVerbose;
Command cmdSetParadigm;
Command cmdSetTone;
Command cmdSetCamera;
Command cmdStart;

//############################################################################
// Global variables and constants
//############################################################################
// digital pin assignment
const int Sound_B_Cont[7] = {7, 8, 9, 10, 11, 12, 13};
const int Sound_B_Cont_OutMode[7] = {1, 1, 1, 1, 0, 1, 0};
const int Sound_B_VOL_Plus = Sound_B_Cont[0];
const int Sound_B_VOL_Minus = Sound_B_Cont[1];
const int Sound_B_UG = Sound_B_Cont[2];
const int Sound_B_RX = Sound_B_Cont[3];
const int Sound_B_TX = Sound_B_Cont[4];
const int Sound_B_RST = Sound_B_Cont[5];
const int Sound_B_ACT = Sound_B_Cont[6];

const int Button[2] = {30, 34};
const int Button_LED[2] = {32, 36};
const int Sound_B[8] = {38, 40, 42, 44, 46, 48, 50, 52};
const int Camera[3] = {31, 33, 35};
const int Shocker = 37;
const int OE_IO[8] = {39, 41, 43, 45, 47, 49, 51, 53};

//####################################
// global variables
long t1;
long currentTime = millis();

long loopMonitorCount1 = 0;
long u_currentTime = micros();
long u_previousTime;
int u_loopTime;
long ms100;
long ms10;
long ms1;
long us100;
long us10;
int elapsedTime;
bool verboseStatus = true;

// buttonLed
unsigned long buttonLedTime[2] = {millis() - 2000, millis() - 2000};
bool buttonLedBool[2] = {false, false};
int buttonLedPin[2] = {32, 36};

// camera
unsigned long cameraTime[3] = {micros() - 30000, micros() - 30000, micros() - 30000};
unsigned long cameraExposure = 20000;      // Default exposure time: 20 ms
unsigned long cameraExpInterval = 33333;   // Default fps: 30
int camerasNumber = 1;
int cameraStatus = 0;
bool cameraBool[3] = {false, false, false};

// loopMonitor
int long loopMonitorCount = 0;
unsigned long loopMonitorTime = micros();;
unsigned long loopMonitorDurationMax = 0;
unsigned long loopMonitorDurationMin = 1000;
unsigned long loopMonitorDurationAverage;

//waveGen
unsigned long waveGenTime = micros();

// menuCommands
int menuCommandsStatus = 0;

// detectButtonPressed
int buttonState = HIGH;
int transientButtonState = HIGH;
unsigned long lastDebounceTime = millis();
unsigned long debounceDelay = 50;

// generateTone
int generateToneStatus = 0;
int generateToneEpoch = 0;

// triggerShocker
int triggerShockerStatus = 0;
int triggerShockerEpoch = 0;

//####################################
// Strings for display
char dispWelcome1[4] = {' ', ' ', 'W', 'E'};
char dispWelcome2[4] = {'L', 'C', 'O', 'M'};
char dispWelcome3[4] = {'E', ' ', ' ', ' '};

char dispClear1[4] = {' ', ' ', ' ', ' '};
char dispClear2[4] = {' ', ' ', ' ', ' '};
char dispClear3[4] = {' ', ' ', ' ', ' '};


char dispStart1[4] = {'P', 'R', 'E', 'S'};
char dispStart2[4] = {'S', ' ', 'B', 'U'};
char dispStart3[4] = {'T', 'T', 'O', 'N'};

// dispStatus
int dispStatusId = 0;
char dispStatusStrings[4][4] = {' ', ' ', ' ', ' ',
                                'T', ' ', ' ', ' ',
                                ' ', 'S', ' ', ' ',
                                'T', 'S', ' ', ' '
                               };

// paradigm table in millisecond.
long none = -1;
long paradigmStart = 0;
long paradigmStartTime = 0;

String paradigmName;
long paradigmEnd = none;

long tone_start[5] =         {none, none, none, none, none};
long tone_end[5] =           {none, none, none, none, none};

long shocker_start[5] =      {none, none, none, none, none};
long shocker_end[5] =        {none, none, none, none, none};

long LED_Driver_1_start[5] = {none, none, none, none, none};
long LED_Driver_1_end[5] =   {none, none, none, none, none};
long LED_Driver_2_start[5] = {none, none, none, none, none};
long LED_Driver_2_end[5] =   {none, none, none, none, none};



//############################################################################
// void setup()
//############################################################################
void setup() {
  // Initialize the USB serial connection with the PC at 115200 baud
  Serial.begin( 115200 );

  // Initialize digital pins
  initalizeDigitalPin();

  // Initialize the three quad alphanumeric display LED backpack kits
  alpha4[0].begin(0x70);  // pass in the address
  alpha4[1].begin(0x71);  // pass in the address
  alpha4[2].begin(0x72);  // pass in the address
  alpha4[0].setBrightness(0);
  alpha4[1].setBrightness(0);
  alpha4[2].setBrightness(0);

  // To make sure the DAC goes to high speed mode
  Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kMHz

  // Command line interface
  // Command "set_verbose"
  cmdSetVerbose = cli.addCommand("set_verbose", set_verboseCallback);
  cmdSetVerbose.addFlagArgument("on");
  cmdSetVerbose.addFlagArgument("off");

  // Command "set_paradigm"
  cmdSetParadigm = cli.addCommand("set_paradigm", set_paradigmCallback);
  cmdSetParadigm.addFlagArgument("tone");
  cmdSetParadigm.addFlagArgument("shocker");
  cmdSetParadigm.addFlagArgument("clear");
  cmdSetParadigm.addFlagArgument("list");
  cmdSetParadigm.addArgument("epoch", "-1");
  cmdSetParadigm.addArgument("start", "-1");
  cmdSetParadigm.addArgument("end", "-1");
  cmdSetParadigm.addArgument("paradigm_duration", "-1");
  cmdSetParadigm.addArgument("paradigm_name", "");

  // Command "set_tone"
  cmdSetTone = cli.addCommand("set_tone", set_toneCallback);
  cmdSetTone.addFlagArgument("UART");
  cmdSetTone.addArgument("tone_volume", "-1");

  // Command "set_camera"
  cmdSetCamera = cli.addCommand("set_camera", set_cameraCallback);
  cmdSetCamera.addFlagArgument("start");
  cmdSetCamera.addFlagArgument("stop");
  cmdSetCamera.addFlagArgument("status");
  cmdSetCamera.addArgument("cameras_number", "-1");
  cmdSetCamera.addArgument("fps", "-1");
  cmdSetCamera.addArgument("exposure", "-1");

  // Command "start"
  cmdStart = cli.addCommand("start", start_Callback);
  cmdStart.addPositionalArgument("str", "");
  cmdStart.addFlagArgument("button");

  // In case of error
  cli.setOnError(errorCallback);

  // Display "Welcome"
  ledChar(0, dispWelcome1);
  ledChar(1, dispWelcome2);
  ledChar(2, dispWelcome3);

  // Rest Adafruit Audio FX Sound Board
  // resetSound_B("UART");
  resetSound_B("GPIO");

  Serial.println("Welcome to PCBox: Paradigm Controling Box");
  // Serial.print("> ");
}

//############################################################################
// void loop()
//############################################################################
void loop() {

  // Command line interface
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    if (verboseStatus) {
      Serial.print("> ");
      Serial.println(input);
    }
    cli.parse(input);

    // Serial.print("> ");
  }

  // Generate triger for three cameras
  if (cameraStatus) {
    for (int i = 0; i < 3; i++) {
      camera(i); // currently 30 ms interval.
    }
  }

  // Generate sinusoidal LED driving signals


}

//############################################################################
// Command interpreter
//############################################################################
void errorCallback(cmd_error* errorPtr) {
  CommandError e(errorPtr);

  Serial.println("ERROR: " + e.toString());

  if (e.hasCommand()) {
    Serial.println("Did you mean? " + e.getCommand().toString());
  } else {
    Serial.println(cli.toString());
  }
}

void set_paradigmCallback(cmd* cmdPtr) {
  Command cmd(cmdPtr);

  Argument arg = cmd.getArgument("tone");
  bool _paradigmTone = arg.isSet();
  //  Serial.print("_paradigmTone: ");
  //  Serial.println(_paradigmTone);

  arg = cmd.getArgument("shocker");
  bool _paradigmShocker = arg.isSet();

  arg = cmd.getArgument("clear");
  bool _paradigmClear = arg.isSet();

  arg = cmd.getArgument("list");
  bool _paradigmList = arg.isSet();
  //  Serial.print("_paradigmList: ");
  //  Serial.println(_paradigmList);

  arg = cmd.getArgument("epoch");
  int _paradigmEpoch = arg.getValue().toInt();

  arg = cmd.getArgument("start");
  float _paradigmStart = arg.getValue().toFloat();

  arg = cmd.getArgument("end");
  float _paradigmEnd = arg.getValue().toFloat();

  arg = cmd.getArgument("paradigm_duration");
  float _paradigmDuration = arg.getValue().toFloat();

  arg = cmd.getArgument("paradigm_name");
  String _paradigmName = arg.getValue();

  if (_paradigmName != "") {
    paradigmName = _paradigmName;
  }

  if (_paradigmDuration != -1.0) {
    paradigmEnd = long(_paradigmDuration * 1000.0);
    //    Serial.print("paradigmDuration: ");
    //    Serial.println(_paradigmDuration);
  }

  if (_paradigmEpoch != -1) {
    if (_paradigmTone) {
      tone_start[_paradigmEpoch] = long(_paradigmStart * 1000.0);
      tone_end[_paradigmEpoch] = long(_paradigmEnd * 1000.0);
      //      Serial.print("tone: ");
      //      Serial.print(_paradigmEpoch);
      //      Serial.print(" ");
      //      Serial.print(_paradigmStart);
      //      Serial.print(" ");
      //      Serial.println(_paradigmEnd);
    }
    if (_paradigmShocker) {
      shocker_start[_paradigmEpoch] = long(_paradigmStart * 1000.0);
      shocker_end[_paradigmEpoch] = long(_paradigmEnd * 1000.0);
      //      Serial.print("shocker: ");
      //      Serial.print(_paradigmEpoch);
      //      Serial.print(" ");
      //      Serial.print(_paradigmStart);
      //      Serial.print(" ");
      //      Serial.println(_paradigmEnd);
    }
  }

  if (_paradigmClear) {
    paradigmEnd = -1;
    for (int i = 0; i < 5; i++) {
      tone_start[i] = -1;
      tone_end[i] = -1;
      shocker_start[i] = -1;
      shocker_end[i] = -1;
    }
  }

  if (_paradigmList) {
    Serial.print("paradigm name: ");
    Serial.println(paradigmName);
    Serial.print("paradigm duration: ");
    Serial.print(paradigmEnd / 1000.0);
    Serial.println(" sec");
    for (int i = 0; i < 5; i++) {
      Serial.print("tone epoch ");
      Serial.print(i);
      Serial.print(" tone_start: ");
      Serial.print(tone_start[i] / 1000.0);
      Serial.print(" sec, tone_end: ");
      Serial.print(tone_end[i] / 1000.0);
      Serial.println(" sec");
    }
    for (int i = 0; i < 5; i++) {
      Serial.print("shocker epoch ");
      Serial.print(i);
      Serial.print(" shocker_start: ");
      Serial.print(shocker_start[i] / 1000.0);
      Serial.print(" sec, shocker_end: ");
      Serial.print(shocker_end[i] / 1000.0);
      Serial.println(" sec");
    }
  }
}

void set_verboseCallback(cmd * cmdPtr) {
  Command cmd(cmdPtr);
  Argument arg = cmd.getArgument("on");
  bool _on = arg.isSet();
  arg = cmd.getArgument("off");
  bool _off = arg.isSet();

  if (_on) {
    verboseStatus = true;
    Serial.println("Verbose mode ON");
  }
  if (_off) {
    verboseStatus = false;
    Serial.println("Verbose mode OFF");
  }
}

void set_toneCallback(cmd * cmdPtr) {
  Command cmd(cmdPtr);
  Argument arg = cmd.getArgument("tone_volume");
  int tone_volume = arg.getValue().toInt();
  arg = cmd.getArgument("UART");
  bool UART = arg.isSet();

  if (UART) {
    // UART communication to Adafruit Sound Board
    menuCommandsStatus = 1;
    resetSound_B("UART");
    Serial.print("... Searching Adafruit Sound Board ... ");
    // softwareserial at 9600 baud
    ss.begin(9600);
    if (!sfx.reset()) {
      Serial.println("Not found!");
      while (1);
    }
    Serial.println("SFX board found!");

    menuCommands();
  }

  if (tone_volume != -1) {
    //Serial.println(tone_volume);
    resetSound_B("GPIO");
    for (int i = 50; i > tone_volume; i--) {
      //Serial.print(i);
      digitalWrite(Sound_B_VOL_Minus, LOW);
      delay(10);
      digitalWrite(Sound_B_VOL_Minus, HIGH);
      delay(10);
    }
    digitalWrite(Sound_B[1], LOW);
    delay(2000);
    digitalWrite(Sound_B[1], HIGH);
  }
}

void set_cameraCallback(cmd * cmdPtr) {
  Command cmd(cmdPtr);
  Argument arg = cmd.getArgument("start");
  bool _set_cameraStart = arg.isSet();
  if (_set_cameraStart) {
    cameraStatus = 1;
    // Serial.println(cameraStatus);
  }

  arg = cmd.getArgument("stop");
  bool _set_cameraStop = arg.isSet();
  if (_set_cameraStop) {
    cameraStatus = 0;
    // Serial.println(cameraStatus);
  }

  arg = cmd.getArgument("status");
  bool _set_cameraStatus = arg.isSet();
  if (_set_cameraStatus) {
    Serial.print("Camera status: ");
    Serial.print(cameraStatus);
    Serial.print(", number: ");
    Serial.print(camerasNumber);
    Serial.print(", fps: ");
    //    Serial.print(cameraExpInterval);
    //    Serial.print(" : ");
    Serial.print(1000000 / cameraExpInterval);
    Serial.print(", exposure: ");
    Serial.print(cameraExposure / 1000);
    Serial.println(" ms");
  }

  arg = cmd.getArgument("cameras_number");
  int _cameras_number = arg.getValue().toInt();
  if (_cameras_number != -1) camerasNumber = _cameras_number;

  arg = cmd.getArgument("fps");
  int _fps = arg.getValue().toInt();
  if (_fps != -1) {
    cameraExpInterval = long(1000000) / long(_fps);
    //    Serial.print("_fps: ");
    //    Serial.print(_fps);
    //    Serial.print(", cameraExpInterval: ");
    //    Serial.println(cameraExpInterval);
  }

  arg = cmd.getArgument("exposure");
  int _exposure = arg.getValue().toInt();
  if (_exposure != -1) {
    cameraExposure = long(_exposure) * long(1000);
    Serial.print("_exposure: ");
    Serial.print(_exposure);
    Serial.print(", cameraExposure: ");
    Serial.println(cameraExposure);
    if (cameraExposure > cameraExpInterval) {
      cameraExposure = cameraExpInterval;
      Serial.println("ERROR: cameraExposure = cameraExpInterval");
    }
  }
}

void start_Callback(cmd * cmdPtr) {
  Command cmd(cmdPtr);
  Argument arg = cmd.getArgument("str");
  String   subCom = arg.getValue();
  arg = cmd.getArgument("button");
  bool _startButton = arg.isSet();

  if (subCom == "paradigm") paradigmOnGoing(_startButton);

}

//############################################################################
// Major loop controlling paradigm
//############################################################################
void paradigmOnGoing(bool _startButton) {

  Serial.println("Starting paradigm ...");
  Serial.println("... Press Blue Button to start the protocol ...");

  // Display "PressBlueButt"
  ledChar(0, dispStart1);
  ledChar(1, dispStart2);
  ledChar(2, dispStart3);

  while (1) {
    buttonLed(0); // slow interval
    // detect button pressed
    detectButtonPressed(0);
    //##############################
    // For remote maintenance
    if (!_startButton) {
      buttonState = LOW;
    }
    //##############################
    if (buttonState == LOW) {
      buttonState = HIGH;
      digitalWrite(Button_LED[0], LOW);
      break;
    }
  }

  Serial.println("Started !");
  Serial.println("To abort, press RED button.");

  deviceReset();
  // Display Clear

  ledChar(0, dispClear1);
  ledChar(1, dispClear2);
  ledChar(2, dispClear3);

  loopMonitorCount = 0;
  loopMonitorCount1 = -1;
  paradigmStartTime = millis();

  u_previousTime = micros();
  ms100 = 0;
  ms10 = 0;
  ms1 = 0;
  us100 = 0;
  us10 = 0;

  while (1) {
    // Keep monitoring the loop interval
    // loopMonitor(); // fast interval

    currentTime = millis();
    u_currentTime = micros();
    loopMonitorCount1++;

    u_loopTime = u_currentTime - u_previousTime;
    if (u_loopTime / 100000 > 0) {
      ms100++;
    } else if (u_loopTime / 10000 > 0) {
      ms10++;
    } else if (u_loopTime / 1000 > 0) {
      ms1++;
    } else if (u_loopTime / 100 > 0) {
      us100++;
    } else {
      us10++;
    }

    // Display elapsed time
    if (loopMonitorCount1 % 50 == 0) {
      elapsedTime = (currentTime - paradigmStartTime) / 1000;
      ledNum(0, elapsedTime);
    }

    // Generate triger for three cameras
    for (int i = 0; i < 3; i++) {
      camera(i); // currently 30 ms interval.
    }

    // Generate sinosoidal signal for LED driver
    //### waveGen takes >670 micro sec
    //t1 = micros();
    //waveGen();
    //Serial.println(micros() - t1);

    // Generate triger for shocker
    triggerShocker();

    // Generate tone
    generateTone();

    // Detect end of protocol
    if (currentTime - paradigmStartTime > paradigmEnd) {
      Serial.println("Protocol completed !");
      break;
    }

    // Red button pressing aborts the protocol
    buttonLed(1); // slow interval
    // detect button pressed
    detectButtonPressed(1);
    if (buttonState == LOW) {
      Serial.println("Aborted");
      break;
    }

    u_previousTime = u_currentTime;
  }

  deviceReset();
  Serial.print("number of loops: ");
  Serial.println(loopMonitorCount1);

  Serial.print("<100us : ");
  Serial.print(us10);
  Serial.print(", 100us-1ms: ");
  Serial.print(us100);
  Serial.print(", 1ms-10ms: ");
  Serial.print(ms1);
  Serial.print(", 10-100ms: ");
  Serial.print(ms10);
  Serial.print(", 100ms<: ");
  Serial.print(ms100);
}

//############################################################################
// Controlling devices and ports during paradigm
//############################################################################
// Reset PCbox
void deviceReset() {
  // generateTone
  generateToneStatus = 0;
  generateToneEpoch = 0;

  // triggerShocker
  triggerShockerStatus = 0;
  triggerShockerEpoch = 0;

  buttonState = HIGH;
  digitalWrite(Button_LED[1], LOW);
  digitalWrite(Sound_B[1], HIGH);

}

//####################################
// Shocker
void triggerShocker() {
  // long currentTime = millis();

  //  Serial.print(currentTime - paradigmStartTime);
  //  Serial.print(", ");
  //  Serial.println(tone_start[generateToneEpoch]);

  if (shocker_start[triggerShockerEpoch] != none) {
    if (triggerShockerStatus == 0) {

      if (currentTime - paradigmStartTime > shocker_start[triggerShockerEpoch]) {
        digitalWrite(Shocker, HIGH);
        triggerShockerStatus = 1;
        dispStatus();
      }
    } else {
      if (currentTime - paradigmStartTime > shocker_end[triggerShockerEpoch]) {
        digitalWrite(Shocker, LOW);
        triggerShockerStatus = 0;
        dispStatus();
        triggerShockerEpoch++;
      }
    }
  }
}

//####################################
// Tone
void generateTone() {
  // long currentTime = millis();

  //  Serial.print(currentTime - paradigmStartTime);
  //  Serial.print(", ");
  //  Serial.println(tone_start[generateToneEpoch]);

  if (tone_start[generateToneEpoch] != none) {
    if (generateToneStatus == 0) {

      if (currentTime - paradigmStartTime > tone_start[generateToneEpoch]) {
        digitalWrite(Sound_B[1], LOW);
        generateToneStatus = 1;
        dispStatus();
      }
    } else {
      if (currentTime - paradigmStartTime > tone_end[generateToneEpoch]) {
        digitalWrite(Sound_B[1], HIGH);
        generateToneStatus = 0;
        dispStatus();
        generateToneEpoch++;
      }
    }
  }
}

//####################################
// Triggering signal for cameras
void camera(int i) {

  if (cameraBool[i] == true) {
    if ((micros() - cameraTime[i]) > cameraExposure) {
      digitalWrite(Camera[i], LOW);    // turn the LED off by making the voltage LOW
      cameraBool[i] = false;
    }
  } else {
    if ((micros() - cameraTime[i]) > cameraExpInterval) {
      digitalWrite(Camera[i], HIGH);   // turn the LED on (HIGH is the voltage level
      cameraBool[i] = true;
      cameraTime[i] =  micros();
    }
  }
}

//####################################
// Monitoring speed for excuting loop
void loopMonitor() {
  int _loopDuration;
  String stringBuffer;
  unsigned long _loopMonitorDisplay;

  // Loop monitor
  loopMonitorCount++;

  _loopDuration = micros() - loopMonitorTime;
  if (_loopDuration > loopMonitorDurationMax) {
    loopMonitorDurationMax = _loopDuration;
  }
  if (_loopDuration < loopMonitorDurationMin) {
    loopMonitorDurationMin = _loopDuration;
  }
  loopMonitorDurationAverage = loopMonitorDurationAverage * (loopMonitorCount - 1) + _loopDuration;
  loopMonitorDurationAverage =  loopMonitorDurationAverage / loopMonitorCount;

  if (loopMonitorCount == 5000) {
    loopMonitorCount = 0;

    //    ledNum(0, loopMonitorDurationAverage);
    //    ledNum(1, loopMonitorDurationMax);
    //    ledNum(2, loopMonitorDurationMin);


    // remainingTime = (paradigmEnd - (currentTime - paradigmStartTime)) / 1000 ;
    // ledNum(0, remainingTime);

    _loopMonitorDisplay = loopMonitorDurationAverage * 100 + loopMonitorDurationMax / 100;
    ledNum(1, _loopMonitorDisplay);


    //        stringBuffer = "Loop Duration (micro sec), ave:"
    //                       + String(loopMonitorDurationAverage) + " , max:"
    //                       + String(loopMonitorDurationMax) + " , min:"
    //                       + String(loopMonitorDurationMin);
    //        Serial.println(stringBuffer);

    loopMonitorDurationMax = 0;
    loopMonitorDurationMin = 1000;
  }

  loopMonitorTime = micros();
}

//############################################################################
// Adafruit Audio FX Sound Board
//############################################################################
// Reset Adafruit Audio FX Sound Board either UART or GPIO mode
void resetSound_B(String a) {
  if (a == "UART") {
    digitalWrite(Sound_B_UG, LOW);
  }
  if (a == "GPIO") {
    digitalWrite(Sound_B_UG, HIGH);
  }

  // Rest Adafruit Audio FX Sound Board
  digitalWrite(Sound_B_RST, HIGH);
  delay(1000);
  digitalWrite(Sound_B_RST, LOW);
  delay(1000);
  digitalWrite(Sound_B_RST, HIGH);
}
//####################################
// Communicate Audio FX Sound Board through UART
// (Universal asynchronous receiver-transmitter)
void menuCommands() {
  char commandList[12] = {'r', 'L', '#', 'P', '+', '-', '=', '>', 'q', 't', 's', 'x'};
  int validCom = 0;
  char cmd;

  while (menuCommandsStatus) {
    Serial.println(F("### Top > SFX board ###"));
    Serial.println(F("[r] - reset"));
    Serial.println(F("[+] - Vol +"));
    Serial.println(F("[-] - Vol -"));
    Serial.println(F("[L] - List files"));
    Serial.println(F("[P] - play by file name"));
    Serial.println(F("[#] - play by file number"));
    Serial.println(F("[=] - pause playing"));
    Serial.println(F("[>] - unpause playing"));
    Serial.println(F("[q] - stop playing"));
    Serial.println(F("[t] - playtime status"));
    Serial.println(F("[x] - back to Top"));
    Serial.println(F("c> "));

    while (1) {
      while (!Serial.available());
      cmd = Serial.read();
      flushInput ();
      for (int i = 0; i < sizeof(commandList) / sizeof(char); i++) {
        if (cmd == commandList[i]) {
          validCom = 1;
          break;
        }
      }
      if (validCom == 1) {
        validCom = 0;
        break;
      }
    }

    switch (cmd) {
      case 'r': {
          if (!sfx.reset()) {
            Serial.println("Reset failed");
          }
          break;
        }

      case 'L': {
          uint8_t files = sfx.listFiles();

          Serial.println("File Listing");
          Serial.println("========================");
          Serial.println();
          Serial.print("Found "); Serial.print(files); Serial.println(" Files");
          for (uint8_t f = 0; f < files; f++) {
            Serial.print(f);
            Serial.print("\tname: "); Serial.print(sfx.fileName(f));
            Serial.print("\tsize: "); Serial.println(sfx.fileSize(f));
          }
          Serial.println("========================");
          break;
        }

      case '#': {
          Serial.print("Enter track #");
          uint8_t n = readnumber();

          Serial.print("\nPlaying track #"); Serial.println(n);
          if (! sfx.playTrack((uint8_t)n) ) {
            Serial.println("Failed to play track?");
          }
          break;
        }

      case 'P': {
          Serial.print("Enter track name (full 12 character name!) >");
          char name[20];
          readline(name, 20);

          Serial.print("\nPlaying track \""); Serial.print(name); Serial.print("\"");
          if (! sfx.playTrack(name) ) {
            Serial.println("Failed to play track?");
          }
          break;
        }

      case '+': {
          Serial.println("Vol up...");
          uint16_t v;
          if (! (v = sfx.volUp()) ) {
            Serial.println("Failed to adjust");
          } else {
            Serial.print("Volume: "); Serial.println(v);
          }
          break;
        }

      case '-': {
          Serial.println("Vol down...");
          uint16_t v;
          if (! (v = sfx.volDown()) ) {
            Serial.println("Failed to adjust");
          } else {
            Serial.print("Volume: ");
            Serial.println(v);
          }
          break;
        }

      case '=': {
          Serial.println("Pausing...");
          if (! sfx.pause() ) Serial.println("Failed to pause");
          break;
        }

      case '>': {
          Serial.println("Unpausing...");
          if (! sfx.unpause() ) Serial.println("Failed to unpause");
          break;
        }

      case 'q': {
          Serial.println("Stopping...");
          if (! sfx.stop() ) Serial.println("Failed to stop");
          break;
        }

      case 't': {
          Serial.print("Track time: ");
          uint32_t current, total;
          if (! sfx.trackTime(&current, &total) ) Serial.println("Failed to query");
          Serial.print(current); Serial.println(" seconds");
          break;
        }

      case 's': {
          Serial.print("Track size (bytes remaining/total): ");
          uint32_t remain, total;
          if (! sfx.trackSize(&remain, &total) )
            Serial.println("Failed to query");
          Serial.print(remain); Serial.print("/"); Serial.println(total);
          break;
        }

      case 'x': {
          Serial.print("... Exiting command mode ... ");
          resetSound_B("GPIO"); // put Sound_B_RST = LOW
          menuCommandsStatus = 0;
          // Need to reset from the command mode
          if (!sfx.reset()) {
            Serial.println("Reset failed");
          }
          Serial.println("Exited!");
          break;
        }
    }
  }
}
/************************ MENU HELPERS ***************************/

void flushInput() {
  // Read all available serial input to flush pending data.
  uint16_t timeoutloop = 0;
  while (timeoutloop++ < 40) {
    while (ss.available()) {
      ss.read();
      timeoutloop = 0;  // If char was received reset the timer
    }
    delay(1);
  }
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}

uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff) {
  uint16_t buffidx = 0;

  while (true) {
    if (buffidx > maxbuff) {
      break;
    }

    if (Serial.available()) {
      char c =  Serial.read();
      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0) {  // the first 0x0A is ignored
          continue;
        }
        buff[buffidx] = 0;  // null term
        return buffidx;
      }
      buff[buffidx] = c;
      buffidx++;
    }
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
/************************ MENU HELPERS ***************************/

//############################################################################
// AD5667 16-BIT 2-CHANNEL DIGITAL TO ANALOG CONVERTER
//############################################################################
// Write DAC at 1kHz speed
void waveGen() {

  const long sampling = 1000; // every 1000 micro sec (1kHz)
  const float pi = 3.14159267;
  const long freq = 50; // kHz
  const long waveLen = 1000000 / freq; // micro sec

  long current = micros();
  float voltage;
  float radian;
  long t1;

  if ((current - waveGenTime) > sampling) {

    //### These floating calculation takes >200 micro sec
    //t1 = micros();
    radian = float(current % waveLen) / float(waveLen) * 2.0 * pi;
    voltage = (sin(radian) + 1.0) * 2.499999;
    //Serial.println(micros() - t1);

    //    Serial.print(current);
    //    Serial.print(" : ");
    //    Serial.print(current % waveLen);
    //    Serial.print(" : ");
    //    Serial.print(radian);
    //    Serial.print(" : ");
    //    Serial.println(voltage);

    //### dacWrite takes >230 micro sec
    //t1 = micros();
    dacWrite(0, voltage);
    //Serial.println(micros() - t1);

    //### dacWrite takes >230 micro sec
    //t1 = micros();
    dacWrite(1, voltage);
    //Serial.println(micros() - t1);

    waveGenTime = micros();
  }
}

/************************ waveGen HELPERS ***************************/
void dacWrite(int dacId, float voltage) {
  // #define Addr 0x08 (8)
  // #define Addr 0x0E (14)
  int Addr = 8;
  int command = 24;
  int commandDacId;
  long voltage16 = long(voltage / 5.0 * 65536.0);
  int data[2] = {int(voltage16 / 256), int(voltage16 % 256)};

  // Start I2C transmission
  Wire.beginTransmission(Addr);
  // 1st byte: Command and select DAC
  //    00 XXX YYY (X:Command, Y:Address)
  //    Command 0001 1000: Update DAC -> 0x18
  //    Address 0000 0000: DAC A      -> 0x00
  //    Address 0000 0001: DAC B      -> 0x01
  //    Address 0000 0111: Both DACs  -> 0x07
  // Add the command and address
  //    Example: 0x18 + 0x07 = 0x1F
  commandDacId = command + dacId;
  Wire.write(commandDacId);
  Wire.write(data[0]);
  Wire.write(data[1]);
  // Stop I2C transmission
  Wire.endTransmission();
}
/************************ waveGen HELPERS ***************************/

//############################################################################
// Quad Alphanumeric Display
//############################################################################
// Display device status (tone, shock) on Quad Alphanumeric Display
void dispStatus() {
  char _dispStatusStrings[4];

  dispStatusId = triggerShockerStatus * 2 + generateToneStatus;

  for (int i; i < 4; i++) {
    _dispStatusStrings[i] = dispStatusStrings[dispStatusId][i];
  }

  ledChar(2, _dispStatusStrings);
}

void ledChar(int ledId, char a[4]) {
  for (int i = 0; i < 4; i++) {
    alpha4[ledId].writeDigitAscii(i, a[i]);
  }
  alpha4[ledId].writeDisplay();
}

void ledNum(int ledId, int value) {
  for (int i = 0; i < 4; i++) {
    alpha4[ledId].writeDigitAscii(3 - i, value % 10 + 48);
    value = value / 10;
  }
  alpha4[ledId].writeDisplay();
}

//############################################################################
// Initialize IO pins
//############################################################################
void initalizeDigitalPin() {
  // initialize digital pins.

  // Sound_B_Cont
  for (int i = 0; i < sizeof(Sound_B_Cont) / sizeof(int); i++) {
    if (Sound_B_Cont_OutMode[i] == 1) {
      pinMode(Sound_B_Cont[i], OUTPUT);
      digitalWrite(Sound_B_Cont[i], HIGH);
    } else {
      pinMode(Sound_B_Cont[i], INPUT);
    }
  }
  // Button
  for (int i = 0; i < sizeof(Button) / sizeof(int); i++) {
    pinMode(Button[i], INPUT);
  }
  // Button_LED
  for (int i = 0; i < sizeof(Button_LED) / sizeof(int); i++) {
    pinMode(Button_LED[i], OUTPUT);
    digitalWrite(Button_LED[i], LOW);
  }
  // Sound_B
  for (int i = 0; i < sizeof(Sound_B) / sizeof(int); i++) {
    pinMode(Sound_B[i], OUTPUT);
    digitalWrite(Sound_B[i], HIGH);
  }
  // Shocker
  pinMode(Shocker, OUTPUT);
  digitalWrite(Shocker, LOW);
  //Camera
  for (int i = 0; i < sizeof(Camera) / sizeof(int); i++) {
    pinMode(Camera[i], OUTPUT);
    digitalWrite(Camera[i], LOW);
  }
  // OE_IO
  for (int i = 0; i < sizeof(OE_IO) / sizeof(int); i++) {
    pinMode(OE_IO[i], OUTPUT);
    digitalWrite(OE_IO[i], LOW);
  }
}
//############################################################################
// Buttons
//############################################################################
void detectButtonPressed(int i) {
  // Keep monitoring button
  int reading = digitalRead(Button[i]);

  // If the switch changed, whatever due to noise or pressing, reset debounce timer
  if (reading != transientButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
    transientButtonState = reading;
  }

  // If the new button state lasts more than debounceDelay, change the lastButtonState
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState != reading) {
      buttonState = reading;
    }
  }

  //    Serial.print(reading);
  //    Serial.print(", ");
  //    Serial.println(buttonState);
}

//############################################################################
// LEDs in Button
//############################################################################
void buttonLed(int buttonLedId) {
  if (buttonLedBool[buttonLedId] == true) {
    if (millis() - buttonLedTime[buttonLedId] > 1000) {
      digitalWrite(buttonLedPin[buttonLedId], LOW);    // turn the LED off by making the voltage LOW
      buttonLedBool[buttonLedId] = false;
    }
  } else {
    if (millis() - buttonLedTime[buttonLedId] > 2000) {
      digitalWrite(buttonLedPin[buttonLedId], HIGH);   // turn the LED on (HIGH is the voltage level
      buttonLedBool[buttonLedId] = true;
      buttonLedTime[buttonLedId] =  millis();
    }
  }
}
//############################################################################
