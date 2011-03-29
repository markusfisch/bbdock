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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "Dock.hh"
#include "Render.hh"

using namespace std;
using namespace bbdock;

/**
 * Add an Icon record to a Dock
 *
 * @param dock - Dock object
 * @param record - an icon record
 * @param delimiter - field delimiter
 */
bool add( Dock &dock, char *record, const char delimiter )
{
	// cut off comments, don't use strtok because it will skip leading
	// delimiters
	{
		char *comment;

		if( (comment = strchr( record, '#' )) )
			*comment = 0;
	}

	enum
	{
		Image,
		Command,
		Title,
		Idle
	};

	char *fields[Idle+1];

	bzero( fields, sizeof( fields ) );
	fields[Image] = record;

	for( int n = Image; n <= Idle; )
	{
		char *stop;

		if( !(stop = strchr( fields[n], delimiter )) )
			break;

		*(stop++) = 0;
		fields[++n] = stop;
	}

	if( !fields[Image] || 
		!fields[Command] )
		return false;

	unsigned int idletime = Icon::Slow;

	if( fields[Idle] )
		if( !strcasecmp( fields[Idle], "fast" ) )
			idletime = Icon::Fast;
		else if( !strcasecmp( fields[Idle], "fastest" ) )
			idletime = Icon::Fastest;
		else if( !strcasecmp( fields[Idle], "slow" ) )
			idletime = Icon::Slow;
		else if( !strcasecmp( fields[Idle], "lame" ) )
			idletime = Icon::Lame;
		else 
		{
			unsigned int t = (unsigned int) 
				atoi( fields[Idle] );

			if( t > 0 )
				idletime = t;
		}

	return dock.add( new Icon( fields[Image], fields[Command], 
		(fields[Title] ? fields[Title] : ""), idletime ) );
}

/**
 * Load icons from file
 *
 * @param binary - name of the currently running binary
 * @param dock - Dock object
 */
void loadRC( char *binary, Dock &dock )
{
	string rcfilename = binary;
	string::size_type p;

	if( (p = rcfilename.find_last_of( '/' )) )
		rcfilename = rcfilename.substr( ++p );

	rcfilename = "/."+rcfilename+"rc";
	rcfilename = getenv( "HOME" )+rcfilename;

	ifstream fin( rcfilename.c_str(), ios::in );

	if( !fin )
		throw "Unable to open resource configuraton file for reading";

	for( string buf; getline( fin, buf ); )
		add( dock, (char *) buf.c_str(), ':' );
}

/**
 * Entry point
 */
int main( int argc, char **argv )
{
	Dock::Settings settings;
	char *binary;

	// process command line arguments  
	for( binary = *argv; --argc; )
		if( **(++argv) != '-' )
			break;
		else
			switch( *((*argv)+1) )
			{
				default:
					cerr << "Unknown argument \"" << *argv << "\" !" << endl 
						<< endl;
					// fall through
				case '?':
				case 'h':
					cout << "usage: " << binary << 
" [-hvdmprlcix] IMAGEFILE:COMMAND[:WINDOWTITLE;...[:IDLE]]...\n\
  -h                    print this help\n\
  -v                    print version\n\
  -d WIDTHxHEIGHT       outer dimensions of dock buttons\n\
  -m TYPE               define look of mark indicating a running instance,\n\
                        TYPE may be \"play\", \"dots\", \"corner\" or\n\
                        \"cross\" (default)\n\
  -p LEFT/TOP           left/top (or if negative right/bottom) padding of\n\
                        the mark in icon image\n\
  -r ACTION             determines what action is performed when a running\n\
                        icon gets right-clicked, possible ACTIONs are:\n\
                        \"nothing\" - do nothing\n\
                        \"iconfiy\" - iconify instance (default)\n\
                        \"lower\" - move instance to background\n\
                        \"close\" - shut down instance\n\
  -l ACTION             determines what action is performed when a icon\n\
                        is clicked again when the corresponding window is\n\
                        already activated, use the same ACTIONs like before,\n\
                        \"nothing\" is default\n\
  -c                    match WINDOWTITLE case-sensitive (recommended)\n\
  -i COMMAND:IMAGEFILE  remotely exchange icon of this command\n\
  -x COMMAND            remotely execute icon with this command\n\
\n\
IMAGEFILE   - should be path and filename of some PNG icon\n\
COMMAND     - a script or binary to execute\n\
WINDOWTITLE - is a semicolon-seperated list of case-insensitive window-titles\n\
              of corresponding application-windows. Those strings may contain\n\
              wildcard characters (* and/or ?) to exclusively  identify a\n\
              window. By providing this list you make the icon exclusive to\n\
              one instance of course. Clicking on already launched icons will\n\
              raise the corresponding window instead of invoking a new\n\
              instance.\n\
IDLE        - idle time after triggering one icon in miliseconds, instead\n\
              of using numbers you may also use the terms \"lame\" (~ 10 s),\n\
              \"slow\" (~ 5 s), \"fast\" (~ 500 ms) or \"fastest\" (~ 250 ms)"
						<< endl;
					return 0;
				case 'v':
					cout << "bbdock, version 0.2.9 <mf@markusfisch.de>" << endl;
					return 0;
				case 'd':
					{
						char *w;
						char *h;

						if( --argc &&
							(w = strtok( *(++argv), "x" )) &&
							(h = strtok( 0, "" )) )
						{
							settings.setSlotWidth( atoi( w ) );
							settings.setSlotHeight( atoi( h ) );
						}
						else
							cerr << "Missing or invalid dimensions !" << 
								endl;
					}
					break;
				case 'm':
					if( !--argc )
						cerr << "Missing mark type !" << endl;
					else
					{
						char *type = *(++argv);

						if( !(strcasecmp( type, "play" )) )
							settings.setMarkType( Render::PlayMark );
						else if( !(strcasecmp( type, "dots" )) )
						{
							settings.setMarkType( Render::DotsMark );
							settings.setMarkLeft( -1 );
							settings.setMarkTop( -1 );
						}
						else if( !(strcasecmp( type, "corner" )) )
						{
							settings.setMarkType( Render::CornerMark );
							settings.setMarkLeft( -1 );
							settings.setMarkTop( -1 );
						}
						else if( !(strcasecmp( type, "cross" )) )
						{
							settings.setMarkType( Render::CrossMark );
							settings.setMarkLeft( -1 );
							settings.setMarkTop( 0 );
						}
						else
							cerr << "Unknown type '" << type << "' !" << endl;
					}
					break;
				case 'p':
					{
						char *l;
						char *t;

						if( --argc &&
							(l = strtok( *(++argv), "/" )) &&
							(t = strtok( 0, "" )) )
						{
							settings.setMarkLeft( atoi( l ) );
							settings.setMarkTop( atoi( t ) );
						}
						else
							cerr << "Missing or invalid padding !" << 
								endl;
					}
					break;
				case 'r':
					if( !--argc )
						cerr << "Missing right-click action !" << endl;
					else
					{
						char *action = *(++argv);

						if( !strcasecmp( action, "nothing" ) )
							settings.setRightClickAction( 
								Dock::Settings::DoNothing );
						else if( !strcasecmp( action, "iconify" ) )
							settings.setRightClickAction( 
								Dock::Settings::IconifyApplication );
						else if( !strcasecmp( action, "lower" ) )
							settings.setRightClickAction( 
								Dock::Settings::LowerApplication );
						else if( !strcasecmp( action, "close" ) )
							settings.setRightClickAction( 
								Dock::Settings::CloseApplication );
						else
							cerr << "Unknown action '" << action << "' !" << 
								endl;
					}
					break;
				case 'l':
					if( !--argc )
						cerr << "Missing left-click action !" << endl;
					else
					{
						char *action = *(++argv);

						if( !strcasecmp( action, "nothing" ) )
							settings.setLeftClickAction( 
								Dock::Settings::DoNothing );
						else if( !strcasecmp( action, "iconify" ) )
							settings.setLeftClickAction( 
								Dock::Settings::IconifyApplication );
						else if( !strcasecmp( action, "lower" ) )
							settings.setLeftClickAction( 
								Dock::Settings::LowerApplication );
						else if( !strcasecmp( action, "close" ) )
							settings.setLeftClickAction( 
								Dock::Settings::CloseApplication );
						else
							cerr << "Unknown action '" << action << "' !" << 
								endl;
					}
					break;
				case 'c':
					settings.setCaseSensitive( true );
					break;
				case 'i':
					{
						char *icon;
						char *cmd;

						if( --argc &&
							(cmd = strtok( *(++argv), ":" )) &&
							(icon = strtok( 0, "" )) )
						{
							Dock::changeIcon( cmd, icon );
							return 0;
						}
						else
							cerr << "Missing or invalid parameter !" << 
								endl;
					}
					break;
				case 'x':
					{
						if( --argc )
						{
							Dock::executeIcon( *(++argv) );
							return 0;
						}
						else
							cerr << "Missing or invalid parameter !" << 
								endl;
					}
					break;
			}

	// detach from shell
	switch( fork() )
	{
		case -1:
			cerr << "Can not fork !" << endl;
			return -1;
		case 0:
			break;
		default:
			return 0;
	}

	// bring up the icons
	try
	{
		Dock dock( settings );

		if( !argc )
		{
			// try to start up by configuration file
			loadRC( binary, dock );
		}
		else
		{
			// try to start up by command line
			for( ; argc--; argv++ )
				if( !add( dock, *argv, ':' ) )
					throw "Invalid argument";
		}

		dock.run();
	}
	catch( const char *e )
	{
		cerr << e << endl;
	}
	catch( ... )
	{
		cerr << "Unknown error" << endl;
	}

	return 0;
}
