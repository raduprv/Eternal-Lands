# Microsoft Developer Studio Project File - Name="elc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=elc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "elc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "elc.mak" CFG="elc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "elc - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "elc - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "elc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 vorbisfile.lib user32.lib SDL_net.lib kernel32.lib opengl32.lib SDL.lib ALut.lib OpenAL32.lib GLU32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/el.exe"

!ELSEIF  "$(CFG)" == "elc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /Gm /Gi /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "WINDOWS" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libxml2.lib vorbisfile_d.lib user32.lib SDL_net.lib kernel32.lib opengl32.lib SDL.lib ALut.lib OpenAL32.lib GLU32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "elc - Win32 Release"
# Name "elc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\2d_objects.c
# End Source File
# Begin Source File

SOURCE=.\3d_objects.c
# End Source File
# Begin Source File

SOURCE=.\actor_scripts.c
# End Source File
# Begin Source File

SOURCE=.\actors.c
# End Source File
# Begin Source File

SOURCE=.\asc.c
# End Source File
# Begin Source File

SOURCE=.\buddy.c
# End Source File
# Begin Source File

SOURCE=.\cache.c
# End Source File
# Begin Source File

SOURCE=.\colors.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\cursors.c
# End Source File
# Begin Source File

SOURCE=.\dialogues.c
# End Source File
# Begin Source File

SOURCE=.\draw_scene.c
# End Source File
# Begin Source File

SOURCE=.\encyclopedia.c
# End Source File
# Begin Source File

SOURCE=.\errors.c
# End Source File
# Begin Source File

SOURCE=.\events.c
# End Source File
# Begin Source File

SOURCE=.\filter.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\frustum.c
# End Source File
# Begin Source File

SOURCE=.\gl_init.c
# End Source File
# Begin Source File

SOURCE=.\help.c
# End Source File
# Begin Source File

SOURCE=.\hud.c
# End Source File
# Begin Source File

SOURCE=.\ignore.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\interface.c
# End Source File
# Begin Source File

SOURCE=.\items.c
# End Source File
# Begin Source File

SOURCE=.\knowledge.c
# End Source File
# Begin Source File

SOURCE=.\lights.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\manufacture.c
# End Source File
# Begin Source File

SOURCE=.\map_io.c
# End Source File
# Begin Source File

SOURCE=.\md2loader.c
# End Source File
# Begin Source File

SOURCE=.\md5.c
# End Source File
# Begin Source File

SOURCE=.\misc.c
# End Source File
# Begin Source File

SOURCE=.\multiplayer.c
# End Source File
# Begin Source File

SOURCE=.\new_actors.c
# End Source File
# Begin Source File

SOURCE=.\new_character.c
# End Source File
# Begin Source File

SOURCE=.\particles.c
# End Source File
# Begin Source File

SOURCE=.\paste.c
# End Source File
# Begin Source File

SOURCE=.\reflection.c
# End Source File
# Begin Source File

SOURCE=.\shadows.c
# End Source File
# Begin Source File

SOURCE=.\sound.c
# End Source File
# Begin Source File

SOURCE=.\spells.c
# End Source File
# Begin Source File

SOURCE=.\stats.c
# End Source File
# Begin Source File

SOURCE=.\text.c
# End Source File
# Begin Source File

SOURCE=.\textures.c
# End Source File
# Begin Source File

SOURCE=.\tile_map.c
# End Source File
# Begin Source File

SOURCE=.\trade.c
# End Source File
# Begin Source File

SOURCE=.\weather.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\2d_objects.h
# End Source File
# Begin Source File

SOURCE=.\3d_objects.h
# End Source File
# Begin Source File

SOURCE=.\actor_scripts.h
# End Source File
# Begin Source File

SOURCE=.\actors.h
# End Source File
# Begin Source File

SOURCE=.\asc.h
# End Source File
# Begin Source File

SOURCE=.\buddy.h
# End Source File
# Begin Source File

SOURCE=.\cache.h
# End Source File
# Begin Source File

SOURCE=.\client_serv.h
# End Source File
# Begin Source File

SOURCE=.\colors.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\cursors.h
# End Source File
# Begin Source File

SOURCE=.\dialogues.h
# End Source File
# Begin Source File

SOURCE=.\draw_scene.h
# End Source File
# Begin Source File

SOURCE=.\e3d.h
# End Source File
# Begin Source File

SOURCE=.\elc_private.h
# End Source File
# Begin Source File

SOURCE=.\encyclopedia.h
# End Source File
# Begin Source File

SOURCE=.\errors.h
# End Source File
# Begin Source File

SOURCE=.\events.h
# End Source File
# Begin Source File

SOURCE=.\filter.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\gl_init.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\help.h
# End Source File
# Begin Source File

SOURCE=.\hud.h
# End Source File
# Begin Source File

SOURCE=.\ignore.h
# End Source File
# Begin Source File

SOURCE=.\init.h
# End Source File
# Begin Source File

SOURCE=.\interface.h
# End Source File
# Begin Source File

SOURCE=.\items.h
# End Source File
# Begin Source File

SOURCE=.\knowledge.h
# End Source File
# Begin Source File

SOURCE=.\lights.h
# End Source File
# Begin Source File

SOURCE=.\manufacture.h
# End Source File
# Begin Source File

SOURCE=.\map_io.h
# End Source File
# Begin Source File

SOURCE=.\md2.h
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\multiplayer.h
# End Source File
# Begin Source File

SOURCE=.\new_actors.h
# End Source File
# Begin Source File

SOURCE=.\new_character.h
# End Source File
# Begin Source File

SOURCE=.\particles.h
# End Source File
# Begin Source File

SOURCE=.\paste.h
# End Source File
# Begin Source File

SOURCE=.\reflection.h
# End Source File
# Begin Source File

SOURCE=.\SDL_opengl.h
# End Source File
# Begin Source File

SOURCE=.\Server_private.h
# End Source File
# Begin Source File

SOURCE=.\shadows.h
# End Source File
# Begin Source File

SOURCE=.\sound.h
# End Source File
# Begin Source File

SOURCE=.\spells.h
# End Source File
# Begin Source File

SOURCE=.\stats.h
# End Source File
# Begin Source File

SOURCE=.\ter_g_private.h
# End Source File
# Begin Source File

SOURCE=.\text.h
# End Source File
# Begin Source File

SOURCE=.\textures.h
# End Source File
# Begin Source File

SOURCE=.\tiles.h
# End Source File
# Begin Source File

SOURCE=.\trade.h
# End Source File
# Begin Source File

SOURCE=.\weather.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\elc.ico
# End Source File
# Begin Source File

SOURCE=.\eternal.ico
# End Source File
# End Group
# Begin Group "Project Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CHANGES
# End Source File
# Begin Source File

SOURCE=.\eternal_lands_license.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# Begin Source File

SOURCE=.\TODO
# End Source File
# End Group
# End Target
# End Project
