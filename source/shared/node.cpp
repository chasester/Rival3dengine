#include "cube.h"
#include "engine.h" //need the model loading function calls

////////ridgid body///////////
ridgidbody::ridgidbody(): radius(5.f), center(5.f), weight(10), afactor(0.8f), lfactor(0.9f), friction(0.5), ldamp(0.1), adamp(0.4), gravity(0.f, 0.f, 0.f), restitution(0.01) \
{}

void ridgidbody::aabbfrommodel(int index) //moved because model has not been defined in ents.h
{
	const char *mdlname = mapmodelname(index);
	if (!mdlname) return;
	model *m = loadmodel(mdlname);
	if (!m) return;
	m->collisionbox(center, radius);
}

///////bulletobj///////////
void bulletobj::move(vec im){}
void bulletobj::moveto(vec _o)
{
	if (!btridgidbody)return;
	btTransform temp = btridgidbody->getWorldTransform();
	temp.setOrigin(FROM_SAUER_VEC(_o));
	btridgidbody->setWorldTransform(temp);
	btridgidbody->forceActivationState(ACTIVE_TAG);
	btridgidbody->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
	btridgidbody->setAngularVelocity(btVector3(0.f, 0.f, 0.f));
}
void bulletobj::rotate(vec im){}
//////scriptinterface//////

scriptinterface::scriptinterface(asIScriptObject *obj) : object(obj)
{
	if (!obj)return;
	typeID = obj->GetTypeId();
	//saveData = asScript->serializer->Store(obj);
	//if (!saveData)conoutf("Failed");
}

void scriptinterface::adddata(CSerializedValue *val)
{saveData = val;}

void scriptinterface::save()
{
	if (saveData)
	{
		saveData->ClearChildren();
		saveData->Store(object, typeID);
	}else saveData = asScript->serializer->Store(object);
}

bool scriptinterface::restore()
{
	if (!saveData) return false;
	saveData->Restore(object, typeID);
	return true;
}

void scriptinterface::setData(str name, void *val, size_t size)
{
	asIScriptObject *prop;
	loopi(object->GetPropertyCount())
	{
		if (object->GetPropertyName(i) == name)
		{
			int typeID = object->GetPropertyTypeId(i);
			memcpy(object->GetAddressOfProperty(i), val, size);
		}
	}
}

////////node///////////
node::node(vec pos, str name, str tags) : name(name), refcount(1),  o(pos), b(NULL),  c(NULL), radius(2.5f), tags(tags) {}
node::node(vec pos, vec rotation) : name(""), refcount(1),  o(pos), b(NULL), c(NULL), radius(2.5f), tags("") {}

int node::addref(){ return ++refcount; }
int node::release(){ if (--refcount == 0){ delete this; return 0; } return refcount; }
//
asILockableSharedBool *node::getweakflagref()
{
	if (!weakRefFlag)
		weakRefFlag = asCreateLockableSharedBool();
	return weakRefFlag;
}
void node::store()
{
	bool force = false;
	if (!c) { c = new CSerializedValue(); force = true; }
	asScript->serializer->storenode(this, c, force);
}
//void node::seteditbox(vec &rad)
//{
//	radius = vec(rad.y, rad.x, rad.z);
//}
void node::seteditbox(int i)
{
	const char *mdlname = mapmodelname(i);
	if (!mdlname) return;
	model *m = loadmodel(mdlname);
	if (!m) return;
	vec c, r;
	m->collisionbox(c, r);
	radius = vec(r.y, r.x, r.z);
}
void node::updatefrombullet()
{
	if (!b || !b->btridgidbody || !b->btridgidbody->getMotionState()) return;
	btRigidBody &r = *(b->btridgidbody);
	btTransform trans;
	r.getMotionState()->getWorldTransform(trans);
	o = vec(trans.getOrigin().getX(), trans.getOrigin().getZ(), trans.getOrigin().getY()).mul(SAUER_FACTOR);
	vec temprot;
	quat a(r.getOrientation());
	temprot = a.quat2euler();
	rot.x = temprot.z; //yaw
	rot.y = temprot.y; //pitch
	rot.z = temprot.x; //roll
}
bool node::restore()
{
	if (!c) {
		conoutf("No C not restored");
		return false;
	}
	c->Restore(this, CSerialnode::getID());
	if (!this) {conoutf("no THis"); return false;}
	return true;
}
void node::restart()
{
	if (b)	b = NULL;
	if (c) 	c = NULL;
}
void node::doawake()
{
	//if(c)this->store();
	loopv(ctrl) if (ctrl[i])asScript->doAwake(ctrl[i]); else ctrl.remove(i);
}

void node::doupdate(){ loopv(ctrl) if (ctrl[i])asScript->doUpdate(ctrl[i]); else ctrl.remove(i);}
void node::dorender(){ loopv(ctrl) if (ctrl[i])asScript->doRender(ctrl[i]); else ctrl.remove(i);}
void node::moveto(vec _o)
{
	if (b)
		b->moveto(_o);
	o = _o;
}
node::~node()
{
	refcount = 0;
  //if(b)delete b;
}
//void move(vec impulse)
//{
//	if (!b) o.add(impulse);
//	b->move(impulse);
//}

//void rotate(vec impulse)
//{
//	if (!b) return;
//	b->rotate(impulse);
//}

//void rotate(float yaw, float pitch, float roll, bool deg = true)
//{
//	if (!b)return;
//	vec i(yaw, pitch, roll);
//	b->rotate(deg ? i.mul(RAD) : i);
//}


//void setfriction(float f){}
//void setelasticity(float e){}
//void setgravity(float g){}
//void setupbulletfrommodel(int indx){}
//void setupbulletfrommodel(str name){}
//void setupbulletfromaabb(vec offset, vec size, float friction = 0.3f, float elasticity = 0.8f){} // add more properties later
//void setupbulletfromcylender(vec offset, float radius, float height, float friction = 0.3f, float elasticity = 0.8f){}

