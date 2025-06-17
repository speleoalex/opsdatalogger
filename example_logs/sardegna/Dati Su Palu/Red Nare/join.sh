#!/bin/bash

# Creazione di un file temporaneo
temp_file=$(mktemp)

# Concatenazione dei contenuti dei file nella cartella ./all/ in ordine alfabetico
cat ./all/*.txt | sort > "$temp_file"

# Creazione di un nuovo file per il risultato finale
result_file="merged_file2.txt"

# Copia della prima riga (header) nel file risultante
echo '"date Y-m-d m:s";"gas adc";"LPG PPM" ' > "$result_file"

# Eliminazione delle righe contenenti la stringa "date Y-m-d m:s", tranne la prima
sed '1!{/date Y-m-d m:s/d}' "$temp_file" >> "$result_file"

# Rimozione del file temporaneo
rm "$temp_file"

echo "Il file risultante è stato creato: $result_file"
