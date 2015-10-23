BEGIN{
}
{
	wrtime=$1*3600+$2*60+$3
	printf("%.3f", wrtime)
	for (i = 4; i<=NF; i++)
		printf(" %s", $i);
	printf("\n");
}
END{
}
