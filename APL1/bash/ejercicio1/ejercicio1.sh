#!/bin/bash


#Integrantes del grupo:
#-Cespedes, Cristian Ivan -> DNI 41704776
#-Gomez, Luciano Dario -> DNI 41572055
#-Luna, Leandro Santiago -> DNI 40886200
#-Panigazzi, Agustin Fabian -> DNI 43744593

function ayuda() {
    echo "Para ejecutar correctamente este script usted debera indicar 2 parametros, uno de entrada y otro de salida."
    echo "Usted debe concatenar al nombre del archivo estos parametros, se los detallaremos a continuacion,"
    echo "elija solo 2 de 3 posibilidades. Las rutas deben indicarse entre comillas dobles"
    echo ""
    echo "Opciones de entrada, debera indicar el directorio donde se ubican los archivos .csv para procesar"
    echo "las jugadas de loteria. Escriba la opcion -d o --directorio seguido un espacio y luego la ruta"
    echo "-d "user/document/jugadas"   o    --directorio "user/document/jugadas""
    echo ""
    echo "Opciones de salida, debera indicar si usted desea mostrar el resultado por pantalla" 
    echo "o guardalo en un archivo json. Solo debera seleccionar una opcion, no es posible ambas."
    echo "Para elegir salida por pantalla escriba solamente la opcion -p o --pantalla"
    echo "no es necesario indicar parametros. Para salida por archivo elija la opcion -a o --archivo"
    echo "seguido de la ruta de destino completa indicando incluso nombre de archivo y extension json"
    echo "Ejemplos: "
    echo "-p       o      -pantalla"
    echo "-a "user/document/resultado.json"   o   -archivo "user/document/resultado.json""
    echo ""
    echo "Ejemplo de ejecucion completo: "
    echo "./Ejercicio1.sh -p -d "user/document/resultado.json""
}

error_output=$(getopt -o d:a:p,h --long help,directorio:,archivo:,pantalla -- "$@" 2>&1)

if [ "$?" != "0" ]; then
    echo "Ha ingresado un parametro incorrecto, solo son validos -a -p o -d"
    echo "Si necesita mas ayuda para la ejecucion utilice el parametro -h o --help"
    exit 1
fi

eval set -- "$error_output"

directorio=""
archivo=""
pantalla=""


while true; do
    case "$1" in
        -d | --directorio)
            directorio="$2"
            shift 2
            ;;
        -a | --archivo)
            archivo="$2"
            shift 2
            ;;
        -p | --pantalla)
            pantalla="Si"
            shift 1
            ;;
        -h | --help)
            ayuda
            exit 0
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Error desconocido"
            exit 1
            ;;
    esac
done

if [ -z "$directorio" ] && ([ -z "$archivo" ] || [ -z "$pantalla" ]); then
    echo "Error: Tanto el parámetro de entrada como el parámetro de salida son obligatorios."
    echo "Utilice el comando -h o --help si desea ayuda"
    exit 1
fi


if [ -z "$directorio" ]; then
    echo "Error: El parámetro -d o --directorio de entrada es obligatorio."
    exit 1
fi

if [ -z "$archivo" ] && [ -z "$pantalla" ]; then
    echo "Error: Se debe indicar con el parametro de salida. Este parametro es obligatorio."
    echo "Utilice -h o --help para recibir ayuda. Puede indicar -a o --archivo seguido de la ruta"
    echo "para escribir el resultado en un archivo, sino puede utilizar -p o --pantalla"
    echo "para mostrar por pantalla el resultado"
    exit 1
fi

if [ -n "$archivo" ] && [ -n "$pantalla" ]; then
    echo "Error: solo se debe indicar un tipo de salida, no se debe indicar mas de uno."
    exit 1
fi

if [ ! -d "$directorio" ]; then
    echo "El directorio '$directorio' no existe."
    exit 1
fi

cantCsv=$(find $directorio -name "*.csv" -type f | wc -l)

if [ "$cantCsv" -eq 0 ] || [ "$cantCsv" -eq 1 ]; then
    echo "No se encontraron archivos csv en el directorio indicado"
    exit 1
fi

if [ -n "$archivo" ]; then
    directorio_salida=$(dirname "$archivo")

    if [ ! -d "$directorio_salida" ]; then
        echo "Error: El directorio del archivo de salida especificado no existe."
        exit 1
    fi

    if [[ "$archivo" != *.json ]]; then
        echo "Error: El archivo debe tener la extensión .json."
        exit 1
    fi

    if [ ! -f "$archivo" ]; then
        touch "$archivo"
    fi
fi

csv_files=$(find "$directorio" -name "*.csv" -type f ! -name "ganadores.csv")

if [ -z "$csv_files" ]; then
    echo "No se encontraron archivos .csv referidos a las loterias en el directorio '$directorio'."
    exit 1
fi

archivo_csv="${directorio}/ganadores.csv"

if [[ ! -f "$archivo_csv" ]]; then
    echo "No se encontró un archivo de ganadores en formato csv en el directorio indicado"
    exit 1
fi

vector_ganadores=$(awk -F, 'NR==1 { for (i = 1; i <= NF; i++) printf "%s ", $i }' "$archivo_csv")

vector_ganadores_array=($vector_ganadores)

dir_temp="/tmp"

archivo_5_aciertos="${dir_temp}/5_aciertos.json"
archivo_4_aciertos="${dir_temp}/4_aciertos.json"
archivo_3_aciertos="${dir_temp}/3_aciertos.json"

echo "[" > "$archivo_5_aciertos"
echo "[" > "$archivo_4_aciertos"
echo "[" > "$archivo_3_aciertos"

for file in $csv_files; do
    agencia=$(basename "$file" .csv)
    awk -F, -v ganadores="${vector_ganadores}" -v agencia="$agencia" \
    -v archivo_5="$archivo_5_aciertos" -v archivo_4="$archivo_4_aciertos" -v archivo_3="$archivo_3_aciertos" '
    BEGIN {
        split(ganadores, g, " ");  # Convertir la cadena de ganadores a un array
    }
    {
        jugada_num = NR;  # Número de jugada
        count = 0;        # Contador de aciertos

        # Comparar los números de la jugada con los ganadores
        for (i = 2; i <= NF; i++) {  # Comenzar desde 2 porque el primero es el número de registro
            for (j in g) {
                if ($i == g[j]) {
                    count++;
                }
            }
        }

        # Escribir resultados en archivos según la cantidad de aciertos
        if (count == 5) {
            print "   {" >> archivo_5;
            print "     \"agencia\":\"" agencia "\"," >> archivo_5;
            print "     \"jugada\":\"" jugada_num "\"" >> archivo_5;
            print "   }," >> archivo_5;
        } else if (count == 4) {
            print "   {" >> archivo_4;
            print "     \"agencia\":\"" agencia "\"," >> archivo_4;
            print "     \"jugada\":\"" jugada_num "\"" >> archivo_4;
            print "   }," >> archivo_4;
        } else if (count == 3) {
            print "   {" >> archivo_3;
            print "     \"agencia\":\"" agencia "\"," >> archivo_3;
            print "     \"jugada\":\"" jugada_num "\"" >> archivo_3;
            print "   }," >> archivo_3;
        }
    }
    ' "$file"
done

for arch_aciertos in "$archivo_5_aciertos" "$archivo_4_aciertos" "$archivo_3_aciertos"; do
    sed -i '$ s/,$//' "$arch_aciertos"
    echo "  ]" >> "$arch_aciertos"
done

archivo_salida=""

if [[ -z "$archivo" ]]; then
    archivo_salida="${dir_temp}/salida.json"
else
    archivo_salida="${archivo}"
fi

echo "{" > "$archivo_salida"

for archivo_aciertos in "$archivo_5_aciertos" "$archivo_4_aciertos" "$archivo_3_aciertos"; do

    nombre_archivo=$(basename "$archivo_aciertos" | sed 's/\(.*\)\..*/\1/')

    echo "  \"$nombre_archivo\":$(cat "$archivo_aciertos")," >> "$archivo_salida"
done

sed -i '$ s/,$//' "$archivo_salida"
echo "}" >> "$archivo_salida"

if [[ "$pantalla" == "Si" ]]; then
    cat "$archivo_salida"
fi

rm -f "$archivo_5_aciertos" "$archivo_4_aciertos" "$archivo_3_aciertos"

if [[ -z "$archivo" ]]; then
    rm -f "$archivo_salida"
fi