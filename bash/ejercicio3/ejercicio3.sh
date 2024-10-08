#!/bin/bash

#Integrantes del grupo:
#-Cespedes, Cristian Ivan -> DNI 41704776
#-Gomez, Luciano Dario -> DNI 41572055
#-Luna, Leandro Santiago -> DNI 40886200
#-Panigazzi, Agustin Fabian -> DNI 43744593

mostrar_ayuda() {
  echo "Uso: $0 [-d <directorio>] [-h|--help]"
  echo
  echo "Parámetros:"
  echo "  -d, --directorio    Especifica el directorio a analizar en busca de archivos duplicados."
  echo "  -h, --help          Muestra este mensaje de ayuda."
  exit 0
}

mostrar_error() {
  echo "Error: Parámetro inválido."
  echo "Usa -h o --help para obtener más información."
  exit 1
}

# Procesar los parámetros
while [[ "$#" -gt 0 ]]; do
  case "$1" in
    -d|--directorio)
        if [[ -z "$2" ]]; then  # Por si no hay nada despues del -d. Sino se quedaba ejecutando
            echo "Error: Se requiere un directorio después de -d."
            exit 1
        fi
        directorio="$2"
        shift 2
        ;;
    -h|--help)
        mostrar_ayuda
        ;;
    *)
        mostrar_error
      ;;
  esac
done

# Verifica si el directorio existe
if [[ ! -d "$directorio" ]]; then
  echo "El directorio especificado no existe: $directorio"
  exit 1
fi

# Buscar archivos en el directorio y subdirectorios, y procesar con AWK
# find ./apl -type f
# ./apl/otro/archivo1.txt
# ./apl/prueba2.txt
# ./apl/final/prueba2.txt
# ./apl/final/final/archivo1.txt
# ./apl/final/archivo1.txt
# ./apl/archivo1.txt

#-rw-r--r-- 1 usuario grupo  1234 Mar  1 12:00 ./apl/otro/archivo1.txt
#-rw-r--r-- 1 usuario grupo  1234 Mar  1 12:00 ./un directorio con espacios/otro/archivo1.txt

find "$directorio" -type f -print0 | xargs -0 ls -l | awk '
{
    #  -rwxr-xr-x 1 leandro leandro 2586 Sep 21 00:19  ejercicio3.sh
    tam = $5              
    archivo = substr($0, index($0, $9))  # Captura el nombre completo del archivo
    n = split(archivo, partes, "/")
    nombre = partes[n]

    n = split(archivo, partes, "/")
    nombre = partes[n]

    clave = nombre "|" tam

    if (duplicados[clave]) {
        # Almacena solo el directorio
        directorio = ""
        for (i = 1; i < n; i++) {
          directorio = directorio partes[i] (i < n-1 ? "/" : "")
        }
        rutas[clave] = rutas[clave] "\n" directorio
    } else {
        duplicados[clave] = nombre
        directorio = ""
        for (i = 1; i < n; i++) {
          directorio = directorio partes[i] (i < n-1 ? "/" : "")
        }
        rutas[clave] = directorio
    }
}

END {
    for (clave in duplicados) {
        if (split(rutas[clave], arr, "\n") > 1) {
          print duplicados[clave]  # Imprime el nombre del archivo
          # Imprime solo los directorios únicos
          for (i = 1; i <= length(arr); i++) {
             print dir arr[i]
          }
          print ""  # Línea en blanco entre duplicados
        }
    }
}'