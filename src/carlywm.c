/** \file 
 *
 *  @author Patrick Murphy
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "linkedlist.h"
#include "workspaces.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ATOM_CT 6

/* Available tiling algorithms, or put another way, layouts. */
enum TilingAlgorithms {VSTACK, HSTACK, COLS, FULLSCREEN, FLOATING};

/* May make things more readable? */
enum NetAtoms {NetSupported, NetNumDesktops, NetCurrentDesktop, NetActiveWindow,
	       NetClientList, NetWMName};

/* Pointers to important nodes in the list */
Node *first = NULL;
Node *last = NULL;
Node *curr_focus = NULL;

Workspace *workspaces[NUM_WORKSPACES];

Workspace *curr_workspace = NULL;

Display *dpy;
int screen;
Window root;

/* Values brought in from config.h */
int GAP;
int PAN;
int MAIN_SZ;

int win_ct;

/* Atoms */
Atom net_atoms[ATOM_CT];

/* Amount of floated windows */
int float_ct;

unsigned screen_height;
unsigned screen_width;
XEvent event;
XWindowAttributes attrs;
XButtonEvent start;

/* Things related to printing of keys (for debugging, will be removed later) */
XComposeStatus compose;
char buffer[20];
int bufferLen = 20;
KeySym keysym;
char *keyASCII;

enum TilingAlgorithms layout_mode;

static void init_ewmh_atoms();
static void update_all_ewmh();
static void set_wm_name_ewmh();
static void active_window_ewmh();
static void client_list_ewmh();
static void setup_workspaces();
static void decrease_gaps();
static void increase_gaps();
static void increase_main();
static void decrease_main();
static void update_focus();
static void focus_next();
static void focus_prev();
static void move_next(Node *current);
static void move_prev(Node *current);
static void float_all_windows();  
static int get_float_ct();
static void tile();
static void force_tile();
static void center_window();
static void teleport();
//static void experimental();
static void vstack();
static void hstack();
static void columns();
static void grab_keys();
static void grab_buttons();
static void cycle_layouts();
static void send_destroy_msg();
static void destroy_notify_event();   
static void key_press_event();
static void button_press_event();
static void motion_notify_event();
static void button_release_event();
static void map_request_event();
static void unmap_notify_event();
static void configure_request_event();
static void swap_window_attrs(Window x, Window y);
static void set_focus_indicators(Window win_id);
static unsigned long get_pixel();
static int get_key_code(const char *keyName);
static void go_to_workspace(int src, int dest);
static Window * query_tree();
static void error(char *message);

/**
 * Return XLib's list of Windows.
 */
Window * query_tree()
{
     Window ret_root;
     Window ret_parent;
     Window *children;
     unsigned int num_children;

     XQueryTree(dpy, root, &ret_root, &ret_parent, &children, &num_children);

     printf("QueryTree: %d\n", num_children);

     return children;
}

void win_name_ewmh(Window win_id)
{
/*     XTextProperty text_prop;
     
     if (!(XGetWMName(dpy, win_id, &text_prop))) {
	  error("XGetWMName");
     }

     XmbTextPropertyToTextList();
	  
     XFree(text_prop);*/

     Node *curr = first;

     int i;
     for (i = 0; i < win_ct; curr = curr->next, i++) {
	  char *win_name;

	  if (!(XFetchName(dpy, curr->win_id, &win_name))) {
	       error("XFetchName");
	  }

	  if (win_name == NULL) {
	       printf("Window name not set by client.\n");
	  }
	  else {
	       printf("ID: %d NAME: %s\n", (int) curr->win_id, *win_name);
	  }
     
	  XFree(win_name);

     }
}

void client_list_ewmh()
{
     /* Can use your own list rather than XLibs? */
     
     Window *win_list = query_tree();
     XChangeProperty(dpy, root, net_atoms[4], XA_WINDOW, 32, PropModeReplace,
		     (unsigned char *) win_list, win_ct);    
     
}

void update_all_ewmh()
{
     // What to update and what to not update?
     client_list_ewmh();
     active_window_ewmh();
}

void set_wm_name_ewmh()
{
     unsigned char *wm_name = "mangowm";
     XChangeProperty(dpy, root, net_atoms[5], XInternAtom(dpy, "UTF8-STRING", False), 8,
		     PropModeReplace, wm_name, strlen(wm_name));
}

void active_window_ewmh()
{
     XChangeProperty(dpy, root, net_atoms[3], XA_WINDOW, 32, PropModeReplace,
		     (unsigned char *) &(curr_focus->win_id), 1);
}

void number_workspaces_ewmh()
{
     XChangeProperty(dpy, root, net_atoms[1], XA_ATOM, 32, PropModeReplace,
		     (unsigned char *) net_atoms, ATOM_CT);
}

void init_ewmh_atoms()
{

     net_atoms[0] = XInternAtom(dpy, "_NET_SUPPORTED", False);

     net_atoms[1] = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", False);

     net_atoms[2] = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", False);

     net_atoms[3] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);

     net_atoms[4] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);

     net_atoms[5] = XInternAtom(dpy, "_NET_WM_NAME", False);
     

     XChangeProperty(dpy, root, net_atoms[1], XA_ATOM, 32, PropModeReplace,
		     (unsigned char *) net_atoms, ATOM_CT);

     set_wm_name_ewmh();
     // that tells what atoms are supported
     
}

/**
 * Wrapper to switch_workspaces() found in workspaces.h, adds ewmh updating 
 */ 
void go_to_workspace(int src, int dest)
{
     printf("Switching workpsace and updating atom property\n");
     
     
     switch_workspace(src, dest);

     dest--;
     
     XChangeProperty(dpy, root, net_atoms[2], XA_CARDINAL, 32, PropModeReplace,
			  (unsigned char *) &(dest), 1);
}

void setup_workspaces()
{
     int i;
     for (i = 0; i < NUM_WORKSPACES; i++) {
	  workspaces[i] = (Workspace *) malloc(NUM_WORKSPACES * sizeof(Workspace));
	  
	  if (workspaces[i] == NULL) {
	       error("malloc");
	  }

	  workspaces[i]->win_ct = win_ct;
	  workspaces[i]->float_ct = float_ct;
	  workspaces[i]->first = first;
	  workspaces[i]->curr_focus = curr_focus;
	  workspaces[i]->last = last;
	  workspaces[i]->layout_mode = layout_mode;
	  workspaces[i]->pos = i + 1;
     }

     curr_workspace = workspaces[0];
     int initial = 1;
     printf("Current workspace is %d.\n", curr_workspace->pos);

     XChangeProperty(dpy, root, net_atoms[1], XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &i, 1);
     XChangeProperty(dpy, root, net_atoms[2], XA_CARDINAL, 32, PropModeReplace,
		     (unsigned char *) &(initial), ATOM_CT);
}

/**
 * Set the border color of the @win_id to the color defined by FOCUSED, and set the
 * border color of every other window in the list to the color defined by UNFOCUSED.
 */
void set_focus_indicators(Window win_id)
{
     Node *curr = first;
     Node *focus = get_node_from_id(win_id);
     curr_focus = focus;
     
     XSetWindowBorder(dpy, focus->win_id, get_pixel(FOCUSED));
     XSetWindowBorderWidth(dpy, focus->win_id, BORDER);
          
     int i;
     for (i = 0; i < win_ct; curr = curr->next, i++) {
	  if (curr->win_id == focus->win_id) {
	       continue;
	  }

	  XSetWindowBorder(dpy, curr->win_id, get_pixel(UNFOCUSED));
	  XSetWindowBorderWidth(dpy, curr->win_id, BORDER);
     }
}

/**
 * For the window represented by the given @node, get it's preferred configuraton and
 * XMoveResizewindow() the window accordinly. This function is used when some window
 * is put in floating mode.
 */
void sizeFromHints(Node *node)
{
     XSizeHints *size_hints;
     long n = -1;

     if (!(size_hints = XAllocSizeHints())) {
	  fprintf(stderr, "Failure allocating memory.");
	  exit(0);
     }

     if (!(XGetWMNormalHints(dpy, node->win_id, size_hints, &n))) {
	  printf("Client has not specified hints.\n");
     }
	 
     printf("Client's preferred config:\nx: %d\ny: %d\nwidth: %d\nheight: %d\n",
	    size_hints->x, size_hints->y, size_hints->base_width, size_hints->base_width);

     if (node->pref_x && node->pref_y && node->pref_height && node->pref_width) {
	  XMoveResizeWindow(dpy, node->win_id, node->pref_x, node->pref_y, node->pref_width, node->pref_height);
     }

     XFree(size_hints);
}

/**
 * 'Grab' the keys that the program will use.
 */ 
void grab_keys()
{
     XGrabKey(dpy, get_key_code("Return"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("Tab"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("space"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("f"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("a"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("d"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("e"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("m"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("q"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("F"), ALT|SHIFT, root, True, GrabModeAsync, GrabModeAsync);    
     XGrabKey(dpy, get_key_code("Shift_L"), AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("Right"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("Left"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("Up"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("Down"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("minus"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("equal"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("bracketleft"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("bracketright"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("h"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("l"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("J"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("K"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("j"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("k"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("r"), ALT, root, True, GrabModeAsync, GrabModeAsync);
     XGrabKey(dpy, get_key_code("c"), ALT, root, True, GrabModeAsync, GrabModeAsync);

     /* Workspace control key grabs */
     int i;
     char str[1];
     for (i = 1; i < NUM_WORKSPACES + 1; i++) {
	  sprintf(str, "%d", i);


	  XGrabKey(dpy, get_key_code(str), ALT, root, True, GrabModeAsync, GrabModeAsync);
	  }

     // XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime); // for quickly getting keycodes
}

/**
 * 'Grab' the mouse buttons that the program will use.
 */
void grab_buttons()
{
     XGrabButton(dpy, 1, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync,
		 None, None);
     XGrabButton(dpy, 3, Mod1Mask, root, True, ButtonPressMask, GrabModeAsync, GrabModeAsync,
		 None, None);
}

/**
 * Send a message to a client in hopes that it will close itself properly rather than simply 
 * killing the process.
 */
void send_destroy_msg()
{
     if (win_ct) {
	  XEvent ev = {.type = ClientMessage};
	  ev.xclient.window = curr_focus->win_id;
	  ev.xclient.format = 32;
	  ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", False);
	  ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	  ev.xclient.data.l[1] = CurrentTime;
	  XSendEvent(dpy, curr_focus->win_id, False, NoEventMask, &ev);
//	  XDestroyWindow(dpy, curr_focus->win_id); // Don't do this! use XKillClient if the client supports it
     }
}

/**
 *  For the handling of a 'DestroyNotify' XEvent when it is received in the event loop. 
 */
void destroy_notify_event()
{
     Window win_id = event.xdestroywindow.window;
//     printf("DESTROY NOTIFY EVENT (%d)\n", (int) win_id);

     Node *to_destroy = get_node_from_id(win_id);

     if (win_ct && to_destroy != NULL) {
	  delete(win_id);
     }

     if (is_in_list(win_id)) {
	  XKillClient(dpy, win_id);
     }

     if (win_ct && layout_mode != FLOATING) {
	  tile();
     }
	       
     update_focus();
}

/**
 * For the handling of a 'UnmapNotify' XEvent when it is received in the event loop.
 */
void unmap_notify_event()
{
     // May not need to implement this
}

/**
 * For the handling of a 'MapRequest' XEvent when it is received in the event loop.
 */
void map_request_event()
{
//     printf("MAP REQUEST EVENT (%d)\n", (int) event.xmaprequest.window);
     Node *new_win;
     new_win = (Node *) malloc(sizeof(Node));
     new_win->win_id = event.xmaprequest.window;
     new_win->floating = 0;
     new_win->decoration = 0;  // unused attribute
     new_win->mapped = 1;      // unused attribute

     /* Check if window is a transient window */
     new_win->transient = XGetTransientForHint(dpy, new_win->win_id,&event.xmaprequest.window);
//     printf("Transient? %d\n", new_win->transient);
	       
     if (is_in_list(new_win->win_id) == 0 && new_win->transient == 0) {
	  insert(new_win);
     } 
	       
     if (new_win->transient) {
	  printf("Not inserting window into list, it's transient, will float.\n");
	  XMoveWindow(dpy, new_win->win_id, screen_width / 2, screen_height / 2);
	  insert(new_win);
	  new_win->floating = 1;
     }

     XMapWindow(dpy, new_win->win_id);
     XSetInputFocus(dpy, new_win->win_id, RevertToPointerRoot, CurrentTime);
     curr_focus = new_win;

     XSetWindowBorder(dpy, new_win->win_id, get_pixel(FOCUSED));
     XSetWindowBorderWidth(dpy, new_win->win_id, BORDER);

     /* Make sure the borders indicate focus */
     set_focus_indicators(new_win->win_id);

     /* Tile this new window, unless we are in FLOATING mode. */
     if (layout_mode != FLOATING && new_win->transient == 0) {
	  tile();
     }
     else {
	  center_window(new_win->win_id);
     }
}

/**
 * For the handling of a 'ConfigureRequest' XEvent when it is received in the event loop.
 * I was having issues with applications crashing after being mapped until I looked at some 
 * other window managers. Apparently, some programs must be configured with their desired
 * configurations before it's 'safe' to impose our own. So, this function applies the application's
 * desired configuration and THEN tiles (effectively undoing all that was set) to keep programs from
 * crashing. 
 */ 
void configure_request_event()
{
//     printf("CONFIGURE REQUEST EVENT\n");
     XConfigureRequestEvent ev = event.xconfigurerequest;
     printf("configuration x: %d y: %d width: %d height: %d\n", ev.x, ev.y, ev.width, ev.height);

     XWindowChanges changes;
     changes.x = ev.x;
     changes.y = ev.y;
     changes.width = ev.width;
     changes.height = ev.height;
     changes.border_width = ev.border_width;
     changes.sibling = ev.above;
     changes.stack_mode = ev.detail;

     if (get_node_from_id(ev.window) != NULL) {
	  XConfigureWindow(dpy, ev.window, ev.value_mask, &changes);
     }

     if (layout_mode != FLOATING) {
	  tile(); // Now undo all of the above and impose our own attributes
     }
}

/**
 * I don't know if this is very useful, but current focus will jump to the location of the
 * pointer (the window will move there).
 */
void teleport()
{
     XGrabPointer(dpy, curr_focus->win_id, True, PointerMotionMask | ButtonMotionMask |
		  ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None,
		  None, CurrentTime);

     XSetInputFocus(dpy, curr_focus->win_id, RevertToPointerRoot, CurrentTime);
     XRaiseWindow(dpy, curr_focus->win_id);
}

/**
 * For the handling of a 'ButtonPress' XEvent when it is received in the event loop.
 */
void button_press_event()
{

     XGrabPointer(dpy, event.xbutton.subwindow, True, PointerMotionMask | ButtonMotionMask |
		  ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None,
		  None, CurrentTime);

     XGetWindowAttributes(dpy, event.xbutton.subwindow, &attrs);
     XSetInputFocus(dpy, event.xbutton.subwindow, RevertToPointerRoot, CurrentTime);
     XRaiseWindow(dpy, event.xbutton.subwindow);

     set_focus_indicators(event.xbutton.subwindow);
     
     start = event.xbutton;
}

/**
 * For the handling of a 'ButtonRelease' XEvent when it is received in the event loop.
 */
void button_release_event()
{
     XUngrabPointer(dpy, CurrentTime);
}

/**
 * For the handling of a 'MotionNotify' XEvent when it is received in the event loop.
 */
void motion_notify_event()
{
     int xdiff;
     int ydiff;

     /* In order to avoid tiling during every motion event */
     int tiled = 0;

     /* Major hack here -- I have issues retiling the other windows when the main
	main window is being floated... so, don't allow users to float it... */
     if (curr_focus && curr_focus != last) {
	  curr_focus->floating = 1;
     }

     /* Possible speedup here, according to TinyWM */
     while(XCheckTypedEvent(dpy, MotionNotify, &event));
	       
     xdiff = event.xbutton.x_root - start.x_root;
     ydiff = event.xbutton.y_root - start.y_root;

     /* Left mouse button --> moving */
     if (start.button == 1) {
	  XMoveWindow(dpy, event.xmotion.window,
		      attrs.x + xdiff,              // x
		      attrs.y + ydiff);             // y
     }
     /* Right mouse button --> resizing */
     else if (start.button == 3) {
	  XResizeWindow(dpy, event.xmotion.window,
			MAX(1, attrs.width + xdiff),
			MAX(1, attrs.height + ydiff));
     }

     XFlush(dpy);

     /* If we haven't already tiled, and we're not in floating mode, and the window we are working
	on is not a transient window, go ahead and tile the tilable windows. */
     if (!tiled && layout_mode != FLOATING &&
	 (get_node_from_id(event.xbutton.window) && ((get_node_from_id(event.xbutton.window))->transient == 0))) {

	  // Bring it to front
	  XRaiseWindow(dpy, event.xbutton.window);
	  
	  tile();
	  tiled = 1;
     }
}

/**
 * For the handling of a 'Keypress' event when it is received in the event loop.
 * Catches the key combination pressed, and takes the appropriate action.
 */
void key_press_event()
{
     XLookupString(&event.xkey, buffer, bufferLen, &keysym, &compose);
     keyASCII = XKeysymToString(keysym);
       printf("Key: %s, KeySym: %d \n", keyASCII, (int) keysym);
       fflush(stdout);
	       
     switch(keysym) {

     case RET:
	  /* Key binding to launch the terminal specified in config.h */
	  system(TERM);
	  break;
     case TAB:
	  printf("Force tile.\n");
	  /* Force all windows to tile */
	  force_tile();
	  break;
     case h_KEY:
	  printf("Warpin\n");
	  XWarpPointer(dpy, None, root, 0, 0, 0, 0, 50, 50);

	  break;
     case l_KEY:
	  query_tree();
	  update_all_ewmh();
	  win_name_ewmh(0);
	  printf("Updating ALL EWMHs!!!!!\n");
	  display();
	  break;
     case c_KEY:
	  if (curr_focus != last && win_ct > 1) {
	       center_window(curr_focus->win_id);
	       tile();
	  }
	  break;
     case a_KEY:
	  /* Key binding to launch an application launcher */
	  system(LAUNCHER);
	  break;
     case d_KEY:
	  break;
     case e_KEY:
	  /* key binding to launch the editor specified in config.h */
	  system(EDITOR);
	  break;
     case f_KEY:
	  /* Float all windows -- puts in FLOATING mode w/o having to cycle through layouts */
	  float_all_windows();
	  break;
     case RIGHT:
	  
	  break;
     case LEFT:

	  break;
     case UP:

	  break;
     case DOWN:

	  break;
     case j_KEY:
	  /* Change focus to the window NEXT in the list */
	  if (layout_mode == VSTACK || layout_mode == FULLSCREEN || layout_mode == FLOATING)  {
	       focus_next();
	  }
	  else {
	       /* Because the order of windows in inverted in these other layouts */
	       focus_prev();
	  }

	  break;
     case k_KEY:
	  /* Change focus to the window PREV in the list */
	  if (layout_mode == VSTACK || layout_mode == FULLSCREEN || layout_mode == FLOATING)  {
	       focus_prev();
	  }
	  else {
	       focus_next();
	  }

	  break;
     case J_KEY:
	  /* Move this window to the position of the window NEXT in the list */
	  if (layout_mode == VSTACK || layout_mode == FULLSCREEN || layout_mode == FLOATING)  {
	       move_next(curr_focus);
	  }
	  else {
	       move_prev(curr_focus);
	  }
	  
	  break;
     case K_KEY:
	  /* Move this window to the position of the window PREV in the list */
	  if (layout_mode == VSTACK || layout_mode == FULLSCREEN || layout_mode == FLOATING)  {
	       move_prev(curr_focus);
	  }
	  else {
	       move_next(curr_focus);
	  }

	  break;
     case BRACKET_RIGHT:
	  /* Decrease the size of the currently focused window horizontally */
	  decrease_main();
	  break;
     case BRACKET_LEFT:
	  /* Increase the size of the currently focused window horizontally */
	  increase_main();
	  break;
     case m_KEY:
	  /* Key binding to launch the browser specified in config.h */
	  system(BROWSER);
	  break;
     case MINUS:
	  /* Decrease the size of all gaps around windows */
	  decrease_gaps();
	  break;
     case EQUAL:
	  /* Increase the size of all gaps around windows */
	  increase_gaps();
	  break;
     case q_KEY:
	  /* Send a client a polite destroy message so it will close properly */
	  send_destroy_msg();
	  break;
     case r_KEY:
	  /* Dumb */
	  teleport();
	  break;
     case SPC:
	  /* Cycle the available layouts -- they are applied automatically */
	  cycle_layouts();
	  break;
     case KEY_1:
	  go_to_workspace(curr_workspace->pos, 1);
	  display();
	  tile();
	  break;
     case KEY_2:
	  go_to_workspace(curr_workspace->pos, 2);
	  display();
	  tile();
	  break;
     case KEY_3:
	  go_to_workspace(curr_workspace->pos, 3);
	  display();
	  tile();
	  break;
     case KEY_4:
	  go_to_workspace(curr_workspace->pos, 4);
	  break;
     case KEY_5:
	  go_to_workspace(curr_workspace->pos, 5);
	  break;
     default:
	  printf("Key not currently bound to anything.\n");
     }
}

/**
 * Swaps window information, in other words, Window x will now have the coordinates 
 * and dimensions of Window y, and vice-versa. 
 */
void swap_window_attrs(Window x, Window y)
{
     XWindowAttributes xattrs;
     XGetWindowAttributes(dpy, x, &xattrs);

     XWindowAttributes yattrs;
     XGetWindowAttributes(dpy, y, &yattrs);

     int xx = xattrs.x;
     int xy = xattrs.y;
     int xw = xattrs.width;
     int xh = xattrs.height;

     /* Move + Resize Window x */
     XMoveResizeWindow(dpy, x, yattrs.x, yattrs.y, yattrs.width, yattrs.height);

     /* Move + Resize Window y */
     XMoveResizeWindow(dpy, y, xx, xy, xw, xh);
}

/**
 * Move the window one position forward in the list. Note that the Nodes in
 * in the list are not actually swapped, only a Node's win_id.
 */ 
void move_next(Node *current)
{
     if (!current || !current->next) {
	  return;
     }
     
     Window tmp;

     tmp = current->win_id;
     current->win_id = current->next->win_id;
     current->next->win_id = tmp;
     
     swap_window_attrs(current->win_id, current->next->win_id);

     /* major hack -- I don't know why this works, but it does */
     focus_next();
     focus_next();
     focus_prev();
}

/**
 * Move the window one position backward in the list. Note that the Nodes in
 * in the list are not actually swapped, only a Node's win_id.
 */ 
void move_prev(Node *current)
{
     if (!current || !current->prev) {
	  return;
     }

     Window tmp;

     tmp = current->win_id;
     current->win_id = current->prev->win_id;
     current->prev->win_id = tmp;
     
     swap_window_attrs(current->win_id, current->prev->win_id);

     /* major hack -- I don't know why this works, but it does */
     focus_prev();
     focus_prev();
     focus_next();
}


/**
 * Cycle available tiling layouts. If there are windows to tile, tile() is called
 *  to apply the new layout.
 */
void cycle_layouts()
{
     layout_mode = (layout_mode + 1) % 5;

     if (win_ct) {
	  /* Quick way of setting windows as non-floating when going from 
	     the floating mode to vstack. */
	  if (layout_mode == VSTACK) {
	       force_tile();
	  }
	  else {
	       tile();
	  }
     }
}

/**
 * Increase the size of the main window (horizontally) while tiled, then retile.
 */
void increase_main()
{
     /* Main window should always be last in the list, or both first and last if 
	it is the only window. */
     if (MAIN_SZ < screen_width && curr_focus == last) {
	  MAIN_SZ += RESIZE_INC;
     }
     else if (curr_focus != last) {
	  MAIN_SZ -= RESIZE_INC;
     }
     else {
	  return;
     }
     
     tile();
}

/**
 * Decrease the size of the main window (horizontally) while tiled, then retile.
 */
void decrease_main()
{
     /* Main window should always be last in the list, or both first and last if 
	it is the only window. */
     if (MAIN_SZ && curr_focus == last) {
	  MAIN_SZ -= RESIZE_INC;
     }
     else if (curr_focus != last) {
	  MAIN_SZ += RESIZE_INC;
     }
     else {
	  return;
     }

     tile();
}

/**
 * Decrease the value of the GAP value (gaps between tiled windows) by GAPS_INC, and 
 * apply the change by retiling.
 *
 * -- Add bounds checking
 */
void decrease_gaps()
{
     GAP -= GAPS_INC;

     if (win_ct) {
	  tile();
     }
}

/**
 * Increase the value of the GAP value (gaps between tiled windows) by GAPS_INC, and 
 * apply the change by retiling.
 *
 * -- Add bounds checking
 */
void increase_gaps()
{
     GAP += GAPS_INC;

     if (win_ct) {
	  tile();
     }
}

/**
 * When the list of windows is altered -- by a deletion, for example -- the focus
 * Must be updated. This involves setting the input focus with XSetInputFocus(), 
 * updating 'curr_focus', updating various window borders to indicate focus, and 
 * raising the newly focused window.
 */
void update_focus() 
{
     if (first != NULL && first->mapped == 1) {  
	  /* Change input focus and update 'curr_focus' */
	  XSetInputFocus(dpy, first->win_id, RevertToPointerRoot, CurrentTime);
	  curr_focus = first;

	  /* Set focused border of newly focused window */
	  XSetWindowBorder(dpy, first->win_id, get_pixel(FOCUSED));
	  XSetWindowBorderWidth(dpy, first->win_id, BORDER);

	  /* Raise Window */
	  XRaiseWindow(dpy, first->win_id);
     }
}

/**
 * Change focus to the next window in the list (if windows exist). This involves setting the 
 * input focus with XSetInputFocus(), updating 'curr_focus', updating various window borders,
 * and raising the newly focused window.
 */
void focus_next()
{
     if (!win_ct || curr_focus->transient) {
	  return;
     }

     /* Change input focus  and update 'curr_focus' */
     XSetInputFocus(dpy, curr_focus->next->win_id, RevertToPointerRoot, CurrentTime);
     curr_focus = curr_focus->next;

     /* Set focused border of newly focused window */
     XSetWindowBorder(dpy, curr_focus->win_id, get_pixel(FOCUSED));
     XSetWindowBorderWidth(dpy, curr_focus->win_id, BORDER);

     /* Set unfocused border of previously focused window */
     XSetWindowBorder(dpy, curr_focus->prev->win_id, get_pixel(UNFOCUSED));
     XSetWindowBorderWidth(dpy, curr_focus->prev->win_id, BORDER);

     /* Raise Window -- won't notice a difference unless in fullscreen mode */
     XRaiseWindow(dpy, curr_focus->win_id);
}

/**
 * Change focus to the previous window in the list (if windows exist). This involves setting the 
 * input focus with XSetInputFocus(), updating 'curr_focus', updating various window borders,
 * and raising the newly focused window.
 */
void focus_prev()
{
     // needs to check if there is anything to focus at all
     if (!win_ct || curr_focus->transient) {
	  return;
     }

     /* Change input focus */
     XSetInputFocus(dpy, curr_focus->prev->win_id, RevertToPointerRoot, CurrentTime);
     curr_focus = curr_focus->prev;

     /* Set focused border of newly focused window */
     XSetWindowBorder(dpy, curr_focus->win_id, get_pixel(FOCUSED));
     XSetWindowBorderWidth(dpy, curr_focus->win_id, BORDER);

     /* Set unfocused border of previously focused window */
     XSetWindowBorder(dpy, curr_focus->next->win_id, get_pixel(UNFOCUSED));
     XSetWindowBorderWidth(dpy, curr_focus->next->win_id, BORDER);

     /* Raise Window */
     XRaiseWindow(dpy, curr_focus->win_id);
}

/**
 * Given the window @win_id, center it based on screen dimensions and it's own size.
 */
void center_window(Window win_id)
{
     XWindowAttributes attrs;
     XGetWindowAttributes(dpy, win_id, &attrs);
     XMoveWindow(dpy, win_id,
		 (screen_width / 2) - (attrs.width / 2),
		 (screen_height / 2) - (attrs.height / 2));

     (get_node_from_id(win_id))->floating = 1;
}

/**
 * Float every window mapped to the screen.
 *
 * -- Handle issue of many windows causing offscreen stacking
 * -- This command ought to be MOD + SHIFT + F
 */
void float_all_windows()
{
     layout_mode = FLOATING;
     int i;
     
     Node *curr = first;

     int yOffset = 0;
     int xOffset = 0;

     for (i = 0; i < win_ct; curr = curr->next, i++) {

	  int windowWidth = (screen_width / 3) + (screen_width / 100);
	  int windowHeight = screen_height / 3;
	  int x = screen_width / 2 - (windowWidth / 2);
	  int y = screen_height / 2 - (windowHeight / 2);

	  XMoveResizeWindow(dpy, curr->win_id, x + xOffset, y + xOffset, windowWidth,
			    windowHeight);
	  xOffset -= 30;
	  yOffset -= 30;

	  curr->floating = 1;

	  XLowerWindow(dpy, curr->win_id);
     }
}

/**
 * Force a retile of all windows mapped to the screen.
 */
void force_tile()
{
     Node *curr = first;
     int i;
     
     for (i = 0; i < win_ct; i++) {
	  if (curr->transient == 0) {
	       curr->floating = 0;
	  }
	  curr = curr->next;
     }

     layout_mode = VSTACK;
     tile();
}

/**
 *  Return the number of windows in the list currently marked as floating. 
 */
int get_float_ct()
{
     Node *curr = first;

     int i;
     int num = 0;
     for (i = 0; i < win_ct; curr = curr->next, i++) {
	  if (curr->floating) {
	       num++;
	  }
     }

     return num;
}

/* Just for testing tiling stuff 
void experimental()
{
     Node *curr = first;

     int i;
     for (i = 0; i < win_ct; curr = curr->next, i++) {
	  
     }
     }*/

/**
 * Tile windows in the classic 'vertical stack' layout.
 *
 * -- Try to redesign algorithm so tiling is done in single for loop.
 */
void vstack()
{
     printf("VSTACK\n");
     
     Node *curr = first;

     /* Get the number of windows to be tiled -- (total wins - floating wins) */
     float_ct = get_float_ct();
     int tile_ct = win_ct - float_ct;
     
     /* Special case: one window */
     if (tile_ct == 1) {
	  XMoveResizeWindow(dpy, first->win_id,
			    0 + (GAP),
			    0 + (GAP) + PAN,
			    screen_width - (2 * GAP) - 2,
			    screen_height - (2 * GAP) - PAN);
     }
     else {
	  int i;
	  int new_y;
	  int new_h;

	  for (i = 0; i < tile_ct; curr = curr->next, i++) {
	       if (curr->floating || curr->transient) {
		    printf("Not tiling this floating window.\n");
		    i--;
		    continue;
	       }

	       /* Main window (half the screen space) */
	       if (curr->next == first) {
		    XMoveResizeWindow(dpy, curr->win_id,
				      0 + (GAP),
				      0 + (GAP) + PAN,
				      MAIN_SZ - (2 * GAP) - 2,
				      screen_height - (2 * GAP) - PAN);
	       }
	       else {
		    new_y = screen_height / (tile_ct - 1) * i;
		    new_h = screen_height / (tile_ct - 1);

		    
		    if (i == 0) {
			 XMoveResizeWindow(dpy, curr->win_id,       
					   MAIN_SZ,
					   new_y + GAP + PAN,  
					   (screen_width - MAIN_SZ) - GAP - 2,
					   new_h - (2 * GAP) - PAN);
		    }
		    else if (i > 0) {
			 XMoveResizeWindow(dpy, curr->win_id, 
					   MAIN_SZ ,
					   new_y,           
					   (screen_width - MAIN_SZ) - GAP - 2,
					   new_h - GAP);        
		    }
		    
	       }
	  }
     }
}

/**
 *  Tile windows in 'horizontal stack' style (flipped version of vertical stack). 
 */
void hstack()
{
     printf("HSTACK\n");
     
     Node *curr = first;

     if (win_ct == 1) {
	  XMoveResizeWindow(dpy, first->win_id,
			    0 + GAP - 2,
			    0 + GAP + PAN,
			    screen_width - (2 * GAP) + 4,
			    screen_height - (2 * GAP) - PAN);
     }
     else if (win_ct == 2) {
	  XMoveResizeWindow(dpy, first->win_id,
			    0 + GAP,
			    (screen_height / 2),
			    screen_width - (2 * GAP),
			    (screen_height / 2) - GAP);

	  XMoveResizeWindow(dpy, first->prev->win_id,
			    0 + GAP - 2,
			    0 + GAP + PAN,
			    screen_width - (2 * GAP) + 4,
			    (screen_height / 2) - (2 * GAP) - PAN);
     }
     else {
	  int i;
	  int newX;
	  int newWidth;
	  
	  for (i = 0; i < win_ct; curr = curr->next, i++) {
	       if (curr == last) {
		    XMoveResizeWindow(dpy, first->prev->win_id,
				      0 + GAP - 2,
				      0 + GAP + PAN,
				      screen_width - (2 * GAP) + 4,
				      (screen_height / 2) - (2 * GAP) - PAN);
	       }
	       else {
		    newX = screen_width / (win_ct - 1) * i;
		    newWidth = screen_width / (win_ct - 1);

		    if (i == 0) {
			 XMoveResizeWindow(dpy,
					   curr->win_id,         
					   newX + GAP,          
					   screen_height / 2,    
					   newWidth - (GAP * 2),      
					   (screen_height / 2) - GAP); 
		    }
		    else if (i > 0) {
			 XMoveResizeWindow(dpy, curr->win_id,
					   newX,
					   screen_height / 2,
					   newWidth - GAP,
					   (screen_height / 2) - GAP);
		    }
	       }
	  }
     }
}

/**
 * Tile windows in such a way that all windows are equally sized, in a column fashion.
 *
 * -- Fix issues in placement of windows
 */ 
void columns()
{
     printf("COLS\n");
     
     Node *curr = first;
     int new_x = 0;
     int i;

     for (i = 0; i < win_ct; curr = curr->prev, new_x += screen_width / win_ct, i++) {
	  if (curr->next == first) {
	       XMoveResizeWindow(dpy, curr->win_id,
				 new_x + (GAP) - 2,
				 0 + GAP + PAN,
				 (screen_width / win_ct) - (2 * GAP) + 4,
				 screen_height - (2 * GAP) - PAN);
	  }

	  else {
	       XMoveResizeWindow(dpy, curr->win_id,
				 new_x + (GAP / 2) - 2,                
				 0 + GAP + PAN,                        
				 (screen_width / win_ct) - GAP + 4,    
				 screen_height - (2 * GAP) - PAN);     
	  }
     }
}

/**
 * Give the window full screen space (minus any gaps or borders)
 * 
 * -- minus the borders
 */ 
void fullscreen()
{
     printf("FULLSCREEN\n");
     
     Node *curr = last;
     int i;

     for (i = 0; i < win_ct; curr = curr->prev, i++) {
	  XMoveResizeWindow(dpy, curr->win_id,
			    0 + (GAP - 2),
			    0 + (GAP) + PAN,
			    screen_width - (2 * GAP) + 4,
			    screen_height - (2 * GAP) - PAN);
     }
}

/**
 * Select the tiling layout based on 'layout_mode' and tile the windows accordingly.
 */
void tile()
{
     if (win_ct == 0) {
	  return;
     }

     switch (layout_mode) {
     case VSTACK:
	  vstack();
	  break;
     case HSTACK:
	  hstack();
	  break;
     case COLS:
	  columns();
	  break;
     case FULLSCREEN:
	  fullscreen();
	  break;
//     case EXPERIMENTAL:
	  // For testing tiling stuff
//	  experimental();
//	  break;
     case FLOATING:
	  float_all_windows();
	  break;
     default:
	  printf("Undefined layout_mode value.\n");
     }
}


/**
 * Return a pixel with the given color. 
 */
unsigned long get_pixel(const char *colorCode)
{
     XColor color;
     XAllocNamedColor(dpy, DefaultColormap(dpy, screen), colorCode, &color, &color);
     return color.pixel;
}

/**
 * Given the key name, convert to a Keysym, then convert to and return the Keycode. 
 */
int get_key_code(const char *keyName)
{
     return XKeysymToKeycode(dpy, XStringToKeysym(keyName));
}

/**
 * The main function initializes a display, grabs keys and buttons, selects which
 * XEvents to listen for, and runs the event loop. Also propagates the EWMH hints
 * by calling the function init_ewmh_atoms().
 */ 
int main()
{
     /* Open the display */
     if (!(dpy = XOpenDisplay(NULL))) {
	  printf("Cannot open display.\n");
     }

     float_ct = 0;
     win_ct = 0;
     layout_mode = VSTACK;  // Initial tiling algorithm is vertical stack
     
     screen = DefaultScreen(dpy);
     root = RootWindow(dpy, screen);
     screen_height = DisplayHeight(dpy, screen);
     screen_width = DisplayWidth(dpy, screen);
     
     init_ewmh_atoms();
     
     /* Setup workspaces */
     setup_workspaces();

     printf("Set up workspaces.\n");

     /* Set the values for gaps and panel if they are turned on in config.h */
     GAP = (GAPS) ? GAP_SIZE : 0; 
     PAN = (PANEL) ? PANEL_HEIGHT : 0;

     /* Bring in main window size from config.h */
     MAIN_SZ = screen_width * MAIN_WIN_SIZE;

     /* Grab keys and buttons */
     grab_keys();
     grab_buttons();

     
     
     /* What XEvents to listen for */
     XSelectInput(dpy, root,
		  SubstructureRedirectMask
		  | SubstructureNotifyMask
		  | StructureNotifyMask
		  | ButtonReleaseMask
		  | EnterWindowMask
		  | PropertyChangeMask
		  | ButtonPressMask
		  | ButtonMotionMask);

     while (1) {
	  XNextEvent(dpy, &event);  // Get a new event 

	  if (event.type == MapRequest) {
	       map_request_event();
	  }
	  else if (event.type == UnmapNotify) {
	       unmap_notify_event();
	  }
	  else if (event.type == ConfigureRequest) {
	       configure_request_event();
	  }
	  else if (event.type == KeyPress) {
	       key_press_event();
	  }
	  else if (event.type == ButtonPress && event.xbutton.subwindow != None) {
	       button_press_event();
	  }
	  else if (event.type == ButtonRelease) {
	       button_release_event();
	  }
	  else if (event.type == MotionNotify) {
	       motion_notify_event();
	  }
	  else if (event.type == DestroyNotify) {
	       destroy_notify_event();
	  }
	  else if (event.type == ClientMessage) {
	       printf("client message\n");
	  }
	  else if (event.type == PropertyNotify) {
	       printf("Property Notify\n");
	  }
     }

     XCloseDisplay(dpy);
     
     return 0;
}

void error(char *message) {
     perror(message);
     exit(1);
}
