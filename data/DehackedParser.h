/*!
* \file DehackedParser.h
* \author Lars Thomas Boye 2018
*
* A parser for Dehacked patches. Dehacked was originally a tool for
* modifying the Doom executable, to change a number of things related
* to game behavior. The modifications are specified in a text file
* format. More modern engines have built-in support to use such
* Dehacked patches as game configuration files, and also added their
* own extensions. We are primarily interested in extracting two
* aspects of the Dehacked data: map names and modifications to map
* "things" affecting their gameplay stats. We also count other types
* of modifications, to produce complete statistics about the contents
* of the DEH file, according to the categories defined by DehackedType.
*
* The parser has support for the different types of Dehacked entries,
* and some rudimentary support for the BEX format added by Boom,
* finding level names in this format.
*/

#ifndef DEHACKEDPARSER_H
#define DEHACKEDPARSER_H

#include <map>
#include <vector>
#include "TextLumpParser.h"
#include "ThingDef.h"
#include "../LtbUtils.h"

/*!
* Defines the different types of modifications found in
* Dehacked patches.
*/
enum DehackedType {
	DEH_THING, //!< Thing modification
	DEH_SOUND, //!< Sound entry
	DEH_FRAME, //!< Frame of animation
	DEH_POINTER,
	DEH_SPRITE,
	DEH_AMMO,
	DEH_WEAPON,
	DEH_TEXT, //!< Text entry, including map names
	DEH_END //!< End marker, used for "none"
};

/*!
* Parses Dehacked patch from file. There are three types of outputs.
* The ThingDef objects provided in the constructor are modified
* according to thing entries in the patch. Map titles found in the
* patch are kept by the parser, and can be returned with getMapTitle.
* And a comprehensive report on the patch content is written with
* the printReport method. A DehackedParser can only be used once;
* create a new instance each time.
*
* The patch can be a stand-alone file or a lump in a wad file. Each
* entry starts with a key-word such as "Thing" and "Text", and spans
* multiple lines. A blank line is assumed to separate each entry.
*/
class DehackedParser : public TextLumpParser
{
	public:
	/*!
	* Created with the original ThingDefs and optionally the name of
	* the DEH file. ThingDef objects in this list will be modified if
	* the corresponding thing is redefined in the dehacked patch
	* (check thingChanges()>0). The filename is just an identifier,
	* included in the reporting, and does not affect parsing. It can
	* be an empty string to signify the patch is not a stand-alone file.
	*/
	DehackedParser(ThingDefList* things, wxString fileNam = "");

	virtual ~DehackedParser();

	/*! From TextLumpParser, parses a Dehacked patch from file. */
	virtual void parseFile(const wxString& filename, int32_t offset, int32_t size);

	/*! From TextLumpParser. */
	virtual void printReport(TextReport* reportView);

	/*!
	* The fileName set in the constructor (may be empty string).
	*/
	wxString getFileName() { return fileName; }

	/*!
	* Get a map title, if it is defined by the parsed patch. The
	* mapName argument is the lump name, such as "MAP01". The title
	* is returned, or empty string if not found in the patch.
	*/
	string getMapTitle(string mapName);

	/*!
	* Number of map "things" changed by the parsed patch. The
	* changes have been applied to the ThingDefs.
	*/
	int thingChanges() { return changedDefs->size(); }

	protected:
		/*! Parses one line of the Dehacked patch. */
		virtual void processLine(wxString& line);

	private:
		wxString fileName; //Identifier set by constructor
		ThingDefList* thingDefs; //Full list of thingDefs
		vector<ThingDef*>* changedDefs; //Changed thingDefs, for reporting
		int typeCounts[DEH_END]; //Counts entries of each type
		map<string, string>* mapTitles;

		DehackedType currentEntry; //Currently parsed entry type, or DEH_END
		ThingDef* currentThing; //Currently parsing entry for this thingDef
		long textIndex; //For parsing text entry

		/*! Find thingDef based on dehacked number. */
		void findCurrentThing(int num);
};

#endif // DEHACKEDPARSER_H
