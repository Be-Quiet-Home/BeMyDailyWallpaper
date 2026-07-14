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

# Haiku's makefile-engine has no O2 optimization level. Disable its optimizer
# and express the project-owned generic O2 policy explicitly.
OPTIMIZE := NONE
WARNINGS := ALL
COMPILER_FLAGS += -O2 -Wextra -Werror

include /boot/system/develop/etc/makefile-engine

.PHONY: help smoke

help:
	@echo "BeMyDailyWall build targets:"
	@echo "  make        Build the application"
	@echo "  make clean  Remove build artifacts"
	@echo "  make smoke  Launch and verify the application stays alive"
	@echo "  make help   Show this help"

smoke: $(OBJ_DIR)/$(NAME)
	@set -e; \
	app="$(OBJ_DIR)/$(NAME)"; \
	test -x "$$app"; \
	"$$app" & pid=$$!; \
	trap 'kill "$$pid" 2>/dev/null || true' EXIT HUP INT TERM; \
	sleep 1; \
	kill -0 "$$pid"; \
	kill "$$pid"; \
	wait "$$pid" 2>/dev/null || true; \
	trap - EXIT HUP INT TERM; \
	echo "BeMyDailyWall smoke: ok"
