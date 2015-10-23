# Puts gps.log from vehicle PC104 in NEMA-like format.
cat gps.log | grep GGA | sed 's/:/,/' | awk '{print $1,$7,$3,$5,$5}' >gps.fix
