#!/bin/bash
MAX_CITIZENS=10
numLines=$3

if (( $numLines > $MAX_CITIZENS )); then
    if [ "$4" != "duplicatesAllowed" ]; then
        echo "Cannot create $numLines records without duplicates."
        echo "Up to $MAX_CITIZENS different citizens can be created."
        echo "Re-run with numLines <= $MAX_CITIZENS or use duplicatesAllowed flag."
        echo "Aborting."
        exit 1
    fi
fi

for ((i=0; i <= MAX_CITIZENS; i++)) do
    citizens[i]=""
done

numViruses=0
# Reading Virus names
while read -r virus; do
    viruses[numViruses]="$virus"
    let "numViruses++"
done < "$1"

numCountries=0
# Reading Country names
while read -r country; do
    countries[numCountries]="$country"
    let "numCountries++"
done < "$2"


lineCount=0
# TODO: Generate Random Dates
date="3-2-2021"

while [ "$lineCount" != "$numLines" ]; do
    id=$RANDOM
    let "id %= $MAX_CITIZENS"
    if [ "${citizens[id]}" == "" ]; then
        # Creating new citizen info
        len=$RANDOM
        let "len %= 10"
        let "len+=2"
        name=$(tr -dc A-Za-z </dev/urandom | head -c $len)
        len=$RANDOM
        let "len %= 10"
        let "len+=2"
        surname=$(tr -dc A-Za-z </dev/urandom | head -c $len)
        country_id=$RANDOM
        let "country_id %= $numCountries"
        age=$RANDOM
        let "age %= 120"
        citizens[id]="${id} ${name}_${id} ${surname}_${id} ${countries[country_id]} ${age}"
    else
        if [ "$4" != "duplicatesAllowed" ]; then
            continue
        fi
    fi
    virus_id=$RANDOM
    let "virus_id %= $numViruses"
    vaccinated=$RANDOM
    let "vaccinated %= 2"
    printf '%s %s ' "${citizens[id]}" "${viruses[virus_id]}"
    if [ "$vaccinated" == "0" ]; then
        printf "YES %s\n" "$date"
    else
        printf "NO\n"
    fi
    let "lineCount++"
done > "inputFile"

exit 0