# carlywm

#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/wm.jpg)

About
-----

A simple dynamic tiling window manager written in C and Xlib that can also float windows (and can do both at the same time, actually). It's better to see how it works than to read about it, so watch the video:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=ZxCnU6D8TNo
" target="_blank"><img src="http://img.youtube.com/vi/ZxCnU6D8TNo/0.jpg"
alt="IMAGE ALT TEXT HERE" width="240" height="180" border="10" /></a>

I run it with a simple status bar (as seen in screenshots), [lemonbar](https://github.com/LemonBoy/bar), and [feh](https://github.com/derf/feh) for setting a background.

Modes
-----

*carlywm* supports a few different tiling/floating modes that I found useful.

The classic *Vertical Stack*:

<img src="other/masterstack.png" alt="Drawing" style="width: 50px;"/>

The not-so-classic-but-obligatory *Horizontal Stack*:

*Columns*:

*Full Screen*:

*Floating*:

*Mixed floating/tiling*:

Configuration
-------------

There's a very basic file, **config.h**, where you can configure things like whether there are gaps or not, or the increment in which windows resize. You'll have to recompile after making changes, of course.

Screenshots
-----------

#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/s11.png)
#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/s8.png)
#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/s4.png)


Extended Window Manager Hints
-----------------------------

This is *kinda* there, at this point. Enough to feed the number of workspaces and the current workspace to a status bar or whatever you want. It also should recognize transient windows, such as an application launcher like [rofi](https://github.com/DaveDavenport/rofi).

Status/To-do
------
+
+
+