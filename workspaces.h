/** \file 
    \author Patrick Murphy
     
    \brief

    Functions for handling workspaces.
*/

#ifndef WORKSPACES_H

typedef struct Workspace Workspace;

#define WORKSPACES_H
#include <X11/Xlib.h>
#include "linkedlist.h"
#include <stdio.h>


/**
 * A workspace contains a list of windows. It also contains information about
 * the current state of the workspace.
 */
struct Workspace {
     int win_ct;                        // Number of windows in this workspace
     int float_ct;                      // Number of floated windows in this workspace
     Node *first;                       // First window in this workspace's list
     Node *last;                        // Last window in this workspace's list
     Node *curr_focus;                  // Currently focused window in this workspace
     int layout_mode;                   // Current layout mode in this workspace
     int pos;                           // The number associated with this workspace
     int g;
};

void save_workspace_state(Workspace *curr_workspace);

void restore_workspace_state(Workspace *curr_workspace);

/**
 * 
 */
void unmap_all();

/**
 * 
 */
void map_all();

/**
 * 
 */
void switch_workspace(int src_workspace, int dest_workspace);

/**
 * 
 */
void send_win_to_workspace(Workspace *workspace);

#endif
