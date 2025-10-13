#!/usr/bin/env bash

# =============================================================================
# evie Project Initialization Script (Linux)
# =============================================================================
# This script initializes and builds the evie project locally. It validates
# the environment, checks for required tools, manages the build setup, and
# ensures analytics tools are properly configured.
#
# Features:
# - Environment and requirements validation
# - Automated build directory setup
# - Project compilation with meson
# - Analytics tools validation and setup
# - Proper file permissions configuration
#
# =============================================================================

# Function to check if the script is being run from the project root directory
check_directory() {
    local current_dir=$(pwd)
    local project_root_name="evie"
    local bin_dir_name="bin"

    # Attempt to find the project root directory by looking for the .git directory
    project_root=$(git rev-parse --show-toplevel 2>/dev/null)

    # If the project root is not found, exit with an error message
    if [ -z "$project_root" ]; then
        echo "Error: Unable to determine the project root directory. Ensure you're in the evie project."
        exit 1
    fi

    # Navigate to the 'bin' directory if not already there
    if [ "$(basename "$project_root")" == "$project_root_name" ] && [ "$current_dir" == "$project_root" ]; then
        echo "Project root directory detected. Changing to the 'bin' directory."
        cd "$project_root/$bin_dir_name" || exit 1
    elif [[ "$current_dir" != "$project_root/$bin_dir_name"* ]]; then
        echo "Navigating to the 'bin' directory within the project."
        cd "$project_root/$bin_dir_name" || exit 1
    fi
}

# Function to check if a command is available and exit if not
check_command() {
    local cmd="$1"
    local cmd_name="$2"

    if ! command -v "$cmd" &>/dev/null; then
        echo "Error: '$cmd_name' is required but not installed. Please install it before running this script."
        exit 1
    fi
}

# Check if we're in the correct directory
check_directory

echo "=== evie Project Initialization ==="
echo

# Set execution permissions on all necessary scripts
echo "Setting execution permissions..."
find . -name "*.sh" -type f -exec chmod +x {} \; 2>/dev/null || true

# Step 1: Ensure the required tools are installed
check_command "meson" "meson"
check_command "g++" "g++"

echo

# Step 2: Set up build environment
echo "Step 2: Setting up build environment..."

# Ensure the CXX variable is set (default to 'g++' if not)
if [ -z "$CXX" ]; then
    export CXX=g++
    echo "CXX environment variable was not set. Defaulting to 'g++'."
fi

# Check if ./build, ./build-win, ./build-release, ./build-linux exists, if so then remove them
if [ -d "./build" ]; then
    echo "Removing existing build directory..."
    rm -rf ./build
fi
if [ -d "./build-release" ]; then
    echo "Removing existing build-release directory..."
    rm -rf ./build-release
fi

echo "Setting up the default build configure..."
meson setup build -Dbuildtype=debug || exit 1

# Step 4: Compile the project using meson
echo "Step 4: Compiling the project..."
meson compile -C build evie_core || exit 1

echo
echo "=== Initialization Complete ==="
echo "Run ./bin/test.sh to test evie integrity"