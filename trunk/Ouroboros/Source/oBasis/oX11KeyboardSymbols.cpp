/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#include <oBasis/oX11KeyboardSymbols.h>

oRTTI_ENUM_BEGIN_DESCRIPTION(oRTTI_CAPS_ARRAY, oKEYBOARD_KEY)
	oRTTI_ENUM_BEGIN_VALUES(oKEYBOARD_KEY)

	oRTTI_VALUE(oKB_None)

#ifdef XK_BONES
	oRTTI_VALUE(oKB_Pos_Head)
	oRTTI_VALUE(oKB_Pos_Neck)
	oRTTI_VALUE(oKB_Pos_Right_Shoulder)
	oRTTI_VALUE(oKB_Pos_Right_Elbow)
	oRTTI_VALUE(oKB_Pos_Right_Wrist)
	oRTTI_VALUE(oKB_Pos_Right_Hand)
	oRTTI_VALUE(oKB_Pos_Left_Shoulder)
	oRTTI_VALUE(oKB_Pos_Left_Elbow)
	oRTTI_VALUE(oKB_Pos_Left_Wrist)
	oRTTI_VALUE(oKB_Pos_Left_Hand)
	oRTTI_VALUE(oKB_Pos_Back)
	oRTTI_VALUE(oKB_Pos_Hip)
	oRTTI_VALUE(oKB_Pos_Right_Hip)
	oRTTI_VALUE(oKB_Pos_Right_Knee)
	oRTTI_VALUE(oKB_Pos_Right_Ankle)
	oRTTI_VALUE(oKB_Pos_Right_Foot)
	oRTTI_VALUE(oKB_Pos_Left_Hip)
	oRTTI_VALUE(oKB_Pos_Left_Knee)
	oRTTI_VALUE(oKB_Pos_Left_Ankle)
	oRTTI_VALUE(oKB_Pos_Left_Foot)
#endif

#ifdef XK_TOUCH
	oRTTI_VALUE(oKB_Touch0)
	oRTTI_VALUE(oKB_Touch1)
	oRTTI_VALUE(oKB_Touch2)
	oRTTI_VALUE(oKB_Touch3)
	oRTTI_VALUE(oKB_Touch4)
	oRTTI_VALUE(oKB_Touch5)
	oRTTI_VALUE(oKB_Touch6)
	oRTTI_VALUE(oKB_Touch7)
	oRTTI_VALUE(oKB_Touch8)
	oRTTI_VALUE(oKB_Touch9)
#endif

	// Microsoft / Windows keyboard media keys
	oRTTI_VALUE(oKB_Volume_Down)
	oRTTI_VALUE(oKB_Volume_Mute)
	oRTTI_VALUE(oKB_Volume_Up)
	oRTTI_VALUE(oKB_Media_Play_Pause)
	oRTTI_VALUE(oKB_Media_Stop)
	oRTTI_VALUE(oKB_Media_Prev_Track)
	oRTTI_VALUE(oKB_Media_Next_Track)
	oRTTI_VALUE(oKB_Launch_Mail)
	oRTTI_VALUE(oKB_Browser_Search)
	oRTTI_VALUE(oKB_Browser_Home)
	oRTTI_VALUE(oKB_Browser_Back)
	oRTTI_VALUE(oKB_Browser_Forward)
	oRTTI_VALUE(oKB_Browser_Stop)
	oRTTI_VALUE(oKB_Browser_Refresh)
	oRTTI_VALUE(oKB_Browser_Favorites)
	oRTTI_VALUE(oKB_Launch_Media_Select)

	oRTTI_VALUE(oKB_VoidSymbol) /* void symbol*/
 
#ifdef XK_MISCELLANY
/*
 * TTY Functions, cleverly chosen to map to ascii, for convenience of
 * programming, but could have been arbitrary (at the cost of lookup
 * tables in client code.
 */
 
	oRTTI_VALUE(oKB_BackSpace) /* back space, back char*/
	oRTTI_VALUE(oKB_Tab) 
	oRTTI_VALUE(oKB_Linefeed) /* Linefeed, LF*/
	oRTTI_VALUE(oKB_Clear) 
	oRTTI_VALUE(oKB_Return) /* Return, enter*/
	oRTTI_VALUE(oKB_Pause) /* Pause, hold*/
	oRTTI_VALUE(oKB_Scroll_Lock) 
	oRTTI_VALUE(oKB_Sys_Req) 
	oRTTI_VALUE(oKB_Escape) 
	oRTTI_VALUE(oKB_Delete) /* Delete, rubout*/
 
/* International & multi-key character composition*/
 
oRTTI_VALUE(oKB_Multi_key) /* Multi-key character compose*/
 
/* Japanese keyboard support*/
 
	oRTTI_VALUE(oKB_Kanji) /* Kanji, Kanji convert*/
	oRTTI_VALUE(oKB_Muhenkan) /* Cancel Conversion*/
	oRTTI_VALUE(oKB_Henkan_Mode) /* Start/Stop Conversion*/
	oRTTI_VALUE(oKB_Henkan) /* Alias for Henkan_Mode*/
	oRTTI_VALUE(oKB_Romaji) /* to Romaji*/
	oRTTI_VALUE(oKB_Hiragana) /* to Hiragana*/
	oRTTI_VALUE(oKB_Katakana) /* to Katakana*/
	oRTTI_VALUE(oKB_Hiragana_Katakana) /* Hiragana/Katakana toggle*/
	oRTTI_VALUE(oKB_Zenkaku) /* to Zenkaku*/
	oRTTI_VALUE(oKB_Hankaku) /* to Hankaku*/
	oRTTI_VALUE(oKB_Zenkaku_Hankaku) /* Zenkaku/Hankaku toggle*/
	oRTTI_VALUE(oKB_Touroku) /* Add to Dictionary*/
	oRTTI_VALUE(oKB_Massyo) /* Delete from Dictionary*/
	oRTTI_VALUE(oKB_Kana_Lock) /* Kana Lock*/
	oRTTI_VALUE(oKB_Kana_Shift) /* Kana Shift*/
	oRTTI_VALUE(oKB_Eisu_Shift) /* Alphanumeric Shift*/
	oRTTI_VALUE(oKB_Eisu_toggle) /* Alphanumeric toggle*/
 
/* 0xFF31 thru 0xFF3F are under XK_KOREAN*/
 
/* Cursor control & motion*/
 
	oRTTI_VALUE(oKB_Home) 
	oRTTI_VALUE(oKB_Left) /* Move left, left arrow*/
	oRTTI_VALUE(oKB_Up) /* Move up, up arrow*/
	oRTTI_VALUE(oKB_Right) /* Move right, right arrow*/
	oRTTI_VALUE(oKB_Down) /* Move down, down arrow*/
	oRTTI_VALUE(oKB_Prior) /* Prior, previous*/
	oRTTI_VALUE(oKB_Page_Up) 
	oRTTI_VALUE(oKB_Next) /* Next*/
	oRTTI_VALUE(oKB_Page_Down) 
	oRTTI_VALUE(oKB_End) /* EOL*/
	oRTTI_VALUE(oKB_Begin) /* BOL*/
 
/* Misc Functions*/
 
	oRTTI_VALUE(oKB_Select) /* Select, mark*/
	oRTTI_VALUE(oKB_Print) 
	oRTTI_VALUE(oKB_Execute) /* Execute, run, do*/
	oRTTI_VALUE(oKB_Insert) /* Insert, insert here*/
	oRTTI_VALUE(oKB_Undo) /* Undo, oops*/
	oRTTI_VALUE(oKB_Redo) /* redo, again*/
	oRTTI_VALUE(oKB_Menu) 
	oRTTI_VALUE(oKB_Find) /* Find, search*/
	oRTTI_VALUE(oKB_Cancel) /* Cancel, stop, abort, exit*/
	oRTTI_VALUE(oKB_Help) /* Help*/
	oRTTI_VALUE(oKB_Break) 
	oRTTI_VALUE(oKB_Mode_switch) /* Character set switch*/
	oRTTI_VALUE(oKB_script_switch) /* Alias for mode_switch*/
	oRTTI_VALUE(oKB_Num_Lock) 
 
/* Keypad Functions, keypad numbers cleverly chosen to map to ascii*/
 
	oRTTI_VALUE(oKB_KP_Space) /* space*/
	oRTTI_VALUE(oKB_KP_Tab) 
	oRTTI_VALUE(oKB_KP_Enter) /* enter*/
	oRTTI_VALUE(oKB_KP_F1) /* PF1, KP_A, ...*/
	oRTTI_VALUE(oKB_KP_F2) 
	oRTTI_VALUE(oKB_KP_F3) 
	oRTTI_VALUE(oKB_KP_F4) 
	oRTTI_VALUE(oKB_KP_Home) 
	oRTTI_VALUE(oKB_KP_Left) 
	oRTTI_VALUE(oKB_KP_Up) 
	oRTTI_VALUE(oKB_KP_Right) 
	oRTTI_VALUE(oKB_KP_Down) 
	oRTTI_VALUE(oKB_KP_Prior) 
	oRTTI_VALUE(oKB_KP_Page_Up) 
	oRTTI_VALUE(oKB_KP_Next) 
	oRTTI_VALUE(oKB_KP_Page_Down) 
	oRTTI_VALUE(oKB_KP_End) 
	oRTTI_VALUE(oKB_KP_Begin) 
	oRTTI_VALUE(oKB_KP_Insert) 
	oRTTI_VALUE(oKB_KP_Delete) 
	oRTTI_VALUE(oKB_KP_Equal) /* equals*/
	oRTTI_VALUE(oKB_KP_Multiply) 
	oRTTI_VALUE(oKB_KP_Add) 
	oRTTI_VALUE(oKB_KP_Separator) /* separator, often comma*/
	oRTTI_VALUE(oKB_KP_Subtract) 
	oRTTI_VALUE(oKB_KP_Decimal) 
	oRTTI_VALUE(oKB_KP_Divide) 
 
	oRTTI_VALUE(oKB_KP_0) 
	oRTTI_VALUE(oKB_KP_1) 
	oRTTI_VALUE(oKB_KP_2) 
	oRTTI_VALUE(oKB_KP_3) 
	oRTTI_VALUE(oKB_KP_4) 
	oRTTI_VALUE(oKB_KP_5) 
	oRTTI_VALUE(oKB_KP_6) 
	oRTTI_VALUE(oKB_KP_7) 
	oRTTI_VALUE(oKB_KP_8) 
	oRTTI_VALUE(oKB_KP_9) 
 
 /*
 * Auxilliary Functions; note the duplicate definitions for left and right
 * function keys; Sun keyboards and a few other manufactures have such
 * function key groups on the left and/or right sides of the keyboard.
 * We've not found a keyboard with more than 35 function keys total.
 */
 
	oRTTI_VALUE(oKB_F1) 
	oRTTI_VALUE(oKB_F2) 
	oRTTI_VALUE(oKB_F3) 
	oRTTI_VALUE(oKB_F4) 
	oRTTI_VALUE(oKB_F5) 
	oRTTI_VALUE(oKB_F6) 
	oRTTI_VALUE(oKB_F7) 
	oRTTI_VALUE(oKB_F8) 
	oRTTI_VALUE(oKB_F9) 
	oRTTI_VALUE(oKB_F10) 
	oRTTI_VALUE(oKB_F11) 
	oRTTI_VALUE(oKB_L1) 
	oRTTI_VALUE(oKB_F12) 
	oRTTI_VALUE(oKB_L2) 
	oRTTI_VALUE(oKB_F13) 
	oRTTI_VALUE(oKB_L3) 
	oRTTI_VALUE(oKB_F14) 
	oRTTI_VALUE(oKB_L4) 
	oRTTI_VALUE(oKB_F15) 
	oRTTI_VALUE(oKB_L5) 
	oRTTI_VALUE(oKB_F16) 
	oRTTI_VALUE(oKB_L6) 
	oRTTI_VALUE(oKB_F17) 
	oRTTI_VALUE(oKB_L7) 
	oRTTI_VALUE(oKB_F18) 
	oRTTI_VALUE(oKB_L8) 
	oRTTI_VALUE(oKB_F19) 
	oRTTI_VALUE(oKB_L9) 
	oRTTI_VALUE(oKB_F20) 
	oRTTI_VALUE(oKB_L10) 
	oRTTI_VALUE(oKB_F21) 
	oRTTI_VALUE(oKB_R1) 
	oRTTI_VALUE(oKB_F22) 
	oRTTI_VALUE(oKB_R2) 
	oRTTI_VALUE(oKB_F23) 
	oRTTI_VALUE(oKB_R3) 
	oRTTI_VALUE(oKB_F24) 
	oRTTI_VALUE(oKB_R4) 
	oRTTI_VALUE(oKB_F25) 
	oRTTI_VALUE(oKB_R5) 
	oRTTI_VALUE(oKB_F26) 
	oRTTI_VALUE(oKB_R6) 
	oRTTI_VALUE(oKB_F27) 
	oRTTI_VALUE(oKB_R7) 
	oRTTI_VALUE(oKB_F28) 
	oRTTI_VALUE(oKB_R8) 
	oRTTI_VALUE(oKB_F29) 
	oRTTI_VALUE(oKB_R9) 
	oRTTI_VALUE(oKB_F30) 
	oRTTI_VALUE(oKB_R10) 
	oRTTI_VALUE(oKB_F31) 
	oRTTI_VALUE(oKB_R11) 
	oRTTI_VALUE(oKB_F32) 
	oRTTI_VALUE(oKB_R12) 
	oRTTI_VALUE(oKB_F33) 
	oRTTI_VALUE(oKB_R13) 
	oRTTI_VALUE(oKB_F34) 
	oRTTI_VALUE(oKB_R14) 
	oRTTI_VALUE(oKB_F35) 
	oRTTI_VALUE(oKB_R15) 
 
/* Modifiers*/
 
	oRTTI_VALUE(oKB_Shift_L) /* Left shift*/
	oRTTI_VALUE(oKB_Shift_R) /* Right shift*/
	oRTTI_VALUE(oKB_Control_L) /* Left control*/
	oRTTI_VALUE(oKB_Control_R) /* Right control*/
	oRTTI_VALUE(oKB_Caps_Lock) /* Caps lock*/
	oRTTI_VALUE(oKB_Shift_Lock) /* Shift lock*/
 
	oRTTI_VALUE(oKB_Meta_L) /* Left meta*/
	oRTTI_VALUE(oKB_Meta_R) /* Right meta*/
	oRTTI_VALUE(oKB_Alt_L) /* Left alt*/
	oRTTI_VALUE(oKB_Alt_R) /* Right alt*/
	oRTTI_VALUE(oKB_Super_L) /* Left super*/
	oRTTI_VALUE(oKB_Super_R) /* Right super*/
	oRTTI_VALUE(oKB_Hyper_L) /* Left hyper*/
	oRTTI_VALUE(oKB_Hyper_R) /* Right hyper*/

#endif/* XK_MISCELLANY*/
 
/*
 * ISO 9995 Function and Modifier Keys
 * Byte 3 = 0xFE
 */
 
#ifdef XK_XKB_KEYS
	oRTTI_VALUE(oKB_ISO_Lock) 
	oRTTI_VALUE(oKB_ISO_Level2_Latch) 
	oRTTI_VALUE(oKB_ISO_Level3_Shift) 
	oRTTI_VALUE(oKB_ISO_Level3_Latch) 
	oRTTI_VALUE(oKB_ISO_Level3_Lock) 
	oRTTI_VALUE(oKB_ISO_Group_Shift) /* Alias for mode_switch*/
	oRTTI_VALUE(oKB_ISO_Group_Latch) 
	oRTTI_VALUE(oKB_ISO_Group_Lock) 
	oRTTI_VALUE(oKB_ISO_Next_Group) 
	oRTTI_VALUE(oKB_ISO_Next_Group_Lock) 
	oRTTI_VALUE(oKB_ISO_Prev_Group) 
	oRTTI_VALUE(oKB_ISO_Prev_Group_Lock) 
	oRTTI_VALUE(oKB_ISO_First_Group) 
	oRTTI_VALUE(oKB_ISO_First_Group_Lock) 
	oRTTI_VALUE(oKB_ISO_Last_Group) 
	oRTTI_VALUE(oKB_ISO_Last_Group_Lock) 
 
	oRTTI_VALUE(oKB_ISO_Left_Tab) 
	oRTTI_VALUE(oKB_ISO_Move_Line_Up) 
	oRTTI_VALUE(oKB_ISO_Move_Line_Down) 
	oRTTI_VALUE(oKB_ISO_Partial_Line_Up) 
	oRTTI_VALUE(oKB_ISO_Partial_Line_Down) 
	oRTTI_VALUE(oKB_ISO_Partial_Space_Left) 
	oRTTI_VALUE(oKB_ISO_Partial_Space_Right) 
	oRTTI_VALUE(oKB_ISO_Set_Margin_Left) 
	oRTTI_VALUE(oKB_ISO_Set_Margin_Right) 
	oRTTI_VALUE(oKB_ISO_Release_Margin_Left) 
	oRTTI_VALUE(oKB_ISO_Release_Margin_Right) 
	oRTTI_VALUE(oKB_ISO_Release_Both_Margins) 
	oRTTI_VALUE(oKB_ISO_Fast_Cursor_Left) 
	oRTTI_VALUE(oKB_ISO_Fast_Cursor_Right) 
	oRTTI_VALUE(oKB_ISO_Fast_Cursor_Up) 
	oRTTI_VALUE(oKB_ISO_Fast_Cursor_Down) 
	oRTTI_VALUE(oKB_ISO_Continuous_Underline) 
	oRTTI_VALUE(oKB_ISO_Discontinuous_Underline) 
	oRTTI_VALUE(oKB_ISO_Emphasize) 
	oRTTI_VALUE(oKB_ISO_Center_Object) 
	oRTTI_VALUE(oKB_ISO_Enter) 
 
	oRTTI_VALUE(oKB_dead_grave) 
	oRTTI_VALUE(oKB_dead_acute) 
	oRTTI_VALUE(oKB_dead_circumflex) 
	oRTTI_VALUE(oKB_dead_tilde) 
	oRTTI_VALUE(oKB_dead_macron) 
	oRTTI_VALUE(oKB_dead_breve) 
	oRTTI_VALUE(oKB_dead_abovedot) 
	oRTTI_VALUE(oKB_dead_diaeresis) 
	oRTTI_VALUE(oKB_dead_abovering) 
	oRTTI_VALUE(oKB_dead_doubleacute) 
	oRTTI_VALUE(oKB_dead_caron) 
	oRTTI_VALUE(oKB_dead_cedilla) 
	oRTTI_VALUE(oKB_dead_ogonek) 
	oRTTI_VALUE(oKB_dead_iota) 
	oRTTI_VALUE(oKB_dead_voiced_sound) 
	oRTTI_VALUE(oKB_dead_semivoiced_sound) 
 
	oRTTI_VALUE(oKB_First_Virtual_Screen) 
	oRTTI_VALUE(oKB_Prev_Virtual_Screen) 
	oRTTI_VALUE(oKB_Next_Virtual_Screen) 
	oRTTI_VALUE(oKB_Last_Virtual_Screen) 
	oRTTI_VALUE(oKB_Terminate_Server) 
 
	oRTTI_VALUE(oKB_Pointer_Left) 
	oRTTI_VALUE(oKB_Pointer_Right) 
	oRTTI_VALUE(oKB_Pointer_Up) 
	oRTTI_VALUE(oKB_Pointer_Down) 
	oRTTI_VALUE(oKB_Pointer_UpLeft) 
	oRTTI_VALUE(oKB_Pointer_UpRight) 
	oRTTI_VALUE(oKB_Pointer_DownLeft) 
	oRTTI_VALUE(oKB_Pointer_DownRight)    /* oooii-tony: Add some more visually pleasing versions for pointer buttons */
	oRTTI_VALUE(oKB_Pointer_Button_Dflt) oRTTI_VALUE(oKB_Pointer_Button_Left) 
	oRTTI_VALUE(oKB_Pointer_Button1) oRTTI_VALUE(oKB_Pointer_Button_Right) 
	oRTTI_VALUE(oKB_Pointer_Button2) oRTTI_VALUE(oKB_Pointer_Button_Middle) 
	oRTTI_VALUE(oKB_Pointer_Button3) oRTTI_VALUE(oKB_Pointer_Button_Back) 
	oRTTI_VALUE(oKB_Pointer_Button4) oRTTI_VALUE(oKB_Pointer_Button_Forward) 
	oRTTI_VALUE(oKB_Pointer_Button5) 
	oRTTI_VALUE(oKB_Pointer_DblClick_Dflt) oRTTI_VALUE(oKB_Pointer_DblClick_Left) 
	oRTTI_VALUE(oKB_Pointer_DblClick1) oRTTI_VALUE(oKB_Pointer_DblClick_Right) 
	oRTTI_VALUE(oKB_Pointer_DblClick2) oRTTI_VALUE(oKB_Pointer_DblClick_Middle) 
	oRTTI_VALUE(oKB_Pointer_DblClick3) oRTTI_VALUE(oKB_Pointer_DblClick_Back) 
	oRTTI_VALUE(oKB_Pointer_DblClick4) oRTTI_VALUE(oKB_Pointer_DblClick_Forward) 
	oRTTI_VALUE(oKB_Pointer_DblClick5) 
	oRTTI_VALUE(oKB_Pointer_Drag_Dflt) oRTTI_VALUE(oKB_Pointer_Drag_Left) 
	oRTTI_VALUE(oKB_Pointer_Drag1) oRTTI_VALUE(oKB_Pointer_Drag_Right) 
	oRTTI_VALUE(oKB_Pointer_Drag2) oRTTI_VALUE(oKB_Pointer_Drag_Middle) 
	oRTTI_VALUE(oKB_Pointer_Drag3) oRTTI_VALUE(oKB_Pointer_Drag_Back) 
	oRTTI_VALUE(oKB_Pointer_Drag4) oRTTI_VALUE(oKB_Pointer_Drag_Forward) 
 
	oRTTI_VALUE(oKB_Pointer_EnableKeys) 
	oRTTI_VALUE(oKB_Pointer_Accelerate) 
	oRTTI_VALUE(oKB_Pointer_DfltBtnNext) 
	oRTTI_VALUE(oKB_Pointer_DfltBtnPrev) 

#endif
 
/*
 * 3270 Terminal Keys
 * Byte 3 = 0xFD
 */
 
#ifdef XK_3270
	oRTTI_VALUE(oKB_3270_Duplicate) 
	oRTTI_VALUE(oKB_3270_FieldMark) 
	oRTTI_VALUE(oKB_3270_Right2) 
	oRTTI_VALUE(oKB_3270_Left2) 
	oRTTI_VALUE(oKB_3270_BackTab) 
	oRTTI_VALUE(oKB_3270_EraseEOF) 
	oRTTI_VALUE(oKB_3270_EraseInput) 
	oRTTI_VALUE(oKB_3270_Reset) 
	oRTTI_VALUE(oKB_3270_Quit) 
	oRTTI_VALUE(oKB_3270_PA1) 
	oRTTI_VALUE(oKB_3270_PA2) 
	oRTTI_VALUE(oKB_3270_PA3) 
	oRTTI_VALUE(oKB_3270_Test) 
	oRTTI_VALUE(oKB_3270_Attn) 
	oRTTI_VALUE(oKB_3270_CursorBlink) 
	oRTTI_VALUE(oKB_3270_AltCursor) 
	oRTTI_VALUE(oKB_3270_KeyClick) 
	oRTTI_VALUE(oKB_3270_Jump) 
	oRTTI_VALUE(oKB_3270_Ident) 
	oRTTI_VALUE(oKB_3270_Rule) 
	oRTTI_VALUE(oKB_3270_Copy) 
	oRTTI_VALUE(oKB_3270_Play) 
	oRTTI_VALUE(oKB_3270_Setup) 
	oRTTI_VALUE(oKB_3270_Record) 
	oRTTI_VALUE(oKB_3270_ChangeScreen) 
	oRTTI_VALUE(oKB_3270_DeleteWord) 
	oRTTI_VALUE(oKB_3270_ExSelect) 
	oRTTI_VALUE(oKB_3270_CursorSelect) 
	oRTTI_VALUE(oKB_3270_PrintScreen) 
	oRTTI_VALUE(oKB_3270_Enter) 
#endif
 
/*
 * Latin 1
 * Byte 3 = 0
 */
#ifdef XK_LATIN1
	oRTTI_VALUE(oKB_space) 
	oRTTI_VALUE(oKB_exclam) 
	oRTTI_VALUE(oKB_quotedbl) 
	oRTTI_VALUE(oKB_numbersign) 
	oRTTI_VALUE(oKB_dollar) 
	oRTTI_VALUE(oKB_percent) 
	oRTTI_VALUE(oKB_ampersand) 
	oRTTI_VALUE(oKB_apostrophe) 
	oRTTI_VALUE(oKB_quoteright) /* deprecated*/
	oRTTI_VALUE(oKB_parenleft) 
	oRTTI_VALUE(oKB_parenright) 
	oRTTI_VALUE(oKB_asterisk) 
	oRTTI_VALUE(oKB_plus) 
	oRTTI_VALUE(oKB_comma) 
	oRTTI_VALUE(oKB_minus) 
	oRTTI_VALUE(oKB_period) 
	oRTTI_VALUE(oKB_slash) 
	oRTTI_VALUE(oKB_0) 
	oRTTI_VALUE(oKB_1) 
	oRTTI_VALUE(oKB_2) 
	oRTTI_VALUE(oKB_3) 
	oRTTI_VALUE(oKB_4) 
	oRTTI_VALUE(oKB_5) 
	oRTTI_VALUE(oKB_6) 
	oRTTI_VALUE(oKB_7) 
	oRTTI_VALUE(oKB_8) 
	oRTTI_VALUE(oKB_9) 
	oRTTI_VALUE(oKB_colon) 
	oRTTI_VALUE(oKB_semicolon) 
	oRTTI_VALUE(oKB_less) 
	oRTTI_VALUE(oKB_equal) 
	oRTTI_VALUE(oKB_greater) 
	oRTTI_VALUE(oKB_question) 
	oRTTI_VALUE(oKB_at) 
	oRTTI_VALUE(oKB_A) 
	oRTTI_VALUE(oKB_B) 
	oRTTI_VALUE(oKB_C) 
	oRTTI_VALUE(oKB_D) 
	oRTTI_VALUE(oKB_E) 
	oRTTI_VALUE(oKB_F) 
	oRTTI_VALUE(oKB_G) 
	oRTTI_VALUE(oKB_H) 
	oRTTI_VALUE(oKB_I) 
	oRTTI_VALUE(oKB_J) 
	oRTTI_VALUE(oKB_K) 
	oRTTI_VALUE(oKB_L) 
	oRTTI_VALUE(oKB_M) 
	oRTTI_VALUE(oKB_N) 
	oRTTI_VALUE(oKB_O) 
	oRTTI_VALUE(oKB_P) 
	oRTTI_VALUE(oKB_Q) 
	oRTTI_VALUE(oKB_R) 
	oRTTI_VALUE(oKB_S) 
	oRTTI_VALUE(oKB_T) 
	oRTTI_VALUE(oKB_U) 
	oRTTI_VALUE(oKB_V) 
	oRTTI_VALUE(oKB_W) 
	oRTTI_VALUE(oKB_X) 
	oRTTI_VALUE(oKB_Y) 
	oRTTI_VALUE(oKB_Z) 
	oRTTI_VALUE(oKB_bracketleft) 
	oRTTI_VALUE(oKB_backslash) 
	oRTTI_VALUE(oKB_bracketright) 
	oRTTI_VALUE(oKB_asciicircum) 
	oRTTI_VALUE(oKB_underscore) 
	oRTTI_VALUE(oKB_grave) 
	oRTTI_VALUE(oKB_quoteleft) /* deprecated*/
	oRTTI_VALUE(oKB_a) 
	oRTTI_VALUE(oKB_b) 
	oRTTI_VALUE(oKB_c) 
	oRTTI_VALUE(oKB_d) 
	oRTTI_VALUE(oKB_e) 
	oRTTI_VALUE(oKB_f) 
	oRTTI_VALUE(oKB_g) 
	oRTTI_VALUE(oKB_h) 
	oRTTI_VALUE(oKB_i) 
	oRTTI_VALUE(oKB_j) 
	oRTTI_VALUE(oKB_k) 
	oRTTI_VALUE(oKB_l) 
	oRTTI_VALUE(oKB_m) 
	oRTTI_VALUE(oKB_n) 
	oRTTI_VALUE(oKB_o) 
	oRTTI_VALUE(oKB_p) 
	oRTTI_VALUE(oKB_q) 
	oRTTI_VALUE(oKB_r) 
	oRTTI_VALUE(oKB_s) 
	oRTTI_VALUE(oKB_t) 
	oRTTI_VALUE(oKB_u) 
	oRTTI_VALUE(oKB_v) 
	oRTTI_VALUE(oKB_w) 
	oRTTI_VALUE(oKB_x) 
	oRTTI_VALUE(oKB_y) 
	oRTTI_VALUE(oKB_z) 
	oRTTI_VALUE(oKB_braceleft) 
	oRTTI_VALUE(oKB_bar) 
	oRTTI_VALUE(oKB_braceright) 
	oRTTI_VALUE(oKB_asciitilde) 
 
	oRTTI_VALUE(oKB_nobreakspace) 
	oRTTI_VALUE(oKB_exclamdown) 
	oRTTI_VALUE(oKB_cent) 
	oRTTI_VALUE(oKB_sterling) 
	oRTTI_VALUE(oKB_currency) 
	oRTTI_VALUE(oKB_yen) 
	oRTTI_VALUE(oKB_brokenbar) 
	oRTTI_VALUE(oKB_section) 
	oRTTI_VALUE(oKB_diaeresis) 
	oRTTI_VALUE(oKB_copyright) 
	oRTTI_VALUE(oKB_ordfeminine) 
	oRTTI_VALUE(oKB_guillemotleft) /* left angle quotation mark*/
	oRTTI_VALUE(oKB_notsign) 
	oRTTI_VALUE(oKB_hyphen) 
	oRTTI_VALUE(oKB_registered) 
	oRTTI_VALUE(oKB_macron) 
	oRTTI_VALUE(oKB_degree) 
	oRTTI_VALUE(oKB_plusminus) 
	oRTTI_VALUE(oKB_twosuperior) 
	oRTTI_VALUE(oKB_threesuperior) 
	oRTTI_VALUE(oKB_acute) 
	oRTTI_VALUE(oKB_mu) 
	oRTTI_VALUE(oKB_paragraph) 
	oRTTI_VALUE(oKB_periodcentered) 
	oRTTI_VALUE(oKB_cedilla) 
	oRTTI_VALUE(oKB_onesuperior) 
	oRTTI_VALUE(oKB_masculine) 
	oRTTI_VALUE(oKB_guillemotright) /* right angle quotation mark*/
	oRTTI_VALUE(oKB_onequarter) 
	oRTTI_VALUE(oKB_onehalf) 
	oRTTI_VALUE(oKB_threequarters) 
	oRTTI_VALUE(oKB_questiondown) 
	oRTTI_VALUE(oKB_Agrave) 
	oRTTI_VALUE(oKB_Aacute) 
	oRTTI_VALUE(oKB_Acircumflex) 
	oRTTI_VALUE(oKB_Atilde) 
	oRTTI_VALUE(oKB_Adiaeresis) 
	oRTTI_VALUE(oKB_Aring) 
	oRTTI_VALUE(oKB_AE) 
	oRTTI_VALUE(oKB_Ccedilla) 
	oRTTI_VALUE(oKB_Egrave) 
	oRTTI_VALUE(oKB_Eacute) 
	oRTTI_VALUE(oKB_Ecircumflex) 
	oRTTI_VALUE(oKB_Ediaeresis) 
	oRTTI_VALUE(oKB_Igrave) 
	oRTTI_VALUE(oKB_Iacute) 
	oRTTI_VALUE(oKB_Icircumflex) 
	oRTTI_VALUE(oKB_Idiaeresis) 
	oRTTI_VALUE(oKB_ETH) 
	oRTTI_VALUE(oKB_Eth) /* deprecated*/
	oRTTI_VALUE(oKB_Ntilde) 
	oRTTI_VALUE(oKB_Ograve) 
	oRTTI_VALUE(oKB_Oacute) 
	oRTTI_VALUE(oKB_Ocircumflex) 
	oRTTI_VALUE(oKB_Otilde) 
	oRTTI_VALUE(oKB_Odiaeresis) 
	oRTTI_VALUE(oKB_multiply) 
	oRTTI_VALUE(oKB_Ooblique) 
	oRTTI_VALUE(oKB_Ugrave) 
	oRTTI_VALUE(oKB_Uacute) 
	oRTTI_VALUE(oKB_Ucircumflex) 
	oRTTI_VALUE(oKB_Udiaeresis) 
	oRTTI_VALUE(oKB_Yacute) 
	oRTTI_VALUE(oKB_THORN) 
	oRTTI_VALUE(oKB_Thorn) /* deprecated*/
	oRTTI_VALUE(oKB_ssharp) 
	oRTTI_VALUE(oKB_agrave) 
	oRTTI_VALUE(oKB_aacute) 
	oRTTI_VALUE(oKB_acircumflex) 
	oRTTI_VALUE(oKB_atilde) 
	oRTTI_VALUE(oKB_adiaeresis) 
	oRTTI_VALUE(oKB_aring) 
	oRTTI_VALUE(oKB_ae) 
	oRTTI_VALUE(oKB_ccedilla) 
	oRTTI_VALUE(oKB_egrave) 
	oRTTI_VALUE(oKB_eacute) 
	oRTTI_VALUE(oKB_ecircumflex) 
	oRTTI_VALUE(oKB_ediaeresis) 
	oRTTI_VALUE(oKB_igrave) 
	oRTTI_VALUE(oKB_iacute) 
	oRTTI_VALUE(oKB_icircumflex) 
	oRTTI_VALUE(oKB_idiaeresis) 
	oRTTI_VALUE(oKB_eth) 
	oRTTI_VALUE(oKB_ntilde) 
	oRTTI_VALUE(oKB_ograve) 
	oRTTI_VALUE(oKB_oacute) 
	oRTTI_VALUE(oKB_ocircumflex) 
	oRTTI_VALUE(oKB_otilde) 
	oRTTI_VALUE(oKB_odiaeresis) 
	oRTTI_VALUE(oKB_division) 
	oRTTI_VALUE(oKB_oslash) 
	oRTTI_VALUE(oKB_ugrave) 
	oRTTI_VALUE(oKB_uacute) 
	oRTTI_VALUE(oKB_ucircumflex) 
	oRTTI_VALUE(oKB_udiaeresis) 
	oRTTI_VALUE(oKB_yacute) 
	oRTTI_VALUE(oKB_thorn) 
	oRTTI_VALUE(oKB_ydiaeresis) 
#endif/* XK_LATIN1*/
 
/*
 * Latin 2
 * Byte 3 = 1
 */
 
#ifdef XK_LATIN2
	oRTTI_VALUE(oKB_Aogonek) 
	oRTTI_VALUE(oKB_breve) 
	oRTTI_VALUE(oKB_Lstroke) 
	oRTTI_VALUE(oKB_Lcaron) 
	oRTTI_VALUE(oKB_Sacute) 
	oRTTI_VALUE(oKB_Scaron) 
	oRTTI_VALUE(oKB_Scedilla) 
	oRTTI_VALUE(oKB_Tcaron) 
	oRTTI_VALUE(oKB_Zacute) 
	oRTTI_VALUE(oKB_Zcaron) 
	oRTTI_VALUE(oKB_Zabovedot) 
	oRTTI_VALUE(oKB_aogonek) 
	oRTTI_VALUE(oKB_ogonek) 
	oRTTI_VALUE(oKB_lstroke) 
	oRTTI_VALUE(oKB_lcaron) 
	oRTTI_VALUE(oKB_sacute) 
	oRTTI_VALUE(oKB_caron) 
	oRTTI_VALUE(oKB_scaron) 
	oRTTI_VALUE(oKB_scedilla) 
	oRTTI_VALUE(oKB_tcaron) 
	oRTTI_VALUE(oKB_zacute) 
	oRTTI_VALUE(oKB_doubleacute) 
	oRTTI_VALUE(oKB_zcaron) 
	oRTTI_VALUE(oKB_zabovedot) 
	oRTTI_VALUE(oKB_Racute) 
	oRTTI_VALUE(oKB_Abreve) 
	oRTTI_VALUE(oKB_Lacute) 
	oRTTI_VALUE(oKB_Cacute) 
	oRTTI_VALUE(oKB_Ccaron) 
	oRTTI_VALUE(oKB_Eogonek) 
	oRTTI_VALUE(oKB_Ecaron) 
	oRTTI_VALUE(oKB_Dcaron) 
	oRTTI_VALUE(oKB_Dstroke) 
	oRTTI_VALUE(oKB_Nacute) 
	oRTTI_VALUE(oKB_Ncaron) 
	oRTTI_VALUE(oKB_Odoubleacute) 
	oRTTI_VALUE(oKB_Rcaron) 
	oRTTI_VALUE(oKB_Uring) 
	oRTTI_VALUE(oKB_Udoubleacute) 
	oRTTI_VALUE(oKB_Tcedilla) 
	oRTTI_VALUE(oKB_racute) 
	oRTTI_VALUE(oKB_abreve) 
	oRTTI_VALUE(oKB_lacute) 
	oRTTI_VALUE(oKB_cacute) 
	oRTTI_VALUE(oKB_ccaron) 
	oRTTI_VALUE(oKB_eogonek) 
	oRTTI_VALUE(oKB_ecaron) 
	oRTTI_VALUE(oKB_dcaron) 
	oRTTI_VALUE(oKB_dstroke) 
	oRTTI_VALUE(oKB_nacute) 
	oRTTI_VALUE(oKB_ncaron) 
	oRTTI_VALUE(oKB_odoubleacute) 
	oRTTI_VALUE(oKB_udoubleacute) 
	oRTTI_VALUE(oKB_rcaron) 
	oRTTI_VALUE(oKB_uring) 
	oRTTI_VALUE(oKB_tcedilla) 
	oRTTI_VALUE(oKB_abovedot) 
#endif/* XK_LATIN2*/
 
/*
 * Latin 3
 * Byte 3 = 2
 */
 
#ifdef XK_LATIN3
	oRTTI_VALUE(oKB_Hstroke) 
	oRTTI_VALUE(oKB_Hcircumflex) 
	oRTTI_VALUE(oKB_Iabovedot) 
	oRTTI_VALUE(oKB_Gbreve) 
	oRTTI_VALUE(oKB_Jcircumflex) 
	oRTTI_VALUE(oKB_hstroke) 
	oRTTI_VALUE(oKB_hcircumflex) 
	oRTTI_VALUE(oKB_idotless) 
	oRTTI_VALUE(oKB_gbreve) 
	oRTTI_VALUE(oKB_jcircumflex) 
	oRTTI_VALUE(oKB_Cabovedot) 
	oRTTI_VALUE(oKB_Ccircumflex) 
	oRTTI_VALUE(oKB_Gabovedot) 
	oRTTI_VALUE(oKB_Gcircumflex) 
	oRTTI_VALUE(oKB_Ubreve) 
	oRTTI_VALUE(oKB_Scircumflex) 
	oRTTI_VALUE(oKB_cabovedot) 
	oRTTI_VALUE(oKB_ccircumflex) 
	oRTTI_VALUE(oKB_gabovedot) 
	oRTTI_VALUE(oKB_gcircumflex) 
	oRTTI_VALUE(oKB_ubreve) 
	oRTTI_VALUE(oKB_scircumflex) 
#endif/* XK_LATIN3*/
 
 
/*
 * Latin 4
 * Byte 3 = 3
 */
 
#ifdef XK_LATIN4
	oRTTI_VALUE(oKB_kra) 
	oRTTI_VALUE(oKB_kappa) /* deprecated*/
	oRTTI_VALUE(oKB_Rcedilla) 
	oRTTI_VALUE(oKB_Itilde) 
	oRTTI_VALUE(oKB_Lcedilla) 
	oRTTI_VALUE(oKB_Emacron) 
	oRTTI_VALUE(oKB_Gcedilla) 
	oRTTI_VALUE(oKB_Tslash) 
	oRTTI_VALUE(oKB_rcedilla) 
	oRTTI_VALUE(oKB_itilde) 
	oRTTI_VALUE(oKB_lcedilla) 
	oRTTI_VALUE(oKB_emacron) 
	oRTTI_VALUE(oKB_gcedilla) 
	oRTTI_VALUE(oKB_tslash) 
	oRTTI_VALUE(oKB_ENG) 
	oRTTI_VALUE(oKB_eng) 
	oRTTI_VALUE(oKB_Amacron) 
	oRTTI_VALUE(oKB_Iogonek) 
	oRTTI_VALUE(oKB_Eabovedot) 
	oRTTI_VALUE(oKB_Imacron) 
	oRTTI_VALUE(oKB_Ncedilla) 
	oRTTI_VALUE(oKB_Omacron) 
	oRTTI_VALUE(oKB_Kcedilla) 
	oRTTI_VALUE(oKB_Uogonek) 
	oRTTI_VALUE(oKB_Utilde) 
	oRTTI_VALUE(oKB_Umacron) 
	oRTTI_VALUE(oKB_amacron) 
	oRTTI_VALUE(oKB_iogonek) 
	oRTTI_VALUE(oKB_eabovedot) 
	oRTTI_VALUE(oKB_imacron) 
	oRTTI_VALUE(oKB_ncedilla) 
	oRTTI_VALUE(oKB_omacron) 
	oRTTI_VALUE(oKB_kcedilla) 
	oRTTI_VALUE(oKB_uogonek) 
	oRTTI_VALUE(oKB_utilde) 
	oRTTI_VALUE(oKB_umacron) 
#endif/* XK_LATIN4*/
 
/*
 * Katakana
 * Byte 3 = 4
 */
 
#ifdef XK_KATAKANA
	oRTTI_VALUE(oKB_overline) 
	oRTTI_VALUE(oKB_kana_fullstop) 
	oRTTI_VALUE(oKB_kana_openingbracket) 
	oRTTI_VALUE(oKB_kana_closingbracket) 
	oRTTI_VALUE(oKB_kana_comma) 
	oRTTI_VALUE(oKB_kana_conjunctive) 
	oRTTI_VALUE(oKB_kana_middledot) /* deprecated*/
	oRTTI_VALUE(oKB_kana_WO) 
	oRTTI_VALUE(oKB_kana_a) 
	oRTTI_VALUE(oKB_kana_i) 
	oRTTI_VALUE(oKB_kana_u) 
	oRTTI_VALUE(oKB_kana_e) 
	oRTTI_VALUE(oKB_kana_o) 
	oRTTI_VALUE(oKB_kana_ya) 
	oRTTI_VALUE(oKB_kana_yu) 
	oRTTI_VALUE(oKB_kana_yo) 
	oRTTI_VALUE(oKB_kana_tsu) 
	oRTTI_VALUE(oKB_kana_tu) /* deprecated*/
	oRTTI_VALUE(oKB_prolongedsound) 
	oRTTI_VALUE(oKB_kana_A) 
	oRTTI_VALUE(oKB_kana_I) 
	oRTTI_VALUE(oKB_kana_U) 
	oRTTI_VALUE(oKB_kana_E) 
	oRTTI_VALUE(oKB_kana_O) 
	oRTTI_VALUE(oKB_kana_KA) 
	oRTTI_VALUE(oKB_kana_KI) 
	oRTTI_VALUE(oKB_kana_KU) 
	oRTTI_VALUE(oKB_kana_KE) 
	oRTTI_VALUE(oKB_kana_KO) 
	oRTTI_VALUE(oKB_kana_SA) 
	oRTTI_VALUE(oKB_kana_SHI) 
	oRTTI_VALUE(oKB_kana_SU) 
	oRTTI_VALUE(oKB_kana_SE) 
	oRTTI_VALUE(oKB_kana_SO) 
	oRTTI_VALUE(oKB_kana_TA) 
	oRTTI_VALUE(oKB_kana_CHI) 
	oRTTI_VALUE(oKB_kana_TI) /* deprecated*/
	oRTTI_VALUE(oKB_kana_TSU) 
	oRTTI_VALUE(oKB_kana_TU) /* deprecated*/
	oRTTI_VALUE(oKB_kana_TE) 
	oRTTI_VALUE(oKB_kana_TO) 
	oRTTI_VALUE(oKB_kana_NA) 
	oRTTI_VALUE(oKB_kana_NI) 
	oRTTI_VALUE(oKB_kana_NU) 
	oRTTI_VALUE(oKB_kana_NE) 
	oRTTI_VALUE(oKB_kana_NO) 
	oRTTI_VALUE(oKB_kana_HA) 
	oRTTI_VALUE(oKB_kana_HI) 
	oRTTI_VALUE(oKB_kana_FU) 
	oRTTI_VALUE(oKB_kana_HU) /* deprecated*/
	oRTTI_VALUE(oKB_kana_HE) 
	oRTTI_VALUE(oKB_kana_HO) 
	oRTTI_VALUE(oKB_kana_MA) 
	oRTTI_VALUE(oKB_kana_MI) 
	oRTTI_VALUE(oKB_kana_MU) 
	oRTTI_VALUE(oKB_kana_ME) 
	oRTTI_VALUE(oKB_kana_MO) 
	oRTTI_VALUE(oKB_kana_YA) 
	oRTTI_VALUE(oKB_kana_YU) 
	oRTTI_VALUE(oKB_kana_YO) 
	oRTTI_VALUE(oKB_kana_RA) 
	oRTTI_VALUE(oKB_kana_RI) 
	oRTTI_VALUE(oKB_kana_RU) 
	oRTTI_VALUE(oKB_kana_RE) 
	oRTTI_VALUE(oKB_kana_RO) 
	oRTTI_VALUE(oKB_kana_WA) 
	oRTTI_VALUE(oKB_kana_N) 
	oRTTI_VALUE(oKB_voicedsound) 
	oRTTI_VALUE(oKB_semivoicedsound) 
	oRTTI_VALUE(oKB_kana_switch) /* Alias for mode_switch*/
#endif/* XK_KATAKANA*/
 
/*
 * Arabic
 * Byte 3 = 5
 */
 
#ifdef XK_ARABIC
	oRTTI_VALUE(oKB_Arabic_comma) 
	oRTTI_VALUE(oKB_Arabic_semicolon) 
	oRTTI_VALUE(oKB_Arabic_question_mark) 
	oRTTI_VALUE(oKB_Arabic_hamza) 
	oRTTI_VALUE(oKB_Arabic_maddaonalef) 
	oRTTI_VALUE(oKB_Arabic_hamzaonalef) 
	oRTTI_VALUE(oKB_Arabic_hamzaonwaw) 
	oRTTI_VALUE(oKB_Arabic_hamzaunderalef) 
	oRTTI_VALUE(oKB_Arabic_hamzaonyeh) 
	oRTTI_VALUE(oKB_Arabic_alef) 
	oRTTI_VALUE(oKB_Arabic_beh) 
	oRTTI_VALUE(oKB_Arabic_tehmarbuta) 
	oRTTI_VALUE(oKB_Arabic_teh) 
	oRTTI_VALUE(oKB_Arabic_theh) 
	oRTTI_VALUE(oKB_Arabic_jeem) 
	oRTTI_VALUE(oKB_Arabic_hah) 
	oRTTI_VALUE(oKB_Arabic_khah) 
	oRTTI_VALUE(oKB_Arabic_dal) 
	oRTTI_VALUE(oKB_Arabic_thal) 
	oRTTI_VALUE(oKB_Arabic_ra) 
	oRTTI_VALUE(oKB_Arabic_zain) 
	oRTTI_VALUE(oKB_Arabic_seen) 
	oRTTI_VALUE(oKB_Arabic_sheen) 
	oRTTI_VALUE(oKB_Arabic_sad) 
	oRTTI_VALUE(oKB_Arabic_dad) 
	oRTTI_VALUE(oKB_Arabic_tah) 
	oRTTI_VALUE(oKB_Arabic_zah) 
	oRTTI_VALUE(oKB_Arabic_ain) 
	oRTTI_VALUE(oKB_Arabic_ghain) 
	oRTTI_VALUE(oKB_Arabic_tatweel) 
	oRTTI_VALUE(oKB_Arabic_feh) 
	oRTTI_VALUE(oKB_Arabic_qaf) 
	oRTTI_VALUE(oKB_Arabic_kaf) 
	oRTTI_VALUE(oKB_Arabic_lam) 
	oRTTI_VALUE(oKB_Arabic_meem) 
	oRTTI_VALUE(oKB_Arabic_noon) 
	oRTTI_VALUE(oKB_Arabic_ha) 
	oRTTI_VALUE(oKB_Arabic_heh) /* deprecated*/
	oRTTI_VALUE(oKB_Arabic_waw) 
	oRTTI_VALUE(oKB_Arabic_alefmaksura) 
	oRTTI_VALUE(oKB_Arabic_yeh) 
	oRTTI_VALUE(oKB_Arabic_fathatan) 
	oRTTI_VALUE(oKB_Arabic_dammatan) 
	oRTTI_VALUE(oKB_Arabic_kasratan) 
	oRTTI_VALUE(oKB_Arabic_fatha) 
	oRTTI_VALUE(oKB_Arabic_damma) 
	oRTTI_VALUE(oKB_Arabic_kasra) 
	oRTTI_VALUE(oKB_Arabic_shadda) 
	oRTTI_VALUE(oKB_Arabic_sukun) 
	oRTTI_VALUE(oKB_Arabic_switch) /* Alias for mode_switch*/
#endif/* XK_ARABIC*/
 
/*
 * Cyrillic
 * Byte 3 = 6
 */
#ifdef XK_CYRILLIC
	oRTTI_VALUE(oKB_Serbian_dje) 
	oRTTI_VALUE(oKB_Macedonia_gje) 
	oRTTI_VALUE(oKB_Cyrillic_io) 
	oRTTI_VALUE(oKB_Ukrainian_ie) 
	oRTTI_VALUE(oKB_Ukranian_je) /* deprecated*/
	oRTTI_VALUE(oKB_Macedonia_dse) 
	oRTTI_VALUE(oKB_Ukrainian_i) 
	oRTTI_VALUE(oKB_Ukranian_i) /* deprecated*/
	oRTTI_VALUE(oKB_Ukrainian_yi) 
	oRTTI_VALUE(oKB_Ukranian_yi) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_je) 
	oRTTI_VALUE(oKB_Serbian_je) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_lje) 
	oRTTI_VALUE(oKB_Serbian_lje) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_nje) 
	oRTTI_VALUE(oKB_Serbian_nje) /* deprecated*/
	oRTTI_VALUE(oKB_Serbian_tshe) 
	oRTTI_VALUE(oKB_Macedonia_kje) 
	oRTTI_VALUE(oKB_Byelorussian_shortu) 
	oRTTI_VALUE(oKB_Cyrillic_dzhe) 
	oRTTI_VALUE(oKB_Serbian_dze) /* deprecated*/
	oRTTI_VALUE(oKB_numerosign) 
	oRTTI_VALUE(oKB_Serbian_DJE) 
	oRTTI_VALUE(oKB_Macedonia_GJE) 
	oRTTI_VALUE(oKB_Cyrillic_IO) 
	oRTTI_VALUE(oKB_Ukrainian_IE) 
	oRTTI_VALUE(oKB_Ukranian_JE) /* deprecated*/
	oRTTI_VALUE(oKB_Macedonia_DSE) 
	oRTTI_VALUE(oKB_Ukrainian_I) 
	oRTTI_VALUE(oKB_Ukranian_I) /* deprecated*/
	oRTTI_VALUE(oKB_Ukrainian_YI) 
	oRTTI_VALUE(oKB_Ukranian_YI) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_JE) 
	oRTTI_VALUE(oKB_Serbian_JE) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_LJE) 
	oRTTI_VALUE(oKB_Serbian_LJE) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_NJE) 
	oRTTI_VALUE(oKB_Serbian_NJE) /* deprecated*/
	oRTTI_VALUE(oKB_Serbian_TSHE) 
	oRTTI_VALUE(oKB_Macedonia_KJE) 
	oRTTI_VALUE(oKB_Byelorussian_SHORTU) 
	oRTTI_VALUE(oKB_Cyrillic_DZHE) 
	oRTTI_VALUE(oKB_Serbian_DZE) /* deprecated*/
	oRTTI_VALUE(oKB_Cyrillic_yu) 
	oRTTI_VALUE(oKB_Cyrillic_a) 
	oRTTI_VALUE(oKB_Cyrillic_be) 
	oRTTI_VALUE(oKB_Cyrillic_tse) 
	oRTTI_VALUE(oKB_Cyrillic_de) 
	oRTTI_VALUE(oKB_Cyrillic_ie) 
	oRTTI_VALUE(oKB_Cyrillic_ef) 
	oRTTI_VALUE(oKB_Cyrillic_ghe) 
	oRTTI_VALUE(oKB_Cyrillic_ha) 
	oRTTI_VALUE(oKB_Cyrillic_i) 
	oRTTI_VALUE(oKB_Cyrillic_shorti) 
	oRTTI_VALUE(oKB_Cyrillic_ka) 
	oRTTI_VALUE(oKB_Cyrillic_el) 
	oRTTI_VALUE(oKB_Cyrillic_em) 
	oRTTI_VALUE(oKB_Cyrillic_en) 
	oRTTI_VALUE(oKB_Cyrillic_o) 
	oRTTI_VALUE(oKB_Cyrillic_pe) 
	oRTTI_VALUE(oKB_Cyrillic_ya) 
	oRTTI_VALUE(oKB_Cyrillic_er) 
	oRTTI_VALUE(oKB_Cyrillic_es) 
	oRTTI_VALUE(oKB_Cyrillic_te) 
	oRTTI_VALUE(oKB_Cyrillic_u) 
	oRTTI_VALUE(oKB_Cyrillic_zhe) 
	oRTTI_VALUE(oKB_Cyrillic_ve) 
	oRTTI_VALUE(oKB_Cyrillic_softsign) 
	oRTTI_VALUE(oKB_Cyrillic_yeru) 
	oRTTI_VALUE(oKB_Cyrillic_ze) 
	oRTTI_VALUE(oKB_Cyrillic_sha) 
	oRTTI_VALUE(oKB_Cyrillic_e) 
	oRTTI_VALUE(oKB_Cyrillic_shcha) 
	oRTTI_VALUE(oKB_Cyrillic_che) 
	oRTTI_VALUE(oKB_Cyrillic_hardsign) 
	oRTTI_VALUE(oKB_Cyrillic_YU) 
	oRTTI_VALUE(oKB_Cyrillic_A) 
	oRTTI_VALUE(oKB_Cyrillic_BE) 
	oRTTI_VALUE(oKB_Cyrillic_TSE) 
	oRTTI_VALUE(oKB_Cyrillic_DE) 
	oRTTI_VALUE(oKB_Cyrillic_IE) 
	oRTTI_VALUE(oKB_Cyrillic_EF) 
	oRTTI_VALUE(oKB_Cyrillic_GHE) 
	oRTTI_VALUE(oKB_Cyrillic_HA) 
	oRTTI_VALUE(oKB_Cyrillic_I) 
	oRTTI_VALUE(oKB_Cyrillic_SHORTI) 
	oRTTI_VALUE(oKB_Cyrillic_KA) 
	oRTTI_VALUE(oKB_Cyrillic_EL) 
	oRTTI_VALUE(oKB_Cyrillic_EM) 
	oRTTI_VALUE(oKB_Cyrillic_EN) 
	oRTTI_VALUE(oKB_Cyrillic_O) 
	oRTTI_VALUE(oKB_Cyrillic_PE) 
	oRTTI_VALUE(oKB_Cyrillic_YA) 
	oRTTI_VALUE(oKB_Cyrillic_ER) 
	oRTTI_VALUE(oKB_Cyrillic_ES) 
	oRTTI_VALUE(oKB_Cyrillic_TE) 
	oRTTI_VALUE(oKB_Cyrillic_U) 
	oRTTI_VALUE(oKB_Cyrillic_ZHE) 
	oRTTI_VALUE(oKB_Cyrillic_VE) 
	oRTTI_VALUE(oKB_Cyrillic_SOFTSIGN) 
	oRTTI_VALUE(oKB_Cyrillic_YERU) 
	oRTTI_VALUE(oKB_Cyrillic_ZE) 
	oRTTI_VALUE(oKB_Cyrillic_SHA) 
	oRTTI_VALUE(oKB_Cyrillic_E) 
	oRTTI_VALUE(oKB_Cyrillic_SHCHA) 
	oRTTI_VALUE(oKB_Cyrillic_CHE) 
	oRTTI_VALUE(oKB_Cyrillic_HARDSIGN) 
#endif/* XK_CYRILLIC*/
 
/*
 * Greek
 * Byte 3 = 7
 */
 
#ifdef XK_GREEK
	oRTTI_VALUE(oKB_Greek_ALPHAaccent) 
	oRTTI_VALUE(oKB_Greek_EPSILONaccent) 
	oRTTI_VALUE(oKB_Greek_ETAaccent) 
	oRTTI_VALUE(oKB_Greek_IOTAaccent) 
	oRTTI_VALUE(oKB_Greek_IOTAdiaeresis) 
	oRTTI_VALUE(oKB_Greek_OMICRONaccent) 
	oRTTI_VALUE(oKB_Greek_UPSILONaccent) 
	oRTTI_VALUE(oKB_Greek_UPSILONdieresis) 
	oRTTI_VALUE(oKB_Greek_OMEGAaccent) 
	oRTTI_VALUE(oKB_Greek_accentdieresis) 
	oRTTI_VALUE(oKB_Greek_horizbar) 
	oRTTI_VALUE(oKB_Greek_alphaaccent) 
	oRTTI_VALUE(oKB_Greek_epsilonaccent) 
	oRTTI_VALUE(oKB_Greek_etaaccent) 
	oRTTI_VALUE(oKB_Greek_iotaaccent) 
	oRTTI_VALUE(oKB_Greek_iotadieresis) 
	oRTTI_VALUE(oKB_Greek_iotaaccentdieresis) 
	oRTTI_VALUE(oKB_Greek_omicronaccent) 
	oRTTI_VALUE(oKB_Greek_upsilonaccent) 
	oRTTI_VALUE(oKB_Greek_upsilondieresis) 
	oRTTI_VALUE(oKB_Greek_upsilonaccentdieresis) 
	oRTTI_VALUE(oKB_Greek_omegaaccent) 
	oRTTI_VALUE(oKB_Greek_ALPHA) 
	oRTTI_VALUE(oKB_Greek_BETA) 
	oRTTI_VALUE(oKB_Greek_GAMMA) 
	oRTTI_VALUE(oKB_Greek_DELTA) 
	oRTTI_VALUE(oKB_Greek_EPSILON) 
	oRTTI_VALUE(oKB_Greek_ZETA) 
	oRTTI_VALUE(oKB_Greek_ETA) 
	oRTTI_VALUE(oKB_Greek_THETA) 
	oRTTI_VALUE(oKB_Greek_IOTA) 
	oRTTI_VALUE(oKB_Greek_KAPPA) 
	oRTTI_VALUE(oKB_Greek_LAMDA) 
	oRTTI_VALUE(oKB_Greek_LAMBDA) 
	oRTTI_VALUE(oKB_Greek_MU) 
	oRTTI_VALUE(oKB_Greek_NU) 
	oRTTI_VALUE(oKB_Greek_XI) 
	oRTTI_VALUE(oKB_Greek_OMICRON) 
	oRTTI_VALUE(oKB_Greek_PI) 
	oRTTI_VALUE(oKB_Greek_RHO) 
	oRTTI_VALUE(oKB_Greek_SIGMA) 
	oRTTI_VALUE(oKB_Greek_TAU) 
	oRTTI_VALUE(oKB_Greek_UPSILON) 
	oRTTI_VALUE(oKB_Greek_PHI) 
	oRTTI_VALUE(oKB_Greek_CHI) 
	oRTTI_VALUE(oKB_Greek_PSI) 
	oRTTI_VALUE(oKB_Greek_OMEGA) 
	oRTTI_VALUE(oKB_Greek_alpha) 
	oRTTI_VALUE(oKB_Greek_beta) 
	oRTTI_VALUE(oKB_Greek_gamma) 
	oRTTI_VALUE(oKB_Greek_delta) 
	oRTTI_VALUE(oKB_Greek_epsilon) 
	oRTTI_VALUE(oKB_Greek_zeta) 
	oRTTI_VALUE(oKB_Greek_eta) 
	oRTTI_VALUE(oKB_Greek_theta) 
	oRTTI_VALUE(oKB_Greek_iota) 
	oRTTI_VALUE(oKB_Greek_kappa) 
	oRTTI_VALUE(oKB_Greek_lamda) 
	oRTTI_VALUE(oKB_Greek_lambda) 
	oRTTI_VALUE(oKB_Greek_mu) 
	oRTTI_VALUE(oKB_Greek_nu) 
	oRTTI_VALUE(oKB_Greek_xi) 
	oRTTI_VALUE(oKB_Greek_omicron) 
	oRTTI_VALUE(oKB_Greek_pi) 
	oRTTI_VALUE(oKB_Greek_rho) 
	oRTTI_VALUE(oKB_Greek_sigma) 
	oRTTI_VALUE(oKB_Greek_finalsmallsigma) 
	oRTTI_VALUE(oKB_Greek_tau) 
	oRTTI_VALUE(oKB_Greek_upsilon) 
	oRTTI_VALUE(oKB_Greek_phi) 
	oRTTI_VALUE(oKB_Greek_chi) 
	oRTTI_VALUE(oKB_Greek_psi) 
	oRTTI_VALUE(oKB_Greek_omega) 
	oRTTI_VALUE(oKB_Greek_switch) /* Alias for mode_switch*/
#endif/* XK_GREEK*/
 
/*
 * Technical
 * Byte 3 = 8
 */
 
#ifdef XK_TECHNICAL
	oRTTI_VALUE(oKB_leftradical) 
	oRTTI_VALUE(oKB_topleftradical) 
	oRTTI_VALUE(oKB_horizconnector) 
	oRTTI_VALUE(oKB_topintegral) 
	oRTTI_VALUE(oKB_botintegral) 
	oRTTI_VALUE(oKB_vertconnector) 
	oRTTI_VALUE(oKB_topleftsqbracket) 
	oRTTI_VALUE(oKB_botleftsqbracket) 
	oRTTI_VALUE(oKB_toprightsqbracket) 
	oRTTI_VALUE(oKB_botrightsqbracket) 
	oRTTI_VALUE(oKB_topleftparens) 
	oRTTI_VALUE(oKB_botleftparens) 
	oRTTI_VALUE(oKB_toprightparens) 
	oRTTI_VALUE(oKB_botrightparens) 
	oRTTI_VALUE(oKB_leftmiddlecurlybrace) 
	oRTTI_VALUE(oKB_rightmiddlecurlybrace) 
	oRTTI_VALUE(oKB_topleftsummation) 
	oRTTI_VALUE(oKB_botleftsummation) 
	oRTTI_VALUE(oKB_topvertsummationconnector) 
	oRTTI_VALUE(oKB_botvertsummationconnector) 
	oRTTI_VALUE(oKB_toprightsummation) 
	oRTTI_VALUE(oKB_botrightsummation) 
	oRTTI_VALUE(oKB_rightmiddlesummation) 
	oRTTI_VALUE(oKB_lessthanequal) 
	oRTTI_VALUE(oKB_notequal) 
	oRTTI_VALUE(oKB_greaterthanequal) 
	oRTTI_VALUE(oKB_integral) 
	oRTTI_VALUE(oKB_therefore) 
	oRTTI_VALUE(oKB_variation) 
	oRTTI_VALUE(oKB_infinity) 
	oRTTI_VALUE(oKB_nabla) 
	oRTTI_VALUE(oKB_approximate) 
	oRTTI_VALUE(oKB_similarequal) 
	oRTTI_VALUE(oKB_ifonlyif) 
	oRTTI_VALUE(oKB_implies) 
	oRTTI_VALUE(oKB_identical) 
	oRTTI_VALUE(oKB_radical) 
	oRTTI_VALUE(oKB_includedin) 
	oRTTI_VALUE(oKB_includes) 
	oRTTI_VALUE(oKB_intersection) 
	oRTTI_VALUE(oKB_union) 
	oRTTI_VALUE(oKB_logicaland) 
	oRTTI_VALUE(oKB_logicalor) 
	oRTTI_VALUE(oKB_partialderivative) 
	oRTTI_VALUE(oKB_function) 
	oRTTI_VALUE(oKB_leftarrow) 
	oRTTI_VALUE(oKB_uparrow) 
	oRTTI_VALUE(oKB_rightarrow) 
	oRTTI_VALUE(oKB_downarrow) 
#endif/* XK_TECHNICAL*/
 
/*
 * Special
 * Byte 3 = 9
 */
 
#ifdef XK_SPECIAL
	oRTTI_VALUE(oKB_blank) 
	oRTTI_VALUE(oKB_soliddiamond) 
	oRTTI_VALUE(oKB_checkerboard) 
	oRTTI_VALUE(oKB_ht) 
	oRTTI_VALUE(oKB_ff) 
	oRTTI_VALUE(oKB_cr) 
	oRTTI_VALUE(oKB_lf) 
	oRTTI_VALUE(oKB_nl) 
	oRTTI_VALUE(oKB_vt) 
	oRTTI_VALUE(oKB_lowrightcorner) 
	oRTTI_VALUE(oKB_uprightcorner) 
	oRTTI_VALUE(oKB_upleftcorner) 
	oRTTI_VALUE(oKB_lowleftcorner) 
	oRTTI_VALUE(oKB_crossinglines) 
	oRTTI_VALUE(oKB_horizlinescan1) 
	oRTTI_VALUE(oKB_horizlinescan3) 
	oRTTI_VALUE(oKB_horizlinescan5) 
	oRTTI_VALUE(oKB_horizlinescan7) 
	oRTTI_VALUE(oKB_horizlinescan9) 
	oRTTI_VALUE(oKB_leftt) 
	oRTTI_VALUE(oKB_rightt) 
	oRTTI_VALUE(oKB_bott) 
	oRTTI_VALUE(oKB_topt) 
	oRTTI_VALUE(oKB_vertbar) 
#endif/* XK_SPECIAL*/
 
/*
 * Publishing
 * Byte 3 = a
 */
 
#ifdef XK_PUBLISHING
	oRTTI_VALUE(oKB_emspace) 
	oRTTI_VALUE(oKB_enspace) 
	oRTTI_VALUE(oKB_em3space) 
	oRTTI_VALUE(oKB_em4space) 
	oRTTI_VALUE(oKB_digitspace) 
	oRTTI_VALUE(oKB_punctspace) 
	oRTTI_VALUE(oKB_thinspace) 
	oRTTI_VALUE(oKB_hairspace) 
	oRTTI_VALUE(oKB_emdash) 
	oRTTI_VALUE(oKB_endash) 
	oRTTI_VALUE(oKB_signifblank) 
	oRTTI_VALUE(oKB_ellipsis) 
	oRTTI_VALUE(oKB_doubbaselinedot) 
	oRTTI_VALUE(oKB_onethird) 
	oRTTI_VALUE(oKB_twothirds) 
	oRTTI_VALUE(oKB_onefifth) 
	oRTTI_VALUE(oKB_twofifths) 
	oRTTI_VALUE(oKB_threefifths) 
	oRTTI_VALUE(oKB_fourfifths) 
	oRTTI_VALUE(oKB_onesixth) 
	oRTTI_VALUE(oKB_fivesixths) 
	oRTTI_VALUE(oKB_careof) 
	oRTTI_VALUE(oKB_figdash) 
	oRTTI_VALUE(oKB_leftanglebracket) 
	oRTTI_VALUE(oKB_decimalpoint) 
	oRTTI_VALUE(oKB_rightanglebracket) 
	oRTTI_VALUE(oKB_marker) 
	oRTTI_VALUE(oKB_oneeighth) 
	oRTTI_VALUE(oKB_threeeighths) 
	oRTTI_VALUE(oKB_fiveeighths) 
	oRTTI_VALUE(oKB_seveneighths) 
	oRTTI_VALUE(oKB_trademark) 
	oRTTI_VALUE(oKB_signaturemark) 
	oRTTI_VALUE(oKB_trademarkincircle) 
	oRTTI_VALUE(oKB_leftopentriangle) 
	oRTTI_VALUE(oKB_rightopentriangle) 
	oRTTI_VALUE(oKB_emopencircle) 
	oRTTI_VALUE(oKB_emopenrectangle) 
	oRTTI_VALUE(oKB_leftsinglequotemark) 
	oRTTI_VALUE(oKB_rightsinglequotemark) 
	oRTTI_VALUE(oKB_leftdoublequotemark) 
	oRTTI_VALUE(oKB_rightdoublequotemark) 
	oRTTI_VALUE(oKB_prescription) 
	oRTTI_VALUE(oKB_minutes) 
	oRTTI_VALUE(oKB_seconds) 
	oRTTI_VALUE(oKB_latincross) 
	oRTTI_VALUE(oKB_hexagram) 
	oRTTI_VALUE(oKB_filledrectbullet) 
	oRTTI_VALUE(oKB_filledlefttribullet) 
	oRTTI_VALUE(oKB_filledrighttribullet) 
	oRTTI_VALUE(oKB_emfilledcircle) 
	oRTTI_VALUE(oKB_emfilledrect) 
	oRTTI_VALUE(oKB_enopencircbullet) 
	oRTTI_VALUE(oKB_enopensquarebullet) 
	oRTTI_VALUE(oKB_openrectbullet) 
	oRTTI_VALUE(oKB_opentribulletup) 
	oRTTI_VALUE(oKB_opentribulletdown) 
	oRTTI_VALUE(oKB_openstar) 
	oRTTI_VALUE(oKB_enfilledcircbullet) 
	oRTTI_VALUE(oKB_enfilledsqbullet) 
	oRTTI_VALUE(oKB_filledtribulletup) 
	oRTTI_VALUE(oKB_filledtribulletdown) 
	oRTTI_VALUE(oKB_leftpointer) 
	oRTTI_VALUE(oKB_rightpointer) 
	oRTTI_VALUE(oKB_club) 
	oRTTI_VALUE(oKB_diamond) 
	oRTTI_VALUE(oKB_heart) 
	oRTTI_VALUE(oKB_maltesecross) 
	oRTTI_VALUE(oKB_dagger) 
	oRTTI_VALUE(oKB_doubledagger) 
	oRTTI_VALUE(oKB_checkmark) 
	oRTTI_VALUE(oKB_ballotcross) 
	oRTTI_VALUE(oKB_musicalsharp) 
	oRTTI_VALUE(oKB_musicalflat) 
	oRTTI_VALUE(oKB_malesymbol) 
	oRTTI_VALUE(oKB_femalesymbol) 
	oRTTI_VALUE(oKB_telephone) 
	oRTTI_VALUE(oKB_telephonerecorder) 
	oRTTI_VALUE(oKB_phonographcopyright) 
	oRTTI_VALUE(oKB_caret) 
	oRTTI_VALUE(oKB_singlelowquotemark) 
	oRTTI_VALUE(oKB_doublelowquotemark) 
	oRTTI_VALUE(oKB_cursor) 
#endif/* XK_PUBLISHING*/
 
/*
 * APL
 * Byte 3 = b
 */
 
#ifdef XK_APL
	oRTTI_VALUE(oKB_leftcaret) 
	oRTTI_VALUE(oKB_rightcaret) 
	oRTTI_VALUE(oKB_downcaret) 
	oRTTI_VALUE(oKB_upcaret) 
	oRTTI_VALUE(oKB_overbar) 
	oRTTI_VALUE(oKB_downtack) 
	oRTTI_VALUE(oKB_upshoe) 
	oRTTI_VALUE(oKB_downstile) 
	oRTTI_VALUE(oKB_underbar) 
	oRTTI_VALUE(oKB_jot) 
	oRTTI_VALUE(oKB_quad) 
	oRTTI_VALUE(oKB_uptack) 
	oRTTI_VALUE(oKB_circle) 
	oRTTI_VALUE(oKB_upstile) 
	oRTTI_VALUE(oKB_downshoe) 
	oRTTI_VALUE(oKB_rightshoe) 
	oRTTI_VALUE(oKB_leftshoe) 
	oRTTI_VALUE(oKB_lefttack) 
	oRTTI_VALUE(oKB_righttack) 
#endif/* XK_APL*/
 
/*
 * Hebrew
 * Byte 3 = c
 */
 
#ifdef XK_HEBREW
	oRTTI_VALUE(oKB_hebrew_doublelowline) 
	oRTTI_VALUE(oKB_hebrew_aleph) 
	oRTTI_VALUE(oKB_hebrew_bet) 
	oRTTI_VALUE(oKB_hebrew_beth) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_gimel) 
	oRTTI_VALUE(oKB_hebrew_gimmel) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_dalet) 
	oRTTI_VALUE(oKB_hebrew_daleth) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_he) 
	oRTTI_VALUE(oKB_hebrew_waw) 
	oRTTI_VALUE(oKB_hebrew_zain) 
	oRTTI_VALUE(oKB_hebrew_zayin) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_chet) 
	oRTTI_VALUE(oKB_hebrew_het) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_tet) 
	oRTTI_VALUE(oKB_hebrew_teth) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_yod) 
	oRTTI_VALUE(oKB_hebrew_finalkaph) 
	oRTTI_VALUE(oKB_hebrew_kaph) 
	oRTTI_VALUE(oKB_hebrew_lamed) 
	oRTTI_VALUE(oKB_hebrew_finalmem) 
	oRTTI_VALUE(oKB_hebrew_mem) 
	oRTTI_VALUE(oKB_hebrew_finalnun) 
	oRTTI_VALUE(oKB_hebrew_nun) 
	oRTTI_VALUE(oKB_hebrew_samech) 
	oRTTI_VALUE(oKB_hebrew_samekh) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_ayin) 
	oRTTI_VALUE(oKB_hebrew_finalpe) 
	oRTTI_VALUE(oKB_hebrew_pe) 
	oRTTI_VALUE(oKB_hebrew_finalzade) 
	oRTTI_VALUE(oKB_hebrew_finalzadi) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_zade) 
	oRTTI_VALUE(oKB_hebrew_zadi) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_qoph) 
	oRTTI_VALUE(oKB_hebrew_kuf) /* deprecated*/
	oRTTI_VALUE(oKB_hebrew_resh) 
	oRTTI_VALUE(oKB_hebrew_shin) 
	oRTTI_VALUE(oKB_hebrew_taw) 
	oRTTI_VALUE(oKB_hebrew_taf) /* deprecated*/
	oRTTI_VALUE(oKB_Hebrew_switch) /* Alias for mode_switch*/
#endif/* XK_HEBREW*/
 
/*
 * Thai
 * Byte 3 = d
 */
 
#ifdef XK_THAI
	oRTTI_VALUE(oKB_Thai_kokai) 
	oRTTI_VALUE(oKB_Thai_khokhai) 
	oRTTI_VALUE(oKB_Thai_khokhuat) 
	oRTTI_VALUE(oKB_Thai_khokhwai) 
	oRTTI_VALUE(oKB_Thai_khokhon) 
	oRTTI_VALUE(oKB_Thai_khorakhang) 
	oRTTI_VALUE(oKB_Thai_ngongu) 
	oRTTI_VALUE(oKB_Thai_chochan) 
	oRTTI_VALUE(oKB_Thai_choching) 
	oRTTI_VALUE(oKB_Thai_chochang) 
	oRTTI_VALUE(oKB_Thai_soso) 
	oRTTI_VALUE(oKB_Thai_chochoe) 
	oRTTI_VALUE(oKB_Thai_yoying) 
	oRTTI_VALUE(oKB_Thai_dochada) 
	oRTTI_VALUE(oKB_Thai_topatak) 
	oRTTI_VALUE(oKB_Thai_thothan) 
	oRTTI_VALUE(oKB_Thai_thonangmontho) 
	oRTTI_VALUE(oKB_Thai_thophuthao) 
	oRTTI_VALUE(oKB_Thai_nonen) 
	oRTTI_VALUE(oKB_Thai_dodek) 
	oRTTI_VALUE(oKB_Thai_totao) 
	oRTTI_VALUE(oKB_Thai_thothung) 
	oRTTI_VALUE(oKB_Thai_thothahan) 
	oRTTI_VALUE(oKB_Thai_thothong) 
	oRTTI_VALUE(oKB_Thai_nonu) 
	oRTTI_VALUE(oKB_Thai_bobaimai) 
	oRTTI_VALUE(oKB_Thai_popla) 
	oRTTI_VALUE(oKB_Thai_phophung) 
	oRTTI_VALUE(oKB_Thai_fofa) 
	oRTTI_VALUE(oKB_Thai_phophan) 
	oRTTI_VALUE(oKB_Thai_fofan) 
	oRTTI_VALUE(oKB_Thai_phosamphao) 
	oRTTI_VALUE(oKB_Thai_moma) 
	oRTTI_VALUE(oKB_Thai_yoyak) 
	oRTTI_VALUE(oKB_Thai_rorua) 
	oRTTI_VALUE(oKB_Thai_ru) 
	oRTTI_VALUE(oKB_Thai_loling) 
	oRTTI_VALUE(oKB_Thai_lu) 
	oRTTI_VALUE(oKB_Thai_wowaen) 
	oRTTI_VALUE(oKB_Thai_sosala) 
	oRTTI_VALUE(oKB_Thai_sorusi) 
	oRTTI_VALUE(oKB_Thai_sosua) 
	oRTTI_VALUE(oKB_Thai_hohip) 
	oRTTI_VALUE(oKB_Thai_lochula) 
	oRTTI_VALUE(oKB_Thai_oang) 
	oRTTI_VALUE(oKB_Thai_honokhuk) 
	oRTTI_VALUE(oKB_Thai_paiyannoi) 
	oRTTI_VALUE(oKB_Thai_saraa) 
	oRTTI_VALUE(oKB_Thai_maihanakat) 
	oRTTI_VALUE(oKB_Thai_saraaa) 
	oRTTI_VALUE(oKB_Thai_saraam) 
	oRTTI_VALUE(oKB_Thai_sarai) 
	oRTTI_VALUE(oKB_Thai_saraii) 
	oRTTI_VALUE(oKB_Thai_saraue) 
	oRTTI_VALUE(oKB_Thai_sarauee) 
	oRTTI_VALUE(oKB_Thai_sarau) 
	oRTTI_VALUE(oKB_Thai_sarauu) 
	oRTTI_VALUE(oKB_Thai_phinthu) 
	oRTTI_VALUE(oKB_Thai_maihanakat_maitho) 
	oRTTI_VALUE(oKB_Thai_baht) 
	oRTTI_VALUE(oKB_Thai_sarae) 
	oRTTI_VALUE(oKB_Thai_saraae) 
	oRTTI_VALUE(oKB_Thai_sarao) 
	oRTTI_VALUE(oKB_Thai_saraaimaimuan) 
	oRTTI_VALUE(oKB_Thai_saraaimaimalai) 
	oRTTI_VALUE(oKB_Thai_lakkhangyao) 
	oRTTI_VALUE(oKB_Thai_maiyamok) 
	oRTTI_VALUE(oKB_Thai_maitaikhu) 
	oRTTI_VALUE(oKB_Thai_maiek) 
	oRTTI_VALUE(oKB_Thai_maitho) 
	oRTTI_VALUE(oKB_Thai_maitri) 
	oRTTI_VALUE(oKB_Thai_maichattawa) 
	oRTTI_VALUE(oKB_Thai_thanthakhat) 
	oRTTI_VALUE(oKB_Thai_nikhahit) 
	oRTTI_VALUE(oKB_Thai_leksun) 
	oRTTI_VALUE(oKB_Thai_leknung) 
	oRTTI_VALUE(oKB_Thai_leksong) 
	oRTTI_VALUE(oKB_Thai_leksam) 
	oRTTI_VALUE(oKB_Thai_leksi) 
	oRTTI_VALUE(oKB_Thai_lekha) 
	oRTTI_VALUE(oKB_Thai_lekhok) 
	oRTTI_VALUE(oKB_Thai_lekchet) 
	oRTTI_VALUE(oKB_Thai_lekpaet) 
	oRTTI_VALUE(oKB_Thai_lekkao) 
#endif/* XK_THAI*/
 
/*
 * Korean
 * Byte 3 = e
 */

#ifdef XK_KOREAN
 
	oRTTI_VALUE(oKB_Hangul) /* Hangul start/stop(toggle)*/
	oRTTI_VALUE(oKB_Hangul_Start) /* Hangul start*/
	oRTTI_VALUE(oKB_Hangul_End) /* Hangul end, English start*/
	oRTTI_VALUE(oKB_Hangul_Hanja) /* Start Hangul->Hanja Conversion*/
	oRTTI_VALUE(oKB_Hangul_Jamo) /* Hangul Jamo mode*/
	oRTTI_VALUE(oKB_Hangul_Romaja) /* Hangul Romaja mode*/
	oRTTI_VALUE(oKB_Hangul_Codeinput) /* Hangul code input mode*/
	oRTTI_VALUE(oKB_Hangul_Jeonja) /* Jeonja mode*/
	oRTTI_VALUE(oKB_Hangul_Banja) /* Banja mode*/
	oRTTI_VALUE(oKB_Hangul_PreHanja) /* Pre Hanja conversion*/
	oRTTI_VALUE(oKB_Hangul_PostHanja) /* Post Hanja conversion*/
	oRTTI_VALUE(oKB_Hangul_SingleCandidate) /* Single candidate*/
	oRTTI_VALUE(oKB_Hangul_MultipleCandidate) /* Multiple candidate*/
	oRTTI_VALUE(oKB_Hangul_PreviousCandidate) /* Previous candidate*/
	oRTTI_VALUE(oKB_Hangul_Special) /* Special symbols*/
	oRTTI_VALUE(oKB_Hangul_switch) /* Alias for mode_switch*/

	/* Hangul Consonant Characters*/
	oRTTI_VALUE(oKB_Hangul_Kiyeog) 
	oRTTI_VALUE(oKB_Hangul_SsangKiyeog) 
	oRTTI_VALUE(oKB_Hangul_KiyeogSios) 
	oRTTI_VALUE(oKB_Hangul_Nieun) 
	oRTTI_VALUE(oKB_Hangul_NieunJieuj) 
	oRTTI_VALUE(oKB_Hangul_NieunHieuh) 
	oRTTI_VALUE(oKB_Hangul_Dikeud) 
	oRTTI_VALUE(oKB_Hangul_SsangDikeud) 
	oRTTI_VALUE(oKB_Hangul_Rieul) 
	oRTTI_VALUE(oKB_Hangul_RieulKiyeog) 
	oRTTI_VALUE(oKB_Hangul_RieulMieum) 
	oRTTI_VALUE(oKB_Hangul_RieulPieub) 
	oRTTI_VALUE(oKB_Hangul_RieulSios) 
	oRTTI_VALUE(oKB_Hangul_RieulTieut) 
	oRTTI_VALUE(oKB_Hangul_RieulPhieuf) 
	oRTTI_VALUE(oKB_Hangul_RieulHieuh) 
	oRTTI_VALUE(oKB_Hangul_Mieum) 
	oRTTI_VALUE(oKB_Hangul_Pieub) 
	oRTTI_VALUE(oKB_Hangul_SsangPieub) 
	oRTTI_VALUE(oKB_Hangul_PieubSios) 
	oRTTI_VALUE(oKB_Hangul_Sios) 
	oRTTI_VALUE(oKB_Hangul_SsangSios) 
	oRTTI_VALUE(oKB_Hangul_Ieung) 
	oRTTI_VALUE(oKB_Hangul_Jieuj) 
	oRTTI_VALUE(oKB_Hangul_SsangJieuj) 
	oRTTI_VALUE(oKB_Hangul_Cieuc) 
	oRTTI_VALUE(oKB_Hangul_Khieuq) 
	oRTTI_VALUE(oKB_Hangul_Tieut) 
	oRTTI_VALUE(oKB_Hangul_Phieuf) 
	oRTTI_VALUE(oKB_Hangul_Hieuh) 
 
	/* Hangul Vowel Characters*/
	oRTTI_VALUE(oKB_Hangul_A) 
	oRTTI_VALUE(oKB_Hangul_AE) 
	oRTTI_VALUE(oKB_Hangul_YA) 
	oRTTI_VALUE(oKB_Hangul_YAE) 
	oRTTI_VALUE(oKB_Hangul_EO) 
	oRTTI_VALUE(oKB_Hangul_E) 
	oRTTI_VALUE(oKB_Hangul_YEO) 
	oRTTI_VALUE(oKB_Hangul_YE) 
	oRTTI_VALUE(oKB_Hangul_O) 
	oRTTI_VALUE(oKB_Hangul_WA) 
	oRTTI_VALUE(oKB_Hangul_WAE) 
	oRTTI_VALUE(oKB_Hangul_OE) 
	oRTTI_VALUE(oKB_Hangul_YO) 
	oRTTI_VALUE(oKB_Hangul_U) 
	oRTTI_VALUE(oKB_Hangul_WEO) 
	oRTTI_VALUE(oKB_Hangul_WE) 
	oRTTI_VALUE(oKB_Hangul_WI) 
	oRTTI_VALUE(oKB_Hangul_YU) 
	oRTTI_VALUE(oKB_Hangul_EU) 
	oRTTI_VALUE(oKB_Hangul_YI) 
	oRTTI_VALUE(oKB_Hangul_I) 
 
	/* Hangul syllable-final (JongSeong) Characters*/
	oRTTI_VALUE(oKB_Hangul_J_Kiyeog) 
	oRTTI_VALUE(oKB_Hangul_J_SsangKiyeog) 
	oRTTI_VALUE(oKB_Hangul_J_KiyeogSios) 
	oRTTI_VALUE(oKB_Hangul_J_Nieun) 
	oRTTI_VALUE(oKB_Hangul_J_NieunJieuj) 
	oRTTI_VALUE(oKB_Hangul_J_NieunHieuh) 
	oRTTI_VALUE(oKB_Hangul_J_Dikeud) 
	oRTTI_VALUE(oKB_Hangul_J_Rieul) 
	oRTTI_VALUE(oKB_Hangul_J_RieulKiyeog) 
	oRTTI_VALUE(oKB_Hangul_J_RieulMieum) 
	oRTTI_VALUE(oKB_Hangul_J_RieulPieub) 
	oRTTI_VALUE(oKB_Hangul_J_RieulSios) 
	oRTTI_VALUE(oKB_Hangul_J_RieulTieut) 
	oRTTI_VALUE(oKB_Hangul_J_RieulPhieuf) 
	oRTTI_VALUE(oKB_Hangul_J_RieulHieuh) 
	oRTTI_VALUE(oKB_Hangul_J_Mieum) 
	oRTTI_VALUE(oKB_Hangul_J_Pieub) 
	oRTTI_VALUE(oKB_Hangul_J_PieubSios) 
	oRTTI_VALUE(oKB_Hangul_J_Sios) 
	oRTTI_VALUE(oKB_Hangul_J_SsangSios) 
	oRTTI_VALUE(oKB_Hangul_J_Ieung) 
	oRTTI_VALUE(oKB_Hangul_J_Jieuj) 
	oRTTI_VALUE(oKB_Hangul_J_Cieuc) 
	oRTTI_VALUE(oKB_Hangul_J_Khieuq) 
	oRTTI_VALUE(oKB_Hangul_J_Tieut) 
	oRTTI_VALUE(oKB_Hangul_J_Phieuf) 
	oRTTI_VALUE(oKB_Hangul_J_Hieuh) 
 
	/* Ancient Hangul Consonant Characters*/
	oRTTI_VALUE(oKB_Hangul_RieulYeorinHieuh) 
	oRTTI_VALUE(oKB_Hangul_SunkyeongeumMieum) 
	oRTTI_VALUE(oKB_Hangul_SunkyeongeumPieub) 
	oRTTI_VALUE(oKB_Hangul_PanSios) 
	oRTTI_VALUE(oKB_Hangul_KkogjiDalrinIeung) 
	oRTTI_VALUE(oKB_Hangul_SunkyeongeumPhieuf) 
	oRTTI_VALUE(oKB_Hangul_YeorinHieuh) 
 
	/* Ancient Hangul Vowel Characters*/
	oRTTI_VALUE(oKB_Hangul_AraeA) 
	oRTTI_VALUE(oKB_Hangul_AraeAE) 
 
	/* Ancient Hangul syllable-final (JongSeong) Characters*/
	oRTTI_VALUE(oKB_Hangul_J_PanSios) 
	oRTTI_VALUE(oKB_Hangul_J_KkogjiDalrinIeung) 
	oRTTI_VALUE(oKB_Hangul_J_YeorinHieuh) 
 
	/* Korean currency symbol*/
	oRTTI_VALUE(oKB_Korean_Won) 
 
#endif/* XK_KOREAN*/

	oRTTI_ENUM_END_VALUES(oKEYBOARD_KEY)
oRTTI_ENUM_END_DESCRIPTION_CASE_SENSITIVE(oKEYBOARD_KEY)
