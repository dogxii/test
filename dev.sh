#!/usr/bin/env sh
set -eu

APP_NAME="Robot3DRoaming"
ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ACTION="${1:-run}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

OS_NAME=$(uname -s 2>/dev/null || echo unknown)
IS_WINDOWS=0
case "$OS_NAME" in
    MINGW*|MSYS*|CYGWIN*)
        IS_WINDOWS=1
        ;;
esac

if [ "$IS_WINDOWS" -eq 1 ]; then
    BUILD_DIR="$ROOT_DIR/build-win"
else
    BUILD_DIR="$ROOT_DIR/build"
fi

print_help() {
    printf '%s\n' "Usage: sh dev.sh [run|build|clean|setup-vcpkg|help]"
    printf '%s\n' ""
    printf '%s\n' "Examples:"
    printf '%s\n' "  sh dev.sh          Build and run"
    printf '%s\n' "  sh dev.sh build    Build only"
    printf '%s\n' "  sh dev.sh clean    Remove build output"
}

need_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        printf '%s\n' "Missing command: $1"
        printf '%s\n' "Please install it first, then run this script again."
        exit 1
    fi
}

cmake_path() {
    if command -v cygpath >/dev/null 2>&1; then
        cygpath -m "$1"
    else
        printf '%s\n' "$1"
    fi
}

find_vcpkg_root() {
    for path in "${VCPKG_ROOT:-}" "/c/vcpkg" "$HOME/vcpkg"; do
        if [ -n "$path" ] && [ -f "$path/scripts/buildsystems/vcpkg.cmake" ]; then
            printf '%s\n' "$path"
            return 0
        fi
    done
    return 1
}

setup_vcpkg() {
    need_command git

    if [ "$IS_WINDOWS" -eq 1 ]; then
        VCPKG_DIR="/c/vcpkg"
    else
        VCPKG_DIR="$HOME/vcpkg"
    fi

    if [ ! -d "$VCPKG_DIR" ]; then
        git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
    fi

    if [ "$IS_WINDOWS" -eq 1 ]; then
        (cd "$VCPKG_DIR" && ./bootstrap-vcpkg.bat)
    else
        (cd "$VCPKG_DIR" && ./bootstrap-vcpkg.sh)
    fi

    printf '%s\n' "vcpkg is ready: $VCPKG_DIR"
}

windows_generator() {
    if [ -n "${CMAKE_GENERATOR:-}" ]; then
        printf '%s\n' "$CMAKE_GENERATOR"
        return 0
    fi

    VSWHERE="/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe"
    if [ -x "$VSWHERE" ]; then
        VERSION=$("$VSWHERE" -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion 2>/dev/null || true)
        MAJOR=$(printf '%s\n' "$VERSION" | awk -F. 'NF { print $1; exit }')
        case "$MAJOR" in
            18)
                printf '%s\n' "Visual Studio 18 2026"
                return 0
                ;;
            17)
                printf '%s\n' "Visual Studio 17 2022"
                return 0
                ;;
            16)
                printf '%s\n' "Visual Studio 16 2019"
                return 0
                ;;
        esac
    fi

    printf '%s\n' "Visual Studio 17 2022"
}

configure_project() {
    need_command cmake

    if [ "$IS_WINDOWS" -eq 1 ]; then
        VCPKG_ROOT_FOUND=$(find_vcpkg_root || true)
        if [ -z "$VCPKG_ROOT_FOUND" ]; then
            printf '%s\n' "Windows build needs vcpkg."
            printf '%s\n' "Run: sh dev.sh setup-vcpkg"
            exit 1
        fi

        TOOLCHAIN=$(cmake_path "$VCPKG_ROOT_FOUND/scripts/buildsystems/vcpkg.cmake")
        GENERATOR=$(windows_generator)
        cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
            -G "$GENERATOR" -A x64 \
            -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
            -DVCPKG_TARGET_TRIPLET=x64-windows-static
    else
        if [ -n "${CMAKE_TOOLCHAIN_FILE:-}" ]; then
            cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
                -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
                -DCMAKE_TOOLCHAIN_FILE="$CMAKE_TOOLCHAIN_FILE"
        elif VCPKG_ROOT_FOUND=$(find_vcpkg_root); then
            TOOLCHAIN=$(cmake_path "$VCPKG_ROOT_FOUND/scripts/buildsystems/vcpkg.cmake")
            cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
                -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
                -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN"
        else
            cmake -S "$ROOT_DIR" -B "$BUILD_DIR" \
                -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        fi
    fi
}

build_project() {
    configure_project

    if [ "$IS_WINDOWS" -eq 1 ]; then
        cmake --build "$BUILD_DIR" --config "$BUILD_TYPE"
    else
        cmake --build "$BUILD_DIR" -j
    fi
}

run_project() {
    build_project

    if [ "$IS_WINDOWS" -eq 1 ]; then
        EXE="$BUILD_DIR/$BUILD_TYPE/$APP_NAME.exe"
    else
        EXE="$BUILD_DIR/$APP_NAME"
    fi

    if [ ! -x "$EXE" ]; then
        printf '%s\n' "Executable not found: $EXE"
        exit 1
    fi

    "$EXE"
}

case "$ACTION" in
    run)
        run_project
        ;;
    build)
        build_project
        ;;
    clean)
        rm -rf "$BUILD_DIR"
        printf '%s\n' "Removed: $BUILD_DIR"
        ;;
    setup-vcpkg)
        setup_vcpkg
        ;;
    help|-h|--help)
        print_help
        ;;
    *)
        printf '%s\n' "Unknown action: $ACTION"
        print_help
        exit 1
        ;;
esac
