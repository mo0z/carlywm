#ifndef CONFIG_H
#define CONFIG_H

/* Size of the "Main" window in tiled layouts (percentage) */
#define MAIN_WIN_SIZE         0.50

/* Size of window borders */
#define BORDER                2

/* Whether or not there are gaps between windows */
#define GAPS                  1

/* Size of gaps */
#define GAP_SIZE              12

/* How much to adjust gaps by per keypress */
#define GAPS_INC              1

/* Whether there is a panel */
#define PANEL                 1

/* Height of panel */
#define PANEL_HEIGHT          16

/* Increment to resize by when resizing in tiling mode */
#define RESIZE_INC            20

/* The number of workspaces */
#define NUM_WORKSPACES        5

/* Focus/Unfocus colors */
#define FOCUSED               "#989898"
#define UNFOCUSED             "#232c33"


#define RET                   65293
#define TAB                   65289
#define SPC                   32
#define a_KEY                 97
#define d_KEY                 100
#define e_KEY                 101
#define f_KEY                 102
#define m_KEY                 109
#define q_KEY                 113
#define h_KEY                 104
#define l_KEY                 108
#define J_KEY                 74
#define K_KEY                 75
#define j_KEY                 106
#define k_KEY                 107
#define R_KEY                 82
#define r_KEY                 114
#define c_KEY                 99
#define RIGHT                 65363
#define LEFT                  65361
#define UP                    65362
#define DOWN                  65364
#define MINUS                 45
#define EQUAL                 61
#define GREATER               62
#define LESS                  60
#define BRACKET_LEFT          91
#define BRACKET_RIGHT         93
#define SHIFT_L               65505
#define KEY_1                 49
#define KEY_2                 50
#define KEY_3                 51
#define KEY_4                 52
#define KEY_5                 53

#define ALT                   Mod1Mask
#define SHIFT                 ShiftMask
#define CTRL                  ControlMask
#define SUPER                 Mod4Mask

/* Preferred applications */

#define TERM                 "urxvt&"
#define BROWSER              "firefox&"
#define FILEMAN              "thunar&"
#define LAUNCHER             "rofi -show run"
#define EDITOR               "emacs&"

#endif
