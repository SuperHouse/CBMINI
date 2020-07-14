/**
   Firmware for the Chair Breakout Mini by SuperHouse Automation Pty Ltd

   Reads from:
    1. Button box
    2. CAN bus
    3. Wheelchair joystick

   Sends to:
    1. Keyboard emulation on USB with keypresses
    2. Transmit events over CAN bus to other connected devices
    3. Game controller emulation on USB with both joystick and button events on CAN

   CAN packet IDs:
    0x12: X axis value
    0x13: Y axis value
    0x14: Button events are sent, like "v+" for press of v and "v-" for release of v

   By Chris Fryer (chris.fryer78@gmail.com) and Jonathan Oxer (jon@superhouse.tv)
   www.superhouse.tv
 **/

/*--------------------------- Configuration ------------------------------*/
// Configuration should be done in the included file:
#include "config.h"

/*--------------------------- Libraries ----------------------------------*/
#include <Joystick.h>    // Joystick emulation: https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <Keyboard.h>    // Keyboard emulation
#include <CAN.h>         // CAN bus communication: https://github.com/sandeepmistry/arduino-CAN (available in library manager)

/*--------------------------- Global Variables ---------------------------*/
// Button input
uint16_t g_button_value[5]          = { 0, 0, 0, 0, 0 };       // Analog reading from input
uint16_t g_button_previous_value[5] = { 0, 0, 0, 0, 0 };      // Analog reading from input
uint8_t g_button_pin[5]             = { BUTTON_1_PIN, BUTTON_2_PIN, BUTTON_3_PIN, BUTTON_4_PIN, BUTTON_5_PIN };

// Joystick input
uint8_t g_joystick_present         = false; // Whether a joystick has been detected
uint16_t g_x_axis_delta            = 0;     // Offset of X axis compared to reference voltage
uint16_t g_y_axis_delta            = 0;     // Offset of Y axis compared to reference voltage
uint16_t g_joystick_reading        = 0;     // ?
uint8_t g_throttle_position        = 0;     // Store the current throttle position
uint16_t g_voltage_ref_value       = 0;     // Reference voltage reading from joystick;

// Reporting
uint32_t g_last_serial_report_time = 0;     // ms timestamp of last report to serial console
uint32_t g_last_can_report_time    = 0;     // ms timestamp of last report to CAN bus

// CAN bus
char g_can_msg[12];                         // Buffer for CAN messages
char g_incoming_message_buffer[12];         // Buffer for incoming CAN messages

/*--------------------------- Function Signatures ---------------------------*/
void reportViaSerial();
void readOneButton(uint8_t button_id);
void readButtons();
void readJoystick();

/*--------------------------- Instantiate Global Objects --------------------*/
// Virtual joystick device
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
                   1, 0,                  // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, true,           // No rudder, but enable throttle
                   false, false, false);  // No accelerator, brake, or steering

/*--------------------------- Program ---------------------------------------*/
/**
   Setup
*/
void setup() {
  Serial.begin(115200);
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  delay(2500); // Delay to allow serial port time to connect if it's present,
  // but continue anyway if it's not
  Serial.println("Wheelchair joystick breakout v2.2");

  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_3_PIN, INPUT_PULLUP);
  pinMode(BUTTON_4_PIN, INPUT_PULLUP);
  pinMode(BUTTON_5_PIN, INPUT_PULLUP);

  if (!CAN.begin(CAN_BUS_SPEED)) {
    Serial.println("Starting CAN failed!");
    while (1);
  } else {
    Serial.println("Started CAN OK");
  }

  uint16_t inputValue = analogRead(BUTTON_1_PIN);
  if (inputValue > JOYSTICK_PRESENT_LEVEL)
  {
    g_joystick_present = true;
  }

  // Begin joystick emulation or keyboard emulation, depending on attached device
  if (g_joystick_present)
  {
    // Joystick found, so initialise joystick emulation
    Serial.println(F("Joystick mode"));
    Joystick.begin();
    Joystick.setXAxisRange(-JOYSTICK_AXIS_RANGE, JOYSTICK_AXIS_RANGE);
    Joystick.setYAxisRange(-JOYSTICK_AXIS_RANGE, JOYSTICK_AXIS_RANGE);
    Joystick.setThrottleRange(-127, 127);
  } else {
    // No joystick found, so initialise keyboard emulation
    Serial.println(F("Button box mode"));
    Keyboard.begin();
  }
  Serial.println("Ready");
}

/**
   Loop
*/
void loop() {
  // Check for CAN messages
  processCAN();

  if (g_joystick_present)
  {
    readJoystick();
  } else {
    readButtons();
  }
  reportViaSerial();
}

/**

*/
void reportViaSerial()
{
  uint32_t current_time = millis();
  if ((current_time - g_last_serial_report_time) > JOYSTICK_DEBUG_INTERVAL)
  {
    if (JOYSTICK_DEBUG)
    {
      Serial.print("T:");
      Serial.println(g_throttle_position);
      g_last_serial_report_time = current_time;
    }
  }
}

/**
   Read one of the input buttons
*/
void readOneButton(uint8_t button_id)
{
  g_button_value[button_id] = analogRead(g_button_pin[button_id]);

  if (BUTTONS_DEBUG)
  {
    Serial.print(F("Button ID: "));
    Serial.print(button_id);
    Serial.print(F(" Button: "));
    Serial.print(g_button_character[button_id]);
    Serial.print(F(": "));
    Serial.println(g_button_value[button_id]);
  }

  // Detect transition from unpressed to pressed
  if ((g_button_value[button_id] <= BUTTON_TRIGGER_LEVEL) && (g_button_previous_value[button_id] > BUTTON_TRIGGER_LEVEL))
  {
    Keyboard.press(g_button_character[button_id]);

    g_can_msg[0] = g_button_character[button_id];
    g_can_msg[1] = '+';          // key press
    CAN.beginPacket(0x14);       // Packet ID 0x14
    for (uint8_t i = 0; i < 12; i++) // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(g_can_msg[i]);
    }
    CAN.endPacket();
    if (BUTTONS_DEBUG)
    {
      Serial.print(F("Button: "));
      Serial.print(g_button_character[button_id]);
      Serial.println(F(" press"));
    }
  }
  // Detect transition from pressed to unpressed
  if ((g_button_value[button_id] > BUTTON_TRIGGER_LEVEL) && (g_button_previous_value[button_id] <= BUTTON_TRIGGER_LEVEL))
  {
    Keyboard.release(g_button_character[button_id]);

    g_can_msg[0] = g_button_character[button_id];
    g_can_msg[1] = '-';          // key release
    CAN.beginPacket(0x14);       // Packet ID 0x14
    for (uint8_t i = 0; i < 12; i++) // Hard coded length to 12 bytes. Make variable? Single byte?
    {
      CAN.write(g_can_msg[i]);
    }
    CAN.endPacket();
    if (BUTTONS_DEBUG)
    {
      Serial.print(F("Button: "));
      Serial.print(g_button_character[button_id]);
      Serial.println(F(" release"));
    }
  }
  g_button_previous_value[button_id] = g_button_value[button_id];
}

/**
  Read each of the possible input buttons
*/
void readButtons()
{
  readOneButton(0);
  readOneButton(1);
  readOneButton(2);
  readOneButton(3);
  readOneButton(4);
}

/**
  Process messages received via CAN bus
*/
void processCAN()
{
  // Try to parse packet
  uint8_t packet_size = CAN.parsePacket();

  if (packet_size) {
    // Received a packet
    if (CAN_DEBUG)
    {
      Serial.print("Received packet with id 0x");
      Serial.print(CAN.packetId(), HEX);
      Serial.print(" and length ");
      Serial.println(packet_size);
    }

    if (CAN_DEBUG)
    {
      Serial.print(">");
    }
    uint8_t i = 0;
    while (CAN.available()) {
      char msg_char = (char)CAN.read();
      if (CAN_DEBUG)
      {
        Serial.print(msg_char);
      }
      g_incoming_message_buffer[i] = msg_char;
      i++;
    }
    if (CAN_DEBUG)
    {
      Serial.print("<");
      Serial.println();
    }

    if (!g_joystick_present)
    {
      if (g_incoming_message_buffer[1] == '+')
      {
        Keyboard.press(g_incoming_message_buffer[0]);
      }

      if (g_incoming_message_buffer[1] == '-')
      {
        Keyboard.release(g_incoming_message_buffer[0]);
      }
    }


    // Check for specific keys if we're in joystick mode, to use for throttle up and down
    if (g_incoming_message_buffer[0] == 'v')
    {
      g_throttle_position = g_throttle_position + THROTTLE_INCREMENT;

      if (g_throttle_position > 127)
      {
        g_throttle_position = 127;
      }
    }
    if (g_incoming_message_buffer[0] == 'w')
    {
      g_throttle_position = g_throttle_position - THROTTLE_INCREMENT;
      if (g_throttle_position < -127)
      {
        g_throttle_position = -127;
      }
    }
  }
}

/**
  Read the joystick position
*/
void readJoystick()
{
  uint32_t current_time = millis();
  g_voltage_ref_value = analogRead(VREF_PIN);

  g_joystick_reading = analogRead(Y_AXIS_PIN);
  if (g_joystick_reading > (g_voltage_ref_value + CENTER_DEAD_SPOT))
  {
    g_y_axis_delta = g_voltage_ref_value - g_joystick_reading + CENTER_DEAD_SPOT;
  }
  if (g_joystick_reading < (g_voltage_ref_value - CENTER_DEAD_SPOT))
  {
    g_y_axis_delta = g_voltage_ref_value - g_joystick_reading - CENTER_DEAD_SPOT;
  }

  g_joystick_reading = analogRead(X_AXIS_PIN);
  if (g_joystick_reading > (g_voltage_ref_value + CENTER_DEAD_SPOT))
  {
    g_x_axis_delta = g_voltage_ref_value - g_joystick_reading + CENTER_DEAD_SPOT;
  }
  if (g_joystick_reading < (g_voltage_ref_value - CENTER_DEAD_SPOT))
  {
    g_x_axis_delta = g_voltage_ref_value - g_joystick_reading - CENTER_DEAD_SPOT;
  }

  /* Send output to joystick HID */
  Joystick.setYAxis(g_y_axis_delta * -1);  // Y axis is reversed
  Joystick.setXAxis(g_x_axis_delta);
  if (JOYSTICK_DEBUG)
  {
    Serial.println(g_x_axis_delta);
  }

  Joystick.setThrottle(g_throttle_position);

  /* Send output to CAN bus */
  if (current_time > (g_last_can_report_time + CAN_MESSAGE_INTERVAL))
  {
    int16_t xLength = (g_x_axis_delta == 0 ? 1 : ((int16_t)(log10(fabs(g_x_axis_delta)) + 1) + (g_x_axis_delta < 0 ? 1 : 0)));
    int16_t yLength = (g_y_axis_delta == 0 ? 1 : ((int16_t)(log10(fabs(g_y_axis_delta)) + 1) + (g_y_axis_delta < 0 ? 1 : 0)));

    //Serial.println(xLength);
    sprintf(g_can_msg, "%d", g_x_axis_delta);
    CAN.beginPacket(0x12);     // Packet ID 0x12 for X axis
    for (uint8_t i = 0; i < xLength; i++)
    {
      CAN.write(g_can_msg[i]);
    }
    CAN.endPacket();

    sprintf(g_can_msg, "%d", g_y_axis_delta);
    CAN.beginPacket(0x13);     // Packet ID 0x13 for Y axis
    for (uint8_t i = 0; i < yLength; i++)
    {
      CAN.write(g_can_msg[i]);
    }
    CAN.endPacket();
    g_last_can_report_time = current_time;
  }
}
