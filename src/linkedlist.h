/** \file */ 

#ifndef LINKEDLIST_H

typedef struct Node Node;

#define LINKEDLIST_H
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>

/**
 *  Functions for manipulating a doubly, circular linked list holding XLib Window IDs. 
 */


/**
 * A Node in a circular, doubly linked list. Represents an XLib Window, and holds some
 * extra information about the window's current state.
 */
struct Node {
     Window win_id;
     Window decoration;
     int floating;
     int mapped;
     int transient;
     int pref_x;
     int pref_y;
     int pref_width;
     int pref_height;
     Node *prev;
     Node *next;
};

/**
 * Insert a new Node into the list and update the total count of mapped windows.
 *  Note that a node is always inserted at the front of the list.
 */
void insert(Node *new);

/**
 * Delete a Node from the list in the event that a window is detroyed given it's
 * ID. Update the total count of mapped windows.
 */
void delete(Window win_id);

/**
 * Return 1 if the window with the given win_id is in the list, otherwise
 * return 0.
 */
int is_in_list(Window win_id);

/**
 * Given the window's win_id, return the Node that holds it. If it is not found, 
 * return NULL.
 */
Node * get_node_from_id(Window win_id);

/**
 *  Display the list in text, for debugging purposes.
 */
void display();

#endif



