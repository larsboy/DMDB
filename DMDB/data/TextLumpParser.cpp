#include "TextLumpParser.h"

TextLumpParser::TextLumpParser()
{
}

TextLumpParser::~TextLumpParser()
{
}

void TextLumpParser::parseFile(const wxString& filename, int32_t offset, int32_t size)
{
	wxFileInputStream files(filename);
	if (!files.IsOk()) throw GuiError("Couldn't open file.");
	//wxFile* file = new wxFile(filename, wxFile::read);
	//if (!file->IsOpened()) throw GuiError("Couldn't open file.");
	//file->Seek(offset, wxFromStart);
	wxBufferedInputStream buf(files, 1024);
	buf.SeekI(offset, wxFromStart);

	char ch = 0;
	//'\r' is carriage return, and '\n' is line feed.
	//Newline in DEH is normally \r + \n
	int32_t bytesLeft = size;
	bool lineDone;
	wxString line("");
	line.Alloc(100);
	while (bytesLeft!=0) {
		lineDone = false;
		while (!lineDone) {
			ch = buf.GetC();
			if (buf.Eof()) {
				bytesLeft=0;
			} else {
				bytesLeft--;
				if (ch == '\n') {
					lineDone=true;
				} else if (ch == '\r') {
					ch = buf.Peek();
					if (ch == '\n') {
						buf.GetC();
						bytesLeft--;
					}
					lineDone=true;
				} else {
					line << ch;
				}
			}
			if (bytesLeft==0) lineDone=true;
		}
		processLine(line);
		line.Empty();
		//wxLogVerbose("Offset: %i", file->Tell());
	}
}

void TextLumpParser::printReport(TextReport* reportView)
{
	//Empty in base class
}
