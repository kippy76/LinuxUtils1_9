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
#include "SDL.h"		// 2D graphics library
#include "SDL_video.h"	// 2D graphics library
#include "SDL_image.h"	// GIF manipulations
#include "SDL_ttf.h"	// SDL TTF support
#include <math.h>		// sqrt function
#include <time.h>		// for polling weather data updating
#include <iostream>
#include <string>
#include <sstream>
#include "SDL_SoundPlayer.h"
#include "Slider.h"
#include "WeatherData.h"

/*
 * Customise to suit your particular monitor.....
 */
const double XGAMMA_MAX_VALUE = 2.0;
const int RGB_DEFAULT = 50; // in percent
const int GAMMA_DEFAULT = 100; // in percent
// D93 Colour Temperature Values
const int D93_R = 82; // in percent
const int D93_G = 84; // in percent
const int D93_B = 89; // in percent
const int D93_A = 35; // in percent
// D65 Colour Temperature Values
const int D65_R = 100; // in percent
const int D65_G = 94; // in percent
const int D65_B = 89; // in percent
const int D65_A = 31; // in percent
// D55 Colour Temperature Values
const int D55_R = 98; // in percent
const int D55_G = 91; // in percent
const int D55_B = 78; // in percent
const int D55_A = 32; // in percent
/*
 * Customise for weather for your location
 * Note, expects 3 day forecast, so only amend 2654497 to the value of your location.
 */
std::string weatherXML = "http://open.live.bbc.co.uk/weather/feeds/en/2654497/3dayforecast.rss";
//
const int VERSION_MAJOR = 1;
const int VERSION_MINOR = 9;
const std::string REV_DATE = "11/04/2012";
//
/* ----------------------------------------------------------------------------------------------
 * Name:				LinuxUtils
 * Author:				Christopher Walker
 * Data Last Modified : 11/04/2012
 * Version :			1.9
 * ----------------------------------------------------------------------------------------------
 * Changes :			1.9: Addition of rudimentary XML parser for RSS weather data.
 * 							 Amended weather reporter to work around proxy issue in 'wget'.
 * 							 Added '-weather' command line option to give 3-day weather forecast.
 * 						1.8: Addition of POSIX threads for weather data collection,
 * 							 addition of Win-Up & Win-Down to carry out gamma changes.
 * 						1.7: Class and menu option added for weather information.
 * 							 Grabs data from BBC website in RSS and parses.
 * 						1.6: Exporting of slider functionality to class level.
 * ----------------------------------------------------------------------------------------------
 * Description :		Utility program with GUI for amendment of gamma
 * 						values, and clearance of history files. Gamma values
 * 						can be adjusted independently for each colour (RGB).
 * 						Values are saved to file on exit from program,
 * 						and automatically loaded & refreshed when program
 * 						is run, or if executed with -gamma option. With the GUI
 * 						active, WIN-UP and WIN-DOWN carry out immediate gamma
 * 						changes.
 * ----------------------------------------------------------------------------------------------
 * Command Switches :	-clean
 *							Shreds history items (BASH history, multimedia MRU and global MRU)
 *							as well as local cache and thumbnail files.
 *						-gamma
 *							Applies last gamma values without invoking GUI.
 *						-help
 * 							Displays assistance information.
 * 						-silent
 * 							Suppresses output of messages to terminal.
 * 						-weather
 * 							Provides a 3-day weather forecast to terminal window.
 * 						no switch
 * 							Launches GUI.
 * ----------------------------------------------------------------------------------------------
 */
/*
 * Function prototypes...
 */
int main(int, char**);
int parseArgs(int argc, char *argv[]);
void showHelp();
void cleanup();
bool initDefaults();
void menu1loadRGBdefaults();
void menu1saveRGBsettings();
void menu2shredItems();
void menu1applyRGB();
void menu1applyPreset(int colourTemp);
int initGFX();
int initFont();
void keyProcess(SDL_keysym*, bool);
void mouseProcess(int x, int y, int xrel, int yrel, char mouseButLR);
void processEvents();
void draw_line(int x1, int y1, int x2, int y2, SDL_Surface*, Uint32 colour);
void drawMenuHL();
void drawMenu1Page();
void drawMenu2Page();
void drawMenu3Page();
void updateGFX();
SDL_Surface* initDisplay();
bool outputText(int x, int y, std::string text, unsigned int r, unsigned int g, unsigned int b, bool large);
template<class T> inline std::string to_string(const T& t);
/*
 * Global variables
 */
SDL_SoundPlayer* wavPlayer;
int screenWidth = 240;
int screenHeight = 240;
TTF_Font* fontFaceLarge; // pointer to font struct
TTF_Font* fontFaceSmall; // pointer to font struct
FILE *program; // text file holding settings
bool globalSilence;
SDL_Surface *screen; // 2D drawing plane
SDL_Surface *background; // 2D drawing plane
SDL_Surface *menu; // 2D drawing plane
SDL_Surface *cleanupBack; // 2D drawing plane
SDL_Surface* menuHLsurf;
bool quitApp;
bool mouseButtonDown;
int activeMenuSelection;
int lastActiveMenuSelection;
Uint32 RED = 0xFF000000;
Uint32 GREEN = 0x00FF0000;
Uint32 BLUE = 0x0000FF00;
Uint32 WHITE = 0xFFFFFF00;
Uint32 YELLOW = 0xFFFF0000;
Slider* menu1sliders[4] = { NULL, NULL, NULL, NULL };
WeatherData* weatherDataGrabber;
int menu1sliderLength = 180;
bool menu2Cleaned;
bool weatherDataValid; // true if weather data successfully updated

int main(int argc, char* argv[])
{
	if (initGFX())
	{
		quitApp = true;
		if (!globalSilence)
		{
			printf("  ** Error: Unable to initialise video mode: %s **\n", SDL_GetError());
		}
	}
	if (parseArgs(argc, argv))
	{
		cleanup();
		return 0;
	}
	if (initDefaults())
	{
		cleanup();
		return 0;
	}
	if (initFont())
	{
		quitApp = true;
	}
	while (!quitApp)
	{
		processEvents();
		updateGFX();
	}
	cleanup();
	return 0;
}

int parseArgs(int argc, char* argv[])
{
	std::string argFull = "";
	int result = 0;
	globalSilence = false;
	if (argc < 2)
	{
		return result;
	}
	for (int arg = 1; arg < argc; arg++)
	{
		argFull += argv[arg];
		if (arg != argc - 1)
			argFull += " ";
	}
	for (unsigned int ch = 0; ch < argFull.length(); ch++)
	{
		argFull[ch] = tolower(argFull[ch]);
	}
	// argFull contains entire concatenated string of arguments
	if (argFull.find("-silent") != std::string::npos)
	{
		globalSilence = true;
		result = 1;
	}
	if (argFull.find("-help") != std::string::npos)
	{
		showHelp();
		result = 1;
	}
	if (argFull.find("-clean") != std::string::npos)
	{
		menu2shredItems();
		result = 1;
	}
	if (argFull.find("-gamma") != std::string::npos)
	{
		menu1loadRGBdefaults();
		result = 1;
	}
	if (argFull.find("-weather") != std::string::npos)
	{
		weatherDataGrabber = new WeatherData();
		weatherDataGrabber->updateWeatherData(weatherXML);
		int tries = 3;
		printf("Trying to connect to weather feed...\n\n");
		time_t timeLast, timeCurrent;
		timeLast = timeCurrent = time(NULL);
		do
		{
			weatherDataValid = weatherDataGrabber->isDataValid();
			if (!weatherDataValid)
			{
				timeCurrent = time(NULL);
				if (timeCurrent - timeLast > 2)	// only polls every 2 seconds
				{
					printf("Trying to connect to weather feed...\n\n");
					weatherDataGrabber->updateWeatherData(weatherXML);
					tries--;
					timeLast = time(NULL);
				}
			}
			else
			{
				weatherDataGrabber->forecastToTerminal();
			}
		}
		while ((tries > 0) && (!weatherDataValid));
		if (!weatherDataValid)
		{
			printf("Failed to connect to weather feed...\n\n");
		}
		result = 1;
	}
	return result; // zero means proceed with GUI launch
}

void showHelp()
{
	printf("\nUsage :\n");
	printf("     LinuxUtils <options>\n");
	printf("\nDescription :\n");
	printf("     A range of utilities which can be utilised from the GUI\n");
	printf("     or via the command line. The current version provides the\n");
	printf("     facility to manually change the gamma values of RGB components\n");
	printf("     individually, and store the settings for easy invocation during\n");
	printf("     startup via the command prompt. Gamma value adjustments are\n");
	printf("     automatically saved and loaded on entry and exit to GUI.\n");
	printf("     The program also clears history items via shredding to ensure\n");
	printf("     secure removal of tracking information\n");
	printf("     With the GUI present, quick gamma changes can be made\n");
	printf("     via the WIN-UP and WIN-DOWN key combinations.\n\n");
	printf("Options :\n");
	printf("     -clean\n");
	printf("          Shreds history items (BASH history, multimedia MRU and global MRU)\n");
	printf("          as well as local cache and thumbnail files.\n");
	printf("     -gamma\n");
	printf("          Applies last gamma values without invoking GUI.\n");
	printf("     -help\n");
	printf("          Displays these assistance notes.\n");
	printf("     -silent\n");
	printf("          Inhibits output to terminal window. Can be used in conjunction\n");
	printf("          with other command switch options.\n");
	printf("     -weather\n");
	printf("          Provides a 3-day weather forecast.\n");
	printf("------------------------------------------------------------------------\n");
	printf("Author  : Christopher Walker\n");
	printf("Version : %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
	printf("Date    : %s\n", &REV_DATE[0]);
	printf("------------------------------------------------------------------------\n");

}

void cleanup()
{
	/*
	 * All sections wrapped in try/catch, as not all
	 * variables will have been initialised, depending
	 * on the point of failure/success on program start.
	 * No action required, just to avoid segmentation faults
	 * by proceeding to execute code further.
	 */
	menu1saveRGBsettings();
	// before we destroy all 4 instances...
	try
	{
		for (int loop = 0; loop < 4; loop++)
		{
			delete menu1sliders[loop];
		}
	}
	catch (std::exception exc)
	{
	}
	try
	{
		SDL_FreeSurface(screen);
		SDL_FreeSurface(background);
		SDL_FreeSurface(menu);
		SDL_FreeSurface(cleanupBack);
		SDL_Quit();
	}
	catch (std::exception exc)
	{
	}
	try
	{
		delete wavPlayer;
		delete weatherDataGrabber;
	}
	catch (std::exception exc)
	{
	}
	try
	{
		TTF_CloseFont(fontFaceLarge);
		TTF_CloseFont(fontFaceSmall);
		TTF_Quit();
	}
	catch (std::exception exc)
	{
	}
}

bool initDefaults()
{
	weatherDataGrabber = new WeatherData();
	activeMenuSelection = 1;
	lastActiveMenuSelection = 1;
	mouseButtonDown = false;
	menu1loadRGBdefaults();
	// if we are not instantiating GUI, quit now by signalling
	if (globalSilence)
	{
		return true;
	}
	menu2Cleaned = false;
	quitApp = false;
	wavPlayer = new SDL_SoundPlayer();
	char shredWav[] = "shred.wav";
	char clickWav[] = "click.wav";
	if ((wavPlayer->loadWav(shredWav, 1)) || (wavPlayer->loadWav(clickWav, 2)))
	{
		if (!globalSilence)
		{
			printf("Unable to load wav file : %s \n", TTF_GetError());
		}
		return true;
	}
	return false;
}

void menu1loadRGBdefaults()
{
	for (int loop = 0; loop < 4; loop++)
	{
		menu1sliders[loop] = new Slider(screen, menu1sliderLength, RGB_DEFAULT, YELLOW, WHITE, 10);
	}
	menu1sliders[0]->sliderControlColour = RED;
	menu1sliders[1]->sliderControlColour = GREEN;
	menu1sliders[2]->sliderControlColour = BLUE;
	menu1sliders[3]->SetSliderValue(GAMMA_DEFAULT); // gamma
	char fileLine[10];
	program = fopen(".gamma", "r");
	if (program == NULL)
	{
		if (!globalSilence)
		{
			printf("No gamma settings file found. Using defaults...\n");
			// note use of %% which escapes a percent symbol
			printf("Red  : %3.1f%%\nGreen: %3.1f%%\nBlue : %3.1f%%\nGamma: %3.1f%%\n", (float) RGB_DEFAULT,
					(float) RGB_DEFAULT, (float) RGB_DEFAULT, (float) GAMMA_DEFAULT);
		}
		// Just call with default values
		menu1applyRGB();
		return;
	}
	if (!globalSilence)
	{
		printf("Gamma settings file found. Parsing...\n");
	}
	// File contents in sequence RGBA
	int curr = 0;
	while (fgets(fileLine, 10, program) != NULL)
	{
		try
		{
			float cursorVal;
			sscanf(fileLine, "%f", &cursorVal);
			menu1sliders[curr]->SetSliderValue(cursorVal);
		}
		catch (std::exception exc)
		{
			// Just skip, as all sliders initialised to defaults
		}
		curr++;
	}
	if (!globalSilence)
	{
		printf("Red  :  %3.1f\n", menu1sliders[0]->GetSliderValue());
	}
	if (!globalSilence)
	{
		printf("Green:  %3.1f\n", menu1sliders[1]->GetSliderValue());
	}
	if (!globalSilence)
	{
		printf("Blue :  %3.1f\n", menu1sliders[2]->GetSliderValue());
	}
	if (!globalSilence)
	{
		printf("Gamma:  %3.1f\n", menu1sliders[3]->GetSliderValue());
	}
	menu1applyRGB();
	fclose(program);
}

void menu1saveRGBsettings()
{
	// if sliders not initialised, cannot save settings
	if (menu1sliders[0] == NULL)
	{
		return;
	}
	program = fopen(".gamma", "w");
	if (program == NULL)
	{
		if (!globalSilence)
		{
			printf("Saving of gamma settings to file failed.\n");
		}
		return;
	}
	if (!globalSilence)
	{
		printf("Saving gamma settings to file...\n");
	}
	for (int loop = 0; loop < 4; loop++)
	{
		fprintf(program, " %3.1f\n", menu1sliders[loop]->GetSliderValue());
		if (!globalSilence)
		{
			switch (loop)
			{
				case 0:
					printf("Red  :  %3.1f\n", menu1sliders[loop]->GetSliderValue());
					break;
				case 1:
					printf("Green:  %3.1f\n", menu1sliders[loop]->GetSliderValue());
					break;
				case 2:
					printf("Blue :  %3.1f\n", menu1sliders[loop]->GetSliderValue());
					break;
				default:
					printf("Gamma:  %3.1f\n", menu1sliders[loop]->GetSliderValue());
			}
		}
	}
	fclose(program);
}

void menu2shredItems()
{
	const char* comm = NULL;
	if (globalSilence)
		comm = "shred -f -u ~/.recently-used.xbel > /dev/null 2>&1";
	else
		comm = "shred -f -u ~/.recently-used.xbel";
	system(comm); // Global MRU
	if (globalSilence)
		comm = "shred -f -u ~/.local/share/recently-used.xbel > /dev/null 2>&1";
	else
		comm = "shred -f -u ~/.local/share/recently-used.xbel";
	system(comm); // MM MRU
	if (globalSilence)
		comm = "shred -f -u ~/.bash_history > /dev/null 2>&1";
	else
		comm = "shred -f -u ~/.bash_history";
	system(comm); // Bash history
	if (globalSilence)
		comm = "rm -rf ~/.local/share/Trash/* > /dev/null 2>&1";
	else
		comm = "rm -rf ~/.local/share/Trash/*";
	system(comm); // User Trash can
	if (globalSilence)
		comm = "rm -rf ~/.cache/* > /dev/null 2>&1";
	else
		comm = "rm -rf ~/.cache/*";
	system(comm); // Cache
	if (globalSilence)
		comm = "rm -rf ~/.thumbnails/* > /dev/null 2>&1";
	else
		comm = "rm -rf ~/.thumbnails/*";
	system(comm); // Thumbnails
}

void menu1applyRGB()
{
	// Use values in menu1sliders to calculate RGB using A as multiplier
	std::string com;
	float rVal, gVal, bVal, aVal;
	aVal = menu1sliders[3]->GetSliderValue();
	rVal = menu1sliders[0]->GetSliderValue() * (aVal / 100);
	gVal = menu1sliders[1]->GetSliderValue() * (aVal / 100);
	bVal = menu1sliders[2]->GetSliderValue() * (aVal / 100);
	// rVal et al now value between 0 > 100%. Map to 0.1 > XGAMMA_MAX_VALUE
	double scaleFactor = ((XGAMMA_MAX_VALUE - 0.1) / 100);
	rVal = 0.1 + (rVal * scaleFactor);
	gVal = 0.1 + (gVal * scaleFactor);
	bVal = 0.1 + (bVal * scaleFactor);
	com = "xgamma -rgamma " + to_string(rVal) + " -ggamma " + to_string(gVal) + " -bgamma " + to_string(bVal)
			+ " -quiet";
	const char* comm = &com[0];
	system(comm);
}

void menu1applyPreset(int colourTemp)
{
	int rTarget, gTarget, bTarget, aTarget;
	switch (colourTemp)
	{
		case 93:
			rTarget = D93_R;
			gTarget = D93_G;
			bTarget = D93_B;
			aTarget = D93_A;
			break;
		case 65:
			rTarget = D65_R;
			gTarget = D65_G;
			bTarget = D65_B;
			aTarget = D65_A;
			break;
		case 55:
			rTarget = D55_R;
			gTarget = D55_G;
			bTarget = D55_B;
			aTarget = D55_A;
			break;
		default:
			rTarget = RGB_DEFAULT;
			gTarget = RGB_DEFAULT;
			bTarget = RGB_DEFAULT;
			aTarget = GAMMA_DEFAULT;
	}
	wavPlayer->playWav(2);
	bool gammaReset = false;
	while (!gammaReset)
	{
		if (menu1sliders[0]->GetSliderValue() > rTarget)
		{
			menu1sliders[0]->SetSliderValue(menu1sliders[0]->GetSliderValue() - 1);
		}
		if (menu1sliders[0]->GetSliderValue() < rTarget)
		{
			menu1sliders[0]->SetSliderValue(menu1sliders[0]->GetSliderValue() + 1);
		}
		if (menu1sliders[1]->GetSliderValue() > gTarget)
		{
			menu1sliders[1]->SetSliderValue(menu1sliders[1]->GetSliderValue() - 1);
		}
		if (menu1sliders[1]->GetSliderValue() < gTarget)
		{
			menu1sliders[1]->SetSliderValue(menu1sliders[1]->GetSliderValue() + 1);
		}
		if (menu1sliders[2]->GetSliderValue() > bTarget)
		{
			menu1sliders[2]->SetSliderValue(menu1sliders[2]->GetSliderValue() - 1);
		}
		if (menu1sliders[2]->GetSliderValue() < bTarget)
		{
			menu1sliders[2]->SetSliderValue(menu1sliders[2]->GetSliderValue() + 1);
		}
		if (menu1sliders[3]->GetSliderValue() > aTarget)
		{
			menu1sliders[3]->SetSliderValue(menu1sliders[3]->GetSliderValue() - 1);
		}
		if (menu1sliders[3]->GetSliderValue() < aTarget)
		{
			menu1sliders[3]->SetSliderValue(menu1sliders[3]->GetSliderValue() + 1);
		}
		// clean up doubles so we can get a realistic match
		for (int loop = 0; loop < 4; loop++)
		{
			int replaceValue = menu1sliders[loop]->GetSliderValue();
			menu1sliders[loop]->SetSliderValue(replaceValue);
		}
		if (((int) (menu1sliders[0]->GetSliderValue())) == ((int) rTarget)
				&& ((int) (menu1sliders[1]->GetSliderValue())) == ((int) gTarget)
				&& ((int) (menu1sliders[2]->GetSliderValue())) == ((int) bTarget)
				&& ((int) (menu1sliders[3]->GetSliderValue())) == ((int) aTarget))
		{
			gammaReset = true;
		}
		SDL_Delay(3);
		menu1applyRGB();
		updateGFX();
	}
	wavPlayer->playWav(2);
}

int initGFX()
{
	SDL_Surface* imageLoaded = NULL;
	if ((screen = initDisplay()) == NULL)
	{
		return 1;
	}
	if ((imageLoaded = IMG_Load("back.png")) == NULL)
	{
		return 1;
	}
	background = SDL_DisplayFormatAlpha(imageLoaded);
	SDL_FreeSurface(imageLoaded);
	if ((imageLoaded = IMG_Load("menu.png")) == NULL)
	{
		return 1;
	}
	menu = SDL_DisplayFormatAlpha(imageLoaded);
	SDL_FreeSurface(imageLoaded);
	if ((imageLoaded = IMG_Load("clearupBack.png")) == NULL)
	{
		return 1;
	}
	cleanupBack = SDL_DisplayFormatAlpha(imageLoaded);
	SDL_FreeSurface(imageLoaded);
	return 0;
}

int initFont()
{
	if (TTF_Init() == -1)
	{
		if (!globalSilence)
		{
			printf("Unable to initialise SDL_ttf: %s \n", TTF_GetError());
		}
		return 1;
	}
	if (((fontFaceSmall = TTF_OpenFont("font.ttf", 10)) == NULL) || ((fontFaceLarge = TTF_OpenFont(
			"font.ttf", 13)) == NULL))
	{
		if (!globalSilence)
		{
			printf("Unable to load TTF font\n");
		}
		return 1;
	}
	return 0;

}

void mouseProcess(int x, int y, int xrel, int yrel, char mouseButLR)
{
	bool mouseMoved = false;
	// Check for menu selections...
	if ((mouseButtonDown) && (y < 50))
	{
		activeMenuSelection = (x / 48) + 1;
		if (lastActiveMenuSelection != activeMenuSelection)
		{
			wavPlayer->playWav(2);
			lastActiveMenuSelection = activeMenuSelection;
		}
		if (activeMenuSelection == 5)
		{
			quitApp = true;
			SDL_Delay(50); // Ensures exit click heard!
			return; // fail fast
		}
	}
	if (activeMenuSelection == 1)
	{
		// Check for gamma reset first...
		if ((mouseButtonDown) && (mouseButLR == 'l'))
		{
			if (x > 30 && x < 55 && y > 196 && y < 210)
			{
				menu1applyPreset(93);
			}
			if (x > 74 && x < 98 && y > 196 && y < 210)
			{
				menu1applyPreset(65);
			}
			if (x > 116 && x < 142 && y > 196 && y < 210)
			{
				menu1applyPreset(55);
			}
			if (x > 167 && x < 215 && y > 196 && y < 210)
			{
				menu1applyPreset(0);
			}
		}
		// Menu 1 handling - Gamma adjustments...
		for (int thisSlider = 0; thisSlider < 4; thisSlider++)
		{
			if ((mouseButtonDown) && (mouseButLR == 'l') && (y > (55 + (thisSlider * 25))) && (y < (80
					+ (thisSlider * 25))) && (x > (((float) menu1sliderLength / 100)
					* menu1sliders[thisSlider]->GetSliderValue()) + 15) && (x <= (((float) menu1sliderLength
					/ 100) * menu1sliders[thisSlider]->GetSliderValue()) + 45))
			{
				menu1sliders[thisSlider]->SetSliderValue(
						menu1sliders[thisSlider]->GetSliderValue() + (100 / (float) menu1sliderLength) * xrel);
				mouseMoved = true;
			}
			// constrain movement limits...
			menu1sliders[thisSlider]->SetSliderValue(
					menu1sliders[thisSlider]->GetSliderValue() < 0 ? 0
							: menu1sliders[thisSlider]->GetSliderValue());
			menu1sliders[thisSlider]->SetSliderValue(
					menu1sliders[thisSlider]->GetSliderValue() > 100 ? 100
							: menu1sliders[thisSlider]->GetSliderValue());
		}
		if (mouseMoved)
		{
			menu1applyRGB();
		}
	}
}

void processEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				quitApp = true;
				break;
			case SDL_MOUSEMOTION:
				mouseProcess(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, 'l');
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouseButtonDown = true;
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mouseProcess(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, 'l');
				}
				else
				{
					mouseProcess(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, 'r');
				}
				break;
			case SDL_MOUSEBUTTONUP:
				mouseButtonDown = false;
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					mouseProcess(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, 'l');
				}
				else
				{
					mouseProcess(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel, 'r');
				}
				break;
		}
	}
	// Check for win +/-
	Uint8* keystates = SDL_GetKeyState(NULL);
	if (keystates[SDLK_LMETA] && keystates[SDLK_UP])
	{
		// Increase gamma
		menu1sliders[3]->SetSliderValue(menu1sliders[3]->GetSliderValue() + 1);
		menu1applyRGB();
		SDL_Delay(50);
	}
	if (keystates[SDLK_LMETA] && keystates[SDLK_DOWN])
	{
		// Decrease gamma
		menu1sliders[3]->SetSliderValue(menu1sliders[3]->GetSliderValue() - 1);
		menu1applyRGB();
		SDL_Delay(50);
	}
}

void draw_line(int x1, int y1, int x2, int y2, SDL_Surface* surf, Uint32 colour)
{
	double x = x2 - x1;
	double y = y2 - y1;
	double length = sqrt(x * x + y * y);
	double addx = x / length;
	double addy = y / length;
	x = x1;
	y = y1;
	for (double i = 0; i < length; i += 1)
	{
		Uint32 *bufp;
		bufp = (Uint32 *) surf->pixels + (int) y * surf->pitch / 4 + (int) x;
		*bufp = colour;
		x += addx;
		y += addy;
	}
}

void drawMenuHL()
{
	int alphaBands = 7; // highlight border width
	int alphaMax = 150; // opaque value
	int alphaMin = 30; // transparent value
	Uint8 alphaValue;
	Uint32 MENUHL;
	const SDL_PixelFormat& fmt = *(menu->format);
	menuHLsurf = SDL_CreateRGBSurface(SDL_SWSURFACE, 240, 48, fmt.BitsPerPixel, fmt.Rmask, fmt.Gmask,
			fmt.Bmask, fmt.Amask);
	int topLeft = (activeMenuSelection - 1) * 48;
	int x1, x2, y1, y2;
	x1 = topLeft;
	x2 = x1 + 47;
	y1 = 0;
	y2 = 47;
	for (int loop = 0; loop < alphaBands; loop++)
	{
		alphaValue = 150 - loop * ((alphaMax - alphaMin) / (alphaBands - 1));
		MENUHL = SDL_MapRGBA(menu->format, 50, 255, 50, alphaValue); // RGBA-using MENU format
		draw_line(x1, y1, x2, y1, menuHLsurf, MENUHL);
		draw_line(x2, y1, x2, y2, menuHLsurf, MENUHL);
		draw_line(x2, y2, x1, y2, menuHLsurf, MENUHL);
		draw_line(x1, y2, x1, y1, menuHLsurf, MENUHL);
		x1++;
		y1++;
		x2--;
		y2--;
	}
	SDL_BlitSurface(menuHLsurf, NULL, screen, NULL);
	SDL_FreeSurface(menuHLsurf);
}

void drawMenu1Page()
{
	// sliders move from x = 30 to x = 176,
	SDL_Rect sliderDest;
	sliderDest.x = 30;
	sliderDest.y = 55;
	sliderDest.w = menu1sliderLength;
	sliderDest.h = 25;
	for (int currentSlider = 0; currentSlider < 4; currentSlider++)
	{
		// grab each slider surface and blit away
		SDL_BlitSurface(menu1sliders[currentSlider]->GetSurface(), NULL, screen, &sliderDest);
		sliderDest.y += 25;
	}
	// And finally the legend text
	char* txtR = new char[7];
	char* txtG = new char[7];
	char* txtB = new char[7];
	char* txtA = new char[16];
	int rNum = sprintf(txtR, "%3.1f", menu1sliders[0]->GetSliderValue());
	int gNum = sprintf(txtG, "%3.1f", menu1sliders[1]->GetSliderValue());
	int bNum = sprintf(txtB, "%3.1f", menu1sliders[2]->GetSliderValue());
	int aNum = sprintf(txtA, "%3.1f", menu1sliders[3]->GetSliderValue());
	txtR[rNum] = '%';
	txtR[rNum + 1] = '\0';
	txtG[gNum] = '%';
	txtG[gNum + 1] = '\0';
	txtB[bNum] = '%';
	txtB[bNum + 1] = '\0';
	txtA[aNum] = '%';
	txtA[aNum + 1] = '\0';
	outputText(30, 150, &txtR[0], 0xff, 0x00, 0x00, true);
	outputText(100, 150, &txtG[0], 0x00, 0xff, 0x00, true);
	outputText(170, 150, &txtB[0], 0x00, 0x00, 0xff, true);
	outputText(80, 170, "GAMMA : ", 0xff, 0xff, 0xff, true);
	outputText(140, 170, &txtA[0], 0xff, 0xff, 0xff, true);
	// Finally, the standard colour presets footer....
	outputText(30, 195, "D93     D65     D55       RESET", 0xff, 0xff, 0xff, true);
	SDL_Flip(screen);
	// clearup dynamic memory
	delete[] (txtR);
	delete[] (txtG);
	delete[] (txtB);
	delete[] (txtA);
}

void drawMenu2Page()
{
	SDL_Rect destClearup;
	destClearup.x = 15;
	destClearup.y = 55;
	SDL_BlitSurface(cleanupBack, NULL, screen, &destClearup);
	if (!menu2Cleaned)
	{
		menu2shredItems();
		wavPlayer->playWav(1);
		outputText(50, 70, "Global MRU Shredded", 0, 0, 0, true);
		SDL_Flip(screen);
		SDL_Delay(200);
		outputText(45, 85, "MMedia MRU Shredded", 0, 0, 0, true);
		SDL_Flip(screen);
		SDL_Delay(200);
		outputText(46, 100, "Bash History Shredded", 0, 0, 0, true);
		SDL_Flip(screen);
		SDL_Delay(200);
		outputText(52, 115, "Trash Can Emptied (User)", 0, 0, 0, true);
		SDL_Flip(screen);
		SDL_Delay(200);
		outputText(68, 130, "Cache Emptied", 0, 0, 0, true);
		SDL_Flip(screen);
		SDL_Delay(200);
		outputText(53, 145, "Thumbnails Emptied", 0, 0, 0, true);
		SDL_Flip(screen);
		SDL_Delay(200);
	}
	else
	{
		outputText(50, 70, "Global MRU Shredded", 0, 0, 0, true);
		outputText(45, 85, "MMedia MRU Shredded", 0, 0, 0, true);
		outputText(46, 100, "Bash History Shredded", 0, 0, 0, true);
		outputText(52, 115, "Trash Can Emptied (User)", 0, 0, 0, true);
		outputText(68, 130, "Cache Emptied", 0, 0, 0, true);
		outputText(53, 145, "Thumbnails Emptied", 0, 0, 0, true);
	}
	menu2Cleaned = true;
}

void drawMenu3Page()
{
	weatherDataValid = weatherDataGrabber->isDataValid();
	if (!weatherDataValid)
	{
		// something went wrong with data fetch...
		outputText(30, 70, "Failed to fetch weather data", 255, 60, 60, true);
		outputText(60, 90, "trying to connect...", 255, 60, 60, true);
		/*
		 * now, calling updateWeatherData spawns a thread, and we
		 * don't want to spawn 100s of threads, so update at most once every 5 seconds.
		 * This way, if we are on the weather tab without an internet connection,
		 * it should automatically update with weather data shortly after
		 * re-establishing an internet connection.
		 */
		time_t seconds = time(NULL);
		if (seconds % 5 == 0)
		{
			weatherDataGrabber->updateWeatherData(weatherXML);
		}
	}
	else
	{
		int linePos = 60;
		int lineStep = 10;
		int blue = 255;
		for (int thisDay = 0; thisDay < 3; thisDay++)
		{
			if (thisDay != 0)
			{
				blue = 0;
			}
			outputText(20, linePos, to_string(weatherDataGrabber->day[thisDay].day), 255, 255, blue, false);
			linePos += lineStep;
			outputText(20, linePos, to_string(weatherDataGrabber->day[thisDay].description), 255, 255, blue,
					false);
			linePos += lineStep;
			outputText(
					20,
					linePos,
					"Hi : " + to_string(weatherDataGrabber->day[thisDay].maxTemp) + "C     Lo : "
							+ to_string(weatherDataGrabber->day[thisDay].minTemp) + "C", 255, 255, blue,
					false);
			linePos += lineStep;
			outputText(
					20,
					linePos,
					"Wind : " + to_string(weatherDataGrabber->day[thisDay].windDirection) + " at "
							+ to_string(weatherDataGrabber->day[thisDay].windSpeed) + "mph", 255, 255, blue,
					false);
			linePos += lineStep;
			linePos += lineStep;
		}
	}
	SDL_Flip(screen);
}

void updateGFX()
{
	// background...
	SDL_BlitSurface(background, NULL, screen, NULL);
	// Menu
	SDL_BlitSurface(menu, NULL, screen, NULL);
	// Highlight selected menu option
	drawMenuHL();
	// footer
	outputText(5, 225, "Linux Utils " + to_string(VERSION_MAJOR) + "." + to_string(VERSION_MINOR), 0xff,
			0xff, 0xff, false);
	outputText(135, 225, "C Walker 2011, 2012", 0xff, 0xff, 0xff, false);
	SDL_UpdateRect(screen, 158, 225, 83, 15);
	// Gamma adjustment
	if (activeMenuSelection == 1)
	{
		drawMenu1Page();
	}
	// Cleanup
	if (activeMenuSelection == 2)
	{
		drawMenu2Page();
	}
	// Weather reports
	if (activeMenuSelection == 3)
	{
		drawMenu3Page();
	}
	// Update the entire window....
	SDL_Flip(screen);
}

SDL_Surface* initDisplay()
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
	{
		return NULL;
	}
	// SDL_putenv("SDL_VIDEO_CENTERED=center"); //Center the window
	char envContents[] = "SDL_VIDEO_WINDOW_POS=20,500";
	SDL_putenv(envContents);
	SDL_Surface *vidRAM;
	vidRAM = SDL_SetVideoMode(screenWidth, screenHeight, 32, SDL_NOFRAME);
	SDL_WM_SetCaption("Linux Utils - C Walker", NULL);
	return vidRAM;
}

bool outputText(int x, int y, std::string text, unsigned int r, unsigned int g, unsigned int b, bool large)
{
	SDL_Surface* txtSurface = NULL;
	SDL_Rect txtArea;
	SDL_Color convCol;
	convCol.r = r;
	convCol.g = g;
	convCol.b = b;
	txtSurface = large ? TTF_RenderText_Blended(fontFaceLarge, &text[0], convCol) : TTF_RenderText_Blended(
			fontFaceSmall, &text[0], convCol);
	if (txtSurface == NULL)
	{
		return false;
	}
	// Got text surface, update screen...
	txtArea.x = x;
	txtArea.y = y;
	txtArea.w = txtSurface->w;
	txtArea.h = txtSurface->h;
	SDL_BlitSurface(txtSurface, NULL, screen, &txtArea);
	SDL_FreeSurface(txtSurface);
	return true;
}

template<class T>
inline std::string to_string(const T& t)
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}

