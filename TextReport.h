/*!
* \file TextReport.h
* \author Lars Thomas Boye 2018
*
* An interface for text output, to be called by content objects
* and implemented by GUI or file report.
*/
#ifndef TEXTREPORT_H
#define TEXTREPORT_H

#include <wx/string.h>
#include <wx/dynarray.h>

/*!
* Simple interface for text output, allowing content objects to create
* a textual report of their content without worrying about the details
* of the formatting. Paragraphs and lines can be written, in sections
* and sub-sections with headings. The implementation should also support
* tabs to create simple tables.
*/
class TextReport
{
	public:
		/*!
		* Write text with heading formatting, naming a main section
		* of the report.
		*/
		virtual void writeHeading(wxString text) = 0;

		/*!
		* Write text with sub-heading formatting, naming a sub-section
		* of the report.
		*/
		virtual void writeSubHeading(wxString text) = 0;

		/*!
		* Basic text output, writing any text in default formatting.
		*/
		virtual void writeText(wxString text) = 0;

		/*!
		* Write single line, ended by newline. Also useful for creating
		* simple tables with tabulators, writing lines like
		* "Column 1/tColumn 2/tColumn 3".
		*/
		virtual void writeLine(wxString text) = 0;

		/*!
		* Sets tabulators as an array of distances between each tabulator
		* stop. This allows creating simple tables by writing lines with
		* '/t' characters separating the columns.
		*/
		virtual void setTabs(const wxArrayInt &tabs) = 0;
};

#endif // TEXTREPORT_H
