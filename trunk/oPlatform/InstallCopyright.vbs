Option Explicit

'
' Collections are very annoying in VBScript, so convert it
' here to an array.
'
Function ScriptGetArgumentArray()

	Dim dict
	Set dict = CreateObject("Scripting.Dictionary")

	Dim args
	Set args = WScript.Arguments

	Dim arg
	Dim i
	i = 0

	For Each arg In args
		dict.Add i, arg
		i = i + 1
	Next

	Dim items
	items = dict.Items()

	ScriptGetArgumentArray = items

End Function

'
' Returns True if the specified file is a text file, 
' False if it is binary.
'
Function FileIsText(strSrcPath)
	Dim ForReading
	ForReading = 1 'By Definition

	Dim fso
	Set fso = CreateObject("Scripting.FileSystemObject")
	
	Dim str, file, actualSize, nonTextCount, percentNonAscii
	
	' Read this much of the file to determine text v. binary
	Dim BLOCK_SIZE, PERCENT_NON_ASCII_TO_BE_BINARY
	BLOCK_SIZE = 512
	PERCENT_NON_ASCII_TO_BE_BINARY = 0.05

	On Error Resume Next
		Set file = fso.OpenTextFile(strSrcPath, ForReading, False)
		str = file.Read(BLOCK_SIZE)
		file.Close
	On Error Goto 0
	
	actualSize = Len(str)
	percentNonAscii = 1.0
	If actualSize > 0 Then
		nonTextCount = 0

		Dim i, c
		For i = 1 To Len(str)
			c = Asc(Mid(str, i, 1))
			If (c <= 0) Then nonTextCount = nonTextCount + 1
		Next

		percentNonAscii = nonTextCount / actualSize
		FileIsText = percentNonAscii < PERCENT_NON_ASCII_TO_BE_BINARY
		Exit Function
  End If
	FileIsText = False
End Function

'
' Loads the entire specifed text file and returns it as a string.
' (Remember that every newline is two characters cr Chr(13) and lf Chr(10).
'
' If this fails, it returns an empty string.
'
Function FileLoadAsString(strSrcPath)
	Dim ForReading
	ForReading = 1 'By Definition

	Dim fso
	Set fso = CreateObject("Scripting.FileSystemObject")
	
	Dim str
	Dim file

	On Error Resume Next

		Set file = fso.OpenTextFile(strSrcPath, ForReading, False)

		str = file.ReadAll()
		file.Close

		If Len(str) = 0 Then str = ""

	On Error Goto 0

	FileLoadAsString = str
End Function

'
' Saves the specified string "strSrc" to the specified text file
' "strDestPath". If a file already exists, it will be overwritten.
'
' This returns true if the write was successful, false otherwise.
'
Function FileSaveString(strDestPath, strSrc)
	Dim ForWriting
	ForWriting = 2 'By Definition

	Dim fso
	Set fso = CreateObject("Scripting.FileSystemObject")

	On Error Resume Next
		
		If (fso.FileExists(strDestPath)) Then 
			fso.DeleteFile strDestPath
			If Err.Number <> 0 Then
				MsgBox "FileSaveString: Failed to delete """ & strDestPath & """" & vbNewline & Err.Description, 0, "ERROR"
				FileSaveString = False
				Exit Function
			End If
		End If			
		
	On Error Goto 0

	On Error Resume Next

		Dim file
		Set file = fso.OpenTextFile(strDestPath, ForWriting, True)

		If Err.Number <> 0 Then
			FileSaveString = False
			Exit Function
		End If

		Dim str
		str = Replace(strSrc, vbLf, "")
		str = Replace(str, vbCr, vbNewline)

		file.Write str
		If Err.Number <> 0 Then
			FileSaveString = False
			Exit Function
		End If

	On Error Goto 0

	file.Close

	FileSaveString = True

End Function

'
' Finds strFind in strSearch and replaces it with strReplacement
' inline. This returns the position of the first character after 
' the insertion.
'
Function StringReplace(iStart, strSearch, strFind, strReplacement, iCompare)
	Dim p
	p = InStr(iStart, strSearch, strFind, iCompare)
	If p <> 0 Then
		strSearch = Left(strSearch, p-1) & strReplacement & Right(strSearch, Len(strSearch) - Len(strFind) - p + 1)
		StringReplace = p + Len(strReplacement)
	Else
		StringReplace = 0
	End If
End Function

Sub ReplaceInAllFiles(Files, strMacro, strCopyright, strIgnoreExtensions)
	Dim file, strFile, p, ext
	For Each file In Files
		ext = Right(file.Path, 4)
    If InStr(1, strIgnoreExtensions, ext, 1) = 0 And FileIsText(file.Path) Then
      strFile = FileLoadAsString(file.Path)
      p = StringReplace(1, strFile, strMacro, strCopyright, 1)
      If p <> 0 And Not FileSaveString(file.Path, strFile) Then
        MsgBox "Error saving " & file.Path, 0, "ERROR"
      End If
    End If
	Next
End Sub

Sub ReplaceAllInFolder(Folder, strMacro, strCopyright, strIgnoreExtensions)
	If folder.Name <> ".svn" Then 'Skip version control folders
		ReplaceInAllFiles Folder.Files, strMacro, strCopyright, strIgnoreExtensions
		Dim subfolder
		For Each subfolder in Folder.SubFolders
			ReplaceAllInFolder subfolder, strMacro, strCopyright, strIgnoreExtensions
		Next
	End If
End Sub

Sub Main()

	Dim args
	args = ScriptGetArgumentArray()

	If UBound(args) < 2 Then
		MsgBox "Usage: " & WScript.ScriptName & " <Macro> <ReplacementFile.txt> <RootPath>"
		WScript.Quit 0
	End If

	Dim copyright, macro
	copyright = FileLoadAsString(args(1))
	macro = "// $(" & args(0) & ")"
	
	Dim fso, root, folder
	Set fso = CreateObject("Scripting.FileSystemObject")
	Set root = fso.GetFolder(fso.GetAbsolutePathName(args(2)))
	ReplaceAllInFolder root, macro, copyright, ".ico .png .bmp .jpg .xml .ini .vcproj .sln .bak"
	msgbox "Done inserting copyright", args(2)
End Sub

Main
