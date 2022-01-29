/*!
* \file TextLumpParser.h
* \author Lars Thomas Boye 2018
*
* Base class for text lump parsers, reading (part of) a file and
* parsing line by line.
*/

#ifndef TEXTLUMPPARSER_H
#define TEXTLUMPPARSER_H

#include <wx/file.h>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include "../gui/GuiBase.h"
#include "../TextReport.h"

/*!
* Partially abstract base class for text lump parsers. The main parse
* method parseFile reads (part of) a file and calls the internal
* method processLine to parse one line at a time. This method is
* implemented by each type of parser.
*/
class TextLumpParser
{
	public:
	TextLumpParser();
	virtual ~TextLumpParser();

	/*!
	* Main parse method, which reads the file and calls processLine
	* to parse each line. filename is the full path of the file. This
	* can be a stand-alone file (offset 0) or the text can be a lump
	* in a wad file, in which case the offset must point to the start
	* of the lump. size is the number of bytes to parse (lump size),
	* or -1 to parse until the end of file.
	*/
	virtual void parseFile(const wxString& filename, int32_t offset, int32_t size);

	/*!
	* To output parsed data through the TextReport interface.
	*/
	virtual void printReport(TextReport* reportView);

	protected:

	/*! Called for each line of text, to parse it. */
	virtual void processLine(wxString& line) = 0;

	private:
};

#endif // TEXTLUMPPARSER_H
