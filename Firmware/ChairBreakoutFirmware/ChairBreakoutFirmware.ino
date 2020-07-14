/**
 * Firmware for the Chair Breakout Mini by SuperHouse Automation Pty Ltd
 *
 * Reads from:
 *  1. Button box
 *  2. CAN bus
 *  3. Wheelchair joystick
 *
 * Sends to:
 *  1. Keyboard emulation on USB with keypresses
 *  2. Transmit events over CAN bus to other connected devices
 *  3. Game controller emulation on USB with both joystick and button events on CAN
 *
 * By Chris Fryer (chris.fryer78@gmail.com) and Jonathan Oxer (jon@superhouse.tv)
 * www.superhouse.tv
 **/

#include <Joystick.h>    // Joystick emulation: https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Keyboard.h>    // Keyboard emulation
#include <CAN.h>         // CAN bus communication: https://github.com/sandeepmistry/arduino-CAN (available in library manager)

// Configuration should be done in the included file:
#include "config.h"

// Current input values:
int button1Value = 0;
int button2Value = 0;
int button3Value = 0;
int button4Value = 0;
int button5Value = 0;

// Previous input values:
int button1PreviousValue = 0;
int button2PreviousValue = 0;
int button3PreviousValue = 0;
int button4PreviousValue = 0;
int button5PreviousValue = 0;

byte joystick_present = false;
int xAxisDelta        = 0;
int yAxisDelta        = 0;
int joystick_reading  = 0;
int throttle_position = 0;
int vRefValue         = 0;

long last_serial_report_time = 0;
long last_can_report_time    = 0;

char can_msg[12];  // Buffer for CAN messages
char incoming_message_buffer[12];  // Buffer for incoming CAN messages

// Configure the virtual joystick device
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  1, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, true,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering


/**
 * Setup
 */
void setup() {
  Serial.begin(19200);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  delay(2500); // Delay to allow serial port time to connect if it's present,
               // but continue anyway if it's not
  Serial.println("Starting");

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  pinMode(BUTTON_5_PIN, INPUT_PULLUP);

  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  } else {
    Serial.println("Started CAN OK");
  }

  int inputValue = analogRead(BUTTON_1_PIN);
  if(inputValue > JOYSTICK_PRESENT_LEVEL)
  {
    joystick_present = true;
  }

  if(joystick_present)
  {
    Serial.println(F("Joystick mode"));
    // Initialise joystick emulation
    Joystick.begin();
    Joystick.setXAxisRange(-JOYSTICK_AXIS_RANGE, JOYSTICK_AXIS_RANGE);
    Joystick.setYAxisRange(-JOYSTICK_AXIS_RANGE, JOYSTICK_AXIS_RANGE);

    Joystick.setThrottleRange(-127, 127);
  } else {
    // Initialise keyboard emulation
    Serial.println(F("Keyboard mode"));
    Keyboard.begin();
  }
  Serial.println("Ready");
}

/**
 * Loop
 */
void loop(){
  // Check for CAN messages
  processCAN();

  if(joystick_present)
  {
    read_joystick();
  } else {
    read_buttons();
  }
  report_via_serial();
}

/**
 * 
 */
void report_via_serial()
{
  long current_time = millis();
  if((current_time - last_serial_report_time) > SERIAL_REPORT_INTERVAL)
  {
    Serial.print("T:");
    Serial.println(throttle_position);
    last_serial_report_time = current_time;
  }
}

/**
 * 
 */
void read_buttons()
{
  // read the state of the pushbutton value:
  button1Value = analogRead(BUTTON_1_PIN);
  button2Value = analogRead(BUTTON_2_PIN);
  button3Value = analogRead(BUTTON_3_PIN);
  button4Value = analogRead(BUTTON_4_PIN);  
  button5Value = analogRead(BUTTON_5_PIN);

  // Detect transition from unpressed to pressed
  if ((button1Value <= BUTTON_TRIGGER_LEVEL) && (button1PreviousValue > BUTTON_TRIGGER_LEVEL)){
    Keyboard.press('w');

    can_msg[0] = 'w';
    can_msg[1] = '+'; // key press
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: w press"));
    }
  }
  // Detect transition from pressed to unpressed
  if ((button1Value > BUTTON_TRIGGER_LEVEL) && (button1PreviousValue <= BUTTON_TRIGGER_LEVEL)){
    Keyboard.release('w');

    can_msg[0] = 'w';
    can_msg[1] = '-'; // key release
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: w release"));
    }
  }
  button1PreviousValue = button1Value;


  // Detect transition from unpressed to pressed
  if ((button2Value <= BUTTON_TRIGGER_LEVEL) && (button2PreviousValue > BUTTON_TRIGGER_LEVEL)){
    Keyboard.press('z');

    can_msg[0] = 'z';
    can_msg[1] = '+'; // key press
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: z press"));
    }
  }
  // Detect transition from pressed to unpressed
  if ((button2Value > BUTTON_TRIGGER_LEVEL) && (button2PreviousValue <= BUTTON_TRIGGER_LEVEL)){
    Keyboard.release('z');

    can_msg[0] = 'z';
    can_msg[1] = '-'; // key release
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: z release"));
    }
  }
  button2PreviousValue = button2Value;




  // Detect transition from unpressed to pressed
  if ((button3Value <= BUTTON_TRIGGER_LEVEL) && (button3PreviousValue > BUTTON_TRIGGER_LEVEL)){
    Keyboard.press('t');

    can_msg[0] = 't';
    can_msg[1] = '+'; // key press
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: t press"));
    }
  }
  // Detect transition from pressed to unpressed
  if ((button3Value > BUTTON_TRIGGER_LEVEL) && (button3PreviousValue <= BUTTON_TRIGGER_LEVEL)){
    Keyboard.release('t');

    can_msg[0] = 't';
    can_msg[1] = '-'; // key release
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: t release"));
    }
  }
  button3PreviousValue = button3Value;



  // Detect transition from unpressed to pressed
  if ((button4Value <= BUTTON_TRIGGER_LEVEL) && (button4PreviousValue > BUTTON_TRIGGER_LEVEL)){
    Keyboard.press('u');

    can_msg[0] = 'u';
    can_msg[1] = '+'; // key press
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: u press"));
    }
  }
  // Detect transition from pressed to unpressed
  if ((button4Value > BUTTON_TRIGGER_LEVEL) && (button4PreviousValue <= BUTTON_TRIGGER_LEVEL)){
    Keyboard.release('u');

    can_msg[0] = 'u';
    can_msg[1] = '-'; // key release
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: u release"));
    }
  }
  button4PreviousValue = button4Value;



    // Detect transition from unpressed to pressed
  if ((button5Value <= BUTTON_TRIGGER_LEVEL) && (button5PreviousValue > BUTTON_TRIGGER_LEVEL)){
    Keyboard.press('v');

    can_msg[0] = 'v';
    can_msg[1] = '+'; // key press
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: v press"));
    }
  }
  // Detect transition from pressed to unpressed
  if ((button5Value > BUTTON_TRIGGER_LEVEL) && (button5PreviousValue <= BUTTON_TRIGGER_LEVEL)){
    Keyboard.release('v');

    can_msg[0] = 'v';
    can_msg[1] = '-'; // key release
    CAN.beginPacket(0x12);      // Packet ID 0x12 for X axis
    for(int i = 0; i< 12; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    if(BUTTONS_DEBUG)
    {
      Serial.println(F("Button: v release"));
    }
  }
  button5PreviousValue = button5Value;




  /*
  // Detect transition from unpressed to pressed
  if ((button2Value <= BUTTON_TRIGGER_LEVEL) && (button2PreviousValue > BUTTON_TRIGGER_LEVEL)){
    Keyboard.press('z');
    button2PreviousValue = button2Value;
  }
  // Detect transition from pressed to unpressed
  if ((button2Value > BUTTON_TRIGGER_LEVEL) && (button2PreviousValue <= BUTTON_TRIGGER_LEVEL)){
    Keyboard.release('z');
    button2PreviousValue = button2Value;
  }
  */

    /*
  if (button2Value <= BUTTON_TRIGGER_LEVEL){
    Keyboard.press('z');
  }    
        
  if (button3Value <= BUTTON_TRIGGER_LEVEL){
    Keyboard.press('t');
  } */
    
    /* if (buttonValue4 <= BUTTON_TRIGGER_LEVEL){
      Keyboard.press('s');
    }

    if (buttonValue5 <= BUTTON_TRIGGER_LEVEL){
      Keyboard.release('u');
    } */
    
  //delay(50);
    
  /* if (button1Value > BUTTON_TRIGGER_LEVEL){
    Keyboard.release('w');
  } */
    /*
  if (button2Value > BUTTON_TRIGGER_LEVEL){
    Keyboard.release('z');
  }    
        
  if (button3Value > BUTTON_TRIGGER_LEVEL){
    Keyboard.release('t');
  } */
    
    /* if (buttonValue4 > BUTTON_TRIGGER_LEVEL){
      Keyboard.release('s');
    }

    if (buttonValue5 > BUTTON_TRIGGER_LEVEL){
      Keyboard.release('u');
    } */
    
  //delay(50);
}

/**
 * Process messages on CAN bus
 */
void processCAN()
{
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    if(CAN_DEBUG)
    {
      Serial.print("Received packet with id 0x");
      Serial.print(CAN.packetId(), HEX);
      Serial.print(" and length ");
      Serial.println(packetSize);
    }
    // only print packet data for non-RTR packets
    int i = 0;
    if(CAN_DEBUG)
    {
      Serial.print(">");
    }
    while (CAN.available()) {
      //Serial.print((char)CAN.read());
      char msgChar = (char)CAN.read();
      if(CAN_DEBUG)
      {
        Serial.print(msgChar);
      }
      incoming_message_buffer[i] = msgChar;
      i++;
    }
    if(CAN_DEBUG)
    {
      Serial.print("<");
      Serial.println();
    }

    if(!joystick_present)
    {
      if(incoming_message_buffer[1] == '+')
      {
        Keyboard.press(incoming_message_buffer[0]);
      }

      if(incoming_message_buffer[1] == '-')
      {
        Keyboard.release(incoming_message_buffer[0]);
      }
    }


    // Check for specific keys if we're in joystick mode, to use for throttle up and down
    if(incoming_message_buffer[0] == 'v')
    {
      throttle_position = throttle_position + THROTTLE_INCREMENT;

      if(throttle_position > 127)
      {
        throttle_position = 127;
      }
    }
    if(incoming_message_buffer[0] == 'w')
    {
      throttle_position = throttle_position - THROTTLE_INCREMENT;
      if(throttle_position < -127)
      {
        throttle_position = -127;
      }
    }


    /*
    for(int i = 0; i< 4; i++)  // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      Serial.print("-");
      Serial.print(incoming_message_buffer[i]);
      Serial.println("<");
    } */

    //Serial.println();
  }
}


void read_joystick()
{
  long current_time = millis();
  vRefValue = analogRead(VREF_PIN);

  joystick_reading = analogRead(Y_AXIS_PIN);
  /*
  if(reading > (vRefValue + CENTER_DEAD_SPOT) || reading < (vRefValue - CENTER_DEAD_SPOT))
  {
    yAxisDelta = vRefValue - reading;
  }
  */
  if(joystick_reading > (vRefValue + CENTER_DEAD_SPOT))
  {
    yAxisDelta = vRefValue - joystick_reading + CENTER_DEAD_SPOT;
  }
  if(joystick_reading < (vRefValue - CENTER_DEAD_SPOT))
  {
    yAxisDelta = vRefValue - joystick_reading - CENTER_DEAD_SPOT;
  }

  joystick_reading = analogRead(X_AXIS_PIN);
  /* if(reading > (vRefValue + CENTER_DEAD_SPOT) || reading < (vRefValue - CENTER_DEAD_SPOT))
  {
    xAxisDelta = vRefValue - reading;
  } */
  if(joystick_reading > (vRefValue + CENTER_DEAD_SPOT))
  {
    xAxisDelta = vRefValue - joystick_reading + CENTER_DEAD_SPOT;
  }
  if(joystick_reading < (vRefValue - CENTER_DEAD_SPOT))
  {
    xAxisDelta = vRefValue - joystick_reading - CENTER_DEAD_SPOT;
  }

  /* Send output to joystick HID */
  Joystick.setYAxis(yAxisDelta * -1);  // Y axis is reversed
  Joystick.setXAxis(xAxisDelta);
  if(JOYSTICK_DEBUG)
  {
    Serial.println(xAxisDelta);
  }

  Joystick.setThrottle(throttle_position);

  /* Send output to CAN bus */
  if(current_time > (last_can_report_time + CAN_REPORT_INTERVAL))
  {
    int xLength = (xAxisDelta == 0 ? 1 : ((int)(log10(fabs(xAxisDelta))+1) + (xAxisDelta < 0 ? 1 : 0)));
    int yLength = (yAxisDelta == 0 ? 1 : ((int)(log10(fabs(yAxisDelta))+1) + (yAxisDelta < 0 ? 1 : 0)));

    //Serial.println(xLength);
    sprintf(can_msg, "%d", xAxisDelta);
    CAN.beginPacket(0x12);     // Packet ID 0x12 for X axis
    for(int i = 0; i< xLength; i++)
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();

    sprintf(can_msg, "%d", yAxisDelta);
    CAN.beginPacket(0x13);     // Packet ID 0x13 for Y axis
    for(int i = 0; i< yLength; i++)
    {
      CAN.write(can_msg[i]);
    }
    CAN.endPacket();
    last_can_report_time = current_time;
  }
}
