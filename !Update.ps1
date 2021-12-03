# args
param (
    [ValidateSet('COPY', 'SOURCEGEN', 'DISTRIBUTE')][string]$Mode,
    [string]$Version,
    [string]$Path,
    [string]$Project,
    [string]$IsAE
)

$ErrorActionPreference = "Stop"

function Resolve-Files {
    param (
        [Parameter(ValueFromPipeline)]
        [string]$a_parent = $PSScriptRoot,

        [Parameter()]
        [string[]]$a_directory = @('include', 'src', 'test'),

        [Parameter()]
        [string[]]$a_extension = '*.c,*.cpp,*.cxx,*.h,*.hpp,*.hxx'
    )
    
    process {
        Push-Location $PSScriptRoot # out of source invocation sucks
        $_generated = ""

        try {
            foreach ($directory in $a_directory) {
                Write-Host "`t<$a_parent/$directory>"
                Get-ChildItem "$a_parent/$directory" -Recurse -File -Include $a_extention -Exclude Version.h -ErrorAction SilentlyContinue | Resolve-Path -Relative | ForEach-Object {
                    Write-Host "`t`t<$_>"
                    $file = $_.SubString(2).Insert(0, "`n`t") -replace '\\', '/' # \\/\//\/\\/\\\/\\/?!?
                    $_generated += $file
                }
            }
        } finally {
            Pop-Location
        }

        return $_generated
    }
}

# project path
$Folder = $PSScriptRoot | Split-Path -Leaf
$GameBase
$Destination
$MO2

# operation
Write-Host "`n`t<$Folder> [$Mode] BEGIN"
if ($Mode -eq 'COPY') { # post build event
    $vcpkg = [IO.File]::ReadAllText("$PSScriptRoot/vcpkg.json") | ConvertFrom-Json
    # Build Target
    if ($IsAE -eq 'TRUE') {
        $GameBase = $env:SkyrimAEPath
        $MO2 = $env:MO2SkyrimAEPath
        Write-Host "`t$Folder $Version | ANNIVERSARY EDITION"
    } elseif ($IsAE -eq 'FALSE') {
        $GameBase = $env:SkyrimSEPath
        $MO2 = $env:MO2SkyrimSEPath
        Write-Host "`t$Folder $Version | SPECIAL EDITION"
    }

    # MO2 support
    if ($MO2) {
        $Destination = Join-Path "$MO2/mods" $vcpkg.'install-name'
        New-Item -Type Directory "$Destination/SKSE/Plugins" -Force | Out-Null
    } else {
        Add-Type -AssemblyName Microsoft.VisualBasic | Out-Null
        $Result = [Microsoft.VisualBasic.Interaction]::MsgBox("$Project has been built`n`nCopy to game folder?", 52, $Project)
        if ($Result -eq 6) {
            $Destination = "$GameBase/Data" 
            New-Item -Type Directory "$Destination/SKSE/Plugins" -Force | Out-Null
        } else {
            Invoke-Item $Path
            Exit
        }
    }
       
    # binary
    Write-Host "`tCopying binary file..."
    Copy-Item "$Path/$Project.dll" "$Destination/SKSE/Plugins/$Project.dll" -Force
    Write-Host "`tDone!"

    # configs
    Get-ChildItem $PSScriptRoot -Recurse | Where-Object {($_.extension -in '.toml', '.json', '.toml') -and ($_.Name -ne 'vcpkg.json')} | ForEach-Object {
        Write-Host "`tCopying $($_.Name)"
        Copy-Item $_.FullName "$Destination/SKSE/Plugins/$($_.Name)" -Force
    }

    # papyrus
    if (Test-Path "$PSScriptRoot/Scripts/Source/*.psc" -PathType Leaf) {
        Add-Type -AssemblyName Microsoft.VisualBasic | Out-Null
        $Result = [Microsoft.VisualBasic.Interaction]::MsgBox('Build papyrus scripts?', 52, 'Papyrus')
        if ($Result -eq 6) {
            Write-Host "`tBuilding papyrus scripts..."
            New-Item -Type Directory "$Destination/Scripts" -Force | Out-Null
            & "$GameBase/Papyrus Compiler/PapyrusCompiler.exe" "$PSScriptRoot/Scripts/Source" -f="$GameBase/Papyrus Compiler/TESV_Papyrus_Flags.flg" -i="$GameBase/Data/Scripts/Source;./Scripts/Source" -o="$PSScriptRoot/Scripts" -a

            Write-Host "`tCopying papyrus scripts..."
            Copy-Item "$PSScriptRoot/Scripts" "$Destination" -Recurse -Force
            Remove-Item "$Destination/Scripts/Source" -Force -Confirm:$false -ErrorAction Ignore
            Write-Host "`tDone!"
        }
    }
    
    # shockwave
    if (Test-Path "$PSScriptRoot/Interface/*.swf" -PathType Leaf) {
        Write-Host "`tCopying shockwave files..."
        New-Item -Type Directory "$Destination/Interface" -Force | Out-Null
        Copy-Item "$PSScriptRoot/Interface" "$Destination" -Recurse -Force
        Write-Host "`tDone!"
    }

    # Check CMake VERSION
    $OutputVersion
    $OriginalVersion = $vcpkg.'version-string'
    $BuildFolder = Get-ChildItem ((Get-Item $Path).Parent.Parent.FullName) "$Project.sln" -Recurse -File
    [IO.File]::ReadAllLines("$($BuildFolder.Directory)/include/Version.h") | ForEach-Object {
        if ($_.Trim().StartsWith('inline constexpr auto NAME = "')) {
            $OutputVersion = $_.Trim().Substring(30, 5)
            if ($OutputVersion -ne $vcpkg.'version-string') {
                Write-Host "`tVersionInfo changed! Updating CMakeLists..."  

                $CMakeLists = [IO.File]::ReadAllText("$PSScriptRoot/CMakeLists.txt") -replace "VERSION\s$($vcpkg.'version-string')", "VERSION $OutputVersion"
                [IO.File]::WriteAllText("$PSScriptRoot/CMakeLists.txt", $CMakeLists)

                $vcpkg.'version-string' = $OutputVersion
                $vcpkg = $vcpkg | ConvertTo-Json
                [IO.File]::WriteAllText("$PSScriptRoot/vcpkg.json", $vcpkg)
                Write-Host "`tDone  "
                
                Add-Type -AssemblyName Microsoft.VisualBasic | Out-Null
                [Microsoft.VisualBasic.Interaction]::MsgBox("$Project has been changed from $($OriginalVersion) to $($OutputVersion)`n`nThis update will be in effect after next succesful build", 48, 'Version Update')
                Exit
            }
        }
    }
} elseif ($Mode -eq 'SOURCEGEN') { # cmake sourcelist generation
    Write-Host "`tGenerating CMake sourcelist..."
    Remove-Item "$Path/sourcelist.cmake" -Force -Confirm:$false -ErrorAction Ignore

    $generated = 'set(SOURCES'
    $generated += $PSScriptRoot | Resolve-Files
    if ($Path) {
        $generated += $Path | Resolve-Files
    }
    $generated += "`n)"
    [IO.File]::WriteAllText("$Path/sourcelist.cmake", $generated)

    if ($Version) {
        # update vcpkg.json accordinly
        $vcpkg = [IO.File]::ReadAllText("$PSScriptRoot/vcpkg.json") | ConvertFrom-Json
        $vcpkg.'version-string' = $Version    
        
        if ($env:RebuildInvoke) {
            if ($vcpkg | Get-Member script-version) {
                $vcpkg.'script-version' = $env:DKScriptVersion
            } else {
                $vcpkg | Add-Member -Name 'script-version' -Value $env:DKScriptVersion -MemberType NoteProperty
            }
            if ($vcpkg | Get-Member build-config) {
                $vcpkg.'build-config' = $env:BuildConfig
            } else {
                $vcpkg | Add-Member -Name 'build-config' -Value $env:BuildConfig -MemberType NoteProperty
            }
            if ($vcpkg | Get-Member build-target) {
                $vcpkg.'build-target' = $env:BuildTarget
            } else {
                $vcpkg | Add-Member -Name 'build-target' -Value $env:BuildTarget -MemberType NoteProperty
            }
            if (-not ($vcpkg | Get-Member install-name)) {
                $vcpkg | Add-Member -Name 'install-name' -Value $Folder -MemberType NoteProperty
            }
        }
        $vcpkg = $vcpkg | ConvertTo-Json
        [IO.File]::WriteAllText("$PSScriptRoot/vcpkg.json", $vcpkg) # damn you encoding
    }
} elseif ($Mode -eq 'DISTRIBUTE') { # update script to every project
    ((Get-ChildItem 'Plugins' -Directory -Recurse) + (Get-ChildItem 'Library' -Directory -Recurse)) | Resolve-Path -Relative | ForEach-Object {
        if (Test-Path "$_/CMakeLists.txt" -PathType Leaf) {
            Write-Host "`tUpdated <$_>"
            Robocopy.exe "$PSScriptRoot" "$_" '!Update.ps1' /MT /NJS /NFL /NDL /NJH
        }
    }
}

Write-Host "`t<$Folder> [$Mode] DONE"

# SIG # Begin signature block
# MIIR2wYJKoZIhvcNAQcCoIIRzDCCEcgCAQExCzAJBgUrDgMCGgUAMGkGCisGAQQB
# gjcCAQSgWzBZMDQGCisGAQQBgjcCAR4wJgIDAQAABBAfzDtgWUsITrck0sYpfvNR
# AgEAAgEAAgEAAgEAAgEAMCEwCQYFKw4DAhoFAAQUyIHw9odQ5oHlqbxr6bcnkKUT
# 8pSggg1BMIIDBjCCAe6gAwIBAgIQbBejp82dcLdHI5AVyyqyxzANBgkqhkiG9w0B
# AQsFADAbMRkwFwYDVQQDDBBES1NjcmlwdFNlbGZDZXJ0MB4XDTIxMTIwMzExMTgx
# OVoXDTIyMTIwMzExMzgxOVowGzEZMBcGA1UEAwwQREtTY3JpcHRTZWxmQ2VydDCC
# ASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKCLTioNBJsXmC6rmQ9af4DL
# 0+zXaFKtkDFaOzLbiDB17sVAgkGjC8uSQ29qK0gr934ekXWSkk3a2QWfVUz+6uKJ
# kgc5d2yRzXItO+8Y83zXHW5xEfqA65ukCEKhoNN8y6iVq9iTYYD3Yv1hNfSSLhsj
# RICd2vkyTm0zwwh69nWMqz6AMcLr4PiNMbO/1yv6bi2lSXFfhWYjnJEKFezMv1fi
# uf85XmXYl08uqRK1NWQJASAbI3azCwR2kNSWamoz8OuBcKEvO+xdsv3UGION5jwt
# 1YyyzEauCzl3rwBU1GHeubhcz4iZqJ7Wb47bOhQBpHkLqrBNzxoVjNBx2aJ7Qz0C
# AwEAAaNGMEQwDgYDVR0PAQH/BAQDAgeAMBMGA1UdJQQMMAoGCCsGAQUFBwMDMB0G
# A1UdDgQWBBRYLE0TxN1kYtIniUU4xnRIZxovITANBgkqhkiG9w0BAQsFAAOCAQEA
# dRa495+I6eK8hxMbFkP9sRWD1ZWw4TPyGWTCBpDKkJ8mUm7SSwnZgfiZ78C6P3AQ
# D+unSQLvTwN+0PISQti0TMf3Sy+92UPyEQVKk/Wky0tZrYWje8DSayEu72SwTtUn
# GhzAMGe7roDCe8+Q4YFAKh8HH3Fz70eJQnBCNewJfiI0tVBav/bCaPfjWKdlYMoi
# LsCHVYYzLfmZWLN6fhWY4NT1F3OBCoDvqvBTUupsknzIQIkR0kl0hvyiTKuTgKmZ
# xJoYX3MvXEBZMs/WUaTDXOt4tLe7viye6T2RUeILyJiuq5PDzM6X1tUbgQEXOFhQ
# dOHtYjGwteueqI+Usmp3cDCCBP4wggPmoAMCAQICEA1CSuC+Ooj/YEAhzhQA8N0w
# DQYJKoZIhvcNAQELBQAwcjELMAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0
# IEluYzEZMBcGA1UECxMQd3d3LmRpZ2ljZXJ0LmNvbTExMC8GA1UEAxMoRGlnaUNl
# cnQgU0hBMiBBc3N1cmVkIElEIFRpbWVzdGFtcGluZyBDQTAeFw0yMTAxMDEwMDAw
# MDBaFw0zMTAxMDYwMDAwMDBaMEgxCzAJBgNVBAYTAlVTMRcwFQYDVQQKEw5EaWdp
# Q2VydCwgSW5jLjEgMB4GA1UEAxMXRGlnaUNlcnQgVGltZXN0YW1wIDIwMjEwggEi
# MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDC5mGEZ8WK9Q0IpEXKY2tR1zoR
# Qr0KdXVNlLQMULUmEP4dyG+RawyW5xpcSO9E5b+bYc0VkWJauP9nC5xj/TZqgfop
# +N0rcIXeAhjzeG28ffnHbQk9vmp2h+mKvfiEXR52yeTGdnY6U9HR01o2j8aj4S8b
# Ordh1nPsTm0zinxdRS1LsVDmQTo3VobckyON91Al6GTm3dOPL1e1hyDrDo4s1SPa
# 9E14RuMDgzEpSlwMMYpKjIjF9zBa+RSvFV9sQ0kJ/SYjU/aNY+gaq1uxHTDCm2mC
# tNv8VlS8H6GHq756WwogL0sJyZWnjbL61mOLTqVyHO6fegFz+BnW/g1JhL0BAgMB
# AAGjggG4MIIBtDAOBgNVHQ8BAf8EBAMCB4AwDAYDVR0TAQH/BAIwADAWBgNVHSUB
# Af8EDDAKBggrBgEFBQcDCDBBBgNVHSAEOjA4MDYGCWCGSAGG/WwHATApMCcGCCsG
# AQUFBwIBFhtodHRwOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHwYDVR0jBBgwFoAU
# 9LbhIB3+Ka7S5GGlsqIlssgXNW4wHQYDVR0OBBYEFDZEho6kurBmvrwoLR1ENt3j
# anq8MHEGA1UdHwRqMGgwMqAwoC6GLGh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9z
# aGEyLWFzc3VyZWQtdHMuY3JsMDKgMKAuhixodHRwOi8vY3JsNC5kaWdpY2VydC5j
# b20vc2hhMi1hc3N1cmVkLXRzLmNybDCBhQYIKwYBBQUHAQEEeTB3MCQGCCsGAQUF
# BzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5jb20wTwYIKwYBBQUHMAKGQ2h0dHA6
# Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydFNIQTJBc3N1cmVkSURUaW1l
# c3RhbXBpbmdDQS5jcnQwDQYJKoZIhvcNAQELBQADggEBAEgc3LXpmiO85xrnIA6O
# Z0b9QnJRdAojR6OrktIlxHBZvhSg5SeBpU0UFRkHefDRBMOG2Tu9/kQCZk3taaQP
# 9rhwz2Lo9VFKeHk2eie38+dSn5On7UOee+e03UEiifuHokYDTvz0/rdkd2NfI1Jp
# g4L6GlPtkMyNoRdzDfTzZTlwS/Oc1np72gy8PTLQG8v1Yfx1CAB2vIEO+MDhXM/E
# EXLnG2RJ2CKadRVC9S0yOIHa9GCiurRS+1zgYSQlT7LfySmoc0NR2r1j1h9bm/cu
# G08THfdKDXF+l7f0P4TrweOjSaH6zqe/Vs+6WXZhiV9+p7SOZ3j5NpjhyyjaW4em
# ii8wggUxMIIEGaADAgECAhAKoSXW1jIbfkHkBdo2l8IVMA0GCSqGSIb3DQEBCwUA
# MGUxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsT
# EHd3dy5kaWdpY2VydC5jb20xJDAiBgNVBAMTG0RpZ2lDZXJ0IEFzc3VyZWQgSUQg
# Um9vdCBDQTAeFw0xNjAxMDcxMjAwMDBaFw0zMTAxMDcxMjAwMDBaMHIxCzAJBgNV
# BAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdp
# Y2VydC5jb20xMTAvBgNVBAMTKERpZ2lDZXJ0IFNIQTIgQXNzdXJlZCBJRCBUaW1l
# c3RhbXBpbmcgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC90DLu
# S82Pf92puoKZxTlUKFe2I0rEDgdFM1EQfdD5fU1ofue2oPSNs4jkl79jIZCYvxO8
# V9PD4X4I1moUADj3Lh477sym9jJZ/l9lP+Cb6+NGRwYaVX4LJ37AovWg4N4iPw7/
# fpX786O6Ij4YrBHk8JkDbTuFfAnT7l3ImgtU46gJcWvgzyIQD3XPcXJOCq3fQDpc
# t1HhoXkUxk0kIzBdvOw8YGqsLwfM/fDqR9mIUF79Zm5WYScpiYRR5oLnRlD9lCos
# p+R1PrqYD4R/nzEU1q3V8mTLex4F0IQZchfxFwbvPc3WTe8GQv2iUypPhR3EHTyv
# z9qsEPXdrKzpVv+TAgMBAAGjggHOMIIByjAdBgNVHQ4EFgQU9LbhIB3+Ka7S5GGl
# sqIlssgXNW4wHwYDVR0jBBgwFoAUReuir/SSy4IxLVGLp6chnfNtyA8wEgYDVR0T
# AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwEwYDVR0lBAwwCgYIKwYBBQUH
# AwgweQYIKwYBBQUHAQEEbTBrMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdp
# Y2VydC5jb20wQwYIKwYBBQUHMAKGN2h0dHA6Ly9jYWNlcnRzLmRpZ2ljZXJ0LmNv
# bS9EaWdpQ2VydEFzc3VyZWRJRFJvb3RDQS5jcnQwgYEGA1UdHwR6MHgwOqA4oDaG
# NGh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEFzc3VyZWRJRFJvb3RD
# QS5jcmwwOqA4oDaGNGh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEFz
# c3VyZWRJRFJvb3RDQS5jcmwwUAYDVR0gBEkwRzA4BgpghkgBhv1sAAIEMCowKAYI
# KwYBBQUHAgEWHGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwCwYJYIZIAYb9
# bAcBMA0GCSqGSIb3DQEBCwUAA4IBAQBxlRLpUYdWac3v3dp8qmN6s3jPBjdAhO9L
# hL/KzwMC/cWnww4gQiyvd/MrHwwhWiq3BTQdaq6Z+CeiZr8JqmDfdqQ6kw/4stHY
# fBli6F6CJR7Euhx7LCHi1lssFDVDBGiy23UC4HLHmNY8ZOUfSBAYX4k4YU1iRiSH
# Y4yRUiyvKYnleB/WCxSlgNcSR3CzddWThZN+tpJn+1Nhiaj1a5bA9FhpDXzIAbG5
# KHW3mWOFIoxhynmUfln8jA/jb7UBJrZspe6HUSHkWGCbugwtK22ixH67xCUrRwII
# fEmuE7bhfEJCKMYYVs9BNLZmXbZ0e/VWMyIvIjayS6JKldj1po5SMYIEBDCCBAAC
# AQEwLzAbMRkwFwYDVQQDDBBES1NjcmlwdFNlbGZDZXJ0AhBsF6OnzZ1wt0cjkBXL
# KrLHMAkGBSsOAwIaBQCgeDAYBgorBgEEAYI3AgEMMQowCKACgAChAoAAMBkGCSqG
# SIb3DQEJAzEMBgorBgEEAYI3AgEEMBwGCisGAQQBgjcCAQsxDjAMBgorBgEEAYI3
# AgEVMCMGCSqGSIb3DQEJBDEWBBQqnYisw/92tz41NvNRj7BAFWVF5jANBgkqhkiG
# 9w0BAQEFAASCAQBJdIZj7QvhAk+KLv/2skIiuK/arjTwuV/znDG3YtIw/y//WM8P
# GzzTQ97DKaax0YwS0oLgUJf30FYvwrnod1Ro/EOXlayxpTTTsoX6hV3een69xIg1
# nQC/aq8v4hNYr/2A4D+/KmgJf51ly1EhZrj6XMDAnUuaULXADnp6RM+JL+yhmPnL
# OKeIF6TyKisl/TGYIuzgCcyE5DvCf9PyephLWUdq6lJbI2vgccxyloyoxc5FNVb2
# Pm2fnrUpf/DZv/tGb7p0iJJNGU9Nl69c28qFJJzew0hIXQxvu0C32GVMzvdigwoI
# e72BwzxHG8o+mM/JPnUzm9EOXxL89a0eJutEoYICMDCCAiwGCSqGSIb3DQEJBjGC
# Ah0wggIZAgEBMIGGMHIxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxEaWdpQ2VydCBJ
# bmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5jb20xMTAvBgNVBAMTKERpZ2lDZXJ0
# IFNIQTIgQXNzdXJlZCBJRCBUaW1lc3RhbXBpbmcgQ0ECEA1CSuC+Ooj/YEAhzhQA
# 8N0wDQYJYIZIAWUDBAIBBQCgaTAYBgkqhkiG9w0BCQMxCwYJKoZIhvcNAQcBMBwG
# CSqGSIb3DQEJBTEPFw0yMTEyMDMxMzI5NTBaMC8GCSqGSIb3DQEJBDEiBCChf5Dy
# SG2tyE87hDWc9ZP/scg0PZK8qspf1iAiBubCFzANBgkqhkiG9w0BAQEFAASCAQC1
# SfMy2hQkFLIZd+sQXp6XQi70y7V01xZmpv6Pxck/rFGZsBVZjpDLDRJ9kzvCr0k4
# JdDmdAycHjhQ8aKVtkkzIlE+HpRgh5bPnuURR3THKB3dm63BZrLDEyy1F2dpp0+Z
# blR9KoDh6v3KthMiOkk0iadr8g5pknt9/zD2eM9ZhYWxfYp1vze2aNLxFeLysDo2
# zCR7/XNcu0ciwqzBp7sa7CJclSK1Wvk9GX7Q7ylMj/jVs2UlH1r1Jr+jckQMbRI+
# FtCSkjnA0L2gZyU8dxMVEDtV2gpHoeNB5a+MEw4cCK718LOzFKc20dElUi7DcsuM
# G6tlK7iraABETgQEHqSC
# SIG # End signature block
