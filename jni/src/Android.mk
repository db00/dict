LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main


SDL_PATH :=../SDL2
IMAGE_PATH :=../SDL2_image
MIXER_PATH :=../SDL2_mixer
TTF_PATH :=../SDL2_ttf
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(IMAGE_PATH) \
	$(LOCAL_PATH)/$(TTF_PATH) \
	$(LOCAL_PATH)/$(MIXER_PATH) \
	$(LOCAL_PATH)/include \


# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	sdlfiles.c sprite.c matrix.c input.c myregex.c kodi.c utf8.c jsonrpc.c files.c httploader.c ipstring.c mystring.c cJSON.c \
	dict.c readbaidu.c update.c textfield.c myttf.c myfont.c bytearray.c zip.c read_card.c pinyin.c music.c searhdict.c \
	tween.c ease.c urlcode.c filetypes.c httpserver.c array.c base64.c mysurface.c datas.c sqlite.c besier.c sdlstring.c loading.c pictures.c testwords.c explain.c btnlist.c sidebtns.c wordinput.c mylist.c alert.c main.c # sqlite3.c
#	YourSourceHere.c
#array.c sdlfiles.c files.c myregex.c ease.c mystring.c tween.c matrix.c sprite.c
# sdlfiles.c myfont.c loading.c ease.c tween.c array.c utf8.c update.c httploader.c ipstring.c urlcode.c base64.c  bytearray.c zip.c files.c myregex.c sdlstring.c textfield.c matrix.c sprite.c mystring.c
#gif.c ease.c array.c files.c mystring.c myregex.c tween.c matrix.c sprite.c dgif_lib.c gif_err.c gifalloc.c
# sprite.c matrix.c array.c files.c regex.c myregex.c mystring.c 

# sprite.c matrix.c array.c\
  \
#LOCAL_CFLAGS +=  -D debugtext
#LOCAL_CFLAGS +=  -D debug_tween
#LOCAL_CFLAGS +=  -D test_gif
#LOCAL_CFLAGS +=  -D debug_sprite

LOCAL_CFLAGS +=  -D STDC_HEADERS

LOCAL_SHARED_LIBRARIES := \
	SDL2 \
	smpeg2 \
	mikmod \
	SDL2_image \
	SDL2_mixer \
	SDL2_ttf \


LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lssl -lcrypto -lz -lm -lsqlite # -lregex

include $(BUILD_SHARED_LIBRARY)




