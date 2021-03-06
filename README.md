bbdock
======

What is bbdock?
---------------

bbdock displays one or more PNG icons in the BlackBox/FluxBox slit from
which you can launch the corresponding applications. It is pretty similar
in function and appearance to bbbutton which you might want to try if you
are reading this on a display with a color-depth below 16 bits.

How to install
--------------

Usual autotools install:

	$ ./configure
	$ make
	$ su
	$ make install

For more options try ./configure --help

You need X11 libraries & headers and libpng. If anything goes wrong,
just drop me a line at mf@markusfisch.de.

Configuration
-------------

You may invoke bbdock either with your icons specified on command line
or simply create a short configuration file (.bbdockrc) in your home
directory.

The syntax for a command line argument or a line in the configuration
file is equal:

	IMAGEFILE:COMMAND[:WINDOWTITLE;[:IDLE]]

	IMAGEFILE   - should be path and filename of some PNG icon
	COMMAND     - a script or binary to execute
	WINDOWTITLE - is a semicolon-seperated list of case-insensitive
				  window titles of corresponding application windows.
				  Those strings may contain wildcard characters
				  (* and/or ?) to exclusively identify a window. By
				  providing this list you make the icon exclusive to
				  one instance of course. Clicking on already launched
				  icons will raise the corresponding window instead of
				  invoking a new instance.
	IDLE        - idle time after triggering one icon in miliseconds,
	              instead of using numbers you may also use the terms
				  "lame" (~ 10 s), "slow" (~ 5 s), "fast" (~ 500 ms) or
				  "fastest" (~ 250 ms)

For example, run it from command line this way:

	$ bbdock "~/.icons/firefox.png:firefox:*Firefox"

Sample out of a ~/.bbdockrc:

	~/.icons/terminal.png:xterm::1
	~/.icons/firefox.png:firefox:*Firefox

To start by configuration file, just run bbdock. Type "bbdock -h" for
a detailed view of options.

Requirements
------------

The images should be not much greater than 48x48 pixels and must be in
PNG format. Use images with transparency to get the most out of it.

You need libpng to compile this software. The raise-window function is
only available to window managers which implement the EWMH specification.

Bugs
----

Currently there are only two known bugs:

If you're using BlackBox > 0.70 you need to patch it in order to make it
work correctly. Just insert the following code after line 250 into
blackbox-0.70.0/src/Slit.cc and recompile BlackBox:

	if ((texture.texture() & bt::Texture::Gradient) && frame.pixmap)
	  XSetWindowBackgroundPixmap(display, frame.window, frame.pixmap);
	else if ((texture.texture() & bt::Texture::Solid))
	  XSetWindowBackground(display, frame.window,
		texture.color1().pixel(screen->screenNumber()));

FluxBox < 0.9 may fail to raise applications that are exclusive. Update
to FluxBox 0.9 to fix this problem.

Links
-----

* [bbdock](http://bbdock.nethence.com)
* [BlackBox](http://blackboxwm.sourceforge.net)
* [FluxBox](http://www.fluxbox.org)
* [BBButton](http://www.angelfire.com/theforce/button)
* [wmctrl](http://sweb.cz/tripie/utils/wmctrl)
