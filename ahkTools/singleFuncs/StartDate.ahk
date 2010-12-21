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

Control, Hide, , Button1, ahk_class Shell_TrayWnd

Gui 5: +ToolWindow -Caption           
Gui 5: +Lastfound                     
GUI_ID := WinExist()                
WinGet, TaskBar_ID, ID, ahk_class Shell_TrayWnd
DllCall("SetParent", "uint", GUI_ID, "uint", Taskbar_ID)

Gui 5: Margin, 0, 0
Gui 5: Font, S14 Bold , Arial
Gui 5: Add,Button, h30 gStartM vTime, %A_MM%-%A_DD%
Gui 5: Show,x0 y0 AutoSize, Start Button Clock


SetTimer, SetStartButtonDate, 10000
GoSub, SetStartButtonDate

if (notSingleRun) {
	funcName = RestoreStart
	IfNotInString, OnExitSubs, %funcName%
	{
		OnExitSubs = %OnExitSubs%`n%funcName%
	}
} else {
	OnExit, Exitt							; Restore the default start button
}

GoSub, FreeMemory

Goto, StartDateEnd

SetStartButtonDate:
	ControlGetText, currDate, Button1, ahk_class Shell_TrayWnd
	if currDate <> %A_MM%-%A_DD% 
	{
		GuiControl, , StartM, %A_MM%-%A_DD%
;		ControlMove, Button1,,,58,, ahk_class Shell_TrayWnd
;		ControlSetText, Button1, %A_MM%-%A_DD%, ahk_class Shell_TrayWnd
	}
	currDate =
	GoSub, FreeMemory
	return

StartM:
	Send {LWin}
	GoSub, FreeMemory
return

RestoreStart:
	Gui 5: Destroy
	Control, Show, ,Button1, ahk_class Shell_TrayWnd
Return

Exitt:
	GoSub, RestoreStart
	ExitApp
return

FreeMemory:
	hProcess := DllCall("GetCurrentProcess")
	DllCall("SetProcessWorkingSetSize", Int, hProcess, Int, -1, Int, -1)
	return


StartDateEnd:
