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
#include "Render.hh"

using namespace bbdock;

const Render::Mark Render::play( 5, 9, (char []){
	1, 0, 0, 0, 0,
	1, 1, 0, 0, 0,
	1, 1, 1, 0, 0,
	1, 1, 1, 1, 0,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 0,
	1, 1, 1, 0, 0,
	1, 1, 0, 0, 0,
	1, 0, 0, 0, 0 } );
const Render::Mark Render::dots( 10, 2, (char []){
	1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	1, 1, 0, 0, 1, 1, 0, 0, 1, 1 } );
const Render::Mark Render::corner( 6, 6, (char []){
	0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 1, 1,
	0, 0, 0, 1, 1, 1,
	0, 0, 1, 1, 1, 1,
	0, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1 } );
const Render::Mark Render::cross( 7, 7, (char []){
	1, 1, 0, 0, 0, 1, 1,
	1, 1, 1, 0, 1, 1, 1,
	0, 1, 1, 1, 1, 1, 0,
	0, 0, 1, 1, 1, 0, 0,
	0, 1, 1, 1, 1, 1, 0,
	1, 1, 1, 0, 1, 1, 1,
	1, 1, 0, 0, 0, 1, 1 } );

/**
 * Return a Render object for the demanded color depth
 *
 * @param bitsperpixel - bits per pixel (color depth)
 * @param width - width of the image in pixels
 * @param height - height of the image in pixels
 */
Render *Render::getInstance( int bitsperpixel, int width, int height )
{
	switch( bitsperpixel )
	{
		default:
			throw "Pixel depth not (yet) supported";
		case 16:
			return (Render *) new Render16( width, height );
		case 24:
			return (Render *) new Render24( width, height );
		case 32:
			return (Render *) new Render32( width, height );
	}
}

/**
 * Initialize internal parameters
 *
 * @param bitsperpixel - bits per pixel (color depth)
 * @param width - width of the image in pixels
 * @param height - height of the image in pixels
 */
void Render::initialize( int bitsperpixel, int width, int height )
{
	this->width = width;
	this->height = height;

	// ceil to full byte if bits per pixel is fewer than a byte
	// or divide by 8 if there are at last 8 bits per pixel
	if( bitsperpixel < 8 )
		bytesperline = ((width*bitsperpixel)+7) & ~7;
	else
		bytesperline = (width*bitsperpixel)>>3;

	// pad every image line to a multiple of 4 bytes
	padding = bytesperline;
	bytesperline = (bytesperline+3) & ~3;
	padding = bytesperline-padding;

	size = bytesperline*height;
	pixels = width*height;
}

/**
 * Get mark of type
 *
 * @param type - mark identifier
 */
const Render::Mark &Render::getMark( MarkType type ) const
{
	switch( type )
	{
		default:
		case PlayMark:
			return play;
		case DotsMark:
			return dots;
		case CornerMark:
			return corner;
		case CrossMark:
			return cross;
	}
}

/**
 * Calculate offset respecting left/top margin
 *
 * @param left - left margin (or if negative right margin)
 * @param top - top margin (or if negative bottom margin)
 * @param mark - mark to draw
 * @param bytesperpixel - bytes per pixel
 */
unsigned int Render::getOffset( int left, int top, Render::Mark &mark, 
	int bytesperpixel ) const
{
	unsigned int skip = 0;

	if( top < 0 && height-mark.getHeight()+(++top) >= 0 )
		skip += (height-mark.getHeight()+top)*bytesperline;
	else if( top > 0 && top+mark.getHeight() <= height )
		skip += top*bytesperline;

	if( left < 0 && width-mark.getWidth()+(++left) >= 0 )
		skip += bytesperline-((mark.getWidth()-left)*bytesperpixel);
	else if( left > 0 && left+mark.getWidth() <= width )
		skip += left*bytesperpixel;

	return skip;
}

/**
 * Initialize object
 *
 * @param width - width of the image in pixels
 * @param height - height of the image in pixels
 */
Render16::Render16( int width, int height )
{
	initialize( 16, width, height );
}

/**
 * Copy a rgba image into a 16 bit pixmap
 *
 * @param dest - destination pixels
 * @param src - source pixels (in rgba format !)
 */
void Render16::opaque( unsigned char *dest, unsigned char *src )
{
	for( int y = height; y--; dest += padding )
		for( int x = width; x--; )
			switch( *((int *) src) & 0xff000000 )
			{
				case 0:
					src += 4;
					dest += 2;
					break;
				case 0xff000000:
					{
						register int blue = *(src++);
						register int green = *(src++);
						register int red = *(src++);

						*((short *) dest) = 
							((short) ((blue&0xf8)>>3)) |
							((short) ((green&0xf8)<<3)) |
							((short) ((red&0xf8)<<8));

						src++;
						dest += 2;
					}
					break;
				default:
					{
						double diff;
						double mod = (double) 255/(double) (*(src+3));

						short pixel = *((short *) dest);

						register int blue = (pixel<<3)&0xf8;
						register int green = (pixel>>3)&0xf8;
						register int red = (pixel>>8)&0xf8;

						diff = *(src++);
						diff -= blue;
						diff /= mod;
						blue += (int) diff;

						diff = *(src++);
						diff -= green;
						diff /= mod;
						green += (int) diff;

						diff = *(src++);
						diff -= red;
						diff /= mod;
						red += (int) diff;

						*((short *) dest) = 
							((short) ((blue&0xf8)>>3)) |
							((short) ((green&0xf8)<<3)) |
							((short) ((red&0xf8)<<8));

						dest += 2;
						src++;
					}
					break;
			}
}

/**
 * Copy a rgba image greyed into a 16 bit pixmap respecting transparency
 *
 * @param dest - destination pixels
 * @param src - source pixels (in rgba format !)
 * @param transparency - global transparency
 */
void Render16::ghosted( unsigned char *dest, unsigned char *src, 
	unsigned char transparency )
{
	double alphamax = ((double) 255*((double) 255/(double) transparency));

	for( int y = height; y--; dest += padding )
		for( int x = width; x--; )
			if( !(*((int *) src) & 0xff000000) )
			{
				src += 4;
				dest += 2;
			}
			else
			{
				double diff;
				double mod = alphamax/(double) (*(src+3));

				// it must be done that way, if you write it in just
				// line, things get messed up terribly by the compiler 
				// (at least to version 3.3)
				register int red = *(src++);
				register int green = *(src++);
				register int blue = *(src++);
				register int grey = (red+green+blue)/3;

				short pixel = *((short *) dest);

				blue = (pixel<<3)&0xf8;
				green = (pixel>>3)&0xf8;
				red = (pixel>>8)&0xf8;

				diff = grey;
				diff -= blue;
				diff /= mod;
				blue += (int) diff;

				diff = grey;
				diff -= green;
				diff /= mod;
				green += (int) diff;

				diff = grey;
				diff -= red;
				diff /= mod;
				red += (int) diff;

				*((short *) dest) = 
					((short) ((blue&0xf8)>>3)) |
					((short) ((green&0xf8)<<3)) |
					((short) ((red&0xf8)<<8));

				dest += 2;
				src++;
			}
}

/**
 * Render active-mark into a 16 bit surface
 *
 * @param dest - destination pixels
 * @param type - type of the mark (optional)
 * @param left - left margin of mark (optional)
 * @param top - top margin of mark (optional)
 */
void Render16::activeMark( unsigned char *dest, MarkType type, 
	int left, int top )
{
	Render::Mark mark = getMark( type );

	dest += getOffset( left, top, mark, 2 );

	short *p = (short *) dest;
	int skip = width-mark.getWidth();
	const char *src = mark.getPixels();

	for( int y = mark.getHeight(); y--; p += skip )
		for( int x = mark.getWidth(); x--; p++ )
			if( *(src++) )
				*p ^= 0xffffff;
}

/**
 * Initialize object
 *
 * @param width - width of the image in pixels
 * @param height - height of the image in pixels
 */
Render24::Render24( int width, int height )
{
	initialize( 24, width, height );
}

/**
 * Copy a rgba image into a 24 bit pixmap
 *
 * @param dest - destination pixels
 * @param src - source pixels (in rgba format !)
 */
void Render24::opaque( unsigned char *dest, unsigned char *src )
{
	for( int y = height; y--; dest += padding )
		for( int x = width; x--; )
			switch( *((int *) src) & 0xff000000 )
			{
				case 0:
					src += 4;
					dest += 3;
					break;
				case 0xff000000:
					*(dest++) = *(src++);
					*(dest++) = *(src++);
					*(dest++) = *(src++);
					src++;
					break;
				default:
					{
						double diff;
						double mod = (double) 255/(double) (*(src+3));

						diff = *(src++);
						diff -= *dest;
						diff /= mod;
						*(dest++) += (int) diff;

						diff = *(src++);
						diff -= *dest;
						diff /= mod;
						*(dest++) += (int) diff;

						diff = *(src++);
						diff -= *dest;
						diff /= mod;
						*(dest++) += (int) diff;

						src++;
					}
					break;
			}
}

/**
 * Copy a rgba image greyed into a 24 bit pixmap respecting transparency
 *
 * @param dest - destination pixels
 * @param src - source pixels (in rgba format !)
 * @param transparency - global transparency
 */
void Render24::ghosted( unsigned char *dest, unsigned char *src, 
	unsigned char transparency )
{
	double alphamax = ((double) 255*((double) 255/(double) transparency));

	for( int y = height; y--; dest += padding )
		for( int x = width; x--; )
			if( !(*((int *) src) & 0xff000000) )
			{
				src += 4;
				dest += 3;
			}
			else
			{
				double diff;
				double mod = alphamax/(double) (*(src+3));

				// it must be done that brain-dead, if you write it in just
				// line, things get messed up terribly by the compiler 
				// (at least to version 3.3)
				register int red = *(src++);
				register int green = *(src++);
				register int blue = *(src++);
				register int grey = (red+green+blue)/3;

				diff = grey;
				diff -= *dest;
				diff /= mod;
				*(dest++) += (int) diff;

				diff = grey;
				diff -= *dest;
				diff /= mod;
				*(dest++) += (int) diff;

				diff = grey;
				diff -= *dest;
				diff /= mod;
				*(dest++) += (int) diff;

				src++;
			}
}

/**
 * Render active-mark into a 24 bit surface
 *
 * @param dest - destination pixels
 * @param type - type of the mark (optional)
 * @param left - left margin of mark (optional)
 * @param top - top margin of mark (optional)
 */
void Render24::activeMark( unsigned char *dest, MarkType type, 
	int left, int top )
{
	Render::Mark mark = getMark( type );

	dest += getOffset( left, top, mark, 3 );

	int skip = bytesperline-(mark.getWidth()*3);
	const char *src = mark.getPixels();

	for( int y = mark.getHeight(); y--; dest += skip )
		for( int x = mark.getWidth(); x--; )
			if( *(src++) )
			{
				*(dest++) ^= 0xff;
				*(dest++) ^= 0xff;
				*(dest++) ^= 0xff;
			}
}

/**
 * Initialize object
 *
 * @param width - width of the image in pixels
 * @param height - height of the image in pixels
 */
Render32::Render32( int width, int height )
{
	initialize( 32, width, height );
}

/**
 * Copy a rgba image into a 32 bit pixmap
 *
 * @param dest - destination pixels
 * @param src - source pixels (in rgba format !)
 */
void Render32::opaque( unsigned char *dest, unsigned char *src )
{
	for( int p = pixels; p--; )
		switch( *((int *) src) & 0xff000000 )
		{
			case 0:
				src += 4;
				dest += 4;
				break;
			case 0xff000000:
				*((int *) dest) = *((int *) src);
				src += 4;
				dest += 4;
				break;
			default:
				{
					double diff;
					double mod = (double) 255/(double) (*(src+3));

					diff = *(src++);
					diff -= *dest;
					diff /= mod;
					*(dest++) += (int) diff;

					diff = *(src++);
					diff -= *dest;
					diff /= mod;
					*(dest++) += (int) diff;

					diff = *(src++);
					diff -= *dest;
					diff /= mod;
					*(dest++) += (int) diff;

					src++;
					dest++;
				}
				break;
		}
}

/**
 * Copy a rgba image greyed into a 32 bit pixmap respecting transparency
 *
 * @param dest - destination pixels
 * @param src - source pixels (in rgba format !)
 * @param transparency - global transparency
 */
void Render32::ghosted( unsigned char *dest, unsigned char *src, 
	unsigned char transparency )
{
	double alphamax = ((double) 255*((double) 255/(double) transparency));

	for( int p = pixels; p--; )
		if( !(*((int *) src) & 0xff000000) )
		{
			src += 4;
			dest += 4;
		}
		else
		{
			double diff;
			double mod = alphamax/(double) (*(src+3));

			// it must be done that brain-dead, if you write it in just
			// line, things get messed up terribly by the compiler 
			// (at least to version 3.3)
			register int red = *(src++);
			register int green = *(src++);
			register int blue = *(src++);
			register int grey = (red+green+blue)/3;

			diff = grey;
			diff -= *dest;
			diff /= mod;
			*(dest++) += (int) diff;

			diff = grey;
			diff -= *dest;
			diff /= mod;
			*(dest++) += (int) diff;

			diff = grey;
			diff -= *dest;
			diff /= mod;
			*(dest++) += (int) diff;

			src++;
			dest++;
		}
}

/**
 * Render active-mark into a 32 bit surface
 *
 * @param dest - destination pixels
 * @param type - type of the mark (optional)
 * @param left - left margin of mark (optional)
 * @param top - top margin of mark (optional)
 */
void Render32::activeMark( unsigned char *dest, MarkType type, 
	int left, int top )
{
	Render::Mark mark = getMark( type );

	dest += getOffset( left, top, mark, 4 );

	int *p = (int *) dest;
	int skip = width-mark.getWidth();
	const char *src = mark.getPixels();

	for( int y = mark.getHeight(); y--; p += skip )
		for( int x = mark.getWidth(); x--; p++ )
			if( *(src++) )
				*p ^= 0xffffff;
}
