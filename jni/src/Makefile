PROJ_PATH=../..
ifneq ($(OS),Windows_NT)
OS:=$(shell uname)
endif
#JAVA_HOME:=/usr/lib/jvm/java-8-openjdk-i386/jre
ifeq ($(OS),Darwin)
NDK_HOME:= $(HOME)/android-ndk-r9c
SDK_HOME:= $(HOME)/android-sdk-macosx/tools
else
NDK_HOME:= $(HOME)/android-ndk-r10
#NDK_HOME:= $(HOME)/android-ndk-r19b/
SDK_HOME:= $(HOME)/android-sdk-linux/tools
endif
APK_DIR:= $(PROJ_PATH)/bin
APP_NAME=test

PACKAGEOLD:=org.libsdl.app
PACKAGEOLD_:=$(subst .,_,$(PACKAGEOLD))#将"."替换成"_"
PACKAGEOLDI:=$(subst .,/,$(PACKAGEOLD))#将"."替换成"/"
CLASS_NAME:=SDLActivity

PACKAGE:=org.db0.app
PACKAGE_:=$(subst .,_,$(PACKAGE))#将"."替换成"_"
PACKAGEI:=$(subst .,/,$(PACKAGE))#将"."替换成"/"
CLASS_NAMEOLD:=SDLActivity


APK:=$(APK_DIR)/$(APP_NAME).apk
DEBUG_APK:=$(APK_DIR)/$(APP_NAME)-debug.apk
CC:=gcc
SRC_DIR:=.
LIB:=-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm -lssl -lcrypto -lpthread
SRC:= sprite.c matrix.c input.c myregex.c kodi.c utf8.c jsonrpc.c files.c httploader.c ipstring.c mystring.c cJSON.c \
		dict.c readbaidu.c update.c textfield.c myttf.c myfont.c bytearray.c zip.c read_card.c pinyin.c music.c searhdict.c\
		tween.c ease.c urlcode.c filetypes.c httpserver.c array.c base64.c mysurface.c datas.c sqlite.c besier.c sdlstring.c loading.c pictures.c testwords.c explain.c btnlist.c sidebtns.c wordinput.c mylist.c alert.c main.c# sqlite3.c
DEFINES:= -Wall -D STDC_HEADERS
TARGET:= ./a.out
ifeq ($(OS),Darwin)
INCLUDE:=-I"/usr/local/include/SDL2/" -I"include"
LIB:=$(LIB) -L"/usr/local/lib/" -ldl -lz -liconv -lsqlite3
else ifeq ($(OS),Linux)
INCLUDE:=$(shell sdl2-config --cflags) -ldl -lsqlite3 -lz
else ifeq ($(OS),Windows_NT)
DEFINES:= $(DEFINES) -D main=SDL_main
TARGET:= a.exe
LIB:=-lmingw32 -lSDL2main $(LIB)  -mwindows  -Wl,--no-undefined -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -liconv -lws2_32 -lGLESv2 -lopengl32 -lwsock32 # lib/libssl.a lib/libcrypto.a lib/libeay32.dll.a lib/libssl32.dll.a#  -lavformat -lavcodec -lavutil -lswscale -lwsock32
INCLUDE:=-I"../SDL2/include/" -I"." -I"../SDL2_image/" -I"../SDL2_mixer/" -I"../SDL2_ttf/" -I"include"
SRC:= $(SRC) regex.c sqlite3.c
endif
# -D debug_sprite
P12:=./my.p12
KEY:=./test.keystore
#.p12 pass
KEYPASS:=android
#.keystore pass
STOREPASS:=android
KEYALIAS:=1
KEYALIAS:=androiddebugkey
DNAME:="CN=db0@qq.com, OU=biao, O=biao, L=hengyang, ST=hunane, C=cn"
#SRC:= $(foreach x,${SRC_DIR}, $(wildcard $(addprefix ${x}/*,.c)))
OBJS:= $(patsubst %.c,%.o,$(SRC))

CFLAGS :=  -g $(INCLUDE) $(DEFINES) $(LIB)
#SDL_PATH:=$(HOME)/SDL2-2.0.5/

all:$(TARGET)
	echo $(OS)
ifeq ($(OS),Windows_NT)
#	@echo $(shell set)
else	
#	@echo $(shell export)
#	@echo $(shell set)
endif
#	gdb $(TARGET)
	IBUS_ADDRESS="---" $(TARGET)
	@echo "all ok"

$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

update:
ifdef SDL_PATH
	rm -rf $(PROJ_PATH)/jni/SDL2/* ; cp -r $(SDL_PATH)/src $(PROJ_PATH)/jni/SDL2/src ; cp -r $(SDL_PATH)/include $(PROJ_PATH)/jni/SDL2/include ; cp -r $(SDL_PATH)/Android.mk $(PROJ_PATH)/jni/SDL2/Androi.mk ;
	sed -e 's/$(PACKAGEOLD)/$(PACKAGE)/' -e 's/$(CLASS_NAMEOLD)/$(CLASS_NAME)/g' $(SDL_PATH)/android-project/src/$(PACKAGEOLDI)/$(CLASS_NAMEOLD).java > tmp ; cat tmp > $(PROJ_PATH)/src/$(PACKAGEI)/$(CLASS_NAME).java
#	sed -e 's/$(PACKAGEOLD_)/$(PACKAGE_)/' -e 's/$(CLASS_NAMEOLD)/$(CLASS_NAME)/g' $(SDL_PATH)/src/main/android/SDL_android_main.c > $(PROJ_PATH)/jni/SDL2/src/main/android/SDL_android_main.c #unnecessary after 2.0.6
	sed -e 's/$(PACKAGEOLD_)/$(PACKAGE_)/' -e 's/$(CLASS_NAMEOLD)/$(CLASS_NAME)/g' $(SDL_PATH)/src/core/android/SDL_android.c > $(PROJ_PATH)/jni/SDL2/src/core/android/SDL_android.c ; rm tmp
	sed -e 's/$(PACKAGEOLD_)/$(PACKAGE_)/' -e 's/$(CLASS_NAMEOLD)/$(CLASS_NAME)/' Android.mk > tmp ; cat tmp >  Android.mk
	sed -e 's/$(CLASS_NAMEOLD)/$(CLASS_NAME)/g' -e 's/$(PACKAGEOLD)/$(PACKAGE)/' -e 's/<application /$(permission)<application /' $(PROJ_PATH)/AndroidManifest.xml > tmp ; cat tmp > $(PROJ_PATH)/AndroidManifest.xml 
endif
	rm -rf $(PROJ_PATH)/bin
	$(SDK_HOME)/android --clear-cache update project --name $(APP_NAME) --path $(PROJ_PATH) --target "android-14"  --subprojects

$(DEBUG_APK): $(KEY) *.c
	$(NDK_HOME)/ndk-build NDK_DEBUG=1 -C $(PROJ_PATH) && ant debug -f $(PROJ_PATH)/build.xml

debug:$(DEBUG_APK)
	curl -F Filedata=@$(DEBUG_APK) -F Filename=test.apk hw:8809/sdcard/sound/ -u test:test
#	adb shell pm uninstall $(PACKAGE)
#	adb install -r $(DEBUG_APK) && adb shell am start -a android.intenon.MAIN -n $(PACKAGE)/.$(CLASS_NAME) && adb shell logcat SDL:V *:I
#	rm $(DEBUG_APK) &&\

$(P12):
	keytool -alias $(KEYALIAS) -genkeypair -keystore $(P12) -storetype pkcs12 -validity 8050 -storepass $(KEYPASS) -dname $(DNAME)
#	rm -f $(PROJ_PATH)/my.keystore


$(KEY):$(P12)
#	keytool -v -genkey -alias $(KEYALIAS) -keyalg RSA -validity 8050 -keystore $(KEY) -keypass $(KEYPASS) -storepass $(STOREPASS) -dname $(DNAME)
	keytool -importkeystore -srckeystore $(P12) -srcstoretype PKCS12 -srcstorepass $(KEYPASS) -destkeystore $(KEY) -deststoretype JKS -storepass $(STOREPASS)
	keytool -list -keystore $(P12) -storetype pkcs12 -storepass $(KEYPASS) -v
#修改test.keystore的 并替换 ~/.android/debug.keystore
#①修改test.keystore的别名
#	keytool -changealias -keystore test.keystore -alias android -destalias androiddebugkey -storepass $(KEYPASS)
#②修改test.keystore的别名密码
#	keytool -keypasswd -keystore test.keystore -storepass $(KEYPASS) -new android -alias androiddebugkey
#③修改test.keystore的密码
#	keytool -storepasswd -keystore test.keystore -storepass $(KEYPASS) -new android
	cp -r $(KEY) ~/.android/debug.keystore

$(APK):$(KEY) *.c
#	$(SDK_HOME)/tools/android create project -n $(APP_NAME) -t 16 -p $(PROJ_PATH) -k $(PACKAGE) -a $(APP_NAME)
#	$(SDK_HOME)/tools/dx -dex --dump-to=out.dex --core-library $(PROJ_PATH)/bin
#	$(SDK_HOME)/tools/aapt -A assets -S res -M AndroidManifest.xml -I $(SDK_HOME)/platforms/android-10/android.jar -F $(PROJ_PATH)/bin/out.ap_
	$(NDK_HOME)/ndk-build NDK_DEBUG=1 -C $(PROJ_PATH) && ant release -f $(PROJ_PATH)/build.xml
#	scp $(APK) p@p:/sdcard/
#	adb push $(APK) /sdcard/
#	jarsigner -verbose -keystore $(P12) -storepass $(KEYPASS) -signedjar $(APK) $(PROJ_PATH)/bin/$(APP_NAME)-release-unsigned.apk $(KEYALIAS)
	jarsigner -verbose -keystore $(KEY) -storepass $(STOREPASS) -keypass $(KEYPASS) -signedjar $(APK) $(PROJ_PATH)/bin/$(APP_NAME)-release-unsigned.apk $(KEYALIAS)

apk:$(APK)
#	adb install -r $(APK) && adb shell am start -a android.intenon.MAIN -n $(PACKAGE)/.$(CLASS_NAME)
#	adb push $(APK) /sdcard/
	curl -F Filedata=@$(APK) -F Filename=test.apk hw:8809/sdcard/sound/ -u test:test

.c.o:
	$(CC) $(CFLAGS) -c $<
#上下两句相等
#%.o:%.c
#	$(CC) -c -o $@ $< $(CFLAGS)

ipa:
ifeq ($(OS),Darwin)
	xcodebuild  CODE_SIGN_IDENTITY="iPhone Distribution:hunan huaien network technology co., ltd.(6ZVSARMRUF)" #-target ___PROJECTNAME___ -configuration Release #
	xcrun -sdk iphoneos PackageApplication -v build/Release-iphoneos/___PROJECTNAME___.app -o ~/Desktop/$(APP_NAME).ipa --sign "iPhone Distribution:hunan huaien network technology co., ltd.(6ZVSARMRUF)"
endif

.PHONY : clean
clean:
ifeq ($(OS),Darwin)
	xcodebuild clean
endif
	$(RM) $(TARGET) *.o
	$(NDK_HOME)/ndk-build clean -C $(PROJ_PATH) && ant clean -f $(PROJ_PATH)/build.xml && rm -rf $(PROJ_PATH)/libs $(PROJ_PATH)/obj
