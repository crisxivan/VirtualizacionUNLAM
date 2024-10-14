<#
.SYNOPSIS
    Monitorea un directorio para archivos duplicados y los comprime en un archivo ZIP.

.DESCRIPTION
    Este script utiliza el FileSystemWatcher para detectar cuando se crean o modifican archivos en el directorio especificado. 
    Si se detecta un archivo duplicado (mismo nombre y tamaño), se crea un archivo ZIP en la ubicación de salida.

.PARAMETER directorioAbs
    Ruta absoluta del directorio que se desea monitorear. Este parámetro es obligatorio.

.PARAMETER salidaAbs
    Ruta absoluta del directorio donde se guardarán los archivos comprimidos. Este parámetro es obligatorio.

.EXAMPLE
    ./ejercicio1.ps1 -directorioAbs "C:\ruta\al\directorio" -salidaAbs "C:\ruta\al\salida"

#>

#Integrantes del grupo:
#-Cespedes, Cristian Ivan -> DNI 41704776
#-Gomez, Luciano Dario -> DNI 41572055
#-Luna, Leandro Santiago -> DNI 40886200
#-Panigazzi, Agustin Fabian -> DNI 43744593

param(
    [Parameter(Mandatory=$true, HelpMessage="Especifica el directorio a monitorear.")][string]$directorio,
    [string]$salida,
    [switch]$kill
)

# Archivo de PID para rastrear si ya hay un proceso en ejecución
$pidFile = Join-Path -Path $env:TEMP -ChildPath "demonio_$((Get-Item $directorio).FullName -replace '[\\/:]', '_').pid"

# Si se usa el parámetro -kill, entonces el objetivo es detener el demonio
if ($kill) {
    if (Test-Path $pidFile) {
        $_pid = Get-Content $pidFile
        Write-Host $_pid
        try {
            Stop-Job -Id $_pid
            Remove-Item $pidFile
            Write-Host "Proceso demonio detenido."
        } catch {
            Write-Host "No se pudo detener el proceso demonio." -ForegroundColor Red
        }
    } else {
        Write-Host "No hay proceso demonio en ejecución para el directorio especificado." -ForegroundColor Red
    }
    exit
}

# Validar que no haya otro proceso demonio ejecutándose
if (Test-Path $pidFile) {
    Write-Host $pidFile
    Write-Host "Ya existe un proceso demonio ejecutándose en este directorio. No se pueden ejecutar dos al mismo tiempo." -ForegroundColor Red
    exit
}

# Validar parámetros de salida
if (-not $salida) {
    Write-Host "Debes proporcionar un directorio de salida para los archivos comprimidos." -ForegroundColor Red
    exit
}

# Resolución de rutas
$directorioAbs = Resolve-Path $directorio
$salidaAbs = Resolve-Path $salida

# Crear el script block que se ejecutará como demonio
$scriptBlock = {
    param (
        [string]$directorioAbs,
        [string]$salidaAbs,
        [string]$archivoLog
    )

    if (-not (Test-Path $archivoLog)) {
        New-Item -Path $archivoLog -ItemType File
    }

    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $directorioAbs
    $watcher.Filter = "*.*"
    $watcher.IncludeSubdirectories = $true
    $watcher.EnableRaisingEvents = $true

    $action = {
        Start-Sleep -Seconds 1
        $nuevoArch = $Event.SourceEventArgs.FullPath
        $nombre = $Event.SourceEventArgs.Name
        $tam = (Get-Item $nuevoArch).length

        $archivos = Get-ChildItem -Path $using:directorioAbs -Recurse | Where-Object { -not $_.PSIsContainer }
        
        # Verificar si ya existe un archivo con el mismo nombre y tamaño
        $existearch = $archivos | Where-Object { 
            $_.Name -eq $nombre -and $_.Length -eq $tam -and $_.FullName -ne $nuevoArch 
        }
        
        if ($existearch.Count -gt 0) {
            # Comprobar si ya existe un archivo ZIP para este archivo
            # $comprimido = Join-Path -Path $using:salidaAbs -ChildPath "$nombre.zip"
            if (-not (Test-Path $comprimido)) {
                # Comprimir el archivo
                $comprimido = Join-Path -Path $using:salidaAbs -ChildPath "$(Get-Date -Format 'yyyyMMdd-HHmmss').zip"
                Compress-Archive -Path $nuevoArch -DestinationPath $comprimido
                $contenido = "$nombre es un archivo duplicado."
                $contenido >> $archivo
            }
        }
    }

    Register-ObjectEvent -InputObject $watcher -EventName Created -Action $action
    Register-ObjectEvent -InputObject $watcher -EventName Changed -Action $action

    while ($true) {
        Start-Sleep -Seconds 1
    }
}

$archivoLog = Join-Path -Path $salidaAbs -ChildPath "log.txt"

# Iniciar el proceso en segundo plano como trabajo (job)
$job = Start-Job -ScriptBlock $scriptBlock -ArgumentList $directorioAbs, $salidaAbs, $archivoLog

# Guardar el PID del trabajo
$job.Id | Out-File -FilePath $pidFile
Write-Host "Demonio iniciado. PID: $($job.Id)"
