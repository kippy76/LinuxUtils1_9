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
#include "WeatherData.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <pthread.h>
#include <algorithm>	// For character replacement
WeatherData::WeatherData()
{
	dataGrabbed = false;
	xmlParser = new XMLParser();
}

WeatherData::~WeatherData()
{
	//pthread_join(threadMethod, NULL);
	delete xmlParser;
}

void WeatherData::updateWeatherData(std::string XML)
{
	// spawn thread, update value of dataGrabbed on completion.
	weatherXML = XML;
	//pthread_create(&threadMethod, 0, WeatherData::start_thread, this);
			getDataThread();
}

void* WeatherData::start_thread(void *obj)
{
	WeatherData* thisClass = static_cast<WeatherData*> (obj);
	thisClass->getDataThread();
	return 0;
}

void WeatherData::getDataThread()
{
	std::string xmlContents = "";
	std::string textLine = "";
	std::string com = "wget --no-proxy -qO .weather " + weatherXML;
	const char* comm = &com[0];
	system(comm);
	std::ifstream myfile(".weather");
	if (myfile.is_open())
	{
		while (myfile.good())
		{
			getline(myfile, textLine);
			xmlContents += textLine;
		}
		myfile.close();
	}
	if ((xmlContents.length() != 0) && (parseXML(xmlContents) != false))
	{
		dataGrabbed = true;
	}
}

bool WeatherData::parseXML(std::string xmlData)
{
	xmlParser->setSource(xmlData);
	try
	{
		for (int currentDay = 0; currentDay < 3; currentDay++)
		{
			std::string fullTitle = xmlParser->getTagValue("title", 3 + currentDay);
			std::string fullDescription = xmlParser->getTagValue("description", 2 + currentDay);
			// DAY NAME
			day[currentDay].day = getParameter(fullTitle, "", ":");
			// DAY DESCRIPTION
			std::string desc = getParameter(fullTitle, ": ", ",");
			day[currentDay].description = desc;
			// MIN TEMP
			std::string minTemp = getParameter(fullDescription, "Minimum Temperature: ", "");
			sscanf(&minTemp[0], "%d", &day[currentDay].minTemp);
			// MAX TEMP
			std::string maxTemp = getParameter(fullDescription, "Maximum Temperature: ", "");
			if (maxTemp.length() != 0)
			{
				sscanf(&maxTemp[0], "%d", &day[currentDay].maxTemp);
			}
			else
			{
				day[currentDay].maxTemp = day[currentDay].minTemp; // No max found!
			}
			// WIND DIRECTION
			std::string windDir = getParameter(fullDescription, "Wind Direction: ", ",");
			day[currentDay].windDirection = windDir;
			// WIND SPEED
			std::string windSpeed = getParameter(fullDescription, "Wind Speed: ", "m");
			sscanf(&windSpeed[0], "%d",	&day[currentDay].windSpeed);
		}
	}
	catch (std::exception exc)
	{
		return false;
	}
	return true;
}

std::string WeatherData::getParameter(std::string text, std::string keyValue, std::string delimit)
{
	if (delimit.length() == 0)
	{
		delimit ="° ,%";
	}
	int length = keyValue.length();
	uint start = text.find(keyValue);
	int start2 = start + length;
	uint end = text.substr(start2).find_first_of(delimit);
	if ((start != std::string::npos) && (end != std::string::npos))
	{
		return text.substr(start2, end);
	}
	return "";
}

void WeatherData::forecastToTerminal()
{
	std::string output = "";
	for (int thisDay = 0; thisDay < 3; thisDay++)
	{
		output += day[thisDay].day;
		output += "\n";
		output += "Overview: ";
		output += day[thisDay].description;
		output += "\n";
		output += "Max Temp: ";
		output += intToString(day[thisDay].maxTemp);
		output += " C";
		output += "\n";
		output += "Min Temp: ";
		output += intToString(day[thisDay].minTemp);
		output += " C";
		output += "\n";
		output += "Wind Direction: ";
		output += day[thisDay].windDirection;
		output += "\n";
		output += "Wind Speed: ";
		output += intToString(day[thisDay].windSpeed);
		output += " mph\n\n";
	}
	printf("%s", &output[0]);
}

bool WeatherData::isDataValid()
{
	return dataGrabbed;
}

std::string WeatherData::intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}
