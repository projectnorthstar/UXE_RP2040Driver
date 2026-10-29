& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e pico

if ($LASTEXITCODE -eq 0) {
    New-Item -ItemType Directory -Force -Path ".\release" | Out-Null

    Copy-Item ".pio\build\pico\firmware.uf2" ".\release\UXE_Firmware.uf2"
    Copy-Item ".pio\build\pico\firmware.bin" ".\release\UXE_Firmware.bin"

    Write-Host "Firmware exported to .\release"
}