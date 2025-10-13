#!/usr/bin/env bash
set -euo pipefail

# ----------------------------------------------------------------------------
# Initializes and builds the evie project locally.
# - robust project root detection (git or directory layout fallback)
# - checks for Meson and a C++ compiler, prints brief versions
# - configures and compiles using Meson
# ----------------------------------------------------------------------------

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"  # .../evie/bin
PROJECT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || true)"
if [ -z "$PROJECT_ROOT" ]; then
    PROJECT_ROOT="$(dirname "$SOURCE_DIR")"
fi

cd "$PROJECT_ROOT/bin" || { echo "[build] ERROR: failed to change to $PROJECT_ROOT/bin" >&2; exit 1; }
BUILD_DIR="$PWD/build"

log() { printf '[build] %s\n' "$*"; }
err() { printf '[build] ERROR: %s\n' "$*" >&2; }

# Check required commands
command -v meson >/dev/null 2>&1 || { err "meson is required but not installed"; exit 1; }

# Prefer user-provided CXX, otherwise try g++ then clang++
: "${CXX:=""}"
if [ -z "${CXX}" ]; then
    if command -v g++ >/dev/null 2>&1; then
        CXX=g++
    elif command -v clang++ >/dev/null 2>&1; then
        CXX=clang++
    else
        err "No C++ compiler found (tried g++ and clang++)"
        exit 1
    fi
fi
export CXX

log "Using project root: ${PROJECT_ROOT}"
log "Using CXX=${CXX}"
# Print brief tool versions for reproducibility
$CXX --version 2>/dev/null | sed -n '1p' | sed 's/^/[build] /' || true
meson --version | sed -n '1p' | sed 's/^/[build] /' || true

# Configure or reconfigure Meson build directory
if [ ! -d "$BUILD_DIR" ]; then
    log "Configuring build directory: ${BUILD_DIR} (source: ${SOURCE_DIR})"
    meson setup "${BUILD_DIR}" "${SOURCE_DIR}" || { err "meson setup failed"; exit 1; }
else
    log "Reconfiguring build directory: ${BUILD_DIR} (source: ${SOURCE_DIR})"
    meson setup --reconfigure "${BUILD_DIR}" "${SOURCE_DIR}" || { err "meson reconfigure failed"; exit 1; }
fi

# Build
log "Compiling the project..."
meson compile -C "${BUILD_DIR}" build_native_archive || { err "Build failed"; exit 1; }
log "Build completed successfully"