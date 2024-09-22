<#
.SYNOPSIS
El script permite obtener información de los personajes y/o peliculas de Star Wars

.DESCRIPTION
El script permite la busqueda de personajes y/o peliculas de star wars a partir de su id, obtiene los datos correspondientes a traves
de la api https://www.swapi.tech/ e informa por pantalla su informacion basica.

.PARAMETER -id
Numero de personaje a buscar

.PARAMETER -film
Numero de pelicula a buscar

.EXAMPLE
.\ejercicio5.ps1 -people 1,2,3 -film 1,2
.\ejercicio5.ps1 -people 1,2
.\ejercicio5.ps1 -film 1,2

.NOTES
Archivo de script: ejercicio5.ps1
Autor: 
Versión: 1.0
#>

Param(
    [string]$people,
    [string]$film
)

$cache = @()

if ($people) {
    $ids = $people -split ','
    foreach ($id in $ids) {
        $result = $cache | Where-Object { $_.result.uid -eq $id }

        if (-not $result) {
            $uri = "https://www.swapi.tech/api/people/$id/"
            $result = Invoke-RestMethod -Uri $uri
            if ($result.message -eq "ok") {
                # Agrega el personaje al cache
                $cache += $result
                
            }
        }
        $output = @{
            Id         = $result.result.uid
            Name       = $result.result.properties.name
            Gender     = $result.result.properties.gender
            Height     = $result.result.properties.height
            Mass       = $result.result.properties.mass
            BirthYear  = $result.result.properties.birth_year
        }
        $output | ConvertTo-Json
    
    }
}

$cacheFilm = @()
if ($film) {
    $ids = $film -split ','
    foreach ($id in $ids) {
        $result = $cacheFilm | Where-Object { $_.result.uid -eq $id }

        if (-not $result) {
            $uri = "https://www.swapi.tech/api/films/$id/"
            $result = Invoke-RestMethod -Uri $uri
            if ($result.message -eq "ok") {
                $cacheFilm += $result
            }
        }
        $output = @{
            Title        = $result.result.properties.title
            EpisodeId    = $result.result.properties.episode_id
            ReleaseDate   = $result.result.properties.release_date
            OpeningCrawl  = $result.result.properties.opening_crawl
        }
        $output | ConvertTo-Json
    }
}
