param(
    [ValidateSet("serve", "build")]
    [string]$Mode
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$mkdocsExe = Join-Path $repoRoot ".venv\Scripts\mkdocs.exe"
$usePythonModule = $false
$mkdocsConfig = Join-Path $scriptDir "mkdocs.yml"
$sourceReadme = Join-Path $repoRoot "README.md"
$docsDir = Join-Path $scriptDir "Manual"
$docsIndex = Join-Path $docsDir "index.md"
$docsReadme = Join-Path $docsDir "README.md"
$manualOutputDir = Join-Path $repoRoot "Manual"
$siteDir = Join-Path $repoRoot "docs"
$manualFileName = "pse84-introductory-ecosystem-e2-training-manual.md"
$sourcePresentationDir = Join-Path $repoRoot "Presentation"
$internalPresentationDir = Join-Path $docsDir "Presentation"
$internalManual = Join-Path $docsDir $manualFileName
$publicManual = Join-Path $repoRoot (Join-Path "Manual" $manualFileName)
$internalAssets = Join-Path $docsDir "assets"
$publicAssets = Join-Path $repoRoot "Manual\assets"
$excludedExtensions = @(".doc", ".docx")
$manualLink = "./Manual/$manualFileName"
$indexLink = "./$manualFileName"

if (-not (Test-Path $mkdocsExe)) {
    $mkdocsExe = "python"
    $usePythonModule = $true
}

if (-not (Test-Path $sourceReadme)) {
    throw "Source README.md not found at: $sourceReadme"
}

if (-not (Test-Path $docsDir)) {
    throw "Docs directory not found at: $docsDir"
}

if (-not (Test-Path $mkdocsConfig)) {
    throw "MkDocs config not found at: $mkdocsConfig"
}

if ([string]::IsNullOrWhiteSpace($Mode)) {
    do {
        $Mode = (Read-Host "Enter mode (serve/build)").Trim().ToLowerInvariant()
    } while ($Mode -notin @("serve", "build"))
}

Copy-Item -Path $internalManual -Destination $publicManual -Force
Write-Host "Published lab manual markdown: $internalManual -> $publicManual"

if (Test-Path $sourcePresentationDir) {
    if (Test-Path $internalPresentationDir) {
        Remove-Item -Path $internalPresentationDir -Recurse -Force
    }
    New-Item -Path $internalPresentationDir -ItemType Directory -Force | Out-Null
    Copy-Item -Path (Join-Path $sourcePresentationDir '*') -Destination $internalPresentationDir -Recurse -Force
    Write-Host "Published presentation assets: $sourcePresentationDir -> $internalPresentationDir"
}

if (Test-Path $internalAssets) {
    if (-not (Test-Path $publicAssets)) {
        New-Item -Path $publicAssets -ItemType Directory -Force | Out-Null
    }
    Copy-Item -Path (Join-Path $internalAssets '*') -Destination $publicAssets -Recurse -Force
    Write-Host "Published lab manual assets: $internalAssets -> $publicAssets"
}

(Get-Content $sourceReadme) `
    -replace [regex]::Escape($manualLink), $indexLink | Set-Content $docsIndex -Force
Write-Host "Copied README with updated link: $sourceReadme -> $docsIndex"

if (Test-Path $docsReadme) {
    Remove-Item -Path $docsReadme -Force
    Write-Host "Removed duplicate docs README: $docsReadme"
}

if ($Mode -eq "serve") {
    Write-Host "Starting MkDocs serve..."
    if ($usePythonModule) {
        & $mkdocsExe -m mkdocs serve --config-file $mkdocsConfig
    }
    else {
        & $mkdocsExe serve --config-file $mkdocsConfig
    }
    exit $LASTEXITCODE
}

Write-Host "Running MkDocs build..."
if ($usePythonModule) {
    & $mkdocsExe -m mkdocs build --config-file $mkdocsConfig
}
else {
    & $mkdocsExe build --config-file $mkdocsConfig
}
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Get-ChildItem -Path $manualOutputDir -Recurse -File -Force |
    Where-Object { $excludedExtensions -contains $_.Extension.ToLowerInvariant() } |
    ForEach-Object {
        Remove-Item -Path $_.FullName -Force
        Write-Host "Removed excluded artifact: $($_.FullName)"
    }

Write-Host "MkDocs output available at: $siteDir"
