/*
 * Copyright (c) 2006 Markus Fisch
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Markus Fisch nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string>

#include "Slot.hh"

extern char **environ;

using namespace std;
using namespace bbdock;

const double Slot::fadestep = 1.05;

/**
 * Initialize object
 *
 * @param d - Dock object
 * @param i - Icon object
 * @param n - next Slot in chain
 */
Slot::Slot( Dock &d, Icon &i, Slot *n ) :
	dock( &d ),
	icon( &i ),
	next( n ),
	render( 0 ),
	normalicon( 0 ),
	surface( 0 ),
	background( 0 ),
	invalid( false ),
	visibility( VisibilityUnobscured ),
	wm( d.getDisplay() ),
	pid( 0 ),
	fading( Complete )
{
	loadIcon();

	Window root;
	Window dummy;

	root = RootWindow( dock->getDisplay(), dock->getScreen() );

	// create dummy window and dock slot
	{
		dummy = XCreateSimpleWindow( dock->getDisplay(), root,
			0, 0, 1, 1, 1, 0, 0 );

		XSizeHints sh;

	    sh.x = 0;
		sh.y = 0;
		sh.width = dock->getSlotWidth();
		sh.height = dock->getSlotHeight();

	    XSetWindowAttributes xswat;
		unsigned long vmask;

		xswat.background_pixmap = ParentRelative;
		vmask = CWBackPixmap;

		window = XCreateWindow( dock->getDisplay(), root,
			sh.x, sh.y,
			sh.width, sh.height,
			0,
			CopyFromParent,
			InputOutput,
			CopyFromParent,
			vmask, &xswat );

		sh.flags = USSize | USPosition;
		XSetWMNormalHints( dock->getDisplay(), dummy, &sh );

		{
			XWMHints xwmh;

			xwmh.initial_state = WithdrawnState;
			xwmh.icon_window = window;
			xwmh.icon_x = sh.x;
			xwmh.icon_y = sh.y;
			xwmh.window_group = dummy;
			xwmh.flags = StateHint | IconWindowHint |
				IconPositionHint | WindowGroupHint;

			XSetWMHints( dock->getDisplay(), dummy, &xwmh );
		}
	}

	// set name
	{
		XClassHint xch;

		xch.res_name = (char *) "bbdock";
		xch.res_class = xch.res_name;

		XSetClassHint( dock->getDisplay(), dummy, &xch );
	}

	XSelectInput( dock->getDisplay(), window,
		ButtonReleaseMask | ButtonPressMask |
		ExposureMask | VisibilityChangeMask |
		StructureNotifyMask | SubstructureNotifyMask |
		PropertyChangeMask );

	XMapWindow( dock->getDisplay(), dummy );
}

/**
 * Clean up
 */
Slot::~Slot()
{
	destorySurface();
	destroyIcon();
}

/**
 * Display icon
 *
 * @param refresh - refresh background
 */
void Slot::draw( bool refresh )
{
	if( refresh || invalid )
	{
		// restore background
		if( invalid )
			getBackground();
		else
			memcpy( image, background, render->getSize() );

		// render active mark
		if( pid && icon->isExclusive() )
			render->activeMark( (unsigned char *) image,
				dock->getMarkType(), dock->getMarkLeft(), dock->getMarkTop() );

		// render icon
		if( fading < Complete )
			render->ghosted( (unsigned char *) image,
				(unsigned char *) normalicon, fading );
		else
			render->opaque( (unsigned char *) image,
				(unsigned char *) normalicon );
	}

	XPutImage( dock->getDisplay(), window, dock->getGC(), surface, 0, 0,
		left, top, width, height );
}

/**
 * Execute command
 *
 * @param arg - command line arguments (optional)
 */
void Slot::exec( char *arg )
{
	if( pid && icon->isExclusive() )
	{
		if( !icon->getTitle().empty() )
		{
			vector<string> t = icon->getTitle();
			vector<string>::iterator it = t.begin();
			vector<string>::iterator end = t.end();

			for( ; it != end; ++it )
			{
				Window w;

				if( !(w = wm.getWindowFromName( *it,
					dock->getCaseSensitive() )) )
					continue;

				wm.activateWindow( w );
			}
		}

		return;
	}

	if( fading < Complete ||
		icon->getCommand().empty() )
		return;

	switch( (pid = fork()) )
	{
		case -1:
			throw "Fork failed !";
			return;
		case 0:
			{
				string cmd = icon->getCommand();

				if( arg )
				{
					cmd += " ";
					cmd += arg;
				}

				char *argv[4];

				argv[0] = (char *) "sh";
				argv[1] = (char *) "-c";
				argv[2] = (char *) cmd.c_str();
				argv[3] = 0;

				setsid();
				execve( "/bin/sh", argv, environ );

				throw "Exec failed !";
			}
			return;
	}

	// calculate offset for fade-in, this is here and done on every launch
	// because 1) fading is a visual effect of Slot and there may be more
	// effects in the future and 2) the icon object may be changed by some
	// user interaction
	{
		double factor = fadestep;

		for( int idle = (int) (((double) 1000000/Dock::Suspend)*
			((double) icon->getIdleTime()/1000));
			idle--; factor *= fadestep );

		fading = (int) ((double) 256/factor);
	}

	draw( true );
}

/**
 * Try to iconify the corresponding instance
 */
void Slot::iconifyApplication()
{
	if( !pid ||
		!icon->isExclusive() ||
		icon->getTitle().empty() )
		return;

	vector<string> t = icon->getTitle();
	vector<string>::iterator it = t.begin();
	vector<string>::iterator end = t.end();

	for( ; it != end; ++it )
	{
		Window w;

		if( !(w = wm.getWindowFromName( *it,
			dock->getCaseSensitive() )) )
			continue;

		XIconifyWindow( dock->getDisplay(), w, dock->getScreen() );
	}
}

/**
 * Move corresponding instance to background
 */
void Slot::lowerApplication()
{
	if( !pid ||
		!icon->isExclusive() ||
		icon->getTitle().empty() )
		return;

	vector<string> t = icon->getTitle();
	vector<string>::iterator it = t.begin();
	vector<string>::iterator end = t.end();

	for( ; it != end; ++it )
	{
		Window w;

		if( !(w = wm.getWindowFromName( *it,
			dock->getCaseSensitive() )) )
			continue;

		XLowerWindow( dock->getDisplay(), w );
	}
}

/**
 * Request running instance to shut down
 */
void Slot::closeApplication()
{
	if( !pid ||
		!icon->isExclusive() )
		return;

	vector<string> t = icon->getTitle();
	int closed = 0;

	if( !t.empty() )
	{
		vector<string>::iterator it = t.begin();
		vector<string>::iterator end = t.end();

		for( ; it != end; ++it )
		{
			Window w;

			if( !(w = wm.getWindowFromName( *it,
				dock->getCaseSensitive() )) )
				continue;

			wm.closeWindow( w );
			closed++;
		}
	}

	if( t.empty() || !closed )
		kill( pid, SIGTERM );

	// don't touch pid since tick() will need to know that the
	// application has closed
}

/**
 * Process timer event
 */
void Slot::tick()
{
	if( !pid )
		return;

	int status;

	if( waitpid( pid, &status, WNOHANG | WUNTRACED ) == pid )
	{
		pid = 0;
		fading = Complete;

		draw( true );
		return;
	}

	if( fading < Complete )
	{
		fading = (int) ((double) fading*fadestep);

		if( fading > Complete )
			fading = Complete;

		draw( true );
	}
}

/**
 * (Re-)Set icon image
 *
 * @param filename - image file to load
 */
void Slot::setIcon( const char *filename )
{
	icon->setImage( filename );
	loadIcon();
	invalidate();
	draw( true );
}

/**
 * Returns true if corresponding application is active
 */
const bool Slot::hasFocus()
{
	Window w;

	if( !(w = wm.getActiveWindow()) )
		return false;

	vector<string> t = icon->getTitle();
	vector<string>::iterator it = t.begin();
	vector<string>::iterator end = t.end();

	for( ; it != end; ++it )
		if( wm.getWindowFromName( *it,
			dock->getCaseSensitive() ) == w )
			return true;

	return false;
}

/**
 * Load PNG icon
 */
void Slot::loadIcon()
{
	png_bytep *rowPointers;

	destroyIcon();
	width = height = 0;

	png_structp png = 0;
	png_infop info = 0;
	FILE *fp = 0;

	if( !(png = png_create_read_struct( PNG_LIBPNG_VER_STRING,
		0, 0, 0 )) )
		throw "PNG library error !";

	try
	{
		if( !(info = png_create_info_struct( png )) ||
			setjmp( png_jmpbuf( png ) ) ||
			!(fp = fopen( icon->getImage().c_str(), "rb" )) )
			throw 0;

		png_init_io( png, fp );
		png_set_sig_bytes( png, 0 );
		png_read_png( png, info,
			PNG_TRANSFORM_IDENTITY |
			PNG_TRANSFORM_EXPAND |
			PNG_TRANSFORM_BGR,
			0 );

		width = png_get_image_width( png, info );
		height = png_get_image_height( png, info );

		if( !png_get_valid( png, info, PNG_INFO_IDAT ) ||
			!(normalicon = new int[width*height]) )
			throw 0;

		rowPointers = png_get_rows( png, info );

		for( int y = 0, *src, *dest = normalicon;
			y < height && (src = (int *) rowPointers[y]);
			y++, dest += width )
			memcpy( dest, src, width<<2 );

		// convert grayscale image to rgb
		if( png_get_color_type(png, info) == PNG_COLOR_TYPE_GRAY_ALPHA )
			for( int y = height, *line = normalicon;
				--y; line += width )
			{
				unsigned char *dest = (unsigned char *) line+(width<<2);
				unsigned char *src = (unsigned char *) line+(width<<1);

				for( int x = width; --x; )
				{
					*(--dest) = *(--src);
					*(--dest) = *(--src);
					*(--dest) = *src;
					*(--dest) = *src;
				}
			}
	}
	catch( int )
	{
		// when failed do nothing, first the file needs to be closed
	}

	// free possibly allocated resources
	if( fp )
		fclose( fp );

	png_destroy_read_struct( &png, &info, (png_infopp) 0 );

	if( !normalicon || !width || !height )
		throw "Invalid image file !";

	if( (left = (dock->getSlotWidth()-width)>>1) < 0 ||
		(top = (dock->getSlotHeight()-height)>>1) < 0 )
		throw "Image is too big !";

	// (re-)create surface for this image size
	createSurface();
}

/**
 * Free image resource
 */
void Slot::destroyIcon()
{
	if( !normalicon )
		return;

	delete normalicon;
	normalicon = 0;
}

/**
 * (Re-)Create the surface
 */
void Slot::createSurface()
{
	destorySurface();

	if( !(render = Render::getInstance( dock->getBitsPerPixel(),
			width, height )) ||
		!(background = (char *) calloc( render->getSize(), sizeof( char ) )) ||
		!(image = (char *) calloc( render->getSize(), sizeof( char ) )) ||
		!(surface = XCreateImage( dock->getDisplay(),
			DefaultVisual( dock->getDisplay(), dock->getScreen() ),
			DefaultDepth( dock->getDisplay(), dock->getScreen() ),
			ZPixmap, 0, image,
			width, height,
			32, 0 )) )
		throw "Could not create XImage !";
}

/**
 * Drop surface
 */
void Slot::destorySurface()
{
	if( !surface )
		return;

	XDestroyImage( surface );
	surface = 0;
	// bitmap memory gets free'd automatically

	if( background )
	{
		free( background );
		background = 0;
	}

	if( render )
	{
		delete render;
		render = 0;
	}
}

/**
 * Get background image from parent window
 */
void Slot::getBackground()
{
	// ensure background is marked as invalid
	invalid = true;

	Window slit;

	// get parent window
	{
		Window root;
		Window *childs;
		unsigned int n;

 		XQueryTree( dock->getDisplay(), window, &root, &slit, &childs, &n );

		if( n && childs )
			XFree( childs );
	}

	if( !slit )
		return;

	// check if window is visible on screen since XGetSubImage
	// fails otherwise
	{
		XWindowAttributes wa;

		XGetWindowAttributes( dock->getDisplay(), slit, &wa );

		if( wa.map_state != IsViewable ||
			wa.x < 0 ||
			wa.y < 0 ||
			wa.x+wa.width > dock->getScreenWidth() ||
			wa.y+wa.height > dock->getScreenHeight() )
			return;
	}

	XClearWindow( dock->getDisplay(), window );
	XGetSubImage( dock->getDisplay(), window, left, top, width, height,
		0xffffffff, ZPixmap, surface, 0, 0 );

	if( visibility == VisibilityUnobscured )
	{
		memcpy( background, image, render->getSize() );

		invalid = false;
	}
}
