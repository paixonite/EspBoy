param (
    [string]$Port = "COM7"
)

Write-Host "[1/2] Compiling firmware in WSL..." -ForegroundColor Cyan
wsl -d ubuntu -- bash -c "/home/paixas/.local/bin/arduino-cli compile --fqbn esp32:esp32:lilygo_t_display_s3 --export-binaries ~/coding-projects/EspBoy/firmware/EspBoy"

if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Compilation failed!" -ForegroundColor Red
    exit 1
}

Write-Host "[2/2] Flashing ESP32-S3 over $Port..." -ForegroundColor Yellow
$esptool = "C:\Users\Paixas\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.1.0\esptool.exe"
$buildDir = "\\wsl.localhost\Ubuntu\home\paixas\coding-projects\EspBoy\firmware\EspBoy\build\esp32.esp32.lilygo_t_display_s3"

& $esptool --chip esp32s3 --port $Port --baud 921600 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 80m --flash-size 16MB `
    0x0 "$buildDir\EspBoy.ino.bootloader.bin" `
    0x8000 "$buildDir\EspBoy.ino.partitions.bin" `
    0xe000 "$buildDir\boot_app0.bin" `
    0x10000 "$buildDir\EspBoy.ino.bin"

if ($LASTEXITCODE -eq 0) {
    Write-Host "[SUCCESS] Flash complete! Enjoy your EspBoy!" -ForegroundColor Green
} else {
    Write-Host "[ERROR] Flash failed. Check if COM port is in use." -ForegroundColor Red
}
