#ifndef SLIDER_H_
#define SLIDER_H_
#include "SDL.h"		// 2D graphics library
#include "SDL_video.h"	// 2D graphics library
class Slider
{
	public:
		Slider(SDL_Surface* destSurf);
		Slider(SDL_Surface* destSurf, int sliderWidth, float cursorValue, Uint32 sliderBackColour, Uint32 sliderControlColour,
				int scaleDivisions);
		virtual ~Slider();
		float GetSliderValue();
		void SetSliderValue(float cursorVal);
		SDL_Surface* GetSurface();
		void drawLine(int x1, int y1, int x2, int y2, Uint32 colour);
		Uint32 sliderBackColour;
		Uint32 sliderControlColour;
	private:
		int sliderWidth;
		float cursorValue;
		int scaleDivisions;
		SDL_Surface* sliderSurface; // 2D drawing plane
		SDL_Surface* destinationSurface; // 2D drawing plane
};

#endif /* SLIDER_H_ */
