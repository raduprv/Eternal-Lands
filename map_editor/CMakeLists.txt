# CMAKE file for Eternal Lands Map Editor
#
# Create a build sub-directory and move into it
# 
# To build a normal release version
#   cmake <path to source>
#
# Build Debug vesion
#   cmake -DCMAKE_BUILD_TYPE=debug <path to source>
#
# To specify a non-default exec name include -DEXEC, for example "-DEXEC=el.mapeditor.$(uname -m).linux.bin"
# To enable local CPU architecture optimisation include "-DMARCHNATIVE=1"
# To enable runtime GCC address sanitization include -DADDRSANITISER=1
#
# Example if located in source directory:
#   mkdir -p build && cd build
#   cmake -DCMAKE_BUILD_TYPE=debug -DEXEC=el.$(uname -m).linux.bin -DMARCHNATIVE=1 -DADDRSANITISER=1 ..
#   make
#
# Example for windows using msys2
#   cmake -G "MSYS Makefiles" -DCMAKE_INCLUDE_PATH="/mingw64/local/include" -DCMAKE_LIBRARY_PATH="/mingw64/local/lib" ..

cmake_minimum_required(VERSION 3.0.2)

project (Eternal-Lands-Map-Editor)

# standard options
add_definitions(
	-DEYE_CANDY						# special effects
	-DCLUSTER_INSIDES				# Group objects into separate clusters for clipping on inside maps
	-DNEW_LIGHT_FORMAT				# ??
	-DZLIB							# use gz() libraries
	-DZLIBW							# use gz() libraries (same ???)
)

# Set the path from the git root to the map editor source files
set(SD "")

# Get compiler flags for used libraries
include(FindPkgConfig)
pkg_check_modules(SDL sdl)
if (NOT SDL_INCLUDE_DIRS)
	include(FindSDL)
endif()
include(FindLibXml2)
include(FindSDL_image)
set(OpenGL_GL_PREFERENCE LEGACY)
include(FindOpenGL)
include(FindZLIB)
include(FindPNG)
include(FindX11)
include(FindGTK2)
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	include(FindIconv)
endif()
include(../cmake/FindCal3d.cmake)

# set the paths to header files
include_directories(${SD}. ${SD}../ ${SD}../io ${SD}../xz ${SD}../engine ${SD}../exceptions ${SD}../eye_candy)

# list the map editor C source files
set(SOURCES
	${SD}2d_objects.c ${SD}3d_objects.c ${SD}browser.c ${SD}draw_scene.c ${SD}e3d.c
	${SD}elwindows.c ${SD}events.c ${SD}font.c ${SD}frustum.c ${SD}gl_init.c ${SD}global.c
	${SD}gui.c ${SD}gui_callbacks.c ${SD}init.c ${SD}interface.c ${SD}lights.c ${SD}main.c
	${SD}map_io.c ${SD}misc.c ${SD}o3dow.c ${SD}${SD}reflection.c ${SD}shadows.c ${SD}tile_map.c
	${SD}replace_window.c ${SD}edit_window.c ${SD}particles_window.c
)

# list the client C source files
set(SOURCES ${SOURCES}
	${SD}../asc.c ${SD}../colors.c ${SD}../elconfig.c ${SD}../errors.c ${SD}../load_gl_extensions.c
	${SD}../md5.c ${SD}../dds.c ${SD}../ddsimage.c ${SD}../el_memory.c ${SD}../particles.c
	${SD}../queue.c ${SD}../textures.c ${SD}../translate.c ${SD}../hash.c ${SD}../image.c
	${SD}../image_loading.c ${SD}../cache.c ${SD}../cluster.c
	${SD}../io/fileutil.c ${SD}../io/e3d_io.c ${SD}../io/elc_io.c ${SD}../io/elpathwrapper.c
	${SD}../io/half.c ${SD}../io/normal.c ${SD}../io/elfilewrapper.c ${SD}../io/unzip.c
	${SD}../io/ioapi.c ${SD}../io/zip.c ${SD}../io/ziputil.c
	${SD}../xz/7zCrc.c ${SD}../xz/7zCrcOpt.c ${SD}../xz/Alloc.c ${SD}../xz/Bra86.c ${SD}../xz/Bra.c
	${SD}../xz/BraIA64.c ${SD}../xz/CpuArch.c ${SD}../xz/Delta.c ${SD}../xz/LzFind.c
	${SD}../xz/Lzma2Dec.c ${SD}../xz/Lzma2Enc.c ${SD}../xz/LzmaDec.c ${SD}../xz/LzmaEnc.c
	${SD}../xz/Sha256.c ${SD}../xz/Xz.c ${SD}../xz/XzCrc64.c ${SD}../xz/XzDec.c ${SD}../xz/XzEnc.c
)

# list the map editor C++ source files
set(SOURCES ${SOURCES}
	${SD}eye_candy_window.cpp
)

# list the client C++ source files
set(SOURCES ${SOURCES}
	${SD}../elloggingwrapper.cpp ${SD}../cal3d_wrapper.cpp
	${SD}../eye_candy_wrapper.cpp
	${SD}../engine/hardwarebuffer.cpp ${SD}../engine/logging.cpp
	${SD}../exceptions/extendedexception.cpp
	${SD}../eye_candy/effect_bag.cpp ${SD}../eye_candy/effect_breath.cpp
	${SD}../eye_candy/effect_campfire.cpp ${SD}../eye_candy/effect_cloud.cpp
	${SD}../eye_candy/effect_candle.cpp ${SD}../eye_candy/effect_firefly.cpp
	${SD}../eye_candy/effect_fountain.cpp ${SD}../eye_candy/effect_glow.cpp
	${SD}../eye_candy/effect_harvesting.cpp ${SD}../eye_candy/effect_impact.cpp
	${SD}../eye_candy/effect_lamp.cpp ${SD}../eye_candy/effect_ongoing.cpp
	${SD}../eye_candy/effect_selfmagic.cpp ${SD}../eye_candy/effect_smoke.cpp
	${SD}../eye_candy/effect_summon.cpp ${SD}../eye_candy/effect_sword.cpp
	${SD}../eye_candy/effect_targetmagic.cpp ${SD}../eye_candy/effect_teleporter.cpp
	${SD}../eye_candy/effect_wind.cpp ${SD}../eye_candy/eye_candy.cpp
	${SD}../eye_candy/math_cache.cpp
)

# if enabled, build a exec optomised for the hosts CPU architecture, this is not portable
if (MARCHNATIVE)
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -march=native")
	set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -march=native")
endif()

# if enabled, include the gcc address sanitisation options, please report andthing you find
if (ADDRSANITISER)
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=address")
	set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -fsanitize=address")
	set(EXTRA_LD_FLAGS "${EXTRA_LD_FLAGS} -fsanitize=address")
endif()

# set the compiler flags for DEBUG and RELEASE builds
set(CMAKE_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Wdeclaration-after-statement -D_GNU_SOURCE=1 -D_REENTRANT")
set(CMAKE_C_FLAGS_DEBUG "-Og -ggdb -pipe -fno-strict-aliasing")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fomit-frame-pointer -ffast-math -pipe -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wall -D_GNU_SOURCE=1 -D_REENTRANT")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb -pipe -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer -ffast-math -pipe -fno-strict-aliasing")

# set the OS level defines and default executable names
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	if (NOT EXEC)
		set(EXEC "mapedit.linux.bin")
	endif()
	add_definitions(-DLINUX)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	if (NOT EXEC)
		set(EXEC "mapedit")
	endif()
	add_definitions(-DWINDOWS -DWINVER=0x500 -mwindows -DNDEBUG)
	set(EXTRA_LD_FLAGS "${EXTRA_LD_FLAGS} -mwindows")
	set(SOURCES ${SOURCES} ${SD}mapedit.rc)
	set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
	set(GTK2_GDKCONFIG_INCLUDE_DIR /mingw64/lib/gtk-2.0/include)
	set(GTK2_GLIBCONFIG_INCLUDE_DIR /mingw64/lib/glib-2.0/include)
	set(GTK2_LIBRARIES ${GTK2_LIBRARIES} ${GTK2_ATK_LIBRARY}
		${GTK2_CAIRO_LIBRARY} ${GTK2_FONTCONFIG_LIBRARY}
		${GTK2_GDKCONFIG_LIBRARY} ${GTK2_GDK_LIBRARY}
		${GTK2_GDK_PIXBUF_LIBRARY} ${GTK2_GLIBCONFIG_LIBRARY}
		${GTK2_GLIB_LIBRARY} ${GTK2_GOBJECT_LIBRARY} ${GTK2_GTK_LIBRARY}
		${GTK2_HARFBUZZ_LIBRARY} ${GTK2_PANGOCAIRO_LIBRARY}
		${GTK2_PANGO_LIBRARY} ${GTK2_X11_LIBRARY})
	set(GTK2_INCLUDE_DIRS ${GTK2_INCLUDE_DIRS} ${GTK2_ATK_INCLUDE_DIR}
		${GTK2_CAIRO_INCLUDE_DIR} ${GTK2_FONTCONFIG_INCLUDE_DIR}
		${GTK2_GDKCONFIG_INCLUDE_DIR} ${GTK2_GDK_INCLUDE_DIR}
		${GTK2_GDK_PIXBUF_INCLUDE_DIR} ${GTK2_GLIBCONFIG_INCLUDE_DIR}
		${GTK2_GLIB_INCLUDE_DIR} ${GTK2_GOBJECT_INCLUDE_DIR}
		${GTK2_GTK_INCLUDE_DIR} ${GTK2_HARFBUZZ_INCLUDE_DIR}
		${GTK2_PANGOCAIRO_INCLUDE_DIR} ${GTK2_PANGO_INCLUDE_DIR}
		${GTK2_X11_INCLUDE_DIR})
else()
	message( FATAL_ERROR "Unknown platform [${CMAKE_SYSTEM_NAME}]" )
endif()

# any final defines needed
add_definitions(-DMAP_EDITOR -D_7ZIP_ST -DGTK2 ${GTK2_DEFINITIONS})

# default to building a release version
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE release)
endif()

# clean up extra strings to avoid policy CMP0004
string(STRIP "${EXTRA_C_FLAGS}" EXTRA_C_FLAGS)
string(STRIP "${EXTRA_CXX_FLAGS}" EXTRA_CXX_FLAGS)
string(STRIP "${EXTRA_LD_FLAGS}" EXTRA_LD_FLAGS)

# build

add_executable(${EXEC} ${SOURCES})

target_link_libraries(${EXEC}
	${EXTRA_LD_FLAGS}
	${SDL_LIBRARY} ${SDL_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${CAL3D_LIBRARIES}
	${SDL_IMAGE_LIBRARY}
	${OPENGL_LIBRARIES}
	${PNG_LIBRARIES}
	${X11_LIBRARIES}
	${GTK2_LIBRARIES}
	${Iconv_LIBRARIES}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${STATIC_LIBRARIES}
)

target_include_directories(${EXEC} SYSTEM PUBLIC
	${SDL_INCLUDE_DIR} ${SDL_INCLUDE_DIRS}
	${LIBXML2_INCLUDE_DIR}
	${CAL3D_INCLUDE_DIR}
	${SDL_IMAGE_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${X11_INCLUDE_DIR}
	${GTK2_INCLUDE_DIRS}
	${Iconv_INCLUDE_DIRS}
)
