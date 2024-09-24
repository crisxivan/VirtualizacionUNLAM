param(
    [Parameter(Mandatory=$true)][string]$directorio,
    [string]$salida,
    [switch]$kill
)

if (-not ($salida -xor $kill)) {
    Write-Host "Debes proporcionar solo uno de los parámetros: salida o kill." -ForegroundColor Red
    exit
}

$scriptBlock = {
    param (
        [string]$directorioAbs,
        [string]$salidaAbs
    )

    $archivo = "$salidaAbs\log.txt"

    if (-not (Test-Path $archivo)) {
        New-Item -Path $archivo -ItemType File
    }

    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $directorioAbs
    $watcher.Filter = "*.*"
    $watcher.IncludeSubdirectories = $true
    $watcher.EnableRaisingEvents = $true

    $action = {
        $nuevoArch = $Event.SourceEventArgs.FullPath
        $nombre = $Event.SourceEventArgs.Name
        $tam = (Get-Item $nuevoArch).length


        $existearch = Get-ChildItem -Path $using:directorioAbs | Where-Object { $_.Name -eq $nombre -and $_.Length -eq $tam }


        if ($existearch -gt 1) {
            # Comprimir el archivo
            $comprimido = Join-Path -Path $using:salidaAbs -ChildPath "$nombre.zip"
            if (-not (Test-Path $comprimido)) {
                Compress-Archive -Path $nuevoArch -DestinationPath $comprimido
                $contenido = "$nombre es un archivo duplicado."
                $contenido >> $archivo
            }
        }
    }

    Register-ObjectEvent -InputObject $watcher -EventName Created -Action $action
    # Register-ObjectEvent -InputObject $watcher -EventName Changed -Action $action


    while ($true) {
        Start-Sleep -Seconds 1
    }
}

# Define la ruta relativa
$directorioAbs = Resolve-Path $directorio
$salidaAbs = Resolve-Path $salida

# Inicia el trabajo en segundo plano
$job = Start-Job -ScriptBlock $scriptBlock -ArgumentList $directorioAbs, $salidaAbs
# Muestra el ID del trabajo
$job.Id