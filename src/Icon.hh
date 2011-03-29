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
#ifndef bbdock_Icon_hh
#define bbdock_Icon_hh

#include <string>
#include <vector>

namespace bbdock
{
	/**
	 * A icon/command pair
	 *
	 * @author mf@markusfisch.de
	 * @version 0.1.4
	 */
	class Icon
	{
		public:
			inline const std::string &getCommand() const { return command; }
			inline const std::string &getImage() const { return image; }
			inline const std::vector<std::string> &getTitle() const { 
				return title; }
			inline const bool &isExclusive() const { return exclusive; }
			inline const unsigned int &getIdleTime() const { 
				return idletime; }
			inline const void setCommand( const std::string s ) { 
				command = expand( s ); }
			inline const void setImage( const std::string s ) { 
				image = expand( s ); }
			inline const void setTitle( const std::string s )
			{ 
				tokenize( s, title, ";" );

				if( title.empty() )
					exclusive = false; 
				else
					exclusive = true;
			}
			inline const void isExclusive( bool e ) { exclusive = e; }
			inline const void setIdleTime( unsigned int t ) { 
				idletime = t; }

			enum
			{
				Fastest = 250,
				Fast = 500,
				Slow = 5000,
				Lame = 10000
			};

			Icon( const std::string, const std::string, 
				const std::string, unsigned int = Slow );
			virtual ~Icon() {}

		private:
			std::string command;
			std::string image;
			std::vector<std::string> title;
			bool exclusive;
			unsigned int idletime;

			std::string expand( std::string ) const;
			void tokenize( const std::string str, 
				std::vector<std::string> &, 
				const std::string & = " " ) const;
	};
}

#endif
