/*!
* \file MapinfoParser.h
* \author Lars Thomas Boye 2018
*
* A parser for the MAPINFO lump used by Hexen and ZDoom. We are
* interested in the map names defined in this lump, and possibly
* also music tracks.
*/

#ifndef MAPINFOPARSER_H
#define MAPINFOPARSER_H

#include <map>
#include <wx/file.h>
#include "TextLumpParser.h"

using namespace std;

/*!
* Parses MAPINFO lump from file. Based on TextLumpParser, with
* main parser method parseFile. Map titles found are kept by the
* parser, and can be returned with getMapTitle. A report on the
* content is written with the printReport method. Music lumps
* referenced in MAPINFO are also kept, letting us check if a lump
* found in the wad is a music track.
*/
class MapinfoParser : public TextLumpParser
{
	public:
	MapinfoParser();
	virtual ~MapinfoParser();

	//parseFile from TextLumpParser

	/*! From TextLumpParser. */
	virtual void printReport(TextReport* reportView);

	/*!
	* Get a map title, if it is defined in the parsed MAPINFO. The
	* mapName argument is the lump name, such as "MAP01". The title
	* is returned, or empty string if not found.
	*/
	string getMapTitle(string mapName);

	/*!
	* Returns true if the lump name is found in a music-entry in MAPINFO,
	* meaning it is a music track.
	*/
	bool isMusic(string lumpName);

	/*!
	* MAPINFO can specify ID numbers for actors, mapping the names of
	* actors (otherwise defined in DECORATE) to the numbers used to
	* identify them in map data. NULL if not found. If non-NULL, this
	* must be given to the Decorate parser, and later deleted.
	*/
	map<string, uint16_t>* exportDoomEdNums();

	protected:
		/*! Parses one line of the MAPINFO lump. */
		virtual void processLine(wxString& line);

	private:
		map<string, string>* mapTitles;
		vector<wxString>* music;
		map<string, uint16_t>* doomEdNums;
		int blockNest; //So far used for DoomEdNums block
		int actors;
};

#endif // MAPINFOPARSER_H
