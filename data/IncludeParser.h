/*!
* \file IncludeParser.h
* \author Lars Thomas Boye 2020
*
* A text parser, parsing lumps/files to find include statements.
*/

#ifndef INCLUDEPARSER_H
#define INCLUDEPARSER_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "TextLumpParser.h"

/*!
* The IncludeParser is a simple text parser looking for include
* statements. It is meant for resource files such as DECORATE, which
* can refer to other files with include statements, and extracts
* these references.
*/
class IncludeParser : public TextLumpParser
{
	public:
		/*!
		* If decorateFormat is true, it looks for #include, otherwise
		* just include (no hash).
		*/
		IncludeParser(bool decorateFormat);

		virtual ~IncludeParser();

		/*!
		* Returns list of strings found in include statements
		* after parsing. This is the quoted string following a
		* "#include", so each should name a file.
		*/
		vector<wxString>* GetIncludes() { return inclFiles; }

		/*!
		* Returns true if there appreas to be decorate code (other
		* than include statements) in the file.
		*/
		bool hasDecorate() { return hasDecorat; }

	protected:
		/*!
		* Called with each line of the file.
		*/
		virtual void processLine(wxString& line);

	private:
		/*! Extract string from within quotes, searching for quotes from start pos. */
		wxString processQuoted(wxString& line, int start);

		bool decFormat;
		vector<wxString>* inclFiles;
		bool hasDecorat;
};

#endif // INCLUDEPARSER_H
