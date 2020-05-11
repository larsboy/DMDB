/*!
* \file StatisticSet.h
* \author Lars Thomas Boye 2018
*
* A StatisticSet computes map statistics according to some
* categorization of the wads or maps. One DBStatistics object is
* created for each category, so this represents a level of
* aggregation above DBStatistics. StatisticSet is the abstract
* base class, and we have a number of implementations for
* different categorizations, such a years and authors, and
* for different types of DBStatistics.
*/

#ifndef STATISTICSET_H
#define STATISTICSET_H

#include <list>
#include <map>
#include "MapStatistics.h"
#include "WadStatistics.h"

/*!
* Abstract base class for a statistics table made up of a set of
* DBStatistics objects. Each wad or map processed is assigned to
* DBStatistics objects based on some categorization of the entries.
* Sub-classes must implement the processing for a categorization.
* This base class defines the processing methods to implement.
* It also has members to define the columns of the resulting table.
* The first column of the table is for the categorization, to
* identify each DBStatistics object. The other columns can be
* freely selected from the fields of DBStatistics.
*/
class StatisticSet
{
	public:
		/*!
		* The StatisticSet is identified by two strings,
		* one for the categorization and one to name the
		* complete set of maps or wads.
		*/
		StatisticSet(wxString category, wxString name);

		/*!
		* The destructor deletes every DBStatistics object
		* created by the StatisticSet.
		*/
		virtual ~StatisticSet();

		/*! Categorization identifier. */
		wxString getCategory() { return categ; }

		/*! A heading to name the StatisticSet, based on category and set name. */
		wxString getHeading() { return setName+" by "+categ; }

		/*!
		* As with DBStatistics, the entries to process are either
		* WadEntries (this method) or MapEntries.
		*/
		virtual void processWad(WadEntry* wadEntry);

		/*!
		* To compute statistics from a set of MapEntry objects,
		* call this with each map.
		*/
		virtual void processMap(MapEntry* mapEntry) = 0;

		/*!
		* This must be called once all maps or wads have been
		* processed, to make final computations. The list of
		* DBStatistics objects will then be ready.
		*/
		virtual void computeResults() = 0;

		/*!
		* Sort the list of DBStatistics based on one of the
		* fields. The field is specified with an index in the
		* field array, with 0 being the category field.
		*/
		void sort(int field);

		/*!
		* Retrieve the list of DBStatistics objects after
		* processing is done.
		*/
		list<DBStatistics*>* getStatistics() { return statList; }

		StatFields fields[30]; //!< Fields to show, in this order
		uint16_t width[30]; //!< Column width of each field
		unsigned char fieldCount; //!< Number of fields specified

		/*!
		* The fields and width arrays should be given their content
		* through this method, to define a table for showing the
		* statistics in a user interface. The first field is always
		* used for the categorization, so only the width is
		* significant in the first call of addField. Subsequent calls
		* add DBStatistics fields to include in the table.
		*/
		void addField(StatFields field, uint16_t w);

	protected:
		wxString categ; //Categorization identifier
		wxString setName; //Names the set of maps
		list<DBStatistics*>* statList;
		bool (*comp)(DBStatistics*, DBStatistics*); //Sort method

	private:
};

/*!
* StatisticSet categorizing maps according to year, for a table
* with a year column.
*/
class YearStatSet : public StatisticSet
{
	public:
		YearStatSet(wxString setName);
		virtual ~YearStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		map<int, MapStatistics*>* statMap; //In map with year
};

/*!
* StatisticSet categorizing maps according to the iwad field
* of WadEntry (which iwad the wad targets, as defined by
* IwadType enum).
*/
class IwadStatSet : public StatisticSet
{
	public:
		IwadStatSet(wxString setName);
		virtual ~IwadStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		map<int, MapStatistics*>* statMap; //In map with iwad
};

/*!
* StatisticSet categorizing maps according to the engine field
* of WadEntry (which engine the wad targets, as defined by
* EngineType enum).
*/
class EngineStatSet : public StatisticSet
{
	public:
		EngineStatSet(wxString setName);
		virtual ~EngineStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		map<int, MapStatistics*>* statMap; //In map with engine
};

/*!
* StatisticSet categorizing maps according to rating (ownRating
* field of MapEntry). While the rating value is stored with a
* range 0-100, this divides this by ten and considers the
* categories 0-10, plus not rated (value 255), for a total of
* 12 possible categories.
*/
class RatingStatSet : public StatisticSet
{
	public:
		RatingStatSet(wxString setName);
		virtual ~RatingStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		map<int, MapStatistics*>* statMap; //In map with rating
};

/*!
* StatisticSet categorizing maps according to difficulty
* rating. The difficulty rating scale has four values, in
* addition to "not rated".
*/
class DifficultyStatSet : public StatisticSet
{
	public:
		DifficultyStatSet(wxString setName);
		virtual ~DifficultyStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		map<int, MapStatistics*>* statMap; //In map with difficulty
};

/*!
* StatisticSet categorizing maps according to the playStyle
* field of WadEntry (2.5D, 3D, Special).
*/
class PlaystyleStatSet : public StatisticSet
{
	public:
		PlaystyleStatSet(wxString setName);
		virtual ~PlaystyleStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		map<int, MapStatistics*>* statMap; //In map with playstyle
};

/*!
* StatisticSet categorizing maps according to author. A map with a
* MapStatistics object for each known author must be provided, with
* author dbid as key. Note that a map can have two authors, in which
* case it is counted in two MapStatistics objects. Authors in a
* group is not considered, so if a map has group "a" as author,
* which consists of people x, y and z, it is only counted for "a",
* not for x, y or z.
*/
class AuthorStatSet : public StatisticSet
{
	public:
		AuthorStatSet(wxString setName, map<int, MapStatistics*>* authorStats);
		virtual ~AuthorStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		MapStatistics* unknown;
		map<int, MapStatistics*>* statMap; //In map with author dbid
};

/*!
* StatisticSet categorizing maps according style tags. A map with a
* MapStatistics object for each known tag must be provided, with
* tag dbid as key. Note that a map can have multiple tags, and is
* counted in the MapStatistics object of each tag.
*/
class TagStatSet : public StatisticSet
{
	public:
		TagStatSet(wxString setName, map<int, MapStatistics*>* tagStats);
		virtual ~TagStatSet();

		virtual void processMap(MapEntry* mapEntry);

		virtual void computeResults();

	private:
		MapStatistics* none;
		map<int, MapStatistics*>* statMap; //In map with tag dbid
};

/*!
* StatisticSet categorizing wads according to year, for a table
* with a year column.
*/
class YearWadStatSet : public StatisticSet
{
	public:
		YearWadStatSet(wxString setName);
		virtual ~YearWadStatSet();

		virtual void processWad(WadEntry* wadEntry);

		virtual void processMap(MapEntry* mapEntry) {}

		virtual void computeResults();

	private:
		map<int, WadStatistics*>* statMap; //In map with year
};

/*!
* StatisticSet categorizing wads according to the iwad field
* (which iwad the wad targets, as defined by IwadType enum).
*/
class IwadWadStatSet : public StatisticSet
{
	public:
		IwadWadStatSet(wxString setName);
		virtual ~IwadWadStatSet();

		virtual void processWad(WadEntry* wadEntry);

		virtual void processMap(MapEntry* mapEntry) {}

		virtual void computeResults();

	private:
		map<int, WadStatistics*>* statMap; //In map with iwad
};

/*!
* StatisticSet categorizing wads according to the engine field
* (which engine the wad targets, as defined by EngineType enum).
*/
class EngineWadStatSet : public StatisticSet
{
	public:
		EngineWadStatSet(wxString setName);
		virtual ~EngineWadStatSet();

		virtual void processWad(WadEntry* wadEntry);

		virtual void processMap(MapEntry* mapEntry) {}

		virtual void computeResults();

	private:
		map<int, WadStatistics*>* statMap; //In map with engine
};

/*!
* StatisticSet categorizing wads according to rating (ownRating
* field of WadEntry). While the rating value is stored with a
* range 0-100, this divides this by ten and considers the
* categories 0-10, plus not rated (value 255), for a total of
* 12 possible categories.
*/
class RatingWadStatSet : public StatisticSet
{
	public:
		RatingWadStatSet(wxString setName);
		virtual ~RatingWadStatSet();

		virtual void processWad(WadEntry* wadEntry);

		virtual void processMap(MapEntry* mapEntry) {}

		virtual void computeResults();

	private:
		map<int, WadStatistics*>* statMap; //In map with rating
};

#endif // STATISTICSET_H
