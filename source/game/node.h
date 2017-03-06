//list of class defines
struct bulletobj;
struct ridgidbody;
struct editnode;
struct node;
struct scene;
struct world;
struct scriptinterface;
struct savenode;
struct light;
//outside structs that need defined so we can use them 
struct selinfo;
struct model;
struct cube;
struct vtxarray;
//list includes
#include "AJMPhys.h"

//list of externs and global variables
#define FROM_SAUER_VEC(sauervec) ( btVector3(sauervec.x/SAUER_FACTOR, sauervec.z/SAUER_FACTOR, sauervec.y/SAUER_FACTOR) )
#define TO_SAUER_VEC(sauervec, btvec) { sauervec.x = btvec.x()*SAUER_FACTOR; sauervec.y = btvec.z()*SAUER_FACTOR; sauervec.z = btvec.y()*SAUER_FACTOR; }
enum NODE_FLAGS
{
	NODE_ACTIVE = 1 << 0, //is the node active, is it sleeping or is it dead?
	NODE_SERIALIZED = 1 << 1, //is this node serializing its self or should we delete it on reset
	NODE_START_ON_RESET = 1 << 2 //Should we let this item sleep or should we run the onstart as soon as we call a reset
};

//class declarations
struct ridgidbody // tempclass for setting up bullet, this gets deleted and converted into btRidgidBody which is used for updating entities
{
	float friction, weight, restitution, ldamp, adamp;
	vec afactor, lfactor, gravity;
	int type;
	//bool staticobj; // set weight to 0 or less to make an object static
	vec radius, center, rot, o;

	ridgidbody();
	void aabbfrommodel(int index); //moved to AJPhyis.cpp due to model not being inited yet
};
struct bulletobj
{
	btRigidBody *btridgidbody;  //change to a vector later
	ridgidbody *tempbody; //stored object data
	bulletobj() : btridgidbody(NULL), tempbody(NULL){}
	void move(vec im);
	void rotate(vec im);
	void moveto(vec _o);
	~bulletobj(){ if(btridgidbody)delete btridgidbody; if(tempbody) delete tempbody; }
};


struct scriptinterface
{
	asIScriptObject *object;
	CSerializedValue *saveData;
	int typeID;

	scriptinterface(asIScriptObject *obj);
	bool restore();
	void save();
	void setData(str name, void *val, size_t size);
	void adddata(CSerializedValue *val);
};

struct node
{
	friend world;
	asILockableSharedBool *weakRefFlag;
	uint parent;
	vector<uint> children;
	str name, tags; //instance name, tag to classify data, set on a per object basis, allows it to search by these break tags up by space
	int refcount; //for weak reference
	uint id;
	vector<asIScriptObject *> ctrl; //link to the script functions on* funcitons
	// position or start origin of the object
	bulletobj *b;
	vec o, rot, radius; //readius should not be changed, this is used to get a general idea of the bounds of the visual reprentation
	vec old_o, old_rot; //store the data before we move it
	CSerializedValue *c; //stored game data for said object;
	//editnode *en;
	void store();
	bool restore();
private:
	node() {}
	node(const node &n) {};
	node(vec pos, str name = "unnamed", str tags = "");
	node(vec pos, vec rotation);
public:
	int addref();
	int release();

	asILockableSharedBool *getweakflagref();
	//void seteditbox(vec &rad);
	void seteditbox(int i);
	void updatefrombullet();
	void doupdate();
	void doawake();
	void dorender();
	void restart();
	uint getid() { return id; }
	void move(vec impulse);
	void moveto(vec _o);
	void rotate(vec impulse, bool degrees = true);
	void rotateto(vec rot, bool degrees = true);
	void rotateto(float impy, float impp, float impr, bool deg = true);
	void rotate(float yaw, float pitch, float roll, bool deg = true);
	void setfriction(float f);
	void setelasticity(float e);
	void setgravity(float g);
	void setupbulletfrommodel(int indx);
	void setupbulletfrommodel(str name);
	void setupbulletfromaabb(vec offset, vec size, float friction = 0.3f, float elasticity = 0.8f); // add more properties later
	void setupbulletfromcylender(vec offset, float radius, float height, float friction = 0.3f, float elasticity = 0.8f);
	~node();
};

struct soundemit
{
	uint index;
	uint vol;
	uint id;
};

enum SCENE_FLAGS
{
	SCENE_NO_RENDER			=	1 << 0, //dont render this scene; set when scene is not in the view fustrum
	SCENE_NO_UPDATE			=	1 << 1, //dont update this scene; set when this part of the world is to far from the player to update
	SCENE_WANTS_LOAD		=	1 << 2, //**This scene wants to load when there is time left over; set this when scene is close enough to back load;
	SCENE_WANTS_NO_UPDATE	=	1 << 3, //**This scene wants to stop being active; Set this when player is leaving the area Well convinute to check the players position to make sure that he doesnt come back then will move to SCENCE_NOUPDATE
	SCENE_SLOW_UPDATE		=	1 << 4, //**This scene is not in top priority to update, update this scene as allowed by the extra processing time. This will spread the update over multipule frames. You should turn Physics off and work off of waypoints or similar system
	SCENE_NO_PHYSICS		=	1 << 5 //**Turns off PHysics calls should rely on prediction. DO this when player is out of the Scene but Scene still needs basic updating. Should be set with SCENE_SLOWUPDATE to use optimization. Wait till all objects are island sleep if Physics less update is not needed (ie say a puzzle game)
	//** we will set these up later when we get a better framwork
};

struct scene 
{
public:
	vector<node *> nodes;
	uchar id;
	str name;
	char flags; //uses SCENEFLAGS
	//physics handled on the world level
	vector<light *> slights;
	vector<node *> &getnodes(){ return nodes; }
	const vector<light *> getlights() 
	{
		if (flags |= SCENE_NO_RENDER) slights.deletecontents();
		return slights;
	}
	//	void pausescene(short force = -1){ paused = force == -1 ? !paused : bool(force); }
	bool ispaused(){ return false; }
	void addobject(node *gn) { if (!gn) return; nodes.add(gn); }
	void clearworld()
	{
		nodes.deletecontents();
		//add in the load from file here
		//read and load nodes
	}
	void resetworld()
	{
		int a = nodes.length();
		vector<node*> nd;
		print(str("helo world"));
		loopi(a)
		{
			node *g = nodes[i];
			if (!g->restore())
			{
				delete g;
			}
			else{
				nd.add(g);
				//g->restart();
			}

		}
		asScript->serializer->ClearRoot();
		nodes = nd;
	}
	void preloadentities(); //load all the enties at gamestart get this list from the map file
	void doawake()
	{
		loopv(nodes){ nodes[i]->doawake();}
		
	}
	void updatenodes()
	{
		loopv(nodes)nodes[i]->updatefrombullet();
		loopv(nodes) nodes[i]->doupdate();
	}
	void rendernodes()
	{
		loopv(nodes) { nodes[i]->dorender(); }
		//loopv(nodes) { nodes[i]->dogui(); } // do gui calls after may need to do this in a different function call
	}
	void store()
	{
		loopv(nodes) nodes[i]->store();
	}
	void savescene(stream *f)
	{
		//if (curworld->touched) //only reset the world if some one moved shit
		resetworld(); //clear the tree before starting the save 

		store(); //save the curent objects
		//if want to save the data in the map just clear the tree and then do a store this will save the world in its current state
		//print(nodes.length());
		asScript->serializer->save(f);
	}
	void loadscene(stream *f)
	{
		asScript->serializer->load(f);
	}
	~scene()
	{
		if (nodes.length())	nodes.deletecontents();
		delete &nodes;
	}
	void updateregistaredobjects()
	{
	}
};

enum MODOE
{
	MODOE_ADD = 1 << 0,
	MODOE_UPDATEBB = 1 << 1,
	MODOE_CHANGED = 1 << 2
};

struct worldeditor //static struct
{

	//utility calls (get sets and checks)
	static bool isnodeedit(bool deselect=true); //force deselection if you shouldnt be editing nodes;
	static bool pointinsel(const selinfo &sel, const vec &o);
	static inline bool nodenoedit();
	static bool nodehavesel();
	static void nodeselcancel();
	static void nodeseladd(int id);
	static inline vec getselpos();
private:
	static vector<node *> getselnodes();
	struct undoinfo;

	//nessary functions that need recoded;
public:
	static float getnearestnode(const vec &o, const vec &ray, float radius, int mode, int &hitnode);
	static undoinfo *newundonode();
	static void makeundonode();
	static undoinfo *copyundonodes(undoinfo *u);
	static void pasteundonode(int idx, const node &un);
	static void pasteundonodes(undoinfo *u);
	static void detachnode(node &n); //use to release links
	
	//global map calls
	static bool emptymap(int scale, bool force, const char *mname, bool usecfg);
	static bool enlargemap(bool force); //this is an obsolete call, this should only enlarge a scene's world root. Check to find the curent  editing scene, and which worldroots are in active edit state then only enlarge the ones that are asked
	static void reset();
	static void startmap(const char *name);
	
	//rendering
	static void rendernodeselection(const vec &o, const vec &ray, bool nodemoving);
	static bool hoveringonnode(int node, int orient);
	static void rendernodering(const node &n, int axis);
	static void rendernodesphere(const node &n);
	static void rendernodelink(const node &n, int type);
	static void rendernodearrow(const node &n, const vec &dir, float radius);
	static void rendernodecone(const node &n, const vec &dir, float radius, float angle);
	static void rendernodebox(const node &n);
	static void rendernodebox(const vec no, vec es);

	//functionality
	static bool editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first);
	static void nodedrag(const vec &ray);
	static void delnode();
	static void nodeflip(); //           flip around selected cube * if no cube * selected call each node through the message system to get a result
	static void noderotate(int *cw); //rotate around selected cube * if no cube * selected call each node through the message system to get a result
	static void nodeautoview(int *dir); //used to rotate through node positions (use later)
	

	//octtree intergration
	static void modifyoctaentity(int flags, int id, node &n, cube *c, const ivec &cor, int size, const ivec &bo, const ivec &br, int leafsize, vtxarray *lastva = NULL);
	static bool modifyoctanode(int flags, int id, node &n);
	static inline bool modifyoctanode(int flags, int id);
	static inline void addnode(int id);// { modifyoctaent(MODOE_ADD | MODOE_UPDATEBB, id); }
	static inline void addnodeedit(int id);// { modifyoctaent(MODOE_ADD | MODOE_UPDATEBB | MODOE_CHANGED, id); }
	static inline void removenode(int id);// { modifyoctaent(MODOE_UPDATEBB, id); }
	static inline void removenodeedit(int id);// { modifyoctaent(MODOE_UPDATEBB | MODOE_CHANGED, id); }
	static void nodesinocta();

	//static void freeoctanodes(cube &c);

	//posibly obsolete
	static inline void transformbb(node &n);
	static inline void mmboundbox(const node &n, model *m);
	static inline void mmcollisionbox(const node &n);

	//aabb calls
	//static inline vec getrenderbounds(node &n);
	//static inline vec getphysicsbounds(node &n);

	//struct selectinfo
	//{
	//private:
	//	selectinfo() {} //only can be created internally
	//};

private:
	struct undoinfo
	{

	};
	worldeditor() {}; //make private so no one tries to instance this
	static int nodehover, oldhover, nodeorient, nfocus, nodemoving;
	static vector<int> nodeselect;
	//static vector<selectinfo *> selinfos;
	static vector<undoinfo *> undoinfos;
	static int nodelooplevel;
	static bool undonext, nodecanedit;
	static bool nodeselsnap, nodeediting;
}; 








//definition of world declaration is in the bottom of world.cpp
struct world
{
public:
	//utilitycalls (get sets and checks)
	world(scene *s, uint maxnodes = 2000, ushort maxallocate = 200, ushort maxalocateperround = 20);
	uint getnumnodes();
	bool ispaused();

	//update and resets
	void doAwake();
	void rendernodes();
	void serializedworld();
	void updateworld();
	void clearworld(bool force = false);
	void resetmap();
	void saveworld(stream *f);
	void loadworld(stream *f);

	//asset changes
	node *newnode(vec o = vec(0), vec rot = vec(0), str mod = ""); //create a new node from script name
	node *newnode(vec o = vec(0), vec rot = vec(0), asIScriptObject *aso = NULL); //create a new node from premade object
	node *newnode(vec o = vec(0), vec rot = vec(0), asITypeInfo *ast = NULL); //create a new node from a type
	node *newnode(uint id, vec o = vec(0), vec rot = vec(0)); //create a new node from a reference copy
	node *newnode(const node &on, vec o = vec(0), vec rot = vec(0)); //create a new node from a pointer
	node *removenode(uint id);
	node *removenode(node *n);

	//add a light manger later
	void addlight(vec o, vec color, int radius, char type);
	vector<light *> getlights();


	bool validate(node *n, bool forcedestroy = true);
	void setnodealocation(uint num, ushort numpercall); //preallocate x amount of nodes, and keep it at this level for as long as posible. You should change this based on your intake this will allow you to save time and memmory
private:
	struct nodemgr
	{
		void init(ushort maxalocate, ushort maxpertime);
		void update();
		node *newnode();
		node *getnodefromid(uint id);
		bool removenode(node *n);
		bool removenode(uint id);
		void kill();
		const inline uint numofnodes();
		uint checkvalid(node *n);
		void setnodealocation(uint num, ushort numpercall);
	private:
		uint getnextopenid();
		void allocatenodes(ushort amt);
		void deallocatenodes(ushort amt);
		node *getnodefrompool();
		vector<node *> nodes;
		vector<node *> nodepool;
		vector<uint> openids;
		ushort maxallocateatonce;
		uint worldnodealocate;
	};
	nodemgr nmgr;
	scene *curscene;
	vector<uint> physicbodies;
	vector<light *> lights;
	vector<scene *> scenes;
	int spotlights, volumetriclights;
	bool paused = false, touched = false;
};

//struct ModelCtrl : IController
//{
//	model *m;
//};
//enum {BT_SHAPE_BOX = 0, BT_SHAPE_SPHERE, BT_SHAPE_BULLET, BT_SHAPE_CYLLENDER};
//struct PhysicsCtrl : IController
//{
//	btRigidBody *btBody;
//	float friction, weight, restitution, ldamp, adamp;
//	vec gravity, afactor, lfactor;
//	int type;
//};
//struct LightCtrl : IController
//{
//	vec4 lightvec;
//	vec4 type;
//	//add more later
//	void render()
//	{
//
//	}
//};
//struct ParticleCtrl : IController
//{
//	int type;
//	vec gravity, color, size;
//
//	void update()
//	{
//
//	}
//	void render()
//	{
//
//	}
//};
//
extern world *curworld;
