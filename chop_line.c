#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "chop_line.h"

chopped_line_t * get_chopped_line( const char * iline )
{
    chopped_line_t * cl;
    char * line_copy;
    const char * delim = " \t\n";
    char * cur_token;

    cl = (chopped_line_t *) malloc ( sizeof(chopped_line_t) );
    cl->tokens = NULL;
    cl->num_tokens = 0;

    if( iline == NULL )
        return cl;

    line_copy = strdup( iline );
    cur_token = strtok( line_copy, delim );
    if( cur_token == NULL )
        return cl;

    do {
        cl->num_tokens++;
        cl->tokens = ( char ** ) realloc( cl->tokens,
                                          cl->num_tokens * sizeof( char * ) );
        cl->tokens[ cl->num_tokens - 1 ] = cur_token;
    } while( cur_token = strtok( NULL, delim ) );

    return cl;
} 

void free_chopped_line( chopped_line_t * icl )
{
    unsigned int i;

    if( icl == NULL )
        return;

    free( icl->tokens );
    free(icl);
}
