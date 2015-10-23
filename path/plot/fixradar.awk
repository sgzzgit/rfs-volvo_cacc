BEGIN{
}
{
	wrtime=$1*3600+$2*60+$3
	dbtime=$4*3600+$5*60+$6
	printf("%.3f %.3f", wrtime, dbtime)
	for (i = 7; i<=NF; i++)
		printf(" %s", $i);
	printf("\n");
}
END{
}
