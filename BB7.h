#define JoystickAButton1  0x10
#define JoystickAButton2  0x20
#define JoystickBButton1  0x40
#define JoystickBButton2  0x80
#define JoystickAAxisX    0x01
#define JoystickAAxisY    0x02
#define JoystickBAxisX    0x04
#define JoystickBAxisY    0x08
#define JOYSTICKPORT 	  0x201

unsigned short JoystickPosition(unsigned char axisnum);
unsigned char  JoystickButton(unsigned char buttonnum);
unsigned char  mouseInstalled();
unsigned short leftPressed();
unsigned short rightPressed();
void flushMouseAndJoystick();

