function ayuda(){
    echo "Este script obtiene información sobre personajes o peliculas de Star Wars a partir de IDs."
    echo "Los personajes o peliculas se buscan primero en cache y, si no se"
    echo "encuentran, se consultan en la API de swapi y se guarda la información"
    echo "en archivos locales."
    echo "Si se desea ingresar mas de un Id de personaje o pelicula a la vez, deben estar separados por una "coma""
	echo ""
    echo "Uso: $0 [-p|--people <números>] [-f|--film <números>] [-h|--help]"

    echo ""
    echo "------------ AYUDA - FORMATO DEL SRCIPT ------------"
	echo "**Para asegurar el correcto funcionamiento del script, es necesario que antes ejecute los siguientes comandos**"
	echo "I)  sudo apt update"
	echo "II) sudo apt-get install jq"
}


options=$(getopt -o p:f:h --l help,people:,film: -- "$@" 2> /dev/null)
if [ "$?" != "0" ] # equivale a:  if test "$?" != "0"
then
    echo 'Opciones incorrectas'
    exit 1
fi

eval set -- "$options"

while true
do
    case "$1" in
        -p | --people)
        entrada="$2"
        IFS=',' read -r -a vector <<< "$entrada"
        touch "people.txt"

        for elemento in "${vector[@]}"; do

        resultado=$(jq --arg uid "$elemento" '. | select(.result.uid == $uid)' "people.txt")        

        if [ -z "$resultado" ]; then
            resultado=$(curl -s -X GET "https://www.swapi.tech/api/people/$elemento/")
            echo $resultado >> people.txt
        fi

            echo "$resultado" | jq '{
                                        Id: .result.uid,
                                        Name: .result.properties.name,
                                        Gender: .result.properties.gender,
                                        Height: .result.properties.height,
                                        Mass: .result.properties.mass,
                                        Birth_Year: .result.properties.birth_year
                                        }'
        done
        
        shift 2
        ;;
        -f | --film)

        entrada="$2"
        IFS=',' read -r -a vector <<< "$entrada"
        touch "films.txt"

        for elemento in "${vector[@]}"; do

        resultado=$(jq --arg uid "$elemento" '. | select(.result.uid == $uid)' "films.txt")        

        if [ -z "$resultado" ]; then
            resultado=$(curl -s -X GET "https://www.swapi.tech/api/films/$elemento/")
            echo $resultado >> films.txt
        fi

            echo "$resultado" | jq '{
                                        Title: .result.properties.title,
                                        "Episode id": .result.properties.episode_id,
                                        "Release date": .result.properties.release_date,
                                        "Opening crawl": .result.properties.opening_crawl
                                    }'
        done
        
        shift 2
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