# Microsoft Developer Studio Project File - Name="map_editor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=map_editor - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "map_editor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "map_editor.mak" CFG="map_editor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "map_editor - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "map_editor - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "map_editor - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /Gi /GX /O2 /D "NDEBUG" /D "WIN32" /D "WINDOWS" /D "MAP_EDITOR" /Fp"Release/elc.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 user32.lib SDL_net.lib kernel32.lib opengl32.lib SDL.lib ALut.lib OpenAL32.lib GLU32.lib comdlg32.lib libxml2.lib /nologo /subsystem:windows /pdb:"Release/el.pdb" /machine:I386 /out:"Release/mapedit.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

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
# ADD CPP /nologo /MDd /Gm /Gi /GX /ZI /Od /D "WIN32" /D "WINDOWS" /D "MAP_EDITOR" /FR /Fp"Debug/elc.pch" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib SDL.lib GLU32.lib comdlg32.lib libxml2.lib iconv.lib /nologo /subsystem:windows /pdb:"Debug/el.pdb" /debug /machine:I386 /out:"../elc/Debug/mapedit.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "map_editor - Win32 Release"
# Name "map_editor - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\2d_objects.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\3d_objects.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\asc.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\browser.c
# End Source File
# Begin Source File

SOURCE=.\colors.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\draw_scene.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\edit_window.c
# End Source File
# Begin Source File

SOURCE=..\elc\elconfig.c
# End Source File
# Begin Source File

SOURCE=.\elwindows.c
# End Source File
# Begin Source File

SOURCE=.\errors.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\events.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\elc\font.c
# End Source File
# Begin Source File

SOURCE=.\frustum.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_init.c
# End Source File
# Begin Source File

SOURCE=.\global.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\init.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interface.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lights.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\map_io.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\misc.c
# End Source File
# Begin Source File

SOURCE=.\o3dow.c
# End Source File
# Begin Source File

SOURCE=..\elc\particles.c
# End Source File
# Begin Source File

SOURCE=.\particles_window.c
# End Source File
# Begin Source File

SOURCE=.\reflection.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\replace_window.c
# End Source File
# Begin Source File

SOURCE=.\shadows.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\elc\textures.c
# End Source File
# Begin Source File

SOURCE=.\tile_map.c

!IF  "$(CFG)" == "map_editor - Win32 Release"

# ADD CPP /D "_CONFIGCP_Win32_20Release" /D "_PRJCP_map_5Feditor_2Evprj"

!ELSEIF  "$(CFG)" == "map_editor - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\elc\translate.c
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

SOURCE=.\asc.h
# End Source File
# Begin Source File

SOURCE=.\browser.h
# End Source File
# Begin Source File

SOURCE=.\draw_scene.h
# End Source File
# Begin Source File

SOURCE=.\e3d.h
# End Source File
# Begin Source File

SOURCE=.\edit_window.h
# End Source File
# Begin Source File

SOURCE=.\editor.h
# End Source File
# Begin Source File

SOURCE=.\errors.h
# End Source File
# Begin Source File

SOURCE=.\events.h
# End Source File
# Begin Source File

SOURCE=.\frustum.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\gui.h
# End Source File
# Begin Source File

SOURCE=.\gui_callbacks.h
# End Source File
# Begin Source File

SOURCE=.\gui_support.h
# End Source File
# Begin Source File

SOURCE=.\height_window.h
# End Source File
# Begin Source File

SOURCE=.\init.h
# End Source File
# Begin Source File

SOURCE=.\interface.h
# End Source File
# Begin Source File

SOURCE=.\lights.h
# End Source File
# Begin Source File

SOURCE=.\map_io.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\o3dow.h
# End Source File
# Begin Source File

SOURCE=..\elc\particles.h
# End Source File
# Begin Source File

SOURCE=.\particles_window.h
# End Source File
# Begin Source File

SOURCE=.\reflection.h
# End Source File
# Begin Source File

SOURCE=.\replace_window.h
# End Source File
# Begin Source File

SOURCE=.\SDL_opengl.h
# End Source File
# Begin Source File

SOURCE=.\shadows.h
# End Source File
# Begin Source File

SOURCE=.\tiles.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\mapeditor.ico
# End Source File
# End Group
# Begin Group "Project Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\eternal_lands_license.txt
# End Source File
# End Group
# End Target
# End Project
