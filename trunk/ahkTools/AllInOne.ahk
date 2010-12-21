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
; A global setting before all scripts
#NoEnv
ListLines Off
; #MaxMem 1
#ErrorStdOut

notSingleRun := true
OnExit, TotalExitSub
Goto, StartMain

TotalExitSub:
	Loop, Parse, OnExitSubs, `n, `r
	{
		If A_LoopField <>
		{
			GoSub, %A_LoopField%
		}
	}
	ExitApp
	return

StartMain:

; ==================== ShowClipboard ================
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
; Enhanced clipboard with only X/C/D/V keys with Ctrl and Windows key
; HotKey:
; 	Ctrl: Show clipboard content tooltip (disappear when release ctrl)
;   Ctrl-Win-V: Paste plain text only ([Plain Paste] checked)
;   Ctrl-Win-C: Move to the next clipboard ([Multi-Clipboard] checked)
;   Ctrl-Win-X: Move to the previous clipboard ([Multi-Clipboard] checked)
;   Ctrl-Win-D: Delete current clipboard content ([Multi-Clipboard] checked)
;   Ctrl-Win:   Display current amount of clipboards ([Multi-Clipboard] checked)
;   Win-X:      Copy the title of the window currently under mouse pointer
;   Win-C:      Copy the text of the control currently under mouse pointer (the text is retrieved by WM_GETTEXT message, some customized control may not support)
;   Win-V:      List all clipboards to opreate:
;                      Left Key Double-Click   - select current clipboard
;                      Right Key Double-Click  - remove selected clipboards

Define:
	WM_GETTEXT := 0x0D
	WM_GETTEXTLENGTH := 0x0E
	WM_ACTIVATE := 0x06
	UTF8 := 65001
	MAX_ROW_NUM := 50
	MIN_ROW_NUM := 10
	ccMenuName = &Clear Clipboard
	ppMenuName = P&lain Paste (usage:Ctrl-Win-V)
	mcMenuName = &Multi-Clipboard (usage:Ctrl-Win-C/D/X)
	raMenuName = Remove && when copy control text
	lsMenuName = L&ist All (usage:Win-V)
	ttMenuName = Enable TrayTip
	ListWinTitle = == Clip List ==  (DoubleClick LButton: Change, DoubleClick RButton: Remove)

Default:
	notPlainPaste := false
	notMultiClipboard := false
	notRemoveAmp := false
	notListAll := false
	notShowTrayTip := false

Init:
	if (not ccMenuAdded) {
		Menu, TRAY, NoStandard
		Menu, TRAY, add, %ccMenuName%, ClearClipboard
		Menu, TRAY, add
		Menu, TRAY, Standard
		ccMenuAdded := true
	}
	if (not raMenuAdded) {
		Menu, TRAY, NoStandard
		Menu, TRAY, add, %raMenuName%, ToggleRemoveAmp
		Menu, TRAY, Standard
		raMenuAdded := true
	}
	if (not mcMenuAdded) {
		Menu, TRAY, NoStandard
		Menu, TRAY, add, %mcMenuName%, ToggleMultiClipboard
		Menu, TRAY, Standard
		mcMenuAdded := true
	}
	if (not ppMenuAdded) {
		Menu, TRAY, NoStandard
		Menu, TRAY, add, %ppMenuName%, TogglePlainPaste
		Menu, TRAY, add
		Menu, TRAY, Standard
		ppMenuAdded := true
	}
	if (not lsMenuAdded) {
		Menu, TRAY, NoStandard
		Menu, TRAY, add, %lsMenuName%, ToggleListAll
		Menu, TRAY, add
		Menu, TRAY, Standard
		lsMenuAdded := true
	}
	if (not ttMenuAdded) {
		Menu, TRAY, NoStandard
		Menu, TRAY, add, %ttMenuName%, ToggleShowTrayTip
		Menu, TRAY, add
		Menu, TRAY, Standard
		ttMenuAdded := true
	}
	Gosub, UpdateMultiClipboardMenu
	Gosub, UpdatePlainPasteMenu
	Gosub, UpdateRemoveAmpMenu
	Gosub, UpdateListAllMenu
	Gosub, UpdateShowTrayTipMenu
	OnMessage(WM_ACTIVATE, "GuiActivate")

	if (notSingleRun) {
		funcName = RemoveTempClipFile
		IfNotInString, OnExitSubs, %funcName%
		{
			OnExitSubs = %OnExitSubs%`n%funcName%
		}
	} else {
		OnExit, ExitSub							; Delete the temp file while script exit
	}

Goto, ShowClipboardEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; While the control key is pressed, show a tooltip beside mouse pointer to show the clipboard content. 
~Ctrl::
	ShowContent(GetClipboard())
	KeyWait Ctrl
	HideContent()
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Paste only plain text
$^#v::
	if (not IsHotkeyEnabled()) {
		return
	}
	if (not notPlainPaste and CpType <> 2) {
		If clipboard <>
		{
			clipFile=%A_Temp%\showClipboard.ahk.temp.clip.pp.YouCanDeleteItManually
			SetTimer, RemovePPTempClipFile, -300000		; Delete the temp file after 5 mins
			FileAppend, %ClipboardAll%, %clipFile%
			Transform, utf8Str, Unicode
			If utf8Str =
			{
				utf8Str := getFileList(clipFile)
			}
			utf8Str := RegExReplace(utf8Str, "([^\r])\n", "$1`r`n")
			utf8Str := RegExReplace(utf8Str, "^\n", "`r`n")
			Transform, Clipboard, Unicode, %utf8Str%
			Send, ^v
			FileRead, Clipboard, *c %clipFile%
		}
	} else {
		Send, ^#v
	}
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Ctrl-Win-C move the clipboard cursor to the next
$^#c::
	if (not IsHotkeyEnabled()) {
		return
	}
	if (notMultiClipboard) {
		Send, ^#c
	} else {
		clipboardOperating := true
		GoSub, Move2NextClip
		SetTimer, UpdateClipboardContent, -100
		GoSub, ShowClipboardStatus
	}
	return

; Ctrl-Win-X move the clipboard cursor to the prev
$^#x::
	x := IsHotkeyEnabled()
	if (not IsHotkeyEnabled()) {
		return
	}
	if (notMultiClipboard) {
		Send, ^#x
	} else {
		clipboardOperating := true
		GoSub, Move2PrevClip
		SetTimer, UpdateClipboardContent, -100
		GoSub, ShowClipboardStatus
	}
	return

; Ctrl-Win-D remove current clipboard content and move to the prev
$^#d::
	if (not IsHotkeyEnabled()) {
		return
	}
	if (notMultiClipboard) {
		Send, ^#d
	} else {
		clipboardOperating := true
		GoSub, RemoveClip
		SetTimer, UpdateClipboardContent, -100
		GoSub, ShowClipboardStatus
	}
	return

~^LWin::
~#Control::
~^RWin::
	if (not IsHotkeyEnabled()) {
		return
	}
	if (not notMultiClipboard) {
		GoSub, ShowClipboardStatus
	}
	return

~*^LWin Up::
~*^RWin Up::
~*#Control Up::
	if (not IsHotkeyEnabled()) {
		return
	}
	if (clipboardOperating || clipboardShowing) {
		clipboardOperating := false
		Gosub, HideClipboardStatus
	}
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Win-X Copy window title under mouse cursor. 
#x::
	if (not IsHotkeyEnabled()) {
		return
	}
	MouseGetPos,,, hWnd,,3
	len := DllCall("GetWindowTextLengthW", "Int", hWnd) + 1
	VarSetCapacity(title, len << 1, 0)
	DllCall("GetWindowTextW", "Int", hWnd, "Str", title, "Int", len)
	Unicode2Ansi(title, utf8Title, UTF8)
	if (title <> "") {
		Transform, Clipboard, Unicode, %utf8Title%
	}
	utf8Title=
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Win-C Copy control text under mouse cursor. 
#c::
	if (not IsHotkeyEnabled()) {
		return
	}
	MouseGetPos,,,, hWnd,3
	len := DllCall("SendMessageW", "Int", hWnd, "Int", WM_GETTEXTLENGTH, "Int", 0, "Int", 0) + 1
	VarSetCapacity(title, len << 1, 0)
	DllCall("SendMessageW", "Int", hWnd, "Int", WM_GETTEXT, "Int", len, "Str", title)
	Unicode2Ansi(title, utf8Title, UTF8)
	if (title <> "") {
		if (not notRemoveAmp) {
			utf8Title := removeAmp(utf8Title)
		}
		Transform, Clipboard, Unicode, %utf8Title%
	}
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Win-V Show all clipboards
$#v::
	if (not IsHotkeyEnabled()) {
		return
	}
	if (notListAll || notMultiClipboard) {
		Send #v
	} else {
		if (clipCount > 1) {
			IfWinNotExist, %ListWinTitle%
			{
				Gosub, ShowListAll
			}
		} else {
			Gosub, ShowClipboardStatus
			ShowContent(GetClipboard())
			SetTimer, HideClipboardAfterChange, -500
		}
	}
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Ctrl-Win-Alt-C Clear All Clipboard
^#!c::
	if (not IsHotkeyEnabled()) {
		return
	}
	Gosub, ClearClipboard
	return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Return whether all the hotkey are enabled
IsHotkeyEnabled() {
	global listShowing
	ret := not listShowing
	return ret
}

; Show List All Dialog
ShowListAll:
	SetTimer, ShowSplash, -200
	; Setup GUI
	listShowing := True
	rowNum := clipCount > MAX_ROW_NUM ? MAX_ROW_NUM : clipCount
	rowNum := rowNum < MIN_ROW_NUM ? MIN_ROW_NUM : rowNum
	if (not guiCreated) {
		Gui, 3: Margin, 0, 0
		Gui, 3: +ToolWindow +Resize
		Gui, 3: Add, ListView, r%rowNum% w500 gListAllEventHandler vClipList -LV0x10 +LV0x08 Grid Count%clipCount% AltSubmit, |No.|Content|Copy Time
		Gui, 3: Add, Button, Hidden Default, OK
		GuiControlGet, ClipList, Pos
		guiCreated := True
		rowH := ClipListH / rowNum
		
		Hotkey, IfWinActive, %ListWinTitle%
		Hotkey, Del, RemoveSelectedClips
		Hotkey, IfWinActive, %ListWinTitle%
		Hotkey, ^a, SelectAllRows
	} else {
		ClipListH := rowNum * rowH
		GuiControl, Move, ClipList, x%ClipListX% y%ClipListY% w%ClipListW% h%ClipListH%
	}
	GuiControl, -Redraw, ClipList
	Gosub, UpdateAllClipList
	GuiControl, +Redraw, ClipList
	Gui, 3: Show, Center AutoSize, %ListWinTitle%
	SetTimer, ShowSplash, Off
	SplashTextOff
	GoSub, FreeMemory
	return

SelectAllRows:
	Loop % LV_GetCount()
	{
		LV_Modify(A_Index, "select")
	}
	return


ShowSplash:
	SplashTextOn,200,20,Loading...,Loading Clipboards...
	return

3GuiClose:
	GuiControlGet, ClipList, Pos
	Gui, 3: Cancel
	GoSub, ClearContentCache
	GoSub, UpdateClipboardContent
	GoSub, FreeMemory
	listShowing := False
	return

3GuiEscape:
	GoSub, 3GuiClose
	return

3GuiSize:
	Gui, 3: Default
	if A_EventInfo = 1
		return
	GuiControl, Move, ClipList, % "w" . A_GuiWidth . "h" . A_GuiHeight
	LV_ModifyCol(3, A_GuiWidth - 155)
	return

; Update the GUI window
UpdateAllClipList:
	Gui, 3: Default
	co := clipboardOperating
	clipboardOperating := true
	LV_Delete()
	count := cid - 1
	list_1 := clipCursor
	Loop, %count%
	{
		list_1 := cprev_%list_1%
	}
	count := clipCount - 1
	Loop, %count%
	{
		next_idx := A_Index + 1
		curr_idx := list_%A_Index%
		list_%next_idx% := cnext_%curr_idx%
	}
	
	fileListPrefix = >>File List`n
	Loop, %clipCount%
	{
		idx := list_%A_Index%
		if (ctype_%idx% <> 2) {
			FileRead, Clipboard, *c %filePrefix%%idx%%fileSuffix%
		}
		curr =
		options =
		if (clipCursor = idx) {
			curr := ">"
			options := "select focus"
		}
		clipContent := GetClipboard(128, ctype_%idx%, fileListPrefix)
		cc_%A_Index% := clipContent
		lf = \n
		if (InStr(clipContent, fileListPrefix) = 1)
		{
			lf = //
		}
		StringReplace, clipContent, clipContent, `n, %lf%, All
		StringReplace, clipContent, clipContent, %A_Tab%, \t, All
		time := ctime_%idx%
		FormatTime, timeString, %time%, MM-dd HH:mm:ss
		LV_Add(options, curr, A_Index, clipContent, timeString)
		time =
		clipContent =
		timeString =
	}
	loadCount := clipCount
	LV_ModifyCol(1, "Auto")
	LV_ModifyCol(2, "Integer Right")
	LV_ModifyCol(3, A_GuiHeight - 155)
	LV_ModifyCOl(4, "Auto")
	Gosub, UpdateClipboardContent
	clipboardOperating := co
	return

ClearContentCache:
	Loop % loadCount
	{
		cc_%A_Index% =
		list_%A_Index% =
	}
	loadCount := 0
	return

; To let the gui window disappear when lost focus
GuiActivate(wParam, lParam)
{
	if (wParam = 0) {
		if A_Gui
		{
			Gosub, 3GuiClose
		}
	}
}

ListAllEventHandler:
	If A_GuiEvent = DoubleClick
	{
		GoSub, ChangeClip2Selected
	}
	Else If A_GuiEvent = R
	{
		GoSub, RemoveSelectedClips
	}
	Else If A_GuiEvent = I
	{
		if (InStr(ErrorLevel, "S", True) || InStr(ErrorLevel, "F", True)) {
			GoSub, ListToolTip
		} else if (LV_GetCount("Selected") = 0){
			GoSub, HideListToolTip
		}
	}
	GoSub, FreeMemory
	return

ListToolTip:
	LV_GetText(listIndex, A_EventInfo, 2)
	ToolTip, % cc_%listIndex%
	SetTimer, HideListToolTip, -5000
	return

HideListToolTip:
	ToolTip
	GoSub, FreeMemory
	return

ButtonOK:
	GoSub, ChangeClip2Selected
	GoSub, FreeMemory
	return

ChangeClip2Selected:
	focused := LV_GetNext(0, "Focused")
	if (focused = 0)
	{
		return
	}
	Loop % LV_GetCount()
	{
		LV_GetText(curr_mark, A_Index, 1)
		if (curr_mark = ">") {
			markRow := A_Index
			break
		}
	}
	if (focused <> markRow) {
		; Get list No.
		LV_GetText(listIndex, focused, 2)
		; Set current clip cursor
		clipCursor := list_%listIndex%
		cid := listIndex
		LV_Modify(focused, "Select", ">")
		LV_Modify(markRow, "", " ")
	}
	focused =
	markRow =
	curr_mark =
	listIndex =
	return

RemoveSelectedClips:
	minR := clipCount
	r := 1
	removedIdx =
	removedCount := 0
	Loop
	{
		oldCursor := clipCursor
		oldCid := cid
		oldCount := clipCount
		r := LV_GetNext(r - 1)
		if not r
		{
			if (removedCount = 0 && A_GuiEvent = "R") {
				GoSub, 3GuiClose
				return
			} else {
				break
			}
		}
		if (r < minR) {
			minR := r
		}
		LV_GetText(listIndex, r, 2)
		needRecover := True
		If list_%listIndex% = %clipCursor%
		{
			needRecover := False
		}
		clipCursor := list_%listIndex%
		cid := listIndex
		Gosub, RemoveClip
		LV_Delete(r)
		
		If removedIdx
		{
			removedIdx = %removedIdx%,%listIndex%
		}
		Else
		{
			removedIdx := listIndex
		}
		removedCount++
		
		if (needRecover) {
			clipCursor := oldCursor
			cid := oldCid
			if (clipCount < oldCount) {
				cid--
				if (cid < 0) {
					cid := clipCount
				}
			}
		}
	}
	
	if (LV_GetCount() = 0) {
		GoSub, 3GuiClose
		return
	}
	
	Sort removedIdx, N R D,
	listCount := LV_GetCount()
	Loop % listCount
	{
		LV_GetText(listIndex, A_Index, 2)
		idx := list_%listIndex%
		curr := " "
		if (clipCursor = idx) {
			curr := ">"
		}
		newIndex := listIndex
		Loop, parse, removedIdx, `,
		{
			If (listIndex > A_LoopField) {
				newIndex := listIndex - removedCount + A_Index - 1
				break
			}
		}
		newList_%newIndex% := idx
		newCc_%newIndex% := cc_%listIndex%
		LV_Modify(A_Index, "", curr, newIndex)
	}
	Loop % LV_GetCount()
	{
		list_%A_Index% := newList_%A_Index%
		cc_%A_Index% := newCc_%A_Index%
		newList_%A_Index% =
		newCc_%A_Index% =
	}
	return

; Get file list from clipboard temp file
getFileList(ByRef clipFile) 
{
	FileRead, fileClip, %clipFile%
	FileGetSize, nSize, %clipFile%
	zeroCount := 0
	i := 0
	Loop, %nSize%
	{
		i := A_Index - 1
		if (*(&fileClip + i) = 0) {
			zeroCount++
		} else {
			zeroCount := 0
		}
		if (zeroCount = 15 and *(&fileClip + i + 1) = 1) {
			break
		}
	}
	offset := i + 5
	count := 0
	zeroCount := 0
	pos_%count% := offset
	Loop
	{
		index := A_Index - 1
		pos := offset + (index << 1)
		n := NumGet(fileClip, pos, "Short")
		if (n = 0) {
			count++
			pos_%count% := pos + 2
			zeroCount++
		} else {
			zeroCount := 0
		}
		if (zeroCount >= 2) {
			count--
			break
		}
	}
	fileList=
	Loop, %count%
	{
		index := A_Index - 1
		len := ((pos_%A_Index% - pos_%index%) >> 1)
		VarSetCapacity(filePath, (len << 1), 0)
		i := 0
		Loop, %len%
		{
			pos := pos_%index% + i
			n := NumGet(fileClip, pos, "Short")
			NumPut(n, filePath, i, "Short")
			i += 2
		}
		Unicode2Ansi(filePath, utf8File, UTF8)
		If fileList=
			fileList=%utf8File%
		Else
			fileList=%fileList%`r`n%utf8File%
		filePath=
		utf8File=
	}
	fileClip=
	
	return fileList
}

; Remove Temp file for Plain Paste
RemovePPTempClipFile:
	if clipFile <>
	{
		IfExist, %clipFile%
		{
			FileDelete, %clipFile%
		}
	}
	return

; Remove Temp file for Multi-Clipboard
RemoveMCTempClipFile:
	if filePrefix <>
	{
		IfExist, %filePrefix%curr%fileSuffix%
		{
			FileDelete, %filePrefix%curr%fileSuffix%
		}
		count := maxHistory > maxCursor ? maxHistory : maxCursor
		Loop, %count%
		{
			fileName=%filePrefix%%A_Index%%fileSuffix%
			IfExist, %fileName%
			{
				FileDelete, %fileName%
			}
		}
	}
	return

; Remove Temp file for clipboard store. 
RemoveTempClipFile:
	Gosub, RemovePPTempClipFile
	Gosub, RemoveMCTempClipFile
	return

ExitSub:
	GoSub, RemoveTempClipFile
	ExitApp
	return

; While the clipboard content is changed, update the tooltip. 
OnClipboardChange:
	CpType := A_EventInfo
	ClipWait, 0
	HideContent()
	if (not IsHotkeyEnabled()) {
		return
	}
	Gosub, UpdateClearClipboardMenu
	if (not notMultiClipboard) {
		GoSub, NewCopied
	}
	ShowContent(GetClipboard())
	SetTimer, HideClipboardAfterChange, -500
	return

HideClipboardAfterChange:
	; If control key is pressed, restart this timer after a very short period. 
	; Otherwise, remove the tooltip
	GetKeyState, state, Ctrl
	if state=D
	{
		SetTimer, HideClipboardAfterChange, -10
	}
	else
	{
		HideContent()
		Gosub, HideClipboardStatus
		SetTimer, HideClipboardAfterChange, off
	}
	return

; Run when copied new thing
NewCopied:
	if (not clipboardOperating) {
		filePrefix = %A_Temp%\showClipboard.ahk.temp.clip.
		fileSuffix = .YouCanDeleteItManually
		if (not IsClipboardEmpty()) {
;			If copyCount=
;				copyCount := 0
;			copyCount++
;			SetTimer, CopiedWait, -300
			if (IsCurrentClipEqualsCursorClip(clipCount, clipCursor, maxCursor, filePrefix, fileSuffix)) {
			} else {
				GoSub, AddClip
			}
		}
		GoSub, ShowClipboardStatus
	}
	GoSub, FreeMemory
	return

CopiedWait:
	if (copyCount > 1) {
		Gosub, ClearClip
		Gosub, AddClip
	}
	copyCount := 0
	return

ShowClipboardStatus:
	if (notShowTrayTip) {
		return
	}
	clipboardShowing := true
	if (clipCount = "" or clipCount = 0) {
		TrayTip, Clipboard, <empty>
	} else {
		timeString := GetTimeInfor(ctime_%clipCursor%, ctick_%clipCursor%)
		TrayTip,Clipboard,%timeString%`nNo.[%cid%] / Total: %clipCount%
	}
	return

GetTimeInfor(ByRef ctime, ByRef tick)
{
	FormatTime, timeString, %ctime%, MM-dd HH:mm:ss
	dtick := A_TickCount - tick
;	dtick //= 1000
;	min := dtick // 60
;	sec := mod(dtick, 60)
;	hr := dtick // 3600
;	min -= hr * 60
;	SetFormat, float, 02.0
;	hr -= 0.0
;	SetFormat, float, 02.0
;	min -= 0.0
;	SetFormat, float, 02.0
;	sec -= 0.0
;	period = %hr%:%min%:%sec%
	SetFormat, floatFast, 4.0
	min := dtick / 60000
	period := "(" . min . " min. ago)"
	ret = Copied at [%timeString%] %period%
	return ret
}

HideClipboardStatus:
	clipboardShowing := false
	TrayTip
	GoSub, FreeMemory
	return

Move2PrevClip:
	if (clipCount = 0 or clipCount = 1) {
		return
	} else {
		clipCursor := cprev_%clipCursor%
		cid--
		if (cid < 1) {
			cid := clipCount
		}
	}
	return

Move2NextClip:
	if (clipCount = 0 or clipCount = 1) {
		return
	} else {
		clipCursor := cnext_%clipCursor%
		cid++
		if (cid > clipCount) {
			cid := 1
		}
	}
	return

UpdateClipboardContent:
	FileRead, Clipboard, *c %filePrefix%%clipCursor%%fileSuffix%
	GoSub, FreeMemory
	return

AddClip:
	clipCount++
	maxCursor++
	newCursor := maxCursor
	fileName = %filePrefix%%newCursor%%fileSuffix%
	ctime_%newCursor% := A_Now
	ctick_%newCursor% := A_TickCount
	ctype_%newCursor% := CpType
	FileAppend, %ClipboardAll%, %fileName%
	if (clipCount = 1) {
		cprev_%newCursor% := 1
		cnext_%newCursor% := 1
		cid := 1
	} else {
		cprev_%newCursor% := clipCursor
		cnext_%newCursor% := cnext_%clipCursor%
		nextCursor := cnext_%clipCursor%
		cprev_%nextCursor% := newCursor
		cnext_%clipCursor% := newCursor
		cid++
		if (cid > clipCount) {
			cid := 1
		}
	}
	clipCursor := newCursor
	GoSub, ShowClipboardStatus
	return

RemoveClip:
	clipCount--
	if (clipCount <= 0) {
		GoSub, ClearClip
	} else {
		prevCursor := cprev_%clipCursor%
		nextCursor := cnext_%clipCursor%
		cnext_%prevCursor% := cnext_%clipCursor%
		cprev_%nextCursor% := cprev_%clipCursor%
		clipCursor := prevCursor
		cid--
		if (cid < 1) {
			cid := clipCount
		}
	}
	return

ClearClip:
	clipCursor := 0
	if (maxCursor > maxHistory) {
		maxHistory := maxCursor
	}
	maxCursor := 0
	clipCount := 0
	return

IsCurrentClipEqualsCursorClip(ByRef clipCount, ByRef clipCursor, ByRef maxCursor, ByRef filePrefix, ByRef fileSuffix) {
	if (clipCount = 0 or clipCursor = "") {
		clipCount := 0
		clipCursor := 0
		maxCursor := 0
		return false
	} else {
		If Clipboard <> %Clipboard%
			return false
		FileAppend, %ClipboardAll%, %filePrefix%curr%fileSuffix%
		FileGetSize, currSize, %filePrefix%curr%fileSuffix%
		FileGetSize, cursSize, %filePrefix%%clipCursor%%fileSuffix%
		if (currSize = cursSize) {
			FileRead, currContent, *c %filePrefix%curr%fileSuffix%
			FileRead, cursContent, *c %filePrefix%%clipCursor%%fileSuffix%
			Loop, %currSize%
			{
				index := A_Index - 1
				r := *(&currContent + index)
				s := *(&cursContent + index)
				if (*(&currContent + index) <> *(&cursContent + index)) 
					return false
			}
			return true
		} else {
			return false
		}
	}
}

; Update Clear Clipboard Menu Status
UpdateClearClipboardMenu:
	if (clipboard <> "" or CpType = 2) {
		Menu, TRAY, enable, %ccMenuName%
	} else {
		Menu, TRAY, disable, %ccMenuName%
	}
	return

; Clear the Clipboard
ClearClipboard:
	if (not notMultiClipboard) {
		GoSub, ClearClip
	}
	Clipboard := ""
	return

; Update Remove & Menu Status
UpdateRemoveAmpMenu:
	if (not notRemoveAmp) {
		Menu, TRAY, Check, %raMenuName%
	} else {
		Menu, TRAY, Uncheck, %raMenuName%
	}
	return

; Toggle Plain Paste
ToggleRemoveAmp:
	notRemoveAmp := !notRemoveAmp
	GoSub, UpdateRemoveAmpMenu
	return

; Update PlainPaste Menu Status
UpdatePlainPasteMenu:
	if (not notPlainPaste) {
		Menu, TRAY, Check, %ppMenuName%
	} else {
		Menu, TRAY, Uncheck, %ppMenuName%
	}
	return

; Toggle Plain Paste
TogglePlainPaste:
	notPlainPaste := !notPlainPaste
	GoSub, UpdatePlainPasteMenu
	return

; Update Multi-clipboard menu status
UpdateMultiClipboardMenu:
	if (not notMultiClipboard) {
		Menu, TRAY, Check, %mcMenuName%
		Menu, TRAY, Enable, %lsMenuName%
		GoSub, NewCopied
	} else {
		Menu, TRAY, Uncheck, %mcMenuName%
		Menu, TRAY, Disable, %lsMenuName%
		GoSub, ClearClip
	}
	return

; Toggle Multi Clipboard
ToggleMultiClipboard:
	notMultiClipboard := !notMultiClipboard
	GoSub, UpdateMultiClipboardMenu
	return

; Update List-All-clipboard menu status
UpdateListAllMenu:
	if (not notListAll) {
		Menu, TRAY, Check, %lsMenuName%
	} else {
		Menu, TRAY, Uncheck, %lsMenuName%
	}
	return

; Toggle List All
ToggleListAll:
	notListAll := !notListAll
	GoSub, UpdateListAllMenu
	return

; Update Show TrayTip menu status
UpdateShowTrayTipMenu:
	if (not notShowTrayTip) {
		Menu, TRAY, Check, %ttMenuName%
	} else {
		Menu, TRAY, Uncheck, %ttMenuName%
	}
	return

; Toggle Show TrayTip
ToggleShowTrayTip:
	notShowTrayTip := !notShowTrayTip
	GoSub, UpdateShowTrayTipMenu
	return

; Show Clipboard ToolTips
ShowContent(content)
{
	ToolTip, %content%
	return
}

; Hide Clipboard ToopTips
HideContent()
{
	ToolTip
	GoSub, FreeMemory
	return
}

; Remove & from a string
RemoveAmp(ByRef text) {
	StringSplit, sArray, text, &
	string=
	count := 0
	Loop, %sArray0%
	{
		s := sArray%A_Index%
		if (s = "" and A_Index < sArray0 and A_Index > 1) {
			count++
			if (mod(count, 2) <> 0) {
				s=&
			}
		} else {
			count := 0
		}
		string=%string%%s%
		oldS := s
	}
	return string
}

; Return a clipboard description string
GetClipboard(MAX_LENGTH = 256, type = -1, filePrefix = "") {
	global CpType
	if (type < 0) {
		type := CpType
	}
	uni =
	if (type = 2) {
		uni = ***<Non-text>***
	} else {
		Transform, utf8Str, Unicode
		if (StrLen(utf8Str) > MAX_LENGTH)
		{
			utf8Str := SubStr(utf8Str, 1, MAX_LENGTH) . "......"
		}
		if (utf8Str <> "") {
			uni := UTF82Ansi(utf8Str)
			utf8Str =
		} else if (clipboard <> "") {
			if (filePrefix = "") {
				filePrefix = <Copied File>`r`n==========`r`n
			}
			uni = %filePrefix%%clipboard%
			if (StrLen(uni) > MAX_LENGTH) {
				n := InStr(uni, "`n", True, MAX_LENGTH)
				if (n > 0) {
					uni := SubStr(uni, 1, n) . "......"
				}
			}
		}
	}
	Gosub, FreeMemory
	return %uni%
}

; Whether the clipboard is empty
IsClipboardEmpty() {
	global CpType
	if (CpType = 2) {
		return false
	} else if (Clipboard = "") {
		return true
	} else {
		return false
	}
}

FreeMemory:
	hProcess := DllCall("GetCurrentProcess")
	DllCall("SetProcessWorkingSetSize", Int, hProcess, Int, -1, Int, -1)
	return

/*
CP_ACP   = 0
CP_OEMCP = 1
CP_MACCP = 2
CP_UTF7  = 65000
CP_UTF8  = 65001
*/

Ansi2Oem(ByRef sString)
{
   Ansi2Unicode(sString, wString, 0)
   Unicode2Ansi(wString, zString, 1)
   Return zString
}

Oem2Ansi(ByRef zString)
{
   Ansi2Unicode(zString, wString, 1)
   Unicode2Ansi(wString, sString, 0)
   Return sString
}

Ansi2UTF8(ByRef sString)
{
   Ansi2Unicode(sString, wString, 0)
   Unicode2Ansi(wString, zString, 65001)
   Return zString
}

UTF82Ansi(ByRef zString)
{
   Ansi2Unicode(zString, wString, 65001)
   Unicode2Ansi(wString, sString, 0)
   Return sString
}

Ansi2Unicode(ByRef sString, ByRef wString, CP = 0)
{
     nSize := DllCall("MultiByteToWideChar"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &sString
      , "int",  -1
      , "Uint", 0
      , "int",  0)

   VarSetCapacity(wString, nSize << 1)

   DllCall("MultiByteToWideChar"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &sString
      , "int",  -1
      , "Uint", &wString
      , "int",  nSize)
}

Unicode2Ansi(ByRef wString, ByRef sString, CP = 0)
{
     nSize := DllCall("WideCharToMultiByte"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &wString
      , "int",  -1
      , "Uint", 0
      , "int",  0
      , "Uint", 0
      , "Uint", 0)

   VarSetCapacity(sString, nSize)

   DllCall("WideCharToMultiByte"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &wString
      , "int",  -1
      , "str",  sString
      , "int",  nSize
      , "Uint", 0
      , "Uint", 0)
}

ShowClipboardEnd:

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

StartDateEnd:

; ==================== AlwaysOnTop ================
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
; Toggle Active window always on top.
; HotKey: 
; 	Ctrl-Alt-HOME: Switch always on top

Goto, AlwaysOnTopActiveEnd

^!Home::
	WinSet, AlwaysOnTop, Toggle, A
	return

AlwaysOnTopActiveEnd:
