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
#ifndef bbdock_Slot_hh
#define bbdock_Slot_hh

#include <unistd.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <png.h>

#include "Dock.hh"
#include "Icon.hh"
#include "Render.hh"
#include "WindowManager.hh"

namespace bbdock
{
	/**
	 * Shows an icon in the BlackBox slit from wich the user
	 * can launch a corresponding application
	 *
	 * @author mf@markusfisch.de
	 * @version 0.2.3
	 */
	class Slot
	{
		public:
			inline const Window &getWindow() const { return window; }
			inline Slot *getNext() const { return next; }
			inline const char *getIcon() const { 
				return icon->getImage().c_str(); }
			inline const char *getCommand() const { 
				return icon->getCommand().c_str(); }
			inline const void setVisibility( int s ) { visibility = s; }
			inline const void invalidate() { invalid = true; }
			inline const int &getLeft() const { return left; }
			inline const int &getTop() const { return top; }
			inline const int &getWidth() const { return width; }
			inline const int &getHeight() const { return height; }

			Slot( Dock &, Icon &, Slot * );
			virtual ~Slot();
			virtual void draw( bool = false );
			virtual void exec( char * = 0 );
			virtual void iconifyApplication();
			virtual void lowerApplication();
			virtual void closeApplication();
			virtual void tick();
			virtual void setIcon( const char * );
			virtual const bool hasFocus();

		protected:
			virtual void loadIcon();
			virtual void destroyIcon();
			virtual void createSurface();
			virtual void destorySurface();
			virtual void getBackground();

		private:
			enum
			{
				Complete = 255
			};

			Dock *dock;
			Icon *icon;
			Slot *next;
			Render *render;
			Window window;
			XImage *surface;
			int *normalicon;
			char *image;
			char *background;
			int left;
			int top;
			int width;
			int height;
			int visibility;
			bool invalid;
			WindowManager wm;
			int pid;
			int fading;
			static const double fadestep;
	};
}

#endif
