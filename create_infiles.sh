#!/bin/bash

if [[ "$#" -lt 3 ]]; then
    echo "Invalid number of arguments given."
    echo "Usage: ./testFile.sh <records file> <input directory name> <files per country directory>"
    exit 1
fi

if [[ ! -f $1 ]]; then
    echo "Invalid records file specified."
    echo "Usage: ./testFile.sh <records file> <input directory name> <files per country directory>"
    exit 1
fi

if [[ -d $2 ]]; then
    echo "A directory with the specified name already exists."
    echo "Aborting."
    exit 1
fi

mkdir $2

regex="^[1-9]+[[:digit:]]*$"
filesPerDir=$3

if ! [[ $filesPerDir =~ $regex ]] ; then
    echo "Invalid number of files per country directory. Make sure it is a positive integer."
    echo "Aborting."
    exit 1
fi

numRecords=0
while read -r line; do
    records[numRecords]="$line"
    let "numRecords++"
done < "$1"

numCountries=0

for ((i=0; i < numRecords; i++)) do
    IFS=' ' read -r -a array <<< "${records[i]}"
    if [ "${#array[@]}" -lt 7 ]; then
        echo "ERROR in record: ${records[i]}"
        continue
    fi
    target=${array[3]}
    target_index=""
    for ((j=0; j < numCountries; j++)) do
        if [ "${countries[j]}" == "$target" ]; then
            target_index=$j
            break
        fi
    done
    if [ "$target_index" == "" ]; then
        countries[numCountries]=$target
        country_file_indexes[numCountries]=1
        target_index=$numCountries
        mkdir "$2/${countries[target_index]}"
        let "numCountries+=1"
        for ((j=1; j <= filesPerDir; j++)) do
            touch "$2/${countries[target_index]}/${countries[target_index]}-$j.txt"
        done
    fi
    echo "${records[i]}" >> "$2/${countries[target_index]}/${countries[target_index]}-${country_file_indexes[target_index]}.txt"
    let "country_file_indexes[target_index] %= (filesPerDir + 1)"
    let "country_file_indexes[target_index]+=1"
done

exit 0
