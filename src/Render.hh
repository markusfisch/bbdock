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
#ifndef bbdock_Render_hh
#define bbdock_Render_hh

#if defined( __GNUC__ ) || \
	defined( __i686__ ) || \
	defined( __i586__ ) || \
	defined( __i486__ ) || \
	defined( __i386__ )
#define bbdock_x86_att		1
#elif defined( _M_IX86 )
#define bbdock_x86_intel	1
#else
#include <math.h>
#endif

namespace bbdock
{
	/**
	 * Abstract class, this object is only a base for the following
	 * implentations, it got also a static member wich use is to invoke
	 * the right object
	 *
	 * @author mf@markusfisch.de
	 * @version 0.1.4
	 */
	class Render
	{
		public:
			enum MarkType
			{
				PlayMark,
				DotsMark,
				CornerMark,
				CrossMark
			};

			inline const int &getSize() const { return size; }

			Render() {}
			virtual ~Render() {}
			Render &operator=( Render & ) {}
			static Render *getInstance( int, int, int );
			virtual void opaque( unsigned char *, unsigned char * ) {}
			virtual void ghosted( unsigned char *, unsigned char *, 
				unsigned char ) {}
			virtual void activeMark( unsigned char *, MarkType = PlayMark,
				int = 0, int = 0 ) {}

		protected:
			/**
			 * A mark
			 */
			class Mark
			{
				public:
					inline const int &getWidth() const { return width; }
					inline const int &getHeight() const { return height; }
					inline const char *getPixels() const { return pixels; }

					Mark( const int w, const int h, const char *p ) :
						width( w ),
						height( h ),
						pixels( p ) {}
					virtual ~Mark() {}

				private:
					const int width;
					const int height;
					const char *pixels;
			};

			int width;
			int height;
			int size;
			int padding;
			int bytesperline;
			int pixels;
			static const Render::Mark play;
			static const Render::Mark dots;
			static const Render::Mark corner;
			static const Render::Mark cross;

			virtual void initialize( int, int, int );
			virtual const Render::Mark &getMark( MarkType ) const;
			virtual unsigned int getOffset( int, int, Render::Mark &, 
				int ) const;
	};

	/**
	 * Render16 knows how to draw on surfaces of 16 bits color depth
	 *
	 * @author mf@markusfisch.de
	 * @version 0.1.3
	 */
	class Render16 : Render
	{
		public:
			Render16( int, int );
			virtual ~Render16() {}
			virtual void opaque( unsigned char *, unsigned char * );
			virtual void ghosted( unsigned char *, unsigned char *, 
				unsigned char );
			virtual void activeMark( unsigned char *, MarkType = PlayMark,
				int = 0, int = 0 );
	};

	/**
	 * Render24 knows how to draw on surfaces of 24 bits color depth
	 *
	 * @author mf@markusfisch.de
	 * @version 0.1.3
	 */
	class Render24 : Render
	{
		public:
			Render24( int, int );
			virtual ~Render24() {}
			virtual void opaque( unsigned char *, unsigned char * );
			virtual void ghosted( unsigned char *, unsigned char *, 
				unsigned char );
			virtual void activeMark( unsigned char *, MarkType = PlayMark,
				int = 0, int = 0 );
	};

	/**
	 * Render32 knows how to draw on surfaces of 32 bits color depth
	 *
	 * @author mf@markusfisch.de
	 * @version 0.1.3
	 */
	class Render32 : Render
	{
		public:
			Render32( int, int );
			virtual ~Render32() {}
			virtual void opaque( unsigned char *, unsigned char * );
			virtual void ghosted( unsigned char *, unsigned char *, 
				unsigned char );
			virtual void activeMark( unsigned char *, MarkType = PlayMark,
				int = 0, int = 0 );
	};
}

#endif
