LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

ELVERSION=1.9.5.8

SDL_PATH := ../SDL2
SDL_IMAGE_PATH := ../SDL2_image
SDL_NET_PATH := ../SDL2_net
CAL3D_PATH := ../cal3d/
LIBXML2_PATH := ../libxml2
GL4ES_PATH := ../gl4es/
ICONV_PATH := ../iconv/
MYGLOB_PATH := ../myglob/
GLU_PATH := ../glu/

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
$(LOCAL_PATH)/$(SDL_IMAGE_PATH) \
$(LOCAL_PATH)/$(SDL_NET_PATH) \
$(LOCAL_PATH)/$(CAL3D_PATH)/cal3d-0.11.0/src/ \
$(LOCAL_PATH)/$(LIBXML2_PATH)/include/ \
$(LOCAL_PATH)/$(ICONV_PATH)/libiconv-1.14/include/ \
$(LOCAL_PATH)/$(EXTRAS_PATH)/ \
$(LOCAL_PATH)/$(GL4ES_PATH)/include/ \
$(LOCAL_PATH)/$(GLU_PATH)/glu-9.0.1/include/ \
$(LOCAL_PATH)/$(MYGLOB_PATH)/include/ \
$(LOCAL_PATH)/nlohmann_json/single_include/

LOCAL_CFLAGS := \
	-O3 -fsigned-char -frtti \
	-DANDROID \
	-DUSE_INLINE \
	-DELC \
	-DJSON_FILES \
	-DFASTER_MAP_LOAD \
	-DNEW_EYES \
	-DCLUSTER_INSIDES \
	-DFASTER_STARTUP \
	-DFUZZY_PATHS \
	-DBANDWIDTH_SAVINGS \
	-DANIMATION_SCALING \
	-DGIT_VERSION=\"$(ELVERSION)\"

# Add your application source files here...
LOCAL_SRC_FILES := \
	$(SDL_PATH)/src/main/android/SDL_android_main.c \
	\
	2d_objects.c \
	3d_objects.c \
	actors.c \
	actor_scripts.c \
	alphamap.c \
	asc.c \
	astrology.c \
	bags.c \
	bbox_tree.c \
	books/fontdef.c \
	books/parser.c \
	books/symbols.c \
	books/typesetter.c \
	buddy.c \
	buffs.c \
	cache.c \
	cal.c \
	calc.c \
	chat.c \
	cluster.c \
	colors.c \
	console.c \
	consolewin.c \
	counters.c \
	cursors.c \
	dds.c \
	ddsimage.c \
	dialogues.c \
	draw_scene.c \
	elconfig.c \
	el_memory.c \
	elmemory.c \
	elwindows.c \
	emotes.c \
	encyclopedia.c \
	errors.c \
	events.c \
	filter.c \
	framebuffer.c \
	frustum.c \
	fsaa/fsaa.c \
	fsaa/fsaa_dummy.c \
	gamewin.c \
	gl_init.c \
	hash.c \
	help.c \
	highlight.c \
	hud.c \
	hud_misc_window.c \
	hud_quickbar_window.c \
	hud_quickspells_window.c \
	hud_statsbar_window.c \
	ignore.c \
	image.c \
	image_loading.c \
	init.c \
	interface.c \
	io/e3d_io.c \
	io/elc_io.c	io/map_io.c \
	io/elfilewrapper.c \
	io/elpathwrapper.c \
	io/fileutil.c \
	io/half.c \
	io/ioapi.c \
	io/normal.c \
	io/unzip.c \
	io/xmlcallbacks.c \
	items.c \
	keys.c \
	knowledge.c \
	langselwin.c \
	lights.c \
	list.c \
	load_gl_extensions.c \
	loading_win.c \
	loginwin.c \
	main.c \
	makeargv.c \
	manufacture.c \
	map.c \
	mapwin.c \
	md5.c \
	mines.c \
	minimap.c \
	misc.c \
	missiles.c \
	multiplayer.c \
	new_actors.c \
	new_character.c \
	notepad.c \
	openingwin.c \
	particles.c \
	paste.c \
	pathfinder.c \
	pm_log.c \
	popup.c \
	queue.c \
	reflection.c \
	rules.c	sky.c \
	serverpopup.c \
	servers.c \
	session.c \
	shader/noise.c \
	shader/shader.c \
	shadows.c \
	skeletons.c \
	skills.c \
	sort.c \
	sound.c	\
	special_effects.c \
	spells.c \
	stats.c \
	storage.c \
	symbol_table.c \
	tabs.c \
	text_aliases.c \
	text.c \
	textures.c \
	tile_map.c \
	timers.c \
	trade.c \
	translate.c \
	url.c \
	weather.c \
	widgets.c \
	xz/7zCrc.c \
	xz/7zCrcOpt.c \
	xz/Alloc.c \
	xz/Bra86.c \
	xz/Bra.c \
	xz/BraIA64.c \
	xz/CpuArch.c \
	xz/Delta.c \
	xz/LzFind.c \
	xz/Lzma2Dec.c \
	xz/LzmaDec.c \
	xz/Sha256.c \
	xz/Xz.c \
	xz/XzCrc64.c \
	xz/XzDec.c \
	\
	achievements.cpp \
	actor_init.cpp \
	books.cpp \
	cal3d_wrapper.cpp \
	command_queue.cpp \
	context_menu.cpp \
	elloggingwrapper.cpp \
	engine/hardwarebuffer.cpp \
	engine/logging.cpp \
	exceptions/extendedexception.cpp \
	eye_candy/effect_bag.cpp \
	eye_candy/effect_breath.cpp \
	eye_candy/effect_campfire.cpp \
	eye_candy/effect_candle.cpp \
	eye_candy/effect_cloud.cpp \
	eye_candy/effect_firefly.cpp \
	eye_candy/effect_fountain.cpp \
	eye_candy/effect_glow.cpp \
	eye_candy/effect_harvesting.cpp \
	eye_candy/effect_impact.cpp \
	eye_candy/effect_lamp.cpp \
	eye_candy/effect_mines.cpp \
	eye_candy/effect_missile.cpp \
	eye_candy/effect_ongoing.cpp \
	eye_candy/effect_selfmagic.cpp \
	eye_candy/effect_smoke.cpp \
	eye_candy/effect_staff.cpp \
	eye_candy/effect_summon.cpp \
	eye_candy/effect_sword.cpp \
	eye_candy/effect_targetmagic.cpp \
	eye_candy/effect_teleporter.cpp \
	eye_candy/effect_wind.cpp \
	eye_candy/eye_candy.cpp \
	eye_candy/kepler_orbit.cpp \
	eye_candy/math_cache.cpp \
	eye_candy/orbital_mover.cpp \
	eye_candy_wrapper.cpp \
	font.cpp \
	hud_indicators.cpp \
	hud_timer.cpp \
	icon_window.cpp \
	io/cal3d_io_wrapper.cpp \
	item_info.cpp \
	item_lists.cpp \
	json_io.cpp \
	named_colours.cpp \
	optimizer.cpp \
	password_manager.cpp \
	quest_log.cpp \
	select.cpp \
	sendvideoinfo.cpp \
	trade_log.cpp \
	user_menus.cpp \
	xml/xmlhelper.cpp \
	xor_cipher.cpp

LOCAL_SHARED_LIBRARIES := \
	SDL2 \
	SDL2_image \
	SDL2_net \
	libGL \
	cal3d \
	iconv \
	libGLU \
	libmyglob \
	libxml2

LOCAL_LDLIBS := -lz -llog

include $(BUILD_SHARED_LIBRARY)
