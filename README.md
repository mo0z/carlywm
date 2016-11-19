# carlywm

#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/wm.jpg)

About
-----

A simple dynamic tiling window manager written in C and Xlib that can also float windows (and can do both at the same time, actually). Like most other tiling WMs, it's keyboard-driven. It's better to see how it works than to read about it, so watch the demos:

<!-- <a href="http://www.youtube.com/watch?feature=player_embedded&v=ZxCnU6D8TNo
" target="_blank"><img src="http://img.youtube.com/vi/ZxCnU6D8TNo/0.jpg"
alt="IMAGE ALT TEXT HERE" width="240" height="180" border="10" /></a> -->

[![...](https://j.gifs.com/r0XEnp.gif)

[Full video link](http://www.youtube.com/watch?feature=player_embedded&v=ZxCnU6D8TNo).

<!-- [![...](http://share.gifyoutube.com/KzB6Gb.gif)](https://www.youtube.com/watch?v=ek1j272iAmc) -->

I run it with a simple status bar (as seen in screenshots), [lemonbar](https://github.com/LemonBoy/bar), and [feh](https://github.com/derf/feh) for setting a background. There's an example *.xinitrc* in the *scripts* folder.

Layouts/Modes
-----

*carlywm* supports a few different tiling/floating modes that I found useful.

The classic *Vertical Stack*:

<img src="other/masterstack.png" alt="Graphic of the vertical stack layout" width="125">

The not-so-classic-but-obligatory *Horizontal Stack*:

<img src="other/hstack.png" alt="Graphic of the horizontal stack layout" width="125">

*Columns*:

<img src="other/cols.png" alt="Graphic of the columns layout" width="125">

*Full Screen*:

<img src="other/full.png" alt="Graphic of the fullscreen layout" width="125">

*Floating*:

<img src="other/floatingdrop.png" alt="Graphic of the floating mode" width="125">

*Mixed floating/tiling*:

<img src="other/mixed.png" alt="Graphic of the mixed floating/tiling mode thing" width="125">

If you're tiling and you drag a window around, it'll float and the existing windows will tile behind it, reconciling the hole left behind.

Configuration
-------------

There's a very basic file, **config.h**, where you can configure things like whether there are gaps or not, or the increment in which windows resize. You'll have to recompile after making changes, of course.

Screenshots
-----------

#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/s11.png)
#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/s8.png)
#![...](https://github.com/patjm1992/carlywm/blob/master/screenshots/s4.png)

Installation
------------

*coming soon*

Extended Window Manager Hints
-----------------------------

This is *kinda* there, at this point. Enough to feed the number of workspaces and the current workspace to a status bar or whatever you want. It also should recognize transient windows, such as an application launcher like [rofi](https://github.com/DaveDavenport/rofi).

Status/To-do
------

This project is put on pause for the time being while I pursue other things.

+ bugfixes
+ more EWMH support
+ code cleanup (*bigtime*)
+ vertical resizing
+ keybinding configuration