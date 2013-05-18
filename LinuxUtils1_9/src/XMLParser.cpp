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
#include "XMLParser.h"
#include <string>
#include <fstream>
#include <stack>

XMLParser::XMLParser()
{
}

XMLParser::~XMLParser()
{
}

bool XMLParser::setSource(std::string xmlText)
{
	xmlContents = xmlText;
	if (xmlContents.length() == 0)
	{
		return false;
	}
	return true;
}

bool XMLParser::setSource(std::ifstream* xmlFile)
{
	xmlContents = "";
	std::string textLine = "";
	if (xmlFile->is_open())
	{
		while (xmlFile->good())
		{
			getline(*xmlFile, textLine);
			xmlContents += textLine;
		}
		xmlFile->close();
	}
	else
	{
		return false; // error loading file
	}
	if (xmlContents.length() == 0)
	{
		return false;
	}
	return true;
}

/*
 * This function grabs the contents of the 'nth' XML tag.
 * If no value given for 'nth', the first occurrence will be returned.
 * Returns an empty string if not found, or if no source has been set.
 *
 */
std::string XMLParser::getTagValue(std::string xmlTag, unsigned int nth)
{
	std::string tagValue = "";
	std::string openTag = "<" + xmlTag + ">";
	if (xmlContents.length() == 0)
	{
		return "";
	}
	unsigned int cursorLocA = 0;
	unsigned int cursorLocB = 0;
	for (int thisTag = nth; thisTag > 0; thisTag--)
	{
		cursorLocA = xmlContents.find(&openTag[0], cursorLocA, (2 + xmlTag.length()));
		if (thisTag != 1)
		{
			cursorLocA++;
		}
	}
	if (cursorLocA == std::string::npos)
	{
		// failed to find closing tag, reached EOF
		return "";
	}
	cursorLocA = cursorLocA + (2 + xmlTag.length()); // got start of data
	// Find closing tag, or another opening tag position...
	cursorLocB = xmlContents.find("<", cursorLocA, 1);

	try
	{
		tagValue = xmlContents.substr(cursorLocA, cursorLocB - cursorLocA);
	}
	catch (std::exception exc)
	{
		tagValue = "";
	}
	return tagValue;
}

int XMLParser::getTagCount(std::string xmlTag)
{
	std::string tagToCount = "<" + xmlTag + ">";
	unsigned int cursorPos = 0;
	int tagCount = 0;
	do
	{
		cursorPos = xmlContents.find(&tagToCount[0], cursorPos, (2 + xmlTag.length()));
		if (cursorPos == std::string::npos)
		{
			break;
		}
		cursorPos++;
		tagCount++;
	}
	while (true);
	return tagCount;
}

/*
 * return 0 if error, 1 otherwise.
 */
bool XMLParser::validateXML(std::string& errorMessage)
{
	bool error = false;
	if (xmlContents.length() == 0)
	{
		return 0;
	}
	std::stack<std::string> tagStack;
	unsigned int cursorLocA = 0;
	unsigned int cursorLocB = 0;
	std::string currentTag = "";
	do
	{
		try
		{
			cursorLocA = xmlContents.find("<", cursorLocB, 1);
			cursorLocB = xmlContents.find(">", cursorLocA, 1);
			if (cursorLocB == std::string::npos)
			{
				// end of file.
				break;
			}
			currentTag = xmlContents.substr(cursorLocA, (cursorLocB - cursorLocA) + 1);
			// First, carry on if tag is DTD or self closing
			if ((currentTag.substr(0, 2) == "<?") || (currentTag.substr(currentTag.length() - 2, 2) == "/>"))
			{
				continue;
			}
			if ((currentTag.substr(0, 1) == "<") && (currentTag.substr(0, 2) != "</"))
			{
				// open tag found
				tagStack.push(currentTag);
			}
			if (currentTag.substr(0, 2) == "</")
			{
				// closing tag found
				if (currentTag.substr(2, currentTag.length() - 2) == tagStack.top().substr(1,
						tagStack.top().length() - 1))
				{
					// okay, they match. Pop off opening tag
					tagStack.pop();
				}
				else
				{
					errorMessage = "Expected closing tag for " + tagStack.top() + ", but found " + currentTag
							+ ".";
					error = true;
				}
			}
		}
		catch (std::exception exc)
		{
			errorMessage = "An unknown error occurred.";
			error = true;
		}
	}
	while (!error);
	if (tagStack.size() == 0)
	{
		return true; // all okay
	}
	if (errorMessage.length() == 0) // check no existing errors....
	{
		errorMessage = tagStack.top() + " lacks closing tag.";
	}
	for (unsigned int emptyStack = 0; emptyStack < tagStack.size(); emptyStack++)
	{
		tagStack.pop();
	}
	return false; // almost okay, stack not empty though so a closing tag missing?
}

void XMLParser::hierarchyToTerminal()
{
	int level = 0;
	bool error = false;
	if (xmlContents.length() == 0)
	{
		return;
	}
	std::stack<std::string> tagStack;
	unsigned int cursorLocA = 0;
	unsigned int cursorLocB = 0;
	std::string currentTag = "";
	do
	{
		try
		{
			cursorLocA = xmlContents.find("<", cursorLocB, 1);
			cursorLocB = xmlContents.find(">", cursorLocA, 1);
			if (cursorLocB == std::string::npos)
			{
				// end of file.
				break;
			}
			currentTag = xmlContents.substr(cursorLocA, (cursorLocB - cursorLocA) + 1);
			// First, carry on if tag is DTD or self closing
			if ((currentTag.substr(0, 2) == "<?") || (currentTag.substr(currentTag.length() - 2, 2) == "/>"))
			{
				outputNode(currentTag, level);
				continue;
			}
			if ((currentTag.substr(0, 1) == "<") && (currentTag.substr(0, 2) != "</"))
			{
				// open tag found
				tagStack.push(currentTag);
				outputNode(currentTag, level);
				level++;
			}
			if (currentTag.substr(0, 2) == "</")
			{
				// closing tag found
				if (currentTag.substr(2, currentTag.length() - 2) == tagStack.top().substr(1,
						tagStack.top().length() - 1))
				{
					// okay, they match. Pop off opening tag
					tagStack.pop();
					level--;
					outputNode(currentTag, level);
				}
				else
				{
					printf("Expected closing tag for %s, but found %s.", &tagStack.top()[0], &currentTag[0]);
					error = true;
				}
			}
		}
		catch (std::exception exc)
		{
			printf("An unknown error occurred.");
			error = true;
		}
	}
	while (!error);
	if (!error) // check no existing errors....
	{
		printf("%s lacks closing tag.", &tagStack.top()[0]);
	}
	for (unsigned int emptyStack = 0; emptyStack < tagStack.size(); emptyStack++)
	{
		tagStack.pop();
	}
	return; // almost okay, stack not empty though so a closing tag missing?
}

void XMLParser::outputNode(std::string nodeName, int hierLevel)
{
	for (int indent = 0; indent < 4 * hierLevel; indent++)
	{
		printf(" ");
	}
	printf("%s", &nodeName[0]);
	printf("\n");
}
