# vim: noai et ts=4 tw=0

!define PKG_VERSION "0.9.13"
!define PKG_PREFIX  "tuxpaint"

!define APP_PREFIX  "TuxPaint"
!define APP_EXE     "${APP_PREFIX}.exe"
!define APP_NAME    "Tux Paint"

OutFile     "${PKG_PREFIX}-${PKG_VERSION}-win32-installer.exe"
Name        "${APP_NAME}"
Caption     ""
CRCCheck    on
WindowIcon  off
BGGradient  off

# Default to not silent
SilentInstall   normal
SilentUnInstall normal

# Various default text options
MiscButtonText
InstallButtonText
FileErrorText

# Default installation dir and registry key of install directory
InstallDir  "$PROGRAMFILES\${APP_PREFIX}"
InstallDirRegKey HKLM SOFTWARE\${APP_PREFIX} "Install_Dir"

# Licence text
LicenseText "You must agree to this license before installing ${APP_NAME}"
LicenseData "docs/copying.txt"

# Directory browsing
DirShow             show
ComponentText       "This will install ${APP_NAME} on your computer. Select which optional things you want installed."
DirText             "Choose a directory to install ${APP_NAME} in to:"
AllowRootDirInstall false

# Install page stuff
InstProgressFlags   smooth
AutoCloseWindow     true

Section
  SetOutPath $INSTDIR
  File "visualc\release\${APP_EXE}"
  File "visualc\release\*.dll"
  File "tuxpaint.cfg"
  SetOutPath $INSTDIR\data
  File /r "data\*.*"
  SetOutPath $INSTDIR\docs
  File /r "docs\*.*"
  SetOutPath $INSTDIR\locale
  File /r "locale\*.*"
  SetOutPath $INSTDIR\userdata
  File /r "userdata\*.*"

  WriteRegStr HKLM SOFTWARE\${APP_PREFIX} "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_PREFIX}" "DisplayName" "${APP_NAME} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_PREFIX}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
SectionEnd


Section "Start Menu Shortcuts"
  SetOutPath $INSTDIR
  CreateDirectory "$SMPROGRAMS\${APP_NAME}"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Readme.lnk" "$INSTDIR\docs\html\README.html" "" "$INSTDIR\docs\html\README.html" 0
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Licence.lnk" "$INSTDIR\docs\copying.txt" "" "$INSTDIR\docs\copying.txt" 0
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd


Section "Desktop Shortcut"
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
SectionEnd

Section "View README"
  SetOutPath $INSTDIR
  ExecShell "open" "docs\html\README.html"
SectionEnd

;Function .onInstSuccess
;  BringToFront
;  MessageBox MB_YESNO|MB_ICONQUESTION \
;             "${APP_NAME} was installed. Would you like to run ${APP_NAME} now ?" \
;             IDNO NoExec
;    Exec '$INSTDIR\${APP_EXE}'
;  NoExec:
;FunctionEnd

; uninstall stuff

UninstallText "This will uninstall ${APP_NAME}. Hit 'Uninstall' to continue."

; special uninstall section.
Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_PREFIX}"
  DeleteRegKey HKLM SOFTWARE\${APP_PREFIX}

  RMDir  /r "$INSTDIR\data"
  RMDir  /r "$INSTDIR\docs"
  RMDir  /r "$INSTDIR\locale"
  Delete    "$INSTDIR\*.*"

  Delete "$DESKTOP\${APP_NAME}.lnk"
  Delete "$SMPROGRAMS\${APP_NAME}\*.*"
  RMDir  "$SMPROGRAMS\${APP_NAME}"
SectionEnd


