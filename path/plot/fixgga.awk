BEGIN{
}
{
	x = $2
	hr = int($2/10000)	
	min = int(($2-hr*10000)/100)
	sec =  $2-(hr*10000+min*100)
	utctime=hr*3600+min*60+sec
	printf(" %.3f", utctime);
	printf(" %.6f %.6f", $3, $5);
	printf("\n");
}
END{
}
