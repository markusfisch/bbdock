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
#ifndef bbdock_WindowManager_hh
#define bbdock_WindowManager_hh

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <string>
#include <vector>

namespace bbdock
{
	/**
	 * WindowManager knows how to talk to the window manager
	 *
	 * @author mf@markusfisch.de
	 * @version 0.0.3
	 */
	class WindowManager
	{
		public:
			WindowManager( Display *d ) :
				display( d ),
				root( DefaultRootWindow( display ) ) {}
			virtual ~WindowManager() {}
			const bool isValid( Window ) const;
			void activateWindow( Window ) const; 
			void closeWindow( Window ) const; 
			Window getActiveWindow() const;
			Window getWindowFromName( std::string, bool = false );

		private:
			/**
			 * Property object
			 */
			template <class T> class Property
			{
				public:
					inline T *getData() const { return data; }
					inline unsigned long getItems() const { return items; }

					Property() : data( 0 ), items( 0 ) {}
					bool getProperty( Display *display, Window w, Atom type, 
						const char *name )
					{
						free();

						Atom returnedtype;
						int format;
						unsigned long items;
						unsigned long bytesafter;
						unsigned char *data;

						if( XGetWindowProperty( display, w, 
							XInternAtom( display, name, False ), 
							0, 1024, False, type, &returnedtype, 
							&format, &items, &bytesafter, 
							&data ) != Success )
							return false;

						if( returnedtype != type )
						{
							XFree( data );
							return false;
						}

						this->data = reinterpret_cast<T *>(data);
						this->items = items;

						return true;
					}
					virtual ~Property()
					{
						free();
					}

				private:
					T *data;
					unsigned long items;

					const void free()
					{
						if( !data )
							return;

						XFree( reinterpret_cast<unsigned char *>(data) );

						data = 0;
						items = 0;
					}
			};

			/**
			 * Iterateable window list
			 */
			class WindowList
			{
				public:
					WindowList( Property <Window> *p ) :
						property( p ),
						item( p->getData() ),
						items( p->getItems() )
					{
					}
					virtual ~WindowList()
					{
						delete property;
					}
					inline Window fetch()
					{
						if( !items-- )
							return 0;

						return item[items];
					}
					inline unsigned long count() const
					{
						return property->getItems();
					}

				private:
					Property <Window> *property;
					Window *item;
					unsigned long items;
			};

			/**
			 * This object caches a pattern/window relation
			 */
			class PatternWindowCache
			{
				public:
					inline const Window &getWindow() const { return window; }
					inline const void setWindow( Window w ) { window = w; }
					inline const std::string &getPattern() const { 
						return pattern; }

					PatternWindowCache( std::string p, Window w ) :
						pattern( p ),
						window( w ) {}
					virtual ~PatternWindowCache() {}

				private:
					std::string pattern;
					Window window;
			};

			Display *display;
			Window root;
			std::vector<PatternWindowCache> cache;

			std::string getWindowTitle( Window ) const;
			WindowManager::WindowList getClientList() const;
			void updateCache( std::string, Window );
			Window peekCache( std::string );
			void sendClientMessage( Window w, const char *, 
				unsigned long = 0, unsigned long = 0, unsigned long = 0, 
				unsigned long = 0, unsigned long = 0 ) const;
			static int throwError( Display *, XErrorEvent * );
			const bool matchesPattern( const char *, const char *,
				bool = false ) const;
	};
}

#endif
