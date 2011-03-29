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
// use GNU extensions
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include <iostream>
#include <string>

#include <string.h>

#include "WindowManager.hh"

using namespace std;
using namespace bbdock;

/**
 * Check if window is valid
 *
 * @param w - window id
 */
const bool WindowManager::isValid( Window w ) const
{
	XErrorHandler handler;
	XWindowAttributes attr;
	bool valid = false;

	handler = XSetErrorHandler( WindowManager::throwError );
	if( XGetWindowAttributes( display, w, &attr ) )
		valid = true;
	XSetErrorHandler( handler );

	return valid;
}

/**
 * Activate a window and switch to its workspace
 *
 * @param w - id of window to activate
 */
void WindowManager::activateWindow( Window w ) const
{
	WindowManager::Property <unsigned long> p;

	// switch to workspace if neccessary
	if( p.getProperty( display, w, XA_CARDINAL, "_NET_WM_DESKTOP" ) ||
		p.getProperty( display, w, XA_CARDINAL, "_WIN_WORKSPACE" ) )
	{
		unsigned long workspace = *p.getData();

		if( (p.getProperty( display, root, XA_CARDINAL, 
			"_NET_CURRENT_DESKTOP" ) ||
			p.getProperty( display, root, XA_CARDINAL, "_WIN_WORKSPACE" )) &&
			workspace != *p.getData() )
			sendClientMessage( root, "_NET_CURRENT_DESKTOP", workspace );
	}

	sendClientMessage( w, "_NET_ACTIVE_WINDOW" );
	XMapRaised( display, w );
}

/**
 * Close a window
 *
 * @param w - id of window to close
 */
void WindowManager::closeWindow( Window w ) const
{
	sendClientMessage( w, "_NET_CLOSE_WINDOW" );
}

/**
 * Return currently active window
 */
Window WindowManager::getActiveWindow() const
{
	WindowManager::Property <Window> p;

	if( !p.getProperty( display, DefaultRootWindow( display ), XA_WINDOW, 
		"_NET_ACTIVE_WINDOW" ) )
		return 0;

	return *p.getData();
}

/**
 * Return first window whose title matches pattern
 *
 * @param pattern - a pattern with wildcards (*?)
 */
Window WindowManager::getWindowFromName( string pattern,
	bool casesensitive )
{
	Window w;

	if( (w = peekCache( pattern )) )
		return w;

	WindowManager::WindowList list = getClientList();

	if( !list.count() )
		return 0;

	for( Window w; (w = list.fetch()); )
		if( isValid( w ) &&
			matchesPattern( getWindowTitle( w ).c_str(), pattern.c_str(),
				casesensitive ) )
		{
			updateCache( pattern, w );
			return w;
		}

	return 0;	
}

/**
 * Return title of a window
 *
 * @param w - window in question
 */
string WindowManager::getWindowTitle( Window w ) const
{
	WindowManager::Property <char> p;
	
	if( !p.getProperty( display, w, XA_STRING, "WM_NAME" ) &&
		!p.getProperty( display, w, 
			XInternAtom( display, "UTF8_STRING", False ), 
			"_NET_WM_NAME" ) )
		return "";

	string s = p.getData();

	return s;
}

/**
 * Return list of all application windows
 */
WindowManager::WindowList WindowManager::getClientList() const
{
	WindowManager::Property <Window> *p = 
		new WindowManager::Property <Window>();

	if( !p->getProperty( display, root, XA_WINDOW, "_NET_CLIENT_LIST" ) )
		p->getProperty( display, root, XA_CARDINAL, "_WIN_CLIENT_LIST" );

	return WindowManager::WindowList( p );
}

/**
 * Update cache with new recognized pattern/window relation
 *
 * @param pattern - pattern matching window title
 * @param w - window id
 */
void WindowManager::updateCache( string pattern, Window w )
{
	vector<PatternWindowCache>::iterator it = cache.begin();
	vector<PatternWindowCache>::iterator end = cache.end();

	for( ; it != end; ++it )
		if( !pattern.compare( it->getPattern() ) )
		{
			if( it->getWindow() != w )
				it->setWindow( w );

			return;
		}

	cache.push_back( PatternWindowCache( pattern, w ) );	
}

/**
 * Return a valid window id if pattern is in cache
 *
 * @param pattern - pattern matching window title
 */
Window WindowManager::peekCache( string pattern )
{
	vector<PatternWindowCache>::iterator it = cache.begin();
	vector<PatternWindowCache>::iterator end = cache.end();

	for( ; it != end; ++it )
		if( !pattern.compare( it->getPattern() ) )
		{
			if( !isValid( it->getWindow() ) )
			{
				cache.erase( it, it );
				return 0;
			}

			return it->getWindow();
		}

	return 0;
}

/**
 * Send a ClientMessage to some window
 *
 * @param w - target window
 * @param msg - message to send
 * @param data0 - data (optional)
 * @param data1 - data (optional)
 * @param data2 - data (optional)
 * @param data3 - data (optional)
 * @param data4 - data (optional)
 */
void WindowManager::sendClientMessage( Window w, const char *msg, 
	unsigned long data0, unsigned long data1, unsigned long data2, 
	unsigned long data3, unsigned long data4 ) const
{
	XEvent event;
	long mask = SubstructureRedirectMask | SubstructureNotifyMask;

	event.xclient.type = ClientMessage;
	event.xclient.serial = 0;
	event.xclient.send_event = True;
	event.xclient.message_type = XInternAtom( display, msg, False );
	event.xclient.window = w;
	event.xclient.format = 32;
	event.xclient.data.l[0] = data0;
	event.xclient.data.l[1] = data1;
	event.xclient.data.l[2] = data2;
	event.xclient.data.l[3] = data3;
	event.xclient.data.l[4] = data4;

	XSendEvent( display, root, False, mask, &event );
}

/**
 * Intercept X errors
 */
int WindowManager::throwError( Display *, XErrorEvent * )
{
	return False;
}

/**
 * Returns if pattern matches string
 *
 * @param literal - string to match
 * @param pattern - pattern with optional wildcard characters (*?)
 * @param casesensitive - true if matching should be case-sensitive (optional)
 */
const bool WindowManager::matchesPattern( const char *literal, 
	const char *pattern, bool casesensitive ) const
{
	for( ; *pattern; )
	{
		switch( *pattern )
		{
			case '*':
				{
					const char *match = pattern;

					while( *match == '*' || *match == '?' )
						match++;

					if( !*match )
						return true;

					const char *wildcard;
					char save;
					size_t length;

					if( (length = strcspn( match, "*?" )) )
					{
						wildcard = match+length;
						save = *wildcard;
						*((char *) wildcard) = 0;
					}

					const char *last = NULL;
					const char *pos;

					if( casesensitive )
						for( pos = literal;
							(pos = strstr( pos, match ));
							pos++ )
							last = pos;
					else
						for( pos = literal;
							(pos = strcasestr( pos, match ));
							pos++ )
							last = pos;

					if( wildcard )
						*((char *) wildcard) = save;

					if( !last )
						return false;

					literal = last+length;

					if( !wildcard )
					{
						if( *literal )
							return false;

						return true;
					}

					pattern = wildcard;
				}
				break;
			case '?':
				if( !*(literal++) )
					return false;
				pattern++;
				break;
			default:
				if( !*literal )
					return false;

				if( casesensitive )
				{
					if( *pattern != *literal )
						return false;
				}
				else
				{
					register unsigned char p = *pattern;
					register unsigned char l = *literal;

					if( p > 64 && p < 91 )
						p += 32;
					if( l > 64 && l < 91 )
						l += 32;

					if( l != p )
						return false;
				}

				literal++;
				pattern++;
				break;
		}
	}

	if( *literal )
		return false;

	return true;
}
