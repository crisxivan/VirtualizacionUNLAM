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

if [ ! -s "$archivo_matriz" ]; then
    echo "Error: El archivo $archivo_matriz está vacío."
    exit 1
fi

if ! grep -q "$separador" "$archivo_matriz"; then
    echo "Error: El separador ingresado no es correcto."
    exit 1
fi

# Leer la matriz desde el archivo
matriz=()
filas=0
max_columnas=0

while IFS= read -r linea; do
    # Si no hay separador, asumimos matriz columna o valor único
    if [[ "$linea" != *"$separador"* && -n "$separador" ]]; then
        fila=("$linea")
    else
        # Si hay separador, dividir la línea en columnas - el tr es porque sale un error de caracteres
        fila=($(echo "$linea" | tr "$separador" " " | tr -d '\r'))
    fi

    for valor in "${fila[@]}"; do
        if ! [[ "$valor" =~ ^-?[0-9]+(\.[0-9]+)?$ ]]; then
            echo "Error: La matriz contiene un valor no numérico: $valor"
            exit 1
        fi
    done

    matriz+=("$(IFS="|"; echo "${fila[*]}")")
    
    if [ ${#fila[@]} -gt $max_columnas ]; then
        max_columnas=${#fila[@]}
    fi

    filas=$((filas + 1))
done < "$archivo_matriz"

# Archivo de salida
directorio_matriz=$(dirname "$archivo_matriz")
archivo_salida="$directorio_matriz/salida.$(basename "$archivo_matriz")"

# Función para transponer la matriz
function transponer_matriz {
    echo "" > "$archivo_salida"
    for ((i=0; i<$max_columnas; i++)); do
        for ((j=0; j<$filas; j++)); do
            IFS="|" read -r -a fila <<< "${matriz[$j]}"
            if [ $i -lt ${#fila[@]} ]; then
                echo -n "${fila[$i]}" >> "$archivo_salida"
            fi

            if (( j < (filas - 1) )); then
                echo -n "$separador" >> "$archivo_salida"
            fi
        done
        echo "" >> "$archivo_salida"
    done
}

function producto_escalar_matriz {
    echo "" > "$archivo_salida"
    for fila in "${matriz[@]}"; do
        IFS="|" read -r -a valores <<< "$fila"
        for valor in "${valores[@]}"; do
            resultado=$(echo "$valor * $producto_escalar" | bc -l)
            echo -n "$resultado" >> "$archivo_salida"
            echo -n "$separador" >> "$archivo_salida"
        done
        echo "" >> "$archivo_salida"
    done
}

if [ $trasponer -eq 1 ]; then
    transponer_matriz
    echo "Transposición completada. Matriz guardada en $archivo_salida."
else
    producto_escalar_matriz
    echo "Producto escalar completado. Matriz guardada en $archivo_salida."
fi
