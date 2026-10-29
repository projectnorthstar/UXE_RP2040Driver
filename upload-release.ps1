# Upload a prebuilt RP2040 UF2 firmware from the local release folder.
# Run from anywhere: .\upload_release.ps1
param(
    [string]$Firmware = (Join-Path $PSScriptRoot 'release\UXE_Firmware.uf2'),
    [int]$WaitSeconds = 60
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Firmware -PathType Leaf)) {
    Write-Error "Firmware not found: $Firmware`nBuild and export release\UXE_Firmware.uf2 first."
    exit 1
}

$Firmware = (Resolve-Path -LiteralPath $Firmware).Path
Write-Host "Firmware: $Firmware"
Write-Host 'Connect the RP2040 while holding BOOTSEL (or reboot it into BOOTSEL mode).'

function Get-PicoBootDrive {
    # The RP2040 ROM BOOTSEL drive is labeled RPI-RP2.
    $volumes = @(Get-CimInstance Win32_LogicalDisk -Filter 'DriveType=2' -ErrorAction SilentlyContinue)
    $volumes += @(Get-CimInstance Win32_LogicalDisk -Filter 'DriveType=3' -ErrorAction SilentlyContinue)
    foreach ($volume in $volumes) {
        if ($volume.VolumeName -eq 'RPI-RP2' -and $volume.DeviceID) {
            return ($volume.DeviceID + '\')
        }
    }
    return $null
}

$drive = $null
$deadline = (Get-Date).AddSeconds($WaitSeconds)
do {
    $drive = Get-PicoBootDrive
    if ($drive) { break }
    Start-Sleep -Milliseconds 500
} while ((Get-Date) -lt $deadline)

if (-not $drive) {
    Write-Error "RPI-RP2 drive not found within $WaitSeconds seconds. Hold BOOTSEL while connecting USB."
    exit 2
}

$destination = Join-Path $drive (Split-Path $Firmware -Leaf)
Write-Host "BOOTSEL drive: $drive"
Write-Host 'Copying firmware...'
try {
    Copy-Item -LiteralPath $Firmware -Destination $destination -Force -ErrorAction Stop
    Write-Host 'UF2 transfer completed. The RP2040 should now reboot into the new firmware.' -ForegroundColor Green
} catch {
    Write-Error "UF2 transfer failed: $($_.Exception.Message)"
    exit 3
}
