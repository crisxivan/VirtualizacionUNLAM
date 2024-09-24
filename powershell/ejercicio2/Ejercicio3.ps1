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
    [Alias("matriz")]
    [string]$archivo_matriz = "",
    
    [Alias("producto")]
    [double]$producto_escalar = 1,
    
    [Alias("transponer")]
    [switch]$trasponer,
    
    [Alias("separador")]
    [string]$separator = "|",
    
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

# Verificar que el separador esté presente en el archivo
$escapedSeparator = [regex]::Escape($separator)
if (-not (Select-String -Pattern $escapedSeparator -Path $archivo_matriz)) {
    Write-Host "Error: El separador ingresado no es correcto o no se encuentra en el archivo."
    exit
}

# Leer la matriz desde el archivo
$matriz = @()
$columnas = -1
$filas = 0

# Leer cada línea del archivo
Get-Content $archivo_matriz | ForEach-Object {
    $linea = $_
    
    # Validar que la línea no contenga comas
    if ($linea -match '[0-9]+,[0-9]+' -and $separator -ne ",") {
        Write-Host "Error: La matriz contiene comas (',') como separador decimal. Usa puntos ('.') en su lugar."
        exit
    }

    # Separar la línea en valores y añadir a la matriz
    $fila = $linea -split [regex]::Escape($separator)
    $matriz += $fila
    $filas++

    # Establecer el número de columnas
    if ($columnas -eq -1) {
        $columnas = $fila.Count
    } elseif ($fila.Count -ne $columnas) {
        Write-Host "Error: Las filas no tienen el mismo número de columnas."
        exit
    }
}

# Verificar si la matriz es cuadrada
if ($filas -ne $columnas) {
    Write-Host "Error: La matriz no es cuadrada. Número de filas: $filas, Número de columnas: $columnas"
    exit
}

# Archivo de salida
$archivo_salida = "salida.$(Split-Path $archivo_matriz -Leaf)"

# Función para transponer la matriz
function TransponerMatriz {
    $transpuesta = @()
    for ($i = 0; $i -lt $columnas; $i++) {
        $nuevaFila = @()
        for ($j = 0; $j -lt $filas; $j++) {
            $nuevaFila += $matriz[$j * $columnas + $i]
        }
        $transpuesta += ($nuevaFila -join $separator)
    }
    $transpuesta | Out-File -FilePath $archivo_salida -Encoding utf8
}

# Función para multiplicar la matriz por un escalar
function ProductoEscalarMatriz {
    $resultados = @()
    for ($i = 0; $i -lt $filas; $i++) {
        $filaResultados = @()
        for ($j = 0; $j -lt $columnas; $j++) {
            $index = ($i * $columnas) + $j
            $valor = [double]$matriz[$index] * $producto_escalar
            $filaResultados += $valor
        }
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
