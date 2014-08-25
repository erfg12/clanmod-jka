// Xmod - Needle in the Haystack (in string & lower case)
char lcase(char c)
{
    if (c >= 'A' && c <= 'Z')
        c = c | 0x20;
    return c;
}

int struberstr(char *haystack, char *needle)
{
    char	*np;
	int		i;

    while(*haystack)
    {
        np = needle;

        while(*haystack && lcase(*haystack) != lcase(*np) ) haystack++;
    
        while(*haystack && *np && (lcase(*haystack) == lcase(*np) || *haystack == '^') )
        {
            if (*haystack == '^')
            {
                for (i = 0; i < 2; i++)
				{
					if (*haystack) haystack++;
				}
            }
            else
            {
                if (*haystack) *haystack++;
                if (*np) *np++;
            }
        }
        if (!*np) return 1;
    }

    return 0;
}