#include "IncludeParser.h"

IncludeParser::IncludeParser(bool decorateFormat)
: decFormat(decorateFormat), hasDecorat(false)
{
	inclFiles = new vector<wxString>();
}

IncludeParser::~IncludeParser()
{
}

void IncludeParser::processLine(wxString& line)
{
	line.Trim(false);
	//line.MakeLower();
	if (decFormat) {
		if (line.StartsWith("#") && (line.Len()>9)) {
			if (line.Mid(1,7).IsSameAs("include", false)) {
				wxString file = processQuoted(line, 8);
				if (file.Len()>0)
					inclFiles->push_back(file);
			}
		} else if ((!hasDecorat) && line.StartsWith("actor")) {
			hasDecorat = true;
		}
	} else {
		if (line.StartsWith("include")) {
			wxString file = processQuoted(line, 8);
			if (file.Len()>0)
				inclFiles->push_back(file);
		}
	}
}

wxString IncludeParser::processQuoted(wxString& line, int start)
{
	if (start >= line.Len())
		return "";
	int i1 = line.find('\"', start);
	int i2 = line.find('\"', i1+1);
	if (i1==string::npos || i2==string::npos)
		return "";
	return line.SubString(i1+1, i2-1);
}
