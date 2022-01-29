/*!
* \file DecorateParser.h
* \author Lars Thomas Boye 2018
*
* Code responsible for creating ThingDef objects from Zdoom
* DECORATE scripts. ThingDef objects represent the "things"
* of Doom-engine games in our code, and is used to analyze
* maps. DECORATE is a powerful way to define things, used
* by the Zdoom engine and derivatives.
*/

#ifndef DECORATEPARSER_H
#define DECORATEPARSER_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include <string>
#include <map>
#include <wx/dir.h>
#include "TextLumpParser.h"
#include "ThingDef.h"

using namespace std;

/*!
* DECORATE defines "actors", corresponding to our ThingDefs.
* DECORATE is parsed into Actor objects. An Actor object
* contains the ThingDef we are ultimately after, plus some
* fields defining how it relates to other Actors. An actor
* is similar to a class in object-oriented programming, and
* can inherit from another actor.
*/
class Actor
{
	public:
		/*!
		* The ThingDef contains the main stats of the Actor,
		* and is the part we want for map analysis.
		*/
		ThingDef* thingDef;

		/*! Name of parent Actor, or empty string. */
		string pName;

		/*!
		* Name of actor to replace, or empty string. This is
		* used when making a custom actor to replace one of the
		* built-in things.
		*/
		string replaces;

		/*!
		* Name of actor dropped by this actor when killed (such
		* as ammo), or empty string. The ThingDef stats of the
		* dropped actor is added to the dropping actor.
		*/
		string drop;

		/*!
		* Pointer to parent Actor, set based on pName. The actor
		* inherits stats from the parent, overriding this with
		* what it redefines.
		*/
		Actor* parent;

		/*!
		* The constructor creates a new ThingDef object with the
		* given name (this is not deleted by the Actor). Initial
		* value of modified-flag is also given.
		*/
		Actor(string name, bool modified);

		/*!
		* Creates an Actor with reference to an existing ThingDef.
		*/
		Actor(ThingDef* td);

		/*!
		* Returns true if the given name matches the name of this
		* actor or any ancestor of this actor.
		*/
		bool hasParentName(string name);

		/*!
		* Used to implement the "replaces" property, replacing
		* the name and stats of the Actor's ThingDef with those
		* from the argument ThingDef. Note that the ThingDef
		* object itself is not replaced just the relevant values.
		*/
		void replaceThing(ThingDef* td);

		/*!
		* The Actor will update the given ThingDef object with
		* stats from its own ThingDef, but only where the
		* argument ThingDef is missing data. This is used to
		* fill in the ThingDef with data from an actor's parent,
		* and is called recursively on parents.
		*/
		void completeThing(ThingDef* td);

		/*!
		* A collection of operations where the Actor updates its
		* ThingDef based on the data it has after parsing, such as
		* calling completeThing on its parent.
		*/
		void completeSelf();

		/*!
		* Add health/ammo/armor counts from the given ThingDef.
		*/
		void addThingCounts(ThingDef* td);
};

/*!
* The DecorateParser parses a (set of) DECORATE text files.
* Firstly, it extracts what we need from the actor definitions,
* creating a set of Actor objects. We are typically only
* interested in ThingDef objects, which is not DECORATE-specific
* and has the data we need for map analysis, so the last step
* is to get these from the parser.
*/
class DecorateParser : public TextLumpParser
{
	public:
		DecorateParser();
		virtual ~DecorateParser();

		//parseFile from TextLumpParser

		/*! From TextLumpParser. */
		virtual void printReport(TextReport* reportView);

		/*!
		* To parse a DECORATE lump which adds/changes things
		* for an existing game, first set the existing ThingDefs
		* with this method. Then call parseFile. The parsed
		* actors can refer to the initial defs as parents or as
		* replacements. extractThingDefs will return the same
		* ThingDefList object, with an updated set of ThingDefs.
		*/
		void setInitialDefs(ThingDefList* things);

		/*!
		* Optionally, a mapping of actor names to thing IDs (DoomEdNums)
		* can be given (such a mapping may be found in MAPINFO). Actors
		* found without ID will look for an ID in this map. The parser
		* takes ownership of the map, making sure it is deleted.
		*/
		void setActorNums(map<string, uint16_t>* an);

		/*! From TextLumpParser. */
		virtual void parseFile(const wxString& filename, int32_t offset, int32_t size);

		/*!
		* Parse each file in a directory, creating Actor
		* objects for any actors found. Relationships
		* between actors are found. The parser will then
		* have a full set of decorate actors. Call
		* extractThingDefs to get the ThingDef objects from
		* the actors.
		*/
		void parseFiles(const wxString &dir);

		/*!
		* Called after we have parsed DECORATE into Actor objects.
		* This returns a ThingDefList with ThingDef objects for all
		* decorate actors with an ID number, meaning they can be
		* placed in the map (included in the THINGS lump). It also
		* includes "hard-coded" things like player starts. If
		* initiated with setInitialDefs, the same ThingDefList
		* object is used, with the new ThingDefs added to the list.
		*/
		ThingDefList* extractThingDefs();

	protected:
		/*! Parse one line of DECORATE. Calls other process functions. */
		virtual void processLine(wxString& line);

	private:
		/*! Process a line starting an actor definition, creating an Actor object. */
		ThingDef* processActor(wxString& line);

		/*! Process old format actor line, without "actor" keyword. */
		ThingDef* processOldActor(wxString& line, int start, ThingCat cat = THING_UNKNOWN);

		/*! Parse text to integer number, from start pos. until end of line. */
		uint32_t processNumber(wxString& line, int start);

		/*! Extract string from within quotes, searching for quotes from start pos. */
		string processQuoted(wxString& line, int start);

		/*! Find Actor in parser vector based on name. */
		Actor* findActor(string name);

		/*! Set parent reference in each Actor with parent name. */
		void findParents();

		/*! Add health/ammo/armor counts to actors with a named drop. */
		void findDrops();

		/*! Process "replaces" property of actors, copying ThingDef values. */
		void replaceActors();

		map<string, uint16_t>* actorNums; //!< Map from actor names to IDs (or NULL), from MAPINFO
		ThingDefList* masterList; //!< Existing ThingDefs, can be NULL
		int blockNest; //!< Keeps track of level of nesting in blocks ("{...}")
		ThingDef* currentThing; //!< ThingDef of actor currently being processed
		vector<Actor*>* actors; //!< All Actor objects parsed from DECORATE
		int actorsThing; //!< Count of actors with doomednum (placeable in map)
		int actorsOther; //!< Count of actors without doomednum
		int actorsReplace; //!< Count of actors replacing existing things
};

#endif // DECORATEPARSER_H
