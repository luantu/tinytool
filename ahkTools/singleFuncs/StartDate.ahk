;================ StartDate =====================
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
; Set Current Date as Start Button Text
; HotKey: 
; 	<None>
#Persistent
SetTimer, SetStartButtonDate, 10000
GoSub, SetStartButtonDate
Goto, StartDateEnd

SetStartButtonDate:
	ControlGetText, currDate, Button1, ahk_class Shell_TrayWnd
	if currDate <> %A_MM%-%A_DD% 
	{
		ControlMove, Button1,,,58,, ahk_class Shell_TrayWnd
		ControlSetText, Button1, %A_MM%-%A_DD%, ahk_class Shell_TrayWnd
	}
	currDate =
	return

StartDateEnd:
