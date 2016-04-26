#include "workspaces.h"
#include "config.h"

extern Workspace *workspaces[NUM_WORKSPACES];
extern Display *dpy;
extern enum TilingAlgorithms {VSTACK, HSTACK, COLS, FULLSCREEN, FLOATING};
extern enum TilingAlgorithms layout_mode;
extern Node *first;
extern Node *last;
extern Node *curr_focus;
extern int float_ct;
extern int win_ct;
extern Workspace *curr_workspace;


void save_workspace_state(Workspace *curr_workspace)
{
     curr_workspace->win_ct = win_ct;
     curr_workspace->float_ct = float_ct;
     curr_workspace->first = first;
     curr_workspace->curr_focus = curr_focus;
     curr_workspace->last = last;
     curr_workspace->layout_mode = layout_mode;
}

void restore_workspace_state(Workspace *curr_workspace)
{
     win_ct = curr_workspace->win_ct;
     float_ct = curr_workspace->float_ct;
     first = curr_workspace->first;
     curr_focus = curr_workspace->curr_focus;
     last = curr_workspace->last;
     layout_mode = curr_workspace->layout_mode;

}

void unmap_all()
{
     Node *curr = first;
     int i;

     for (i = 0; i < win_ct && curr; curr = curr->next, i++) {
	  printf("unmapping........\n");
	  XUnmapWindow(dpy, curr->win_id);
     }
}

void map_all()
{
     Node *curr = first;
     int i;

     for (i = 0; i < win_ct && curr; curr = curr->next, i++) {
	  printf("mapping...........\n");
	  XMapWindow(dpy, curr->win_id);
     }

     printf("No windows to map yet.\n");
}

/**
 * 
 */
void switch_workspace(int src_workspace, int dest_workspace)
{
     // if flicker is an issue, change order of map/unmap
     printf("Switch to workspace %d from %d\n", dest_workspace, curr_workspace->pos);

     curr_workspace->win_ct = win_ct;
     curr_workspace->float_ct = float_ct;
     curr_workspace->first = first;
     curr_workspace->curr_focus = curr_focus;
     curr_workspace->last = last;
     curr_workspace->layout_mode = layout_mode;

     Node *curr = curr_workspace->first;
     
     int i;
     for (i = 0; i < curr_workspace->win_ct; curr = curr->next, i++) {
	  XUnmapWindow(dpy, curr->win_id);
     }
     
     curr_workspace = workspaces[dest_workspace - 1]; // OFF BY ONE ERROR IS FIXED!

     printf("Switching to %d\n", curr_workspace->pos);
     
     win_ct = curr_workspace->win_ct;
     float_ct = curr_workspace->float_ct;
     first = curr_workspace->first;
     curr_focus = curr_workspace->curr_focus;
     last = curr_workspace->last;
     layout_mode = curr_workspace->layout_mode;

     curr = curr_workspace->first;
     
     for (i = 0; i < curr_workspace->win_ct; curr = curr->next, i++) {
	  XMapWindow(dpy, curr->win_id);
     }
     

}

void send_win_to_workspace(Workspace *workspace)
{
     /*   XUnMapWindow(dpy, workspaces[workspace->pos - 1]->curr_focus->win_id);

     delete(workspace[curr_workspace]->curr_focus);
     insert(workspaces[workspace]->curr_focus);

     switch_workspace(curr_workspace, workspace);*/
}

