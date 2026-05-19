# resolve_ghidra_names.ps1
# Resolves FUN_/DAT_ Ghidra placeholders in docs/fa/ against fa_symbols.csv
# Usage: .\scripts\resolve_ghidra_names.ps1 [-Apply]
param(
    [switch]$Apply
)

$symbolsCsv = "C:\Users\John\src\fa\fa_symbols.csv"
$globalsCsv = "C:\Users\John\src\fa\output\DumpGlobals_named.csv"
$docsRoot   = "$PSScriptRoot\..\docs\fa"

# --- Build address -> clean name map ---
function Undecorate-Name([string]$raw) {
    # Strip stdcall/cdecl decoration: leading _ and trailing @N
    if ($raw -match '^_([A-Za-z_][A-Za-z0-9_]*)@\d+$') { return $Matches[1] }
    if ($raw -match '^_([A-Za-z_][A-Za-z0-9_]*)$')      { return $Matches[1] }
    # Watcom @name@N
    if ($raw -match '^@([A-Za-z_][A-Za-z0-9_]*)@\d+$')  { return $Matches[1] }
    # MSVC C++ mangled: extract base name between ? and @@
    if ($raw -match '^\?([A-Za-z_][A-Za-z0-9_]*)@@')     { return $Matches[1] }
    # Already clean
    return $raw
}

$addrMap = @{}
Import-Csv $symbolsCsv | ForEach-Object {
    $va   = $_.va.ToLower().TrimStart('0x').PadLeft(8, '0')
    $name = Undecorate-Name $_.name
    $addrMap[$va] = $name
}

# Also pull named globals (functions that appear as data refs)
Import-Csv $globalsCsv | ForEach-Object {
    $va = $_.address.ToLower().TrimStart('0x').PadLeft(8, '0')
    if (-not $addrMap.ContainsKey($va)) {
        $addrMap[$va] = Undecorate-Name $_.name
    }
}

Write-Host "Loaded $($addrMap.Count) named symbols."

# --- Scan docs ---
$mdFiles = Get-ChildItem $docsRoot -Filter "*.md" -Recurse
$pattern = '(FUN_|DAT_)([0-9a-fA-F]+)'

$resolved   = [System.Collections.Generic.List[object]]::new()
$unresolved = [System.Collections.Generic.List[object]]::new()

foreach ($file in $mdFiles) {
    $content = Get-Content $file.FullName -Raw
    $hits = [regex]::Matches($content, $pattern)
    foreach ($m in $hits) {
        $prefix  = $m.Groups[1].Value
        $hexAddr = $m.Groups[2].Value.ToLower().PadLeft(8, '0')
        $key     = $hexAddr
        if ($addrMap.ContainsKey($key)) {
            $resolved.Add([pscustomobject]@{
                File    = $file.Name
                Token   = $m.Value
                Address = "0x$hexAddr"
                Name    = $addrMap[$key]
            })
        } else {
            $unresolved.Add([pscustomobject]@{
                File    = $file.Name
                Token   = $m.Value
                Address = "0x$hexAddr"
            })
        }
    }
}

$resolvedUnique   = $resolved   | Sort-Object Token -Unique
$unresolvedUnique = $unresolved | Sort-Object Token -Unique

Write-Host ""
Write-Host "=== RESOLVABLE ($($resolvedUnique.Count) unique tokens) ==="
$resolvedUnique | Format-Table Token, Name, File -AutoSize

Write-Host ""
Write-Host "=== UNRESOLVABLE ($($unresolvedUnique.Count) unique tokens) ==="
$unresolvedUnique | Format-Table Token, Address, File -AutoSize

# --- Apply replacements ---
if ($Apply) {
    Write-Host ""
    Write-Host "Applying replacements..."
    $changes = 0
    foreach ($file in $mdFiles) {
        $original = Get-Content $file.FullName -Raw
        $updated  = $original
        $hits = [regex]::Matches($original, $pattern) | Sort-Object Index -Descending
        foreach ($m in $hits) {
            $hexAddr = $m.Groups[2].Value.ToLower().PadLeft(8, '0')
            if ($addrMap.ContainsKey($hexAddr)) {
                $replacement = $addrMap[$hexAddr]
                $updated = $updated.Remove($m.Index, $m.Length).Insert($m.Index, $replacement)
            }
        }
        if ($updated -ne $original) {
            Set-Content $file.FullName $updated -NoNewline
            Write-Host "  Updated: $($file.Name)"
            $changes++
        }
    }
    Write-Host "Done. $changes file(s) modified."
}
