# Microsoft Developer Studio Project File - Name="TuxPaint" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=TuxPaint - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TuxPaint.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TuxPaint.mak" CFG="TuxPaint - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TuxPaint - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "TuxPaint - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TuxPaint - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /O1 /I "..\..\include" /I "..\visualc" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 iconv.lib libpng1.lib libintl.lib sdl.lib sdlmain.lib sdl_mixer.lib sdl_image.lib sdl_ttf.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "TuxPaint - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W4 /Zi /Od /Op /I "..\..\include" /I "..\visualc" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 iconv.lib libpng1.lib libintl.lib sdl.lib sdlmain.lib sdl_mixer.lib sdl_image.lib sdl_ttf.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\libd"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "TuxPaint - Win32 Release"
# Name "TuxPaint - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\hq4x.c
# End Source File
# Begin Source File

SOURCE=..\src\hqxx.c
# End Source File
# Begin Source File

SOURCE=..\src\tuxpaint.c
# End Source File
# Begin Source File

SOURCE=..\src\win32_dirent.c
# End Source File
# Begin Source File

SOURCE=..\src\win32_print.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\src\mouse\arrow-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\arrow.xbm
# End Source File
# Begin Source File

SOURCE=D:\Dx8sdk\include\basetsd.h
# End Source File
# Begin Source File

SOURCE=..\..\include\begin_code.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\brush-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\brush.xbm
# End Source File
# Begin Source File

SOURCE=..\..\include\close_code.h
# End Source File
# Begin Source File

SOURCE=..\src\colors.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\crosshair-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\crosshair.xbm
# End Source File
# Begin Source File

SOURCE="..\src\mouse\down-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\down.xbm
# End Source File
# Begin Source File

SOURCE=..\src\great.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\hand-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\hand.xbm
# End Source File
# Begin Source File

SOURCE=..\src\hq3x.h
# End Source File
# Begin Source File

SOURCE=..\src\hq4x.h
# End Source File
# Begin Source File

SOURCE=..\src\hqxx.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\insertion-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\insertion.xbm
# End Source File
# Begin Source File

SOURCE=..\..\include\libintl.h
# End Source File
# Begin Source File

SOURCE=..\src\magic.h
# End Source File
# Begin Source File

SOURCE=..\..\include\png.h
# End Source File
# Begin Source File

SOURCE=..\..\include\pngconf.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\rotate-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\rotate.xbm
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_active.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_audio.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_byteorder.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_cdrom.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_error.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_events.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_getenv.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_image.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_joystick.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_keyboard.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_keysym.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_main.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_mixer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_mouse.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_quit.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_rwops.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_syswm.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_timer.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_ttf.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_types.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_version.h
# End Source File
# Begin Source File

SOURCE=..\..\include\SDL_video.h
# End Source File
# Begin Source File

SOURCE=..\src\shapes.h
# End Source File
# Begin Source File

SOURCE=..\src\sounds.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\tiny-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\tiny.xbm
# End Source File
# Begin Source File

SOURCE=..\src\tip_tux.h
# End Source File
# Begin Source File

SOURCE=..\src\titles.h
# End Source File
# Begin Source File

SOURCE=..\src\tools.h
# End Source File
# Begin Source File

SOURCE="..\src\mouse\up-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\up.xbm
# End Source File
# Begin Source File

SOURCE="..\src\mouse\wand-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\wand.xbm
# End Source File
# Begin Source File

SOURCE="..\src\mouse\watch-mask.xbm"
# End Source File
# Begin Source File

SOURCE=..\src\mouse\watch.xbm
# End Source File
# Begin Source File

SOURCE=..\src\win32_dirent.h
# End Source File
# Begin Source File

SOURCE=..\src\win32_print.h
# End Source File
# Begin Source File

SOURCE=..\..\include\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\include\zlib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE="..\data\images\icon-win32.ico"
# End Source File
# Begin Source File

SOURCE=.\resources.rc
# End Source File
# End Group
# End Target
# End Project
