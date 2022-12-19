param([string[]]$Export = @())

# Files to export, relative to this script's directory
$files = [ordered]@{
    'skin'        = 'src/core/res/skin.ai';
    'system'      = 'src/core/res/system.ai';
    'layout-grid' = 'src/core/res/layout-grid.ai';
}

# Scales to export
$scales = @(
    1.00, 1.25, 1.50, 1.75,
    2.00, 2.25, 2.50, 2.75,
    3.00, 3.25, 3.50, 3.75,
    4.00
)

# *************************************************************************
# **                 !!! DO NOT EDIT BELOW THIS LINE !!!                 **
# *************************************************************************

$filesToExport =
if ($Export.Count -eq 0) {
    $files.Values
} else {
    $files[$Export]
}

if (($filesToExport.Count -eq 0) -or ($scales.Count -eq 0)) { return }

Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;

namespace Windower
{
    public static class ComUtils
    {
        public static object GetActiveObject(string progId)
        {
            if (progId == null)
            {
                throw new ArgumentNullException("progId");
            }

            Guid clsid;
            var hresult = CLSIDFromProgIDEx(progId, out clsid);
            if (hresult < 0)
            {
                Marshal.ThrowExceptionForHR(hresult);
            }

            object obj;
            hresult = GetActiveObject(clsid, IntPtr.Zero, out obj);
            if (hresult < 0)
            {
                Marshal.ThrowExceptionForHR(hresult);
            }

            return obj;
        }

        [DllImport("ole32")]
        private static extern int CLSIDFromProgIDEx(
            [MarshalAs(UnmanagedType.LPWStr)] string lpszProgID,
            out Guid lpclsid);

        [DllImport("oleaut32")]
        private static extern int GetActiveObject(
            [MarshalAs(UnmanagedType.LPStruct)] Guid rclsid,
            IntPtr pvReserved,
            [MarshalAs(UnmanagedType.IUnknown)] out object ppunk);
    }
}
"@

try {
    $illustrator = [Windower.ComUtils]::GetActiveObject(
        "Illustrator.Application")
}
catch {
    Write-Host -NoNewline "Starting Adobe Illustrator... "
    try {
        $illustrator = New-Object -ComObject "Illustrator.Application"
        Write-Host -ForegroundColor Green "OK"
    } catch {
        Write-Host -ForegroundColor Red "FAILED"
        throw
    }
    $closeIllustrator = $true
}

$root = Split-Path $script:MyInvocation.MyCommand.Path

$options = New-Object -ComObject "Illustrator.ExportOptionsPNG24"
$options.antiAliasing = $true;
$options.transparency = $true;
$options.artBoardClipping = $true;

function Export-Document
{
    param($document, $base)

    $scales | ForEach-Object {
        $scale = [int]($_ * 100)
        $outFile = "$base.$scale.png"
        $destination = $ExecutionContext.SessionState.Path.
            GetUnresolvedProviderPathFromPSPath((Join-Path $root $outFile))

        Write-Host -NoNewline "Exporting $outFile... "
        try {
            $options.verticalScale = $scale;
            $options.horizontalScale = $scale;
            $document.Export($destination, 5 <#aiPNG24#>, $options)
            Write-Host -ForegroundColor Green "OK"
        } catch {
            Write-Host -ForegroundColor Red "FAILED"
            throw
        }
    }
}

$filesToExport | ForEach-Object {
    $file = Join-Path '.' $_
    $path = Resolve-Path (Join-Path $root $file)
    $base = [System.IO.Path]::Combine(
        (Split-Path $file),
        [System.IO.Path]::GetFileNameWithoutExtension($file))

    $document = $illustrator.Documents |
        Where-Object { $_.FullName -ieq $path } |
        Select-Object -First 1
    if ($null -eq $document) {
        Write-Host -NoNewline "Opening $file... "
        try {
            $document = $illustrator.open($path)
            $closeDocument = $true
            Write-Host -ForegroundColor Green "OK"
        } catch {
            Write-Host -ForegroundColor Red "FAILED"
            throw
        }
    }

    $layers = $document.Layers
    $stableLayers = $layers | Where-Object { $_.Name -eq '<stable>' }
    $testLayers = $layers | Where-Object { $_.Name -eq '<test>' }

    if ($stableLayers.Count -gt 0 -or $testLayers.Count -gt 0) {
        $stableLayers | ForEach-Object { $_.Visible = $true }
        $testLayers | ForEach-Object { $_.Visible = $false }
        $stableBase = Join-Path `
            (Join-Path (Split-Path $base) 'stable') `
            (Split-Path $base -Leaf)
        Export-Document $document $stableBase

        $stableLayers | ForEach-Object { $_.Visible = $false }
        $testLayers | ForEach-Object { $_.Visible = $true }
        $testBase = Join-Path `
            (Join-Path (Split-Path $base) 'test') `
            (Split-Path $base -Leaf)
        Export-Document $document $testBase
    } else {
        Export-Document $document $base
    }

    if ($closeDocument) {
        Write-Host -NoNewline "Closing $file... "
        try {
            $document.Close(2 <#aiDoNotSaveChanges#>)
            Write-Host -ForegroundColor Green "OK"
        } catch {
            Write-Host -ForegroundColor Red "FAILED"
            throw
        }
    }
}

if ($closeIllustrator) {
    Write-Host -NoNewline "Closing Adobe Illustrator... "
    try {
        $illustrator.Quit()
        Write-Host -ForegroundColor Green "OK"
    }
    catch {
        Write-Host -ForegroundColor Red "FAILED"
        throw
    }
}

[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($options)
[void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($illustrator)

Write-Host "Export complete!"
