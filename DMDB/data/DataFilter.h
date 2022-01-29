/*!
* \file DataFilter.h
* \author Lars Thomas Boye 2018
*
* A DataFilter is used to define which Wad or Map entries are
* included in a set of entries. The DataFilter super-class defines
* includes-methods to make the decision for an entry. Filters are
* classified as being for wads or maps, and for whether they
* are based on conditions or on a list of specific entries.
* DataListFilter is the implementation for the latter option,
* where a list of dbids are used to specify the included entries.
* Other filters are condition-based, meaning they specify boolean
* operators on wad or map fields. There is also a ComboDataFilter
* to combine multiple filters.
*
* All filter objects are created with one of the FILTER_* values
* to classify it as being intended for either wads or maps.
* DataFilter defines includes-methods for both types of entry,
* but should only be used with the intended entry type. The
* different filter classes vary in how they support the two entry
* types. Some operate on both types. Some operate only on wad
* entries, in which case it operates on the WadEntry of the
* MapEntry when used for maps (all maps belong to a wad). Some
* operate only on map entries, in which case wads are not
* supported (includes will always return false for wads).
*/

#ifndef DATAFILTER_H
#define DATAFILTER_H

#include <list>
#include "DataModel.h"

const unsigned char FILTER_WAD = 0; //!< For wads, with conditions
const unsigned char FILTER_WAD_LIST = 1; //!< List of specific wads
const unsigned char FILTER_MAP = 100; //!< For maps, with conditions
const unsigned char FILTER_MAP_LIST = 101; //!< List of specific maps

/*!
* Defines a set of boolean operators for conditions.
*/
enum DataFilterOp {
	DFOP_EQUALS,
	DFOP_MORE,
	DFOP_LESS
};

/*!
* Base class for DataFilter, defining the common interface.
* The includes-method is used to make the decision for any given entry.
* The base class does not filter, and returns true for all entries.
* DataFilter can also be named, and specifies how to sort the entries.
*/
class DataFilter
{
	public:
	/*!
	* The filter type must be specified. Use the FILTER_* constants.
	*/
	DataFilter(unsigned char t);

	virtual ~DataFilter() {}

	/*!
	* true if no filtering is implemented (includes all entries).
	*/
	virtual bool hasFilter() {return false;}

	/*!
	* true if filtering is based on a list of explicit entries.
	*/
	virtual bool isList() {return false;}

	/*!
	* Filter method, only returning true for entries to include.
	*/
	virtual bool includes(WadEntry* we) {return true;}

	/*!
	* Filter method, only returning true for entries to include.
	*/
	virtual bool includes(MapEntry* me) {return true;}

	const unsigned char type; //!< See FILTER_* constants
	wxString name; //!< To name the filter
	bool isChanged; //!< true if filter parameters are changed after creation
	WadMapFields sortField; //!< Specify how to sort the included entries
	bool sortReverse; //!< true to reverse sort direction
};

/*!
* A DataFilter composed of multiple DataFilters. Filters are
* AND-ed, so that an entry is only included if it is included
* in all contained filters.
*/
class ComboDataFilter : public DataFilter
{
	public:
	ComboDataFilter(unsigned char t);
	virtual ~ComboDataFilter() {}

	void addFilter(DataFilter* f) {filters.push_back(f);}

	/*!
	* Returns true only if ALL contained filters hasFilters.
	*/
	virtual bool hasAllFilters();

	virtual bool hasFilter();
	virtual bool isList();
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	/*! true if this or any contained filter isChanged. */
	bool hasChanges();

	/*! Set isChanged recursively on contained filters. */
	void setChanged(bool ch);

	vector<DataFilter*> filters;
};

/*!
* Numeric value filter for database ID, for either WadEntry or
* MapEntry. Since these IDs are always added in order, it is useful
* for distinguishing new from older entries.
*/
class DataIdFilter : public DataFilter
{
	public:
	DataIdFilter(unsigned char t, DataFilterOp dfop, uint32_t dbid);
	virtual ~DataIdFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	uint32_t val;
};

/*!
* Numeric value filter for wad fileSize.
*/
class FileSizeFilter : public DataFilter
{
	public:
	FileSizeFilter(unsigned char t, DataFilterOp dfop, uint32_t fileSize);
	virtual ~FileSizeFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	uint32_t val;
};

/*!
* Only includes wads with non-empty extraFiles string.
*/
class ExtraFilesFilter : public DataFilter
{
	public:
	ExtraFilesFilter(unsigned char t);
	virtual ~ExtraFilesFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);
};

/*!
* Numeric value filter for wad year.
*/
class YearFilter : public DataFilter
{
	public:
	YearFilter(unsigned char t, DataFilterOp dfop, uint16_t year);
	virtual ~YearFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	uint16_t val;
};

/*!
* Numeric value filter for iwad (values defined by IwadType enum).
* This is mainly useful with DFOP_EQUALS, but two IwadFilter
* instances can be combined for a range.
*/
class IwadFilter : public DataFilter
{
	public:
	IwadFilter(unsigned char t, DataFilterOp dfop, unsigned char iwad);
	virtual ~IwadFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for wad engine (values defined by
* EngineType enum). This is most useful with DFOP_EQUALS, but
* since higher numbers in many cases represent engines
* compatible with lower-numbered engines, a DFOP_LESS or range
* can be useful.
*/
class EngineFilter : public DataFilter
{
	public:
	EngineFilter(unsigned char t, DataFilterOp dfop, unsigned char engine);
	virtual ~EngineFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for wad playStyle. Probably only
* useful with DFOP_EQUALS.
*/
class PlayStyleFilter : public DataFilter
{
	public:
	PlayStyleFilter(unsigned char t, DataFilterOp dfop, unsigned char playStyle);
	virtual ~PlayStyleFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for number of maps in wad. This also
* works on MapEntries, using the wad the map belongs to.
*/
class MapCountFilter : public DataFilter
{
	public:
	MapCountFilter(unsigned char t, DataFilterOp dfop, unsigned char mapCount);
	virtual ~MapCountFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Flag filter for wad flags, with values defined by the
* WadFlags enum. Using one of the WadFlags (or a combination),
* only wads with this flag set are included.
*/
class WadFlagsFilter : public DataFilter
{
	public:
	WadFlagsFilter(unsigned char t, uint16_t flags);
	virtual ~WadFlagsFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	uint16_t val;
};

/*!
* Same as WadFlagsFilter, but only including wads where the
* flag is not set.
*/
class WadFlagsInvFilter : public DataFilter
{
	public:
	WadFlagsInvFilter(unsigned char t, uint16_t flags);
	virtual ~WadFlagsInvFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	uint16_t val;
};

/*!
* Numeric value filter for rating field, for either WadEntry
* or MapEntry.
*/
class RatingFilter : public DataFilter
{
	public:
	RatingFilter(unsigned char t, DataFilterOp dfop, unsigned char rating);
	virtual ~RatingFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for ownRating field, for either
* WadEntry or MapEntry.
*/
class OwnRatingFilter : public DataFilter
{
	public:
	OwnRatingFilter(unsigned char t, DataFilterOp dfop, unsigned char ownRating);
	virtual ~OwnRatingFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Flag filter for ownFlags field, with values defined by the
* OwnFlags enum. It is applied to either WadEntry or MapEntry,
* but mainly interesting for WadEntry.
*/
class OwnFlagsFilter : public DataFilter
{
	public:
	OwnFlagsFilter(unsigned char t, unsigned char flags);
	virtual ~OwnFlagsFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	unsigned char val;
};

/*!
* Same as OwnFlagsFilter, but only including wads/maps where
* the flag is not set.
*/
class OwnFlagsInvFilter : public DataFilter
{
	public:
	OwnFlagsInvFilter(unsigned char t, unsigned char flags);
	virtual ~OwnFlagsInvFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	unsigned char val;
};

/*!
* Filtering on string matches titles. It looks for the provided
* string in the title fields of either the wad or map entry, and
* only includes entries where a match is found. Not case sensitive.
*/
class TitleSearchFilter : public DataFilter
{
	public:
	TitleSearchFilter(unsigned char t, wxString str);
	virtual ~TitleSearchFilter() {}

	virtual bool hasFilter() {return searchStr.Len()>0;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	wxString searchStr;
};

/*!
* Filtering on string matches. It looks for the provided string
* in the string fields of either the wad or map entry, and only
* includes entries where a match is found. Not case sensitive.
* For maps it also searches its wad and author strings.
*/
class TextSearchFilter : public DataFilter
{
	public:
	TextSearchFilter(unsigned char t, wxString str);
	virtual ~TextSearchFilter() {}

	virtual bool hasFilter() {return searchStr.Len()>0;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);

	wxString searchStr;
};

/*!
* Only includes maps which references another map with the
* basedOn field.
*/
class BasedOnFilter : public DataFilter
{
	public:
	BasedOnFilter(unsigned char t);
	virtual ~BasedOnFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);
};

/*!
* Map filter for author. Considers both author fields,
* and looks in AuthorGroupEntries for the specific author.
*/
class AuthorFilter : public DataFilter
{
	public:
	AuthorFilter(unsigned char t, uint32_t authorId);
	virtual ~AuthorFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	bool checkAuthor(AuthorEntry* ae);

	uint32_t val;
};

/*!
* Numeric value filter for map singlePlayer field (level
* of support for singlePlayer).
*/
class SinglePlayerFilter : public DataFilter
{
	public:
	SinglePlayerFilter(unsigned char t, DataFilterOp dfop, unsigned char singlePlayer);
	virtual ~SinglePlayerFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for map cooperative field (level
* of support for cooperative play).
*/
class CooperativeFilter : public DataFilter
{
	public:
	CooperativeFilter(unsigned char t, DataFilterOp dfop, unsigned char coop);
	virtual ~CooperativeFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for map deathmatch field (level
* of support for deathmatch play). Combine it with SinglePlayerFilter
* to find maps only meant for deathmatch.
*/
class DeathmatchFilter : public DataFilter
{
	public:
	DeathmatchFilter(unsigned char t, DataFilterOp dfop, unsigned char deathmatch);
	virtual ~DeathmatchFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for map otherMode field (specifies
* non-standard play mode if larger than 0). Useful for finding
* maps supporting a specific mode, or all maps for non-standard
* modes.
*/
class OtherModeFilter : public DataFilter
{
	public:
	OtherModeFilter(unsigned char t, DataFilterOp dfop, unsigned char otherMode);
	virtual ~OtherModeFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for map area.
*/
class AreaFilter : public DataFilter
{
	public:
	AreaFilter(unsigned char t, DataFilterOp dfop, float area);
	virtual ~AreaFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	float val;
};

/*!
* Tag filter for map, including maps with a specific tag.
*/
class TagFilter : public DataFilter
{
	public:
	TagFilter(unsigned char t, uint32_t tagId);
	virtual ~TagFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	uint32_t val;
};

/*!
* Flag filter for map flags field, with values defined by the
* MapFlags enum. Using one of the MapFlags (or a combination),
* only maps with this flag set are included.
*/
class MapFlagsFilter : public DataFilter
{
	public:
	MapFlagsFilter(unsigned char t, unsigned char flags);
	virtual ~MapFlagsFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	unsigned char val;
};

/*!
* Numeric value filter for map "played" field (has values for
* beating the map at different levels).
*/
class PlayedFilter : public DataFilter
{
	public:
	PlayedFilter(unsigned char t, DataFilterOp dfop, unsigned char played);
	virtual ~PlayedFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for map difficulty field.
*/
class DifficultyFilter : public DataFilter
{
	public:
	DifficultyFilter(unsigned char t, DataFilterOp dfop, unsigned char difficulty);
	virtual ~DifficultyFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* Numeric value filter for map playTime field.
*/
class PlayTimeFilter : public DataFilter
{
	public:
	PlayTimeFilter(unsigned char t, DataFilterOp dfop, unsigned char playTime);
	virtual ~PlayTimeFilter() {}

	virtual bool hasFilter() {return true;}
	virtual bool includes(WadEntry* we) {return false;}
	virtual bool includes(MapEntry* me);

	DataFilterOp op;
	unsigned char val;
};

/*!
* A DataFilter specifying the exact subset of entries to include,
* having a list of dbids. The IDs are kept in sorted order for
* efficient processing of a list of entries ordered by ID.
*/
class DataListFilter : public DataFilter
{
	public:
	DataListFilter(unsigned char t);
	virtual ~DataListFilter();

	virtual bool hasFilter();
	virtual bool isList() {return true;}
	virtual bool includes(WadEntry* we);
	virtual bool includes(MapEntry* me);
	void addEntry(uint32_t id);
	bool removeEntry(uint32_t id);

	list<uint32_t>* entries; //!< dbids of the entries
	list<uint32_t>::iterator searchIter;
};

#endif // DATAFILTER_H
