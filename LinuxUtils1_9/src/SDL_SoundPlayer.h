#ifndef SDL_SOUNDPLAYER_H_
#define SDL_SOUNDPLAYER_H_

#include <SDL.h>
#include <SDL_mixer.h>
#include <map>

class SDL_SoundPlayer
{
	public:

		SDL_SoundPlayer();
		virtual ~SDL_SoundPlayer();
		int loadWav(char* filename, int wavID);
		int playWav(int wavID);

	private:
		std::map<int, Mix_Chunk*> wavStore;
		std::map<int, Mix_Chunk*>::iterator wavIterator;
		char* filename;
};

#endif /* SDL_SOUNDPLAYER_H_ */
