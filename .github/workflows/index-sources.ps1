$tools = Join-Path ${env:ProgramFiles(x86)} "Windows Kits"
$srctool = Get-ChildItem $tools -Filter "srctool.exe" -Recurse |
            Where-Object { $_.FullName -Match '[/\\]x86[/\\]' }
$pdbstr = Get-ChildItem $tools -Filter "pdbstr.exe" -Recurse |
            Where-Object { $_.FullName -Match '[/\\]x86[/\\]' }

$outDir = Join-Path "build" "bin" | Join-Path -ChildPath "release"

Get-ChildItem -Path $outDir -File -Recurse -Filter "*.pdb" |
    ForEach-Object {
        "Indexing sources for $(
            (Resolve-Path $_.FullName -Relative).Replace(".\", ''))..."

        $index = New-TemporaryFile
        $url = "https://raw.githubusercontent.com/" +
            "$env:GITHUB_REPOSITORY/$env:GITHUB_SHA/"

        ("SRCSRV: ini ------------------------------------------------",
            "VERSION=2",
            "VERCTRL=http",
            "SRCSRV: variables ------------------------------------------",
            "HTTP_ALIAS=$url",
            "HTTP_EXTRACT_TARGET=%HTTP_ALIAS%%var2%",
            "SRCSRVTRG=%HTTP_EXTRACT_TARGET%",
            "SRCSRV: source files ---------------------------------------") |
            Out-File $index.FullName -Encoding "ASCII"

        & $srctool -r $_.FullName |
            Where-Object { Test-Path $_ -PathType Leaf } |
            ForEach-Object {
                try {
                    $cleaned = & git ls-files $_ 2>$null
                    if ($cleaned.Length -ne 0) {
                        "    $url$cleaned"
                        "$($_.path)*$cleaned" |
                            Out-File -Append $index.FullName -Encoding "ASCII"
                    }
                } catch {}
            }

        "SRCSRV: end ------------------------------------------------" |
            Out-File -Append $index.FullName -Encoding "ASCII"

        & $pdbstr -w -p:$_.FullName -s:srcsrv -i:$index.FullName

        Remove-Item $index.FullName -Force

        ""
    }
