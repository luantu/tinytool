#Persistent
#SingleInstance ignore
#NoTrayIcon

OldServers := 0

SetTimer, GetServerStatus, 500

GetServerStatus:
	Servers := 0

	for proc in ComObjGet("winmgmts:").ExecQuery("Select * from Win32_Process")
	{
		If InStr(proc.Name, "java")
		{
			pattern=WebSphere\\AppServer\\profiles\\(?P<Profile>[A-Za-z0-9_-]+)\\config"\s+(?P<Cell>[A-Za-z0-9_-]+)\s+(?P<Node>[A-Za-z0-9_-]+)\s+(?P<Server>[A-Za-z0-9_-]+)
			If RegExMatch(proc.CommandLine, pattern, ServerInfo)
			{
				Servers++
				Servers%Servers% := ServerInfoProfile . " (Cell=" . ServerInfoCell . ", Node=" . ServerInfoNode . ", Server=" . ServerInfoServer . ")"
			}
		}
	}

	If Servers > 0
	{
		SwitchIcon(True)
		TipMsg := ">> Running WAS Server <<`n"
		Loop, %Servers%
		{
			TipMsg := TipMsg . Servers%A_Index%
			If (A_Index < Servers)
			{
				TipMsg := TipMsg . "`n-------`n"
			}
		}
		
		Menu, Tray, Tip, %TipMsg%
		
		If (OldServers < Servers)
		{
			TrayTip, Server Started!, %TipMsg%, , 1
			OldServers := Servers
		}
		Else If (OldServers > Servers)
		{
			TrayTip, Some Server Stopped., %TipMsg%, 2
			OldServers := Servers
		}
	}
	Else
	{
		SwitchIcon(False)
		Menu, Tray, Tip, >> Running WAS Server <<`nNo Server running.
		If (OldServers > Servers)
		{
			TrayTip, All Server Stopped!, No Server running now., , 2
			OldServers := Servers
		}
	}
	Return
	
SwitchIcon(running)
{
	If running
	{
		Menu, Tray, Icon, Shell32.dll, 138, 1
	}
	Else
	{
		Menu, Tray, Icon, Shell32.dll, 110, 1		
	}
	Menu, Tray, Icon
}