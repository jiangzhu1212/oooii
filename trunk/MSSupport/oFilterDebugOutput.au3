#Include <File.au3>

If $CmdLine[0] < 3 Then
	MsgBox(64, @ScriptName, "Usage: " & @ScriptName & " <log filepath> switch regex [... switch regex]"  & @CRLF & "o switch is either ""-i"" for include or ""-e"" for exclude" & @CRLF & "o regex is an AutoIt 3.0 regular expression that if a line matches it will either include or exclude that line based on the switch.")
	Exit(1)
EndIf

Dim $log
$result = _FileReadToArray($CmdLine[1], $log)
If $result = 0 Then
	MsgBox(16, @ScriptName, "Failed to read file """ & $CmdLine[1] & """")
	Exit(2)
EndIf

$DestinationPath = StringReplace($CmdLine[1], ".", "_filtered.", -1, 2)
$DestinationFile = FileOpen($DestinationPath, 2+8)

ProgressOn(@ScriptName, "Processing " & $CmdLine[1] & " (" & $log[0] & " lines)", "")

For $i = 1 To $log[0]
	If FilterChainPasses($log[$i], $CmdLine, 2, False) Then FileWriteLine($DestinationFile,$log[$i])
	$percent = $i * 100 / $log[0]
	If Mod($i, 200) == 0 Then ProgressSet($percent, "Processing " & $CmdLine[1] & " (" & $log[0] & " lines)", "")
Next

ProgressSet(100, "Processing " & $CmdLine[1] & " complete.", "")
FileClose($DestinationFile)
ShellExecute($DestinationPath, "", @ScriptDir, "edit")
Sleep(500)
ProgressOff()
Exit(0)

Func FilterChainPasses_IsInclude($strFilterTypeSwitch)
	If (0 = StringCompare("-e", $strFilterTypeSwitch, 2)) Then Return False
	If (0 = StringCompare("-i", $strFilterTypeSwitch, 2)) Then Return True
	MsgBox(64, @ScriptName, "Invalid filter type switch """ & $strFilterTypeSwitch &  """")
	Exit(3)
EndFunc

Func FilterChainPasses($strTest, $FilterArray, $FilterArrayStartIndex, $bPassesWhenEmpty)
	If $FilterArrayStartIndex > UBound($FilterArray) Then Return $bPassesWhenEmpty
	; Initialize passing to the opposite of the first filter options so if
	; the first filter matches it changes the initial state
	$Passes = Not FilterChainPasses_IsInclude($FilterArray[$FilterArrayStartIndex])
	For $i = $FilterArrayStartIndex To UBound($FilterArray)-1 Step 2
		$IsInclude = FilterChainPasses_IsInclude($FilterArray[$i])
		$test = StringRegExp($strTest, $FilterArray[$i+1], 0) == 1
		If 1 = StringRegExp($strTest, $FilterArray[$i+1], 0) Then $Passes = $IsInclude
	Next
	Return $Passes
EndFunc
