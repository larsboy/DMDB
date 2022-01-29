/*!
* \file WadStats.h
* \author Lars Thomas Boye 2018
*
* A ThingDef represents one type of "thing" found in Doom-engine
* maps, such as a monster, pickup or decoration. Each map has a
* THINGS lump, listing all the things found in the map. Each thing
* in the list is identified by an integer ID, giving its type. In
* order to analyze a map, we need to know the key attributes of
* each type of thing found in the map. Each Doom-engine game has
* its set of built-in things, and most engines allow some form of
* modification of the set of things.
*
* The ThingDef struct holds the thing attributes used for analyzing
* maps. In addition we define a set of thing categories. The
* ThingDefList class represents a container of ThingDefs, and has
* functions for loading a set of ThingDef objects from file and
* writing them to file.
*/

#ifndef THINGDEF_H
#define THINGDEF_H

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include <list>
#include <map>
#include <string>
#include <wx/stream.h>
#include <wx/wfstream.h>
#include <wx/file.h>
#include "../gui/GuiBase.h" //For GuiError

using namespace std;

/*!
* A set of categories applied to ThingDefs, giving their function
* in the game. Each ThingDef belongs to one of these categories, in
* the range THING_PLAYER1 - THING_EFFECT inclusive.
*/
enum ThingCat {
	THING_UNKNOWN, //!< For things which have no ThingDef
	THING_PLAYER1, //!< Player 1 start
	THING_COOP, //!< Coop (player 2-n) start
	THING_DM, //!< Deathmatch start
	THING_MONSTER, //!< Enemy (contributes to kill count)
	THING_FRIENDLY, //!< Actor which can fight for the player
	THING_PICKUP, //!< Pickup, other than weapon
	THING_WEAPON, //!< Weapon pickup (gives player weapon)
	THING_DECORATION, //!< Inanimate objects
	THING_EFFECT, //!< Other things, such as teleport destinations
	THING_GLLIGHT, //!< GL lighting (for OpenGL engines)
	THING_END //!< End marker (not a category)
};

//! Names of thing categories
const wxString thingCatStr[] = {
	"Unknown",
	"P1 start",
	"Coop start",
	"DM start",
	"Monster",
	"Friendly",
	"Pickup",
	"Weapon",
	"Decoration",
	"Effect",
	"GL lights"
};

/*!
* Represents one type of "thing" found in a Doom-engine game,
* with the attributes we use in map analysis. The primary
* identifier is the id field, and this corresponds to the id
* used in the THINGS lump. when analyzing a map, we find the
* ThingDef with the id corresponding to each entry in the THINGS
* lump, and use the other fields of the ThingDef to update map
* statistics.
*
* It is important to note that filling in the numerical attributes
* which defines how a thing affects overall gameplay stats is not
* always straight-forward, and there is some room for interpretation.
* For instance, how many health points is a mega-sphere? We could
* say 200, since it makes the player health 200 no matter how low it
* was before, but the player health is always above 0 when picking it
* up. The important thing is to understand how these attributes
* affect gameplay statistics, and set the values according to the
* wanted effect. hp will contribute to the total monster hit-points
* of a map for all ThingDef categories except friendly monsters,
* and thereby affect ratios. An exploding barrel may be given some hp,
* since it takes some damage before exploding, but we probably don't
* want to do so, since this gives barrels a negative effect on ammo
* and health ratios. Exploding barrels also damage monsters, so it
* could be argued they should have a positive effect on the ratios.
*/
struct ThingDef
{
	/*!
	* Primary ID of the thing type, corresponding to the number
	* found in the THINGS lump. Also known as the DoomEd number.
	*/
	uint16_t id;

	/*!
	* A human-readable name for the thing type, used to list
	* and work with ThingDefs. In map analysis we want to name
	* the types of weapon found in a map.
	*/
	string name;

	/*!
	* ID number used by Dehacked, which is the standard way to
	* modify things for the original engine (and supported by
	* most ports). This attribute is important for Dehacked support,
	* allowing us to read a Dehacked patch and apply its
	* modifications to the corresponding ThingDefs. 0 for things
	* not supported by Dehacked.
	*/
	uint16_t dehacked;

	/*!
	* Category, as defined by ThingCat enum. We may want to count the
	* number of things of each category (such as number of enemies),
	* and the category is considered when generating gameplay statistics.
	*/
	ThingCat cat;

	/*!
	* true if things of this type can create or reanimate enemies,
	* increasing the monster count of the map above the original number.
	*/
	bool spawner;

	/*!
	* Monster hit-points (health). How much each thing of this type
	* contributes to the total monster hit-point count of the map.
	* For Doom we use 1 bullet damage = 1 HP, so an Imp has 6 HP.
	* This should match the ammo attribute, so that total counts of
	* hp / ammo gives the ammo ratio of the map.
	*/
	uint32_t hp;

	/*!
	* Health pickup points - how much each thing of this type restores
	* or increases the player's health. Note that we use the scale of
	* 1 soulsphere = 100 health, which is different from the scale of hp.
	*/
	uint32_t health;

	/*!
	* Ammo pickup points - how much damage to monster HP the player can
	* do by picking up this ammo. This uses the same scale as the hp field,
	* with each bullet being 1 ammo.
	*/
	uint32_t ammo;

	/*!
	* Armor pickup points - how many armor points the player gets from
	* picking up a thing of this type.
	*/
	uint32_t armor;

	/*!
	* Flag to tag the ThingDef as new or modified. Useful to track
	* changes from user input or patch wads.
	*/
	bool modified;

	//! Constructor with name
	ThingDef(string n);

	//! Copy values from the provided ThingDef (except modified).
	void copyFrom(ThingDef* other);
};

/*!
* Wrapper for a collection of ThingDef objects, owning the objects and
* making them available in different collection objects. It can save to
* and load from file.
*/
class ThingDefList
{
	public:
	/*! Creates empty list. */
	ThingDefList();

	/*! Creates list with vector as content. */
	ThingDefList(vector<ThingDef*>* things);

	/*! Deletes all contained ThingDef objects. */
	virtual ~ThingDefList();

	/*!
	* Returns the number of ThingDef objects in the list.
	*/
	int size() { return (thingDefs==NULL)? 0: thingDefs->size(); }

	/*!
	* true if any contained ThingDef has the modified flag set.
	*/
	bool hasModifiedDefs();

	/*!
	* Set/remove the modified-flag on all contained ThingDefs.
	*/
	void setModified(bool modified);

	/*!
	* Loads a list of ThingDefs from a file, throwing a GuiError if it fails.
	*/
	void loadDefs(wxString fileName);

	/*!
	* Persists the contained ThingDefs to a file, throwing a GuiError if it fails.
	*/
	void saveDefs(wxString fileName);

	/*!
	* Access the ThingDefs through a map with ThingDef id as
	* key. Should be used read-only.
	*/
	map<int, ThingDef*>* getMapPointer();

	/*!
	* Gives access to all contained ThingDefs as an std::vector. The
	* vector continues to be owned by the ThingDefList, and can be
	* deleted by it.
	*/
	vector<ThingDef*>* exportVector();

	/*!
	* Import a new vector of ThingDefs. This replaces any existing
	* container, but does NOT delete contained ThingDefs. So existing
	* ThingDefs from this list should either be deleted through
	* exported containers or be present in the new vector.
	*/
	void importVector(vector<ThingDef*>* thingVec);

	/*!
	* Puts the ThingDef pointers into an std::list. Client code can
	* add, delete and modify ThingDefs in this list, in which case it
	* should call importFromList when done. The std::list object should
	* be deleted after import.
	*/
	list<ThingDef*>* exportToList();

	/*!
	* Called with the std::list retrieved with exportToList, to update
	* the internal state of the ThingDefList.
	*/
	void importFromList(list<ThingDef*>* thingList);

	private:
		/*! Deletes all ThingDefs and their containers. */
		void deleteContent();

	vector<ThingDef*>* thingDefs; //Master list
	map<int, ThingDef*>* thingMap; //In map with IDs
};

#endif // THINGDEF_H
