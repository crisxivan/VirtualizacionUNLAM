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

#Integrantes del grupo:
#-Cespedes, Cristian Ivan -> DNI 41704776
#-Gomez, Luciano Dario -> DNI 41572055
#-Luna, Leandro Santiago -> DNI 40886200
#-Panigazzi, Agustin Fabian -> DNI 43744593

Param(
    [string]$people,
    [string]$film
)

if(-not $people -and -not $film) {
    Write-Output "Se esperaba algun parametro"
    exit
}

$cache = @()
if(Test-Path -path people.txt){
    $cache = Get-Content -Path people.txt | ConvertFrom-Csv
}

if ($people) {
    $ids = $people -split ' '
    foreach ($id in $ids) {
        $result = $cache | Where-Object { $_.id -eq $id }
        if (-not $result) {
            try{
                $uri = "https://www.swapi.tech/api/people/$id/"
                $result = Invoke-RestMethod -Uri $uri
                if ($result.message -eq "ok") {
                    $output = @{
                        Id         = $result.result.uid
                        Name       = $result.result.properties.name
                        Gender     = $result.result.properties.gender
                        Height     = $result.result.properties.height
                        Mass       = $result.result.properties.mass
                        BirthYear  = $result.result.properties.birth_year
                    }
                    $output | Select-Object Id, Name, Gender, Height, Mass, BirthYear | Export-Csv -Path people.txt -Delimiter ',' -Append
                    # Agrega el personaje al cache
                    $cache+=$output
                }
                $output | ConvertTo-Json
            }
            catch {
                Write-Error "El personaje con id: $id no existe."
            }
        }else {
            $output = @{
                Id         = $result.id
                Name       = $result.Name
                Gender     = $result.Gender
                Height     = $result.Height
                Mass       = $result.Mass
                BirthYear  = $result.BirthYear
            }
            $output | ConvertTo-Json
        }
    }
}

$cacheFilm = @()
if(Test-Path -path films.txt){
    $cacheFilm = Get-Content -Path films.txt -Raw | ConvertFrom-Csv
}

if ($film) {
    $ids = $film -split ' '
    foreach ($id in $ids) {
        $result = $cacheFilm | Where-Object { $_.uid -eq $id }
        if (-not $result) {
            try {
                $uri = "https://www.swapi.tech/api/films/$id/"
                $result = Invoke-RestMethod -Uri $uri
                if ($result.message -eq "ok") {
                    $output = @{
                        Title        = $result.result.properties.title
                        EpisodeId    = $result.result.properties.episode_id
                        ReleaseDate   = $result.result.properties.release_date
                        OpeningCrawl  = $result.result.properties.opening_crawl
                    }
                    $output | Select-Object @{Name='uid';Expression={$id}}, Title, EpisodeId, ReleaseDate, OpeningCrawl | Export-Csv -Path films.txt -Delimiter ',' -Append
                    $cacheFilm+=$output
                }
                $output | ConvertTo-Json
            }
            catch {
                Write-Error "La pelicula con id: $id no existe."
            }
        }else {
            $output = @{
                Title        = $result.Title
                EpisodeId    = $result.EpisodeId
                ReleaseDate   = $result.ReleaseDate
                OpeningCrawl  = $result.OpeningCrawl
            }
            $output | ConvertTo-Json
        }
    }
}
