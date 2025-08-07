Push-Location $PSScriptRoot

# Disabled because the OpenSSL version was outdated (07.08.2025)
# choco install openssl -y

Invoke-WebRequest https://slproweb.com/download/Win64OpenSSL-3_5_2.exe -OutFile Win64OpenSSL-3_5_2.exe
# From: https://community.chocolatey.org/packages/openssl#files
.\Win64OpenSSL-3_5_2.exe /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /SP-

Pop-Location
