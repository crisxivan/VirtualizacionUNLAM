function ayuda() {
    	echo " "

	echo "------------ AYUDA - DESCRIPCION DEL SCRIPT -----------"
	
	echo " "	
	
	echo "El script ejecuta un sistema de integracion continua. Cada vez que detecta la generacion de un archivo duplicado en el directorio especificado, lo archiva en un archivo comprimido y crea un registro en un archivo log"
	
	echo " "
	
	echo "------------ AYUDA - FORMATO DEL SRCIPT ------------"
	echo "**Para asegurar el correcto funcionamiento del script, es necesario que antes ejecute los siguientes comandos**"
	echo "I)  sudo apt update"
	echo "II) sudo apt install inotify-tools"
	echo " "

	echo "./ejercicio4.sh [--help | -h] => Muestra ayuda"

    echo "**Parametros de ejecucion**"
    echo "[--directorio | -d] => Directorio que se desea monitorear"
    echo "[--salida | -s] => Directorio donde se guardaran los backups y el archivo de log"
    echo "[--kill | -k] => flag para terminar la ejecucion del script en un determinado directorio"
    
    echo " "
	
    echo "**Ejemplo de ejecucion**"
    echo "./ejercicio4.sh -d <path> -s <path>"

    echo " "
	
	echo "----------- AYUDA - CIERRE DEFINITIVO DEL SCRIPT ------------"
	
	echo "1) Para cerrar definitivamente el script, ingrese:"
    echo "./ejercicio4.sh -d <path> -k"
}
function demonio() {

while out=$(inotifywait -e create -r -q --format "%w%f %e" "$directorio")
do
    read nue_arch _ <<< "$out"

    nom_arch=$(basename "$nue_arch")
    tam_arch=$(stat -c%s "$nue_arch")

  
    arch_existente=$(find "$directorio" -type f -name "$nom_arch" -size "${tam_arch}c")
    cantidad_archivos=$(echo "$arch_existente" | wc -l)

    if [[ $cantidad_archivos -gt 1 ]]; then
        arch_existente=$(echo "$arch_existente" | tail -n 1)
    else
        arch_existente=""    
    fi

    if [[ -n "$arch_existente" ]]; then
        cd /tmp
        fecha=$(date)
        cp $arch_existente /tmp/"$fecha"
        tar -czf backup.tar.gz "$fecha"
        mv backup.tar.gz "$fecha".tar.gz

        cp "$fecha".tar.gz $salida
        rm -f "$fecha".tar.gz
        rm -f "$fecha"
        
        cd $salida
        echo ""$fecha" $arch_existente archivo duplicado" >> log.txt
        
        rm "$nue_arch"
    fi
done


}

options=$(getopt -o s:d:hk --l help,directorio:,salida:,kill -- "$@" 2> /dev/null)
if [ "$?" != "0" ] # equivale a:  if test "$?" != "0"
then
    echo 'Opciones incorrectas'
    exit 1
fi

salidaIn=false
valido1=false
valido2=false
valido3=false

eval set -- "$options"
while true
do
    case "$1" in # switch ($1) { 
        -s | --salida) # case "-e":
           # salida="$2"

            salida=$(realpath -m "$2")
            if [[ -d "$salida" ]]; then 
			    salidaIn=true
            fi
            valido2=true

            shift 2
            ;;
        -d | --directorio)
            directorio=$(realpath -m "$2")
            shift 2

            valido1=true
            ;;
        -k | --kill)
            valido3=true
            shift 1
            ;;
        -h | --help)
            ayuda
            exit 0
            ;;
        --) # case "--":
            shift
            break
            ;;
        *) # default: 
            echo "error"
            exit 1
            ;;
    esac
done


# ------------ EJECUCION EN MODO "DEMONIO" ------------

# Se verifica que se hayan ingresado parametros validos.
if [[ "$valido1" == true && "$valido2" == true ]]; then
	
	# Se verifica que el directorio exista y tenga los permisos de lectura.
	if [[ -d "$directorio" && -r "$directorio" ]]; then
	
		# Se verifica que el directorio a destino exista, si la accion publicar fue
		# especificada.
		if [[ "$salidaIn" == false ]]; then
			
            mkdir $salida
		fi

		demonio &
		trap finalizar SIGUSR1
	else
		echo "$directorio no es un directorio o no posee los permiso de lectura"
		exit
	fi
elif [[ "$valido1" == true && "$valido3" == true ]]; then

    pkill -9 -f "inotifywait -m $directorio"
    pkill -9 -f "$0"

    exit

else
	echo "No se ingresaron parametros validos. Pruebe ingresando ./ejercicio4.sh [-h, --help]"
	exit
fi