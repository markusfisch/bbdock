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
#ifndef bbdock_Dock_hh
#define bbdock_Dock_hh

#include <unistd.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Icon.hh"
#include "Render.hh"

namespace bbdock
{
	// forward declaration
	class Slot;

	/**
	 * Dock manages a number of Slot object
	 *
	 * @author mf@markusfisch.de
	 * @version 0.1.6
	 */
	class Dock
	{
		public:
			inline const int &getBitsPerPixel() const { return bitsperpixel; }
			inline Display *getDisplay() const { return display; }
			inline const int &getScreen() const { return screen; }
			inline const GC &getGC() const { return gc; }
			inline const int &getScreenWidth() const { return screenwidth; }
			inline const int &getScreenHeight() const { return screenheight; }
			inline const int &getSlotWidth() const { 
				return settings.getSlotWidth(); }
			inline const int &getSlotHeight() const { 
				return settings.getSlotHeight(); }
			inline const Render::MarkType &getMarkType() const { 
				return settings.getMarkType(); }
			inline const int &getMarkLeft() const { 
				return settings.getMarkLeft(); }
			inline const int &getMarkTop() const { 
				return settings.getMarkTop(); }
			inline const bool &getCaseSensitive() const { 
				return settings.getCaseSensitive(); }

			/**
			 * Preset settings for a Dock object
			 */
			class Settings
			{
				public:
					enum ClickAction
					{
						DoNothing,
						IconifyApplication,
						LowerApplication,
						CloseApplication
					};

					inline const int &getSlotWidth() const { 
						return slotwidth; }
					inline const int &getSlotHeight() const { 
						return slotheight; }
					inline const Render::MarkType &getMarkType() const { 
						return marktype; }
					inline const int &getMarkLeft() const { return markleft; }
					inline const int &getMarkTop() const { return marktop; }
					inline const ClickAction &getRightClickAction() const
						{ return rightclickaction; }
					inline const ClickAction &getLeftClickAction() const
						{ return leftclickaction; }
					inline const bool &getCaseSensitive() const { 
						return casesensitive; }
					inline const void setSlotWidth( int w ) { slotwidth = w; }
					inline const void setSlotHeight( int h ) { slotheight = h; }
					inline const void setMarkType( Render::MarkType t ) { 
						marktype = t; }
					inline const void setMarkLeft( int l ) { markleft = l; }
					inline const void setMarkTop( int t ) { marktop = t; }
					inline const void setRightClickAction( 
						ClickAction a ) { rightclickaction = a; }
					inline const void setLeftClickAction( 
						ClickAction a ) { leftclickaction = a; }
					inline const void setCaseSensitive( bool c ) { 
						casesensitive = c; }

					Settings() :
						slotwidth( 64 ),
						slotheight( 64 ),
						marktype( Render::CrossMark ),
						markleft( -1 ),
						marktop( 0 ),
						rightclickaction( IconifyApplication ),
						leftclickaction( DoNothing ),
						casesensitive( false ) {}
					virtual ~Settings() {}

				private:
					int slotwidth;
					int slotheight;
					Render::MarkType marktype;
					int markleft;
					int marktop;
					ClickAction rightclickaction;
					ClickAction leftclickaction;
					bool casesensitive;
			};

			Dock( Dock::Settings & );
			virtual ~Dock();
			virtual bool add( Icon * );
			virtual void run();
			static void send( const char *, const char *, int );
			static void changeIcon( const char *, const char * );
			static void executeIcon( const char * );

			enum
			{
				Suspend = 250000 // = a quarter second
			};
			
		private:
			static const char *ChangeIconMessage;
			static const char *ExecuteIconMessage;

			Display *display;
			int screen;
			GC gc;
			int bitsperpixel;
			int screenwidth;
			int screenheight;
			Dock::Settings settings;
			Slot *first;

			void client( XEvent * );
	};
}

#endif
