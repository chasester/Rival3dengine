
enum                            // hardcoded texture numbers
{
    DEFAULT_SKY = 0,
    DEFAULT_GEOM,
    NUMDEFAULTSLOTS
};

#define OCTAVERSION 33

struct octaheader
{
    char magic[4];              // "OCTA"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int lightmaps;
    int blendmap;
    int numvars;
    int numvslots;
};

#define MAPVERSION 666            // bump if map format changes, see worldio.cpp

struct mapheader
{
    char magic[4];              // "TMAP"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int blendmap;
    int numvars;
    int numvslots;
};

#define WATER_AMPLITUDE 0.4f
#define WATER_OFFSET 1.1f

enum
{
	MATSURF_NOT_VISIBLE = 0,
	MATSURF_VISIBLE,
	MATSURF_EDIT_ONLY
};

#define TEX_SCALE 16.0f

struct vertex { vec pos; bvec4 norm; vec tc; bvec4 tangent; };

#define MAX_ALLOWED_NODES std::numeric_limits<float>::digits
static class NodeMgr
{
public:
	uint addnode(node *n);
	bool addlistener(uint id, node callback, int cbType = 0);

	node *lookup(uint id);
	bool cleanup(bool killall = true);
	
	uint setmaxnodes(uint maxnodes);
	uint getmaxnodes();
private:
	uint maxnodes = MAX_ALLOWED_NODES;
	uint getnextempty();
	uint valuetoindex(uint val);
	uint indextovalue(uint val);
};

static inline float heapscore(node* n) { return n->getid(); } //allows use of heap calls in the vector::node for binary heap;

uint NodeMgr::addnode(node *n)
{
	
}