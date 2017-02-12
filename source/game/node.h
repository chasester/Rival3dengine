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
	asILockableSharedBool *weakRefFlag;
	uint parent;
	vector<uint> children;
	str name, tags; //instance name, tag to classify data, set on a per object basis, allows it to search by these break tags up by space
	int refcount; //for weak reference
	vector<asIScriptObject *> ctrl; //link to the script functions on* funcitons
	// position or start origin of the object
	bulletobj *b;
	vec o, rot, radius; //readius should not be changed, this is used to get a general idea of the bounds of the visual reprentation
	vec old_o, old_rot; //store the data before we move it
	CSerializedValue *c; //stored game data for said object;
	//editnode *en;
	void store();
	bool restore();
	node(vec pos, str name = "unnamed", str tags = "");
	node(vec pos, vec rotation);

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

struct scenegraph 
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
		//loopv(nodes)nodes[i]->doawake();
		//if (nodes.length()) nodes.deletecontents();

		//loopv(editnodes)
		//{
		//	editnode *en = editnodes[i];
		//	node *g = new node(vec(en->o));
		//	g->ctrl = asScript->CreateController(en->name, g);
		//	g->e = en; en->g = g;
		//	if(!g->c) g->c = asScript->serializer->storenode(g);
		//	else g->c->Restore(g, CSerialnode::getID());
		//	
		//	asScript->doStart(g->ctrl);
		//	nodes.add(g);
		//}
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
	~scenegraph()
	{
		if (nodes.length())	nodes.deletecontents();
		delete &nodes;
	}
	void updateregistaredobjects()
	{
	}
};

//definition of world declaration is in the bottom of world.cpp
struct world
{
	struct nodemgr
	{
	private:
		
		node *newnode();  //call to get a new node from scratch; recieve the node
		uint newnode(node *n); //class to copy an existing node; recieve the id
		
		bool removenode(uint id);
		bool removenode(node *n);

		node *root;
		vector<node *> nodes();
	};
	int nodehover, oldhover, nodeorient, nfocus, nodemoving = 0;
	vector<uint> physicbodies;
	//int nodemoving = 0;
	vector<int> nodeselect;
	scenegraph *curscene;
	bool nodeselsnap = false, nodeediting = true;
	int spotlights, volumetriclights, nodelooplevel;
	bool paused = false, touched = false;
	vector<light *> lights;
	//vector<scenegraph *> scenes; add later

	world(scenegraph *s);

	inline bool nonodeedit();

	void addlight(vec o, vec color, int radius, char type);
	bool haveselnode();
	void nodeselcancel();
	void nodeseladd(int id);
	vec getselpos();
	vector<node *> getselnodes();
	vector<light *> getlights();

	void serializedworld();
	void updateworld();
	void clearworld(bool force = false);
	void rendernodeselection(const vec &o, const vec &ray, bool nodemoving);

	uint getnumnodes();
	void addnodetoscene(node *g);
	void rendernodes();
	bool ispaused() { return paused; }
	bool hoveringonnode(int node, int orient);
	float getnearestent(const vec &o, const vec &ray, float radius, int mode, int &hitnode);

	void saveworld(stream *f);
	void loadworld(stream *f) { curscene->loadscene(f); }
	void doAwake() { curscene->doawake(); }
	void addctrltonode(str name, bool first);
	//undo add later
	//undoblock *newundoent();
	//void makeundoent();
	//undoblock *copyundoents(undoblock *u);
	//void pasteundoent(int idx, const entity &ue);
	//void pasteundoents(undoblock *u)

	//nessary functions that need recoded
	//void detachentity(extentity &e);
	//void entflip();
	//void entrotate(int *cw);
	//void startmap(const char *name)
	//bool emptymap(int scale, bool force, const char *mname, bool usecfg) 
	//bool enlargemap(bool force)
	//void entautoview(int *dir);

	//	bool pointinsel(const selinfo &sel, const vec &o);
	//bool editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first);
	void nodedrag(const vec &ray);
	void delnode();
	void resetmap();
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
