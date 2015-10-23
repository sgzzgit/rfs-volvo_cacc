BEGIN{
}
{
	x = $37
	hr = int($37/10000)	
	min = int(($37-hr*10000)/100)
	sec =  $37-(hr*10000+min*100)
	utctime=hr*3600+min*60+sec
	for (i = 1; i<37; i++)
		printf(" %s", $i);
	printf(" %.3f", utctime);
	for (i = 38; i<NF; i++)
		printf(" %s", $i);
	printf(" %.3f", x);
	printf("\n");
}
END{
}
