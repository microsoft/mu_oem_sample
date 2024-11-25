


# Get the certificate file content and compute SHA256 hash
$certPath = "./Test/Certs/TestCert.cer"
$certContent = Get-Content -Path $certPath -Encoding Byte
$sha256 = [System.Security.Cryptography.SHA256]::Create()
$hashBytes = $sha256.ComputeHash($certContent)

# Convert hash to hex string for the comment
$hashHex = ($hashBytes | ForEach-Object { "{0:x2}" -f $_ }) -join ""

# Convert to C array format with proper formatting
$cArrayElements = $hashBytes | ForEach-Object { "0x{0:X2}" -f $_ }

# Create formatted output
$formattedOutput = "  {   // SHA256: $hashHex (Test/Certs/TestCert.cer)`n"
$formattedOutput += "    "

# Add elements in groups of 16 per line
for ($i = 0; $i -lt $cArrayElements.Count; $i++) {
    if ($i -gt 0 -and $i % 16 -eq 0) {
        $formattedOutput += ",`n    "
    } elseif ($i -gt 0) {
        $formattedOutput += ", "
    }
    $formattedOutput += $cArrayElements[$i]
}

$formattedOutput += "`n  },"

# Print the formatted output
Write-Host "`nCertificate SHA256 hash as C array:"
Write-Host $formattedOutput
