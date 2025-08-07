Push-Location $PSScriptRoot

# Disabled because the OpenSSL version was outdated (07.08.2025)
# choco install openssl -y

Write-Host "Download Win64 OpenSSL"
Invoke-WebRequest https://slproweb.com/download/Win64OpenSSL-3_5_2.exe -OutFile Win64OpenSSL-3_5_2.exe

Write-Host "Install Win64 OpenSSL"
# From: https://community.chocolatey.org/packages/openssl#files
.\Win64OpenSSL-3_5_2.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-
Write-Host "Finished installation of Win64 OpenSSL"

Pop-Location
