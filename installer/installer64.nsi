;!include "FileAssociation.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

;--------------------------------

; The name of the installer
Name "IPTVUtils"
Caption "IPTVUtils"
Icon "icons\logo.ico"

; The file to write
OutFile "IPTVUtilsInstaller.x64.exe"

; The default installation directory
InstallDir "$PROGRAMFILES64\IPTVUtils"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\IPTVUtils" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; 64 bit program using 64 bit registry
Function .onInit
  SetRegView 64
FunctionEnd

; The stuff to install
Section "IPTVUtils (required)"

  SectionIn RO
  SetRegView 64
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  File "Prerequisites\WinPcap.exe"
  File "Prerequisites\vc_redist-2017.x64.exe"  
  
  ; Check if MSVC Redist is installed and version is not super old, request to install
  ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{E512788E-C50B-3858-A4B9-73AD5F3F9E93}" "VersionMajor"
  ReadRegStr $1 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{E512788E-C50B-3858-A4B9-73AD5F3F9E93}" "VersionMinor"
  ${IF} $0 != "14"
    ${IF} $1 != "10"
      MessageBox MB_OKCANCEL "Microsoft Visual C++ Runtime 2017 is required to use IPTVUtils. Press Ok to install Microsoft Visual C++ Runtime 2017 or Cancel to exit installation." IDYES install_msvcr IDCANCEL quit_msvcr
      install_msvcr:
        ExecWait "$INSTDIR\vc_redist-2017.x64.exe /install /norestart"
        Goto msvcr_installed
      quit_msvcr:
        Quit
    ${ENDIF}
  ${ENDIF}
  msvcr_installed:
  

  ; Check if WinPcap is installed and version is not super old, request to install WinPcap
  ReadRegStr $0 HKLM32 "Software\Microsoft\Windows\CurrentVersion\Uninstall\WinPcapInst" "VersionMajor"
  ${IF} $0 != "4"
    MessageBox MB_OKCANCEL "WinPcap is required to use IPTVUtils. Press Ok to install WinPcap or Cancel to exit installation." IDYES install IDCANCEL quit
    install:
      ExecWait "$INSTDIR\WinPcap.exe"
      Goto pcap_installed
    quit:
      Quit
  ${ENDIF}
  pcap_installed:
  
  Delete "$INSTDIR\WinPcap.exe"
  Delete "$INSTDIR\vc_redist-2017.x64.exe"
  
  ; Put file there
  File /r "artefacts-win64\*"
  File "icons\logo.ico"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\IPTVUtils" "Install_Dir" "$INSTDIR"
  
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "DisplayName" "IPTVUtils"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "Publisher" "WISI Norden"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "DisplayVersion" "0.6"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "EstimatedSize" $0
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "DisplayIcon" "$INSTDIR\logo.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\IPTVUtils"
  CreateShortcut "$SMPROGRAMS\IPTVUtils\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\IPTVUtils\IPTVUtils.lnk" "$INSTDIR\IPTVUtils.exe" "" "$INSTDIR\IPTVUtils.exe" 0
  
SectionEnd

; Option section for file extensions
;Section "Register file extension (*.pcap)"
;
; ${registerExtension} "$INSTDIR\IPTVUtils.exe" ".pcap" "IPTVUtils recording"
; 
;SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  SetRegView 64
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\IPTVUtils"
  
  DeleteRegKey HKLM "Software\IPTVUtils"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\IPTVUtils\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\IPTVUtils"
  RMDir /r "$INSTDIR"
  
  ; Remove registered file extensions
  ;${unregisterExtension} ".pcap" "IPTVUtils recording"

SectionEnd
