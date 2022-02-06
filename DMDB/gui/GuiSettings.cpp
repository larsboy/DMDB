/*
* GuiSettings implementation
* The ApplicationSettings constructor defines attributes for the settings,
* the other ApplicationSettings-functions are simple.
* GuiSettingsDialog has a constructor where the dialog is defined, an event
* table, and event handlers.
*/

#include "GuiSettings.h"

#include <wx/filename.h>
#include "wx/statline.h"

//************************ EntryListFields ************************

EntryListFields::EntryListFields()
: fieldCount(0), order(), modified(false)
{
	for (int i=0; i<30; i++) {
		fields[i] = MAP_END;
		width[i] = 0;
	}
}

void EntryListFields::addField(WadMapFields field, uint16_t w)
{
	if (fieldCount < 30) {
		fields[fieldCount] = field;
		width[fieldCount] = w;
		fieldCount++;
	}
}

int EntryListFields::findIndex(WadMapFields field)
{
	for (int i=0; i<fieldCount; i++) {
		if (fields[i]==field) return i;
	}
	return -1;
}

void EntryListFields::setWidth(int index, uint16_t w)
{
	if (index<fieldCount && width[index]!=w) {
		width[index] = w;
		modified = true;
	}
}

bool EntryListFields::isSame(EntryListFields* other)
{
	if (other->fieldCount != fieldCount)
		return false;
	for (int i=0; i<fieldCount; i++) {
		if (other->fields[i] != fields[i]) return false;
		if (other->width[i] != width[i]) return false;
	}
	return true;
}

//************************ ApplicationSettings ************************

ApplicationSettings::ApplicationSettings()
{
	appDir = wxGetCwd();

	//Defining boolean settings:
	flags[UI_SIDEPANEL] = defaultFlags[UI_SIDEPANEL] = true;
	flagRegKeys[UI_SIDEPANEL] = "UI_SIDEPANEL";
	flagDescriptions[UI_SIDEPANEL] = "Show side panel";

    flags[UI_LOGPANEL] = defaultFlags[UI_LOGPANEL] = false;
	flagRegKeys[UI_LOGPANEL] = "UI_LOGPANEL";
	flagDescriptions[UI_LOGPANEL] = "Show log panel";

	flags[UI_COMBOWAD] = defaultFlags[UI_COMBOWAD] = true;
	flagRegKeys[UI_COMBOWAD] = "UI_COMBOWAD";
	flagDescriptions[UI_COMBOWAD] = "Open wads in combined dialog (wad+maps)";

	flags[UI_COMBOMAP] = defaultFlags[UI_COMBOMAP] = false;
	flagRegKeys[UI_COMBOMAP] = "UI_COMBOMAP";
	flagDescriptions[UI_COMBOMAP] = "Open maps in combined dialog (wad+maps)";

	flags[DB_EDITMODE] = defaultFlags[DB_EDITMODE] = false;
	flagRegKeys[DB_EDITMODE] = "DB_EDITMODE";
	flagDescriptions[DB_EDITMODE] = "Edit mode";

	//Defining numerical settings:
	values[DB_AUTHORNAME] = defaultValues[DB_AUTHORNAME] = 0;
	minValues[DB_AUTHORNAME] = 0; maxValues[DB_AUTHORNAME] = 2; //AuthorNaming enum
	intRegKeys[DB_AUTHORNAME] = "DB_AUTHORNAME";
	intDescriptions[DB_AUTHORNAME] = "Person name preference";

	values[UI_FILETYPE] = defaultValues[UI_FILETYPE] = 0;
	minValues[UI_FILETYPE] = 0; maxValues[UI_FILETYPE] = 99;
	intRegKeys[UI_FILETYPE] = "UI_FILETYPE";
	intDescriptions[UI_FILETYPE] = "";

	values[UI_WADSORT] = defaultValues[UI_WADSORT] = WAD_TITLE;
	minValues[UI_WADSORT] = WAD_DBID; maxValues[UI_WADSORT] = WAD_END_MAP_START;
	intRegKeys[UI_WADSORT] = "UI_WADSORT";
	intDescriptions[UI_WADSORT] = "";

	values[UI_MAPSORT] = defaultValues[UI_MAPSORT] = WAD_TITLE;
	minValues[UI_MAPSORT] = WAD_DBID; maxValues[UI_MAPSORT] = MAP_END;
	intRegKeys[UI_MAPSORT] = "UI_MAPSORT";
	intDescriptions[UI_MAPSORT] = "";

	values[IMG_WIDTH] = defaultValues[IMG_WIDTH] = 640;
	minValues[IMG_WIDTH] = 160; maxValues[IMG_WIDTH] = 3440;
	intRegKeys[IMG_WIDTH] = "IMG_WIDTH";
	intDescriptions[IMG_WIDTH] = "";

	values[IMG_HEIGHT] = defaultValues[IMG_HEIGHT] = 480;
	minValues[IMG_HEIGHT] = 100; maxValues[IMG_HEIGHT] = 1600;
	intRegKeys[IMG_HEIGHT] = "IMG_HEIGHT";
	intDescriptions[IMG_HEIGHT] = "";

	//Defining path settings:
	//ThingDef files will be in same path as exe by default
	paths[TDG_DOOM] = defaultPaths[TDG_DOOM] = "doom.thg";
	pathRegKeys[TDG_DOOM] = "TDG_DOOM";
	pathDescriptions[TDG_DOOM] = "Doom (1/2, Boom)";

	paths[TDG_ZDOOM] = defaultPaths[TDG_ZDOOM] = "doom_zdoom.thg";
	pathRegKeys[TDG_ZDOOM] = "TDG_ZDOOM";
	pathDescriptions[TDG_ZDOOM] = "Doom 1/2 in ZDoom";

	paths[TDG_SKULLT] = defaultPaths[TDG_SKULLT] = "doom_skulltag.thg";
	pathRegKeys[TDG_SKULLT] = "TDG_SKULLT";
	pathDescriptions[TDG_SKULLT] = "Skulltag";

	paths[TDG_HERET] = defaultPaths[TDG_HERET] = "heretic.thg";
	pathRegKeys[TDG_HERET] = "TDG_HERET";
	pathDescriptions[TDG_HERET] = "Heretic";

	//Defining wad list fields (column width, or 0 to not show):
	wadWidths[WAD_DBID] = wadWidthDefaults[WAD_DBID] = 0;
	wadWidthKeys[WAD_DBID] = "WAD_DBID";
	wadWidths[WAD_FILENAME] = wadWidthDefaults[WAD_FILENAME] = 100;
	wadWidthKeys[WAD_FILENAME] = "WAD_FILENAME";
	wadWidths[WAD_FILESIZE] = wadWidthDefaults[WAD_FILESIZE] = 80;
	wadWidthKeys[WAD_FILESIZE] = "WAD_FILESIZE";
	wadWidths[WAD_MD5DIGEST] = wadWidthDefaults[WAD_MD5DIGEST] = 0;
	wadWidthKeys[WAD_MD5DIGEST] = "WAD_MD5DIGEST";
	wadWidths[WAD_EXTRAFILES] = wadWidthDefaults[WAD_EXTRAFILES] = 0;
	wadWidthKeys[WAD_EXTRAFILES] = "WAD_EXTRAFILES";
	wadWidths[WAD_IDGAMES] = wadWidthDefaults[WAD_IDGAMES] = 0;
	wadWidthKeys[WAD_IDGAMES] = "WAD_IDGAMES";
	wadWidths[WAD_TITLE] = wadWidthDefaults[WAD_TITLE] = 120;
	wadWidthKeys[WAD_TITLE] = "WAD_TITLE";
	wadWidths[WAD_YEAR] = wadWidthDefaults[WAD_YEAR] = 50;
	wadWidthKeys[WAD_YEAR] = "WAD_YEAR";
	wadWidths[WAD_IWAD] = wadWidthDefaults[WAD_IWAD] = 80;
	wadWidthKeys[WAD_IWAD] = "WAD_IWAD";
	wadWidths[WAD_ENGINE] = wadWidthDefaults[WAD_ENGINE] = 80;
	wadWidthKeys[WAD_ENGINE] = "WAD_ENGINE";
	wadWidths[WAD_PLAYSTYLE] = wadWidthDefaults[WAD_PLAYSTYLE] = 60;
	wadWidthKeys[WAD_PLAYSTYLE] = "WAD_PLAYSTYLE";
	wadWidths[WAD_MAPS] = wadWidthDefaults[WAD_MAPS] = 50;
	wadWidthKeys[WAD_MAPS] = "WAD_MAPS";
	wadWidths[WAD_WF_IWAD] = wadWidthDefaults[WAD_WF_IWAD] = 50;
	wadWidthKeys[WAD_WF_IWAD] = "WAD_WF_IWAD";
	wadWidths[WAD_WF_SPRITES] = wadWidthDefaults[WAD_WF_SPRITES] = 50;
	wadWidthKeys[WAD_WF_SPRITES] = "WAD_WF_SPRITES";
	wadWidths[WAD_WF_TEX] = wadWidthDefaults[WAD_WF_TEX] = 50;
	wadWidthKeys[WAD_WF_TEX] = "WAD_WF_TEX";
	wadWidths[WAD_WF_GFX] = wadWidthDefaults[WAD_WF_GFX] = 50;
	wadWidthKeys[WAD_WF_GFX] = "WAD_WF_GFX";
	wadWidths[WAD_WF_COLOR] = wadWidthDefaults[WAD_WF_COLOR] = 50;
	wadWidthKeys[WAD_WF_COLOR] = "WAD_WF_COLOR";
	wadWidths[WAD_WF_SOUND] = wadWidthDefaults[WAD_WF_SOUND] = 50;
	wadWidthKeys[WAD_WF_SOUND] = "WAD_WF_SOUND";
	wadWidths[WAD_WF_MUSIC] = wadWidthDefaults[WAD_WF_MUSIC] = 50;
	wadWidthKeys[WAD_WF_MUSIC] = "WAD_WF_MUSIC";
	wadWidths[WAD_WF_DEHBEX] = wadWidthDefaults[WAD_WF_DEHBEX] = 50;
	wadWidthKeys[WAD_WF_DEHBEX] = "WAD_WF_DEHBEX";
	wadWidths[WAD_WF_THINGS] = wadWidthDefaults[WAD_WF_THINGS] = 50;
	wadWidthKeys[WAD_WF_THINGS] = "WAD_WF_THINGS";
	wadWidths[WAD_WF_SCRIPT] = wadWidthDefaults[WAD_WF_SCRIPT] = 50;
	wadWidthKeys[WAD_WF_SCRIPT] = "WAD_WF_SCRIPT";
	wadWidths[WAD_WF_GLNODES] = wadWidthDefaults[WAD_WF_GLNODES] = 50;
	wadWidthKeys[WAD_WF_GLNODES] = "WAD_WF_GLNODES";
	wadWidths[WAD_RATING] = wadWidthDefaults[WAD_RATING] = 0;
	wadWidthKeys[WAD_RATING] = "WAD_RATING";
	wadWidths[WAD_OWNRATING] = wadWidthDefaults[WAD_OWNRATING] = 50;
	wadWidthKeys[WAD_OWNRATING] = "WAD_OWNRATING";
	wadWidths[WAD_PLAYTIME] = wadWidthDefaults[WAD_PLAYTIME] = 0;
	wadWidthKeys[WAD_PLAYTIME] = "WAD_PLAYTIME";
	wadWidths[WAD_OF_HAVEFILE] = wadWidthDefaults[WAD_OF_HAVEFILE] = 50;
	wadWidthKeys[WAD_OF_HAVEFILE] = "WAD_OF_HAVEFILE";

	//Defining map list fields (column width, or 0 to not show):
	mapWidths[0] = mapWidthDefaults[0] = 0; //MAP_DBID
	mapWidthKeys[0] = "MAP_DBID";
	mapWidths[WAD_FILENAME] = mapWidthDefaults[WAD_FILENAME] = 100;
	mapWidthKeys[WAD_FILENAME] = "WAD_FILENAME";
	mapWidths[WAD_FILESIZE] = mapWidthDefaults[WAD_FILESIZE] = 0;
	mapWidthKeys[WAD_FILESIZE] = "WAD_FILESIZE";
	mapWidths[WAD_MD5DIGEST] = mapWidthDefaults[WAD_MD5DIGEST] = 0;
	mapWidthKeys[WAD_MD5DIGEST] = "WAD_MD5DIGEST";
	mapWidths[WAD_EXTRAFILES] = mapWidthDefaults[WAD_EXTRAFILES] = 0;
	mapWidthKeys[WAD_EXTRAFILES] = "WAD_EXTRAFILES";
	mapWidths[WAD_IDGAMES] = mapWidthDefaults[WAD_IDGAMES] = 0;
	mapWidthKeys[WAD_IDGAMES] = "WAD_IDGAMES";
	mapWidths[WAD_TITLE] = mapWidthDefaults[WAD_TITLE] = 100;
	mapWidthKeys[WAD_TITLE] = "WAD_TITLE";
	mapWidths[WAD_YEAR] = mapWidthDefaults[WAD_YEAR] = 50;
	mapWidthKeys[WAD_YEAR] = "WAD_YEAR";
	mapWidths[WAD_IWAD] = mapWidthDefaults[WAD_IWAD] = 0;
	mapWidthKeys[WAD_IWAD] = "WAD_IWAD";
	mapWidths[WAD_ENGINE] = mapWidthDefaults[WAD_ENGINE] = 0;
	mapWidthKeys[WAD_ENGINE] = "WAD_ENGINE";
	mapWidths[WAD_PLAYSTYLE] = mapWidthDefaults[WAD_PLAYSTYLE] = 0;
	mapWidthKeys[WAD_PLAYSTYLE] = "WAD_PLAYSTYLE";
	mapWidths[MAP_NAME-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_NAME-MAPWIDTHS_MAPOFFSET] = 70;
	mapWidthKeys[MAP_NAME-MAPWIDTHS_MAPOFFSET] = "MAP_NAME";
	mapWidths[MAP_TITLE-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_TITLE-MAPWIDTHS_MAPOFFSET] = 120;
	mapWidthKeys[MAP_TITLE-MAPWIDTHS_MAPOFFSET] = "MAP_TITLE";
	mapWidths[MAP_BASEDON-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_BASEDON-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_BASEDON-MAPWIDTHS_MAPOFFSET] = "MAP_BASEDON";
	mapWidths[MAP_AUTHOR1-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_AUTHOR1-MAPWIDTHS_MAPOFFSET] = 100;
	mapWidthKeys[MAP_AUTHOR1-MAPWIDTHS_MAPOFFSET] = "MAP_AUTHOR1";
	mapWidths[MAP_AUTHOR2-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_AUTHOR2-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_AUTHOR2-MAPWIDTHS_MAPOFFSET] = "MAP_AUTHOR2";
	mapWidths[MAP_SINGLE-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_SINGLE-MAPWIDTHS_MAPOFFSET] = 80;
	mapWidthKeys[MAP_SINGLE-MAPWIDTHS_MAPOFFSET] = "MAP_SINGLE";
	mapWidths[MAP_COOP-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_COOP-MAPWIDTHS_MAPOFFSET] = 80;
	mapWidthKeys[MAP_COOP-MAPWIDTHS_MAPOFFSET] = "MAP_COOP";
	mapWidths[MAP_DM-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_DM-MAPWIDTHS_MAPOFFSET] = 80;
	mapWidthKeys[MAP_DM-MAPWIDTHS_MAPOFFSET] = "MAP_DM";
	mapWidths[MAP_MODE-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MODE-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_MODE-MAPWIDTHS_MAPOFFSET] = "MAP_MODE";
	mapWidths[MAP_LINEDEFS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_LINEDEFS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_LINEDEFS-MAPWIDTHS_MAPOFFSET] = "MAP_LINEDEFS";
	mapWidths[MAP_SECTORS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_SECTORS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_SECTORS-MAPWIDTHS_MAPOFFSET] = "MAP_SECTORS";
	mapWidths[MAP_THINGS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_THINGS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_THINGS-MAPWIDTHS_MAPOFFSET] = "MAP_THINGS";
	mapWidths[MAP_SECRETS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_SECRETS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_SECRETS-MAPWIDTHS_MAPOFFSET] = "MAP_SECRETS";
	mapWidths[MAP_ENEMIES-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_ENEMIES-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_ENEMIES-MAPWIDTHS_MAPOFFSET] = "MAP_ENEMIES";
	mapWidths[MAP_TOTALHP-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_TOTALHP-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_TOTALHP-MAPWIDTHS_MAPOFFSET] = "MAP_TOTALHP";
	mapWidths[MAP_AMMORAT-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_AMMORAT-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_AMMORAT-MAPWIDTHS_MAPOFFSET] = "MAP_AMMORAT";
	mapWidths[MAP_HEALTHRAT-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_HEALTHRAT-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_HEALTHRAT-MAPWIDTHS_MAPOFFSET] = "MAP_HEALTHRAT";
	mapWidths[MAP_ARMORRAT-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_ARMORRAT-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_ARMORRAT-MAPWIDTHS_MAPOFFSET] = "MAP_ARMORRAT";
	mapWidths[MAP_AREA-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_AREA-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_AREA-MAPWIDTHS_MAPOFFSET] = "MAP_AREA";
	mapWidths[MAP_LINEDEF_DENS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_LINEDEF_DENS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_LINEDEF_DENS-MAPWIDTHS_MAPOFFSET] = "MAP_LINEDEF_DENS";
	mapWidths[MAP_ENEMY_DENS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_ENEMY_DENS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_ENEMY_DENS-MAPWIDTHS_MAPOFFSET] = "MAP_ENEMY_DENS";
	mapWidths[MAP_HP_DENS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_HP_DENS-MAPWIDTHS_MAPOFFSET] = 60;
	mapWidthKeys[MAP_HP_DENS-MAPWIDTHS_MAPOFFSET] = "MAP_HP_DENS";
	mapWidths[MAP_RATING-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_RATING-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_RATING-MAPWIDTHS_MAPOFFSET] = "MAP_RATING";
	mapWidths[MAP_MF_SPAWN-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_SPAWN-MAPWIDTHS_MAPOFFSET] = 50;
	mapWidthKeys[MAP_MF_SPAWN-MAPWIDTHS_MAPOFFSET] = "MAP_MF_SPAWN";
	mapWidths[MAP_MF_MORESPAWN-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_MORESPAWN-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_MF_MORESPAWN-MAPWIDTHS_MAPOFFSET] = "MAP_MF_MORESPAWN";
	mapWidths[MAP_MF_DIFFSET-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_DIFFSET-MAPWIDTHS_MAPOFFSET] = 50;
	mapWidthKeys[MAP_MF_DIFFSET-MAPWIDTHS_MAPOFFSET] = "MAP_MF_DIFFSET";
	mapWidths[MAP_MF_VOODOO-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_VOODOO-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_MF_VOODOO-MAPWIDTHS_MAPOFFSET] = "MAP_MF_VOODOO";
	mapWidths[MAP_MF_UNKNOWN-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_UNKNOWN-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_MF_UNKNOWN-MAPWIDTHS_MAPOFFSET] = "MAP_MF_UNKNOWN";
	mapWidths[MAP_MF_SAMEAS-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_SAMEAS-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_MF_SAMEAS-MAPWIDTHS_MAPOFFSET] = "MAP_MF_SAMEAS";
	mapWidths[MAP_MF_NOTLEVEL-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_MF_NOTLEVEL-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_MF_NOTLEVEL-MAPWIDTHS_MAPOFFSET] = "MAP_MF_NOTLEVEL";
	mapWidths[MAP_OWNRATING-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_OWNRATING-MAPWIDTHS_MAPOFFSET] = 50;
	mapWidthKeys[MAP_OWNRATING-MAPWIDTHS_MAPOFFSET] = "MAP_OWNRATING";
	mapWidths[MAP_PLAYED-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_PLAYED-MAPWIDTHS_MAPOFFSET] = 50;
	mapWidthKeys[MAP_PLAYED-MAPWIDTHS_MAPOFFSET] = "MAP_PLAYED";
	mapWidths[MAP_DIFFICULTY-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_DIFFICULTY-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_DIFFICULTY-MAPWIDTHS_MAPOFFSET] = "MAP_DIFFICULTY";
	mapWidths[MAP_PLAYTIME-MAPWIDTHS_MAPOFFSET] = mapWidthDefaults[MAP_PLAYTIME-MAPWIDTHS_MAPOFFSET] = 0;
	mapWidthKeys[MAP_PLAYTIME-MAPWIDTHS_MAPOFFSET] = "MAP_PLAYTIME";

	//Defining color settings:
	//colours[COLR_BACKGROUND] = defaultColours[COLR_BACKGROUND] = wxColour(230,230,230);
	//colourRegKeys[COLR_BACKGROUND] = "COLR_BACKGROUND";
	//colourDescriptions[COLR_BACKGROUND] = "Background colour";
}

void ApplicationSettings::setDefaults()
{
	//Set all to default values:
	for (int i=0; i<ENDOF_BOOLSETELEMS; i++)
		flags[i] = defaultFlags[i];
	for (int i=0; i<ENDOF_INTSETELEMS; i++)
		values[i] = defaultValues[i];
	for (int i=0; i<ENDOF_PATHSETELEMS; i++)
		paths[i] = defaultPaths[i];
	for (int i=0; i<WAD_END_MAP_START; i++)
		wadWidths[i] = wadWidthDefaults[i];
	for (int i=0; i<ENDOF_MAPWIDTHS; i++)
		mapWidths[i] = mapWidthDefaults[i];
}

void ApplicationSettings::debug()
{
	for (int i=0; i<WAD_END_MAP_START; i++)
		wxLogVerbose(wadWidthKeys[i]);
	//for (int i=0; i<ENDOF_MAPWIDTHS; i++)
	//	wxLogVerbose(mapWidthKeys[i]);
}

void ApplicationSettings::loadConfig(wxConfigBase* conf)
{
	conf->SetPath("/AppSettings");
	//Load boolean flags:
	bool* bp = new bool(false);
	for (int i=0; i<ENDOF_BOOLSETELEMS; i++)
	{
		if (conf->Read(flagRegKeys[i],bp)) flags[i] = *bp;
		else flags[i] = defaultFlags[i];
	}
	delete bp;
	//Load integer settings:
	for (int i=0; i<ENDOF_INTSETELEMS; i++)
		values[i] = (int) conf->ReadLong(intRegKeys[i], defaultValues[i]);
	//Load string settings:
	wxString* sp = new wxString();
	for (int i=0; i<ENDOF_PATHSETELEMS; i++)
	{
		if (conf->Read(pathRegKeys[i],sp)) paths[i] = *sp;
		else paths[i] = defaultPaths[i];
	}
	delete sp;
	conf->SetPath("..");
	//Load wad list fields:
	conf->SetPath("/WadList");
	for (int i=0; i<WAD_END_MAP_START; i++)
		wadWidths[i] = (int) conf->ReadLong(wadWidthKeys[i], wadWidthDefaults[i]);
	conf->SetPath("..");
	//Load map list fields:
	conf->SetPath("/MapList");
	for (int i=0; i<ENDOF_MAPWIDTHS; i++)
		mapWidths[i] = (int) conf->ReadLong(mapWidthKeys[i], mapWidthDefaults[i]);
	conf->SetPath("..");
}

void ApplicationSettings::saveConfig(wxConfigBase* conf)
{
	conf->SetPath("/AppSettings");
	//Save boolean flags:
	for (int i=0; i<ENDOF_BOOLSETELEMS; i++)
		conf->Write(flagRegKeys[i], flags[i]);
	//Save integer settings:
	for (int i=0; i<ENDOF_INTSETELEMS; i++)
		conf->Write(intRegKeys[i], values[i]);
	//Save string settings:
	for (int i=0; i<ENDOF_PATHSETELEMS; i++)
		conf->Write(pathRegKeys[i], paths[i]);
	conf->SetPath("..");
	conf->SetPath("/WadList");
	for (int i=0; i<WAD_END_MAP_START; i++) {
		conf->Write(wadWidthKeys[i], wadWidths[i]);
	}
	conf->SetPath("..");
	//Save map list fields:
	conf->SetPath("/MapList");
	for (int i=0; i<ENDOF_MAPWIDTHS; i++)
		conf->Write(mapWidthKeys[i], mapWidths[i]);
	conf->SetPath("..");
}

void ApplicationSettings::setValue(intSetElems index, int val)
{
	if (val<minValues[index]) val=minValues[index];
	if (val>maxValues[index]) val=maxValues[index];
	values[index]=val;
}

wxString ApplicationSettings::getFullPath(pathSetElems index)
{
	if (paths[index].IsSameAs(defaultPaths[index],false))
		return wxString(appDir+"\\"+paths[index]);
	else
		return paths[index];
}

void ApplicationSettings::setWadWidth(int index, bool show)
{
	if (!show) {
		wadWidths[index] = 0;
	} else if (wadWidths[index] == 0) {
		if (wadWidthDefaults[index] > 0)
			wadWidths[index] = wadWidthDefaults[index];
		else
			wadWidths[index] = 60;
	}
	//else keep value
}

EntryListFields* ApplicationSettings::getWadFields()
{
	EntryListFields* fields = new EntryListFields();
	for (int i=0; i<WAD_END_MAP_START; i++) {
		if (wadWidths[i] > 0)
			fields->addField(WadMapFields(i), wadWidths[i]);
	}
	return fields;
}

void ApplicationSettings::setWadFields(EntryListFields* elf)
{
	for (int i=0; i<elf->fieldCount; i++) {
		wadWidths[elf->fields[i]] = elf->width[i];
	}
}

void ApplicationSettings::setMapWidth(int index, bool show)
{
	if (!show)
		mapWidths[index] = 0;
	else if (mapWidths[index] == 0) {
		if (mapWidthDefaults[index] > 0)
			mapWidths[index] = mapWidthDefaults[index];
		else
			mapWidths[index] = 60;
	}
}

EntryListFields* ApplicationSettings::getMapFields()
{
	EntryListFields* fields = new EntryListFields();
	if (mapWidths[0] > 0)
		fields->addField(MAP_DBID, mapWidths[0]);
	for (int i=WAD_FILENAME; i<=WAD_PLAYSTYLE; i++) {
		if (mapWidths[i] > 0)
			fields->addField(WadMapFields(i), mapWidths[i]);
	}
	int j;
	for (int i=MAP_NAME; i<MAP_END; i++) {
		j = i-MAPWIDTHS_MAPOFFSET;
		if (mapWidths[j] > 0)
			fields->addField(WadMapFields(i), mapWidths[j]);
	}
	return fields;
}

void ApplicationSettings::setMapFields(EntryListFields* elf)
{
	int j;
	for (int i=0; i<elf->fieldCount; i++) {
		j = elf->fields[i];
		if (j == MAP_DBID) j=0;
		else if (j >= MAP_NAME) j -= MAPWIDTHS_MAPOFFSET;
		mapWidths[j] = elf->width[i];
	}
}

/*
float ApplicationSettings::getColourFloat(colourSetElems index, int rgb)
{
	float col=0.0;
	switch (rgb)
	{
		case 0: col = float(colours[index].Red()) / 255.0; break;
		case 1: col = float(colours[index].Green()) / 255.0; break;
		case 2: col = float(colours[index].Blue()) / 255.0;
	}
	return col;
}*/

void ApplicationSettings::setValueFromString(intSetElems index, wxString s)
{
	if (index>=ENDOF_INTSETELEMS) return; //Safety measure
	//Try converting string to number:
	long number;
	if (!s.ToLong(&number))
		throw GuiError("Input must be a number");
	else
	{
		if (number<minValues[index])
			throw GuiError("Value too small, must be at least "+wxIntToString(minValues[index]));
		else if (number>maxValues[index])
			throw GuiError("Value too large, maximum is "+wxIntToString(maxValues[index]));
		else
			values[index]=int(number);
	}
}


//************************ GuiSettingsDialog ************************

GuiSettingsDialog::GuiSettingsDialog(wxWindow* parent, const wxPoint& pos, ApplicationSettings* appSet)
: wxDialog(parent, -1, "Application settings", pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	appSettings = appSet;
	notebook = new wxNotebook(this, -1, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
	wxSize intInpSize=wxSize(50,20);
	//for (int i=0; i<ENDOF_INTSETELEMS; i++)
	//	intInputs[i]="";
    for (int i=0; i<ENDOF_PATHSETELEMS; i++)
		pathFields[i]="";

	//First page: UI
	wxPanel* uiPage = new wxPanel(notebook,-1);
	wxBoxSizer* uiTopSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* ui1Sizer = new wxFlexGridSizer(0,2,10,6);

	flagBoxes[UI_SIDEPANEL] = new wxCheckBox(uiPage, -1, appSettings->flagDescriptions[UI_SIDEPANEL]);
	ui1Sizer->Add(flagBoxes[UI_SIDEPANEL],0);

	flagBoxes[UI_COMBOWAD] = new wxCheckBox(uiPage, -1, appSettings->flagDescriptions[UI_COMBOWAD]);
	ui1Sizer->Add(flagBoxes[UI_COMBOWAD],0);

	flagBoxes[UI_LOGPANEL] = new wxCheckBox(uiPage, -1, appSettings->flagDescriptions[UI_LOGPANEL]);
	ui1Sizer->Add(flagBoxes[UI_LOGPANEL],0);

	flagBoxes[UI_COMBOMAP] = new wxCheckBox(uiPage, -1, appSettings->flagDescriptions[UI_COMBOMAP]);
	ui1Sizer->Add(flagBoxes[UI_COMBOMAP],0);

	ui1Sizer->AddSpacer(1);
	ui1Sizer->AddSpacer(1);

	ui1Sizer->Add(new wxStaticText(uiPage,-1,appSettings->intDescriptions[DB_AUTHORNAME]),0,wxALIGN_CENTER_VERTICAL);
	intChoice = new wxChoice(uiPage,-1,wxDefaultPosition,wxDefaultSize,3,authorNamingSchemes);
	ui1Sizer->Add(intChoice,0,wxALIGN_CENTER_VERTICAL);

	//wxFlexGridSizer* ui2Sizer = new wxFlexGridSizer(2,4,10,6);
	/*
	wxFlexGridSizer* render3Sizer = new wxFlexGridSizer(2,4,10,6);
	colourPickers[COLR_GRIDHOR] = new wxColourPickerCtrl(renderPage, -1, *wxBLACK, wxDefaultPosition, wxDefaultSize);
	render3Sizer->Add(new wxStaticText(renderPage,-1,appSettings->colourDescriptions[COLR_GRIDHOR]), 0);
	render3Sizer->Add(colourPickers[COLR_GRIDHOR],0);
    */

    uiTopSizer->AddSpacer(10);
	uiTopSizer->Add(ui1Sizer,0,wxALL,10);
	//uiTopSizer->Add(ui2Sizer,0,wxALL,10);
	//uiTopSizer->Add(ui3Sizer,0,wxALL,10);
	uiPage->SetSizer(uiTopSizer);
	notebook->AddPage(uiPage, "UI", false);
	uiTopSizer->Fit(uiPage);
    uiTopSizer->SetSizeHints(uiPage);
    notebookPages[0] = uiPage;

	//Second page: Wad list
    wxPanel* wadPage = new wxPanel(notebook,-1);
    wxBoxSizer* wadTopSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* wadSizer = new wxFlexGridSizer(10,3,10,6);
	for (int i=0; i<WAD_END_MAP_START; i++) {
		wadBoxes[i] = new wxCheckBox(wadPage, -1, wadMapLabels[i]);
		wadSizer->Add(wadBoxes[i],0);
	}
	wadTopSizer->Add(wadSizer,0,wxALL,10);

	wadPage->SetSizer(wadTopSizer);
	notebook->AddPage(wadPage, "Wad list", false);
	wadTopSizer->Fit(wadPage);
    wadTopSizer->SetSizeHints(wadPage);
    notebookPages[1] = wadPage;

    //Third page: Map list
    wxPanel* mapPage = new wxPanel(notebook,-1);
    wxBoxSizer* mapTopSizer = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* mapSizer = new wxFlexGridSizer(12,4,10,6);
	for (int i=0; i<=WAD_PLAYSTYLE; i++) {
		mapBoxes[i] = new wxCheckBox(mapPage, -1, wadMapLabels[i]);
		mapSizer->Add(mapBoxes[i],0);
	}
	for (int i=MAP_NAME; i<MAP_END; i++) {
		mapBoxes[i-MAPWIDTHS_MAPOFFSET] = new wxCheckBox(mapPage, -1, wadMapLabels[i]);
		mapSizer->Add(mapBoxes[i-MAPWIDTHS_MAPOFFSET],0);
	}
	mapTopSizer->Add(mapSizer,0,wxALL,10);

	mapPage->SetSizer(mapTopSizer);
	notebook->AddPage(mapPage, "Map list", false);
	mapTopSizer->Fit(mapPage);
    mapTopSizer->SetSizeHints(mapPage);
    notebookPages[2] = mapPage;

    //Fourth page: ThingDefs
	wxPanel* pathPage = new wxPanel(notebook,-1);
	wxBoxSizer* pathTopSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* statText = new wxStaticText(pathPage,-1,"These files contain definitions of game things, used as a basis for computing gameplay stats. By default, these are located in the same folder as the application (see or edit the contents from the ThingDef menu). It is possible to assign other files.");
	statText->Wrap(460);
	pathTopSizer->Add(statText,0,wxALL|wxALIGN_LEFT,10);

	wxFlexGridSizer* pathSizer = new wxFlexGridSizer(4,3,10,6);
	wxTextCtrl* tc = new wxTextCtrl(pathPage,TDG_DOOM,"",wxDefaultPosition,wxSize(220,20),0,
        wxTextValidator(0,&pathFields[TDG_DOOM]));
	tc->SetEditable(false);
	pathSizer->Add(new wxStaticText(pathPage,-1,appSettings->pathDescriptions[TDG_DOOM]),0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(tc,0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(new wxButton(pathPage,LTHING_DOOM,"Change"),0);

	tc = new wxTextCtrl(pathPage,TDG_ZDOOM,"",wxDefaultPosition,wxSize(220,20),0,
        wxTextValidator(0,&pathFields[TDG_ZDOOM]));
	tc->SetEditable(false);
	pathSizer->Add(new wxStaticText(pathPage,-1,appSettings->pathDescriptions[TDG_ZDOOM]),0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(tc,0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(new wxButton(pathPage,LTHING_ZDOOM,"Change"),0);

	tc = new wxTextCtrl(pathPage,TDG_SKULLT,"",wxDefaultPosition,wxSize(220,20),0,
        wxTextValidator(0,&pathFields[TDG_SKULLT]));
	tc->SetEditable(false);
	pathSizer->Add(new wxStaticText(pathPage,-1,appSettings->pathDescriptions[TDG_SKULLT]),0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(tc,0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(new wxButton(pathPage,LTHING_SKULLT,"Change"),0);

	tc = new wxTextCtrl(pathPage,TDG_HERET,"",wxDefaultPosition,wxSize(220,20),0,
        wxTextValidator(0,&pathFields[TDG_HERET]));
	tc->SetEditable(false);
	pathSizer->Add(new wxStaticText(pathPage,-1,appSettings->pathDescriptions[TDG_HERET]),0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(tc,0,wxALIGN_CENTER_VERTICAL);
	pathSizer->Add(new wxButton(pathPage,LTHING_HERET,"Change"),0);
	pathTopSizer->Add(pathSizer,0,wxALL,10);

	pathPage->SetSizer(pathTopSizer);
	notebook->AddPage(pathPage, "ThingDefs", false);
	pathTopSizer->Fit(pathPage);
    pathTopSizer->SetSizeHints(pathPage);
    notebookPages[3] = pathPage;


	//The main dialog has a the notebook plus buttons
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(notebook,0,wxALL,10);

	//Dialog buttons (Defaults/Ok/Cancel):
	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(new wxButton(this,BUTTON_DEFAULT,"Defaults"), 0, wxALL, 10);
	buttonSizer->Add(new wxButton(this,BUTTON_OK,"Ok"), 0, wxALL, 10);
	buttonSizer->Add(new wxButton(this,BUTTON_CANCEL,"Cancel"), 0, wxALL, 10);
	mainSizer->Add(buttonSizer,0,wxALL|wxALIGN_CENTER,10);

	//Implementing the sizer:
    SetSizer(mainSizer);
    mainSizer->Fit(this); //Resize window to match mainSizer minimal size
    mainSizer->SetSizeHints(this); //Set minimal size for window based on mainSizer

	//Other possibilities for implementing the sizer:
	//SetAutoLayout(TRUE); //Call Layout() on each resize
    //Layout(); //Invoke sizer algorithm

    //Setting values from appSettings:
    setValues();
}

void GuiSettingsDialog::setValues()
{
	//Set values from settings:
	for (int i=0; i<ENDOF_BOOLSETELEMS; i++)
		flagBoxes[i]->SetValue(appSettings->getFlag( boolSetElems(i) ));
	//for (int i=0; i<ENDOF_INTSETELEMS; i++)
	//	intInputs[i] = wxIntToString(appSettings->getValue( intSetElems(i) ));
	intChoice->SetSelection(appSettings->getValue(DB_AUTHORNAME));
	for (int i=0; i<ENDOF_PATHSETELEMS; i++)
		pathFields[i] = appSettings->getPath( pathSetElems(i) );
	for (int i=0; i<WAD_END_MAP_START; i++)
		wadBoxes[i]->SetValue(appSettings->hasWadWidth(i));
	for (int i=0; i<ENDOF_MAPWIDTHS; i++)
		mapBoxes[i]->SetValue(appSettings->hasMapWidth(i));

	//input strings are put into the input fields by TransferDataToWindow()
	for (int i=0; i<SETTINGPAGES; i++)
		notebookPages[i]->TransferDataToWindow();
}

//Macro connecting events and handler functions:
BEGIN_EVENT_TABLE(GuiSettingsDialog, wxDialog)
	EVT_BUTTON(LTHING_DOOM, GuiSettingsDialog::onPathChange)
	EVT_BUTTON(LTHING_ZDOOM, GuiSettingsDialog::onPathChange)
	EVT_BUTTON(LTHING_SKULLT, GuiSettingsDialog::onPathChange)
	EVT_BUTTON(LTHING_HERET, GuiSettingsDialog::onPathChange)

	EVT_BUTTON(BUTTON_DEFAULT, GuiSettingsDialog::onDefault)
    EVT_BUTTON(BUTTON_OK, GuiSettingsDialog::onOk)
    EVT_BUTTON(BUTTON_CANCEL, GuiSettingsDialog::onCancel)
    EVT_CLOSE(GuiSettingsDialog::onClose)
END_EVENT_TABLE()

void GuiSettingsDialog::onPathChange(wxCommandEvent& event)
{
	pathSetElems pse;
	if (event.GetId() == LTHING_DOOM) pse = TDG_DOOM;
	else if (event.GetId() == LTHING_ZDOOM) pse = TDG_ZDOOM;
	else if (event.GetId() == LTHING_SKULLT) pse = TDG_SKULLT;
	else pse = TDG_HERET;

	wxFileName fname(appSettings->getFullPath(pse));
	wxFileDialog* fdlg = new wxFileDialog(this, "Select ThingDef file", fname.GetPath(),
		fname.GetFullName(), "ThingDef files (*.thg)|*.thg|All files (*.*)|*.*",
		wxFD_OPEN|wxFD_FILE_MUST_EXIST);
	if (fdlg->ShowModal() == wxID_OK) {
		appSettings->setPath(pse, fdlg->GetPath());
		pathFields[pse] = fdlg->GetPath();
		notebookPages[3]->TransferDataToWindow();
	}
	fdlg->Destroy();
}

//Event handler for Default button:
void GuiSettingsDialog::onDefault(wxCommandEvent& event)
{
	appSettings->setDefaults();
	setValues();
}

//Event handler for Ok button:
void GuiSettingsDialog::onOk(wxCommandEvent& event)
{
	//Use the validators, ending if they fail:
	for (int i=0; i<SETTINGPAGES; i++)
	{
		if (!notebookPages[i]->Validate()) return;
		if (!notebookPages[i]->TransferDataFromWindow()) return;
	}

	//Get the values of all check-boxes:
	for (int i=0; i<ENDOF_BOOLSETELEMS; i++)
	{
		appSettings->flags[i] = flagBoxes[i]->GetValue();
	}
	appSettings->values[DB_AUTHORNAME] = intChoice->GetSelection();
	//Get the values of all numerical input fields:
	bool canEnd=true;
	/*
	for (int i=0; i<ENDOF_INTSETELEMS; i++)
	{
		//Attempt giving the value to the ApplicationSettings-object:
		try {
			appSettings->setValueFromString(intSetElems(i), intInputs[i]);
		} catch (GuiError e) {
			wxMessageDialog* dlg=new wxMessageDialog(this, appSettings->intDescriptions[i]+"\n"+e.text1,
				"Invalid setting", wxOK | wxICON_ERROR);
			//Set position? dlg->Move()
			dlg->ShowModal();
			dlg->Destroy();
			canEnd=false;
		}
	}*/
	/*Get the values of all string input fields:
	for (int i=0; i<ENDOF_STRSETELEMS; i++) {
        appSettings->strings[i] = strInputs[i];
	}*/
	//Get on/off list columns
	for (int i=0; i<WAD_END_MAP_START; i++)
		appSettings->setWadWidth(i, wadBoxes[i]->GetValue());
	for (int i=0; i<ENDOF_MAPWIDTHS; i++)
		appSettings->setMapWidth(i, mapBoxes[i]->GetValue());

	/*Get the colours of all colour pickers:
	for (int i=0; i<ENDOF_COLOURSETELEMS; i++)
	{
		appSettings->colours[i] = colourPickers[i]->GetColour();
	}*/
	if (canEnd) EndModal(1);
}

//Event handler for Cancel button:
void GuiSettingsDialog::onCancel(wxCommandEvent& event)
{
	EndModal(0);
}

void GuiSettingsDialog::onClose(wxCloseEvent& event)
{
	EndModal(0);
}
