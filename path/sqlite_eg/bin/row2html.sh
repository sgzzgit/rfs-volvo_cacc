cat < start_html.txt
sqlite3 other_gps.db "select distinct object_id, latitude, longitude,speed, num_sats,date_time from pttbl limit 8" | awk -F '|' {printf("<tr><td>%s<\td><td>%.7f</td><td>%.7f</td><td>%.2f</td><td>%d</td><td>%s</td</tr>\n", $1,$2,$3,$4,$5,$6,$7)}' 
cat <end_html.txt
