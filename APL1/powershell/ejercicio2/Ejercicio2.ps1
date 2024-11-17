<#
.SYNOPSIS
    Este script realiza operaciones con matrices.
.DESCRIPTION
    Este script toma un archivo de matriz, un producto y un separador como parámetros para procesar datos.
.PARAMETER Matriz
    El archivo de matriz que contiene los datos a procesar.
.PARAMETER Producto
    El número del producto a procesar.
.PARAMETER Separador
    El separador utilizado en el archivo de matriz. Por defecto es ','.
.EXAMPLE
    .\Ejercicio1.ps1 -Matriz "matriz.txt" -Producto 4 -Separador ","
.NOTES
    #Integrantes del grupo:
    #-Cespedes, Cristian Ivan -> DNI 41704776
    #-Gomez, Luciano Dario -> DNI 41572055
    #-Luna, Leandro Santiago -> DNI 40886200
    #-Panigazzi, Agustin Fabian -> DNI 43744593
    Fecha: 2024
#>

param (
    [Parameter(Mandatory=$true)]
    [Alias("matriz")]
    [string]$archivo_matriz,
    
    [Alias("producto")]
    [double]$producto_escalar=1,
    
    [switch]$trasponer,
    
    [Alias("separador")]
    [string]$separator,
    
    [switch]$Help
)

if ($Help) {
    Get-Help $MyInvocation.MyCommand.Path
    exit
}

# Función para mostrar cómo se usa el script
function MostrarUso {
    Write-Host "Uso: .\$($MyInvocation.MyCommand) -m archivo_matriz [-p escalar] [-t] [-s separador]"
    exit
}

# Verificar si hay suficientes parámetros
if (-not $archivo_matriz) {
    MostrarUso
}

# Verificar que el archivo de la matriz existe
if (-not (Test-Path $archivo_matriz)) {
    Write-Host "Error: El archivo $archivo_matriz no existe."
    exit
}

# Verificar si el archivo está vacío
if ((Get-Content $archivo_matriz).Length -eq 0) {
    Write-Host "Error: El archivo $archivo_matriz está vacío."
    exit
}

# Verificar que no se pasen ambos parámetros
if ($trasponer -and $producto_escalar -ne 1) {
    Write-Host "Error: No se puede usar -trasponer junto con -producto. Elija solo uno."
    exit
}

# Verificar que el separador no sea un número o '-'
if ($separator -match '^\d+$' -or $separator -eq "'-'") {
    Write-Host "Error: El separador no puede ser un número o el símbolo menos (-)."
    exit
}

# Leer el contenido del archivo
$contenido = Get-Content $archivo_matriz
# Archivo de salida
$archivo_salida = Join-Path (Split-Path $archivo_matriz -Parent) "salida.$(Split-Path $archivo_matriz -Leaf)"

if ($contenido.Count -eq 1) {
    $primerValor = $contenido -split [regex]::Escape($separator)
    Write-Host $primerValor
    if ($primerValor.Count -eq 1 -and $primerValor[0] -match '^(-?[0-9]+(\.[0-9]+)?)$') {
        # Si es un único valor numérico
        $matriz = @(@($contenido[0]))  # Convertirlo en una matriz 1x1
        Write-Host "Matriz 1x1 procesada: $matriz"

        # Si el producto escalar es diferente de 1, lo aplicamos
        if ($producto_escalar -ne 1) {
            $matriz = @(@([double]$contenido[0] * $producto_escalar))  # Aplicar el producto escalar
            Write-Host "Producto escalar aplicado: $matriz"
        }

        $matriz | Out-File -FilePath $archivo_salida -Encoding utf8
        Write-Host "Matriz 1x1 procesada."
        exit
    }
}


# Leer matriz en un arreglo de filas
$matriz = @()
$contenido | ForEach-Object {
    $linea = $_.Trim()
    $fila = $linea -split [regex]::Escape($separator)

    if ($fila.Count -eq 1) {
        $matriz += ,@($fila[0]) 
    } else {
        $matriz += ,$fila
    }
}



# Función para trasponer la matriz
function TransponerMatriz {
    $maxColumnas = ($matriz | ForEach-Object { $_.Count }) | Measure-Object -Maximum | Select-Object -ExpandProperty Maximum
    $transpuesta = @()

    for ($i = 0; $i -lt $maxColumnas; $i++) {
        $nuevaFila = @()
        foreach ($fila in $matriz) {
            if ($i -lt $fila.Count) {
                $nuevaFila += $fila[$i]
            } else {
                $nuevaFila += ""  # Espacio vacío para celdas inexistentes
            }
        }
        $transpuesta += ($nuevaFila -join $separator)
    }

    $transpuesta | Out-File -FilePath $archivo_salida -Encoding utf8
}


# Función para multiplicar la matriz por un escalar
function ProductoEscalarMatriz {
    $resultados = @()
    foreach ($fila in $matriz) {
        $filaResultados = $fila | ForEach-Object { [double]$_ * $producto_escalar }
        $resultados += ($filaResultados -join $separator)
    }
    $resultados | Out-File -FilePath $archivo_salida -Encoding utf8
}

# Ejecución según los parámetros
if ($trasponer) {
    TransponerMatriz
    Write-Host "Operación completada."
} else {
    ProductoEscalarMatriz
    Write-Host "Operación completada."
}
