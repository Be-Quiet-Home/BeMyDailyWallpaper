## Haiku Generic Makefile

NAME = BeMyDailyWall
TYPE = APP

APP_MIME_SIG = application/x-vnd.BeQuietHome-BeMyDailyWall

SRCS = \
	src/BeMyDailyWallApp.cpp \
	src/MainWindow.cpp \
	src/DeskbarView.cpp \
	src/WallpaperInfo.cpp \
	src/ProviderResult.cpp \
	src/DailyImageProvider.cpp \
	src/DemoProvider.cpp \
	src/WallpaperSetter.cpp \
	src/AppSettings.cpp

LIBS = be $(STDCPPLIBS)

LOCAL_INCLUDE_PATHS = src
SYSTEM_INCLUDE_PATHS =

OPTIMIZE := FULL

include /boot/system/develop/etc/makefile-engine
