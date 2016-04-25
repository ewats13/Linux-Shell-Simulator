#if !defined (__chop_line_h )
#define __chop_line_h 1

typedef struct {
    char ** tokens;           //pointer to "num_tokens" null-terminated strings
    unsigned int num_tokens;  //size of "tokens" string pointer array
} chopped_line_t ;

/* get_chopped_line(): chops a line into individual tokens separated by whitespace 
 * input: a null-terminated line as returned by fgets()
 * return value: chopped_line_t struct containing white space separated tokens from the line  
 */
chopped_line_t * get_chopped_line( const char * iline );

 
/* free_chopped_line(): frees memory allocated for chopped_line_t struct
 * input: chopped_line_t struct returned from get_chopped_line()
 * return value: n/a
 */
void free_chopped_line( chopped_line_t * icl );

#endif /* __chop_line_h */
