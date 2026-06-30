#!/usr/bin/env bash
set -euo pipefail

# ----------------------------------------------------------------------------
# builds the evie.
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

log "Using project root: ${PROJECT_ROOT}"

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
meson compile -C "${BUILD_DIR}" evie || { err "Build failed"; exit 1; }
log "Build completed successfully"