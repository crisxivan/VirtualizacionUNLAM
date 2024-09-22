<#
.SYNOPSIS
    Script que procesa los numeros ganadores y las jugadas realizadas en las distintas agencias de loterias para determinar los ganadores

.DESCRIPTION
    Este script requiere que se especifique un directorio y uno de los siguientes: un archivo o la opción de pantalla.
    No se pueden usar ambas opciones de archivo y pantalla simultáneamente.

.PARAMETER directorio
    Ruta del directorio donde se encuentran el archivo con los numeros ganadores y los archivos de las agencias de loteria a 
    procesar, todos los archivos deben ser de extension .csv, no deben contener encabezados (obligatorio).

.PARAMETER archivo
    Ruta completa del archivo de salida, debe ser de extensión json (obligatorio si no se usa -pantalla).

.PARAMETER pantalla
    Modo de ejecución en pantalla, este parametro imprimira la salida del proceso por la pantalla (obligatorio si no se usa -archivo).

.EXAMPLE
    .\ejercicio1.ps1 -d "\mnt\c\Users\juan\jugadas" -a "\mnt\c\Users\juan\salida.json"
    .\ejercicio1.ps1 -d "\mnt\c\Users\juan\jugadas" -p
#>

<#
Integrantes del grupo:
-Cespedes, Cristian Ivan -> DNI 41704776
-Gomez, Luciano Dario -> DNI 41572055
-Luna, Leandro Santiago -> DNI 40886200
-Panigazzi, Agustin Fabian -> DNI 43744593
#>

param (
    [Parameter(Mandatory = $true)]
    [string]$directorio,

    [string]$archivo,

    [switch]$pantalla
)

if (-not (Test-Path $directorio -PathType Container)) {
    throw "El directorio especificado no existe."
}

if ($archivo) {
    if ([System.IO.Path]::GetExtension($archivo) -ne ".json") {
        throw "El archivo de salida debe tener la extensión .json."
    }
    $directorioSalida = [System.IO.Path]::GetDirectoryName($archivo)
    if (-not (Test-Path $directorioSalida)) {
        throw "El directorio especificado para el archivo de salida no existe."
    }
    if (-not (Test-Path $archivo)) {
        New-Item -Path $archivo -ItemType File | Out-Null
    }
}

if ($pantalla -and $archivo) {
    Write-Host "No se pueden usar ambos parámetros -archivo y -pantalla a la vez, debe elegir uno de ellos." -ForegroundColor Red
    Write-Host "Para más información escriba Get-Help .\ejercicio1.ps1" -ForegroundColor Red
    exit
}

if (-not $pantalla -and -not $archivo) {
    Write-Host "Debe especificar -archivo o -pantalla." -ForegroundColor Red
    Write-Host "Para más información escriba Get-Help .\ejercicio1.ps1" -ForegroundColor Red
    exit
}

$csvFiles = Get-ChildItem -Path $directorio -Filter "*.csv"

if ($csvFiles.Count -lt 2) {
    Write-Host "En el directorio debe haber al menos dos archivos CSV para procesar." -ForegroundColor Red
    Write-Host "Uno debe ser con el nombre ganadores.csv y el resto hará referencia a las loterías." -ForegroundColor Red
    exit
}

if (-not ($csvFiles | Where-Object { $_.Name -eq "ganadores.csv" })) {
    Write-Host "El archivo 'ganadores.csv' no se encuentra en el directorio." -ForegroundColor Red
    exit
}

$ganadoresPath = Join-Path -Path $directorio -ChildPath "ganadores.csv"

$ganadores = (Get-Content -Path $ganadoresPath) -split ',' | ForEach-Object { [int]$_ }

$archivosCSV = Get-ChildItem -Path $directorio -File -Filter "*.csv" | Where-Object { $_.Name -ne "ganadores.csv" }

$aciertosOutput = @{
    '5_aciertos' = @()
    '4_aciertos' = @()
    '3_aciertos' = @()
}

foreach ($arch in $archivosCSV) {
    $contenido = Get-Content -Path $arch.FullName

    if ($contenido.Count -gt 0) {
        $archivoValido = $true

        foreach ($linea in $contenido) {
            $numeros = $linea -split ',' | ForEach-Object {
                try {
                    [int]$_
                } catch {
                    $archivoValido = $false
                    break
                }
            }

            if (-not $archivoValido) {
                Write-Host "El archivo '$($arch.FullName)' no contiene datos válidos y será ignorado." -ForegroundColor Yellow
                break
            }

            $numeroRegistro = $numeros[0]
            $jugada = $numeros[1..($numeros.Length - 1)]

            $aciertos = ($jugada | Where-Object { $ganadores -contains $_ }).Count

            if ($aciertos -ge 3) {
                $nombreArchivoSinExtension = [System.IO.Path]::GetFileNameWithoutExtension($arch.Name)

                if ($aciertos -eq 5) {
                    $aciertosOutput['5_aciertos'] += @{ 'agencia' = $nombreArchivoSinExtension; 'jugada' = $numeroRegistro }
                } elseif ($aciertos -eq 4) {
                    $aciertosOutput['4_aciertos'] += @{ 'agencia' = $nombreArchivoSinExtension; 'jugada' = $numeroRegistro }
                } elseif ($aciertos -eq 3) {
                    $aciertosOutput['3_aciertos'] += @{ 'agencia' = $nombreArchivoSinExtension; 'jugada' = $numeroRegistro }
                }
            }
        }
    }
}


$salidaDir = "/tmp"
$tempFile = Join-Path $salidaDir "salida.txt"

$jsonContent = @"
{
  "5_aciertos":[`n
"@

foreach ($ganador in $aciertosOutput['5_aciertos']) {
    $jsonContent += @"
   {
     "agencia":"$($ganador.agencia)",
     "jugada":"$($ganador.jugada)"
   },`n
"@
}

$jsonContent = $jsonContent.TrimEnd(',', "`n") + @"
    `n  ],
  "4_aciertos":[`n
"@

foreach ($ganador in $aciertosOutput['4_aciertos']) {
    $jsonContent += @"
   {
     "agencia":"$($ganador.agencia)",
     "jugada":"$($ganador.jugada)"
   },`n
"@
}

$jsonContent = $jsonContent.TrimEnd(',', "`n") + @"
    `n  ],
  "3_aciertos":[`n
"@

foreach ($ganador in $aciertosOutput['3_aciertos']) {
    $jsonContent += @"
   {
     "agencia":"$($ganador.agencia)",
     "jugada":"$($ganador.jugada)"
   },`n
"@
}

$jsonContent = $jsonContent.TrimEnd(',', "`n") + @"
    `n  ]
}
"@

$jsonContent | Set-Content -Path $tempFile

$jsonOutput = Get-Content -Path $tempFile -Raw

if ($pantalla) {
    Write-Output $jsonOutput
} elseif ($archivo) {
    $jsonOutput | Set-Content -Path $archivo
}

Remove-Item -Path $tempFile

Write-Output "Proceso finalizado con exito"