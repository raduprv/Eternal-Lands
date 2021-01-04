# CMAKE file for Eternal Lands
#
# Create a build sub-directory and move into it
#
# To build a normal release version
#   cmake <path to source>
#
# Build Debug vesion
#   cmake -DCMAKE_BUILD_TYPE=debug <path to source>
#
# To specify a non-default exec name include -DEXEC, for example "-DEXEC=el.$(uname -m).linux.bin"
# To enable local CPU architecture optimisation include "-DMARCHNATIVE=On"
# To enable runtime GCC address sanitization include -DADDRSANITISER=On
#
# Example if located in source directory:
#   mkdir -p build && cd build
#   cmake -DCMAKE_BUILD_TYPE=debug -DEXEC=el.$(uname -m).linux.bin -DMARCHNATIVE=On -DADDRSANITISER=On ..
#   make
#
# Example for windows using msys2
#   cmake -G "MSYS Makefiles" -DCMAKE_INCLUDE_PATH="/mingw64/local/include" -DCMAKE_LIBRARY_PATH="/mingw64/local/lib" ..

cmake_minimum_required(VERSION 3.5.0)

project (Eternal-Lands)

# standard options, some that also set source files to build
OPTION(CUSTOM_UPDATE "Allows autoupdating of custom look information" On)
OPTION(FSAA "Adds support for full screen anti-aliasing using OpenGL multisampling." On)
OPTION(JSON_FILES "Switch existing binary user config files to JSON format." On)
OPTION(LOCAL_NLOHMANN_JSON "Enable if you want to use an embedded copy of nlohmann_json, placed in the nlohmann_json sub-directory of the source." Off)
OPTION(MARCHNATIVE "Enable -march=native compiler flag, for when not building a portable executable." Off)
OPTION(ADDRSANITISER "Enable the gcc address sanitisation options to do run-time checking." Off)
OPTION(TTF "Add the ability to use True Type system fonts." On)

# standard options
add_definitions(
	-DCLUSTER_INSIDES				# Group objects into separate clusters for clipping on inside maps
	-DCUSTOM_LOOK					# allows you to customize your look.
	-DFUZZY_PATHS					# Makes Tab Map walking not always follow exaclty the same path
	-DNEW_SOUND					# Enables extended sound effects system
	-DPNG_SCREENSHOT				# make screenshots in W3C's PNG format in game, requires libpng
	-DUSE_INLINE					# enable some optimizations to use inline functions instead of pure function calls
	# new for 1.9.0 release
	-DBANDWIDTH_SAVINGS				# enable bandwidth saving changes for message protocol
	# new for 1.9.2 release
	-DANIMATION_SCALING				# Adds support for animation scaling, depending on the number of animation commands in the actor queue.
	# new for 1.9.3 release
	-DFASTER_MAP_LOAD				# Enables optimizations to speed up the process of switching between maps
	-DFASTER_STARTUP				# An attempt to reduce the startup time of EL somewhat
	-DMIDDLE_MOUSE_PASTE			# Paste using the middle mouse button from the primary buffer on X systems
	# new for 1.9.4 release
	-DNEW_EYES						# Support for different eye colors, needs new data files
	# proposed for next release
	-DNEW_CURSOR					# New coloured cursors made by Emajekral (Experimental).  An old unused feature. Derive cursor2.dss from http://users.on.net/~gingerman/sky_cursor-textures.zip (cursor2.bmp + alpha).
)


# Testing options (these options are possible for the a future release and should be tested)
add_definitions(
#	-DANTI_ALIAS					# allows to enable/disable anti-aliasing in the ini file
#	-DDYNAMIC_ANIMATIONS			# (appears broken) Synchronizes animation to FPS instead of a fixed timer
#	_DEXT_ACTOR_DICT				# Removes remaining hard-coded actor def dictionaries - requires updated actor defs files (http://el.grug.redirectme.net/actor_defs.zip)
#	-DNEW_ALPHA						# (undocumented)
#	-DUSE_SIMD						# Enables usage of simd instructions
)

# Machine specific options (fixes or performance enhancements)
add_definitions(
#	-DEL_BIG_ENDIAN					# Enable big-endian code (automatically detected for OSX)
#	-DNO_PF_MACRO					# Use a function instead of a macro for pf_get_tile()
#	-DSIMPLE_LOD					# enable a simplistic distance culling to improve performance
#	-DSOUND_FORK_BUGFIX				# needed for current linux static build to prevent crash when forking in url.c after F2 press and sound/music enabled
#	-DUSE_ALGETSOURCEI_AL_BUFFER	# disable to workaround issue with newer openal versions (also works on older versions), enable to restore original usage of alGetSourcei(pSource->source, AL_BUFFER, &buffer) for sound
)

# Debug options
add_definitions(
#	-DCONTEXT_MENUS_TEST			# Enable "#cmtest" command to help test/demo the context menu code
#	-DDEBUG							# (undocumented)
#	-DDEBUG_XML						# Enables missing (optional) XML string property messages
#	-DDEBUG_POINT_PARTICLES			# (undocumented)
#	-DDEBUG_TIME					# Fakes the acceleration of time for use in debugging shadows and the like.
#	-DDEBUG_MAP_SOUND				# Adds (some??) map boundary areas to tab maps and additional logging to stdout
#	-DECDEBUGWIN					# press ctrl-alt-c to open the Eye Candy debug window: trigger most effects without casting spells or server side events
#	-DEXTRA_DEBUG					# (undocumented)
#	-DMEMORY_DEBUG					# gather information about memory allocation and freeing
#	-DMISSILES_DEBUG				# Enables debug for missiles feature. It will create a file missiles_log.txt file in your settings directory.
#	-DMUTEX_DEBUG					# (undocumented)
#	-DOPENGL_TRACE					# make far more frequent checks for OpenGL errors (requires -DDEBUG to be of any use). Will make error_log.txt a lot larger.
#	-DTIMER_CHECK					# (undocumented)
#	-D_EXTRA_SOUND_DEBUG			# Enable debug for sound effects
)

# Other options (Experimental, unfinished, defunct or otherwise unknown)
add_definitions(
#	-DPAWN							# Experimental, not for release, will need server support to function properly. This *will* eat your cat. You've been warned. Enables the Pawn abstract machine.
#	-DUID							# use unique ID sent from server for custom looks. (INCOMPLETE)
#	-DUSE_ACTORS_OPTIMIZER			# Enables actor optimizations
#	-DUSE_BOOST
#	-DUSE_TR1						# Needs tr1 version of c++ stl, used for NEW_FILE_IO
#	-DWRITE_XML						# dangerous to use, will wipe out some XML files that are part of the main game download
#	-DZLIBW							# Enables being able to write gzip compressed files, requires -lzlib/-lz needed for map_editor
)

# Set the path from the git root to the sours files
set(SD "")

# Get compiler flags for used libraries
include(cmake/FindSDL2.cmake)
include(cmake/FindSDL2_image.cmake)
include(cmake/FindSDL2_net.cmake)
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(LibXml2 REQUIRED)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	find_package(X11 REQUIRED)
endif()
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	find_package(Iconv REQUIRED)
endif()
# these definitions were re-used from Xapher's opengl client
include(cmake/FindVorbis.cmake)
include(cmake/FindVorbisFile.cmake)
include(cmake/FindCal3d.cmake)
if (TTF)
	include(cmake/FindSDL2TTF.cmake)
endif()

# require the nlohmann_json package if building for json files
if (JSON_FILES)
	# if enabled use a local copy of the package
	if (LOCAL_NLOHMANN_JSON)
		set(JSON_BuildTests OFF CACHE INTERNAL "")
		set(JSON_Install OFF CACHE INTERNAL "")
		add_subdirectory(nlohmann_json)
	else()
		find_package(nlohmann_json REQUIRED)
	endif()
endif()

# set the paths to header files
include_directories(${SD} ${SD}io ${SD}shader ${SD}xz ${SD}engine ${SD}exceptions ${SD}xml ${SD}eye_candy)

# list the C source files
set(SOURCES
	${SD}2d_objects.c ${SD}3d_objects.c ${SD}actors.c ${SD}actor_scripts.c ${SD}alphamap.c ${SD}asc.c
	${SD}astrology.c ${SD}bags.c ${SD}bbox_tree.c ${SD}buddy.c ${SD}buffs.c ${SD}cache.c ${SD}cal.c
	${SD}calc.c ${SD}chat.c ${SD}cluster.c ${SD}colors.c ${SD}console.c ${SD}consolewin.c ${SD}counters.c
	${SD}cursors.c ${SD}dds.c ${SD}ddsimage.c ${SD}dialogues.c ${SD}draw_scene.c ${SD}elconfig.c ${SD}el_memory.c
	${SD}elwindows.c ${SD}emotes.c ${SD}encyclopedia.c ${SD}errors.c ${SD}events.c ${SD}eye_candy_debugwin.c
	${SD}filter.c ${SD}framebuffer.c ${SD}frustum.c ${SD}gamewin.c ${SD}gl_init.c ${SD}hash.c ${SD}help.c
	${SD}highlight.c ${SD}hud.c ${SD}hud_misc_window.c ${SD}hud_quickbar_window.c ${SD}hud_quickspells_window.c
	${SD}hud_statsbar_window.c ${SD}ignore.c ${SD}image.c ${SD}image_loading.c ${SD}init.c ${SD}interface.c
	${SD}items.c ${SD}keys.c ${SD}knowledge.c ${SD}langselwin.c ${SD}lights.c ${SD}list.c ${SD}load_gl_extensions.c
	${SD}loading_win.c ${SD}loginwin.c ${SD}main.c ${SD}makeargv.c ${SD}manufacture.c ${SD}map.c ${SD}mapwin.c
	${SD}md5.c ${SD}mines.c ${SD}minimap.c ${SD}misc.c ${SD}missiles.c ${SD}multiplayer.c ${SD}new_actors.c
	${SD}new_character.c ${SD}notepad.c ${SD}openingwin.c ${SD}particles.c ${SD}paste.c ${SD}pathfinder.c
	${SD}pm_log.c ${SD}popup.c ${SD}queue.c ${SD}reflection.c ${SD}rules.c ${SD}serverpopup.c ${SD}servers.c
	${SD}session.c ${SD}shader/noise.c ${SD}shader/shader.c ${SD}shadows.c ${SD}skeletons.c ${SD}skills.c ${SD}sky.c
	${SD}sound.c ${SD}special_effects.c ${SD}spells.c ${SD}stats.c ${SD}storage.c ${SD}tabs.c ${SD}text_aliases.c
	${SD}text.c ${SD}textures.c ${SD}tile_map.c ${SD}timers.c ${SD}trade.c ${SD}translate.c ${SD}update.c ${SD}url.c
	${SD}weather.c ${SD}widgets.c ${SD}io/e3d_io.c ${SD}io/elc_io.c ${SD}io/elfilewrapper.c ${SD}io/elpathwrapper.c
	${SD}io/fileutil.c ${SD}io/half.c ${SD}io/ioapi.c ${SD}io/map_io.c ${SD}io/normal.c ${SD}io/unzip.c
	${SD}io/xmlcallbacks.c ${SD}io/zip.c ${SD}io/ziputil.c ${SD}xz/7zCrc.c ${SD}xz/7zCrcOpt.c ${SD}xz/Alloc.c
	${SD}xz/Bra86.c ${SD}xz/Bra.c ${SD}xz/BraIA64.c ${SD}xz/CpuArch.c ${SD}xz/Delta.c ${SD}xz/LzFind.c
	${SD}xz/Lzma2Dec.c ${SD}xz/Lzma2Enc.c ${SD}xz/LzmaDec.c ${SD}xz/LzmaEnc.c ${SD}xz/Sha256.c ${SD}xz/Xz.c
	${SD}xz/XzCrc64.c ${SD}xz/XzDec.c ${SD}xz/XzEnc.c
)

# list the C++ source files
set(SOURCES ${SOURCES}
	${SD}achievements.cpp ${SD}actor_init.cpp  ${SD}books.cpp ${SD}cal3d_wrapper.cpp ${SD}command_queue.cpp ${SD}context_menu.cpp
	${SD}elloggingwrapper.cpp  ${SD}font.cpp ${SD}hud_indicators.cpp ${SD}hud_timer.cpp ${SD}icon_window.cpp
	${SD}io/cal3d_io_wrapper.cpp ${SD}item_info.cpp ${SD}item_lists.cpp ${SD}named_colours.cpp
	${SD}optimizer.cpp ${SD}password_manager.cpp ${SD}quest_log.cpp ${SD}select.cpp ${SD}sendvideoinfo.cpp
	${SD}trade_log.cpp ${SD}user_menus.cpp ${SD}xor_cipher.cpp ${SD}engine/hardwarebuffer.cpp ${SD}engine/logging.cpp
	${SD}exceptions/extendedexception.cpp ${SD}eye_candy/effect_bag.cpp ${SD}eye_candy/effect_breath.cpp
	${SD}eye_candy/effect_campfire.cpp ${SD}eye_candy/effect_candle.cpp ${SD}eye_candy/effect_cloud.cpp
	${SD}eye_candy/effect_firefly.cpp ${SD}eye_candy/effect_fountain.cpp ${SD}eye_candy/effect_glow.cpp
	${SD}eye_candy/effect_harvesting.cpp ${SD}eye_candy/effect_impact.cpp ${SD}eye_candy/effect_lamp.cpp
	${SD}eye_candy/effect_mines.cpp ${SD}eye_candy/effect_missile.cpp ${SD}eye_candy/effect_ongoing.cpp
	${SD}eye_candy/effect_selfmagic.cpp ${SD}eye_candy/effect_smoke.cpp ${SD}eye_candy/effect_staff.cpp
	${SD}eye_candy/effect_summon.cpp ${SD}eye_candy/effect_sword.cpp ${SD}eye_candy/effect_targetmagic.cpp
	${SD}eye_candy/effect_teleporter.cpp ${SD}eye_candy/effect_wind.cpp ${SD}eye_candy/eye_candy.cpp
	${SD}eye_candy/kepler_orbit.cpp ${SD}eye_candy/math_cache.cpp ${SD}eye_candy/orbital_mover.cpp
	${SD}eye_candy_wrapper.cpp ${SD}xml/xmlhelper.cpp
)

# if CUSTOM_UPDATE wanted add the define and additional source modules
if (CUSTOM_UPDATE)
	add_definitions(-DCUSTOM_UPDATE)
	set(SOURCES ${SOURCES} ${SD}custom_update.c ${SD}new_update.c)
endif()

# if JSON_FILES wanted add the define and additional source modules
if (JSON_FILES)
	add_definitions(-DJSON_FILES)
	set(SOURCES ${SOURCES} ${SD}json_io.cpp )
endif()

# if FSAA wanted add the define and additional source modules
if (FSAA)
	add_definitions(-DFSAA)
	if (CMAKE_SYSTEM_NAME MATCHES "Windows")
		set(SOURCES ${SOURCES} ${SD}fsaa/fsaa_wgl.c ${SD}fsaa/fsaa.c)
	else()
		set(SOURCES ${SOURCES} ${SD}fsaa/fsaa_glx.c ${SD}fsaa/fsaa.c)
	endif()
endif()

# if enabled, build a exec optomised for the hosts CPU architecture, this is not portable
if (MARCHNATIVE)
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -march=native")
	set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -march=native")
endif()

# if enabled, include the gcc address sanitisation options, please report anything you find
if (ADDRSANITISER)
	set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -fsanitize=address")
	set(EXTRA_CXX_FLAGS "${EXTRA_CXX_FLAGS} -fsanitize=address")
	set(EXTRA_LD_FLAGS "${EXTRA_LD_FLAGS} -fsanitize=address -static-libasan")
endif()

if (TTF)
	add_definitions(-DTTF)
endif()

# set the compiler flags for DEBUG and RELEASE builds
set(CMAKE_C_FLAGS "${EXTRA_C_FLAGS} -Wall -Wdeclaration-after-statement -W -Wno-unused-parameter -Wno-sign-compare -D_GNU_SOURCE=1 -D_REENTRANT")
set(CMAKE_C_FLAGS_DEBUG "-Og -ggdb -pipe -fno-strict-aliasing")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fomit-frame-pointer -ffast-math -pipe -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${EXTRA_CXX_FLAGS} -Wall -D_GNU_SOURCE=1 -W -Wno-unused-parameter -Wno-sign-compare -Wno-cast-function-type -D_REENTRANT -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb -pipe -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer -ffast-math -pipe -fno-strict-aliasing")

# set the OS level defines and default executable names
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	if (NOT EXEC)
		set(EXEC "el.linux.bin")
	endif()
	add_definitions(-DLINUX)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	if (NOT EXEC)
		set(EXEC "el")
	endif()
	add_definitions(-DWINDOWS -DWINVER=0x500 -mwindows -DNDEBUG)
	set(EXTRA_LD_FLAGS "${EXTRA_LD_FLAGS} -mwindows")
	set(SOURCES ${SOURCES} ${SD}elc_private.rc)
	set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")
else()
	message( FATAL_ERROR "Unknown platform [${CMAKE_SYSTEM_NAME}]" )
endif()

# any final defines needed
add_definitions(-DELC -D_7ZIP_ST)

# default to building a release version
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE release)
endif()

# set version number if prefix specified
if (VERSION_PREFIX)
	execute_process(COMMAND bash -c "date +'%Y%m%d.%H%M'" OUTPUT_VARIABLE VERSION_DATE)
	string(STRIP "${VERSION_PREFIX}-${VERSION_DATE}" FULL_VERSION)
	set_property(SOURCE ${SD}multiplayer.c APPEND PROPERTY COMPILE_DEFINITIONS GIT_VERSION="${FULL_VERSION}")
endif()

# clean up extra strings to avoid policy CMP0004
string(STRIP "${EXTRA_C_FLAGS}" EXTRA_C_FLAGS)
string(STRIP "${EXTRA_CXX_FLAGS}" EXTRA_CXX_FLAGS)
string(STRIP "${EXTRA_LD_FLAGS}" EXTRA_LD_FLAGS)

# build

add_executable(${EXEC} ${SOURCES})

# if JSON_FILES wanted define dependancies
if (JSON_FILES)
	set(EXTRA_TARGET_LINK nlohmann_json::nlohmann_json)
	set(EXTRA_TARGET_INCLUDE nlohmann_json::nlohmann_json)
else()
	set(EXTRA_TARGET_LINK "")
	set(EXTRA_TARGET_INCLUDE "")
endif()

target_link_libraries(${EXEC}
	${EXTRA_LD_FLAGS}
	${SDL2_LIBRARY} ${SDL2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${CAL3D_LIBRARIES}
	${SDL2_NET_LIBRARY}
	${SDL2_IMAGE_LIBRARY}
	${OPENGL_LIBRARIES}
	${OPENAL_LIBRARY}
	${VORBIS_LIBRARIES}
	${VORBISFILE_LIBRARIES}
	${PNG_LIBRARIES}
	${X11_LIBRARIES}
	${Iconv_LIBRARIES}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${STATIC_LIBRARIES}
	${EXTRA_TARGET_LINK}
)

target_include_directories(${EXEC} SYSTEM PUBLIC
	${SDL2_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS}
	${LIBXML2_INCLUDE_DIR}
	${CAL3D_INCLUDE_DIR}
	${SDL2_NET_INCLUDE_DIR}
	${SDL2_IMAGE_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${OPENAL_INCLUDE_DIR}
	${VORBIS_INCLUDE_DIR}
	${VORBISFILE_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${X11_INCLUDE_DIR}
	${Iconv_INCLUDE_DIRS}
	${EXTRA_TARGET_INCLUDE}
)

if (TTF)
	target_link_libraries(${EXEC} ${SDL2TTF_LIBRARY})
	target_include_directories(${EXEC} SYSTEM PUBLIC ${SDL2TTF_INCLUDE_DIR})
endif()

install(
	TARGETS ${EXEC} RUNTIME DESTINATION bin
)
