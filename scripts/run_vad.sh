#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh


DB=/Users/Francesc/Documents/CesC_47/TELECOS/3B/PAV/LAB/P2/db.v4
#/home/albino/PAV/Prácticas/enunciados/obsoletos/P2/db.test
CMD=/Users/Francesc/Documents/CesC_47/TELECOS/3B/PAV/LAB/P2/bin/vad  #write here the name and path of your program
#bin/vad
for filewav in $DB/*/*wav; do
#    echo
    echo "**************** $filewav ****************"
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad}

    $CMD -i $filewav -o $filevad || exit 1

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
#    $CMD $filewav $filevad $filewavOut || exit 1

done

vad_evaluation.pl /Users/Francesc/Documents/CesC_47/TELECOS/3B/PAV/LAB/P2/scripts/*/*lab

exit 0
