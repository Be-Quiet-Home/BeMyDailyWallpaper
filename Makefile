## Haiku Generic Makefile

NAME = BeMyDailyWall
TYPE = APP

# Catalog signature used by the Generic Makefile localization targets.
APP_MIME_SIG = x-vnd.BeQuietHome-BeMyDailyWall

SRCS = \
	src/BeMyDailyWallApp.cpp \
	src/MainWindow.cpp \
	src/DeskbarView.cpp \
	src/WallpaperInfo.cpp \
	src/ProviderResult.cpp \
	src/DailyImageProvider.cpp \
	src/DemoProvider.cpp \
	src/LocalFolderProvider.cpp \
	src/WallpaperSetter.cpp \
	src/AppSettings.cpp

LIBS = be localestub translation $(STDCPPLIBS)

LOCALES = en

LOCAL_INCLUDE_PATHS = src
SYSTEM_INCLUDE_PATHS =

# Haiku's makefile-engine has no O2 optimization level. Disable its optimizer
# and express the project-owned generic O2 policy explicitly.
OPTIMIZE := NONE
WARNINGS := ALL
COMPILER_FLAGS += -O2 -Wextra -Werror

include /boot/system/develop/etc/makefile-engine

PROVIDER_SMOKE = $(OBJ_DIR)/provider-contract-smoke
PROVIDER_SMOKE_SRCS = \
	tests/provider_contract_smoke.cpp \
	src/DailyImageProvider.cpp \
	src/DemoProvider.cpp \
	src/ProviderResult.cpp \
	src/WallpaperInfo.cpp

LOCAL_FOLDER_PROVIDER_SMOKE = $(OBJ_DIR)/local-folder-provider-smoke
LOCAL_FOLDER_PROVIDER_SMOKE_SRCS = \
	tests/local_folder_provider_smoke.cpp \
	src/DailyImageProvider.cpp \
	src/LocalFolderProvider.cpp \
	src/ProviderResult.cpp \
	src/WallpaperInfo.cpp

SETTINGS_SMOKE = $(OBJ_DIR)/settings-roundtrip-smoke
SETTINGS_SMOKE_SRCS = \
	tests/settings_roundtrip_smoke.cpp \
	src/AppSettings.cpp

SETTER_SMOKE = $(OBJ_DIR)/wallpaper-setter-smoke
SETTER_SMOKE_SRCS = \
	tests/wallpaper_setter_smoke.cpp \
	src/WallpaperSetter.cpp \
	src/ProviderResult.cpp \
	src/WallpaperInfo.cpp

WALLPAPER_INFO_SMOKE = $(OBJ_DIR)/wallpaper-info-smoke
WALLPAPER_INFO_SMOKE_SRCS = \
	tests/wallpaper_info_smoke.cpp \
	src/WallpaperInfo.cpp

.PHONY: help smoke smoke-provider smoke-local-folder-provider \
	smoke-settings smoke-setter smoke-wallpaper-info

help:
	@echo "BeMyDailyWall build targets:"
	@echo "  make                Build the application"
	@echo "  make clean          Remove build artifacts"
	@echo "  make smoke          Run all smoke checks"
	@echo "  make smoke-provider Verify provider result statuses"
	@echo "  make smoke-local-folder-provider Verify local folder image selection"
	@echo "  make smoke-settings Verify settings persistence round trip"
	@echo "  make smoke-setter   Verify wallpaper setter statuses and errors"
	@echo "  make smoke-wallpaper-info Verify tooltip formatting and omissions"
	@echo "  make catkeys        Collect the English catalog keys"
	@echo "  make catalogs       Compile configured catalogs"
	@echo "  make bindcatalogs   Bind catalogs into the application"
	@echo "  make help           Show this help"

$(PROVIDER_SMOKE): $(PROVIDER_SMOKE_SRCS)
	@mkdir -p "$(OBJ_DIR)"
	$(C++) $(INCLUDES) $(CFLAGS) $(PROVIDER_SMOKE_SRCS) \
		-lbe -llocalestub -o "$@"

$(LOCAL_FOLDER_PROVIDER_SMOKE): $(LOCAL_FOLDER_PROVIDER_SMOKE_SRCS)
	@mkdir -p "$(OBJ_DIR)"
	$(C++) $(INCLUDES) $(CFLAGS) $(LOCAL_FOLDER_PROVIDER_SMOKE_SRCS) \
		-lbe -llocalestub -ltranslation -o "$@"

$(SETTINGS_SMOKE): $(SETTINGS_SMOKE_SRCS)
	@mkdir -p "$(OBJ_DIR)"
	$(C++) $(INCLUDES) $(CFLAGS) $(SETTINGS_SMOKE_SRCS) -lbe -o "$@"

$(SETTER_SMOKE): $(SETTER_SMOKE_SRCS)
	@mkdir -p "$(OBJ_DIR)"
	$(C++) $(INCLUDES) $(CFLAGS) $(SETTER_SMOKE_SRCS) \
		-lbe -llocalestub -o "$@"

$(WALLPAPER_INFO_SMOKE): $(WALLPAPER_INFO_SMOKE_SRCS)
	@mkdir -p "$(OBJ_DIR)"
	$(C++) $(INCLUDES) $(CFLAGS) $(WALLPAPER_INFO_SMOKE_SRCS) \
		-lbe -llocalestub -o "$@"

smoke-provider: $(PROVIDER_SMOKE)
	@"$(PROVIDER_SMOKE)"

smoke-local-folder-provider: $(LOCAL_FOLDER_PROVIDER_SMOKE)
	@"$(LOCAL_FOLDER_PROVIDER_SMOKE)"

smoke-settings: $(SETTINGS_SMOKE)
	@"$(SETTINGS_SMOKE)"

smoke-setter: $(SETTER_SMOKE)
	@"$(SETTER_SMOKE)"

smoke-wallpaper-info: $(WALLPAPER_INFO_SMOKE)
	@"$(WALLPAPER_INFO_SMOKE)"

smoke: $(OBJ_DIR)/$(NAME) smoke-provider smoke-local-folder-provider \
	smoke-settings smoke-setter smoke-wallpaper-info
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
