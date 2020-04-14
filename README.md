# Doom Map Database

Doom Map Database is an application for analysing and storing information about content for Doom-engine games. Doom in this respect being the first person shooter by id Software from 1993. During its long life, countless new levels have been created for the original Doom games, the other games based on its engine and the source ports of this engine. Having been playing Doom on and off since 1994, I wanted a way to keep track of what I have played. Combined with wanting to learn more about the Doom engine and to do something in C++, this resulted in a hobby project I tinkered with over the years.

A binary for Windows is available under [releases](https://github.com/larsboy/DMDB/releases).

![Screenshot](https://raw.githubusercontent.com/wiki/larsboy/DMDB/img/full_50.png)

## Key features
- Reads wad and pk3 files, and zips and collections of such files, producing a report on the contents.
- Analyses level data, in original, Hexen/Zdoom and UDMF formats.
- Produces gameplay statistics based on editable definitions of the map things.
- Supports Dehacked patches and Decorate, to produce correct statistics for mods.
- Key properties and stats are saved in a database, with entries for each map and each wad.
- Stores a line drawing of each map.
- Calculates square area of the maps, using the simple polygons of the BSP tree.
- You can add screenshots, descriptions, ratings and other personal content.
- Custom tags to label your map entries.
- A People table to store map authors.
- Text search, data views/filters and custom wad and map lists.
- Generates extensive statistics from the database.

## Implementation notes
One of my motivations for the project, was to make something in C++. While I am a programmer, I have never worked professionally with C/C++. I did NOT choose C++ because it was suitable for the task or something I knew how to do - rather the opposite. I wanted a traditional desktop application with windows, as I think that is perfectly suited to a database application. I chose to use wxWidgets, as it is a mature library and supports multiple platforms. I only work in Windows and so that is what the initial version is for. I have tried to be platform-independent, and so it shouldn't be too much work to get it to compile for Linux and Mac as well. I also wanted to implement as much as possible myself, so I don't use any database back end - the database implementation is all coded for this application. The core database - the wad and map entries - is very compact. On average about 100 bytes per map entry and 70 bytes per wad entry. So the complete core database should always fit in memory, even with a million maps. The analysis of wads and archives could probably be given better performance. Beware that big UDMF maps can take a long time to process, as these are all text which needs to be parsed.

If you want to look at the source code, first see my [source code notes](https://github.com/larsboy/DMDB/blob/master/NOTES.md).
