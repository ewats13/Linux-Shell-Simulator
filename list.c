#include <stdlib.h>
#include <stdio.h>

#include "list.h"

list_t * list_create( void )
{
    list_t * new_list = ( list_t * )malloc(sizeof(list_t));
    new_list->head = NULL;
    return new_list;
}

void list_clear( list_t * ilist )
{
    struct list_node_t * cur_node, * tmp_node;

    cur_node = ilist->head;
    while( cur_node != NULL ) {
        tmp_node = cur_node;
        cur_node = cur_node->next;
        free( tmp_node );
    }
    ilist->head = NULL;
}

void list_delete( list_t * ilist )
{
    struct list_node_t * cur_node, * tmp_node;

    cur_node = ilist->head;
    while( cur_node != NULL ) {
        tmp_node = cur_node;
        cur_node = cur_node->next;
        free( tmp_node );
    }
    free( ilist );
}

void list_insert_val( list_t * ilist, int i )
{
    struct list_node_t * new_node = (struct list_node_t *)
        malloc(sizeof(struct list_node_t));
    new_node->val = i;
    new_node->next = ilist->head;
    ilist->head = new_node;
}

void list_remove_val( list_t * ilist, int i )
{
    struct list_node_t * prev_node, *cur_node;

    prev_node = NULL;
    cur_node = ilist->head;
    while( cur_node != NULL ) {
        if( cur_node->val == i ) {
            if( prev_node == NULL ) {
                ilist->head = cur_node->next;
            }
            else{
                prev_node->next = cur_node->next;
            }
            free( cur_node );
            return;
        }
        prev_node = cur_node;
        cur_node = cur_node->next;
    }
}
