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
	[Parameter(Mandatory=$true)] [ValidateNotNullOrEmpty()]
    [string] $directorio,
    [string] $salida,
	[switch] $kill
)


if (-not ($salida -xor $kill)) {
    Write-Host "Debes proporcionar solo uno de los parámetros: salida o kill." -ForegroundColor Red
    exit
}

if (-not (Test-Path $directorio)) {
	Write-Error "Directorio no encontrado."
	exit
}

function stop_daemon {
	# Aquí verificamos si hay algún monitor activo en el directorio y lo detenemos
	$fullPath = Resolve-Path $directorio
	$jobList = $(Get-Job | Where-Object { $_.Name -eq $fullPath })
	if ($jobList) {
		Write-Host "Deteniendo monitoreo '$directorio'..."
		$jobList | ForEach-Object { Stop-Job -Id $_.Id; Remove-Job -Id $_.Id }
		Write-Host "Detenido!."
	} else {
		Write-Host "$directorio no estaba monitoreado."
	}
}

if ($kill) {
	stop_daemon
	exit
} else {
	$fullPath = Resolve-Path $directorio
	$fullOut = Resolve-Path $salida

    $scriptBlock = {
		param (
        	[string]$fuente,
        	[string]$destino
    	)
		$fsw = New-Object IO.FileSystemWatcher $fuente
		$fsw.IncludeSubdirectories = $true
		$fsw.Filter = '*.*'
		$fsw.EnableRaisingEvents = $true

		$action = {
			$newFile = $Event.SourceEventArgs.FullPath
			Start-Sleep -Seconds 1

			$fileName = [System.IO.Path]::GetFileName($newFile)
			$fileSize = (Get-Item $newFile).Length
			$duplicatedFile = $(Get-ChildItem -Path $fuente -Recurse | Where-Object { $_.Name -eq $fileName -and $_.Length -eq $fileSize -and $_.FullName -ne $newFile })

			if ($duplicatedFile) {
				$backupName =  "$(Get-Date -forma yyyyMMddhhmmss).zip"
				Compress-Archive -Path $newFile -DestinationPath $($destino + '\' + $backupName) 
				Remove-Item -Path $newFile
				Add-Content -Path "$destino\log.txt" -Value "Backup $newFile - Archivo $backupName"
				Add-Content -Path "$destino\log.txt" -Value ""
			}
		}

		Register-ObjectEvent -InputObject $fsw -EventName "Created" -Action $action
        #faltaba esto
		while ($true) {
			Start-Sleep -Seconds 5
		}
	}

	$jobList = $(Get-Job | Where-Object { $_.Name -eq $fullPath })
	if ($jobList) {
		Write-Host "El directorio $directorio está monitoreado. No se permite la acción."
		stop_monitor
		exit
	}
	Write-Host "Iniciando monitoreo de directorio: $directorio"

	# proceso en seg plano le pasamos el script block del demonio
	Start-Job -Name $fullPath -ScriptBlock $scriptBlock -ArgumentList $fullPath, $fullOut | Out-Null
}
