# vim: noai et ts=4 tw=0
#
# Nullsoft Installer file
# for Tux Paint
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# (See COPYING.txt)
# 
# with a few tiny modifications by Phil Harper(philh@theopencd.org)
#
# Last modified: 2006.02.17
#
# NOTE: Is this file used any more???

!define PKG_VERSION "0.9.18"
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
# DirShow           show
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
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME} (Full Screen).lnk" "$INSTDIR\${APP_EXE}" "-f" "$INSTDIR\${APP_EXE}" 0 "" "" "Start TuxPaint in Fullscreen mode"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\${APP_NAME} (Windowed).lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0 "" "" "Start TuxPaint in a Window"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Readme.lnk" "$INSTDIR\docs\html\README.html" "" "$INSTDIR\docs\html\README.html" 0 "" "" "View ReadMe"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Licence.lnk" "$INSTDIR\docs\copying.txt" "" "$INSTDIR\docs\copying.txt" 0 "" "" "Read License"
  CreateShortCut  "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0 "" "" "Remove TuxPaint"
SectionEnd


Section "Desktop Shortcut"
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0  "" "" "Create a picture with TuxPaint"
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


