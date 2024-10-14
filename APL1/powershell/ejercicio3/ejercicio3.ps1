<#
.SYNOPSIS
Este script busca archivos duplicados en un directorio especificado y muestra los directorios donde se encuentran.

.DESCRIPTION
Busca archivos en el directorio especificado y verifica si existen duplicados por nombre y tamaño.
En caso de duplicados, muestra los nombres de los archivos y sus ubicaciones.

.PARAMETER Directorio
Especifica el directorio en el que se deben buscar los archivos duplicados.

.PARAMETER Help
Muestra este mensaje de ayuda.

.EXAMPLE
.\ejercicio3.ps1 -directorio "C:\ruta\al\directorio"
Ejecuta el script para buscar archivos duplicados en el directorio especificado.

.EXAMPLE
Get-Help .\ejercicio3.ps1
Muestra la ayuda del script.

.NOTES
#Integrantes del grupo:
#-Cespedes, Cristian Ivan -> DNI 41704776
#-Gomez, Luciano Dario -> DNI 41572055
#-Luna, Leandro Santiago -> DNI 40886200
#-Panigazzi, Agustin Fabian -> DNI 43744593
Fecha: 20 de septiembre de 2024
#>


param (
    [Parameter(Mandatory=$true, HelpMessage="Especifica el directorio a analizar.")]
    [ValidateNotNullOrEmpty()]
    [string]$directorio,

    [switch]$Help
)

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path
    exit
}

if (-not (Test-Path -Path $directorio -PathType Container)) {
    Write-Host "El directorio especificado no existe: $directorio"
    exit 1
}

$duplicados = @{}
$rutas = @{}

Get-ChildItem -Path $directorio -Recurse -File | ForEach-Object {
    $tam = $_.Length
    $nombre = $_.Name
    $clave = "$nombre|$tam"

    if ($duplicados.ContainsKey($clave)) {
        $rutas[$clave] += "`n$($_.DirectoryName)"
    } else {
        $duplicados[$clave] = $nombre
        $rutas[$clave] = $_.DirectoryName
    }
}

# Imprime resultados
foreach ($clave in $duplicados.Keys) {
    $directorios = $rutas[$clave] -split "`n" | Sort-Object -Unique
    # Write-Host $rutas[$clave] -split "`n"
    if ($directorios.Count -gt 1) {
        Write-Host $duplicados[$clave] 

        foreach ($dir in $directorios) {
            Write-Host $dir
        }
        Write-Host "" 
    }
}

# Ayuda
if ($args -contains '-h' -or $args -contains '--help') {
    Mostrar-Ayuda
}