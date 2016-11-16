#include "linkedlist.h"

/**
 * Implementations of functions for manipulating a doubly, circular linked list holding XLib
 * Window IDs.
 */


extern Node *first;
extern Node *last;
extern Node *curr_focus;
extern int win_ct;

void insert(Node *new)
{
     if (first == NULL) {
	  first = new;
	  first->next = first;
	  first->prev = first;
	  last = first;
     }
     else {
	  new->next = first;
	  (new->next)->prev = new;
	  first = new;
	  last->next = first;
	  first->prev = last;
     }

     win_ct++;
}

void delete(Window win_id)
{
     Node *curr = first;

     while (curr->win_id != win_id) 
	  curr = curr->next;

     if (curr->next == curr) {
	  last = NULL;
	  first = NULL;
     } 
     else {
	  curr->next->prev = curr->prev;
	  curr->prev->next = curr->next;
	  first = curr->next;
	  if (curr == last) {
	       last = curr->prev;
	  }
     }
     
     free(curr);
     display();
     win_ct--;
}

int is_in_list(Window win_id)
{
     Node *curr = first;

     int i;
     for (i = 0; i < win_ct; i++) {
	  if (curr->win_id == win_id) {
	       return 1;
	  }
     }

     return 0;
}

Node * get_node_from_id(Window win_id)
{
     if (!win_ct) {
	  return NULL; 
     }
     
     Node *curr= first;

     int i;
     for (i = 0; i < win_ct; curr = curr->next, i++) {
	  if (curr->win_id == win_id) {
	       break;
	  }
     }

     if (curr == first && first->win_id != win_id) {
	  printf("%d not found in list.\n", (int) win_id);
	  return NULL;
     }
     
     return curr;
}

void display()
{
     Node *curr = first;
     int i;

     if (first == NULL) {
	  printf("Empty list.\n");
	  return;
     }

     printf("win_ct: %d\n", win_ct);

     for (i = 0; i < win_ct; curr = curr->next, i++) {
	  if (curr == curr_focus) {
	       printf("((%d))->", (int) curr->win_id);
	  }
	  else {
	       printf("[%d]->", (int) curr->win_id);
	  }
     }

     printf("\n");
}
