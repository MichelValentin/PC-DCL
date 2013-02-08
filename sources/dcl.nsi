Name "PC-DCL"
Caption "PC-DCL V4.07 - Setup"
OutFile ..\bin\PCDCL_Setup_V407.exe

#BGGradient 000000 800000 FFFFFF
#InstallColors FF8080 000000

LicenseText "You must read the following license before installing:"
LicenseData ..\license.txt
ComponentText "This will install PC-DCL on your computer:"
InstType Normal
AutoCloseWindow false
ShowInstDetails show
DirText "Please select a location to install PC-DCL (or use the default):"
SetOverwrite on
SetDateSave on

InstallDir C:\PCDCL

Section "PC-DCL (required)"
  SectionIn 1 2
  SetOutPath $INSTDIR
  File ..\dcl2.exe
  File ..\dcl.ini
  File ..\license.txt
  File ..\changes.txt
  File ..\login.dcl
  File ..\logout.dcl
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PC-DCL" "DisplayName" "PC-DCL V4 (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PC-DCL" "UninstallString" '"$INSTDIR\uninst.exe"'
  WriteUninstaller uninst.exe
SectionEnd

Section "-PC-DCL Help (required)"
  SectionIn 1 2
  SetOutPath $INSTDIR\Help
  File ..\Help\*.hlp
SectionEnd

Section "PC-DCL Examples"
  SectionIn 1 2
  SetOutPath $INSTDIR\Examples
  File ..\Examples\*.dcl
SectionEnd

Section "PC-DCL Start Menu Group"
  SectionIn 1 2
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\PC-DCL"
  CreateShortCut "$SMPROGRAMS\PC-DCL\PC-DCL.lnk" \
                 "$INSTDIR\dcl2.exe"
  CreateShortCut "$SMPROGRAMS\PC-DCL\Uninstall PC-DCL.lnk" \
                 "$INSTDIR\uninst.exe"
SectionEnd

Section "PC-DCL Desktop Icon"
  SectionIn 1 2
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\PC-DCL.lnk" "$INSTDIR\dcl2.exe"
SectionEnd

Section "PC-DCL Shell Extensions"
  SectionIn 1 2

  WriteRegStr HKCR ".dcl" "" "PC-DCLFile"
  WriteRegStr HKCR "PC-DCLFile" "" "PC-DCL Script File"
  WriteRegStr HKCR "PC-DCLFile\shell" "" "open"
  WriteRegStr HKCR "PC-DCLFile\DefaultIcon" "" $INSTDIR\dcl2.exe,0
  WriteRegStr HKCR "PC-DCLFile\shell\open\command" "" '"$INSTDIR\dcl2.exe" "@%1"'
SectionEnd


UninstallText "This will uninstall PC-DCL from your system:"

Section Uninstall
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\PC-DCL"
  DeleteRegKey HKCR ".dcl"
  DeleteRegKey HKCR "PC-DCLFile"
  Delete $SMPROGRAMS\PC-DCL\*.lnk
  RMDir $SMPROGRAMS\PC-DCL
  Delete $DESKTOP\PC-DCL.lnk
  Delete $INSTDIR\dcl2.exe
  Delete $INSTDIR\dcl.ini
  Delete $INSTDIR\license.txt
  Delete $INSTDIR\login.dcl
  Delete $INSTDIR\logout.dcl
  Delete $INSTDIR\uninst.exe
  ; Uninstall Example files-----
  Delete $INSTDIR\Examples\*.dcl
  RMDir /r $INSTDIR\Examples
  ; Uninstall help files--------
  Delete $INSTDIR\Help\*.htm
  RMDir /r $INSTDIR\Help
  RMDir $INSTDIR

  ; if $INSTDIR was removed, skip these next ones
  IfFileExists $INSTDIR 0 Removed
    MessageBox MB_YESNO|MB_ICONQUESTION \
      "Remove all files in your PC-DCL directory? (If you have anything \
 you created that you want to keep, click No)" IDNO Removed
    Delete $INSTDIR\*.* ; this would be skipped if the user hits no
    RMDir /r $INSTDIR
    IfFileExists $INSTDIR 0 Removed
      MessageBox MB_OK|MB_ICONEXCLAMATION \
                 "Note: $INSTDIR could not be removed."
  Removed:
SectionEnd
