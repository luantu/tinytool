;================ StartDate =====================
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
