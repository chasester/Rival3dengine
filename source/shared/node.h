//list of class defines
struct bulletobj;
struct ridgidbody;
struct editnode;
struct node;
struct scenegraph;
struct world;
struct scriptinterface;
struct savenode;
struct light;
struct cube;
struct vtxarray;
struct CSerialworld;
//list includes
#include "AJMPhys.h"

//list of externs and global variables
#define FROM_SAUER_VEC(sauervec) ( btVector3(sauervec.x/SAUER_FACTOR, sauervec.z/SAUER_FACTOR, sauervec.y/SAUER_FACTOR) )
#define TO_SAUER_VEC(sauervec, btvec) { sauervec.x = btvec.x()*SAUER_FACTOR; sauervec.y = btvec.z()*SAUER_FACTOR; sauervec.z = btvec.y()*SAUER_FACTOR; }
enum NODE_FLAGS
{
	NODE_ACTIVE = 1 << 0, //is the node active, is it sleeping or is it dead?
	NODE_DEACTIVE = 0 << 0, //opisite
	NODE_SERIALIZED = 1 << 1, //is this node serializing its self or should we delete it on reset
	NODE_START_ON_RESET = 1 << 2, //Should we let this item sleep or should we run the onstart as soon as we call a reset
	NODE_NO_RENDER = 0 << 3, //Should we let this item render (maybe cuz there is no rendering needed or cuz the object died or some other reason
	NODE_RENDER = 1 << 3, //opisite
	NODE_CHANGED = 1 << 4 //this will be true on first update
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
	friend poolidmgr<node *>;
	asILockableSharedBool *weakRefFlag;
	uint parent;
	vector<uint> children;
	str name, tags; //instance name, tag to classify data, set on a per object basis, allows it to search by these break tags up by space
	int refcount; //for weak reference
	uint id;
	ushort sceneid;
	uint flags;
	vector<asIScriptObject *> ctrl; //link to the script functions on* funcitons
									// position or start origin of the object
	bulletobj *b;
	vec o, rot, radius; //readius should not be changed, this is used to get a general idea of the bounds of the visual reprentation
	vec old_o, old_rot; //store the data before we move it
	CSerializedValue *c; //stored game data for said object;
	bool touched = false; //editnode *en;
	void store();
	bool restore();
	//private: make private later
	node(){}
	node(const node &n) : radius(vec(2.5f))  {};
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
	void rotateto(vec rot, bool degrees = true) { this->rot = vec(rot); }
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

//struct scenegraph 
//{
//public:
//	vector<node *> nodes;
//	uchar id;
//	str name;
//	char flags; //uses SCENEFLAGS
//	//physics handled on the world level
//	vector<light *> slights;
//	vector<node *> &getnodes(){ return nodes; }
//	const vector<light *> getlights() 
//	{
//		if (flags |= SCENE_NO_RENDER) slights.deletecontents();
//		return slights;
//	}
//	//	void pausescene(short force = -1){ paused = force == -1 ? !paused : bool(force); }
//	bool ispaused(){ return false; }
//	void addobject(node *gn) { if (!gn) return; nodes.add(gn); }
//	void clearworld()
//	{
//		nodes.deletecontents();
//		//add in the load from file here
//		//read and load nodes
//	}
//	void resetworld()
//	{
//		int a = nodes.length();
//		vector< node * > nd;
//		loopi(a)
//		{
//			node * g = nodes[i];
//			if (!g->restore())
//			{
//				delete g;
//			}
//			else{
//				nd.add(g);
//				//g->restart();
//			}
//
//		}
//		asScript->serializer->ClearRoot();
//		nodes = nd;
//		//loopv(nodes)nodes[i]->doawake();
//		//if (nodes.length()) nodes.deletecontents();
//
//		//loopv(editnodes)
//		//{
//		//	editnode *en = editnodes[i];
//		//	node *g = new node(vec(en->o));
//		//	g->ctrl = asScript->CreateController(en->name, g);
//		//	g->e = en; en->g = g;
//		//	if(!g->c) g->c = asScript->serializer->storenode(g);
//		//	else g->c->Restore(g, CSerialnode::getID());
//		//	
//		//	asScript->doStart(g->ctrl);
//		//	nodes.add(g);
//		//}
//	}
//	void preloadentities(); //load all the enties at gamestart get this list from the map file
//	void doawake()
//	{
//		loopv(nodes){ nodes[i]->doawake();}
//		
//	}
//	void updatenodes()
//	{
//		loopv(nodes)nodes[i]->updatefrombullet();
//		loopv(nodes) nodes[i]->doupdate();
//	}
//	void dorender()
//	{
//		loopv(nodes) { nodes[i]->dorender(); }
//		//loopv(nodes) { nodes[i]->dogui(); } // do gui calls after may need to do this in a different function call
//	}
//	void store()
//	{
//		loopv(nodes) nodes[i]->store();
//	}
//	void savescene(stream *f)
//	{
//		//if (curworld->touched) //only reset the world if some one moved shit
//		resetworld(); //clear the tree before starting the save 
//
//		store(); //save the curent objects
//		//if want to save the data in the map just clear the tree and then do a store this will save the world in its current state
//		//print(nodes.length());
//		asScript->serializer->save(f);
//	}
//	void loadscene(stream *f)
//	{
//		asScript->serializer->load(f);
//	}
//	~scenegraph()
//	{
//		if (nodes.length())	nodes.deletecontents();
//		delete &nodes;
//	}
//	void updateregistaredobjects()
//	{
//	}
//};

enum OCTROOT //all the modes for worldroot::octroot struct
{
	OCTROOT_LOCK = 1 << 0, //unlock root
	OCTROOT_UNLOCK = 0 << 0, // invers of the above for simplicty will check every edit call
	OCTROOT_PHYSICS = 1 << 1, //registar with physics
	OCTROOT_OCCO = 1 << 2, //use as octa ents // will only pick one per worldroot (It will pick the first active).
	OCTROOT_RENDER = 1 << 3, //Send to the renderer this only effects in game not edit mode use both hidden and render = 0 to turn a node completely off. If a node is Deactive it will have the same effect. But if registared with physics it will still create the mesh, just wont use it because it will be set to deactive state in bullet
	OCTROOT_ACTIVE = 1 << 4, //Checked every frame cycle, turning it off will cause the octree not to render be a physic object or be used for occolision (if it is deactived when it is the main occulator, then it will find a new one, and will not move back when activated, you must deactivate the current occulator then then it will automatically pick the top one in the array).
	OCTROOT_DEACTIVE = 0 << 4, //inverse of the above for simplicity
	OCTROOT_HIDDEN = 1 << 5, //this is a edit command, this will only hide during editing, works like render but only hides in edit mode (this also makes it unselectable), like lock.
	OCTROOT_CHANGED = 1 << 6 // if false does not need to rebuild the mesh for physics, bullet should save the physic mesh and will use an id system to over ride the data if nessary to save on load time.
};

struct octroot //keeps track of the purpose and goals of a particular octree, This also handling locking and unlocking octrees and basic changes to the octroot. Root is you parent cube this is equivalent to the former cube *worldroot and worldsize is equivalent to the same as before. The flags will alow you to lock and unlock oct trees and set up the purpose and rendering setting of each octree
{
	
	friend world;
	friend idoctmgr_REPLACE<octroot *>;
	//basic lock functions
	bool lock(bool force = false);
	bool unlock(bool force = false);
	bool toggglelock(bool force = false);
	vector<vtxarray *> rendervaocts();
	void buildtricolisionmesh();

	uint getoctid();
	cube *getroot();
	char flags;
	~octroot(); //delete root and tell the world to release its id;
private:
	octroot(int _worldsize, cube *_cube = NULL, char _flags = 0);
	vector<vtxarray *> lastrender; //save last time we rendered this mesh so that we dont need to update the vtx every time;
	cube *root;
	uint id;
	int worldsize;
	
};


struct worldroot 
{
	friend world;
	uint newoctroot(char flags); //returns the id
	void seteditoct(uint id);
	void setposition(vec pos);
	void setrotation(vec pos);
	void setscale(vec pos);
	void changeworldsize(uint size);
	
	node *getnode(uint id);
	void dorender();
	void doawake();
	void doupdate();

	void getnodes(vector<node *>& nodes);
	void getupdatenodes(vector<node *>& nodes);
	void getrendernodes(vector<node *>& nodes);

	void getnodesid(vector<uint>& nodeids);
	void getupdatenodesid(vector<uint>& nodeids);
	void getrendernodesid(vector<uint>& nodeids);

	void serializeworld(vector<uint> &ids);
	void clearnodes(); //removes nodes so we can concatinate;
	void clearworld();
	void restartworld();
	void saveworld(stream *f);
	void loadworld(stream *f);

private:
	worldroot(world *w, int worldsize, ushort id, matrix4 worldmatrix = matrix4(), vector<uint> &nodeid = vector<uint>(), vector<uint> &rootid=vector<uint>());
	void addnode(uint id); //add to private cuz only world and worldroot should use this functionality; this may later be changed to allow things in other worldroots to be changed (but this probably will be done through the world any way);
	world *m_world;
	bool touched = false;
	vector<uint> rootsid;
	vector<uint> nodeid;
	int worldsize;
	matrix4 worldmatrix;
	ushort id;
	~worldroot(); //destroy all roots and nodes by telling the world DONOT DELETE m_world it will delete its self;
};

struct worldeditor
{
	static bool nonodeedit();
	static bool nodehaveselection();
	static bool nodehavehover();
	static void getselecnodesid(vector<uint> &nodeids);
	static void getselnodes(vector<node *>& nodes);
	static node *getnfocus();
	static int getnfocusid();
	static int gethovernodeid();
	static node *gethovernode();
	static void nodecancelselect();
	static void nodeselectionadd(int id=0);
	static bool nodeisselected(int id=0);
	static bool nodeisselected(node *n);
	static void geteditablenodes(vector<node *>& nodes); //gets all nodes in the edit frame;
	static void nodedrag(const vec &ray);
	static bool hoveringonnode(uint node, int orient);
	static bool nodetoggleselect(uint id);
	static float getnearestnode(const vec &o, const vec &ray, float radius, int mode, int &hitnode);

	static char nodemoving, nodeorient;
	static bool nodeselsnap, nodeediting;
	//static cube* editroot;
	static worldroot *editworldroot;
private:
	static vector<uint> nodeselect;
	static uint nodehover,oldhover, nfocus, nodelooplevel;
};

struct world
{
	friend CSerialworld;
public:
	//utilitycalls (get sets and checks)
	world(uint maxnodes = 2000, ushort maxallocate = 200, ushort maxalocateperround = 20, worldroot *root = NULL);
	uint getnumnodes();
	bool ispaused();

	//update and resets
	void doAwake();
	void dorender();
	void serializedworld();
	void updateworld();
	void restartworld(bool force = false);
	void clearmap();
	void saveworld(stream *f);
	void loadworld(stream *f);

	//asset changes
	node *newnode(vec o = vec(0), vec rot = vec(0), str mod = ""); //create a new node from script name
	node *newnode(asIScriptObject *aso = NULL); //create a new node from premade object
	node *newnode(asITypeInfo *ast = NULL); //create a new node from a type
//	node *newnode(node *n, vec o = vec(0), vec rot = vec(0)); //create a new node from a reference copy //obsolete use below
	node *newnode(const node &on, vec o = vec(0), vec rot = vec(0)); //create a new node from a pointer
	node *newnode(uint id); //used by serializer to create a new node for loading and unloading forceing a particular id;

	bool removenode(uint id);
	bool removenode(node *n);
	bool removenode(const vector<uint > &nodes);
	node *getnodefromid(uint id);
	void getnodefromid(vector<uint> &nids, vector<node *> &nodes); //pass as reference so world can fix any out of date ids (this should not be relied on and objects should tell the scene when they are removed);
	void clearlights();

	octroot *newoctree(int worldsize, char flags=0);
	void removeoctree(uint id);
	octroot *getoctreefromid(uint id);
	void getoctreefromid(vector<uint> &ids, vector<octroot *> &octs);  //pass as reference so world can fix any out of date ids (this should not be relied on and objects should tell the scene when they are removed);


	//add a light manger later
	void addlight(vec o, vec color, int radius, char type);
	vector<light *> &getlights();


	bool nodevalidate(node *n, bool forcedestroy = true);
	bool nodevalidate(vector<uint> &nodes);
	void setnodealocation(uint num, ushort numpercall); //preallocate x amount of nodes, and keep it at this level for as long as posible. You should change this based on your intake this will allow you to save time and memmory

	bool paused = false;
	scenegraph *curscene;
private:
	vector<node *> &concatinatenodes();
	poolidmgr<node *> nmgr;
	poolidmgr<CSerializedValue *> serialobjects;
	//poolmgr<btRigidBody *> physicbodies;
	idoctmgr_REPLACE<octroot *> omgr;
	
	vector<uint> physicbodies;
	vector<light *> lights;
	vector<worldroot *> worldroots;
	int spotlights, volumetriclights;
	bool touched = false;
	~world();//build order, destroy all physics bodys registared with bullet, delete the lights, destroy all worldroots, this should destroy the data in the world mangrs but call their deconstructors anyway;
};

//struct ModelCtrl : IController
//{
//	model ;
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
