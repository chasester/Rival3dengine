// world.cpp: core map management stuff

#include "engine.h"

VARR(mapversion, 1, MAPVERSION, 0);
VARNR(mapscale, worldscale, 1, 0, 0);
VARNR(mapsize, worldsize, 1, 0, 0);
SVARR(maptitle, "Untitled Map by Unknown");
VARNR(emptymap, _emptymap, 1, 0, 0);

VAR(octaentsize, 0, 64, 1024);
VAR(entselradius, 0, 2, 10);

//#define getnodesvec vector<node *> &nodes = curscene->getnodes(); //obsolete
//
//
//#pragma region "obsolete"
//static inline void transformbb(const entity &e, vec &center, vec &radius)
//{
//   /* if(e.attr5 > 0) { float scale = e.attr5/100.0f; center.mul(scale); radius.mul(scale); }
//    rotatebb(center, radius, e.attr2, e.attr3, e.attr4);*/
//}
//
//static inline void mmboundbox(const entity &e, model *m, vec &center, vec &radius)
//{
//   /* m->boundbox(center, radius);
//    transformbb(e, center, radius);*/
//}
//
//static inline void mmcollisionbox(const entity &e, model *m, vec &center, vec &radius)
//{
//    /*m->collisionbox(center, radius);
//    transformbb(e, center, radius);*/
//}
//
//static inline void decalboundbox(const entity &e, DecalSlot &s, vec &center, vec &radius)
//{
////    float size = max(float(e.attr5), 1.0f);
////    center = vec(0, s.depth * size/2, 0);
////    radius = vec(size/2, s.depth * size/2, size/2);
////    rotatebb(center, radius, e.attr2, e.attr3, e.attr4);
//}
//
//bool getentboundingbox(const extentity &e, ivec &o, ivec &r)
//{
////    switch(e.type)
////    {
////        case ET_EMPTY:
////			o = vec(e.o).sub(entselradius);
////			r = vec(e.o).add(entselradius + 1);
////			break;
////        case ET_DECAL:
////            {
////                DecalSlot &s = lookupdecalslot(e.attr1, false);
////                vec center, radius;
////                decalboundbox(e, s, center, radius);
////                center.add(e.o);
////                radius.max(entselradius);
////                o = vec(center).sub(radius);
////                r = vec(center).add(radius).add(1);
////                break;
////            }
////        case ET_MAPMODEL:
////            if(model *m = loadmapmodel(e.attr1))
////            {
////                vec center, radius;
////                mmboundbox(e, m, center, radius);
////                center.add(e.o);
////                radius.max(entselradius);
////                o = vec(center).sub(radius);
////                r = vec(center).add(radius).add(1);
////                break;
////            }
////        // invisible mapmodels use entselradius
////        default:
////            o = vec(e.o).sub(entselradius);
////            r = vec(e.o).add(entselradius+1);
////            break;
////    }
////    return true;
//}
//
////enum now defined in node.h
////enum MODOE 
////{
////    MODOE_ADD      = 1<<0,
////    MODOE_UPDATEBB = 1<<1,
////    MODOE_CHANGED  = 1<<2
////};
//
//void modifyoctaentity(int flags, int id, extentity &e, cube *c, const ivec &cor, int size, const ivec &bo, const ivec &br, int leafsize, vtxarray *lastva = NULL)
//{
//    //loopoctabox(cor, size, bo, br)
//    //{
//    //    ivec o(i, cor, size);
//    //    vtxarray *va = c[i].ext && c[i].ext->va ? c[i].ext->va : lastva;
//    //    if(c[i].children != NULL && size > leafsize)
//    //        modifyoctaentity(flags, id, e, c[i].children, o, size>>1, bo, br, leafsize, va);
//    //    else if(flags&MODOE_ADD)
//    //    {
//    //        if(!c[i].ext || !c[i].ext->ents) ext(c[i]).ents = new octaentities(o, size);
//    //        octaentities &oe = *c[i].ext->ents;
//    //        switch(e.type)
//    //        {
//    //            case ET_DECAL:
//    //                if(va)
//    //                {
//    //                    va->bbmin.x = -1;
//    //                    if(oe.decals.empty()) va->decals.add(&oe);
//    //                }
//    //                oe.decals.add(id);
//    //                oe.bbmin.min(bo).max(oe.o);
//    //                oe.bbmax.max(br).min(ivec(oe.o).add(oe.size));
//    //                break;
//    //            case ET_MAPMODEL:
//    //                if(loadmapmodel(e.attr1))
//    //                {
//    //                    if(va)
//    //                    {
//    //                        va->bbmin.x = -1;
//    //                        if(oe.mapmodels.empty()) va->mapmodels.add(&oe);
//    //                    }
//    //                    oe.mapmodels.add(id);
//    //                    oe.bbmin.min(bo).max(oe.o);
//    //                    oe.bbmax.max(br).min(ivec(oe.o).add(oe.size));
//    //                    break;
//    //                }
//    //                // invisible mapmodel
//    //            default:
//    //                oe.other.add(id);
//    //                break;
//    //        }
//
//    //    }
//    //    else if(c[i].ext && c[i].ext->ents)
//    //    {
//    //        octaentities &oe = *c[i].ext->ents;
//    //        switch(e.type)
//    //        {
//    //            case ET_DECAL:
//    //                oe.decals.removeobj(id);
//    //                if(va)
//    //                {
//    //                    va->bbmin.x = -1;
//    //                    if(oe.decals.empty()) va->decals.removeobj(&oe);
//    //                }
//    //                oe.bbmin = oe.bbmax = oe.o;
//    //                oe.bbmin.add(oe.size);
//    //                loopvj(oe.decals)
//    //                {
//    //                    extentity &e = *entities::getents()[oe.decals[j]];
//    //                    ivec eo, er;
//    //                    if(getentboundingbox(e, eo, er))
//    //                    {
//    //                        oe.bbmin.min(eo);
//    //                        oe.bbmax.max(er);
//    //                    }
//    //                }
//    //                oe.bbmin.max(oe.o);
//    //                oe.bbmax.min(ivec(oe.o).add(oe.size));
//    //                break;
//    //            case ET_MAPMODEL:
//    //                if(loadmapmodel(e.attr1))
//    //                {
//    //                    oe.mapmodels.removeobj(id);
//    //                    if(va)
//    //                    {
//    //                        va->bbmin.x = -1;
//    //                        if(oe.mapmodels.empty()) va->mapmodels.removeobj(&oe);
//    //                    }
//    //                    oe.bbmin = oe.bbmax = oe.o;
//    //                    oe.bbmin.add(oe.size);
//    //                    loopvj(oe.mapmodels)
//    //                    {
//    //                        extentity &e = *entities::getents()[oe.mapmodels[j]];
//    //                        ivec eo, er;
//    //                        if(getentboundingbox(e, eo, er))
//    //                        {
//    //                            oe.bbmin.min(eo);
//    //                            oe.bbmax.max(er);
//    //                        }
//    //                    }
//    //                    oe.bbmin.max(oe.o);
//    //                    oe.bbmax.min(ivec(oe.o).add(oe.size));
//    //                    break;
//    //                }
//    //                // invisible mapmodel
//    //            default:
//    //                oe.other.removeobj(id);
//    //                break;
//    //        }
//    //        if(oe.mapmodels.empty() && oe.decals.empty() && oe.other.empty())
//    //            freeoctaentities(c[i]);
//    //    }
//    //    if(c[i].ext && c[i].ext->ents) c[i].ext->ents->query = NULL;
//    //    if(va && va!=lastva)
//    //    {
//    //        if(lastva)
//    //        {
//    //            if(va->bbmin.x < 0) lastva->bbmin.x = -1;
//    //        }
//    //        else if(flags&MODOE_UPDATEBB) updatevabb(va);
//    //    }
//    //}
//}
//
////vector<int> outsideents;
////int spotlights = 0, volumetriclights = 0;
//
//static bool modifyoctaent(int flags, int id, extentity &e)
//{
//	return false;
//    //if(flags&MODOE_ADD ? e.flags&EF_OCTA : !(e.flags&EF_OCTA)) return false;
//
//    //ivec o, r;
//    //if(!getentboundingbox(e, o, r)) return false;
//
//    //if(!insideworld(e.o))
//    //{
//    //    int idx = outsideents.find(id);
//    //    if(flags&MODOE_ADD)
//    //    {
//    //        if(idx < 0) outsideents.add(id);
//    //    }
//    //    else if(idx >= 0) outsideents.removeunordered(idx);
//    //}
//    //else
//    //{
//    //    int leafsize = octaentsize, limit = max(r.x - o.x, max(r.y - o.y, r.z - o.z));
//    //    while(leafsize < limit) leafsize *= 2;
//    //    int diff = ~(leafsize-1) & ((o.x^r.x)|(o.y^r.y)|(o.z^r.z));
//    //    if(diff && (limit > octaentsize/2 || diff < leafsize*2)) leafsize *= 2;
//    //    modifyoctaentity(flags, id, e, worldroot, ivec(0, 0, 0), worldsize>>1, o, r, leafsize);
//    //}
//    //e.flags ^= EF_OCTA;
//    //switch(e.type)
//    //{
//    //    case ET_LIGHT: clearlightcache(id); if(e.attr5&L_VOLUMETRIC) { if(flags&MODOE_ADD) volumetriclights++; else --volumetriclights; } break;
//    //    case ET_SPOTLIGHT: if(!(flags&MODOE_ADD ? spotlights++ : --spotlights)) { cleardeferredlightshaders(); cleanupvolumetric(); } break;
//    //    case ET_PARTICLES: clearparticleemitters(); break;
//    //    case ET_DECAL: if(flags&MODOE_CHANGED) changed(o, r, false); break;
//    //}
//    //return true;
//}
//
//static inline bool modifyoctaent(int flags, int id)
//{
//	return false;
//    /*vector<extentity *> &ents = entities::getents();
//    return ents.inrange(id) && modifyoctaent(flags, id, *ents[id]);*/
//}
//
//static inline void addentity(int id)        { /*modifyoctaent(MODOE_ADD|MODOE_UPDATEBB, id);*/ }
//static inline void addentityedit(int id)    { /*modifyoctaent(MODOE_ADD|MODOE_UPDATEBB|MODOE_CHANGED, id);*/ }
//static inline void removeentity(int id)     { /*modifyoctaent(MODOE_UPDATEBB, id);*/ }
//static inline void removeentityedit(int id) { /*modifyoctaent(MODOE_UPDATEBB|MODOE_CHANGED, id);*/ }
//
//void freeoctaentities(cube &c)
//{
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
//}
//
//void entitiesinoctanodes()
//{
//    /*vector<extentity *> &ents = entities::getents();
//    loopv(ents) modifyoctaent(MODOE_ADD, i, *ents[i]);*/
//}
//
//static inline void findents(octaentities &oe, int low, int high, bool notspawned, const vec &pos, const vec &invradius, vector<int> &found)
//{
//    /*vector<extentity *> &ents = entities::getents();
//    loopv(oe.other)
//    {
//        int id = oe.other[i];
//        extentity &e = *ents[id];
//        if(e.type >= low && e.type <= high && (e.spawned() || notspawned) && vec(e.o).sub(pos).mul(invradius).squaredlen() <= 1) found.add(id);
//    }*/
//}
//
//static inline void findents(cube *c, const ivec &o, int size, const ivec &bo, const ivec &br, int low, int high, bool notspawned, const vec &pos, const vec &invradius, vector<int> &found)
//{
//    /*loopoctabox(o, size, bo, br)
//    {
//        if(c[i].ext && c[i].ext->ents) findents(*c[i].ext->ents, low, high, notspawned, pos, invradius, found);
//        if(c[i].children && size > octaentsize)
//        {
//            ivec co(i, o, size);
//            findents(c[i].children, co, size>>1, bo, br, low, high, notspawned, pos, invradius, found);
//        }
//    }*/
//}
//
//void findents(int low, int high, bool notspawned, const vec &pos, const vec &radius, vector<int> &found)
//{
//    /*vec invradius(1/radius.x, 1/radius.y, 1/radius.z);
//    ivec bo = vec(pos).sub(radius).sub(1),
//         br = vec(pos).add(radius).add(1);
//    int diff = (bo.x^br.x) | (bo.y^br.y) | (bo.z^br.z) | octaentsize,
//        scale = worldscale-1;
//    if(diff&~((1<<scale)-1) || uint(bo.x|bo.y|bo.z|br.x|br.y|br.z) >= uint(worldsize))
//    {
//        findents(worldroot, ivec(0, 0, 0), 1<<scale, bo, br, low, high, notspawned, pos, invradius, found);
//        return;
//    }
//    cube *c = &worldroot[octastep(bo.x, bo.y, bo.z, scale)];
//    if(c->ext && c->ext->ents) findents(*c->ext->ents, low, high, notspawned, pos, invradius, found);
//    scale--;
//    while(c->children && !(diff&(1<<scale)))
//    {
//        c = &c->children[octastep(bo.x, bo.y, bo.z, scale)];
//        if(c->ext && c->ext->ents) findents(*c->ext->ents, low, high, notspawned, pos, invradius, found);
//        scale--;
//    }
//    if(c->children && 1<<scale >= octaentsize) findents(c->children, ivec(bo).mask(~((2<<scale)-1)), 1<<scale, bo, br, low, high, notspawned, pos, invradius, found);*/
//}
//
////obsolete
////char *entname(entity &e)
////{
////    static cubestr fullentname;
////    copystring(fullentname, entities::entname(e.type));
////    const char *einfo = entities::entnameinfo(e);
////    if(*einfo)
////    {
////        concatstring(fullentname, ": ");
////        concatstring(fullentname, einfo);
////    }
////    return fullentname;
////} 
//
////obsolete
//
////extern bool havesel;
////int entlooplevel = 0;
////int efocus = -1, enthover = -1, entorient = -1, oldhover = -1;
////bool undonext = true;

//change from ent to node later
VARF(nodeediting, 0, 0, 1, { worldeditor::isnodeedit();}); //change to a toggle

//undoblock *newundoent()
//{
//   /* int numents = entgroup.length();
//    if(numents <= 0) return NULL;
//    undoblock *u = (undoblock *)new uchar[sizeof(undoblock) + numents*sizeof(undoent)];
//    u->numents = numents;
//    undoent *e = (undoent *)(u + 1);
//    loopv(entgroup)
//    {
//        e->i = entgroup[i];
//        e->e = *entities::getents()[entgroup[i]];
//        e++;
//    }
//    return u;*/
//}
//
//void makeundoent()
//{
//
//   /* if(!undonext) return;
//    undonext = false;
//    oldhover = enthover;
//    undoblock *u = newundoent();
//    if(u) addundo(u);*/
//}
//
//void detachentity(extentity &e)
//{
//   /* if(!e.attached) return;
//    e.attached->attached = NULL;
//    e.attached = NULL;*/
//}

VAR(attachradius, 1, 100, 1000);

//void attachentity(extentity &e)
//{
//   /* switch(e.type)
//    {
//        case ET_SPOTLIGHT:
//            break;
//
//        default:
//            if(e.type<ET_GAMESPECIFIC || !entities::mayattach(e)) return;
//            break;
//    }
//
//    detachentity(e);
//
//    vector<extentity *> &ents = entities::getents();
//    int closest = -1;
//    float closedist = 1e10f;
//    loopv(ents)
//    {
//        extentity *a = ents[i];
//        if(a->attached) continue;
//        switch(e.type)
//        {
//            case ET_SPOTLIGHT:
//                if(a->type!=ET_LIGHT) continue;
//                break;
//
//            default:
//                if(e.type<ET_GAMESPECIFIC || !entities::attachent(e, *a)) continue;
//                break;
//        }
//        float dist = e.o.dist(a->o);
//        if(dist < closedist)
//        {
//            closest = i;
//            closedist = dist;
//        }
//    }
//    if(closedist>attachradius) return;
//    e.attached = ents[closest];
//    ents[closest]->attached = &e;*/
//}
//
//void attachentities()
//{
//    /*vector<extentity *> &ents = entities::getents();
//    loopv(ents) attachentity(*ents[i]);*/
//}
//
//// convenience macros implicitly define:
//// e         entity, currently edited ent
//// n         int,    index to currently edited ent
//
////dont use these confusing macros to make code more clear. Maybe add simpiler macros later
////#define addimplicit(f)    { if(entgroup.empty() && enthover>=0) { entadd(enthover); undonext = (enthover != oldhover); f; entgroup.drop(); } else f; }
////#define entfocusv(i, f, v)
////#define entfocus(i, f)    entfocusv(i, f, entities::getents())
////#define enteditv(i, f, v) \
////{ \
////    entfocusv(i, \
////    { \
////        int oldtype = e.type; \
////        removeentityedit(n);  \
////        f; \
////        if(oldtype!=e.type) detachentity(e); \
////        if(e.type!=ET_EMPTY) { addentityedit(n); if(oldtype!=e.type) attachentity(e); } \
////        entities::editent(n, true); \
////        clearshadowcache(); \
////    }, v); \
////}
////#define entedit(i, f)   enteditv(i, f, entities::getents())
////#define addgroup(exp)   { vector<extentity *> &ents = entities::getents(); loopv(ents) entfocusv(i, if(exp) entadd(n), ents); }
////#define setgroup(exp)   { entcancel(); addgroup(exp); }
////#define groupeditloop(f){ vector<extentity *> &ents = entities::getents(); entlooplevel++; int _ = efocus; loopv(entgroup) enteditv(entgroup[i], f, ents); efocus = _; entlooplevel--; }
////#define groupeditpure(f){ if(entlooplevel>0) { entedit(efocus, f); } else { groupeditloop(f); commitchanges(); } }
////#define groupeditundo(f){ makeundoent(); groupeditpure(f); }
////#define groupedit(f)    { addimplicit(groupeditundo(f)); }
/////////
//
//vec getselpos()
//{
//   /* vector<extentity *> &ents = entities::getents();
//    if(entgroup.length() && ents.inrange(entgroup[0])) return ents[entgroup[0]]->o;
//    if(ents.inrange(enthover)) return ents[enthover]->o;
//    return vec(sel.o);*/
//}
//
//undoblock *copyundoents(undoblock *u)
//{
//	/* entcancel();
//	undoent *e = u->ents();
//	loopi(u->numents)
//	entadd(e[i].i);
//	undoblock *c = newundoent();
//	loopi(u->numents) if(e[i].e.type==ET_EMPTY)
//	entgroup.removeobj(e[i].i);
//	return c;*/
//	return NULL;
//}
//
//void pasteundoent(int idx, const entity &ue)
//{
//   /* if(idx < 0 || idx >= MAXENTS) return;
//    vector<extentity *> &ents = entities::getents();
//    while(ents.length() < idx) ents.add(entities::newentity())->type = ET_EMPTY;
//    int efocus = -1;
//    entedit(idx, (entity &)e = ue);*/
//}
//
//void pasteundoents(undoblock *u)
//{
//   /* undoent *ue = u->ents();
//    loopi(u->numents) pasteundoent(ue[i].i, ue[i].e);*/
//}
//
//void entflip()
//{
//    /*if(nonodeedit()) return;
//    int d = dimension(sel.orient);
//    float mid = sel.s[d]*sel.grid/2+sel.o[d];
//    groupeditundo(e.o[d] -= (e.o[d]-mid)*2);*/
//}
//
//void entrotate(int *cw)
//{
//   /* if(nonodeedit()) return;
//    int d = dimension(sel.orient);
//    int dd = (*cw<0) == dimcoord(sel.orient) ? R[d] : C[d];
//    float mid = sel.s[dd]*sel.grid/2+sel.o[dd];
//    vec s(sel.o.v);
//    groupeditundo(
//        e.o[dd] -= (e.o[dd]-mid)*2;
//        e.o.sub(s);
//        swap(e.o[R[d]], e.o[C[d]]);
//        e.o.add(s);
//    );*/
//}
//
//void entselectionbox(const entity &e, vec &eo, vec &es)
//{
//    //model *m = NULL;
//    //const char *mname = entities::entmodel(e);
//    //if(mname && (m = loadmodel(mname)))
//    //{
//    //    m->collisionbox(eo, es);
//    //    if(es.x > es.y) es.y = es.x; else es.x = es.y; // square
//    //    es.z = (es.z + eo.z + 1 + entselradius)/2; // enclose ent radius box and model box
//    //    eo.x += e.o.x;
//    //    eo.y += e.o.y;
//    //    eo.z = e.o.z - entselradius + es.z;
//    //}
//    //else if(e.type == ET_MAPMODEL && (m = loadmapmodel(e.attr1)))
//    //{
//    //    mmcollisionbox(e, m, eo, es);
//    //    es.max(entselradius);
//    //    eo.add(e.o);
//    //}
//    //else if(e.type == ET_DECAL)
//    //{
//    //    DecalSlot &s = lookupdecalslot(e.attr1, false);
//    //    decalboundbox(e, s, eo, es);
//    //    es.max(entselradius);
//    //    eo.add(e.o);
//    //}
//    //else
//    //{
//    //    es = vec(entselradius);
//    //    eo = e.o;
//    //}
//    //eo.sub(es);
//    //es.mul(2);
//}

VAR(entselsnap, 0, 0, 1);
VAR(nodemovingshadow, 0, 1, 1);

//rendering functionality
extern void boxs(int orient, vec o, const vec &s); 
extern void boxs3D(const vec &o, vec s, int g);
extern bool editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first);

//void entdrag(const vec &ray)
//{
//   /* if(nonodeedit() || !haveselent()) return;
//
//    float r = 0, c = 0;
//    static vec dest, handle;
//    vec eo, es;
//    int d = dimension(entorient),
//        dc= dimcoord(entorient);
//
//    entfocus(entgroup.last(),
//        entselectionbox(e, eo, es);
//
//        if(!editmoveplane(e.o, ray, d, eo[d] + (dc ? es[d] : 0), handle, dest, entmoving==1))
//            return;
//
//        ivec g = dest;
//        int z = g[d]&(~(sel.grid-1));
//        g.add(sel.grid/2).mask(~(sel.grid-1));
//        g[d] = z;
//
//        r = (entselsnap ? g[R[d]] : dest[R[d]]) - e.o[R[d]];
//        c = (entselsnap ? g[C[d]] : dest[C[d]]) - e.o[C[d]];
//    );
//
//    if(entmoving==1) makeundoent();
//    groupeditpure(e.o[R[d]] += r; e.o[C[d]] += c);
//    entmoving = 2;*/
//}

VAR(showentradius, 0, 1, 1);

//void renderentring(const extentity &e, float radius, int axis)
//{
//    /*if(radius <= 0) return;
//    gle::defvertex();
//    gle::begin(GL_LINE_LOOP);
//    loopi(15)
//    {
//        vec p(e.o);
//        const vec2 &sc = sincos360[i*(360/15)];
//        p[axis>=2 ? 1 : 0] += radius*sc.x;
//        p[axis>=1 ? 2 : 1] += radius*sc.y;
//        gle::attrib(p);
//    }
//    xtraverts += gle::end();*/
//}
//
//void renderentsphere(const extentity &e, float radius)
//{
//   /* if(radius <= 0) return;
//    loopk(3) renderentring(e, radius, k);*/
//}
//
//void renderentattachment(const extentity &e)
//{
//  /*  if(!e.attached) return;
//    gle::defvertex();
//    gle::begin(GL_LINES);
//    gle::attrib(e.o);
//    gle::attrib(e.attached->o);
//    xtraverts += gle::end();*/
//}
//
//void renderentarrow(const extentity &e, const vec &dir, float radius)
//{
//   /* if(radius <= 0) return;
//    float arrowsize = min(radius/2, 0.5f);
//    vec target = vec(dir).mul(radius).add(e.o), arrowbase = vec(dir).mul(radius - arrowsize).add(e.o), spoke;
//    spoke.orthogonal(dir);
//    spoke.normalize();
//    spoke.mul(arrowsize);
//
//    gle::defvertex();
//
//    gle::begin(GL_LINES);
//    gle::attrib(e.o);
//    gle::attrib(target);
//    xtraverts += gle::end();
//
//    gle::begin(GL_TRIANGLE_FAN);
//    gle::attrib(target);
//    loopi(5) gle::attrib(vec(spoke).rotate(2*M_PI*i/4.0f, dir).add(arrowbase));
//    xtraverts += gle::end();*/
//}
//
//void renderentcone(const extentity &e, const vec &dir, float radius, float angle)
//{
//    /*if(radius <= 0) return;
//    vec spot = vec(dir).mul(radius*cosf(angle*RAD)).add(e.o), spoke;
//    spoke.orthogonal(dir);
//    spoke.normalize();
//    spoke.mul(radius*sinf(angle*RAD));
//
//    gle::defvertex();
//
//    gle::begin(GL_LINES);
//    loopi(8)
//    {
//        gle::attrib(e.o);
//        gle::attrib(vec(spoke).rotate(2*M_PI*i/8.0f, dir).add(spot));
//    }
//    xtraverts += gle::end();
//
//    gle::begin(GL_LINE_LOOP);
//    loopi(8) gle::attrib(vec(spoke).rotate(2*M_PI*i/8.0f, dir).add(spot));
//    xtraverts += gle::end();*/
//}
//
//void renderentbox(const extentity &e, const vec &center, const vec &radius, int yaw, int pitch, int roll)
//{
//  /*  matrix4x3 orient;
//    orient.identity();
//    orient.settranslation(e.o);
//    if(yaw) orient.rotate_around_z(sincosmod360(yaw));
//    if(pitch) orient.rotate_around_x(sincosmod360(pitch));
//    if(roll) orient.rotate_around_y(sincosmod360(-roll));
//    orient.translate(center);
//
//    gle::defvertex();
//
//    vec front[4] = { vec(-radius.x, -radius.y, -radius.z), vec( radius.x, -radius.y, -radius.z), vec( radius.x, -radius.y,  radius.z), vec(-radius.x, -radius.y,  radius.z) },
//        back[4] = { vec(-radius.x, radius.y, -radius.z), vec( radius.x, radius.y, -radius.z), vec( radius.x, radius.y,  radius.z), vec(-radius.x, radius.y,  radius.z) };
//    loopi(4)
//    {
//        front[i] = orient.transform(front[i]);
//        back[i] = orient.transform(back[i]);
//    }
//
//    gle::begin(GL_LINE_LOOP);
//    loopi(4) gle::attrib(front[i]);
//    xtraverts += gle::end();
//
//    gle::begin(GL_LINES);
//    gle::attrib(front[0]);
//        gle::attrib(front[2]);
//    gle::attrib(front[1]);
//        gle::attrib(front[3]);
//    loopi(4)
//    {
//        gle::attrib(front[i]);
//        gle::attrib(back[i]);
//    }
//    xtraverts += gle::end();
//
//    gle::begin(GL_LINE_LOOP);
//    loopi(4) gle::attrib(back[i]);
//    xtraverts += gle::end();*/
//}
//
//void renderentradius(extentity &e, bool color)
//{
//   /* switch(e.type)
//    {
//        case ET_LIGHT:
//            if(e.attr1 <= 0) break;
//            if(color) gle::colorf(e.attr2/255.0f, e.attr3/255.0f, e.attr4/255.0f);
//            renderentsphere(e, e.attr1);
//            break;
//
//        case ET_SPOTLIGHT:
//            if(e.attached)
//            {
//                if(color) gle::colorf(0, 1, 1);
//                float radius = e.attached->attr1;
//                if(radius <= 0) break;
//                vec dir = vec(e.o).sub(e.attached->o).normalize();
//                float angle = clamp(int(e.attr1), 1, 89);
//                renderentattachment(e);
//                renderentcone(*e.attached, dir, radius, angle);
//            }
//            break;
//
//        case ET_SOUND:
//            if(color) gle::colorf(0, 1, 1);
//            renderentsphere(e, e.attr2);
//            break;
//
//        case ET_ENVMAP:
//        {
//            extern int envmapradius;
//            if(color) gle::colorf(0, 1, 1);
//            renderentsphere(e, e.attr1 ? max(0, min(10000, int(e.attr1))) : envmapradius);
//            break;
//        }
//
//        case ET_MAPMODEL:
//        {
//            if(color) gle::colorf(0, 1, 1);
//            entities::entradius(e, color);
//            vec dir;
//            vecfromyawpitch(e.attr2, e.attr3, 1, 0, dir);
//            renderentarrow(e, dir, 4);
//            break;
//        }
//
//        case ET_PLAYERSTART:
//        {
//            if(color) gle::colorf(0, 0, 1);
//            entities::entradius(e, color);
//            vec dir;
//            vecfromyawpitch(e.attr1, 0, 1, 0, dir);
//            renderentarrow(e, dir, 4);
//            break;
//        }
//
//        case ET_DECAL:
//        {
//            if(color) gle::colorf(0, 1, 1);
//            DecalSlot &s = lookupdecalslot(e.attr1, false);
//            float size = max(float(e.attr5), 1.0f);
//            renderentbox(e, vec(0, s.depth * size/2, 0), vec(size/2, s.depth * size/2, size/2), e.attr2, e.attr3, e.attr4);
//            break;
//        }
//
//        default:
//            if(e.type>=ET_GAMESPECIFIC)
//            {
//                if(color) gle::colorf(0, 1, 1);
//                entities::entradius(e, color);
//            }
//            break;
//    }*/
//}
//
//void renderentbox(const vec &eo, vec es)
//{
////    es.add(eo);
////
////    // bottom quad
////    gle::attrib(eo.x, eo.y, eo.z); gle::attrib(es.x, eo.y, eo.z);
////    gle::attrib(es.x, eo.y, eo.z); gle::attrib(es.x, es.y, eo.z);
////    gle::attrib(es.x, es.y, eo.z); gle::attrib(eo.x, es.y, eo.z);
////    gle::attrib(eo.x, es.y, eo.z); gle::attrib(eo.x, eo.y, eo.z);
////
////    // top quad
////    gle::attrib(eo.x, eo.y, es.z); gle::attrib(es.x, eo.y, es.z);
////    gle::attrib(es.x, eo.y, es.z); gle::attrib(es.x, es.y, es.z);
////    gle::attrib(es.x, es.y, es.z); gle::attrib(eo.x, es.y, es.z);
////    gle::attrib(eo.x, es.y, es.z); gle::attrib(eo.x, eo.y, es.z);
////
////    // sides
////    gle::attrib(eo.x, eo.y, eo.z); gle::attrib(eo.x, eo.y, es.z);
////    gle::attrib(es.x, eo.y, eo.z); gle::attrib(es.x, eo.y, es.z);
////    gle::attrib(es.x, es.y, eo.z); gle::attrib(es.x, es.y, es.z);
////    gle::attrib(eo.x, es.y, eo.z); gle::attrib(eo.x, es.y, es.z);
//}
//
//void renderentselection(const vec &o, const vec &ray, bool entmoving)
//{
//	//if (nonodeedit())return;// || (entgroup.empty() && enthover < 0)) return;
// //   vec eo, es;
//
//	////xtraverts += entities::rendereditent();
//	//if (entgroup.empty() && enthover < 0) return;
// //   if(entgroup.length())
// //   {
// //       gle::colorub(0, 40, 0);
// //       gle::defvertex();
// //       gle::begin(GL_LINES, entgroup.length()*24);
// //       loopv(entgroup) entfocus(entgroup[i],
// //           entselectionbox(e, eo, es);
// //           renderentbox(eo, es);
// //       );
// //       xtraverts += gle::end();
// //   }
//
//	//
//
// //   if(enthover >= 0)
// //   {
// //       gle::colorub(0, 40, 0);
// //       entfocus(enthover, entselectionbox(e, eo, es)); // also ensures enthover is back in focus
// //       boxs3D(eo, es, 1);
// //       if(entmoving && entmovingshadow==1)
// //       {
// //           vec a, b;
// //           gle::colorub(20, 20, 20);
// //           (a = eo).x = eo.x - fmod(eo.x, worldsize); (b = es).x = a.x + worldsize; boxs3D(a, b, 1);
// //           (a = eo).y = eo.y - fmod(eo.y, worldsize); (b = es).y = a.x + worldsize; boxs3D(a, b, 1);
// //           (a = eo).z = eo.z - fmod(eo.z, worldsize); (b = es).z = a.x + worldsize; boxs3D(a, b, 1);
// //       }
// //       gle::colorub(200,0,0);
// //       boxs(entorient, eo, es);
// //   }
//
// //   if(showentradius)
// //   {
// //       glDepthFunc(GL_GREATER);
// //       gle::colorf(0.25f, 0.25f, 0.25f);
// //       loopv(entgroup) entfocus(entgroup[i], renderentradius(e, false));
// //       if(enthover>=0) entfocus(enthover, renderentradius(e, false));
// //       glDepthFunc(GL_LESS);
// //       loopv(entgroup) entfocus(entgroup[i], renderentradius(e, true));
// //       if(enthover>=0) entfocus(enthover, renderentradius(e, true));
// //   }
//
// //   gle::disable();
//}
//
//bool enttoggle(int id)
//{
//  /*  undonext = true;
//    int i = curworld->nodeselect.find(id);
//    if(i < 0)
//        entadd(id);
//    else
//        curworld->nodeselect.remove(i);
//    return i < 0;*/
//}
//
//bool hoveringonent(int ent, int orient)
//{
//    /*if(nonodeedit()) return false;
//    entorient = orient;
//    if((efocus = enthover = ent) >= 0)
//        return true;
//    efocus   = entgroup.empty() ? -1 : entgroup.last();
//    enthover = -1;
//    return false;*/
//}

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

//void entpush(int *dir)
//{
//    //if(nonodeedit()) return;
//    //int d = dimension(entorient);
//    //int s = dimcoord(entorient) ? -*dir : *dir;
//    //if(entmoving)
//    //{
//    //    groupeditpure(e.o[d] += float(s*sel.grid)); // editdrag supplies the undo
//    //}
//    //else
//    //    groupedit(e.o[d] += float(s*sel.grid));
//    //if(entitysurf==1)
//    //{
//    //    player->o[d] += float(s*sel.grid);
//    //    player->resetinterp();
//    //}
//}

VAR(nodeautoviewdist, 0, 25, 100);
//void entautoview(int *dir)
//{
//    /*if(!haveselent()) return;
//    static int s = 0;
//    vec v(player->o);
//    v.sub(worldpos);
//    v.normalize();
//    v.mul(entautoviewdist);
//    int t = s + *dir;
//    s = abs(t) % entgroup.length();
//    if(t<0 && s>0) s = entgroup.length() - s;
//    entfocus(entgroup[s],
//        v.add(e.o);
//        player->o = v;
//        player->resetinterp();
//    );*/
//}
//
//COMMAND(entautoview, "i");
//COMMAND(entflip, "");
//COMMAND(entrotate, "i");
//COMMAND(entpush, "i");

//void delent()
//{
//   /* if(nonodeedit()) return;
//    groupedit(e.type = ET_EMPTY;);
//    entcancel();*/
//}
//
//int findtype(char *what)
//{
//    /*for(int i = 0; *entities::entname(i); i++) if(strcmp(what, entities::entname(i))==0) return i;
//    conoutf(CON_ERROR, "unknown entity type \"%s\"", what);*/
//    return ET_EMPTY;
//}

VAR(entdrop, 0, 2, 3);

//bool dropentity(entity &e, int drop = -1)
//{
//   /* vec radius(4.0f, 4.0f, 4.0f);
//    if(drop<0) drop = entdrop;
//    if(e.type == ET_MAPMODEL)
//    {
//        model *m = loadmapmodel(e.attr1);
//        if(m)
//        {
//            vec center;
//            mmboundbox(e, m, center, radius);
//            radius.x += fabs(center.x);
//            radius.y += fabs(center.y);
//        }
//        radius.z = 0.0f;
//    }
//    switch(drop)
//    {
//    case 1:
//        if(e.type != ET_LIGHT && e.type != ET_SPOTLIGHT)
//            dropenttofloor(&e);
//        break;
//    case 2:
//    case 3:
//        int cx = 0, cy = 0;
//        if(sel.cxs == 1 && sel.cys == 1)
//        {
//            cx = (sel.cx ? 1 : -1) * sel.grid / 2;
//            cy = (sel.cy ? 1 : -1) * sel.grid / 2;
//        }
//        e.o = vec(sel.o);
//        int d = dimension(sel.orient), dc = dimcoord(sel.orient);
//        e.o[R[d]] += sel.grid / 2 + cx;
//        e.o[C[d]] += sel.grid / 2 + cy;
//        if(!dc)
//            e.o[D[d]] -= radius[D[d]];
//        else
//            e.o[D[d]] += sel.grid + radius[D[d]];
//
//        if(drop == 3)
//            dropenttofloor(&e);
//        break;
//    }*/
//	//retrun true;
//    return false;
//}
//
//void dropent()
//{
//   /* if(nonodeedit()) return;
//    groupedit(dropentity(e));*/
//}
//
//void attachent()
//{
//   /* if(nonodeedit()) return;
//    groupedit(attachentity(e));*/
//}COMMAND(attachent, "");
//
//static int keepents = 0; //posibly the most ents that can be kept at once
//
//extentity *newentity(bool local, const vec &o, int type, int v1, int v2, int v3, int v4, int v5, int &idx, bool fix = true)
//{
//    //vector<extentity *> &ents = entities::getents();
//    //if(local)
//    //{
//    //    idx = -1;
//    //   for(int i = keepents; i < ents.length(); i++) if(false && ents[i]->type == ET_EMPTY) { idx = i; break; }
//    //    if(idx < 0 && ents.length() >= MAXENTS) { conoutf("too many entities"); return NULL; }
//    //}
//    //else while(ents.length() < idx) ents.add(entities::newentity())->type = ET_EMPTY;
//    //extentity &e = *entities::newentity();
//    //e.o = o;
//    //e.attr1 = v1;
//    //e.attr2 = v2;
//    //e.attr3 = v3;
//    //e.attr4 = v4;
//    //e.attr5 = v5;
//    //e.type = type;
//    //e.reserved = 0;
//    //if(local && fix)
//    //{
//    //    switch(type)
//    //    {
//    //            case ET_DECAL:
//    //                if(!e.attr2 && !e.attr3 && !e.attr4)
//    //                {
//    //                    e.attr2 = (int)camera1->yaw;
//    //                    e.attr3 = (int)camera1->pitch;
//    //                    e.attr4 = (int)camera1->roll;
//    //                }
//    //                break;
//    //            case ET_MAPMODEL:
//    //                if(!e.attr2) e.attr2 = (int)camera1->yaw;
//    //                break;
//    //            case ET_PLAYERSTART:
//    //                e.attr5 = e.attr4;
//    //                e.attr4 = e.attr3;
//    //                e.attr3 = e.attr2;
//    //                e.attr2 = e.attr1;
//    //                e.attr1 = (int)camera1->yaw;
//    //                break;
//    //    }
//    //    entities::fixentity(e);
//    //}
//    //if(ents.inrange(idx)) { entities::deleteentity(ents[idx]); ents[idx] = &e; }
//    //else { idx = ents.length(); ents.add(&e); }
//    //return &e;
//	return NULL;
//}
//
//void newentity(int type, int a1, int a2, int a3, int a4, int a5, bool fix = true)
//{
//   /* int idx;
//    extentity *t = newentity(true, player->o, type, a1, a2, a3, a4, a5, idx, fix);
//    if(!t) return;
//    dropentity(*t);
//    t->type = ET_EMPTY;
//    enttoggle(idx);
//    makeundoent();
//    entedit(idx, e.type = type);
//    commitchanges();*/
//}
//
//void newent(char *what, int *a1, int *a2, int *a3, int *a4, int *a5)
//{
//    //if(nonodeedit()) return;
//    //int type = findtype(what);
//    //if(type != ET_EMPTY)
//    //    newentity(type, *a1, *a2, *a3, *a4, *a5);
//}
//
//
////int entcopygrid;
////vector<entity> entcopybuf;
//
//void entcopy()
//{
//  /*  if(nonodeedit()) return;
//    entcopygrid = sel.grid;
//    entcopybuf.shrink(0);
//    addimplicit({
//        loopv(entgroup) entfocus(entgroup[i], entcopybuf.add(e).o.sub(vec(sel.o)));
//    });*/
//}
//
//void entpaste()
//{
//    //if(nonodeedit() || entcopybuf.empty()) return;
//    //entcancel();
//    //float m = float(sel.grid)/float(entcopygrid);
//    //loopv(entcopybuf)
//    //{
//    //    const entity &c = entcopybuf[i];
//    //    vec o = vec(c.o).mul(m).add(vec(sel.o));
//    //    int idx;
//    //    extentity *e = newentity(true, o, ET_EMPTY, c.attr1, c.attr2, c.attr3, c.attr4, c.attr5, idx);
//    //    if(!e) continue;
//    //    entadd(idx);
//    //    keepents = max(keepents, idx+1);
//    //}
//    //keepents = 0;
//    //int j = 0;
//    //groupeditundo(e.type = entcopybuf[j++].type;);
//}
//
//void entreplace()
//{
//    /*if(nonodeedit() || entcopybuf.empty()) return;
//    const entity &c = entcopybuf[0];
//    if(entgroup.length() || enthover >= 0)
//    {
//        groupedit({
//            e.type = c.type;
//            e.attr1 = c.attr1;
//            e.attr2 = c.attr2;
//            e.attr3 = c.attr3;
//            e.attr4 = c.attr4;
//            e.attr5 = c.attr5;
//        });
//    }
//    else
//    {
//        newentity(c.type, c.attr1, c.attr2, c.attr3, c.attr4, c.attr5, false);
//    }*/
//}
//
//COMMAND(newent, "siiiii");
//COMMAND(delent, "");
//COMMAND(dropent, "");
//COMMAND(entcopy, "");
//COMMAND(entpaste, "");
//COMMAND(entreplace, "");
//
//void entset(char *what, int *a1, int *a2, int *a3, int *a4, int *a5)
//{
//  /*  if(nonodeedit()) return;
//    int type = findtype(what);
//    if(type != ET_EMPTY)
//        groupedit(e.type=type;
//                  e.attr1=*a1;
//                  e.attr2=*a2;
//                  e.attr3=*a3;
//                  e.attr4=*a4;
//                  e.attr5=*a5);*/
//}
//
//void printent(extentity &e, char *buf, int len)
//{
//    /*switch(e.type)
//    {
//        case ET_PARTICLES:
//            if(printparticles(e, buf, len)) return;
//            break;
//
//        default:
//            if(e.type >= ET_GAMESPECIFIC && entities::printent(e, buf, len)) return;
//            break;
//    }
//    nformatstring(buf, len, "%s %d %d %d %d %d", entities::entname(e.type), e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);*/
//}
//
//void nearestent()
//{
//   /* if(nonodeedit()) return;
//    int closest = -1;
//    float closedist = 1e16f;
//	vector<node *> nodes = curworld->curscene->getnodes();
//    loopv(nodes)
//    {
//		node *e = nodes[i];
//        if(!e) continue;
//        float dist = e->o.dist(player->o);
//        if(dist < closedist)
//        {
//            closest = i;
//            closedist = dist;
//        }
//    }
//    if(closest >= 0) entadd(closest);*/
//}
//
//#define nodefocusv(i,f, v) { int n = curworld->nfocus = int(i); if (n >= 0) { node *e = v[n]; f; } }
//
//#define addimplicitnode(f) { if(curworld->nodeselect.empty() && curworld->nodehover>=0) { curworld->nodeseladd(enthover); /*undonext = (enthover != oldhover);*/ f; curworld->nodeselect.drop(); } else f; }
//#define addgroupnodes(exp) { /*vector<node *> nodes = curworld->curscene->getnodes(); loopv(nodes) nodefocusv( i, if (exp) curworld->nodeseladd(i) , nodes);*/ }
//#define groupnodeloop(e, s) ;
//#define nodefocus (n, f) entfocusv(i, f, /*curworld->curscene->getnodes()*/)

////Icomands need fixed try adding to worldeditor
//ICOMMAND(enthavesel, "", (), /* addimplicitnode(curworld->nodeselect.length());*/); //addimplicit(intret(curworld->nodeselect.length())));
//ICOMMAND(entselect, "e", /*(uint *body), if (!nonodeedit()) addgroupnodes(e && curworld->nodeselect.find(n)<0 && executebool(body))*/);
//ICOMMAND(entloop, "e", /*(uint *body), if (!nonodeedit()) addimplicitnode(groupnodeloop(((void)e, execute(body))));*/);
//ICOMMAND(insel, "", (),/* entfocus(efocus, intret(pointinsel(sel, e.o)))*/);
//ICOMMAND(entget, "", (), /*entfocus(efocus, cubestr s; printent(e, s, sizeof(s)); result(s))*/);
//ICOMMAND(entindex, "", (), /*intret(curworld->nfocus)*/);
//COMMAND(entset, "siiiii");
//COMMAND(nearestent, "");


//void enttype(char *type, int *numargs)
//{
// //   if(*numargs >= 1)
// //   {
// //       int typeidx = findtype(type);
// //       if(typeidx != ET_EMPTY) groupedit(e.type = typeidx);
// //   }
// //   else entfocus(efocus,
// //   {
// //       result(entities::entname(e.type));
//	//});
//}
//
//void entattr(int *attr, int *val, int *numargs)
//{
//    /*if(*numargs >= 2)
//    {
//        if(*attr >= 0 && *attr <= 4)
//            groupedit(
//                switch(*attr)
//                {
//                    case 0: e.attr1 = *val; break;
//                    case 1: e.attr2 = *val; break;
//                    case 2: e.attr3 = *val; break;
//                    case 3: e.attr4 = *val; break;
//                    case 4: e.attr5 = *val; break;
//                }
//            );
//    }
//    else entfocus(efocus,
//    {
//        switch(*attr)
//        {
//            case 0: intret(e.attr1); break;
//            case 1: intret(e.attr2); break;
//            case 2: intret(e.attr3); break;
//            case 3: intret(e.attr4); break;
//            case 4: intret(e.attr5); break;
//        }
//    });*/
//}
//
//COMMAND(enttype, "sN");
//COMMAND(entattr, "iiN");
//
//int findentity(int type, int index, int attr1, int attr2)
//{
//    //const vector<extentity *> &ents = entities::getents();
//    //if(index > ents.length()) index = ents.length();
//    //else for(int i = index; i<ents.length(); i++)
//    //{
//    //    extentity &e = *ents[i];
//    //    if(e.type==type && (attr1<0 || e.attr1==attr1) && (attr2<0 || e.attr2==attr2))
//    //        return i;
//    //}
//    //loopj(index)
//    //{
//    //    extentity &e = *ents[j];
//    //    if(e.type==type && (attr1<0 || e.attr1==attr1) && (attr2<0 || e.attr2==attr2))
//    //        return j;
//    //}
//    //return -1;
//}
//
//int spawncycle = -1;
//
//void findplayerspawn(dynent *d, int forceent, int tag) // place at random spawn
//{
//   /* int pick = forceent;
//    if(pick<0)
//    {
//        int r = rnd(10)+1;
//        pick = spawncycle;
//        loopi(r)
//        {
//            pick = findentity(ET_PLAYERSTART, pick+1, -1, tag);
//            if(pick < 0) break;
//        }
//        if(pick < 0 && tag)
//        {
//            pick = spawncycle;
//            loopi(r)
//            {
//                pick = findentity(ET_PLAYERSTART, pick+1, -1, 0);
//                if(pick < 0) break;
//            }
//        }
//        if(pick >= 0) spawncycle = pick;
//    }
//    if(pick>=0)
//    {
//        const vector<extentity *> &ents = entities::getents();
//        d->pitch = 0;
//        d->roll = 0;
//        for(int attempt = pick;;)
//        {
//            d->o = ents[attempt]->o;
//            d->yaw = ents[attempt]->attr1;
//            if(entinmap(d, true)) break;
//            attempt = findentity(ET_PLAYERSTART, attempt+1, -1, tag);
//            if(attempt<0 || attempt==pick)
//            {
//                d->o = ents[pick]->o;
//                d->yaw = ents[pick]->attr1;
//                entinmap(d);
//                break;
//            }
//        }
//    }
//    else
//    {
//        d->o.x = d->o.y = d->o.z = 0.5f*worldsize;
//        d->o.z += 1;
//        entinmap(d);
//    }
//    if(d == player) ovr::reset();*/
//}
//

//
//
////void resetmap()
////{
////    clearoverrides();
////    clearmapsounds();
////    resetblendmap();
////    clearlights();
////    clearpvs();
////    clearslots();
////    clearparticles();
////    clearstains();
////    clearsleep();
////    cancelsel();
////    pruneundos();
////    clearmapcrc();
////    entities::clearents();
////	curworld->clearworld();
////	PHYSrebuildLevel();
////    outsideents.setsize(0);
////    spotlights = 0;
////    volumetriclights = 0;
////}
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

//#define getnodesvec vector<node *> &nodes = curscene->getnodes();
//
//	world::world(scene *s) : curscene(s), nodehover(-1), nodeorient(-1), nfocus(-1), oldhover(-1){}
//
//	inline bool world::nonodeedit() { if (!editmode) { conoutf(CON_ERROR, "operation only allowed in edit mode"); return true; } return !nodeediting; }
//
//	bool world::haveselnode(){ return nodeselect.length() > 0; }
//	void world::nodeselcancel(){ nodeselect.shrink(0); }
//	void world::nodeseladd(int id){ nodeselect.add(id); }//undonext = true; } add undo later
//	vec world::getselpos()
//	{
//		getnodesvec
//		if (nodeselect.length() && nodes.inrange(nodeselect[0])) return nodes[nodeselect[0]]->o;
//		if (nodes.inrange(nodehover)) return nodes[nodehover]->o;
//		return vec(sel.o);
//	}

	//void world::rendernodeselection(const vec &o, const vec &ray, bool moving)
	//{
	//	if (nonodeedit())return;// || (entgroup.empty() && enthover < 0)) return;
	//	vec eo, es;
	//	getnodesvec
	//		gle::colorub(0, 30, 0);
	//	gle::defvertex();
	//	gle::begin(GL_LINES, nodes.length() * 24);
	//	loopv(nodes){
	//		node *e = nodes[i];
	//		vec o(e->o);
	//		o.z += e->radius.z+2.5; // add it for particles and for bounds :)
	//		regular_particle_splash(PART_EDIT, 2, 40, o, 0x3232FF, 0.32f*100.f / 100.0f);
	//		particle_textcopy(o, e->name.c_str(), PART_TEXT, 1, 0x6496FF, 2.0f);
	//		o.z -= e->radius.z+2.5;
	//		renderentbox(o.sub(e->radius), vec(e->radius).mul(2));
	//	}
	//	xtraverts += gle::end();
	//	if (nodeselect.empty() && nodehover < 0) return;
	//	if (nodeselect.length())
	//	{
	//		gle::colorub(0, 50, 0);
	//		gle::defvertex();
	//		gle::begin(GL_LINES, nodeselect.length() * 24);
	//		loopv(nodeselect){
	//			if (!nodes.inrange(nodeselect[i]))continue; // if the node is outside the range of nodes ignore it
	//			node *e = nodes[nodeselect[i]];
	//			vec o(e->o);
	//			//o.z += e->radius.z;
	//			renderentbox(o.sub(e->radius), vec(e->radius).mul(2));
	//		}
	//		xtraverts += gle::end();
	//	}
	//	if (nodehover >= 0 && nodes.inrange(nodehover) && nodes[nodehover])
	//	{
	//		node *n = nodes[nodehover];
	//		gle::colorub(0, 40, 0);
	//		 // also ensures enthover is back in focus
	//		eo = vec(n->o); es = (n->radius*2);
	//		eo -= es; es.mul(2);
	//		boxs3D(eo, es, 1);
	//		if (nodemoving && entmovingshadow == 1)
	//		{
	//			int plane = dimension(nodeorient); //do not render if can not move in that direction
	//			vec a, b;
	//			gle::colorub(40, 20, 20);
	//			(a = eo).x = eo.x - fmod(eo.x, worldsize); (b = es).x = a.x + worldsize; if (plane != 0){ boxs3D(a, b, 1); } //on x plane
	//			gle::colorub(20, 40, 20);
	//			(a = eo).y = eo.y - fmod(eo.y, worldsize); (b = es).y = a.x + worldsize; if (plane != 1){ boxs3D(a, b, 1); } //on y plane
	//			gle::colorub(20, 20, 40);
	//			(a = eo).z = eo.z - fmod(eo.z, worldsize); (b = es).z = a.x + worldsize; if (plane != 2){ boxs3D(a, b, 1); } //on z plane
	//		}
	//		
	//		gle::colorub(200, 0, 0);
	//		boxs(nodeorient, eo, es);
	//	}

	//	/*if (showentradius)
	//	{
	//	glDepthFunc(GL_GREATER);
	//	gle::colorf(0.25f, 0.25f, 0.25f);
	//	loopv(entgroup) entfocus(entgroup[i], renderentradius(e, false));
	//	if (enthover >= 0) entfocus(enthover, renderentradius(e, false));
	//	glDepthFunc(GL_LESS);
	//	loopv(entgroup) entfocus(entgroup[i], renderentradius(e, true));
	//	if (enthover >= 0) entfocus(enthover, renderentradius(e, true));
	//	}
	//	*/
	//	gle::disable();
	//}
	//void world::rendernodes(){ curscene->rendernodes(); }
	//void world::serializedworld()
	//{
	//	if (asScript->serializer) asScript->resetserializer();
	//	else asScript->setupserializer(new CSerializer()); 
	//	//loopk(scenes)
	//	//scene * cursn = sense[k];
	//	curscene->store();
	//	
	//}
	//void world::updateworld()
	//{
	//	if (!touched) { serializedworld(); curscene->doawake(); touched = true;  return; }
	//	
	//	curscene->updatenodes();
	//}
	//void world::addnodetoscene(node *g) { curscene->addobject(g); if (!paused) conoutf("there are %d objects in the world",curscene->nodes.length()); }
	//uint world::getnumnodes(){ /*uint num; loopv(scenes) num += scenes[i]->getnumnodes(); return num + worldnodes.length();*/ return uint(curscene->getnodes().length()); }
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

	/*bool world::pointinsel(const selinfo &sel, const vec &o)
	{
		return(o.x <= sel.o.x + sel.s.x*sel.grid && o.x >= sel.o.x && o.y <= sel.o.y + sel.s.y*sel.grid && o.y >= sel.o.y && o.z <= sel.o.z + sel.s.z*sel.grid && o.z >= sel.o.z);
	}*/

	//bool world::editmoveplane(const vec &o, const vec &ray, int d, float off, vec &handle, vec &dest, bool first)
	//{
	//	plane pl(d, off);
	//	float dist = 0.0f;
	//	//if (!pl.rayintersect(camera1->o, ray, dist))
	//		//return false;

	//	dest = vec(ray).mul(dist).add(camera1->o);
	//	if (first) handle = vec(dest).sub(o);
	//	dest.sub(handle);
	//	return true;
	//}
	/*float world::getnearestent(const vec &o, const vec &ray, float radius, int mode, int &hitnode)
	{
		vec eo, es;
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
		}
		return dist;
	}*/
	//void world::clearworld(bool force){ if(touched || force) curscene->resetworld(); touched = false; }


	//void world::nodedrag(const vec &ray)
	//{
	//	if (nonodeedit() || !haveselnode()) return;
	//	getnodesvec
	//	float r = 0, c = 0;
	//	static vec dest, handle;
	//	vec eo, es;
	//	int d = dimension(nodeorient),
	//		dc = dimcoord(nodeorient);

	//	int n = curworld->nfocus = nodeselect.last();
	//	if (n >= 0 && nodes.inrange(n) && nodes[n]) {

	//		node &e = *nodes[n];
	//		eo = vec(e.o); es = vec(e.radius);
	//		es.mul(2);
	//		if (!editmoveplane(e.o, ray, d, eo[d] + (dc ? es[d] : 0), handle, dest, entmoving == 1))
	//			return;
	//		ivec g = dest;
	//		int z = g[d] & (~(sel.grid - 1));
	//		g.add(sel.grid / 2).mask(~(sel.grid - 1));
	//		g[d] = z;

	//		r = (entselsnap ? g[R[d]] : dest[R[d]]) - e.o[R[d]];
	//		c = (entselsnap ? g[C[d]] : dest[C[d]]) - e.o[C[d]];
	//		loopv(nodeselect){ nodes[nodeselect[i]]->o[R[d]] += r; nodes[nodeselect[i]]->o[C[d]] += c; nodes[nodeselect[i]]->moveto(nodes[nodeselect[i]]->o); }
	//	}
	//	entmoving = 2;
	//	//if (entmoving == 1) makeundoent();
	//	
	//}
	//bool world::hoveringonnode(int node, int orient)
	//{
	//	if (nonodeedit()) return false;
	//	//nodeorient = orient;
	//	if ((nodehover) >= 0)
	//		return true;
	//	nfocus = nodeselect.empty() ? -1 : nodeselect.last();
	//	nodehover = -1;
	//	return false;
	//}
	//void world::resetmap()
	//{
	//	clearoverrides();
	//	clearmapsounds();
	//	resetblendmap();
	//	clearlights();
	//	clearpvs();
	//	clearslots();
	//	clearparticles();
	//	clearstains();
	//	clearsleep();	
	//	cancelsel();
	//	pruneundos();
	//	clearmapcrc();
	//	curscene->clearworld();
	//	PHYSrebuildLevel();
	//	//outsideents.setsize(0);
	//	spotlights = 0;
	//	volumetriclights = 0;
	//}

	/*vector<node *> world::getselnodes()
	{
		vector<node *> n;
		vector<node *> an;
		loopv(nodeselect)
		{
			n.add(curscene->nodes[nodeselect[i]]);
		}
		return n;
	}*/
	//void world::saveworld(stream *f)
	//{
	//	//loopv(scences)
	//	{
	//		//curscence = scences[i];
	//		vector<node *> ns = curscene->getnodes();
	//		//f->putstring(curscene->name);
	//		//f->putlil<uint>(ns.length());
	//		curscene->savescene(f);
	//	}
	//}
	/*vector<light *> world::getlights()
	{
		vector<light *> li;
		loopv(lights)
			li.add(lights.pop());
		return li;
	}*/
	//void world::addlight(vec o, vec c, int r, char ty)
	//{
	//	light *l = new light();
	//	l->color = c;
	//	l->type = ty;
	//	l->o = o;
	//	l->radius = r;
	//	lights.add(l);
	//}
	world *curworld = new world(new scene());
	
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
//void world::delnode()
//{
//	
//	//loopv(nodeselect)
//
//	//{
//	//	curscene->nodes.remove(nodeselect[i]);
//	//}
//}
//void delnode()
//{
//	/*curworld->delnode();*/
//}
//COMMAND(delnode, "");
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
			curscene->store();
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
				nodepool.pad(amt);
				loopi(amt) nodepool[amt + len] = new node();
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
			if (deselect && nodecanedit & nodeediting) nodeselectcancel(); //remove entediting later
			return nodecanedit;
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
			return nodeselect.length() > 0;
		}
		void worldeditor::nodeselectcancel()
		{
			nodeselect.shrink(0);
		}
		void worldeditor::nodeselectadd(int id)
		{
			undonext = true;
			nodeselect.add(id);
		}
		vec worldeditor::getselpos()
		{
			vector<node *> nodes = curworld->getnodefromid(nodeselect);
			if (nodes.length() > 0 && nodes[0]) return nodes[0]->o;
			if (node *n = curworld->getnodefromid(nodehover)) return n->o;
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
		inline vector<node*> &worldeditor::getselectnodes() { return curworld->getnodefromid(nodeselect); }
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
			undonext = true;
			nodehover = -1; oldhover = -1; nodeorient = 0;
			nfocus = -1; nodemoving = 0;
			nodelooplevel = 0;
			nodeselect.shrink(0); /*selinfos.shrink(0); undoblocks.shrink(0);*/
			nodeselsnap = false; nodeediting = true;
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
			if (nodeselect.empty() && nodehover < 0) return;
			vec no, ns;
			if (nodeselect.length())
			{
				vector<node *> nodes = curworld->getnodefromid(nodeselect);
				gle::colorub(0, 50, 0);
				gle::defvertex();
				gle::begin(GL_LINES, nodes.length() * 24);
				loopv(nodes){
					node *n = nodes[i];
					vec o(n->o);
					renderentbox(o.sub(n->radius), vec(n->radius).mul(2));
				}
				xtraverts += gle::end();
			}
			if (nodehover)
			{
				node *n = curworld->getnodefromid(nodehover);
				if (!n)return;
				gle::colorub(0, 40, 0);
					// also ensurns enthover is back in focus
				no = vec(n->o); ns = (n->radius*2);
				no -= ns; ns.mul(2);
				boxs3D(no, ns, 1);
				if (nodemoving && nodemovingshadow == 1)
				{
					int plane = dimension(nodeorient); //do not render if can not move in that direction
					vec a, b;
					gle::colorub(40, 20, 20);
					(a = no).x = no.x - fmod(no.x, worldsize); (b = ns).x = a.x + worldsize; if (plane != 0){ boxs3D(a, b, 1); } //on x plane
					gle::colorub(20, 40, 20);
					(a = no).y = no.y - fmod(no.y, worldsize); (b = ns).y = a.x + worldsize; if (plane != 1){ boxs3D(a, b, 1); } //on y plane
					gle::colorub(20, 20, 40);
					(a = no).z = no.z - fmod(no.z, worldsize); (b = ns).z = a.x + worldsize; if (plane != 2){ boxs3D(a, b, 1); } //on z plane
				}
					
				gle::colorub(200, 0, 0);
				::boxs(nodeorient, no, ns);
			}
			gle::disable();
		}
		bool worldeditor::hoveringonnode(int node, int orient)
		{
			if(nodenoedit()) return false;
			nodeorient = orient;
			if((nfocus = nodehover = node) >= 0)
			return true;
			nfocus   = nodeselect.empty() ? -1 : nodeselect.last();
			nodehover = -1;
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
			int d = dimension(nodeorient);
			int dc = dimcoord(nodeorient);
			vector<node *> nodes = curworld->getnodefromid(nodeselect);
			loopv(nodes) {
				if (!editmoveplane(nodes[i]->o, ray, d, nodes[i]->o[d] + (dc ? nodes[i]->radius[d] : 0), handle, dest, nodemoving == 1))
					return;

				ivec g = dest;
				int z = g[d] & (~(sel.grid - 1));
				g.add(sel.grid / 2).mask(~(sel.grid - 1));
				g[d] = z;

				r = (entselsnap ? g[R[d]] : dest[R[d]]) - nodes[i]->o[R[d]];
				c = (entselsnap ? g[C[d]] : dest[C[d]]) - nodes[i]->o[C[d]];
			}

			if(nodemoving==1) makeundonode();
			if (nodes.length() < 1)nodes.add(curworld->getnodefromid(nfocus));
			makeundonode();
			loopv(nodes)
			{ 
			nodes[i]->o[R[d]] += r; 
			nodes[i]->o[C[d]] += c; 
			}
			nodemoving = 2;
		}
		void worldeditor::delnode(){ if(!curworld->removenode(nodeselect))  curworld->removenode(nfocus); nodeselectcancel(); }//this removes all the nodeselects if nodeselect.length() > 0; if not it returns false;
		void worldeditor::nodeflip()
		{
			if(nodenoedit()) return;
			int d = dimension(sel.orient);
			float mid = sel.s[d]*sel.grid/2+sel.o[d];
			//groupeditundo(e.o[d] -= (e.o[d]-mid)*2);
			vector<node *> nodes = curworld->getnodefromid(nodeselect);
			if (nodes.length() < 1)nodes.add(curworld->getnodefromid(nfocus));
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
			vector<node *> nodes = curworld->getnodefromid(nodeselect);
			if (nodes.length() < 1)nodes.add(curworld->getnodefromid(nfocus));
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
			int numents = nodeselect.length();
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
			if(!undonext) return;
			undonext = false;
			oldhover = nodehover;
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
#pragma endregion (calls from worldeditor struct)



