#!/bin/bash
MAX_CITIZENS=20
numLines=$3

if [[ ! -f $1 ]]; then
    echo "Invalid viruses file specified."
    echo "Execution: ./testFile.sh <viruses file> <countries file> <num lines> [duplicatesAllowed]"
    exit 1
fi

if [[ ! -f $2 ]]; then
    echo "Invalid countries file specified."
    echo "Execution: ./testFile.sh <viruses file> <countries file> <num lines> [duplicatesAllowed]"
    exit 1
fi

if (( $numLines > $MAX_CITIZENS )); then
    if [ "$4" != "duplicatesAllowed" ]; then
        echo "Cannot create $numLines records without duplicates."
        echo "Up to $MAX_CITIZENS different citizens can be created."
        echo "Re-run with numLines <= $MAX_CITIZENS or use duplicatesAllowed flag."
        echo "Aborting."
        exit 1
    fi
fi

for ((i=0; i < MAX_CITIZENS; i++)) do
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
        let "len %= 11"
        let "len+=1"
        name=$(tr -dc A-Z </dev/urandom | head -c 1)
        name="$name$(tr -dc a-z </dev/urandom | head -c $len)"
        len=$RANDOM
        let "len %= 11"
        let "len+=1"
        surname=$(tr -dc A-Z </dev/urandom | head -c 1)
        surname="$surname$(tr -dc a-z </dev/urandom | head -c $len)"
        country_id=$RANDOM
        let "country_id %= $numCountries"
        age=$RANDOM
        let "age %= 121"
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
        # Generate random date
        day=$RANDOM
        let "day %= 30"
        let "day++"
        month=$RANDOM
        let "month %= 12"
        let "month++"
        year=$RANDOM
        let "year %= 22"
        let "year+=2000"
        date="${day}-${month}-${year}"
        printf "YES %s\n" "$date"
    else
        printf "NO\n"
    fi
    let "lineCount++"
done > "inputFile"

exit 0