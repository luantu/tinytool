;================ TransActive =====================
; Author: Programus
;
; Set Current active window Transparent. 
; HotKey: 
; 	Ctrl-Alt-0: Switch transparent
; 	Ctrl-Alt-=: less transparent
; 	Ctrl-Alt--: more transparent

Goto, TransActiveEnd

^!=::
	trans += 16
	Gosub, SetTrans
	return

^!-::
	trans -= 16
	Gosub, SetTrans
	return
	
^!0::
	WinGet, transValue, Transparent, A
	if (transValue = "") {
		Gosub, SetTrans
	} else {
		WinSet, Transparent, OFF, A
	}
	return
	
SetTrans:
	if (trans = "") {
		; Default transparent value
		trans := 209
	}
	if (trans <= 0) {
		trans = 1
	}
	if (trans >= 255) {
		trans := 255
		WinSet, Transparent, OFF, A
	} else {
		WinSet, Transparent, %trans%, A
	}
	return

TransActiveEnd:
