/*!
* \file NodeStats.h
* \author Lars Thomas Boye 2018
*
* NodeStats handles the analysis of the node structures of Doom-engine
* maps. It is a tree structure dividing the map into smaller and smaller
* parts, until we reach sub-sectors which are simple polygons. This
* structure is used by the engine to efficiently render the map, but we
* are primarily interested in it because it allows us to calculate the
* square area of the map. So the NodeStats class processes the map lumps
* related to the nodes, and calculates map area. In addition to the
* original node format used by all the original games and any user-made
* content compatible with the original engines, a number of other node
* formats have been developed, to support larger, more complex maps and
* OpenGL engines. There is one NodeStats sub-class for each additional
* format we support. In addition there are some helper structs for
* representing and processing the node structures.
*
* Quite a bit of processing is needed to find the subsector polygons.
* The calculation of subsector polygons is based on code from
* DoomBuilder 2 by Pascal van der Heiden.
*/

#ifndef NODESTATS_H
#define NODESTATS_H

using namespace std;

//Include wxWidgets headers:
#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <vector>
#include <wx/stream.h>
#include "../TextReport.h"
#include "WadStats.h"

/*!
* A 2D point in map geometry has two signed 16-bit integers.
*/
struct Vertex {
	int16_t x; //!< X coordinate
	int16_t y; //!< Y coordinate

	Vertex(int16_t xx = 0, int16_t yy = 0) : x(xx), y(yy) {}

	/*! Inverts the sign on both coordinates. */
	Vertex inverse() { return Vertex(-x, -y); }
};

/*!
* Simple struct for storing two unsigned integers.
* Used for our representation of SEGS and SSECTORS.
* It uses 32-bit integers to fit all node formats.
*/
struct PairUint {
	uint32_t first;
	uint32_t second;

	PairUint() : first(0), second(0)
	{}
};

/*!
* A line in map geometry is defined by the vertices at either
* end. It indexes the list of Vertex objects. Vertex index
* increased to 32-bit to fit all formats.
*/
struct MapLine {
	uint32_t v1; //!< Start vertex - index in Vertex list
	uint32_t v2; //!< End vertex - index in Vertex list
	bool twoSided; //!< Two-sided line flag

	MapLine(uint32_t vv1 = 0, uint32_t vv2 = 0, bool two = false)
	: v1(vv1), v2(vv2), twoSided(two) {}
};

/*!
* A partition line for the node structure, used for calculating
* the sub-sector polygons.
*/
struct Split {
	Vertex lineStart; //!< Start coordinates of partition line
	Vertex lineDelta; //!< x,y delta to end of line

	Split(Vertex ls, Vertex ld) : lineStart(ls), lineDelta(ld) {}
};

/*!
* The nodes of the node tree are represented by this class,
* with the properties we need to calculate sub-sector polygons.
* Each node represents a partition line, splitting the area
* represented by the parent node in two. The MapNodes are kept
* in a list. A node can have two children, and these are
* referenced with list indices. The children can be nodes or
* sub-sectors, with sub-sectors being the leafs of the tree.
*/
class MapNode {
	public:
	Vertex lineStart; //!< Start coordinates of partition line
	Vertex lineDelta; //!< x,y delta to end of line
	int32_t rightIndex; //!< Right child, either NODE or SSECTOR (bit 15)
	int32_t leftIndex; //!< Left child, either NODE or SSECTOR (bit 15)

	MapNode()
	: lineStart(), lineDelta(), rightIndex(0), leftIndex(0)
	{}

	virtual ~MapNode()
	{}

	/*! true if the right child is a sub-sector (otherwise another MapNode). */
	virtual bool rightSubsector();

	/*! true if the left child is a sub-sector (otherwise another MapNode). */
	virtual bool leftSubsector();

	/*! List index of right child (check if node or sub-sector). */
	virtual int32_t rightChild();

	/*! List index of left child (check if node or sub-sector). */
	virtual int32_t leftChild();
};

/*!
* Version of MapNode for nodes which use 32 bits for the child indices,
* supporting a larger node tree.
*/
class MapNode32 : public MapNode {
	public:
	MapNode32() : MapNode()
	{}

	virtual ~MapNode32()
	{}

	virtual bool rightSubsector();

	virtual bool leftSubsector();

	virtual int32_t rightChild();

	virtual int32_t leftChild();
};

/*!
* A pair of double-precision floating-point numbers. Used for
* the points of calculated sub-sector polygons.
*/
struct Vector2D {
	double x;
	double y;

	Vector2D(double xx, double yy): x(xx), y(yy) {}
};

/*! Small number used in sub-sector calculations. */
const float EPSILON = 0.00001f;

const wxColour DRAW_BACKGROUND(255,255,255); //!< Canvas background color: White
const wxColour DRAW_FOREGROUND(0,0,0); //!< Line color: Black

/*!
* This class can draw each subsector polygon of a node tree,
* to visualize the structure.
*/
class NodeDraw
{
	public:
		/*!
		* Drawn line lengths in pixels are Doom units / scale
		* (8 or 16 recommended).
		*/
		NodeDraw(int scale);

		virtual ~NodeDraw();

		/*!
		* Called to prepare for drawing, this must give the
		* bounding box of the map to set the canvas dimensions.
		*/
		void setup(Vector2D& minXY, Vector2D& maxXY);

		/*!
		* Draw one polygon, given as a set of coordinates.
		*/
		void drawPoly(vector<Vector2D>* poly);

		/*!
		* Called once drawing is finished, to save the image to
		* file.
		*/
		void saveImage(wxString fileName);

	private:
		wxBitmap* bm;
		wxMemoryDC* dc;
		int xTrans, yTrans;
		int drawScale;
};

/*!
* Base class for NodeStats, processing the original node format.
* The main usage of any NodeStats class is to call readFile to
* read the relevant wad lumps, then computeArea if we want to
* compute the square area of the map. The class also has the same
* printReport method to output its results, which in this case is
* the number of entries in the different lumps and the area if
* this has been computed. For anything other than the original
* node format, the correct sub-class must be used to handle the
* changes in node format.
*/
class NodeStats
{
	public:
		/*!
		* Created with a pointer to the full list of vertices from
		* the corresponding VERTEXES lump. Some NodeStats-types can
		* add vertices to this list.
		*/
		NodeStats(vector<Vertex>* vert, vector<MapLine>* lin);

		virtual ~NodeStats();

		/*!
		* Returns the name of the node type handled by this NodeStats
		* implementation.
		*/
		virtual wxString getTypeLabel();

		/*!
		* true if a GL node format
		*/
		virtual bool isGL();

		/*!
		* Reads the relevant lumps from a wad file. After doing this
		* we can call computeArea.
		*/
		virtual void readFile(wxInputStream* file, vector<DirEntry*>* lumps);

		/*!
		* Performs a validity check of the node structures. Call this
		* after reading from file, if you need to check if they can be
		* used to computeArea.
		*/
		virtual bool checkNodes();

		/*!
		* Computes the total area of the map sub-sectors by building
		* their polygons from the node structure and computing the
		* area of each polygon. We specify a bounding box, discarding
		* sub-sectors outside these limits, in order to guard against
		* node errors where certain nodes lay outside map bounds. The
		* bounding box should contain all the vertices, plus a minimal
		* margin on all sides to avoid false errors.
		*/
		virtual double computeArea(Vector2D minXY, Vector2D maxXY);

		/*!
		* Outputs its contents through the TextReport interface.
		*/
		virtual void printReport(TextReport* reportView);

		/*!
		* The NodeStats must have a TaskProgress object in order to
		* run readFile.
		*/
		TaskProgress* progress;

	protected:
		virtual void processSegs(wxInputStream* file, int32_t lsize);
		virtual void processSSectors(wxInputStream* file, int32_t lsize);
		virtual void processNodes(wxInputStream* file, int32_t lsize);

		/*!
		* Top-level method for creating polygons for sub-sectors
		* based on the node tree. It is called recursively on the
		* child nodes, until reaching the sub-sector nodes, where
		* buildSubsectorPoly is invoked. The partitioning lines
		* of the nodes are stacked in the vector<Split*>.
		*/
		void recursiveBuildSubsectorPoly(int nodeIndex, vector<Split*>* splits);

		/*!
		* Creates the polygon of one sub-sector, and calculates its
		* area, adding this to the total area of the NodeStats.
		* Called from recursiveBuildSubsectorPoly, with a stack of
		* splits from traversing the node tree.
		*/
		void buildSubsectorPoly(int ss, vector<Split*>* nodesplits);

		/*!
		* The argument polygon is cropped by the split, the resulting
		* polygon returned. The argument polygon is deleted.
		*/
		vector<Vector2D>* cropPolygon(vector<Vector2D>* poly, Split* split);

		/*!
		* Checks if a line intersects with line coordinates, returning
		* intersection distance from the ray.
		*/
		float getIntersection(Vertex v1, Vertex v2, float x3, float y3, float x4, float y4);

		/*!
		* Calculates the area of a polygon. The polygon is given as a
		* vector of 2D coordinates, and these must be ordered (clockwise
		* or counter-clockwise).
		*/
		double polygonArea(vector<Vector2D>* poly);

		vector<Vertex>* vertices; //Vertices passed in to calculate polygons
		vector<MapLine>* lines; //Lines area also passed in, in case we need them
		uint32_t nodes; //Number of nodes
		vector<PairUint>* segs; //startVertex index, endVertex index
		vector<PairUint>* ssectors; //numberOfSegs, first seg index
		vector<MapNode*>* nodeList;
		double minx, maxx, miny, maxy; //For area calculation
		double area;
		int invalids; //Subsectors not used in area
		NodeDraw* draw; //If we want to draw the nodes

	private:
};

/*!
* NodeStats specialisation for the DeePBSP format. It is recognized
* by the signature 0x784E643400000000 in the NODES lump. It uses
* 32-bit instead of 16-bit values to reference vertices and segments.
*/
class DeepNodeStats : public NodeStats
{
	public:
		DeepNodeStats(vector<Vertex>* vert, vector<MapLine>* lin);
		virtual ~DeepNodeStats() {}

		virtual wxString getTypeLabel();

	protected:
		virtual void processSegs(wxInputStream* file, int32_t lsize);
		virtual void processSSectors(wxInputStream* file, int32_t lsize);
		virtual void processNodes(wxInputStream* file, int32_t lsize);
};

/*!
* NodeStats specialisation for ZDBSP uncompressed nodes. It is
* recognized by the signature "XNOD". The SEGS and SSECTORS lumps are
* left empty, and the NODES lump contains these lists in addition to
* the additional vertices needed for nodes and the node list itself.
* 32-bit values are used to refer to vertices, segments and subsectors.
*/
class ZDoomNodeStats : public NodeStats
{
	public:
		ZDoomNodeStats(vector<Vertex>* vert, vector<MapLine>* lin);
		virtual ~ZDoomNodeStats() {}

		virtual wxString getTypeLabel();

	protected:
		virtual void processSegs(wxInputStream* file, int32_t lsize);
		virtual void processSSectors(wxInputStream* file, int32_t lsize);
		virtual void processNodes(wxInputStream* file, int32_t lsize);

		virtual void zVertexes(wxInputStream* file);
		virtual uint32_t zSubsectors(wxInputStream* file);
		virtual void zSegs(wxInputStream* file, uint32_t seg);
		virtual void zNodes(wxInputStream* file);
};

/*!
* For ZDBSP compressed nodes. It is recognized by the signature
* "ZNOD". Processing of these nodes have so far not been implemented,
* so no calculations are done.
*/
class ZDoomComprNodeStats : public ZDoomNodeStats
{
	public:
		ZDoomComprNodeStats(vector<Vertex>* vert, vector<MapLine>* lin);
		virtual ~ZDoomComprNodeStats() {}

		virtual wxString getTypeLabel();

		virtual double computeArea(Vector2D minXY, Vector2D maxXY);

	protected:
		virtual void processNodes(wxInputStream* file, int32_t lsize);
};

/*!
* NodeStats specialisation for v2 GL nodes. It is recognized by
* the GL_VERT lump with the signature "gNd2". This has additional
* vertices. Other lumps are GL_SEGS, GL_SSECT and GL_NODES. Area
* computation is much simpler than with normal nodes, as each
* subsector is a complete polygon with ordered segments, so we
* don't need to process the nodes to build polygons using splits.
*/
class GLv2NodeStats : public NodeStats
{
	public:
		GLv2NodeStats(vector<Vertex>* vert);
		virtual ~GLv2NodeStats() {}

		virtual wxString getTypeLabel();

		virtual bool isGL();

		virtual void readFile(wxInputStream* file, vector<DirEntry*>* lumps);

		virtual double computeArea(Vector2D minXY, Vector2D maxXY);

	protected:
		virtual void processVert(wxInputStream* file, int32_t lsize);
		virtual void processSegs(wxInputStream* file, int32_t lsize);
		//processSSectors(wxInputStream* file, int32_t lsize) as normal
		//processNodes(wxInputStream* file, int32_t lsize) as normal, not needed

		int glVertStart;

};

/*!
* NodeStats specialisation for v5 GL nodes. It is recognized by
* the GL_VERT lump with the signature "gNd5". It is similar to
* the v2 GL format, but with 32-bit indices instead of 16-bit,
* allowing larger structures.
*/
class GLv5NodeStats : public GLv2NodeStats
{
	public:
		GLv5NodeStats(vector<Vertex>* vert);
		virtual ~GLv5NodeStats() {}

		virtual wxString getTypeLabel();

	protected:
		//processVert(wxInputStream* file, int32_t lsize) as GLv2NodeStats
		virtual void processSegs(wxInputStream* file, int32_t lsize);
		virtual void processSSectors(wxInputStream* file, int32_t lsize);
		virtual void processNodes(wxInputStream* file, int32_t lsize);
};

/*!
* NodeStats specialisation for ZDoom GL nodes. The SEGS and NODES
* lumps are unused, and the SSECTORS lumps has the signature "XGLN".
* It contains all the data, similar to the NODES lump for regular
* ZDoom nodes. For maps in the Universal Doom Map Format (UDMF),
* the data is in the ZNODES lump.
*/
class ZDoomGLNodeStats : public ZDoomNodeStats
{
	public:
		ZDoomGLNodeStats(vector<Vertex>* vert);
		virtual ~ZDoomGLNodeStats() {}

		virtual wxString getTypeLabel();

		virtual bool isGL();

		virtual void readFile(wxInputStream* file, vector<DirEntry*>* lumps);

		virtual double computeArea(Vector2D minXY, Vector2D maxXY);

	protected:
		virtual void zSegs(wxInputStream* file, uint32_t seg);
};

/*!
* For ZDoom GL compressed nodes. It is recognized by the signature
* "ZGLN", "ZGL2" or "ZGL3", for the different GL node types. Processing
* of these nodes have so far not been implemented, so no calculations
* are done.
*/
class ZDoomGLComprNodeStats : public ZDoomComprNodeStats
{
	public:
		/*! typeLabel is what to return from getTypeLabel. */
		ZDoomGLComprNodeStats(vector<Vertex>* vert, wxString typeLabel);
		virtual ~ZDoomGLComprNodeStats() {}

		virtual wxString getTypeLabel();

		virtual bool isGL();

	private:
		wxString typeStr;
};

/*!
* NodeStats specialisation for ZDoom GL2 nodes. This is used for
* UDMF maps, with the data in the ZNODES lump. It is the same as
* GLN except for the SEGS part, where each seg entry has two more
* bytes.
*/
class ZDoomGL2NodeStats : public ZDoomGLNodeStats
{
	public:
		ZDoomGL2NodeStats(vector<Vertex>* vert);
		virtual ~ZDoomGL2NodeStats() {}

		virtual wxString getTypeLabel();

	protected:
		virtual void zSegs(wxInputStream* file, uint32_t seg);
};

/*!
* NodeStats specialisation for ZDoom GL3 nodes. This is used for
* UDMF maps, with the data in the ZNODES lump. It is the same as
* GL2 except for the nodes part, where the splitter fields are
* 32-bit 16.16 fixed point numbers instead of 16-bit integers.
*/
class ZDoomGL3NodeStats : public ZDoomGL2NodeStats
{
	public:
		ZDoomGL3NodeStats(vector<Vertex>* vert);
		virtual ~ZDoomGL3NodeStats() {}

		virtual wxString getTypeLabel();

	protected:
		virtual void zNodes(wxInputStream* file);
};

#endif // NODESTATS_H
