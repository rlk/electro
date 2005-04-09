# Microsoft Developer Studio Project File - Name="electro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=electro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "electro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "electro.mak" CFG="electro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "electro - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "electro - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "electro - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glu32.lib opengl32.lib ws2_32.lib SDLmain.lib SDL.lib libpng.lib zlib.lib lua.lib lualib.lib vorbisfile_static.lib ogg_static.lib vorbis_static.lib libjpeg.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "electro - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glu32.lib opengl32.lib ws2_32.lib SDL.lib SDLmain.lib libpng.lib zlib.lib lua.lib lualib.lib vorbis_static.lib vorbisfile_static.lib ogg_static.lib libjpeg.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "electro - Win32 Release"
# Name "electro - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\buffer.c
# End Source File
# Begin Source File

SOURCE=..\src\camera.c
# End Source File
# Begin Source File

SOURCE=..\src\client.c
# End Source File
# Begin Source File

SOURCE=..\src\console.c
# End Source File
# Begin Source File

SOURCE=..\src\display.c
# End Source File
# Begin Source File

SOURCE=..\src\entity.c
# End Source File
# Begin Source File

SOURCE=..\src\frustum.c
# End Source File
# Begin Source File

SOURCE=..\src\galaxy.c
# End Source File
# Begin Source File

SOURCE=..\src\glyph.c
# End Source File
# Begin Source File

SOURCE=..\src\image.c
# End Source File
# Begin Source File

SOURCE=..\src\joystick.c
# End Source File
# Begin Source File

SOURCE=..\src\light.c
# End Source File
# Begin Source File

SOURCE=..\src\main.c
# End Source File
# Begin Source File

SOURCE=..\src\matrix.c
# End Source File
# Begin Source File

SOURCE=..\src\node.c
# End Source File
# Begin Source File

SOURCE=..\src\object.c
# End Source File
# Begin Source File

SOURCE=..\src\opengl.c
# End Source File
# Begin Source File

SOURCE=..\src\pivot.c
# End Source File
# Begin Source File

SOURCE=..\src\script.c
# End Source File
# Begin Source File

SOURCE=..\src\server.c
# End Source File
# Begin Source File

SOURCE=..\src\sound.c
# End Source File
# Begin Source File

SOURCE=..\src\sprite.c
# End Source File
# Begin Source File

SOURCE=..\src\star.c
# End Source File
# Begin Source File

SOURCE=..\src\tracker.c
# End Source File
# Begin Source File

SOURCE=..\src\utility.c
# End Source File
# Begin Source File

SOURCE=..\src\version.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\buffer.h
# End Source File
# Begin Source File

SOURCE=..\src\camera.h
# End Source File
# Begin Source File

SOURCE=..\src\client.h
# End Source File
# Begin Source File

SOURCE=..\src\console.h
# End Source File
# Begin Source File

SOURCE=..\src\display.h
# End Source File
# Begin Source File

SOURCE=..\src\entity.h
# End Source File
# Begin Source File

SOURCE=..\src\event.h
# End Source File
# Begin Source File

SOURCE=..\src\frustum.h
# End Source File
# Begin Source File

SOURCE=..\src\galaxy.h
# End Source File
# Begin Source File

SOURCE=..\src\glyph.h
# End Source File
# Begin Source File

SOURCE=..\src\image.h
# End Source File
# Begin Source File

SOURCE=..\src\joystick.h
# End Source File
# Begin Source File

SOURCE=..\src\light.h
# End Source File
# Begin Source File

SOURCE=..\src\matrix.h
# End Source File
# Begin Source File

SOURCE=..\src\node.h
# End Source File
# Begin Source File

SOURCE=..\src\object.h
# End Source File
# Begin Source File

SOURCE=..\src\opengl.h
# End Source File
# Begin Source File

SOURCE=..\src\pivot.h
# End Source File
# Begin Source File

SOURCE=..\src\script.h
# End Source File
# Begin Source File

SOURCE=..\src\server.h
# End Source File
# Begin Source File

SOURCE=..\src\sound.h
# End Source File
# Begin Source File

SOURCE=..\src\sprite.h
# End Source File
# Begin Source File

SOURCE=..\src\star.h
# End Source File
# Begin Source File

SOURCE=..\src\tracker.h
# End Source File
# Begin Source File

SOURCE=..\src\utility.h
# End Source File
# Begin Source File

SOURCE=..\src\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
