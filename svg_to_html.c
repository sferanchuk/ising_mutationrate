
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main( int argc, char **argv )
{
    FILE *ifile = 0;
    FILE *ofile = stdout;
    float offsetx = 0, offsety = 0;
    float scale = 1;
    char c;
    int i;
    int state = 0, braces = 0, ctagpos = 0;
    const int bufsize = 1024;
    char buf[1024] = "";
    const char *closing_tag = "</svg>";
    float screenwidth = 2000, screenheight = 1000;
    
    if ( argc == 1 ) { printf( "replaces width and height and viewBox in input svg file to fit to a screen, %g x %g by default, and add extra <g> tags to transorm a scale and to translate the image\n\narguments: input_file [-o output_file] [-s scale (1)] [-x scaled offset x (0)] [-y scaled offset y (0)] [-w screen width (%g) -h screen height (%g)]\n", screenwidth, screenheight, screenwidth, screenheight ); return 0; }
    
    for ( i = 1; i < argc; i++ )
    {
        if ( argv[i][0] != '-' )
        {
            ifile = fopen( argv[i], "rt" );
            if ( ifile == 0 ) { printf( "incorrect name of input file\n" ); return 1; } 
        }
        else
        {
            switch( argv[ i ][1] )
            {
                case 'o': ofile = fopen( argv[ i + 1 ], "wt" ); break;
                case 's': scale = atof( argv[ i + 1 ] ); break;
                case 'x': offsetx = atof( argv[ i + 1 ] ); break;
                case 'y': offsety = atof( argv[ i + 1 ] ); break;
                case 'w': screenwidth = atof( argv[ i + 1 ] ); break;
                case 'h': screenheight = atof( argv[ i + 1 ] ); break;
            }
            i++;
       }
    }
    if ( ifile == 0 ) { printf( "input file not specified\n" ); return 1; }
            
    fgets( buf, bufsize, ifile );
    if ( strstr( buf, "<?xml" ) == 0 ) fseek( ifile, 0, SEEK_SET );
    fprintf( ofile, "<html><body><svg width=\"%g\" height=\"%g\"", screenwidth, screenheight );
    while ( fread( &c, 1, 1, ifile ) == 1 )
    {
        if ( state == 0 )
        {
            if ( c == '>' )
            {
                fprintf( ofile, "><g transform=\"translate( %g, %g )\"><g transform=\"scale( %g, %g )\">", offsetx, offsety, scale, scale );
                state = 1;
                ctagpos = 0;
            }
            else
            {
                if ( ctagpos >= bufsize )
                {
                    printf( "internal error - buffer for a tag string is too small" );
                    return( 1 );
                }
                buf[ ctagpos ] = c;
                ctagpos++;
                if ( c == '"' )
                {
                    if ( braces == 0 ) braces = 1;
                    else
                    {
                        if ( strstr( buf, "\"height\"" ) != 0  && strstr( buf, "\"width\"" ) != 0 && strstr( buf, "\"viewBox\"" ) != 0 )
                        {
                            printf( "%s", buf );
                            for ( i = 0; i < ctagpos; i++ ) buf[i] = 0;
                            ctagpos = 0;
                        }
                        braces = 0;
                    }
                }
            }
        }
        else
        {
            if ( braces == 0 )
            {
                if ( c == '<' ) 
                {
                    braces = 1;
                    buf[0] = c;
                    ctagpos = 1;
                }
                else putc( c, ofile );
            }
            else
            {
                buf[ ctagpos ] = c;
                if ( c != closing_tag[ ctagpos ] )
                { 
                    for ( i = 0; i <= ctagpos; i++ ) putc( buf[i], ofile );
                    braces = 0;
                    ctagpos = 0;
                }
                else
                {
                    ctagpos++;
                    if ( ctagpos == 6 )
                    {
                        fprintf( ofile, "</g></g>\n</svg></body></html>\n" );
                        return 0;
                    }
                }
            }
        }
    }
    printf( "warning, no closing tag in svg!\n" );
    return 1;
}
 
