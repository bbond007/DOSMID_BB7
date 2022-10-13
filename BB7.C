#include <conio.h>  // inp() and outp() 
#include "BB7.h"

unsigned short JoystickPosition(unsigned char axisnum)
{	
	unsigned short count = 0;
    __asm 
	{
		// mov word ptr count, 0 
		cli          // Disable interrupts so they don't interfere with timing 
		mov dx, JOYSTICKPORT   // Write dummy byte to joystick port 
		out dx, al
		joystickloop:
		inc count              // Add one to count 
		cmp count, 0xFFFF      // Check for time out 
		je done
		in al, dx              // Get joystick port value 
		and al, axisnum        // Test the appropriate bit 
		jne joystickloop
		done:
		st
	}
	return count;
}

// Button returns true is button is pressed 
unsigned char JoystickButton(unsigned char buttonnum)
{
	return (inp(JOYSTICKPORT) & buttonnum) == 0;
}
static unsigned char bMouse = 0;

unsigned char mouseInstalled()
{
    unsigned char result = 0;
	__asm 
	{
		XOr AX, AX
		Int 33h
		cmp AX, - 1
		je skip
		XOr AL, AL
		skip:
		Mov result, AL
		Mov bMouse, AL
	}
	return result;
}

unsigned short leftPressed()
{
	unsigned short result = 0;
	__asm 
	{
		cmp bMouse, 0
		je skip
		mov AX, 3
		Int 33h
		And BX, 1
		//mov AX, BX
		mov result, BX
		skip:
	}
	return result;
}

unsigned short rightPressed()
{
	unsigned short result = 0;
	__asm 
	{
		cmp bMouse, 0
		je skip
		mov AX, 3
		Int 33h
		And BX, 2
		//mov AX, BX
		mov result, BX
		skip:		
	}
	return result;
}

void flushMouseAndJoystick()
{
	while(rightPressed() || JoystickButton(JoystickAButton2))
	{
		
	}
}
