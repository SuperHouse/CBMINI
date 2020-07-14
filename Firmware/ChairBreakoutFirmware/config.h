// Debug options
#define CAN_DEBUG false
#define BUTTONS_DEBUG false
#define JOYSTICK_DEBUG true

// Pin connections for button box input
#define BUTTON_1_PIN A0
#define BUTTON_2_PIN A1
#define BUTTON_3_PIN A2
#define BUTTON_4_PIN A3
#define BUTTON_5_PIN A4

// Pin connections for joystick input
#define Y_AXIS_PIN   A0
#define X_AXIS_PIN   A1
#define VREF_PIN     A2

// Options for joystick input
#define JOYSTICK_AXIS_RANGE 75
//#define JOYSTICK_AXIS_RANGE 127
#define THROTTLE_INCREMENT 10      // Move this many steps when button is pressed
#define JOYSTICK_PRESENT_LEVEL 700 // When a joystick is connected, it pulls the input voltage above this level
#define CENTER_DEAD_SPOT 0 // +/- this value from 0 gives 0 output

// Options for button input
// Inputs sit about 572 normally, drop to about 406 when activated
#define BUTTON_TRIGGER_LEVEL 500

// CAN bus options
#define CAN_REPORT_INTERVAL     100 // Reporting frequency for CAN in milliseconds
#define SERIAL_REPORT_INTERVAL  500 // Reporting frequency for serial in milliseconds
