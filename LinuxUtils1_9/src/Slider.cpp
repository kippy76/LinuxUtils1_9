/*
 Copyright (C) 2011, 2012  Christopher Walker

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 3 as
 published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Slider.h"
#include "SDL.h"		// 2D graphics library
#include "SDL_video.h"	// 2D graphics library
#include <math.h>		// sqrt function
Slider::Slider(SDL_Surface* destSurf)
{
	// generate a slider with defaults...
	destinationSurface = destSurf;
	sliderWidth = 150;
	cursorValue = 0;
	sliderBackColour = 0xFFFF0000;
	sliderControlColour = 0xFFFFFF00;
	scaleDivisions = 10;
	// create surface
	SDL_PixelFormat& fmt = *(destinationSurface->format);
	sliderSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, sliderWidth, 25, fmt.BitsPerPixel, fmt.Rmask,
			fmt.Gmask, fmt.Bmask, fmt.Amask);
	// set black as chromakey
	SDL_SetColorKey( sliderSurface, SDL_SRCCOLORKEY, SDL_MapRGB(sliderSurface->format, 0, 0, 0));
	SDL_SetAlpha(sliderSurface, 0, 0);
}

Slider::Slider(SDL_Surface* destSurf, int width, float value, Uint32 scaleColour, Uint32 pointerColour,
		int divisions)
{
	destinationSurface = destSurf;
	sliderWidth = width;
	cursorValue = value;
	sliderBackColour = scaleColour;
	sliderControlColour = pointerColour;
	scaleDivisions = divisions;
	// create surface
	SDL_PixelFormat& fmt = *(destinationSurface->format);
	sliderSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, sliderWidth, 25, fmt.BitsPerPixel, fmt.Rmask,
			fmt.Gmask, fmt.Bmask, fmt.Amask);
	// set black as chromakey
	SDL_SetColorKey( sliderSurface, SDL_SRCCOLORKEY, SDL_MapRGB(sliderSurface->format, 0, 0, 0));
		SDL_SetAlpha(sliderSurface, 0, 0);
}

Slider::~Slider()
{
	SDL_FreeSurface(sliderSurface);
	SDL_FreeSurface(destinationSurface);
}

float Slider::GetSliderValue()
{
	return cursorValue;
}

void Slider::SetSliderValue(float cursorVal)
{
	cursorValue = cursorVal;
	// restrict within bounds...
	cursorValue = cursorValue < 0 ? 0 : cursorValue;
	cursorValue = cursorValue > 100 ? 100 : cursorValue;
}

SDL_Surface* Slider::GetSurface()
{
	SDL_FillRect(sliderSurface, NULL, 0x000000);	// black out before we draw...
	double markerStep = (double) (sliderWidth) / (double) scaleDivisions;
	for (int thisMarker = 1; thisMarker < scaleDivisions; thisMarker++)
	{
		// draw each marker...
		drawLine((markerStep * thisMarker), 0, (markerStep * thisMarker), 20, sliderBackColour);
	}
	// now draw horizontal scale
	drawLine(0, 10, sliderWidth, 10, sliderBackColour); // RGBA, 4bits per colour
	drawLine(0, 11, sliderWidth, 11, sliderBackColour);
	// and end 'caps'
	drawLine(0, 8, 0, 14, sliderBackColour); // Left outer
	drawLine(1, 9, 1, 13, sliderBackColour); // Left inner
	drawLine(sliderWidth -1, 8, sliderWidth - 1, 14, sliderBackColour); // Right outer
	drawLine(sliderWidth -2, 9, sliderWidth - 2, 13, sliderBackColour); // Right inner
	// and 'cursor'
	int cursorX = (((float) (sliderWidth-5) / 100) * cursorValue); //take off 5 for cursor width
	cursorX = (cursorX > (sliderWidth - 5)) ? sliderWidth - 5 : cursorX;
	cursorX = cursorX < 0 ? 0 : cursorX;
	drawLine(cursorX, 2, cursorX, 18, sliderControlColour);
	drawLine(1 + cursorX, 1, 1 + cursorX, 19, sliderControlColour);
	drawLine(2 + cursorX, 1, 2 + cursorX, 19, sliderControlColour);
	drawLine(3 + cursorX, 1, 3 + cursorX, 19, sliderControlColour);
	drawLine(4 + cursorX, 2, 4 + cursorX, 18, sliderControlColour);
	return sliderSurface;
}

void Slider::drawLine(int x1, int y1, int x2, int y2, Uint32 colour)
{
	double x = x2 - x1;
	double y = y2 - y1;
	double length = sqrt(x * x + y * y);
	double addx = x / length;
	double addy = y / length;
	x = x1;
	y = y1;
	Uint8 r = (0xFF000000 & colour) >> 24;	// mask and bit shift down to 8bit
	Uint8 g = (0x00FF0000 & colour) >> 16;
	Uint8 b = (0x0000FF00 & colour) >> 8;
	Uint32 convColour = SDL_MapRGB( sliderSurface->format, r, g, b );
	for (double i = 0; i < length; i += 1)
	{
		Uint32 *bufp;
		bufp = (Uint32 *) sliderSurface->pixels + (int) y * sliderSurface->pitch / 4 + (int) x;
		*bufp = convColour;
		x += addx;
		y += addy;
	}
}
