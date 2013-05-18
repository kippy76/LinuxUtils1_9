/*
    Copyright (C) 2011,2012  Christopher Walker

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
#include "SDL_SoundPlayer.h"

SDL_SoundPlayer::SDL_SoundPlayer()
{
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024);
}

SDL_SoundPlayer::~SDL_SoundPlayer()
{
	for (wavIterator = wavStore.begin(); wavIterator != wavStore.end(); wavIterator++)
	{
		Mix_FreeChunk(wavStore[(*wavIterator).first]);
	}
	wavStore.clear();
	Mix_CloseAudio();
}

/*
 * Loads a WAV file into a vector, ready for
 * playback at a later date.
 * Function returns 0 if load OK.
 * Returns 1 if file IO error.
 * Returns 2 if the wavID is a duplicate of one already stored.
 */
int SDL_SoundPlayer::loadWav(char* filename, int wavID)
{
	wavIterator = wavStore.find(wavID);
	if (wavIterator != wavStore.end())
	{
		return 2;
	}
	wavStore[wavID] = Mix_LoadWAV(filename);
	if (wavStore[wavID] == NULL)
	{
		return 1;
	}
	return 0;
}

/*
 * Plays a wav file previously stored by 'wavID'.
 * returns 0 if playback successful, -1 otherwise.
 */
int SDL_SoundPlayer::playWav(int wavID)
{
	wavIterator = wavStore.find(wavID);
	if (wavIterator == wavStore.end())
	{
		return -1;
	}
	Mix_PlayChannel(-1, wavStore[wavID], 0);
	return 0;
}
