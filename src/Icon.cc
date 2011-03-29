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
#include <string>
#include <vector>

#include <stdlib.h>

#include "Icon.hh"

using namespace std;
using namespace bbdock;

/**
 * Initialize icon
 *
 * @param i - filename of image file
 * @param c - command to execute
 * @param w - window title if application should run exclusively (optional)
 * @param ms - idle time in miliseconds (optional)
 */
Icon::Icon( string i, string c, string t, unsigned int ms ) :
	image( expand( i ) ),
	command( expand( c ) ),
	idletime( ms )
{
	setTitle( t );
}

/**
 * Shell expansion
 *
 * @param path - path to expand
 */
string Icon::expand( string path ) const
{
	if( path[0] == '~' )
		path = getenv( "HOME" )+path.substr( 1 );

	return path;
}

/**
 * Tokenize string into vector
 *
 * @param str - string to split
 * @param tokens - vector that receives the tokens
 * @param delimiter - delimiter characters
 */
void Icon::tokenize( const string str, vector<string> &tokens,
	const string& delimiters ) const
{
    string::size_type last = str.find_first_not_of( delimiters, 0 );
    string::size_type next = str.find_first_of( delimiters, last );

    while( next != string::npos || last != string::npos )
    {
		tokens.push_back( str.substr( last, next-last ) );

		last = str.find_first_not_of( delimiters, next );
		next = str.find_first_of( delimiters, last );
    }
}
