/* -------------------------- User configuration ----------------------------*/
// Button options
char g_button_character[5] = { 'w', 'z', 't', 'u', 'v' }; // Character to send when each key is pressed

// Joystick options
#define THROTTLE_INCREMENT         20  // Move this many steps when button is pressed
#define CENTER_DEAD_SPOT            0  // +/- this value from 0 gives 0 output

// CAN options
#define CAN_MESSAGE_INTERVAL       50  // Message interval for CAN in milliseconds
#define CAN_BUS_SPEED           500E3  // 500kbps

// Debug options
#define CAN_DEBUG               false  // Report CAN events to serial console
#define BUTTONS_DEBUG           false  // Report button events to serial console
#define JOYSTICK_DEBUG          false  // Report joystick values to serial console
#define JOYSTICK_DEBUG_INTERVAL   500  // Reporting interval for debug messages in milliseconds

/* -------------------------- Hardware setup --------------------------------*/
// Pin connections for button box input
#define BUTTON_1_PIN            A0
#define BUTTON_2_PIN            A1
#define BUTTON_3_PIN            A2
#define BUTTON_4_PIN            A3
#define BUTTON_5_PIN            A4

// Inputs sit about 572 normally, drop to about 406 when activated
#define BUTTON_TRIGGER_LEVEL 500

// Pin connections for joystick input
#define Y_AXIS_PIN              A0
#define X_AXIS_PIN              A1
#define VREF_PIN                A2

#define JOYSTICK_AXIS_RANGE     75  //#define JOYSTICK_AXIS_RANGE 127
#define JOYSTICK_PRESENT_LEVEL 700  // When a joystick is connected, it pulls the input voltage above this level
