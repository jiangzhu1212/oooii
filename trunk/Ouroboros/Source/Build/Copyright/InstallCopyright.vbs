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

Function ArrayFromString(string, strToken)

	Dim dict
	Set dict = CreateObject("Scripting.Dictionary")

	Dim lstring
	lstring = string

	Dim pos
	pos = InStr(1, lstring, strToken)

	Dim index
	index = 0

	Do While pos <> 0
		Dim token
		token = Left(lstring, pos - 1)
		lstring = Right(lstring, Len(lstring) - (pos + Len(strToken) - 1))
		dict.Add index, token
		index = index + 1
		pos = InStr(1, lstring, strToken)
	Loop

	If Len(lstring) > 0 Then
		dict.Add index, lstring
	End If

	Dim arr
	arr = dict.Items
	ArrayFromString = arr
End Function

'
' Goes through an array of strings and returns the number of characters that are
' the same at the beginning of all strings
'
Function StringArrayGetCommonPrefixLength(strarrStrings)
    Dim prefixLen
    prefixLen = 0
    Dim len0
    len0 = Len(strarrStrings(0))
    Dim cc
    While prefixLen < Len0
        cc = Mid(strarrStrings(0), prefixLen+1, 1)
        Dim i
        For i = 0 To UBound(strarrStrings)
            If Mid(strarrStrings(i), prefixLen+1, 1) <> cc Then
                StringArrayGetCommonPrefixLength = prefixLen
                Exit Function
            End If
        Next
        prefixLen = prefixLen + 1
    WEnd
    StringArrayGetCommonPrefixLength = 0
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

Sub ReplaceInAllFiles(Files, strMacro, strCopyright, strIgnoreExtensions, strNoMacroFileList)
	Dim file, strFile, p, ext
	For Each file In Files
		ext = Right(file.Path, 4)
    If InStr(1, strIgnoreExtensions, ext, 1) = 0 And FileIsText(file.Path) Then
      strFile = FileLoadAsString(file.Path)
      p = StringReplace(1, strFile, strMacro, strCopyright, 1)
      If p <> 0 Then
        If Not FileSaveString(file.Path, strFile) Then
            MsgBox "Error saving " & file.Path, 0, "ERROR"
        End If
      Else
        If Len(strNoMacroFileList) = 0 Then 
            strNoMacroFileList = file.Path
        Else
            strNoMacroFileList = strNoMacroFileList & ";" & file.Path
        End If
      End If
    End If
	Next
End Sub

Sub ReplaceAllInFolder(Folder, strMacro, strCopyright, strIgnoreExtensions, strNoMacroFileList)
	If folder.Name <> ".svn" Then 'Skip version control folders
		ReplaceInAllFiles Folder.Files, strMacro, strCopyright, strIgnoreExtensions, strNoMacroFileList
		Dim subfolder
		For Each subfolder in Folder.SubFolders
			ReplaceAllInFolder subfolder, strMacro, strCopyright, strIgnoreExtensions, strNoMacroFileList
		Next
	End If
End Sub

Function Min(a, b)
    If a < b Then
        Min = a
    Else
        Min = b
    End If
End Function

Sub Main()

    Dim FileTypesToIgnore
    FileTypesToIgnore = ".ico .png .bmp .jpg .xml .ini .vcproj .vcxproj .sln .bak .filters .user .props .targets .vbs .cmd .txt .au3"

    Dim vbInformation
    vbInformation = 64

    Dim vbCritical 
    vbCritical = 16

	Dim args
	args = ScriptGetArgumentArray()

	If UBound(args) < 2 Then
		MsgBox "Usage: " & WScript.ScriptName & " <Macro> <ReplacementFile.txt> <RootPath1> <RootPath2> <RootPath...>", vbInformation, "InstallCopyright"
		WScript.Quit 0
	End If

	Dim macro, copyright
	macro = "// $(" & args(0) & ")"
	copyright = FileLoadAsString(args(1))

    if Len(copyright) = 0 Then
        MsgBox "Failed to load copyright """ & args(1) & """", vbCritical, "InstallCopyright: " & args(0)
        WScript.Quit 0
    End If 

	Dim strNoMacroFileList

	Dim fso, root, folder
	Set fso = CreateObject("Scripting.FileSystemObject")

    Dim argI
    For argI = 2 To UBound(args)
        On Error Resume Next
	        Set root = fso.GetFolder(fso.GetAbsolutePathName(args(argI)))
            If root Is Nothing Then
                MsgBox "Failed to find folder """ & args(argI) & """. (this cannot be a filename)", vbCritical, "InstallCopyright: " & args(0)
                WScript.Quit 0
            End If
        On Error Goto 0

        ReplaceAllInFolder root, macro, copyright, FileTypesToIgnore, strNoMacroFileList
    Next

    Dim summary
    summary = "Done inserting copyright."
	
    If Len(strNoMacroFileList) <> 0 Then
        Dim arr
        arr = ArrayFromString(strNoMacroFileList, ";")

        Dim commonLen
        commonLen = StringArrayGetCommonPrefixLength(arr)

        summary = summary & " The following files did not contain the macro """ & args(0) & """:"

        Dim Upper
        Upper = Min(Ubound(arr), 15)

        Dim i
        For i = LBound(arr) To Upper
            summary = summary & vbNewline & Mid(arr(i), commonLen)
        Next

        If Ubound(arr) > 15 Then
            summary = summary & vbNewline & "..."
        End If
    End If
    MsgBox summary, vbInformation, "InstallCopyright: " & args(2)
End Sub

Main
