APP_ABI := all
APP_PLATFORM := android-3
APP_OPTIM := release

APP_CFLAGS += -ffunction-sections -fdata-sections -fvisibility=hidden -fPIC
APP_LDFLAGS += -Wl,--gc-sections -fPIE

APP_STL := stlport_static

NDK_TOOLCHAIN_VERSION := clang
