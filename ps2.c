/**
 * File:	ps2.c
 * Author:	Grant Kurtz
 *
 * Description:
 *		Handles communicating with the PS/2 controller and also acts as an
 *		interface to PS/2 devices for user/kernel programs.
 */

 #include "mouse.c"
 #include "keyboard.c"

/**
 * Handles initalization of PS/2 devices (keyboard and mouse).
 * 
 * @return		1 if initialization was successful, otherwise 0;
 */
 int _ps2_init( void ){
	
	//_ps2_mouse_init();
	_ps2_keyboard_init();
	return 1;
 }
