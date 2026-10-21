#!/usr/bin/env bash
# ==============================================================================
# EspBoy - Build & Upload Script for WSL (Windows Subsystem for Linux)
# ==============================================================================
# Compiles the firmware inside WSL using arduino-cli and uploads it to the ESP32-S3
# via the Windows host's esptool.exe (bridging the WSL2 USB serial isolation).
# ==============================================================================

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default configurations
PORT="COM7"
BAUD="921600"
FQBN="esp32:esp32:lilygo_t_display_s3"
FIRMWARE_DIR="${SCRIPT_DIR}/firmware/EspBoy"
CUSTOM_CLI=""
CUSTOM_ESPTOOL=""

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  -p, --port <PORT>        Serial COM port on Windows (default: COM7)
  -b, --baud <BAUD>        Upload baud rate (default: 921600)
  -f, --fqbn <FQBN>        Board FQBN (default: esp32:esp32:lilygo_t_display_s3)
  -d, --dir <DIR>          Path to firmware directory (default: firmware/EspBoy)
  -c, --cli <PATH>         Path to arduino-cli inside WSL (auto-detected if omitted)
  -e, --esptool <PATH>     Path to Windows esptool.exe (auto-detected if omitted)
  -h, --help               Show this help message and exit

Examples:
  ./build_and_upload.sh
  ./build_and_upload.sh -p COM3
  ./build_and_upload.sh --port COM7 --baud 460800
EOF
    exit 0
}

# Parse command line flags
while [[ $# -gt 0 ]]; do
    case "$1" in
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -b|--baud)
            BAUD="$2"
            shift 2
            ;;
        -f|--fqbn)
            FQBN="$2"
            shift 2
            ;;
        -d|--dir)
            FIRMWARE_DIR="$2"
            shift 2
            ;;
        -c|--cli)
            CUSTOM_CLI="$2"
            shift 2
            ;;
        -e|--esptool)
            CUSTOM_ESPTOOL="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Error: Unknown argument '$1'" >&2
            usage
            ;;
    esac
done

if [[ ! -d "$FIRMWARE_DIR" ]]; then
    echo "Error: Firmware directory not found at '$FIRMWARE_DIR'" >&2
    exit 1
fi

# 1. Locate arduino-cli inside WSL
if [[ -n "$CUSTOM_CLI" ]]; then
    ARDUINO_CLI="$CUSTOM_CLI"
elif command -v arduino-cli &>/dev/null; then
    ARDUINO_CLI="arduino-cli"
elif [[ -x "${HOME}/.local/bin/arduino-cli" ]]; then
    ARDUINO_CLI="${HOME}/.local/bin/arduino-cli"
else
    echo "Error: 'arduino-cli' not found in PATH or ~/.local/bin. Specify path with -c <path>." >&2
    exit 1
fi

# 2. Locate esptool.exe on Windows host
if [[ -n "$CUSTOM_ESPTOOL" ]]; then
    ESPTOOL_WIN="$CUSTOM_ESPTOOL"
else
    # Auto-detect esptool.exe via PowerShell without hardcoding any user or directory
    ESPTOOL_WIN=$(cd /mnt/c && powershell.exe -NoProfile -NonInteractive -Command '
        $cmd = (Get-Command esptool.exe -ErrorAction SilentlyContinue).Source
        if (-not $cmd) {
            $cmd = Get-ChildItem -Path "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools\esptool_py" -Filter "esptool.exe" -Recurse -ErrorAction SilentlyContinue |
                Sort-Object FullName -Descending |
                Select-Object -First 1 -ExpandProperty FullName
        }
        if ($cmd) { Write-Output $cmd }
    ' </dev/null | tr -d '\r')

    if [[ -z "$ESPTOOL_WIN" ]]; then
        echo "Error: 'esptool.exe' not found on Windows host. Specify path with -e <path>." >&2
        exit 1
    fi
fi

# 3. Compile sketch
echo -e "\033[36m[1/2] Compiling firmware in WSL (${FQBN})...\033[0m"
"$ARDUINO_CLI" compile --fqbn "$FQBN" --export-binaries "$FIRMWARE_DIR"

# 4. Prepare temporary staging directory on Windows host
WIN_TEMP=$(cd /mnt/c && powershell.exe -NoProfile -NonInteractive -Command '[System.IO.Path]::GetTempPath()' </dev/null | tr -d '\r')
WIN_STAGE_DIR="${WIN_TEMP}EspBoy_Build"
WSL_STAGE_DIR="$(wslpath -u "${WIN_TEMP}")EspBoy_Build"

mkdir -p "$WSL_STAGE_DIR"

FQBN_FOLDER="${FQBN//:/.}"
BUILD_OUTPUT_DIR="${FIRMWARE_DIR}/build/${FQBN_FOLDER}"

if [[ ! -d "$BUILD_OUTPUT_DIR" ]]; then
    echo "Error: Build output folder not found at '$BUILD_OUTPUT_DIR'" >&2
    exit 1
fi

cp -f "${BUILD_OUTPUT_DIR}"/*.bin "${WSL_STAGE_DIR}/"

# 5. Flash via Windows esptool over target serial port
echo -e "\033[33m[2/2] Flashing ESP32-S3 over ${PORT} at ${BAUD} baud...\033[0m"

(
    cd /mnt/c && powershell.exe -NoProfile -NonInteractive -Command "
        & '${ESPTOOL_WIN}' --chip esp32s3 --port ${PORT} --baud ${BAUD} --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 80m --flash-size 16MB \
            0x0 '${WIN_STAGE_DIR}\\EspBoy.ino.bootloader.bin' \
            0x8000 '${WIN_STAGE_DIR}\\EspBoy.ino.partitions.bin' \
            0xe000 '${WIN_STAGE_DIR}\\boot_app0.bin' \
            0x10000 '${WIN_STAGE_DIR}\\EspBoy.ino.bin'
    " </dev/null
)

echo -e "\033[32m[SUCCESS] Flash complete! Enjoy your EspBoy!\033[0m"
