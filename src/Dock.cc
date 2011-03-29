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
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include "Dock.hh"
#include "Slot.hh"
#include "WindowManager.hh"

using namespace std;
using namespace bbdock;

const char *Dock::ChangeIconMessage = "_BBDOCK_CHANGE_ICON_";
const char *Dock::ExecuteIconMessage = "_BBDOCK_EXECUTE_ICON_";

/**
 * Initialize dock
 *
 * @param s - settings
 */
Dock::Dock( Dock::Settings &s ) :
	settings( s ),
	first( 0 )
{
	if(	!(display = XOpenDisplay( 0 )) )
		throw "Unable to open display";

	screen = DefaultScreen( display );
	screenwidth = DisplayWidth( display, screen );
	screenheight = DisplayHeight( display, screen );

	gc = XDefaultGC( display, screen );

	// determine bits per pixel of an screen-compatible XImage, does not 
	// need to be the same as the color-depth of the screen
	{
		XPixmapFormatValues *pf;
		int formats;

		bitsperpixel = DefaultDepth( display, screen );
		pf = XListPixmapFormats( display, &formats );

		if( formats )
		{
			int format;

			for( format = formats;
			     format--; )
				if( pf[format].depth == bitsperpixel )
				{
					bitsperpixel = pf[format].bits_per_pixel;
					break; 
				}

			XFree( (char *) pf );
		}
	}
}

/**
 * Free resources  
 */
Dock::~Dock()
{
	XCloseDisplay( display );
}

/**
 * Add an icon to the dock
 *
 * @param icon - Icon object
 */
bool Dock::add( Icon *icon )
{
	if( !icon )
		return false;

	first = new Slot( *this, *icon, first );

	return true;
}

/**
 * Run the dock
 */
void Dock::run()
{
	if( !first )
		return;

	for( int xfd = ConnectionNumber( display ); ; )
	{
		if( !XPending( display ) )
		{
			Slot *slot;

			for( slot = first;
				slot;
				slot = slot->getNext() )
				slot->tick();

			fd_set rfds;
			struct timeval tv;

			FD_ZERO( &rfds );
			FD_SET( xfd, &rfds );

			tv.tv_sec = 0;
			tv.tv_usec = Suspend;

			select( xfd+1, &rfds, 0, 0, &tv );
			continue;
		}

		XEvent event;

		bzero( &event, sizeof( event ) );
		XNextEvent( display, &event );

		Slot *slot;

		for( slot = first;
			slot;
			slot = slot->getNext() )
			if( event.xany.window == slot->getWindow() )
				break;

		if( !slot )
		{
			if( event.type == ClientMessage )
				client( &event );

			continue;
		}

		switch( event.type )
		{
			case Expose:
				if( !event.xexpose.count )
					slot->draw();
				break;
			case VisibilityNotify:
				slot->setVisibility( event.xvisibility.state );
				break;
			case ConfigureNotify:
				slot->invalidate();
				slot->draw( true );
				break;
			case SelectionNotify:
				if( event.xselection.property != None )
				{
					Atom type;
					int format;
					unsigned long len;
					unsigned long after;
					unsigned long bytes;
					unsigned char *data;

					if( XGetWindowProperty( display, event.xany.window, 
						event.xselection.property, 0, 0, 0, AnyPropertyType, 
						&type, &format, &len, &bytes, &data ) != Success ||
						!bytes )
						break;

					if( XGetWindowProperty( display, event.xany.window, 
						event.xselection.property, 0, bytes, 0, 
						AnyPropertyType, &type, &format, &len, &after, 
						&data ) != Success )
						break;

					slot->exec( (char *) data );
					XFree( data );
				}
				break;
			case ButtonRelease:
				switch( event.xbutton.button )
				{
					case 1:
						{
							if( getMarkType() == Render::CrossMark )
							{
								int l = getMarkLeft();
								int t = getMarkTop();
								int x = event.xbutton.x;
								int y = event.xbutton.y;

								x -= slot->getLeft();
								y -= slot->getTop();

								if( l < 0 )
									l = slot->getWidth()-6+l;
								if( t < 0 )
									t = slot->getHeight()-6+t;

								if( x > l && x < l+8 &&
									y > t && y < t+8 )
								{
									slot->closeApplication();
									break;
								}
							}

							if( settings.getLeftClickAction() != 
								Settings::DoNothing &&
								slot->hasFocus() )
							{
								switch( settings.getLeftClickAction() )
								{
									case Settings::IconifyApplication:
										slot->iconifyApplication();
										continue;
									case Settings::LowerApplication:
										slot->lowerApplication();
										continue;
									case Settings::CloseApplication:
										slot->closeApplication();
										continue;
								}
							}

							slot->exec();
						}
						break;
					case 2:
						{
							Window owner;

							if( (owner = XGetSelectionOwner( display, 
								XA_PRIMARY )) == None )
								break;

							Atom property = XInternAtom( display, 
								"BBDOCK_SELECTION", False );

							XConvertSelection( display, XA_PRIMARY, XA_STRING,
								property, event.xany.window, CurrentTime );
						}
						break;
					case 3:
						switch( settings.getRightClickAction() )
						{
							case Settings::IconifyApplication:
								slot->iconifyApplication();
								break;
							case Settings::LowerApplication:
								slot->lowerApplication();
								break;
							case Settings::CloseApplication:
								slot->closeApplication();
								break;
						}
						break;
				}
				break;
		}
	}
}

/**
 * Send a message to a running instance of bbdock
 *
 * @param message - the message to send
 */
void Dock::send( const char *message, const char *data, int len )
{
	if( !message || !*message ||
		!data || !*data )
		return;

	Display *display;
	int screen;
	Window root;

	if(	!(display = XOpenDisplay( 0 )) )
		throw "Unable to open display";

	screen = DefaultScreen( display );

	if( !(root = RootWindow( display, screen )) )
		throw "Can not access root window";

	Window dummy;
	Window *childs;
	unsigned int n;

 	XQueryTree( display, root, &dummy, &dummy, &childs, &n );

	Atom msg = XInternAtom( display, message, false );

	for( ; n--; )
	{
		XClassHint xch;

		if( !XGetClassHint( display, childs[n], &xch ) )
			continue;

		if( !strcmp( xch.res_name, "bbdock" ) &&
			!strcmp( xch.res_class, "bbdock" ) )
		{
			XChangeProperty( display, childs[n], msg, msg, 8, 
				PropModeReplace, (unsigned char *) data, len );
		
			XEvent event;

			event.xclient.type = ClientMessage;
			event.xclient.serial = 0;
			event.xclient.send_event = True;
			event.xclient.message_type = msg;
			event.xclient.window = childs[n];
			event.xclient.format = 32;
			event.xclient.data.l[0] = 0;
			event.xclient.data.l[1] = 0;
			event.xclient.data.l[2] = 0;
			event.xclient.data.l[3] = 0;
			event.xclient.data.l[4] = 0;

			XSendEvent( display, childs[n], True, NoEventMask, &event );

			// exit loop but not without freeing resources below
			n = 0;
		}

		XFree( xch.res_name );
		XFree( xch.res_class );
	}

	if( childs )
		XFree( childs );

	XCloseDisplay( display );
}

/**
 * Change icon of a command (remotely)
 *
 * @param cmd - command of icon to change
 * @param icon - new icon
 */
void Dock::changeIcon( const char *cmd, const char *icon )
{
	if( !cmd || !*cmd ||
		!icon || !*icon )
		return;

	string data = cmd;

	data += ":";
	data += icon;

	Dock::send( ChangeIconMessage, data.c_str(), data.length() );
}

/**
 * Execute icon whose command is cmd (remotely)
 *
 * @param cmd - command of icon to change
 */
void Dock::executeIcon( const char *cmd )
{
	if( !cmd || !*cmd )
		return;

	Dock::send( ExecuteIconMessage, cmd, strlen( cmd ) );
}

/**
 * Process a client message
 *
 * @param e - event
 */
void Dock::client( XEvent *e )
{
	char *msg;

	if( !(msg = XGetAtomName( display, 
		e->xclient.message_type )) )
		return;

	Atom returnedtype;
	int format;
	unsigned long n;
	unsigned long bytesafter;
	char *prop;

	if( XGetWindowProperty( display, e->xany.window, e->xclient.message_type, 
		0, 1024, False, e->xclient.message_type, &returnedtype, &format, &n, 
		&bytesafter, (unsigned char **) &prop ) != Success )
		return;

	if( !strcmp( msg, ChangeIconMessage ) )
	{
		char *cmd;
		char *icon;

		if( (cmd = strtok( prop, ":" )) &&
			(icon = strtok( 0, "\n" )) )
		{
			Slot *slot;

			for( slot = first;
				slot;
				slot = slot->getNext() )
				if( !strcasecmp( slot->getCommand(), cmd ) && 
					strcasecmp( slot->getIcon(), icon ) )
				{
					slot->setIcon( icon );
					break;
				}
		}
	}
	else if( !strcmp( msg, ExecuteIconMessage ) )
	{
		Slot *slot;

		for( slot = first;
			slot;
			slot = slot->getNext() )
			if( !strcasecmp( slot->getCommand(), prop ) )
			{
				slot->exec();
				break;
			}
	}

	XFree( prop );
	XDeleteProperty( display, e->xany.window, e->xclient.message_type );

	XFree( msg );
}
