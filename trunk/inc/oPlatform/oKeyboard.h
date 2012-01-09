// $(header)
#pragma once
#ifndef oKeyboard_h
#define oKeyboard_h

#include <oBasis/oInterface.h>

interface oKeyboard : oInterface
{
	enum KEY
	{
		KEY_UNKNOWN,
		KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
		KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
		KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
		KEY_PAD0, KEY_PAD1, KEY_PAD2, KEY_PAD3, KEY_PAD4, KEY_PAD5, KEY_PAD6, KEY_PAD7, KEY_PAD8, KEY_PAD9,
		KEY_PADENTER, KEY_PADADD, KEY_PADSUBTRACT, KEY_PADMULTIPLY, KEY_PADDIVIDE, KEY_PADDECIMAL,
		KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_PGUP, KEY_PGDOWN, KEY_HOME, KEY_END, KEY_INS, KEY_DEL, KEY_ESC, KEY_PRINTSCREEN, KEY_PAUSEBREAK, KEY_SCROLLLOCK, KEY_TILDE, KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB,
		KEY_LBRACKET, KEY_RBRACKET, KEY_BACKSLASH, KEY_CAPSLOCK, KEY_SEMICOLON, KEY_QUOTE, KEY_ENTER, KEY_COMMA, KEY_PERIOD, KEY_SLASH,
		KEY_LSHIFT, KEY_RSHIFT, KEY_START, KEY_LCTRL, KEY_RCTRL, KEY_LALT, KEY_RALT, KEY_MENU, KEY_SPACE,
		KEY_MUTE, KEY_MEDIAPLAYPAUSE, KEY_MEDIASTOP, KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_MEDIAPREV, KEY_MEDIANEXT,
		NUM_KEYS,
	};

	static char ToAscii(KEY _Key, bool _IsShiftDown);

	// On event-driven platforms (Windows) where there isn't an explicit update 
	// for the application's context, a button might not be able to be determined
	// as "still down from last frame" until the next event fires. In order to
	// work around this, call this function once a frame and it will change all
	// pressed keys from last frame into down keys and all released keys into up 
	// keyss.
	virtual void Update() threadsafe = 0;

	// Default unpressed state of a key
	virtual bool IsUp(KEY _Key) const threadsafe = 0;

	// True on first frame of a transition from down to up
	virtual bool IsReleased(KEY _Key) const threadsafe  = 0;

	// Raw state ignoring key repeat
	virtual bool IsDown(KEY _Key) const threadsafe = 0;

	// Returns true the first time the key goes from up to down, and again 
	// according to platform key repeat settings.
	virtual bool IsPressed(KEY _Key) const threadsafe = 0;

	// Find the first key pressed starting from (and not including) _KeyStart
	virtual KEY FindPressedKey(KEY _KeyStart = KEY_UNKNOWN) const threadsafe = 0;
};

bool oKeyboardCreate(void* _WindowNativeHandle, bool _ShortCircuitKeyEvents, threadsafe oKeyboard** _ppKeyboard);

#endif
