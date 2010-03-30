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

