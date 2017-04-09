// world.cpp: core map management stuff

#include "engine.h"

VARR(mapversion, 1, MAPVERSION, 0);
VARNR(mapscale, worldscale, 1, 0, 0);
VARNR(mapsize, worldsize, 1, 0, 0);
SVARR(maptitle, "Untitled Map by Unknown");
VARNR(emptymap, _emptymap, 1, 0, 0);

VAR(octaentsize, 0, 64, 1024);
VAR(entselradius, 0, 2, 10);


void freeoctaentities(cube &c)
{
//   /* if(!c.ext) return;
//    if(entities::getents().length())
//    {
//        while(c.ext->ents && !c.ext->ents->mapmodels.empty()) removeentity(c.ext->ents->mapmodels.pop());
//        while(c.ext->ents && !c.ext->ents->decals.empty())    removeentity(c.ext->ents->decals.pop());
//        while(c.ext->ents && !c.ext->ents->other.empty())     removeentity(c.ext->ents->other.pop());
//    }
//    if(c.ext->ents)
//    {
//        delete c.ext->ents;
//        c.ext->ents = NULL;
//    }*/
}


//change from ent to node later
VARF(nodeediting, 0, 0, 1, { worldeditor::isnodeedit();}); //change to a toggle


VAR(attachradius, 1, 100, 1000);



VAR(entselsnap, 0, 0, 1);
VAR(nodemovingshadow, 0, 1, 1);

//rendering functionality
extern void boxs(int orient, vec o, const vec &s); 
extern void boxs3D(const vec &o, vec s, int g);
extern bool editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first);



VAR(showentradius, 0, 1, 1);


VAR(entitysurf, 0, 0, 1);

ICOMMAND(entadd, "", (),
{
   /* if(curworld->nodeselect.length() >= 0 && !curworld->nonodeedit())
    {
        if(curworld->nodeselect.find(curworld->nodehover) < 0) entadd(curworld->nodehover);
        if(curworld->nodemoving > 1) curworld->nodemoving = 1;
    }*/
});

ICOMMAND(enttoggle, "", (),
{
    /*if(curworld->nodehover < 0 || curworld->nonodeedit() || !enttoggle(curworld->nodehover)) { curworld->nodemoving = 0; intret(0); }
	else { if (curworld->nodemoving > 1) curworld->nodemoving = 1; intret(1); }*/
});

ICOMMAND(entmoving, "b", (int *n),
{
   /* if(*n >= 0)
    {
		if (!*n || curworld->nodehover < 0 || curworld->nonodeedit()) curworld->nodemoving = 0;
        else
        {
			if (entgroup.find(curworld->nodehover) < 0) { entadd(curworld->nodehover); curworld->nodemoving = 1; }
			else if (!curworld->nodemoving) curworld->nodemoving = 1;
        }
    }
	intret(curworld->nodemoving);*/
});



VAR(nodeautoviewdist, 0, 25, 100);

VAR(entdrop, 0, 2, 3);



void splitocta(cube *c, int size)
{
	if (size <= 0x1000) return;
	loopi(8)
	{
		if (!c[i].children) c[i].children = newcubes(isempty(c[i]) ? F_EMPTY : F_SOLID);
		splitocta(c[i].children, size >> 1);
	}
}
#pragma endregion old code that needs killed check for depences

void startmap(const char *name)
{
    game::startmap(name);
    ovr::reset();
}

bool emptymap(int scale, bool force, const char *mname, bool usecfg)    // main empty world creation routine
{
    if(!force && !editmode)
    {
        conoutf(CON_ERROR, "newmap only allowed in edit mode");
        return false;
    }

    

    setvar("mapscale", scale<10 ? 10 : (scale>16 ? 16 : scale), true, false);
    setvar("mapsize", 1<<worldscale, true, false);
    setvar("emptymap", 1, true, false);

    texmru.shrink(0);
    freeocta(worldroot);
    worldroot = newcubes(F_EMPTY);
    loopi(4) solidfaces(worldroot[i]);

    if(worldsize > 0x1000) splitocta(worldroot, worldsize>>1);

    clearmainmenu();

    if(usecfg)
    {
        identflags |= IDF_OVERRIDDEN;
        execfile("config/default_map_settings.cfg", false);
        identflags &= ~IDF_OVERRIDDEN;
    }

    initlights();
    allchanged(usecfg);

    startmap(mname);

    return true;
}

bool enlargemap(bool force)
{
    if(!force && !editmode)
    {
        conoutf(CON_ERROR, "mapenlarge only allowed in edit mode");
        return false;
    }
    if(worldsize >= 1<<16) return false;

   // while(outsideents.length()) removeentity(outsideents.pop());

    worldscale++;
    worldsize *= 2;
    cube *c = newcubes(F_EMPTY);
    c[0].children = worldroot;
    loopi(3) solidfaces(c[i+1]);
    worldroot = c;

    if(worldsize > 0x1000) splitocta(worldroot, worldsize>>1);

    enlargeblendmap();

    allchanged();

    return true;
}

static bool isallempty(cube &c)
{
    if(!c.children) return isempty(c);
    loopi(8) if(!isallempty(c.children[i])) return false;
    return true;
}

void shrinkmap()
{
    extern int nompedit;
    if(noedit(true) || (nompedit && multiplayer())) return;
    if(worldsize <= 1<<10) return;

    int octant = -1;
    loopi(8) if(!isallempty(worldroot[i]))
    {
        if(octant >= 0) return;
        octant = i;
    }
    if(octant < 0) return;

    //while(outsideents.length()) removeentity(outsideents.pop());

    if(!worldroot[octant].children) subdividecube(worldroot[octant], false, false);
    cube *root = worldroot[octant].children;
    worldroot[octant].children = NULL;
    freeocta(worldroot);
    worldroot = root;
    worldscale--;
    worldsize /= 2;

    ivec offset(octant, ivec(0, 0, 0), worldsize);
    vector<extentity *> &ents = entities::getents();
    loopv(ents) ents[i]->o.sub(vec(offset));

    shrinkblendmap(octant);

    allchanged();

    conoutf("shrunk map to size %d", worldscale);
}

void newmap(int *i) { bool force = !isconnected(); if(force) game::forceedit(""); if(emptymap(*i, force, NULL)) game::newmap(max(*i, 0)); }
void mapenlarge() { if(enlargemap(false)) game::newmap(-1); }
COMMAND(newmap, "i");
COMMAND(mapenlarge, "");
COMMAND(shrinkmap, "");

void mapname()
{
    result(game::getclientmap());
}

COMMAND(mapname, "");

void mpeditent(int i, const vec &o, int type, int attr1, int attr2, int attr3, int attr4, int attr5, bool local)
{
	return;
    /*if(i < 0 || i >= MAXENTS) return;
    vector<extentity *> &ents = entities::getents();
    if(ents.length()<=i)
    {
        extentity *e = newentity(local, o, type, attr1, attr2, attr3, attr4, attr5, i);
        if(!e) return;
        addentityedit(i);
        attachentity(*e);
    }
    else
    {
        extentity &e = *ents[i];
        removeentityedit(i);
        int oldtype = e.type;
        if(oldtype!=type) detachentity(e);
        e.type = type;
        e.o = o;
        e.attr1 = attr1; e.attr2 = attr2; e.attr3 = attr3; e.attr4 = attr4; e.attr5 = attr5;
        addentityedit(i);
        if(oldtype!=type) attachentity(e);
    }
    entities::editent(i, local);
    clearshadowcache();
    commitchanges();*/
}

int getworldsize() { return worldsize; }
int getmapversion() { return mapversion; }


	//world *curworld = new world(new scene());
	
	void editnodeattr(const char *input, int *v)
	{
		//vector<node *> n = curworld->getselnodes();
		//if (n.length() == 0) conoutf("nothing selected");
		//asIScriptObject *c;
		//loopv(n) 
		//{
		//	loopvj(n[i]->ctrl) {
		//		c = n[i]->ctrl[j];
		//		loopk(c->GetPropertyCount())
		//		{
		//			if (str(c->GetPropertyName(k)) == input)
		//			{
		//				//if (ctrl->GetPropertyTypeId(i) == asTYPEID_INT64) {

		//				//void * b = asScript->convertPrimitive(v, c->GetPropertyTypeId(k));
		//				//asScript->assigntoptrfromtypeid(c->GetAddressOfProperty(k), b, c->GetPropertyTypeId(k));
		//				*reinterpret_cast<int *>(c->GetAddressOfProperty(k)) = *(int *) (v);
		//				//conoutf("changed %s to %d", input, *(int*) b);
		//				break;
		//				//}else conoutf("not an int");
		//			}

		//		}
		//	}
		//n[i]->doawake();
		//}
		
	}COMMAND(editnodeattr, "si");

	void runnodecode(const char *input)
	{

	}COMMAND(runnodecode, "s");
	VARR(maxnodes, 0, 700, 100000);
node *newasent(str name, vec o, bool addnode)
{
	//move this code to the scene graph
	//create the game object and contrler from the scriptmgr, then check to make sure the ctrl exist and it compiled correctly
	if (curworld->getnumnodes() >= maxnodes) return NULL;
	node *g = curworld->newnode(o, vec(), name);
	if (!g) return NULL;
	loopv(g->ctrl)asScript->doCreate(g->ctrl[i]);

	if (addnode)g->store();
	else g->doawake();
	//if(en) en->name = name;
	//curworld->addnodetoscene(g);
	
	//assign a reference to the edit entity
	return g;

}
void nodechangename(const char *name)
{
	/*vector<node *> nodes = curworld->getselnodes();
	node *g = nodes[0];
	if (!g)return;
	g->name = str(name);
}COMMAND(nodechangename, "s");
void nodeaddctrl (const char *name, int i)
{
	vector<node *> nodes = curworld->getselnodes();
	node *g = nodes[0];
	if (!g) return;
	asIScriptObject *obj = asScript->CreateController(str(name), g);
	if (!obj) return;
	if (i < 1) g->ctrl.add(obj);
	else g->ctrl.insert(0, obj);
	asScript->doCreate(obj);*/
} /*COMMAND(nodeaddctrl, "si");*/

#pragma region "render fuctions"
void renderring(const vec &o, float radius, int axis, int smoothness)
{
	if (radius <= 0) return;
	smoothness = smoothness < 0 ? 15 : smoothness < 4 ? 4 : smoothness > 60 ? 60 : smoothness; //make sure we dont over render the detail
	gle::defvertex();
	gle::begin(GL_LINE_LOOP);
	//fix later where axis is a vec direction
	loopi(smoothness)
	{
		vec p(o);
		const vec2 &sc = sincos360[i*(360 / smoothness)];
		p[axis >= 2 ? 1 : 0] += radius*sc.x;
		p[axis >= 1 ? 2 : 1] += radius*sc.y;
		gle::attrib(p);
	}
	xtraverts += gle::end();
}
void rendersphere(const vec &o, float radius, int smoothness)
{
	if (radius <= 0) return;
	smoothness = smoothness < 0 ? 15 : clamp(smoothness, 5, 60);
	loopk(3) renderring(o, radius, k, smoothness);
}
void renderbezier(const vec &st, const vec &ds, const vec &mid1 = vec(0), const vec &mid2 = vec(0))
{
	vec p = vec(st), mp, mp2;
	if (mid1.iszero()) { mp = ((vec(st).mul(3)).add(ds)).div(4); mp.z = st.z; }
	else mp = vec(mid1);
	if (mid2.iszero()) { mp2 = ((vec(ds).mul(3)).add(st)).div(4); mp2.z = ds.z; }
	else mp2 = vec(mid2);
	gle::defvertex();
	gle::begin(GL_LINE_STRIP);
	for (float t = 0; t <= 1; t += 0.1)
	{
		loopi(3)p[i] = pow((1 - t), 3)*st[i] + 3 * pow((1 - t), 2)*t*mp[i] + 3 * (1 - t)*pow(t, 2)*mp2[i] + pow(t, 3)*ds[i];
		gle::attrib(p);
	}
	gle::attrib(ds);
	xtraverts += gle::end();
}
void renderarrow(const vec &o, const vec &dir, float radius, float maxradius = 2.f)
{
	if (radius <= 0) return;
	float arrowsize = min(radius / 2, maxradius);
	vec target = vec(dir).mul(radius).add(o), arrowbase = vec(dir).mul(radius - arrowsize).add(o), spoke;
	spoke.orthogonal(dir);
	spoke.normalize();
	spoke.mul(arrowsize);

	gle::defvertex();

	gle::begin(GL_LINES);
	gle::attrib(o);
	gle::attrib(target);
	xtraverts += gle::end();

	gle::begin(GL_TRIANGLE_FAN);
	gle::attrib(target);
	loopi(5) gle::attrib(vec(spoke).rotate(2 * M_PI*i / 4.0f, dir).add(arrowbase));
	xtraverts += gle::end();
}
void renderbox(const vec &o, const vec &rot, const vec &radius, bool axisaligned = false)
{
	matrix4x3 orient;
	orient.identity();
	orient.settranslation(o);
	if (axisaligned)
	{
		if (rot.x) orient.rotate_around_z(sincosmod360(rot.x));
		if (rot.y) orient.rotate_around_x(sincosmod360(rot.y));
		if (rot.z) orient.rotate_around_y(sincosmod360(-rot.z));
	}
	gle::defvertex();

	vec front[4] = { vec(-radius.x, -radius.y, -radius.z), vec(radius.x, -radius.y, -radius.z), vec(radius.x, -radius.y,  radius.z), vec(-radius.x, -radius.y,  radius.z) },
		back[4] = { vec(-radius.x, radius.y, -radius.z), vec(radius.x, radius.y, -radius.z), vec(radius.x, radius.y,  radius.z), vec(-radius.x, radius.y,  radius.z) };
	loopi(4)
	{
		front[i] = orient.transform(front[i]);
		back[i] = orient.transform(back[i]);
	}

	gle::begin(GL_LINE_LOOP);
	loopi(4) gle::attrib(front[i]);
	xtraverts += gle::end();

	gle::begin(GL_LINES);
	gle::attrib(front[0]);
	gle::attrib(front[2]);
	gle::attrib(front[1]);
	gle::attrib(front[3]);
	loopi(4)
	{
		gle::attrib(front[i]);
		gle::attrib(back[i]);
	}
	xtraverts += gle::end();

	gle::begin(GL_LINE_LOOP);
	loopi(4) gle::attrib(back[i]);
	xtraverts += gle::end();
}
#pragma endregion 

extern selinfo sel;
ICOMMAND(newasent, "s", (char *s), {if(!worldeditor::nodenoedit()) newasent(str(s), player->o, true); });

#pragma region "world"
	#pragma region "Utility"
		world::world(scene *s, uint maxnodes, ushort maxallocate, ushort maxallocateperround)
		{
			scenes.add(s);
			nmgr.init(maxallocate, maxallocateperround);
			//maxnodes not used right now
		}
		uint world::getnumnodes() { return nmgr.numofnodes(); }
		bool world::ispaused() { return paused; }
	#pragma endregion
	#pragma region "Updates/Reset"
		void world::doAwake() { curscene->doawake(); }
		void world::rendernodes() { curscene->rendernodes(); }
		void world::serializedworld() 
		{
			if (asScript->serializer) asScript->resetserializer();
			else asScript->setupserializer(new CSerializer());
			//loopk(scenes)
			//scene * cursn = sense[k];
			//if(curscene && curscene->nodes.length())
			//curscene->store();
		}
		void world::updateworld() { if (!touched) { serializedworld(); curscene->doawake(); touched = true;  return; }	curscene->updatenodes(); }
		void world::clearworld(bool force) { if (touched || force) curscene->resetworld(); touched = false; }
		void world::resetmap() 
		{
			clearoverrides();
			clearmapsounds();
			resetblendmap();
			clearlights();
			clearpvs();
			clearslots();
			clearparticles();
			clearstains();
			clearsleep();
			cancelsel();
			pruneundos();
			clearmapcrc();
			curscene->clearworld();
			PHYSrebuildLevel();
			//outsideents.setsize(0);
			spotlights = 0;
			volumetriclights = 0;
		}
		void world::saveworld(stream *f) 
		{
			//loopv(scences)
			{
				//curscence = scences[i];
				vector<node *> ns = curscene->getnodes();
				//f->putstring(curscene->name);
				//f->putlil<uint>(ns.length());
				curscene->savescene(f);
			}
		}
		void world::loadworld(stream *f) { curscene->loadscene(f); }
	#pragma endregion
	#pragma region "AssetChanges"
		node * world::newnode(vec o, vec rot, str mod)
		{
			node * n = nmgr.newnode();
			asIScriptObject *aso = asScript->CreateController(mod, n);
			n->moveto(o); n->rotateto(rot); n->ctrl.add(aso);
			return n;
		}
		node * world::newnode(vec o, vec rot, asIScriptObject * aso)
		{
			node *n = nmgr.newnode();
			n->moveto(o); n->rotateto(rot); n->ctrl.add(aso);
			return n;
		}
		node * world::newnode(vec o, vec rot, asITypeInfo * ast)
		{
			return newnode(o, rot, str(ast->GetName()));
		}
		node * world::newnode(uint id, vec o, vec rot) //creates a new node keep in mind this uses the same controler ptr, this should only be used if you want to link the nodes with the same controllers (call the prefab if you want a instance)
		{
			return newnode(*nmgr.getnodefromid(id), o, rot);
		}
		node * world::newnode(const node &on, vec o, vec rot)
		{
			node *n = new node(on);
			n->moveto(o); n->rotateto(rot);
			return n;
		}
		bool world::removenode(uint id) { return nmgr.removenode(id); }
		bool world::removenode(node *n) { return nmgr.removenode(n); }
		bool world::removenode(const vector<uint > &nodes) { if (nodes.length() < 1)return false; loopv(nodes)nmgr.removenode(nodes[i]); return true; }
		node *world::getnodefromid(uint id) {return nmgr.getnodefromid(id); }
		vector<node *> &world::getnodefromid(const vector<uint> nids)
		{
			vector<node *> nodes;
			loopv(nids)
			{
				node *n = nmgr.getnodefromid(nids[i]);
				if (n)nodes.add(n);
			}
			return nodes;
		}

		void world::addlight(vec o, vec color, int radius, char type)
		{
			light *l = new light();
			l->color = color;
			l->type = type;
			l->o = o;
			l->radius = radius;
			lights.add(l);
		}
		vector<light *> world::getlights() { return lights; }
		
		bool world::nodevalidate(node *n, bool forcedestroy) //this function calls the nodemager validate which allows us to check the validity of a *n. This should be done periodically to see if a node is referenced by * but doesnt have an id
		{
			uint id = nmgr.checkvalid(n);
			if (id < 1 && forcedestroy) { delete n; return false; }
			n->id = id;
		}
		void world::setnodealocation(uint num, ushort numpercall) { setnodealocation(num, numpercall); }
	#pragma endregion
	#pragma region "nodemgr"
#define NODEALOCATEOVERLOADPERCENT 1.10f
		node *world::nodemgr::newnode() 
		{ 
			node *n = getnodefrompool();
			uint indx = getnextopenid();
			nodes[indx] = n;
			n->id = indx;
			return 	n;
		}
		node *world::nodemgr::getnodefromid(uint id) 
		{
			return id < nodes.length() ? nodes[id] : nullptr;
		}

		bool world::nodemgr::removenode(node *n)
		{
			uint id = checkvalid(n);
			if (id == 0) { delete n; return false; }
			return removenode(id);
		}
		bool world::nodemgr::removenode(uint id)
		{

			if (nodes[id] == nullptr) return false;
			node *n = nodes[id];
			nodes[id] = nullptr;
			delete n;
			openids.add(id);
			return true;
		
		}
		void world::nodemgr::init(ushort maxalocate, ushort maxpertime) //keep in mind you can only init size of ushort to start with
		{
			maxallocateatonce = maxpertime;
			worldnodealocate = maxalocate;
			allocatenodes(maxalocate);
		}
		void world::nodemgr::update()
		{
			if (nodepool.length() < worldnodealocate) allocatenodes((nodepool.length() - worldnodealocate) % maxallocateatonce);
			else if (nodepool.length() > int(worldnodealocate*NODEALOCATEOVERLOADPERCENT)) deallocatenodes((nodepool.length() - int(worldnodealocate*NODEALOCATEOVERLOADPERCENT)) % maxallocateatonce);
		}
		void world::nodemgr::kill()
		{ //kill all vectors to limit memory leaks or problems
			nodes.shrink(0);
			nodepool.shrink(0);			
			openids.shrink(0);
			worldnodealocate = 0;
			maxallocateatonce = 0;
		}
		uint world::nodemgr::checkvalid(node *n) // if return 0 this means the node needs removed because it does not actually exist other wise we return the correct id
		{
			if (!n)return false; // make sure we are given a vaild pointer 
			if (nodes[n->id] != n || (n->id == 0)) //to make sure we have this node has a valid id, and that the id given is not null
			{
				int id = 0; // nodes.find(*n);
				if (id < 0) return 0;
				return id;
			}
			return n->id;
		}
		const inline uint world::nodemgr::numofnodes() { return nodes.length() - openids.length(); }
		void world::nodemgr::setnodealocation(uint num, ushort numpercall) { worldnodealocate = num; maxallocateatonce = numpercall; }
		#pragma region "private"
			uint world::nodemgr::getnextopenid() {if (openids.length()) return openids.pop(); else {nodes.pad(1); return nodes.length() - 1;}}
			void world::nodemgr::allocatenodes(ushort amt)
			{
				uint len = nodepool.length();
				//nodepool.advance(amt);
				loopi(amt) nodepool.add(new node());
			}
			void world::nodemgr::deallocatenodes(ushort amt){ nodepool.shrink(amt);	}
			node *world::nodemgr::getnodefrompool(){ if (nodepool.length()) return nodepool.pop(); else return new node(); }
		#pragma endregion
	#pragma endregion
#pragma endregion

#pragma region "worldeditor"
	#pragma region "UTILITY"
		bool worldeditor::isnodeedit(bool deselect)
		{
			if (deselect && worldeditor::nodecanedit & worldeditor::nodeediting) nodeselectcancel(); //remove entediting later
			return worldeditor::nodecanedit;
		}
		bool worldeditor::pointinselect(const selinfo &sel, const vec &o)
		{
			return(o.x <= sel.o.x + sel.s.x*sel.grid
				&& o.x >= sel.o.x
				&& o.y <= sel.o.y + sel.s.y*sel.grid
				&& o.y >= sel.o.y
				&& o.z <= sel.o.z + sel.s.z*sel.grid
				&& o.z >= sel.o.z);
		}
		bool worldeditor::nodenoedit()
		{
			if (!editmode) { conoutf(CON_ERROR, "operation only allowed in edit mode"); return true; }
			return !isnodeedit();
		}
		bool worldeditor::nodehaveselect()
		{
			return worldeditor::nodeselect.length() > 0;
		}
		void worldeditor::nodeselectcancel()
		{
			worldeditor::nodeselect.shrink(0);
		}
		void worldeditor::nodeselectadd(int id)
		{
			worldeditor::undonext = true;
			worldeditor::nodeselect.add(id);
		}
		vec worldeditor::getselpos()
		{
			vector<node *> nodes = curworld->getnodefromid(worldeditor::nodeselect);
			if (nodes.length() > 0 && nodes[0]) return nodes[0]->o;
			if (node *n = curworld->getnodefromid(worldeditor::nodehover)) return n->o;
			return vec(0); //return sel.o; fix when octselect is added
		}
		float worldeditor::getnearestnode(const vec &o, const vec &ray, float radius, int mode, int &hitnode)
		{
			/*vec eo, es;
			int orient;
			float dist = radius, f = 0.0f;
			nodeorient = -1;
			nodehover = -1;
			getnodesvec;
			loopv(nodes)
			{
				node *e = nodes[i];
				if (!e)continue;
				eo = vec(e->o);
				eo.sub(e->radius); eo.z += e->radius.z;
				es = vec(e->radius).mul(2);
				if (!rayboxintersect(eo, es, o, ray, f, orient)) continue;
				if (f<dist && f > 0)
				{
					nodeorient = orient;
					nodehover = i;
					return f;
				}
			}*/
			//obsolete call
			return radius;
		}
		inline vector<node*> &worldeditor::getselectnodes() { return curworld->getnodefromid(worldeditor::nodeselect); }
	#pragma endregion (Gets sets checks)
	#pragma region "Map Calls"
		bool worldeditor::emptymap(int scale, bool force, const char *mname, bool usecfg)
		{
			if (!force && !editmode)
			{
				conoutf(CON_ERROR, "newmap only allowed in edit mode");
				return false;
			}

			//change worldroot properties
			setvar("mapscale", scale<10 ? 10 : (scale>16 ? 16 : scale), true, false);
			setvar("mapsize", 1 << worldscale, true, false);
			setvar("emptymap", 1, true, false);

			texmru.shrink(0);
			freeocta(worldroot);
			worldroot = newcubes(F_EMPTY);
			loopi(4) solidfaces(worldroot[i]);

			if (worldsize > 0x1000) splitocta(worldroot, worldsize >> 1);

			clearmainmenu();

			if (usecfg)
			{
				identflags |= IDF_OVERRIDDEN;
				execfile("config/default_map_settings.cfg", false);
				identflags &= ~IDF_OVERRIDDEN;
			}

			initlights();
			allchanged(usecfg);

			startmap(mname);

			return true;
		}
		bool worldeditor::enlargemap(bool force)
		{
			if (!force && !editmode)
			{
				conoutf(CON_ERROR, "mapenlarge only allowed in edit mode");
				return false;
			}
			if (worldsize >= 1 << 16) return false;

		//	while (outsideents.length()) removeentity(outsideents.pop());// look at me

			worldscale++;
			worldsize *= 2;
			cube *c = newcubes(F_EMPTY);
			c[0].children = worldroot;
			loopi(3) solidfaces(c[i + 1]);
			worldroot = c;

			if (worldsize > 0x1000) splitocta(worldroot, worldsize >> 1);

			enlargeblendmap();

			allchanged();

			return true;
		}
		void worldeditor::reset() //inits or resets the editor. Should be done on map load or change, or when entering or exiting editmode
		{
			worldeditor::undonext = true;
			worldeditor::nodehover = -1; worldeditor::oldhover = -1; worldeditor::nodeorient = 0;
			worldeditor::nfocus = -1; worldeditor::nodemoving = 0;
			worldeditor::nodelooplevel = 0;
			worldeditor::nodeselect.shrink(0); /*selinfos.shrink(0); undoblocks.shrink(0);*/
			worldeditor::nodeselsnap = false; worldeditor::nodeediting = true;
		}
		void worldeditor::startmap(const char *name)
		{
			game::startmap(name);
			ovr::reset();
		}
	#pragma endregion (calls used when loading and reseting, and changing the map)
	#pragma region "Rendering"
		void worldeditor::rendernodeselection(const vec &o, const vec &ray, bool nodemoving)
		{
			if (nodenoedit())return;// || (entgroup.empty() && enthover < 0)) return;
			

			//fix this so that it is called from world when the render frame is being called
			//gle::colorub(0, 30, 0);
			//gle::defvertex();
			//gle::begin(GL_LINES, nodes.length() * 24);
			//loopv(nodes){
			//	node *e = nodes[i];
			//	vec o(e->o);
			//	o.z += e->radius.z+2.5; // add it for particles and for bounds :)
			//	regular_particle_splash(PART_EDIT, 2, 40, o, 0x3232FF, 0.32f*100.f / 100.0f);
			//	particle_textcopy(o, e->name.c_str(), PART_TEXT, 1, 0x6496FF, 2.0f);
			//	o.z -= e->radius.z+2.5;
			//	rendernodebox(o.sub(e->radius), vec(e->radius).mul(2));
			//}
			//xtraverts += gle::end();
			if (worldeditor::nodeselect.empty() && worldeditor::nodehover < 0) return;
			vec no, ns;
			if (worldeditor::nodeselect.length())
			{
				vector<node *> nodes = curworld->getnodefromid(worldeditor::nodeselect);
				gle::colorub(0, 50, 0);
				gle::defvertex();
				gle::begin(GL_LINES, nodes.length() * 24);
				loopv(nodes){
					node *n = nodes[i];
					vec o(n->o);
					renderbox(o.sub(n->radius), vec(n->radius).mul(2),vec(0), true);
				}
				xtraverts += gle::end();
			}
			if (worldeditor::nodehover)
			{
				node *n = curworld->getnodefromid(worldeditor::nodehover);
				if (!n)return;
				gle::colorub(0, 40, 0);
					// also ensurns enthover is back in focus
				no = vec(n->o); ns = (n->radius*2);
				no -= ns; ns.mul(2);
				boxs3D(no, ns, 1);
				if (nodemoving && nodemovingshadow == 1)
				{
					int plane = dimension(worldeditor::nodeorient); //do not render if can not move in that direction
					vec a, b;
					gle::colorub(40, 20, 20);
					(a = no).x = no.x - fmod(no.x, worldsize); (b = ns).x = a.x + worldsize; if (plane != 0){ boxs3D(a, b, 1); } //on x plane
					gle::colorub(20, 40, 20);
					(a = no).y = no.y - fmod(no.y, worldsize); (b = ns).y = a.x + worldsize; if (plane != 1){ boxs3D(a, b, 1); } //on y plane
					gle::colorub(20, 20, 40);
					(a = no).z = no.z - fmod(no.z, worldsize); (b = ns).z = a.x + worldsize; if (plane != 2){ boxs3D(a, b, 1); } //on z plane
				}
					
				gle::colorub(200, 0, 0);
				::boxs(worldeditor::nodeorient, no, ns);
			}
			gle::disable();
		}
		bool worldeditor::hoveringonnode(int node, int orient)
		{
			if(nodenoedit()) return false;
			worldeditor::nodeorient = orient;
			if((worldeditor::nfocus = worldeditor::nodehover = node) >= 0)
			return true;
			worldeditor::nfocus   = worldeditor::nodeselect.empty() ? -1 : worldeditor::nodeselect.last();
			worldeditor::nodehover = -1;
			return false;
		}
		void worldeditor::rendernodering(const node &n, int axis)
		{
			renderring(n.o, 10, axis, 30);
		}
		void rendernodesphere(const node &n)
		{
			rendersphere(n.o, n.radius.x, 30);
		}
		void worldeditor::rendernodelink(const node &n, int type)
		{
			//if children
			vector<node *> nodes = curworld->getnodefromid(n.children);
			loopv(nodes)
			{
				if (nodes[i]->o.iszero() || nodes[i]->o.magnitude() < 3) continue; //node is either ontop of or to close to the parent to draw to
				renderbezier(n.o, nodes[i]->o + n.o); //remember children are dependent on there parents position
			}
		}
		void worldeditor::rendernodearrow(const node &n, float radius)
		{
			renderarrow(n.o, n.rot, radius, 0.5f);
		}
		void worldeditor::rendernodecone(const node &n, float radius, float angle)
		{
			if(radius <= 0) return;
			vec spot = vec(n.rot).mul(radius*cosf(angle*RAD)).add(n.o), spoke;
			spoke.orthogonal(n.rot);
			spoke.normalize();
			spoke.mul(radius*sinf(angle*RAD));

			gle::defvertex();

			gle::begin(GL_LINES);
			loopi(8)
			{
			gle::attrib(n.o);
			gle::attrib(vec(spoke).rotate(2*M_PI*i/8.0f, n.rot).add(spot));
			}
			xtraverts += gle::end();

			gle::begin(GL_LINE_LOOP);
			loopi(8) gle::attrib(vec(spoke).rotate(2*M_PI*i/8.0f, n.rot).add(spot));
			xtraverts += gle::end();
		}
		void worldeditor::rendernodebox(const node &n)
		{
			renderbox(n.o, n.rot, n.radius, false);
		}
	#pragma endregion (calls related to rendering editmode visual aids)
	#pragma region "Functionality"
		bool worldeditor::editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first)
		{
			plane pl(d, off);
			float dist = 0.0f;
			//if (!pl.rayintersect(camera1->o, ray, dist))
				//return false;

			dest = vec(ray).mul(dist).add(camera1->o);
			if (first) handle = vec(dest).sub(o);
			dest.sub(handle);
			return true;
		}
		void worldeditor::nodedrag(const vec &ray)
		{
			if(nodenoedit() || !nodehaveselect()) return;

			float r = 0, c = 0;
			static vec dest, handle;
			vec eo, es;
			int d = dimension(worldeditor::nodeorient);
			int dc = dimcoord(worldeditor::nodeorient);
			vector<node *> nodes = curworld->getnodefromid(worldeditor::nodeselect);
			loopv(nodes) {
				if (!editmoveplane(nodes[i]->o, ray, d, nodes[i]->o[d] + (dc ? nodes[i]->radius[d] : 0), handle, dest, worldeditor::nodemoving == 1))
					return;

				ivec g = dest;
				int z = g[d] & (~(sel.grid - 1));
				g.add(sel.grid / 2).mask(~(sel.grid - 1));
				g[d] = z;

				r = (entselsnap ? g[R[d]] : dest[R[d]]) - nodes[i]->o[R[d]];
				c = (entselsnap ? g[C[d]] : dest[C[d]]) - nodes[i]->o[C[d]];
			}

			if(worldeditor::nodemoving==1) makeundonode();
			if (nodes.length() < 1)nodes.add(curworld->getnodefromid(worldeditor::nfocus));
			makeundonode();
			loopv(nodes)
			{ 
			nodes[i]->o[R[d]] += r; 
			nodes[i]->o[C[d]] += c; 
			}
			worldeditor::nodemoving = 2;
		}
		void worldeditor::delnode(){ if(!curworld->removenode(worldeditor::nodeselect))  curworld->removenode(worldeditor::nfocus); nodeselectcancel(); }//this removes all the nodeselects if nodeselect.length() > 0; if not it returns false;
		void worldeditor::nodeflip()
		{
			if(nodenoedit()) return;
			int d = dimension(sel.orient);
			float mid = sel.s[d]*sel.grid/2+sel.o[d];
			//groupeditundo(e.o[d] -= (e.o[d]-mid)*2);
			vector<node *> nodes = curworld->getnodefromid(worldeditor::nodeselect);
			if (nodes.length() < 1)nodes.add(curworld->getnodefromid(worldeditor::nfocus));
			makeundonode();
			loopv(nodes) nodes[i]->o[d] -= (nodes[i]->o[d] - mid) * 2;
		} 
		void worldeditor::noderotate(int *cw)
		{
			if(nodenoedit()) return;
			int d = dimension(sel.orient);
			int dd = (*cw<0) == dimcoord(sel.orient) ? R[d] : C[d];
			float mid = sel.s[dd]*sel.grid/2+sel.o[dd];
			vec s(sel.o.v);
			vector<node *> nodes = curworld->getnodefromid(worldeditor::nodeselect);
			if (nodes.length() < 1)nodes.add(curworld->getnodefromid(worldeditor::nfocus));
			makeundonode();
			loopv(nodes)
			{
				nodes[i]->o[dd] -= (nodes[i]->o[dd] - mid) * 2;
				nodes[i]->o.sub(s);
				swap(nodes[i]->o[R[d]], nodes[i]->o[C[d]]);
				nodes[i]->o.add(s);
			}
		} 
		void worldeditor::nodeautoview(int *dir)
		{
			//when camera class and player is more refined rewrite
			//if(!nodehaveselect()) return;
			//static int s = 0;
			//vec v(camera1->o);
			//v.sub(worldpos);
			//v.normalize();
			//v.mul(nodeautoviewdist);
			//int t = s + *dir;
			//s = abs(t) % entgroup.length();
			//if(t<0 && s>0) s = entgroup.length() - s;
			//entfocus(entgroup[s],
			//v.add(e.o);
			//player->o = v;
			//player->resetinterp();
			//);
		}
		undoblock *worldeditor::newundonode()
		{
			int numents = worldeditor::nodeselect.length();
			if(numents <= 0) return NULL;
			undoblock *ub = (undoblock *)new uchar[sizeof(undoblock) + numents*sizeof(undonode)];
			ub->numents = numents;
			undonode *un = (undonode *)(ub + 1);
			//loopv(entgroup)
			//{
			//un->i = entgroup[i]; //nodes id
			//un->n = *entities::getents()[entgroup[i]]; //serialized node
			//un++; //???
			//}
			return ub;
		}
		void worldeditor::makeundonode()
		{ 
			if(!worldeditor::undonext) return;
			worldeditor::undonext = false;
			worldeditor::oldhover = worldeditor::nodehover;
			undoblock *u = newundonode();
			if(u) addundo(u);
		}
		undoblock *worldeditor::copyundonodes(undoblock *u) 
		{
			//we are gonna have to do this differently
	/*		nodeselectcancel();
			undonode *n = u->ents();
			loopi(u->numents)
			addnode(n[i].i);
			undoblock *c = newundoent();
			loopi(u->numents) if(e[i].e.type==ET_EMPTY)
			entgroup.removeobj(e[i].i);
			return c;*/
			return NULL;
		}
		void worldeditor::pasteundonode(int idx, const node &un)
		{
			//will have to redo
			/* if(idx < 0 || idx >= MAXENTS) return;
			vector<extentity *> &ents = entities::getents();
			while(ents.length() < idx) ents.add(entities::newentity())->type = ET_EMPTY;
			int efocus = -1;
			entedit(idx, (entity &)e = ue);*/
			return;
		}
		void worldeditor::pasteundonodes(undoblock *u)
		{
		//will have to redo
		/* undoent *ue = u->ents();
		loopi(u->numents) pasteundoent(ue[i].i, ue[i].e);*/
		}
		void worldeditor::detachnode(node &n, bool ophanchildren)
		{
			//use this to ophan a node to the root;
			if(!curworld->getnodefromid(n.parent)) return;
			if(ophanchildren) curworld->removenode(n.children);
			n.parent = NULL; //make it part of the scene
			//make a call to the scene to tell the to add them to the root nodes;
		}
	#pragma endregion
	#pragma region "Octree intergration"
		//fix octree intergration later
		void worldeditor::modifyoctanode(int flags, int id, node &n, cube *c, const ivec &cor, int size, const ivec &bo, const ivec &br, int leafsize, vtxarray *lastva)
		{
			//loopoctabox(cor, size, bo, br)
			//{
			//    ivec o(i, cor, size);
			//    vtxarray *va = c[i].ext && c[i].ext->va ? c[i].ext->va : lastva;
			//    if(c[i].children != NULL && size > leafsize)
			//        modifyoctaentity(flags, id, n, c[i].children, o, size>>1, bo, br, leafsize, va);
			//    else if(flags&MODOE_ADD)
			//    {
			//        if(!c[i].ext || !c[i].ext->ents) ext(c[i]).ents = new octaentities(o, size);
			//        octaentities &oe = *c[i].ext->ents;
			//        switch(n.type)
			//        {
			//            case ET_DECAL:
			//                if(va)
			//                {
			//                    va->bbmin.x = -1;
			//                    if(oe.decals.empty()) va->decals.add(&oe);
			//                }
			//                oe.decals.add(id);
			//                oe.bbmin.min(bo).max(oe.o);
			//                oe.bbmax.max(br).min(ivec(oe.o).add(oe.size));
			//                break;
			//            case ET_MAPMODEL:
			//                if(loadmapmodel(e.attr1))
			//                {
			//                    if(va)
			//                    {
			//                        va->bbmin.x = -1;
			//                        if(oe.mapmodels.empty()) va->mapmodels.add(&oe);
			//                    }
			//                    oe.mapmodels.add(id);
			//                    oe.bbmin.min(bo).max(oe.o);
			//                    oe.bbmax.max(br).min(ivec(oe.o).add(oe.size));
			//                    break;
			//                }
			//                // invisible mapmodel
			//            default:
			//                oe.other.add(id);
			//                break;
			//        }

			//    }
			//    else if(c[i].ext && c[i].ext->ents)
			//    {
			//        octaentities &oe = *c[i].ext->ents;
			//        switch(n.type)
			//        {
			//            case ET_DECAL:
			//                oe.decals.removeobj(id);
			//                if(va)
			//                {
			//                    va->bbmin.x = -1;
			//                    if(oe.decals.empty()) va->decals.removeobj(&oe);
			//                }
			//                oe.bbmin = oe.bbmax = oe.o;
			//                oe.bbmin.add(oe.size);
			//                loopvj(oe.decals)
			//                {
			//                    extentity &e = *entities::getents()[oe.decals[j]];
			//                    ivec eo, er;
			//                    if(getentboundingbox(e, eo, er))
			//                    {
			//                        oe.bbmin.min(eo);
			//                        oe.bbmax.max(er);
			//                    }
			//                }
			//                oe.bbmin.max(oe.o);
			//                oe.bbmax.min(ivec(oe.o).add(oe.size));
			//                break;
			//            case ET_MAPMODEL:
			//                if(loadmapmodel(e.attr1))
			//                {
			//                    oe.mapmodels.removeobj(id);
			//                    if(va)
			//                    {
			//                        va->bbmin.x = -1;
			//                        if(oe.mapmodels.empty()) va->mapmodels.removeobj(&oe);
			//                    }
			//                    oe.bbmin = oe.bbmax = oe.o;
			//                    oe.bbmin.add(oe.size);
			//                    loopvj(oe.mapmodels)
			//                    {
			//                        extentity &e = *entities::getents()[oe.mapmodels[j]];
			//                        ivec eo, er;
			//                        if(getentboundingbox(e, eo, er))
			//                        {
			//                            oe.bbmin.min(eo);
			//                            oe.bbmax.max(er);
			//                        }
			//                    }
			//                    oe.bbmin.max(oe.o);
			//                    oe.bbmax.min(ivec(oe.o).add(oe.size));
			//                    break;
			//                }
			//                // invisible mapmodel
			//            default:
			//                oe.other.removeobj(id);
			//                break;
			//        }
			//        if(oe.mapmodels.empty() && oe.decals.empty() && oe.other.empty())
			//            freeoctaentities(c[i]);
			//    }
			//    if(c[i].ext && c[i].ext->ents) c[i].ext->ents->query = NULL;
			//    if(va && va!=lastva)
			//    {
			//        if(lastva)
			//        {
			//            if(va->bbmin.x < 0) lastva->bbmin.x = -1;
			//        }
			//        else if(flags&MODOE_UPDATEBB) updatevabb(va);
			//    }
			//}
		}
		bool worldeditor::modifyoctanode(int flags, int id, node &n)
		{
			return false;
			//if(flags&MODOE_ADD ? e.flags&EF_OCTA : !(e.flags&EF_OCTA)) return false;

			//ivec o, r;
			//if(!getentboundingbox(e, o, r)) return false;

			//if(!insideworld(e.o))
			//{
			//    int idx = outsideents.find(id);
			//    if(flags&MODOE_ADD)
			//    {
			//        if(idx < 0) outsideents.add(id);
			//    }
			//    else if(idx >= 0) outsideents.removeunordered(idx);
			//}
			//else
			//{
			//    int leafsize = octaentsize, limit = max(r.x - o.x, max(r.y - o.y, r.z - o.z));
			//    while(leafsize < limit) leafsize *= 2;
			//    int diff = ~(leafsize-1) & ((o.x^r.x)|(o.y^r.y)|(o.z^r.z));
			//    if(diff && (limit > octaentsize/2 || diff < leafsize*2)) leafsize *= 2;
			//    modifyoctaentity(flags, id, e, worldroot, ivec(0, 0, 0), worldsize>>1, o, r, leafsize);
			//}
			//e.flags ^= EF_OCTA;
			//switch(e.type)
			//{
			//    case ET_LIGHT: clearlightcache(id); if(e.attr5&L_VOLUMETRIC) { if(flags&MODOE_ADD) volumetriclights++; else --volumetriclights; } break;
			//    case ET_SPOTLIGHT: if(!(flags&MODOE_ADD ? spotlights++ : --spotlights)) { cleardeferredlightshaders(); cleanupvolumetric(); } break;
			//    case ET_PARTICLES: clearparticleemitters(); break;
			//    case ET_DECAL: if(flags&MODOE_CHANGED) changed(o, r, false); break;
			//}
			//return true;
		}
		bool worldeditor::modifyoctanode(int flags, int id)
		{
			return false;
			/*vector<extentity *> &ents = entities::getents();
			return ents.inrange(id) && modifyoctaent(flags, id, *ents[id]);*/
		}
		void worldeditor::addnode(int id)		{ modifyoctanode(MODOE_ADD | MODOE_UPDATEBB, id); }
		void worldeditor::addnodeedit(int id)	{ modifyoctanode(MODOE_ADD | MODOE_UPDATEBB | MODOE_CHANGED, id); }
		void worldeditor::removenode(int id)		{ modifyoctanode(MODOE_UPDATEBB, id); }
		void worldeditor::removenodeedit(int id)	{ modifyoctanode(MODOE_UPDATEBB | MODOE_CHANGED, id); }
		void worldeditor::nodesinocta() 
		{
			/*vector<extentity *> &ents = entities::getents();
			loopv(ents) modifyoctaent(MODOE_ADD, i, *ents[i]);*/
		}
	#pragma endregion (calls used to update and add and remove nodes from the octree to speed up culling)
		int worldeditor::nodehover = -1, worldeditor::oldhover=-1, worldeditor::nodeorient=0, worldeditor::nfocus=-1, worldeditor::nodemoving=-1;
		int worldeditor::orient=0, worldeditor::gridsize=4;
		ivec worldeditor::cor = ivec(), worldeditor::lastcor = ivec(), worldeditor::cur = ivec(), worldeditor::lastcur = ivec();
		bool worldeditor::haveselection = false, worldeditor::hmapselection=false, worldeditor::editmode = true;
		vector<uint> worldeditor::nodeselect = vector<uint>();
		//static vector<selectinfo *> selinfos;
		//static vector<undoblock *> undoblocks;
		int worldeditor::nodelooplevel=0;
		bool worldeditor::undonext=false, worldeditor::nodecanedit=true;
		bool worldeditor::nodeselsnap = false, worldeditor::nodeediting = true;
		vector<uint> octrootselect; //ids to octrees that we are modifying ;)
#pragma endregion (calls from worldeditor struct)



