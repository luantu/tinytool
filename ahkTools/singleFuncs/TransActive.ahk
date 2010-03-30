;================ TransActive =====================
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  Copyright 2010 Programus
;
;  Licensed under the Apache License, Version 2.0 (the "License");
;  you may not use this file except in compliance with the License.
;  You may obtain a copy of the License at
;
;      http://www.apache.org/licenses/LICENSE-2.0
;
;  Unless required by applicable law or agreed to in writing, software
;  distributed under the License is distributed on an "AS IS" BASIS,
;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;  See the License for the specific language governing permissions and
;  limitations under the License.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
