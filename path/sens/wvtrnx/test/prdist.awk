BEGIN{
}
{
	doprint = 0
       	for (i = 0; i < 25; i++){
	       	if ($((i*7)+5) == 1) 
			doprint = 1
	}
       	if (doprint) {
	       	printf("%s %s ", $1,$2);
	       	for (i=0; i<25; i++) 
			printf("%.2f ", $((i*7)+8)); printf("\n");
	}
}
END {
}
