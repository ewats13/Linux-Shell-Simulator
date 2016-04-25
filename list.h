#if !defined( __list_h ) 
#define __list_h 1

struct list_node_t{
    int val;
    struct list_node_t * next;
};

typedef struct {
    struct list_node_t * head;
} list_t;

list_t * list_create( void );
void list_clear( list_t * ilist );
void list_delete( list_t * ilist );
void list_insert_val( list_t * ilist, int i );
void list_remove_val( list_t * ilist, int i );
//void list_remove_node( list_t * ilist, struct list_node_t * in );
//list_node_t * list_find_node( list_t * ilist, int i );

#endif /* __list_h */
