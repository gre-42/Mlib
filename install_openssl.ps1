Push-Location $PSScriptRoot

choco install openssl -y

# Write-Host "Download Win64 OpenSSL"
# Invoke-WebRequest https://slproweb.com/download/Win64OpenSSL-4_0_0.exe -OutFile Win64OpenSSL-4_0_0.exe
# 
# Write-Host "Install Win64 OpenSSL"
# # From: https://community.chocolatey.org/packages/openssl#files
# .\Win64OpenSSL-4_0_0.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-
# Write-Host "Finished installation of Win64 OpenSSL"

Pop-Location
