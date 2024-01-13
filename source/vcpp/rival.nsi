Name "Rival"

OutFile "rival_20YY_MM_DD_foo_edition_windows.exe"

InstallDir $PROGRAMFILES\Rival

InstallDirRegKey HKLM "Software\Rival" "Install_Dir"

SetCompressor /SOLID lzma
XPStyle on

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Rival (required)"

  SectionIn RO
  
  SetOutPath $INSTDIR
  
  File /r "..\..\*.*"
  
  WriteRegStr HKLM SOFTWARE\Rival "Install_Dir" "$INSTDIR"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rival" "DisplayName" "Rival"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rival" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rival" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rival" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  IfFileExists "$DOCUMENTS\My Games\Rival\config\saved.cfg" ConfigFound NoConfig  
  ConfigFound:
     Delete "$DOCUMENTS\My Games\Rival\config\old-saved.cfg"
     Rename "$DOCUMENTS\My Games\Rival\config\saved.cfg" "$DOCUMENTS\My Games\Rival\config\old-saved.cfg"
  NoConfig:

SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Rival"

  CreateDirectory "$DOCUMENTS\My Games\Rival"
 
  SetOutPath "$INSTDIR"
  
  CreateShortCut "$INSTDIR\Rival.lnk"                "$INSTDIR\rival.bat" "" "$INSTDIR\bin\rival.exe" 0 SW_SHOWMINIMIZED
  CreateShortCut "$SMPROGRAMS\Rival\Rival.lnk"   "$INSTDIR\rival.bat" "" "$INSTDIR\bin\rival.exe" 0 SW_SHOWMINIMIZED
;  CreateShortCut "$SMPROGRAMS\Rival\README.lnk"      "$INSTDIR\README.html"   "" "$INSTDIR\README.html" 0

  CreateShortCut "$INSTDIR\User Data.lnk"                "$DOCUMENTS\My Games\Rival"
  CreateShortCut "$SMPROGRAMS\Rival\User Data.lnk"   "$DOCUMENTS\My Games\Rival"  

  CreateShortCut "$SMPROGRAMS\Rival\Uninstall.lnk"   "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Uninstall"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Rival"
  DeleteRegKey HKLM SOFTWARE\Rival

  RMDir /r "$SMPROGRAMS\Rival"
  RMDir /r "$INSTDIR"

SectionEnd
