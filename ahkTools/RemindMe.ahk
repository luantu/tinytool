#SingleInstance off
DEFINE:
    pId := ""
    pName := ""
    listTitle := "Processes [Double click to select]"
Main:
    Hotkey, IfWinActive, %listTitle%
    Hotkey, Enter, ButtonOK
    Hotkey, Esc, EscPressed
    Hotkey, Up, UpPressed
    Hotkey, Down, DownPressed
    GoSub, ShowProcessList
    WinWaitClose, %listTitle%
    Goto, RemindMeEnd

UpPressed:
    Gui, 3:Default
    n := LV_GetNext() - 1
    if n <= 0
        n = 1
    LV_Modify(n, "Select Focus Vis")
    return

DownPressed:
    Gui, 3:Default
    count := LV_GetCount()
    n := LV_GetNext() + 1
    if n > count
        n = count
    LV_Modify(n, "Select Focus Vis")
    return

EscPressed:
    Gui, 3:Default
    GuiControlGet, ProcFilter
    filter := Trim(ProcFilter)
    if (filter <> "")
        GuiControl, , ProcFilter, 
    else
        Gui, Cancel
    return

WaitAndRemind:
    if pId <> ,
    {
        pInfo = %pName%[%pId%]
        TrayTip, Wating..., %pInfo%
        Menu, tray, Tip, Waiting %pInfo% to close...
        Process, WaitClose, %pId%
    }
    else
    {
        pInfo = %pName%
        TrayTip, Wating..., %pInfo%
        Menu, tray, Tip, Waiting %pInfo% to close...
        Process, WaitClose, %pName%
    }
    MsgBox, 0x1040, Remind, Process [%pName%] you are waiting is finished!
    return

HandleSelect:
    Gui, 3:Default
    n := LV_GetNext()
    if n = 0 
        n := 1
    LV_GetText(pId, n, 1)
    LV_GetText(pName, n, 2)
	Gui, 3: Destroy
    GoSub, WaitAndRemind
    return

ShowSplash:
	SplashTextOn,200,20,Loading...,Loading Processes...
	return

ShowProcessList:
	SetTimer, ShowSplash, -200
	; Setup GUI
	listShowing := True
	if (not guiCreated) {
		Gui, 3: Margin, 0, 0
		Gui, 3: +Resize
		Gui, 3: Add, Edit, vProcFilter HwndProcFilterHwnd
		Gui, 3: Add, ListView, -Multi r50 w300 gListEventHandler vProcList -LV0x10 +LV0x08 Grid Count%pCount% AltSubmit, PID|Names
		Gui, 3: Add, Button, Hidden Default, OK
		SetTimer, FilterUpdated, 50
		guiCreated := True
	}
	GuiControl, -Redraw, ProcList
	Gosub, RetrieveProcessList
	Gosub, UpdateProcessList
	GuiControl, +Redraw, ProcList
	Gui, 3: Show, Center AutoSize, %listTitle%
	SetTimer, ShowSplash, Off
	SplashTextOff
    return

ListEventHandler:
	If A_GuiEvent = DoubleClick
	{
		GoSub, HandleSelect
	}
	If A_GuiEvent = K
	{
	    ControlFocus, , ahk_id %ProcFilterHwnd%
	    key := Chr(A_EventInfo)
	    Send %key%
	}
    return

ButtonOK:
    Gosub, HandleSelect
    return

3GuiSize:
	Gui, 3: Default
	if A_EventInfo = 1
		return
    GuiControl, Move, ProcFilter, % "w" . A_GuiWidth
	GuiControl, Move, ProcList, % "w" . A_GuiWidth . "h" . (A_GuiHeight - 20)
	LV_ModifyCol(2, A_GuiWidth - 26)
	return

GuiEscape:
GuiClose:
    Goto, RemindMeEnd
    return

FilterUpdated:
    Gui, 3: Default
	GuiControlGet, ProcFilter
	filter := Trim(ProcFilter)
	if (filter <> preFilter)
	{
	    GoSub, UpdateProcessList
	    preFilter := filter
	}
    return

UpdateProcessList:
	Gui, 3: Default
	LV_Delete()
	Loop, %pCount%
	{
	    c := A_Index - 1
	    if (filter = "" or RegExMatch(pName_%c%, filter) > 0)
	        LV_Add("", pId_%c%, pName_%c%)
	}
	LV_ModifyCol(1, "Auto")
	LV_ModifyCol(1, "Integer Right")
	;LV_ModifyCOl(2, "Auto")
	return

RetrieveProcessList:
    ; Retrieves a list of running processes via DllCall then shows them in a ListBox.

    d := "  |  "  ; string separator
    s := 4096  ; size of buffers and arrays (4 KB)
    pCount := 0

    Process, Exist  ; sets ErrorLevel to the PID of this running script
    ; Get the handle of this script with PROCESS_QUERY_INFORMATION (0x0400)
    h := DllCall("OpenProcess", "UInt", 0x0400, "Int", false, "UInt", ErrorLevel, "Ptr")
    ; Open an adjustable access token with this process (TOKEN_ADJUST_PRIVILEGES = 32)
    DllCall("Advapi32.dll\OpenProcessToken", "Ptr", h, "UInt", 32, "PtrP", t)
    VarSetCapacity(ti, 16, 0)  ; structure of privileges
    NumPut(1, ti, 0, "UInt")  ; one entry in the privileges array...
    ; Retrieves the locally unique identifier of the debug privilege:
    DllCall("Advapi32.dll\LookupPrivilegeValue", "Ptr", 0, "Str", "SeDebugPrivilege", "Int64P", luid)
    NumPut(luid, ti, 4, "Int64")
    NumPut(2, ti, 12, "UInt")  ; enable this privilege: SE_PRIVILEGE_ENABLED = 2
    ; Update the privileges of this process with the new access token:
    r := DllCall("Advapi32.dll\AdjustTokenPrivileges", "Ptr", t, "Int", false, "Ptr", &ti, "UInt", 0, "Ptr", 0, "Ptr", 0)
    DllCall("CloseHandle", "Ptr", t)  ; close this access token handle to save memory
    DllCall("CloseHandle", "Ptr", h)  ; close this process handle to save memory

    hModule := DllCall("LoadLibrary", "Str", "Psapi.dll")  ; increase performance by preloading the library
    s := VarSetCapacity(a, s)  ; an array that receives the list of process identifiers:
    c := 0  ; counter for process idendifiers
    DllCall("Psapi.dll\EnumProcesses", "Ptr", &a, "UInt", s, "UIntP", r)
    Loop, % r // 4  ; parse array for identifiers as DWORDs (32 bits):
    {
       id := NumGet(a, A_Index * 4, "UInt")
       pId_%c% := id
       ; Open process with: PROCESS_VM_READ (0x0010) | PROCESS_QUERY_INFORMATION (0x0400)
       h := DllCall("OpenProcess", "UInt", 0x0010 | 0x0400, "Int", false, "UInt", id, "Ptr")
       if !h
          continue
       VarSetCapacity(n, s, 0)  ; a buffer that receives the base name of the module:
       e := DllCall("Psapi.dll\GetModuleBaseName", "Ptr", h, "Ptr", 0, "Str", n, "UInt", A_IsUnicode ? s//2 : s)
       if !e    ; fall-back method for 64-bit processes when in 32-bit mode:
          if e := DllCall("Psapi.dll\GetProcessImageFileName", "Ptr", h, "Str", n, "UInt", A_IsUnicode ? s//2 : s)
             SplitPath n, n
       DllCall("CloseHandle", "Ptr", h)  ; close process handle to save memory
       pName_%c% := n
       if (n && e)  ; if image is not null add to list:
          l .= n . d, c++
    }
    pCount := c
    DllCall("FreeLibrary", "Ptr", hModule)  ; unload the library to free memory
    return

RemindMeEnd:
    ExitApp
    return
