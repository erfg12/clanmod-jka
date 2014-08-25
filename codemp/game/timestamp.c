# include <stdlib.h>
# include <stdio.h>
# include <time.h>

# include "timestamp.h"

/******************************************************************************/

double cpu_time ( void )

/******************************************************************************/
/*
  Purpose:

    CPU_TIME returns the current reading on the CPU clock.

  Modified:

    06 June 2005

  Author:

    John Burkardt

  Parameters:

    Output, double CPU_TIME, the current reading of the CPU clock, in seconds.
*/
{
  double value;

  value = ( double ) clock ( ) / ( double ) CLOCKS_PER_SEC;

  return value;
}
/******************************************************************************/

void timestamp ( void )

/******************************************************************************/
/*
  Purpose:

    TIMESTAMP prints the current YMDHMS date as a time stamp.

  Example:

    31 May 2001 09:45:54 AM

  Modified:

    24 September 2003

  Author:

    John Burkardt

  Parameters:

    None
*/
{
# define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  size_t len;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  len = strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  printf ( "%s\n", time_buffer );

  return;
# undef TIME_SIZE
}
/******************************************************************************/

char *timestring ( void )

/******************************************************************************/
/*
  Purpose:

    TIMESTRING returns the current YMDHMS date as a string.

  Example:

    31 May 2001 09:45:54 AM

  Modified:

    24 September 2003

  Author:

    John Burkardt

  Parameters:

    Output, char *TIMESTRING, a string containing the current YMDHMS date.
*/
{
# define TIME_SIZE 40

  const struct tm *tm;
  size_t len;
  time_t now;
  char *s;

  now = time ( NULL );
  tm = localtime ( &now );

  s = malloc ( TIME_SIZE * sizeof ( char ) );

  len = strftime ( s, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  return s;
# undef TIME_SIZE
}
