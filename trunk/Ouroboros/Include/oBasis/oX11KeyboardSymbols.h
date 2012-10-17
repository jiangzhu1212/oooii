// $(noheader)
/* $XConsortium: keysymdef.h,v 1.21 94/08/28 16:17:06 rws Exp $*/

/***********************************************************
Copyright (c) 1987, 1994 X Consortium
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:
 
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
 
Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.
 
 
Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts
 
All Rights Reserved
 
Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.
 
DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
 
******************************************************************/

// === OOOii Changes ===
// To be more standards-compliant, OOOii is working towards using RFB: the 
// remote frame buffer protocol. It uses X11 key definitions as its standard for 
// sending client user input to the server, so this define sets up key defines
// for keyboard, mouse, and touch interfaces that will be compliant with RFB.
// http://www.realvnc.com/docs/rfbproto.pdf

// Downloaded from http://trac.imagemagick.org/browser/xlib/trunk/X11/keysymdef.h
// CHANGE: The XK_ prefix has been converted to oKB_ for library consistency
// CHANGE: Whitespace patch-up (copy-pasted from HTML version of source)
// CHANGE: Converted from #define's to an enum.
// CHANGE: Added #define's to include some of the definitions that are being used
// CHANGE: Add oKB_Touch#

#pragma once
#ifndef oX11KeyboardSymbols_h
#define oX11KeyboardSymbols_h

#define XK_MISCELLANY
#define XK_XKB_KEYS // for the pointer clicks
#define XK_LATIN1

enum oKEYBOARD_KEY
{
	oKB_Touch0 = 0xFFF0,
	oKB_Touch1 = 0xFFF1,
	oKB_Touch2 = 0xFFF2,
	oKB_Touch3 = 0xFFF3,
	oKB_Touch4 = 0xFFF4,
	oKB_Touch5 = 0xFFF5,
	oKB_Touch6 = 0xFFF6,
	oKB_Touch7 = 0xFFF7,
	oKB_Touch8 = 0xFFF8,
	oKB_Touch9 = 0xFFF9,

	// Microsoft / Windows keyboard media keys
	oKB_Volume_Down = 0x1008FF11,
	oKB_Volume_Mute = 0x1008FF12,
	oKB_Volume_Up = 0x1008FF13,
	oKB_Media_Play_Pause = 0x1008FF14,
	oKB_Media_Stop = 0x1008FF15,
	oKB_Media_Prev_Track = 0x1008FF16,
	oKB_Media_Next_Track = 0x1008FF17,
	oKB_Launch_Mail = 0x1008FF19,
	oKB_Browser_Search = 0x1008FF1B,
	oKB_Browser_Home = 0x1008FF1F,
	oKB_Browser_Back = 0x1008FF26,
	oKB_Browser_Forward = 0x1008FF27,
	oKB_Browser_Stop = 0x1008FF28,
	oKB_Browser_Refresh = 0x1008FF29,
	oKB_Browser_Favorites = 0x1008FF30,
	oKB_Launch_Media_Select = 0x1008FF32,

	oKB_VoidSymbol = 0xFFFFFF, /* void symbol*/
 
#ifdef XK_MISCELLANY
/*
 * TTY Functions, cleverly chosen to map to ascii, for convenience of
 * programming, but could have been arbitrary (at the cost of lookup
 * tables in client code.
 */
 
	oKB_BackSpace = 0xFF08, /* back space, back char*/
	oKB_Tab = 0xFF09, 
	oKB_Linefeed = 0xFF0A, /* Linefeed, LF*/
	oKB_Clear = 0xFF0B, 
	oKB_Return = 0xFF0D, /* Return, enter*/
	oKB_Pause = 0xFF13, /* Pause, hold*/
	oKB_Scroll_Lock = 0xFF14, 
	oKB_Sys_Req = 0xFF15, 
	oKB_Escape = 0xFF1B, 
	oKB_Delete = 0xFFFF, /* Delete, rubout*/
 
/* International & multi-key character composition*/
 
oKB_Multi_key = 0xFF20, /* Multi-key character compose*/
 
/* Japanese keyboard support*/
 
	oKB_Kanji = 0xFF21, /* Kanji, Kanji convert*/
	oKB_Muhenkan = 0xFF22, /* Cancel Conversion*/
	oKB_Henkan_Mode = 0xFF23, /* Start/Stop Conversion*/
	oKB_Henkan = 0xFF23, /* Alias for Henkan_Mode*/
	oKB_Romaji = 0xFF24, /* to Romaji*/
	oKB_Hiragana = 0xFF25, /* to Hiragana*/
	oKB_Katakana = 0xFF26, /* to Katakana*/
	oKB_Hiragana_Katakana = 0xFF27, /* Hiragana/Katakana toggle*/
	oKB_Zenkaku = 0xFF28, /* to Zenkaku*/
	oKB_Hankaku = 0xFF29, /* to Hankaku*/
	oKB_Zenkaku_Hankaku = 0xFF2A, /* Zenkaku/Hankaku toggle*/
	oKB_Touroku = 0xFF2B, /* Add to Dictionary*/
	oKB_Massyo = 0xFF2C, /* Delete from Dictionary*/
	oKB_Kana_Lock = 0xFF2D, /* Kana Lock*/
	oKB_Kana_Shift = 0xFF2E, /* Kana Shift*/
	oKB_Eisu_Shift = 0xFF2F, /* Alphanumeric Shift*/
	oKB_Eisu_toggle = 0xFF30, /* Alphanumeric toggle*/
 
/* 0xFF31 thru 0xFF3F are under XK_KOREAN*/
 
/* Cursor control & motion*/
 
	oKB_Home = 0xFF50, 
	oKB_Left = 0xFF51, /* Move left, left arrow*/
	oKB_Up = 0xFF52, /* Move up, up arrow*/
	oKB_Right = 0xFF53, /* Move right, right arrow*/
	oKB_Down = 0xFF54, /* Move down, down arrow*/
	oKB_Prior = 0xFF55, /* Prior, previous*/
	oKB_Page_Up = 0xFF55, 
	oKB_Next = 0xFF56, /* Next*/
	oKB_Page_Down = 0xFF56, 
	oKB_End = 0xFF57, /* EOL*/
	oKB_Begin = 0xFF58, /* BOL*/
 
/* Misc Functions*/
 
	oKB_Select = 0xFF60, /* Select, mark*/
	oKB_Print = 0xFF61, 
	oKB_Execute = 0xFF62, /* Execute, run, do*/
	oKB_Insert = 0xFF63, /* Insert, insert here*/
	oKB_Undo = 0xFF65, /* Undo, oops*/
	oKB_Redo = 0xFF66, /* redo, again*/
	oKB_Menu = 0xFF67, 
	oKB_Find = 0xFF68, /* Find, search*/
	oKB_Cancel = 0xFF69, /* Cancel, stop, abort, exit*/
	oKB_Help = 0xFF6A, /* Help*/
	oKB_Break = 0xFF6B, 
	oKB_Mode_switch = 0xFF7E, /* Character set switch*/
	oKB_script_switch = 0xFF7E, /* Alias for mode_switch*/
	oKB_Num_Lock = 0xFF7F, 
 
/* Keypad Functions, keypad numbers cleverly chosen to map to ascii*/
 
	oKB_KP_Space = 0xFF80, /* space*/
	oKB_KP_Tab = 0xFF89, 
	oKB_KP_Enter = 0xFF8D, /* enter*/
	oKB_KP_F1 = 0xFF91, /* PF1, KP_A, ...*/
	oKB_KP_F2 = 0xFF92, 
	oKB_KP_F3 = 0xFF93, 
	oKB_KP_F4 = 0xFF94, 
	oKB_KP_Home = 0xFF95, 
	oKB_KP_Left = 0xFF96, 
	oKB_KP_Up = 0xFF97, 
	oKB_KP_Right = 0xFF98, 
	oKB_KP_Down = 0xFF99, 
	oKB_KP_Prior = 0xFF9A, 
	oKB_KP_Page_Up = 0xFF9A, 
	oKB_KP_Next = 0xFF9B, 
	oKB_KP_Page_Down = 0xFF9B, 
	oKB_KP_End = 0xFF9C, 
	oKB_KP_Begin = 0xFF9D, 
	oKB_KP_Insert = 0xFF9E, 
	oKB_KP_Delete = 0xFF9F, 
	oKB_KP_Equal = 0xFFBD, /* equals*/
	oKB_KP_Multiply = 0xFFAA, 
	oKB_KP_Add = 0xFFAB, 
	oKB_KP_Separator = 0xFFAC, /* separator, often comma*/
	oKB_KP_Subtract = 0xFFAD, 
	oKB_KP_Decimal = 0xFFAE, 
	oKB_KP_Divide = 0xFFAF, 
 
	oKB_KP_0 = 0xFFB0, 
	oKB_KP_1 = 0xFFB1, 
	oKB_KP_2 = 0xFFB2, 
	oKB_KP_3 = 0xFFB3, 
	oKB_KP_4 = 0xFFB4, 
	oKB_KP_5 = 0xFFB5, 
	oKB_KP_6 = 0xFFB6, 
	oKB_KP_7 = 0xFFB7, 
	oKB_KP_8 = 0xFFB8, 
	oKB_KP_9 = 0xFFB9, 
 
 /*
 * Auxilliary Functions; note the duplicate definitions for left and right
 * function keys; Sun keyboards and a few other manufactures have such
 * function key groups on the left and/or right sides of the keyboard.
 * We've not found a keyboard with more than 35 function keys total.
 */
 
	oKB_F1 = 0xFFBE, 
	oKB_F2 = 0xFFBF, 
	oKB_F3 = 0xFFC0, 
	oKB_F4 = 0xFFC1, 
	oKB_F5 = 0xFFC2, 
	oKB_F6 = 0xFFC3, 
	oKB_F7 = 0xFFC4, 
	oKB_F8 = 0xFFC5, 
	oKB_F9 = 0xFFC6, 
	oKB_F10 = 0xFFC7, 
	oKB_F11 = 0xFFC8, 
	oKB_L1 = 0xFFC8, 
	oKB_F12 = 0xFFC9, 
	oKB_L2 = 0xFFC9, 
	oKB_F13 = 0xFFCA, 
	oKB_L3 = 0xFFCA, 
	oKB_F14 = 0xFFCB, 
	oKB_L4 = 0xFFCB, 
	oKB_F15 = 0xFFCC, 
	oKB_L5 = 0xFFCC, 
	oKB_F16 = 0xFFCD, 
	oKB_L6 = 0xFFCD, 
	oKB_F17 = 0xFFCE, 
	oKB_L7 = 0xFFCE, 
	oKB_F18 = 0xFFCF, 
	oKB_L8 = 0xFFCF, 
	oKB_F19 = 0xFFD0, 
	oKB_L9 = 0xFFD0, 
	oKB_F20 = 0xFFD1, 
	oKB_L10 = 0xFFD1, 
	oKB_F21 = 0xFFD2, 
	oKB_R1 = 0xFFD2, 
	oKB_F22 = 0xFFD3, 
	oKB_R2 = 0xFFD3, 
	oKB_F23 = 0xFFD4, 
	oKB_R3 = 0xFFD4, 
	oKB_F24 = 0xFFD5, 
	oKB_R4 = 0xFFD5, 
	oKB_F25 = 0xFFD6, 
	oKB_R5 = 0xFFD6, 
	oKB_F26 = 0xFFD7, 
	oKB_R6 = 0xFFD7, 
	oKB_F27 = 0xFFD8, 
	oKB_R7 = 0xFFD8, 
	oKB_F28 = 0xFFD9, 
	oKB_R8 = 0xFFD9, 
	oKB_F29 = 0xFFDA, 
	oKB_R9 = 0xFFDA, 
	oKB_F30 = 0xFFDB, 
	oKB_R10 = 0xFFDB, 
	oKB_F31 = 0xFFDC, 
	oKB_R11 = 0xFFDC, 
	oKB_F32 = 0xFFDD, 
	oKB_R12 = 0xFFDD, 
	oKB_F33 = 0xFFDE, 
	oKB_R13 = 0xFFDE, 
	oKB_F34 = 0xFFDF, 
	oKB_R14 = 0xFFDF, 
	oKB_F35 = 0xFFE0, 
	oKB_R15 = 0xFFE0, 
 
/* Modifiers*/
 
	oKB_Shift_L = 0xFFE1, /* Left shift*/
	oKB_Shift_R = 0xFFE2, /* Right shift*/
	oKB_Control_L = 0xFFE3, /* Left control*/
	oKB_Control_R = 0xFFE4, /* Right control*/
	oKB_Caps_Lock = 0xFFE5, /* Caps lock*/
	oKB_Shift_Lock = 0xFFE6, /* Shift lock*/
 
	oKB_Meta_L = 0xFFE7, /* Left meta*/
	oKB_Meta_R = 0xFFE8, /* Right meta*/
	oKB_Alt_L = 0xFFE9, /* Left alt*/
	oKB_Alt_R = 0xFFEA, /* Right alt*/
	oKB_Super_L = 0xFFEB, /* Left super*/
	oKB_Super_R = 0xFFEC, /* Right super*/
	oKB_Hyper_L = 0xFFED, /* Left hyper*/
	oKB_Hyper_R = 0xFFEE, /* Right hyper*/

#endif/* XK_MISCELLANY*/
 
/*
 * ISO 9995 Function and Modifier Keys
 * Byte 3 = 0xFE
 */
 
#ifdef XK_XKB_KEYS
	oKB_ISO_Lock = 0xFE01, 
	oKB_ISO_Level2_Latch = 0xFE02, 
	oKB_ISO_Level3_Shift = 0xFE03, 
	oKB_ISO_Level3_Latch = 0xFE04, 
	oKB_ISO_Level3_Lock = 0xFE05, 
	oKB_ISO_Group_Shift = 0xFF7E, /* Alias for mode_switch*/
	oKB_ISO_Group_Latch = 0xFE06, 
	oKB_ISO_Group_Lock = 0xFE07, 
	oKB_ISO_Next_Group = 0xFE08, 
	oKB_ISO_Next_Group_Lock = 0xFE09, 
	oKB_ISO_Prev_Group = 0xFE0A, 
	oKB_ISO_Prev_Group_Lock = 0xFE0B, 
	oKB_ISO_First_Group = 0xFE0C, 
	oKB_ISO_First_Group_Lock = 0xFE0D, 
	oKB_ISO_Last_Group = 0xFE0E, 
	oKB_ISO_Last_Group_Lock = 0xFE0F, 
 
	oKB_ISO_Left_Tab = 0xFE20, 
	oKB_ISO_Move_Line_Up = 0xFE21, 
	oKB_ISO_Move_Line_Down = 0xFE22, 
	oKB_ISO_Partial_Line_Up = 0xFE23, 
	oKB_ISO_Partial_Line_Down = 0xFE24, 
	oKB_ISO_Partial_Space_Left = 0xFE25, 
	oKB_ISO_Partial_Space_Right = 0xFE26, 
	oKB_ISO_Set_Margin_Left = 0xFE27, 
	oKB_ISO_Set_Margin_Right = 0xFE28, 
	oKB_ISO_Release_Margin_Left = 0xFE29, 
	oKB_ISO_Release_Margin_Right = 0xFE2A, 
	oKB_ISO_Release_Both_Margins = 0xFE2B, 
	oKB_ISO_Fast_Cursor_Left = 0xFE2C, 
	oKB_ISO_Fast_Cursor_Right = 0xFE2D, 
	oKB_ISO_Fast_Cursor_Up = 0xFE2E, 
	oKB_ISO_Fast_Cursor_Down = 0xFE2F, 
	oKB_ISO_Continuous_Underline = 0xFE30, 
	oKB_ISO_Discontinuous_Underline = 0xFE31, 
	oKB_ISO_Emphasize = 0xFE32, 
	oKB_ISO_Center_Object = 0xFE33, 
	oKB_ISO_Enter = 0xFE34, 
 
	oKB_dead_grave = 0xFE50, 
	oKB_dead_acute = 0xFE51, 
	oKB_dead_circumflex = 0xFE52, 
	oKB_dead_tilde = 0xFE53, 
	oKB_dead_macron = 0xFE54, 
	oKB_dead_breve = 0xFE55, 
	oKB_dead_abovedot = 0xFE56, 
	oKB_dead_diaeresis = 0xFE57, 
	oKB_dead_abovering = 0xFE58, 
	oKB_dead_doubleacute = 0xFE59, 
	oKB_dead_caron = 0xFE5A, 
	oKB_dead_cedilla = 0xFE5B, 
	oKB_dead_ogonek = 0xFE5C, 
	oKB_dead_iota = 0xFE5D, 
	oKB_dead_voiced_sound = 0xFE5E, 
	oKB_dead_semivoiced_sound = 0xFE5F, 
 
	oKB_First_Virtual_Screen = 0xFED0, 
	oKB_Prev_Virtual_Screen = 0xFED1, 
	oKB_Next_Virtual_Screen = 0xFED2, 
	oKB_Last_Virtual_Screen = 0xFED4, 
	oKB_Terminate_Server = 0xFED5, 
 
	oKB_Pointer_Left = 0xFEE0, 
	oKB_Pointer_Right = 0xFEE1, 
	oKB_Pointer_Up = 0xFEE2, 
	oKB_Pointer_Down = 0xFEE3, 
	oKB_Pointer_UpLeft = 0xFEE4, 
	oKB_Pointer_UpRight = 0xFEE5, 
	oKB_Pointer_DownLeft = 0xFEE6, 
	oKB_Pointer_DownRight = 0xFEE7,    /* oooii-tony: Add some more visually pleasing versions for pointer buttons */
	oKB_Pointer_Button_Dflt = 0xFEE8, oKB_Pointer_Button_Left = 0xFEE8, 
	oKB_Pointer_Button1 = 0xFEE9, oKB_Pointer_Button_Right = 0xFEE9, 
	oKB_Pointer_Button2 = 0xFEEA, oKB_Pointer_Button_Middle = 0xFEEA, 
	oKB_Pointer_Button3 = 0xFEEB, oKB_Pointer_Button_Back = 0xFEEB, 
	oKB_Pointer_Button4 = 0xFEEC, oKB_Pointer_Button_Forward = 0xFEEC, 
	oKB_Pointer_Button5 = 0xFEED, 
	oKB_Pointer_DblClick_Dflt = 0xFEEE, oKB_Pointer_DblClick_Left = 0xFEEE, 
	oKB_Pointer_DblClick1 = 0xFEEF, oKB_Pointer_DblClick_Right = 0xFEEF, 
	oKB_Pointer_DblClick2 = 0xFEF0, oKB_Pointer_DblClick_Middle = 0xFEF0, 
	oKB_Pointer_DblClick3 = 0xFEF1, oKB_Pointer_DblClick_Back = 0xFEF1, 
	oKB_Pointer_DblClick4 = 0xFEF2, oKB_Pointer_DblClick_Forward = 0xFEF2, 
	oKB_Pointer_DblClick5 = 0xFEF3, 
	oKB_Pointer_Drag_Dflt = 0xFEF4, oKB_Pointer_Drag_Left = 0xFEF4, 
	oKB_Pointer_Drag1 = 0xFEF5, oKB_Pointer_Drag_Right = 0xFEF5, 
	oKB_Pointer_Drag2 = 0xFEF6, oKB_Pointer_Drag_Middle = 0xFEF6, 
	oKB_Pointer_Drag3 = 0xFEF7, oKB_Pointer_Drag_Back = 0xFEF7, 
	oKB_Pointer_Drag4 = 0xFEF8, oKB_Pointer_Drag_Forward = 0xFEF8, 
 
	oKB_Pointer_EnableKeys = 0xFEF9, 
	oKB_Pointer_Accelerate = 0xFEFA, 
	oKB_Pointer_DfltBtnNext = 0xFEFB, 
	oKB_Pointer_DfltBtnPrev = 0xFEFC, 

#endif
 
/*
 * 3270 Terminal Keys
 * Byte 3 = 0xFD
 */
 
#ifdef XK_3270
	oKB_3270_Duplicate = 0xFD01, 
	oKB_3270_FieldMark = 0xFD02, 
	oKB_3270_Right2 = 0xFD03, 
	oKB_3270_Left2 = 0xFD04, 
	oKB_3270_BackTab = 0xFD05, 
	oKB_3270_EraseEOF = 0xFD06, 
	oKB_3270_EraseInput = 0xFD07, 
	oKB_3270_Reset = 0xFD08, 
	oKB_3270_Quit = 0xFD09, 
	oKB_3270_PA1 = 0xFD0A, 
	oKB_3270_PA2 = 0xFD0B, 
	oKB_3270_PA3 = 0xFD0C, 
	oKB_3270_Test = 0xFD0D, 
	oKB_3270_Attn = 0xFD0E, 
	oKB_3270_CursorBlink = 0xFD0F, 
	oKB_3270_AltCursor = 0xFD10, 
	oKB_3270_KeyClick = 0xFD11, 
	oKB_3270_Jump = 0xFD12, 
	oKB_3270_Ident = 0xFD13, 
	oKB_3270_Rule = 0xFD14, 
	oKB_3270_Copy = 0xFD15, 
	oKB_3270_Play = 0xFD16, 
	oKB_3270_Setup = 0xFD17, 
	oKB_3270_Record = 0xFD18, 
	oKB_3270_ChangeScreen = 0xFD19, 
	oKB_3270_DeleteWord = 0xFD1A, 
	oKB_3270_ExSelect = 0xFD1B, 
	oKB_3270_CursorSelect = 0xFD1C, 
	oKB_3270_PrintScreen = 0xFD1D, 
	oKB_3270_Enter = 0xFD1E, 
#endif
 
/*
 * Latin 1
 * Byte 3 = 0
 */
#ifdef XK_LATIN1
	oKB_space = 0x020, 
	oKB_exclam = 0x021, 
	oKB_quotedbl = 0x022, 
	oKB_numbersign = 0x023, 
	oKB_dollar = 0x024, 
	oKB_percent = 0x025, 
	oKB_ampersand = 0x026, 
	oKB_apostrophe = 0x027, 
	oKB_quoteright = 0x027, /* deprecated*/
	oKB_parenleft = 0x028, 
	oKB_parenright = 0x029, 
	oKB_asterisk = 0x02a, 
	oKB_plus = 0x02b, 
	oKB_comma = 0x02c, 
	oKB_minus = 0x02d, 
	oKB_period = 0x02e, 
	oKB_slash = 0x02f, 
	oKB_0 = 0x030, 
	oKB_1 = 0x031, 
	oKB_2 = 0x032, 
	oKB_3 = 0x033, 
	oKB_4 = 0x034, 
	oKB_5 = 0x035, 
	oKB_6 = 0x036, 
	oKB_7 = 0x037, 
	oKB_8 = 0x038, 
	oKB_9 = 0x039, 
	oKB_colon = 0x03a, 
	oKB_semicolon = 0x03b, 
	oKB_less = 0x03c, 
	oKB_equal = 0x03d, 
	oKB_greater = 0x03e, 
	oKB_question = 0x03f, 
	oKB_at = 0x040, 
	oKB_A = 0x041, 
	oKB_B = 0x042, 
	oKB_C = 0x043, 
	oKB_D = 0x044, 
	oKB_E = 0x045, 
	oKB_F = 0x046, 
	oKB_G = 0x047, 
	oKB_H = 0x048, 
	oKB_I = 0x049, 
	oKB_J = 0x04a, 
	oKB_K = 0x04b, 
	oKB_L = 0x04c, 
	oKB_M = 0x04d, 
	oKB_N = 0x04e, 
	oKB_O = 0x04f, 
	oKB_P = 0x050, 
	oKB_Q = 0x051, 
	oKB_R = 0x052, 
	oKB_S = 0x053, 
	oKB_T = 0x054, 
	oKB_U = 0x055, 
	oKB_V = 0x056, 
	oKB_W = 0x057, 
	oKB_X = 0x058, 
	oKB_Y = 0x059, 
	oKB_Z = 0x05a, 
	oKB_bracketleft = 0x05b, 
	oKB_backslash = 0x05c, 
	oKB_bracketright = 0x05d, 
	oKB_asciicircum = 0x05e, 
	oKB_underscore = 0x05f, 
	oKB_grave = 0x060, 
	oKB_quoteleft = 0x060, /* deprecated*/
	oKB_a = 0x061, 
	oKB_b = 0x062, 
	oKB_c = 0x063, 
	oKB_d = 0x064, 
	oKB_e = 0x065, 
	oKB_f = 0x066, 
	oKB_g = 0x067, 
	oKB_h = 0x068, 
	oKB_i = 0x069, 
	oKB_j = 0x06a, 
	oKB_k = 0x06b, 
	oKB_l = 0x06c, 
	oKB_m = 0x06d, 
	oKB_n = 0x06e, 
	oKB_o = 0x06f, 
	oKB_p = 0x070, 
	oKB_q = 0x071, 
	oKB_r = 0x072, 
	oKB_s = 0x073, 
	oKB_t = 0x074, 
	oKB_u = 0x075, 
	oKB_v = 0x076, 
	oKB_w = 0x077, 
	oKB_x = 0x078, 
	oKB_y = 0x079, 
	oKB_z = 0x07a, 
	oKB_braceleft = 0x07b, 
	oKB_bar = 0x07c, 
	oKB_braceright = 0x07d, 
	oKB_asciitilde = 0x07e, 
 
	oKB_nobreakspace = 0x0a0, 
	oKB_exclamdown = 0x0a1, 
	oKB_cent = 0x0a2, 
	oKB_sterling = 0x0a3, 
	oKB_currency = 0x0a4, 
	oKB_yen = 0x0a5, 
	oKB_brokenbar = 0x0a6, 
	oKB_section = 0x0a7, 
	oKB_diaeresis = 0x0a8, 
	oKB_copyright = 0x0a9, 
	oKB_ordfeminine = 0x0aa, 
	oKB_guillemotleft = 0x0ab, /* left angle quotation mark*/
	oKB_notsign = 0x0ac, 
	oKB_hyphen = 0x0ad, 
	oKB_registered = 0x0ae, 
	oKB_macron = 0x0af, 
	oKB_degree = 0x0b0, 
	oKB_plusminus = 0x0b1, 
	oKB_twosuperior = 0x0b2, 
	oKB_threesuperior = 0x0b3, 
	oKB_acute = 0x0b4, 
	oKB_mu = 0x0b5, 
	oKB_paragraph = 0x0b6, 
	oKB_periodcentered = 0x0b7, 
	oKB_cedilla = 0x0b8, 
	oKB_onesuperior = 0x0b9, 
	oKB_masculine = 0x0ba, 
	oKB_guillemotright = 0x0bb, /* right angle quotation mark*/
	oKB_onequarter = 0x0bc, 
	oKB_onehalf = 0x0bd, 
	oKB_threequarters = 0x0be, 
	oKB_questiondown = 0x0bf, 
	oKB_Agrave = 0x0c0, 
	oKB_Aacute = 0x0c1, 
	oKB_Acircumflex = 0x0c2, 
	oKB_Atilde = 0x0c3, 
	oKB_Adiaeresis = 0x0c4, 
	oKB_Aring = 0x0c5, 
	oKB_AE = 0x0c6, 
	oKB_Ccedilla = 0x0c7, 
	oKB_Egrave = 0x0c8, 
	oKB_Eacute = 0x0c9, 
	oKB_Ecircumflex = 0x0ca, 
	oKB_Ediaeresis = 0x0cb, 
	oKB_Igrave = 0x0cc, 
	oKB_Iacute = 0x0cd, 
	oKB_Icircumflex = 0x0ce, 
	oKB_Idiaeresis = 0x0cf, 
	oKB_ETH = 0x0d0, 
	oKB_Eth = 0x0d0, /* deprecated*/
	oKB_Ntilde = 0x0d1, 
	oKB_Ograve = 0x0d2, 
	oKB_Oacute = 0x0d3, 
	oKB_Ocircumflex = 0x0d4, 
	oKB_Otilde = 0x0d5, 
	oKB_Odiaeresis = 0x0d6, 
	oKB_multiply = 0x0d7, 
	oKB_Ooblique = 0x0d8, 
	oKB_Ugrave = 0x0d9, 
	oKB_Uacute = 0x0da, 
	oKB_Ucircumflex = 0x0db, 
	oKB_Udiaeresis = 0x0dc, 
	oKB_Yacute = 0x0dd, 
	oKB_THORN = 0x0de, 
	oKB_Thorn = 0x0de, /* deprecated*/
	oKB_ssharp = 0x0df, 
	oKB_agrave = 0x0e0, 
	oKB_aacute = 0x0e1, 
	oKB_acircumflex = 0x0e2, 
	oKB_atilde = 0x0e3, 
	oKB_adiaeresis = 0x0e4, 
	oKB_aring = 0x0e5, 
	oKB_ae = 0x0e6, 
	oKB_ccedilla = 0x0e7, 
	oKB_egrave = 0x0e8, 
	oKB_eacute = 0x0e9, 
	oKB_ecircumflex = 0x0ea, 
	oKB_ediaeresis = 0x0eb, 
	oKB_igrave = 0x0ec, 
	oKB_iacute = 0x0ed, 
	oKB_icircumflex = 0x0ee, 
	oKB_idiaeresis = 0x0ef, 
	oKB_eth = 0x0f0, 
	oKB_ntilde = 0x0f1, 
	oKB_ograve = 0x0f2, 
	oKB_oacute = 0x0f3, 
	oKB_ocircumflex = 0x0f4, 
	oKB_otilde = 0x0f5, 
	oKB_odiaeresis = 0x0f6, 
	oKB_division = 0x0f7, 
	oKB_oslash = 0x0f8, 
	oKB_ugrave = 0x0f9, 
	oKB_uacute = 0x0fa, 
	oKB_ucircumflex = 0x0fb, 
	oKB_udiaeresis = 0x0fc, 
	oKB_yacute = 0x0fd, 
	oKB_thorn = 0x0fe, 
	oKB_ydiaeresis = 0x0ff, 
#endif/* XK_LATIN1*/
 
/*
 * Latin 2
 * Byte 3 = 1
 */
 
#ifdef XK_LATIN2
	oKB_Aogonek = 0x1a1, 
	oKB_breve = 0x1a2, 
	oKB_Lstroke = 0x1a3, 
	oKB_Lcaron = 0x1a5, 
	oKB_Sacute = 0x1a6, 
	oKB_Scaron = 0x1a9, 
	oKB_Scedilla = 0x1aa, 
	oKB_Tcaron = 0x1ab, 
	oKB_Zacute = 0x1ac, 
	oKB_Zcaron = 0x1ae, 
	oKB_Zabovedot = 0x1af, 
	oKB_aogonek = 0x1b1, 
	oKB_ogonek = 0x1b2, 
	oKB_lstroke = 0x1b3, 
	oKB_lcaron = 0x1b5, 
	oKB_sacute = 0x1b6, 
	oKB_caron = 0x1b7, 
	oKB_scaron = 0x1b9, 
	oKB_scedilla = 0x1ba, 
	oKB_tcaron = 0x1bb, 
	oKB_zacute = 0x1bc, 
	oKB_doubleacute = 0x1bd, 
	oKB_zcaron = 0x1be, 
	oKB_zabovedot = 0x1bf, 
	oKB_Racute = 0x1c0, 
	oKB_Abreve = 0x1c3, 
	oKB_Lacute = 0x1c5, 
	oKB_Cacute = 0x1c6, 
	oKB_Ccaron = 0x1c8, 
	oKB_Eogonek = 0x1ca, 
	oKB_Ecaron = 0x1cc, 
	oKB_Dcaron = 0x1cf, 
	oKB_Dstroke = 0x1d0, 
	oKB_Nacute = 0x1d1, 
	oKB_Ncaron = 0x1d2, 
	oKB_Odoubleacute = 0x1d5, 
	oKB_Rcaron = 0x1d8, 
	oKB_Uring = 0x1d9, 
	oKB_Udoubleacute = 0x1db, 
	oKB_Tcedilla = 0x1de, 
	oKB_racute = 0x1e0, 
	oKB_abreve = 0x1e3, 
	oKB_lacute = 0x1e5, 
	oKB_cacute = 0x1e6, 
	oKB_ccaron = 0x1e8, 
	oKB_eogonek = 0x1ea, 
	oKB_ecaron = 0x1ec, 
	oKB_dcaron = 0x1ef, 
	oKB_dstroke = 0x1f0, 
	oKB_nacute = 0x1f1, 
	oKB_ncaron = 0x1f2, 
	oKB_odoubleacute = 0x1f5, 
	oKB_udoubleacute = 0x1fb, 
	oKB_rcaron = 0x1f8, 
	oKB_uring = 0x1f9, 
	oKB_tcedilla = 0x1fe, 
	oKB_abovedot = 0x1ff, 
#endif/* XK_LATIN2*/
 
/*
 * Latin 3
 * Byte 3 = 2
 */
 
#ifdef XK_LATIN3
	oKB_Hstroke = 0x2a1, 
	oKB_Hcircumflex = 0x2a6, 
	oKB_Iabovedot = 0x2a9, 
	oKB_Gbreve = 0x2ab, 
	oKB_Jcircumflex = 0x2ac, 
	oKB_hstroke = 0x2b1, 
	oKB_hcircumflex = 0x2b6, 
	oKB_idotless = 0x2b9, 
	oKB_gbreve = 0x2bb, 
	oKB_jcircumflex = 0x2bc, 
	oKB_Cabovedot = 0x2c5, 
	oKB_Ccircumflex = 0x2c6, 
	oKB_Gabovedot = 0x2d5, 
	oKB_Gcircumflex = 0x2d8, 
	oKB_Ubreve = 0x2dd, 
	oKB_Scircumflex = 0x2de, 
	oKB_cabovedot = 0x2e5, 
	oKB_ccircumflex = 0x2e6, 
	oKB_gabovedot = 0x2f5, 
	oKB_gcircumflex = 0x2f8, 
	oKB_ubreve = 0x2fd, 
	oKB_scircumflex = 0x2fe, 
#endif/* XK_LATIN3*/
 
 
/*
 * Latin 4
 * Byte 3 = 3
 */
 
#ifdef XK_LATIN4
	oKB_kra = 0x3a2, 
	oKB_kappa = 0x3a2, /* deprecated*/
	oKB_Rcedilla = 0x3a3, 
	oKB_Itilde = 0x3a5, 
	oKB_Lcedilla = 0x3a6, 
	oKB_Emacron = 0x3aa, 
	oKB_Gcedilla = 0x3ab, 
	oKB_Tslash = 0x3ac, 
	oKB_rcedilla = 0x3b3, 
	oKB_itilde = 0x3b5, 
	oKB_lcedilla = 0x3b6, 
	oKB_emacron = 0x3ba, 
	oKB_gcedilla = 0x3bb, 
	oKB_tslash = 0x3bc, 
	oKB_ENG = 0x3bd, 
	oKB_eng = 0x3bf, 
	oKB_Amacron = 0x3c0, 
	oKB_Iogonek = 0x3c7, 
	oKB_Eabovedot = 0x3cc, 
	oKB_Imacron = 0x3cf, 
	oKB_Ncedilla = 0x3d1, 
	oKB_Omacron = 0x3d2, 
	oKB_Kcedilla = 0x3d3, 
	oKB_Uogonek = 0x3d9, 
	oKB_Utilde = 0x3dd, 
	oKB_Umacron = 0x3de, 
	oKB_amacron = 0x3e0, 
	oKB_iogonek = 0x3e7, 
	oKB_eabovedot = 0x3ec, 
	oKB_imacron = 0x3ef, 
	oKB_ncedilla = 0x3f1, 
	oKB_omacron = 0x3f2, 
	oKB_kcedilla = 0x3f3, 
	oKB_uogonek = 0x3f9, 
	oKB_utilde = 0x3fd, 
	oKB_umacron = 0x3fe, 
#endif/* XK_LATIN4*/
 
/*
 * Katakana
 * Byte 3 = 4
 */
 
#ifdef XK_KATAKANA
	oKB_overline = 0x47e, 
	oKB_kana_fullstop = 0x4a1, 
	oKB_kana_openingbracket = 0x4a2, 
	oKB_kana_closingbracket = 0x4a3, 
	oKB_kana_comma = 0x4a4, 
	oKB_kana_conjunctive = 0x4a5, 
	oKB_kana_middledot = 0x4a5, /* deprecated*/
	oKB_kana_WO = 0x4a6, 
	oKB_kana_a = 0x4a7, 
	oKB_kana_i = 0x4a8, 
	oKB_kana_u = 0x4a9, 
	oKB_kana_e = 0x4aa, 
	oKB_kana_o = 0x4ab, 
	oKB_kana_ya = 0x4ac, 
	oKB_kana_yu = 0x4ad, 
	oKB_kana_yo = 0x4ae, 
	oKB_kana_tsu = 0x4af, 
	oKB_kana_tu = 0x4af, /* deprecated*/
	oKB_prolongedsound = 0x4b0, 
	oKB_kana_A = 0x4b1, 
	oKB_kana_I = 0x4b2, 
	oKB_kana_U = 0x4b3, 
	oKB_kana_E = 0x4b4, 
	oKB_kana_O = 0x4b5, 
	oKB_kana_KA = 0x4b6, 
	oKB_kana_KI = 0x4b7, 
	oKB_kana_KU = 0x4b8, 
	oKB_kana_KE = 0x4b9, 
	oKB_kana_KO = 0x4ba, 
	oKB_kana_SA = 0x4bb, 
	oKB_kana_SHI = 0x4bc, 
	oKB_kana_SU = 0x4bd, 
	oKB_kana_SE = 0x4be, 
	oKB_kana_SO = 0x4bf, 
	oKB_kana_TA = 0x4c0, 
	oKB_kana_CHI = 0x4c1, 
	oKB_kana_TI = 0x4c1, /* deprecated*/
	oKB_kana_TSU = 0x4c2, 
	oKB_kana_TU = 0x4c2, /* deprecated*/
	oKB_kana_TE = 0x4c3, 
	oKB_kana_TO = 0x4c4, 
	oKB_kana_NA = 0x4c5, 
	oKB_kana_NI = 0x4c6, 
	oKB_kana_NU = 0x4c7, 
	oKB_kana_NE = 0x4c8, 
	oKB_kana_NO = 0x4c9, 
	oKB_kana_HA = 0x4ca, 
	oKB_kana_HI = 0x4cb, 
	oKB_kana_FU = 0x4cc, 
	oKB_kana_HU = 0x4cc, /* deprecated*/
	oKB_kana_HE = 0x4cd, 
	oKB_kana_HO = 0x4ce, 
	oKB_kana_MA = 0x4cf, 
	oKB_kana_MI = 0x4d0, 
	oKB_kana_MU = 0x4d1, 
	oKB_kana_ME = 0x4d2, 
	oKB_kana_MO = 0x4d3, 
	oKB_kana_YA = 0x4d4, 
	oKB_kana_YU = 0x4d5, 
	oKB_kana_YO = 0x4d6, 
	oKB_kana_RA = 0x4d7, 
	oKB_kana_RI = 0x4d8, 
	oKB_kana_RU = 0x4d9, 
	oKB_kana_RE = 0x4da, 
	oKB_kana_RO = 0x4db, 
	oKB_kana_WA = 0x4dc, 
	oKB_kana_N = 0x4dd, 
	oKB_voicedsound = 0x4de, 
	oKB_semivoicedsound = 0x4df, 
	oKB_kana_switch = 0xFF7E, /* Alias for mode_switch*/
#endif/* XK_KATAKANA*/
 
/*
 * Arabic
 * Byte 3 = 5
 */
 
#ifdef XK_ARABIC
	oKB_Arabic_comma = 0x5ac, 
	oKB_Arabic_semicolon = 0x5bb, 
	oKB_Arabic_question_mark = 0x5bf, 
	oKB_Arabic_hamza = 0x5c1, 
	oKB_Arabic_maddaonalef = 0x5c2, 
	oKB_Arabic_hamzaonalef = 0x5c3, 
	oKB_Arabic_hamzaonwaw = 0x5c4, 
	oKB_Arabic_hamzaunderalef = 0x5c5, 
	oKB_Arabic_hamzaonyeh = 0x5c6, 
	oKB_Arabic_alef = 0x5c7, 
	oKB_Arabic_beh = 0x5c8, 
	oKB_Arabic_tehmarbuta = 0x5c9, 
	oKB_Arabic_teh = 0x5ca, 
	oKB_Arabic_theh = 0x5cb, 
	oKB_Arabic_jeem = 0x5cc, 
	oKB_Arabic_hah = 0x5cd, 
	oKB_Arabic_khah = 0x5ce, 
	oKB_Arabic_dal = 0x5cf, 
	oKB_Arabic_thal = 0x5d0, 
	oKB_Arabic_ra = 0x5d1, 
	oKB_Arabic_zain = 0x5d2, 
	oKB_Arabic_seen = 0x5d3, 
	oKB_Arabic_sheen = 0x5d4, 
	oKB_Arabic_sad = 0x5d5, 
	oKB_Arabic_dad = 0x5d6, 
	oKB_Arabic_tah = 0x5d7, 
	oKB_Arabic_zah = 0x5d8, 
	oKB_Arabic_ain = 0x5d9, 
	oKB_Arabic_ghain = 0x5da, 
	oKB_Arabic_tatweel = 0x5e0, 
	oKB_Arabic_feh = 0x5e1, 
	oKB_Arabic_qaf = 0x5e2, 
	oKB_Arabic_kaf = 0x5e3, 
	oKB_Arabic_lam = 0x5e4, 
	oKB_Arabic_meem = 0x5e5, 
	oKB_Arabic_noon = 0x5e6, 
	oKB_Arabic_ha = 0x5e7, 
	oKB_Arabic_heh = 0x5e7, /* deprecated*/
	oKB_Arabic_waw = 0x5e8, 
	oKB_Arabic_alefmaksura = 0x5e9, 
	oKB_Arabic_yeh = 0x5ea, 
	oKB_Arabic_fathatan = 0x5eb, 
	oKB_Arabic_dammatan = 0x5ec, 
	oKB_Arabic_kasratan = 0x5ed, 
	oKB_Arabic_fatha = 0x5ee, 
	oKB_Arabic_damma = 0x5ef, 
	oKB_Arabic_kasra = 0x5f0, 
	oKB_Arabic_shadda = 0x5f1, 
	oKB_Arabic_sukun = 0x5f2, 
	oKB_Arabic_switch = 0xFF7E, /* Alias for mode_switch*/
#endif/* XK_ARABIC*/
 
/*
 * Cyrillic
 * Byte 3 = 6
 */
#ifdef XK_CYRILLIC
	oKB_Serbian_dje = 0x6a1, 
	oKB_Macedonia_gje = 0x6a2, 
	oKB_Cyrillic_io = 0x6a3, 
	oKB_Ukrainian_ie = 0x6a4, 
	oKB_Ukranian_je = 0x6a4, /* deprecated*/
	oKB_Macedonia_dse = 0x6a5, 
	oKB_Ukrainian_i = 0x6a6, 
	oKB_Ukranian_i = 0x6a6, /* deprecated*/
	oKB_Ukrainian_yi = 0x6a7, 
	oKB_Ukranian_yi = 0x6a7, /* deprecated*/
	oKB_Cyrillic_je = 0x6a8, 
	oKB_Serbian_je = 0x6a8, /* deprecated*/
	oKB_Cyrillic_lje = 0x6a9, 
	oKB_Serbian_lje = 0x6a9, /* deprecated*/
	oKB_Cyrillic_nje = 0x6aa, 
	oKB_Serbian_nje = 0x6aa, /* deprecated*/
	oKB_Serbian_tshe = 0x6ab, 
	oKB_Macedonia_kje = 0x6ac, 
	oKB_Byelorussian_shortu = 0x6ae, 
	oKB_Cyrillic_dzhe = 0x6af, 
	oKB_Serbian_dze = 0x6af, /* deprecated*/
	oKB_numerosign = 0x6b0, 
	oKB_Serbian_DJE = 0x6b1, 
	oKB_Macedonia_GJE = 0x6b2, 
	oKB_Cyrillic_IO = 0x6b3, 
	oKB_Ukrainian_IE = 0x6b4, 
	oKB_Ukranian_JE = 0x6b4, /* deprecated*/
	oKB_Macedonia_DSE = 0x6b5, 
	oKB_Ukrainian_I = 0x6b6, 
	oKB_Ukranian_I = 0x6b6, /* deprecated*/
	oKB_Ukrainian_YI = 0x6b7, 
	oKB_Ukranian_YI = 0x6b7, /* deprecated*/
	oKB_Cyrillic_JE = 0x6b8, 
	oKB_Serbian_JE = 0x6b8, /* deprecated*/
	oKB_Cyrillic_LJE = 0x6b9, 
	oKB_Serbian_LJE = 0x6b9, /* deprecated*/
	oKB_Cyrillic_NJE = 0x6ba, 
	oKB_Serbian_NJE = 0x6ba, /* deprecated*/
	oKB_Serbian_TSHE = 0x6bb, 
	oKB_Macedonia_KJE = 0x6bc, 
	oKB_Byelorussian_SHORTU = 0x6be, 
	oKB_Cyrillic_DZHE = 0x6bf, 
	oKB_Serbian_DZE = 0x6bf, /* deprecated*/
	oKB_Cyrillic_yu = 0x6c0, 
	oKB_Cyrillic_a = 0x6c1, 
	oKB_Cyrillic_be = 0x6c2, 
	oKB_Cyrillic_tse = 0x6c3, 
	oKB_Cyrillic_de = 0x6c4, 
	oKB_Cyrillic_ie = 0x6c5, 
	oKB_Cyrillic_ef = 0x6c6, 
	oKB_Cyrillic_ghe = 0x6c7, 
	oKB_Cyrillic_ha = 0x6c8, 
	oKB_Cyrillic_i = 0x6c9, 
	oKB_Cyrillic_shorti = 0x6ca, 
	oKB_Cyrillic_ka = 0x6cb, 
	oKB_Cyrillic_el = 0x6cc, 
	oKB_Cyrillic_em = 0x6cd, 
	oKB_Cyrillic_en = 0x6ce, 
	oKB_Cyrillic_o = 0x6cf, 
	oKB_Cyrillic_pe = 0x6d0, 
	oKB_Cyrillic_ya = 0x6d1, 
	oKB_Cyrillic_er = 0x6d2, 
	oKB_Cyrillic_es = 0x6d3, 
	oKB_Cyrillic_te = 0x6d4, 
	oKB_Cyrillic_u = 0x6d5, 
	oKB_Cyrillic_zhe = 0x6d6, 
	oKB_Cyrillic_ve = 0x6d7, 
	oKB_Cyrillic_softsign = 0x6d8, 
	oKB_Cyrillic_yeru = 0x6d9, 
	oKB_Cyrillic_ze = 0x6da, 
	oKB_Cyrillic_sha = 0x6db, 
	oKB_Cyrillic_e = 0x6dc, 
	oKB_Cyrillic_shcha = 0x6dd, 
	oKB_Cyrillic_che = 0x6de, 
	oKB_Cyrillic_hardsign = 0x6df, 
	oKB_Cyrillic_YU = 0x6e0, 
	oKB_Cyrillic_A = 0x6e1, 
	oKB_Cyrillic_BE = 0x6e2, 
	oKB_Cyrillic_TSE = 0x6e3, 
	oKB_Cyrillic_DE = 0x6e4, 
	oKB_Cyrillic_IE = 0x6e5, 
	oKB_Cyrillic_EF = 0x6e6, 
	oKB_Cyrillic_GHE = 0x6e7, 
	oKB_Cyrillic_HA = 0x6e8, 
	oKB_Cyrillic_I = 0x6e9, 
	oKB_Cyrillic_SHORTI = 0x6ea, 
	oKB_Cyrillic_KA = 0x6eb, 
	oKB_Cyrillic_EL = 0x6ec, 
	oKB_Cyrillic_EM = 0x6ed, 
	oKB_Cyrillic_EN = 0x6ee, 
	oKB_Cyrillic_O = 0x6ef, 
	oKB_Cyrillic_PE = 0x6f0, 
	oKB_Cyrillic_YA = 0x6f1, 
	oKB_Cyrillic_ER = 0x6f2, 
	oKB_Cyrillic_ES = 0x6f3, 
	oKB_Cyrillic_TE = 0x6f4, 
	oKB_Cyrillic_U = 0x6f5, 
	oKB_Cyrillic_ZHE = 0x6f6, 
	oKB_Cyrillic_VE = 0x6f7, 
	oKB_Cyrillic_SOFTSIGN = 0x6f8, 
	oKB_Cyrillic_YERU = 0x6f9, 
	oKB_Cyrillic_ZE = 0x6fa, 
	oKB_Cyrillic_SHA = 0x6fb, 
	oKB_Cyrillic_E = 0x6fc, 
	oKB_Cyrillic_SHCHA = 0x6fd, 
	oKB_Cyrillic_CHE = 0x6fe, 
	oKB_Cyrillic_HARDSIGN = 0x6ff, 
#endif/* XK_CYRILLIC*/
 
/*
 * Greek
 * Byte 3 = 7
 */
 
#ifdef XK_GREEK
	oKB_Greek_ALPHAaccent = 0x7a1, 
	oKB_Greek_EPSILONaccent = 0x7a2, 
	oKB_Greek_ETAaccent = 0x7a3, 
	oKB_Greek_IOTAaccent = 0x7a4, 
	oKB_Greek_IOTAdiaeresis = 0x7a5, 
	oKB_Greek_OMICRONaccent = 0x7a7, 
	oKB_Greek_UPSILONaccent = 0x7a8, 
	oKB_Greek_UPSILONdieresis = 0x7a9, 
	oKB_Greek_OMEGAaccent = 0x7ab, 
	oKB_Greek_accentdieresis = 0x7ae, 
	oKB_Greek_horizbar = 0x7af, 
	oKB_Greek_alphaaccent = 0x7b1, 
	oKB_Greek_epsilonaccent = 0x7b2, 
	oKB_Greek_etaaccent = 0x7b3, 
	oKB_Greek_iotaaccent = 0x7b4, 
	oKB_Greek_iotadieresis = 0x7b5, 
	oKB_Greek_iotaaccentdieresis = 0x7b6, 
	oKB_Greek_omicronaccent = 0x7b7, 
	oKB_Greek_upsilonaccent = 0x7b8, 
	oKB_Greek_upsilondieresis = 0x7b9, 
	oKB_Greek_upsilonaccentdieresis = 0x7ba, 
	oKB_Greek_omegaaccent = 0x7bb, 
	oKB_Greek_ALPHA = 0x7c1, 
	oKB_Greek_BETA = 0x7c2, 
	oKB_Greek_GAMMA = 0x7c3, 
	oKB_Greek_DELTA = 0x7c4, 
	oKB_Greek_EPSILON = 0x7c5, 
	oKB_Greek_ZETA = 0x7c6, 
	oKB_Greek_ETA = 0x7c7, 
	oKB_Greek_THETA = 0x7c8, 
	oKB_Greek_IOTA = 0x7c9, 
	oKB_Greek_KAPPA = 0x7ca, 
	oKB_Greek_LAMDA = 0x7cb, 
	oKB_Greek_LAMBDA = 0x7cb, 
	oKB_Greek_MU = 0x7cc, 
	oKB_Greek_NU = 0x7cd, 
	oKB_Greek_XI = 0x7ce, 
	oKB_Greek_OMICRON = 0x7cf, 
	oKB_Greek_PI = 0x7d0, 
	oKB_Greek_RHO = 0x7d1, 
	oKB_Greek_SIGMA = 0x7d2, 
	oKB_Greek_TAU = 0x7d4, 
	oKB_Greek_UPSILON = 0x7d5, 
	oKB_Greek_PHI = 0x7d6, 
	oKB_Greek_CHI = 0x7d7, 
	oKB_Greek_PSI = 0x7d8, 
	oKB_Greek_OMEGA = 0x7d9, 
	oKB_Greek_alpha = 0x7e1, 
	oKB_Greek_beta = 0x7e2, 
	oKB_Greek_gamma = 0x7e3, 
	oKB_Greek_delta = 0x7e4, 
	oKB_Greek_epsilon = 0x7e5, 
	oKB_Greek_zeta = 0x7e6, 
	oKB_Greek_eta = 0x7e7, 
	oKB_Greek_theta = 0x7e8, 
	oKB_Greek_iota = 0x7e9, 
	oKB_Greek_kappa = 0x7ea, 
	oKB_Greek_lamda = 0x7eb, 
	oKB_Greek_lambda = 0x7eb, 
	oKB_Greek_mu = 0x7ec, 
	oKB_Greek_nu = 0x7ed, 
	oKB_Greek_xi = 0x7ee, 
	oKB_Greek_omicron = 0x7ef, 
	oKB_Greek_pi = 0x7f0, 
	oKB_Greek_rho = 0x7f1, 
	oKB_Greek_sigma = 0x7f2, 
	oKB_Greek_finalsmallsigma = 0x7f3, 
	oKB_Greek_tau = 0x7f4, 
	oKB_Greek_upsilon = 0x7f5, 
	oKB_Greek_phi = 0x7f6, 
	oKB_Greek_chi = 0x7f7, 
	oKB_Greek_psi = 0x7f8, 
	oKB_Greek_omega = 0x7f9, 
	oKB_Greek_switch = 0xFF7E, /* Alias for mode_switch*/
#endif/* XK_GREEK*/
 
/*
 * Technical
 * Byte 3 = 8
 */
 
#ifdef XK_TECHNICAL
	oKB_leftradical = 0x8a1, 
	oKB_topleftradical = 0x8a2, 
	oKB_horizconnector = 0x8a3, 
	oKB_topintegral = 0x8a4, 
	oKB_botintegral = 0x8a5, 
	oKB_vertconnector = 0x8a6, 
	oKB_topleftsqbracket = 0x8a7, 
	oKB_botleftsqbracket = 0x8a8, 
	oKB_toprightsqbracket = 0x8a9, 
	oKB_botrightsqbracket = 0x8aa, 
	oKB_topleftparens = 0x8ab, 
	oKB_botleftparens = 0x8ac, 
	oKB_toprightparens = 0x8ad, 
	oKB_botrightparens = 0x8ae, 
	oKB_leftmiddlecurlybrace = 0x8af, 
	oKB_rightmiddlecurlybrace = 0x8b0, 
	oKB_topleftsummation = 0x8b1, 
	oKB_botleftsummation = 0x8b2, 
	oKB_topvertsummationconnector = 0x8b3, 
	oKB_botvertsummationconnector = 0x8b4, 
	oKB_toprightsummation = 0x8b5, 
	oKB_botrightsummation = 0x8b6, 
	oKB_rightmiddlesummation = 0x8b7, 
	oKB_lessthanequal = 0x8bc, 
	oKB_notequal = 0x8bd, 
	oKB_greaterthanequal = 0x8be, 
	oKB_integral = 0x8bf, 
	oKB_therefore = 0x8c0, 
	oKB_variation = 0x8c1, 
	oKB_infinity = 0x8c2, 
	oKB_nabla = 0x8c5, 
	oKB_approximate = 0x8c8, 
	oKB_similarequal = 0x8c9, 
	oKB_ifonlyif = 0x8cd, 
	oKB_implies = 0x8ce, 
	oKB_identical = 0x8cf, 
	oKB_radical = 0x8d6, 
	oKB_includedin = 0x8da, 
	oKB_includes = 0x8db, 
	oKB_intersection = 0x8dc, 
	oKB_union = 0x8dd, 
	oKB_logicaland = 0x8de, 
	oKB_logicalor = 0x8df, 
	oKB_partialderivative = 0x8ef, 
	oKB_function = 0x8f6, 
	oKB_leftarrow = 0x8fb, 
	oKB_uparrow = 0x8fc, 
	oKB_rightarrow = 0x8fd, 
	oKB_downarrow = 0x8fe, 
#endif/* XK_TECHNICAL*/
 
/*
 * Special
 * Byte 3 = 9
 */
 
#ifdef XK_SPECIAL
	oKB_blank = 0x9df, 
	oKB_soliddiamond = 0x9e0, 
	oKB_checkerboard = 0x9e1, 
	oKB_ht = 0x9e2, 
	oKB_ff = 0x9e3, 
	oKB_cr = 0x9e4, 
	oKB_lf = 0x9e5, 
	oKB_nl = 0x9e8, 
	oKB_vt = 0x9e9, 
	oKB_lowrightcorner = 0x9ea, 
	oKB_uprightcorner = 0x9eb, 
	oKB_upleftcorner = 0x9ec, 
	oKB_lowleftcorner = 0x9ed, 
	oKB_crossinglines = 0x9ee, 
	oKB_horizlinescan1 = 0x9ef, 
	oKB_horizlinescan3 = 0x9f0, 
	oKB_horizlinescan5 = 0x9f1, 
	oKB_horizlinescan7 = 0x9f2, 
	oKB_horizlinescan9 = 0x9f3, 
	oKB_leftt = 0x9f4, 
	oKB_rightt = 0x9f5, 
	oKB_bott = 0x9f6, 
	oKB_topt = 0x9f7, 
	oKB_vertbar = 0x9f8, 
#endif/* XK_SPECIAL*/
 
/*
 * Publishing
 * Byte 3 = a
 */
 
#ifdef XK_PUBLISHING
	oKB_emspace = 0xaa1, 
	oKB_enspace = 0xaa2, 
	oKB_em3space = 0xaa3, 
	oKB_em4space = 0xaa4, 
	oKB_digitspace = 0xaa5, 
	oKB_punctspace = 0xaa6, 
	oKB_thinspace = 0xaa7, 
	oKB_hairspace = 0xaa8, 
	oKB_emdash = 0xaa9, 
	oKB_endash = 0xaaa, 
	oKB_signifblank = 0xaac, 
	oKB_ellipsis = 0xaae, 
	oKB_doubbaselinedot = 0xaaf, 
	oKB_onethird = 0xab0, 
	oKB_twothirds = 0xab1, 
	oKB_onefifth = 0xab2, 
	oKB_twofifths = 0xab3, 
	oKB_threefifths = 0xab4, 
	oKB_fourfifths = 0xab5, 
	oKB_onesixth = 0xab6, 
	oKB_fivesixths = 0xab7, 
	oKB_careof = 0xab8, 
	oKB_figdash = 0xabb, 
	oKB_leftanglebracket = 0xabc, 
	oKB_decimalpoint = 0xabd, 
	oKB_rightanglebracket = 0xabe, 
	oKB_marker = 0xabf, 
	oKB_oneeighth = 0xac3, 
	oKB_threeeighths = 0xac4, 
	oKB_fiveeighths = 0xac5, 
	oKB_seveneighths = 0xac6, 
	oKB_trademark = 0xac9, 
	oKB_signaturemark = 0xaca, 
	oKB_trademarkincircle = 0xacb, 
	oKB_leftopentriangle = 0xacc, 
	oKB_rightopentriangle = 0xacd, 
	oKB_emopencircle = 0xace, 
	oKB_emopenrectangle = 0xacf, 
	oKB_leftsinglequotemark = 0xad0, 
	oKB_rightsinglequotemark = 0xad1, 
	oKB_leftdoublequotemark = 0xad2, 
	oKB_rightdoublequotemark = 0xad3, 
	oKB_prescription = 0xad4, 
	oKB_minutes = 0xad6, 
	oKB_seconds = 0xad7, 
	oKB_latincross = 0xad9, 
	oKB_hexagram = 0xada, 
	oKB_filledrectbullet = 0xadb, 
	oKB_filledlefttribullet = 0xadc, 
	oKB_filledrighttribullet = 0xadd, 
	oKB_emfilledcircle = 0xade, 
	oKB_emfilledrect = 0xadf, 
	oKB_enopencircbullet = 0xae0, 
	oKB_enopensquarebullet = 0xae1, 
	oKB_openrectbullet = 0xae2, 
	oKB_opentribulletup = 0xae3, 
	oKB_opentribulletdown = 0xae4, 
	oKB_openstar = 0xae5, 
	oKB_enfilledcircbullet = 0xae6, 
	oKB_enfilledsqbullet = 0xae7, 
	oKB_filledtribulletup = 0xae8, 
	oKB_filledtribulletdown = 0xae9, 
	oKB_leftpointer = 0xaea, 
	oKB_rightpointer = 0xaeb, 
	oKB_club = 0xaec, 
	oKB_diamond = 0xaed, 
	oKB_heart = 0xaee, 
	oKB_maltesecross = 0xaf0, 
	oKB_dagger = 0xaf1, 
	oKB_doubledagger = 0xaf2, 
	oKB_checkmark = 0xaf3, 
	oKB_ballotcross = 0xaf4, 
	oKB_musicalsharp = 0xaf5, 
	oKB_musicalflat = 0xaf6, 
	oKB_malesymbol = 0xaf7, 
	oKB_femalesymbol = 0xaf8, 
	oKB_telephone = 0xaf9, 
	oKB_telephonerecorder = 0xafa, 
	oKB_phonographcopyright = 0xafb, 
	oKB_caret = 0xafc, 
	oKB_singlelowquotemark = 0xafd, 
	oKB_doublelowquotemark = 0xafe, 
	oKB_cursor = 0xaff, 
#endif/* XK_PUBLISHING*/
 
/*
 * APL
 * Byte 3 = b
 */
 
#ifdef XK_APL
	oKB_leftcaret = 0xba3, 
	oKB_rightcaret = 0xba6, 
	oKB_downcaret = 0xba8, 
	oKB_upcaret = 0xba9, 
	oKB_overbar = 0xbc0, 
	oKB_downtack = 0xbc2, 
	oKB_upshoe = 0xbc3, 
	oKB_downstile = 0xbc4, 
	oKB_underbar = 0xbc6, 
	oKB_jot = 0xbca, 
	oKB_quad = 0xbcc, 
	oKB_uptack = 0xbce, 
	oKB_circle = 0xbcf, 
	oKB_upstile = 0xbd3, 
	oKB_downshoe = 0xbd6, 
	oKB_rightshoe = 0xbd8, 
	oKB_leftshoe = 0xbda, 
	oKB_lefttack = 0xbdc, 
	oKB_righttack = 0xbfc, 
#endif/* XK_APL*/
 
/*
 * Hebrew
 * Byte 3 = c
 */
 
#ifdef XK_HEBREW
	oKB_hebrew_doublelowline = 0xcdf, 
	oKB_hebrew_aleph = 0xce0, 
	oKB_hebrew_bet = 0xce1, 
	oKB_hebrew_beth = 0xce1, /* deprecated*/
	oKB_hebrew_gimel = 0xce2, 
	oKB_hebrew_gimmel = 0xce2, /* deprecated*/
	oKB_hebrew_dalet = 0xce3, 
	oKB_hebrew_daleth = 0xce3, /* deprecated*/
	oKB_hebrew_he = 0xce4, 
	oKB_hebrew_waw = 0xce5, 
	oKB_hebrew_zain = 0xce6, 
	oKB_hebrew_zayin = 0xce6, /* deprecated*/
	oKB_hebrew_chet = 0xce7, 
	oKB_hebrew_het = 0xce7, /* deprecated*/
	oKB_hebrew_tet = 0xce8, 
	oKB_hebrew_teth = 0xce8, /* deprecated*/
	oKB_hebrew_yod = 0xce9, 
	oKB_hebrew_finalkaph = 0xcea, 
	oKB_hebrew_kaph = 0xceb, 
	oKB_hebrew_lamed = 0xcec, 
	oKB_hebrew_finalmem = 0xced, 
	oKB_hebrew_mem = 0xcee, 
	oKB_hebrew_finalnun = 0xcef, 
	oKB_hebrew_nun = 0xcf0, 
	oKB_hebrew_samech = 0xcf1, 
	oKB_hebrew_samekh = 0xcf1, /* deprecated*/
	oKB_hebrew_ayin = 0xcf2, 
	oKB_hebrew_finalpe = 0xcf3, 
	oKB_hebrew_pe = 0xcf4, 
	oKB_hebrew_finalzade = 0xcf5, 
	oKB_hebrew_finalzadi = 0xcf5, /* deprecated*/
	oKB_hebrew_zade = 0xcf6, 
	oKB_hebrew_zadi = 0xcf6, /* deprecated*/
	oKB_hebrew_qoph = 0xcf7, 
	oKB_hebrew_kuf = 0xcf7, /* deprecated*/
	oKB_hebrew_resh = 0xcf8, 
	oKB_hebrew_shin = 0xcf9, 
	oKB_hebrew_taw = 0xcfa, 
	oKB_hebrew_taf = 0xcfa, /* deprecated*/
	oKB_Hebrew_switch = 0xFF7E, /* Alias for mode_switch*/
#endif/* XK_HEBREW*/
 
/*
 * Thai
 * Byte 3 = d
 */
 
#ifdef XK_THAI
	oKB_Thai_kokai = 0xda1, 
	oKB_Thai_khokhai = 0xda2, 
	oKB_Thai_khokhuat = 0xda3, 
	oKB_Thai_khokhwai = 0xda4, 
	oKB_Thai_khokhon = 0xda5, 
	oKB_Thai_khorakhang = 0xda6, 
	oKB_Thai_ngongu = 0xda7, 
	oKB_Thai_chochan = 0xda8, 
	oKB_Thai_choching = 0xda9, 
	oKB_Thai_chochang = 0xdaa, 
	oKB_Thai_soso = 0xdab, 
	oKB_Thai_chochoe = 0xdac, 
	oKB_Thai_yoying = 0xdad, 
	oKB_Thai_dochada = 0xdae, 
	oKB_Thai_topatak = 0xdaf, 
	oKB_Thai_thothan = 0xdb0, 
	oKB_Thai_thonangmontho = 0xdb1, 
	oKB_Thai_thophuthao = 0xdb2, 
	oKB_Thai_nonen = 0xdb3, 
	oKB_Thai_dodek = 0xdb4, 
	oKB_Thai_totao = 0xdb5, 
	oKB_Thai_thothung = 0xdb6, 
	oKB_Thai_thothahan = 0xdb7, 
	oKB_Thai_thothong = 0xdb8, 
	oKB_Thai_nonu = 0xdb9, 
	oKB_Thai_bobaimai = 0xdba, 
	oKB_Thai_popla = 0xdbb, 
	oKB_Thai_phophung = 0xdbc, 
	oKB_Thai_fofa = 0xdbd, 
	oKB_Thai_phophan = 0xdbe, 
	oKB_Thai_fofan = 0xdbf, 
	oKB_Thai_phosamphao = 0xdc0, 
	oKB_Thai_moma = 0xdc1, 
	oKB_Thai_yoyak = 0xdc2, 
	oKB_Thai_rorua = 0xdc3, 
	oKB_Thai_ru = 0xdc4, 
	oKB_Thai_loling = 0xdc5, 
	oKB_Thai_lu = 0xdc6, 
	oKB_Thai_wowaen = 0xdc7, 
	oKB_Thai_sosala = 0xdc8, 
	oKB_Thai_sorusi = 0xdc9, 
	oKB_Thai_sosua = 0xdca, 
	oKB_Thai_hohip = 0xdcb, 
	oKB_Thai_lochula = 0xdcc, 
	oKB_Thai_oang = 0xdcd, 
	oKB_Thai_honokhuk = 0xdce, 
	oKB_Thai_paiyannoi = 0xdcf, 
	oKB_Thai_saraa = 0xdd0, 
	oKB_Thai_maihanakat = 0xdd1, 
	oKB_Thai_saraaa = 0xdd2, 
	oKB_Thai_saraam = 0xdd3, 
	oKB_Thai_sarai = 0xdd4, 
	oKB_Thai_saraii = 0xdd5, 
	oKB_Thai_saraue = 0xdd6, 
	oKB_Thai_sarauee = 0xdd7, 
	oKB_Thai_sarau = 0xdd8, 
	oKB_Thai_sarauu = 0xdd9, 
	oKB_Thai_phinthu = 0xdda, 
	oKB_Thai_maihanakat_maitho = 0xdde, 
	oKB_Thai_baht = 0xddf, 
	oKB_Thai_sarae = 0xde0, 
	oKB_Thai_saraae = 0xde1, 
	oKB_Thai_sarao = 0xde2, 
	oKB_Thai_saraaimaimuan = 0xde3, 
	oKB_Thai_saraaimaimalai = 0xde4, 
	oKB_Thai_lakkhangyao = 0xde5, 
	oKB_Thai_maiyamok = 0xde6, 
	oKB_Thai_maitaikhu = 0xde7, 
	oKB_Thai_maiek = 0xde8, 
	oKB_Thai_maitho = 0xde9, 
	oKB_Thai_maitri = 0xdea, 
	oKB_Thai_maichattawa = 0xdeb, 
	oKB_Thai_thanthakhat = 0xdec, 
	oKB_Thai_nikhahit = 0xded, 
	oKB_Thai_leksun = 0xdf0, 
	oKB_Thai_leknung = 0xdf1, 
	oKB_Thai_leksong = 0xdf2, 
	oKB_Thai_leksam = 0xdf3, 
	oKB_Thai_leksi = 0xdf4, 
	oKB_Thai_lekha = 0xdf5, 
	oKB_Thai_lekhok = 0xdf6, 
	oKB_Thai_lekchet = 0xdf7, 
	oKB_Thai_lekpaet = 0xdf8, 
	oKB_Thai_lekkao = 0xdf9, 
#endif/* XK_THAI*/
 
/*
 * Korean
 * Byte 3 = e
 */

#ifdef XK_KOREAN
 
	oKB_Hangul = 0xff31, /* Hangul start/stop(toggle)*/
	oKB_Hangul_Start = 0xff32, /* Hangul start*/
	oKB_Hangul_End = 0xff33, /* Hangul end, English start*/
	oKB_Hangul_Hanja = 0xff34, /* Start Hangul->Hanja Conversion*/
	oKB_Hangul_Jamo = 0xff35, /* Hangul Jamo mode*/
	oKB_Hangul_Romaja = 0xff36, /* Hangul Romaja mode*/
	oKB_Hangul_Codeinput = 0xff37, /* Hangul code input mode*/
	oKB_Hangul_Jeonja = 0xff38, /* Jeonja mode*/
	oKB_Hangul_Banja = 0xff39, /* Banja mode*/
	oKB_Hangul_PreHanja = 0xff3a, /* Pre Hanja conversion*/
	oKB_Hangul_PostHanja = 0xff3b, /* Post Hanja conversion*/
	oKB_Hangul_SingleCandidate = 0xff3c, /* Single candidate*/
	oKB_Hangul_MultipleCandidate = 0xff3d, /* Multiple candidate*/
	oKB_Hangul_PreviousCandidate = 0xff3e, /* Previous candidate*/
	oKB_Hangul_Special = 0xff3f, /* Special symbols*/
	oKB_Hangul_switch = 0xFF7E, /* Alias for mode_switch*/

	/* Hangul Consonant Characters*/
	oKB_Hangul_Kiyeog = 0xea1, 
	oKB_Hangul_SsangKiyeog = 0xea2, 
	oKB_Hangul_KiyeogSios = 0xea3, 
	oKB_Hangul_Nieun = 0xea4, 
	oKB_Hangul_NieunJieuj = 0xea5, 
	oKB_Hangul_NieunHieuh = 0xea6, 
	oKB_Hangul_Dikeud = 0xea7, 
	oKB_Hangul_SsangDikeud = 0xea8, 
	oKB_Hangul_Rieul = 0xea9, 
	oKB_Hangul_RieulKiyeog = 0xeaa, 
	oKB_Hangul_RieulMieum = 0xeab, 
	oKB_Hangul_RieulPieub = 0xeac, 
	oKB_Hangul_RieulSios = 0xead, 
	oKB_Hangul_RieulTieut = 0xeae, 
	oKB_Hangul_RieulPhieuf = 0xeaf, 
	oKB_Hangul_RieulHieuh = 0xeb0, 
	oKB_Hangul_Mieum = 0xeb1, 
	oKB_Hangul_Pieub = 0xeb2, 
	oKB_Hangul_SsangPieub = 0xeb3, 
	oKB_Hangul_PieubSios = 0xeb4, 
	oKB_Hangul_Sios = 0xeb5, 
	oKB_Hangul_SsangSios = 0xeb6, 
	oKB_Hangul_Ieung = 0xeb7, 
	oKB_Hangul_Jieuj = 0xeb8, 
	oKB_Hangul_SsangJieuj = 0xeb9, 
	oKB_Hangul_Cieuc = 0xeba, 
	oKB_Hangul_Khieuq = 0xebb, 
	oKB_Hangul_Tieut = 0xebc, 
	oKB_Hangul_Phieuf = 0xebd, 
	oKB_Hangul_Hieuh = 0xebe, 
 
	/* Hangul Vowel Characters*/
	oKB_Hangul_A = 0xebf, 
	oKB_Hangul_AE = 0xec0, 
	oKB_Hangul_YA = 0xec1, 
	oKB_Hangul_YAE = 0xec2, 
	oKB_Hangul_EO = 0xec3, 
	oKB_Hangul_E = 0xec4, 
	oKB_Hangul_YEO = 0xec5, 
	oKB_Hangul_YE = 0xec6, 
	oKB_Hangul_O = 0xec7, 
	oKB_Hangul_WA = 0xec8, 
	oKB_Hangul_WAE = 0xec9, 
	oKB_Hangul_OE = 0xeca, 
	oKB_Hangul_YO = 0xecb, 
	oKB_Hangul_U = 0xecc, 
	oKB_Hangul_WEO = 0xecd, 
	oKB_Hangul_WE = 0xece, 
	oKB_Hangul_WI = 0xecf, 
	oKB_Hangul_YU = 0xed0, 
	oKB_Hangul_EU = 0xed1, 
	oKB_Hangul_YI = 0xed2, 
	oKB_Hangul_I = 0xed3, 
 
	/* Hangul syllable-final (JongSeong) Characters*/
	oKB_Hangul_J_Kiyeog = 0xed4, 
	oKB_Hangul_J_SsangKiyeog = 0xed5, 
	oKB_Hangul_J_KiyeogSios = 0xed6, 
	oKB_Hangul_J_Nieun = 0xed7, 
	oKB_Hangul_J_NieunJieuj = 0xed8, 
	oKB_Hangul_J_NieunHieuh = 0xed9, 
	oKB_Hangul_J_Dikeud = 0xeda, 
	oKB_Hangul_J_Rieul = 0xedb, 
	oKB_Hangul_J_RieulKiyeog = 0xedc, 
	oKB_Hangul_J_RieulMieum = 0xedd, 
	oKB_Hangul_J_RieulPieub = 0xede, 
	oKB_Hangul_J_RieulSios = 0xedf, 
	oKB_Hangul_J_RieulTieut = 0xee0, 
	oKB_Hangul_J_RieulPhieuf = 0xee1, 
	oKB_Hangul_J_RieulHieuh = 0xee2, 
	oKB_Hangul_J_Mieum = 0xee3, 
	oKB_Hangul_J_Pieub = 0xee4, 
	oKB_Hangul_J_PieubSios = 0xee5, 
	oKB_Hangul_J_Sios = 0xee6, 
	oKB_Hangul_J_SsangSios = 0xee7, 
	oKB_Hangul_J_Ieung = 0xee8, 
	oKB_Hangul_J_Jieuj = 0xee9, 
	oKB_Hangul_J_Cieuc = 0xeea, 
	oKB_Hangul_J_Khieuq = 0xeeb, 
	oKB_Hangul_J_Tieut = 0xeec, 
	oKB_Hangul_J_Phieuf = 0xeed, 
	oKB_Hangul_J_Hieuh = 0xeee, 
 
	/* Ancient Hangul Consonant Characters*/
	oKB_Hangul_RieulYeorinHieuh = 0xeef, 
	oKB_Hangul_SunkyeongeumMieum = 0xef0, 
	oKB_Hangul_SunkyeongeumPieub = 0xef1, 
	oKB_Hangul_PanSios = 0xef2, 
	oKB_Hangul_KkogjiDalrinIeung = 0xef3, 
	oKB_Hangul_SunkyeongeumPhieuf = 0xef4, 
	oKB_Hangul_YeorinHieuh = 0xef5, 
 
	/* Ancient Hangul Vowel Characters*/
	oKB_Hangul_AraeA = 0xef6, 
	oKB_Hangul_AraeAE = 0xef7, 
 
	/* Ancient Hangul syllable-final (JongSeong) Characters*/
	oKB_Hangul_J_PanSios = 0xef8, 
	oKB_Hangul_J_KkogjiDalrinIeung = 0xef9, 
	oKB_Hangul_J_YeorinHieuh = 0xefa, 
 
	/* Korean currency symbol*/
	oKB_Korean_Won = 0xeff, 
 
#endif/* XK_KOREAN*/
};

#endif
