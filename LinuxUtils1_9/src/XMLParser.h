#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include <string>

class XMLParser
{
	public:
		XMLParser();
		virtual ~XMLParser();
		bool setSource(std::string xmlText);
		bool setSource(std::ifstream* xmlFile);
		std::string getTagValue(std::string xmlTag, unsigned int nth = 1);
		int getTagCount(std::string xmlTag);
		bool validateXML(std::string& errorMessage);
		void hierarchyToTerminal();
	private :
		std::string xmlContents;
		void outputNode(std::string nodeName, int hierLevel);
};

#endif /* XMLPARSER_H_ */
