#cs ----------------------------------------------------------------------------

 AutoIt Version: 3.3.6.1
 Author:         myName

 Script Function:
	Template AutoIt script.

#ce ----------------------------------------------------------------------------

; Script Start - Add your code below here

$ProcessName = "MSBuild.exe"

While (ProcessExists($ProcessName))
	ProcessClose($ProcessName)
WEnd