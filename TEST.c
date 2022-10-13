#include <dos.h>    /* REGS */
#include <stdio.h>  /* printf() */
#include <limits.h> /* ULONG_MAX */
#include <stdlib.h> /* rand() */
#include <string.h> /* memset(), strcpy(), strncat(), memcpy() */
#include "BB7.h"

int main(int argc, char **argv) 
{
	mouseInstalled();
	while (!rightPressed()) 
	{
		printf("LeftMouse --> %d     JoystickButtonA2 --> %d  \r\n", leftPressed(), JoystickButton(JoystickAButton2));	
	}
	
	printf("Waiting for release%d\r\n");
	
	flushMouseAndJoystick();
	
	return 0;
}
