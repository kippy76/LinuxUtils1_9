#ifndef WEATHERDATA_H_
#define WEATHERDATA_H_

#include <string>
#include <pthread.h>
#include "XMLParser.h"

class WeatherData
{
	public:
		WeatherData();
		virtual ~WeatherData();
		void updateWeatherData(std::string weatherXML);
		void forecastToTerminal();
		bool isDataValid();
		struct FORECAST
		{
				std::string day;
				std::string description;
				int maxTemp; // Celsius
				int minTemp; // Celsius
				std::string windDirection;
				int windSpeed; // mph
		};
		FORECAST day[3];
	private:
		bool parseXML(std::string xmlData);
		std::string getParameter(std::string, std::string, std::string);
		std::string intToString(int value);
		static void* start_thread(void *obj);
		void getDataThread();
		pthread_t threadMethod;
		std::string weatherXML;
		bool dataGrabbed;
		XMLParser* xmlParser;
};

#endif /* WEATHERDATA_H_ */
