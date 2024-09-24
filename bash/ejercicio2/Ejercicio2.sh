#!/bin/bash

#Integrantes del grupo:
#-Cespedes, Cristian Ivan -> DNI 41704776
#-Gomez, Luciano Dario -> DNI 41572055
#-Luna, Leandro Santiago -> DNI 40886200
#-Panigazzi, Agustin Fabian -> DNI 43744593

# Función para mostrar la ayuda
mostrar_ayuda() {
    echo "Uso: $0 [opciones]"
    echo
    echo "Opciones:"
    echo "  -h, --help      Muestra esta ayuda"
    echo "  -m <archivo>    Especifica el archivo de entrada"
    echo "  -s <separador>  Especifica el separador (por defecto es ',')"
    echo "  -p <producto>   Especifica el número de producto"
    # mostrar_uso
    exit 0
}

# Función para mostrar cómo se usa el script
function mostrar_uso {
    echo "Uso: $0 -m archivo_matriz [-p escalar] [-t] [-s separador]"
    exit 1
}

# Verificar si hay suficientes parámetros
# if [ $# -lt 2 ]; then
#     mostrar_uso
# fi

# Inicializar variables
archivo_matriz=""
producto_escalar=1
trasponer=0
separador="|"

# Procesar los parámetros
while [[ "$#" -gt 0 ]]; do
    case "$1" in
        -h|--help) mostrar_ayuda ;;
        -m|--matriz) archivo_matriz="$2"; shift ;;
        -p|--producto) producto_escalar="$2"; shift ;;
        -t|--trasponer) trasponer=1 ;;
        -s|--separador)
            if [[ "$2" =~ [0-9-] ]]; then
                echo "Error: El separador no puede ser un número o el símbolo menos (-)."
                exit 1
            fi
            separador="$2"
            shift ;;
        *) echo "Parámetro desconocido: $1"; mostrar_uso ;;
    esac
    shift
done

# echo $separador
# Verificar si producto escalar y trasponer están activados al mismo tiempo
if [ "$producto_escalar" -ne 1 ] && [ "$trasponer" -eq 1 ]; then
    echo "Error: No puedes usar producto escalar y trasponer al mismo tiempo."
    exit 1
fi

# Verificar que el archivo de la matriz existe
if [ ! -f "$archivo_matriz" ]; then
    echo "Error: El archivo $archivo_matriz no existe."
    exit 1
fi

if ! grep -q "$separador" "$archivo_matriz"; then
    echo "Error: El separador ingresado no es correcto."
    exit 1
fi

# Leer la matriz desde el archivo
matriz=()
columnas=-1
filas=0

while IFS="$separador" read -r linea; do
    fila=()
    if [[ "$linea" =~ [0-9]+,[0-9]+ && "$separador" != "," ]]; then
        echo "Error: La matriz contiene comas como valores decimales. Usa puntos ('.') en su lugar para los decimales."
        exit 1
    fi
    for valor in $(echo "$linea" | tr "$separador" " "); do
        fila+=("$valor")
    done
    matriz+=("${fila[@]}")
    if [ $columnas -eq -1 ]; then
        columnas=${#fila[@]}
    else
        # Verificar que todas las filas tengan el mismo número de columnas
        if [ ${#fila[@]} -ne $columnas ]; then
            echo "Error: Las filas no tienen el mismo número de columnas."
            exit 1
        fi
    fi
    filas=$((filas + 1))
done < "$archivo_matriz"

# echo ${#fila[@]}
# echo $columnas
if [ $filas -ne $columnas ]; then
    echo "Error: La matriz no es cuadrada. Número de filas: $filas, Número de columnas: $columnas"
    exit 1
fi

# Archivo de salida
archivo_salida="salida.$(basename "$archivo_matriz")"

# Función para transponer la matriz
function transponer_matriz {
    for ((i=0; i<$columnas; i++)); do
        for ((j=0; j<${#matriz[@]}/$columnas; j++)); do
            echo -n "${matriz[$((j * columnas + i))]}" >> "$archivo_salida"
            if (( j < ((${#matriz[@]} / columnas) - 1) )); then
                echo -n "$separador" >> "$archivo_salida"
            fi
        done
        echo "" >> "$archivo_salida"
    done
}

function producto_escalar_matriz {
    for ((i=0; i<${#matriz[@]}; i++)); do
        # Multiplicar usando bc
        matriz[$i]=$(echo "${matriz[$i]} * $producto_escalar" | bc -l)
        echo -n "${matriz[$i]}" >> "$archivo_salida"
        if (( (i + 1) % columnas == 0 )); then
            echo "" >> "$archivo_salida"
        else
            echo -n "$separador" >> "$archivo_salida"
        fi
    done
}

# Ejecución según los parámetros
if [ $trasponer -eq 1 ]; then
    transponer_matriz
    echo "Operación completada."
else
    producto_escalar_matriz
    echo "Operación completada."
fi

# echo "Operación completada."
