$tools = Join-Path ${env:ProgramFiles(x86)} "Windows Kits"
$symstore = Get-ChildItem $tools -Filter "symstore.exe" -Recurse |
    Where-Object { $_.FullName -Match '[/\\]x86[/\\]' }

Write-Output "Downloading symbol database..."
New-Item -ItemType Directory -Force -Path './staging/symbols' *>$null
&scp -r "${env:SSH_USER}@${env:SSH_HOST}:${env:SYMBOLS_PATH}/*" './staging/symbols'

Write-Output "Updating symbol database..."
$cutoffDate = (Get-Date).AddDays(-$env:SYMBOLS_DAYS_TO_KEEP)
New-Item -ItemType Directory -Force -Path './staging/symbols' *>$null
Get-ChildItem $stagingSymbols -Directory |
    ForEach-Object {
        Get-ChildItem $_.FullName -Directory |
            Sort-Object -Property CreationTime -Descending |
            Select-Object -Skip $env:SYMBOLS_KEEP_LAST |
            Where-Object { $_.CreationTime -lt $cutoffDate } |
            Remove-Item -Recurse -Force
    }
&$symstore add /r /f './temp/symbols' /s './staging/symbols' /t '.'`
    /compress -:NOREFS
Remove-Item './staging/symbols/000Admin' -Recurse -Force -ErrorAction Ignore
