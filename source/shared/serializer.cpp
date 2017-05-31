//
// CSerializer
//
// This code was based on the CScriptReloader written by FDsagizi
// http://www.gamedev.net/topic/604890-dynamic-reloading-script/
//

//#include <assert.h>
//#include <string.h> // strstr
//#include <stdio.h>  // sprintf
#include "cube.h"

SaveType::serialType::serialType(str name, str typeName) : name(name), typeName(typeName){}

void SaveType::toPrint()
{
	str outdata;
	loopv(members)
	{
		outdata += " " + members[i]->name + ":" + members[i]->typeName;
	}
	conoutf("%s:\n %d members:%d bytes\n %s", name.c_str(), numprops, size, outdata.c_str());
}
void SaveType::addProperty(str name, str typeName)
{
	if (name == "" || typeName == "") return;
	members.add(new serialType(name, typeName));
	numprops++;
}

///////////////////////////////////////////////////////////////////////////////////

CSerializer::CSerializer()
{
	m_engine = 0;
}

CSerializer::~CSerializer()
{
	// Clean the serialized values before we remove the user types
	m_root.Uninit();

	// Delete the user types
	std::map<std::string, CUserType*>::iterator it;
	for( it = m_userTypes.begin(); it != m_userTypes.end(); it++  )
		delete it->second;

	if( m_engine )
		m_engine->Release();
}

void CSerializer::print(){m_root.print(0);}

const char *typeidtoname(uint typeId)
{
	const char *typeName;
	if (typeId < asTYPEID_DOUBLE+1) typeName = asScript->getprimitivename(typeId).c_str();
	else if (typeId == asTYPEID_DOUBLE + 1) typeName = "string";
	else
	{
		asITypeInfo *temp = asScript->asEngine->GetTypeInfoById(typeId);
		if (temp)typeName = temp->GetName();
		else return "";
	}
	return typeName;
}

struct DataHold;

enum { asTYPEID_STRING = asTYPEID_DOUBLE + 1 };

vector<str > types;
vector<str > names;
vector<str > namespaces;
vector<DataHold *> saveddata;

#define VECTOROVERLOADCHECK(v,n) n < v.length() ? n : 0
struct DataHold
{
	std::vector<char> data;
	uint children, namespaceID, nameID, typeID;
	vector<DataHold *> childs = vector<DataHold *>();
	DataHold(uint nsID, uint tID, uint nID) : namespaceID(nsID), typeID(tID), nameID(nID){}
	
	void print()
	{
		conoutf("%s%s %s %d",\
			namespaces[VECTOROVERLOADCHECK(namespaces, namespaceID)] == "" ? "" : (namespaces[VECTOROVERLOADCHECK(namespaces, namespaceID)] + "::").c_str(),\
			typeID > asTYPEID_STRING ? types[VECTOROVERLOADCHECK(types, typeID - (asTYPEID_STRING+1))].c_str() : typeID == asTYPEID_STRING ? "string" : asScript->getprimitivename(typeID).c_str(),\
			names[VECTOROVERLOADCHECK(names, nameID)].c_str(), children);
	}
	void write(stream *f)
	{
		f->putlil<uint>(namespaceID);
		f->putlil<uint>(typeID);
		f->putlil<uint>(nameID);
		f->putlil<uint>(children);

		if (typeID == asTYPEID_STRING && data.size() == children) f->write(&data[0], data.size()); //will return below;

		uint mlen = asScript->asEngine->GetSizeOfPrimitiveType(typeID);
		if (!mlen) return;
		else if (data.size() == mlen)
			f->write(&data[0], data.size()); //if the length of the data doesnt match the size it should
		else conoutf("SUPER ERROR");// f->write(new std::vector<char>(mlen), mlen); //then just add blank data to fill in (latter change to add the real data first then add blank data after.
 	}
	static DataHold *read(stream *f)
	{
		uint nsp = f->getlil<uint>();
		uint tp = f->getlil<uint>();
		uint nm = f->getlil<uint>();
		DataHold *temp = new DataHold(nsp, tp , nm);
		temp->children = f->getlil<uint>();
		str out = "";
		if (temp->typeID < asTYPEID_STRING && temp->typeID != asTYPEID_VOID)
		{
			
			int size = asScript->asEngine->GetSizeOfPrimitiveType(temp->typeID);
			if (size)
			{
				temp->data.resize(size);
				f->read(&temp->data[0], size);
			}
			
		}
		else if (temp->typeID == asTYPEID_STRING) { 
			temp->data.resize(temp->children); f->read(&temp->data[0], temp->children); temp->children = 0;
			str unpack = str();
			unpack.resize(temp->data.size());
			memcpy(&unpack[0], &temp->data[0], temp->data.size());
			conoutf(unpack.c_str());	
		}
		return temp;
	}
	int count(uint start)
	{
		start++;
		loopi(children)
		{
			if (!VECTOROVERLOADCHECK(saveddata, start)) return start;
			start = saveddata[start]->count(start);
		}
		return start;
	} 
};


#define GETID(v,a)\
{\
	loopv(v)\
if (name == v[i])\
	return i + a; \
	v.put(name); \
	return v.length() - 1 + a; \
};

#define PUTLILVECTOR(v)\
{\
	f->putlil<uint>(v.length()); \
	loopv(v)\
	PUTLILSTRING(v[i]); \
}

#define PUTLILSTRING(s)\
{\
	const char *sc = s.c_str(); \
	f->putlil<ushort>(strlen(sc)); \
	f->write(sc, strlen(sc)); \
}

#define PRINT_VECTOR(v)\
{\
	str strv; \
	loopv(v) strv += v[i]; \
	conoutf(strv.c_str());\
}

int gettypeid(str name) { GETID(types, asTYPEID_STRING+1); }

int getnameid(str name){ GETID(names, 0); }

int getnamespaceid(str name){ GETID(namespaces, 0); }

void CSerializer::save(stream *f)
{
	//conoutf("%d", m_root.m_children.size());
	if (!m_engine) SetEngine(asScript->asEngine);
	names.shrink(0);
	types.shrink(0);
	namespaces.shrink(0);
	saveddata.shrink(0);
	names.put("__ERROR__");
	types.put("string");
	types.put("VOID");
	m_root.save();
	//save namespaces
	PUTLILVECTOR(namespaces);
	//save types
	PUTLILVECTOR(types);
	//save names
	PUTLILVECTOR(names);
	//save saveddata
	f->putlil<uint>(saveddata.length());
	//conoutf("%d", saveddata.length());
	loopv(saveddata) {
		saveddata[i]->write(f);
		//saveddata[i]->print();
	}
}

#define PULLLILSTRING(st)\
{\
	const ushort slen = f->getlil<ushort>();\
	char s[512];\
	f->read(&s, slen > 512 ? 512 : slen); \
	s[slen>512?512:slen] = '\0';\
	if(slen > 512) f->seek(slen - (512), SEEK_CUR);\
	st = s;\
}\

#define PULLLILVECTOR(v)\
{\
	uint len = f->getlil<uint>();\
	loopi(len){\
		str string;\
		PULLLILSTRING(string); \
		v.put(string); \
	}\
}

static int index = -1; //define a local static because passing a index via the peramiters creates an unknow error;
void CSerializer::load(stream *f)
{
	if (!m_engine) SetEngine(asScript->asEngine);
	m_root.ClearChildren();
	names.shrink(0);
	types.shrink(0);
	namespaces.shrink(0);
	saveddata.shrink(0);
	curworld->clearworld();
	curworld->curscene->nodes.shrink(0);
	PULLLILVECTOR(namespaces);
	PULLLILVECTOR(types);
	PULLLILVECTOR(names);
	PRINT_VECTOR(names);
	uint size = f->getlil<uint>();
	loopi(size)saveddata.put(DataHold::read(f));
	loopv(saveddata)
	{
		DataHold &sd = *saveddata[i];
		sd.print();
	}
	m_root.Uninit();
	m_root.Init();
	m_root.m_serializer = asScript->serializer;
	if (!m_root.m_serializer) { conoutf("failed to init serializer, map will not load nodes."); return; }
	index = -1; //this is to avoid a compile time error that causes the function to add extra number to index
	m_root.load();
	curworld->serializedworld();
	return;
}
#define CALLCONSTRUCTOR(n) new n();

void CSerializedValue::load()
{
	index++;
	//index is always the element in the array where the object we are calling from is at.
	if (index >= saveddata.length()) { conoutf("TOO MANY OBJECTS IN TREE %d", index);  return ; }
	DataHold &sd = *saveddata[index];
	if (sd.children && (sd.typeID > asTYPEID_STRING || sd.typeID == asTYPEID_VOID))
		loopi(sd.children)
	{
		CSerializedValue *child = new CSerializedValue();
		child->m_serializer = m_serializer;
		child->load();
		if (child->m_isInit) m_children.push_back(child);
	}
	//now we have our tree skeloton so now we take the data from savedata and add it to the objects starting with the childen and working up the tree
	m_isInit = true;
	m_name = names[VECTOROVERLOADCHECK(names, sd.nameID)];
	m_typeName = types[VECTOROVERLOADCHECK(types, sd.typeID - (asTYPEID_STRING + 1))];
	m_nameSpace = namespaces[VECTOROVERLOADCHECK(namespaces, sd.namespaceID)];

	if (sd.typeID == asTYPEID_VOID || sd.typeID == asTYPEID_STRING + 1)  return;
	
	if (sd.typeID == asTYPEID_STRING) 
	{
		m_mem.resize(sd.data.size()); m_mem = sd.data;
		m_typeId = CSerialstring::getID();
		m_serializer->m_userTypes["string"]->Create(this);
		return;
	}

	
	asITypeInfo *ot;
	if (sd.typeID > asTYPEID_STRING) // is object a non perimitive
	{
		if (m_typeName == str("__ERROR__")) return;
		ot = asScript->asEngine->GetTypeInfoByName(m_typeName.c_str());
		if (!ot)
		{
			asScript->recompilemodule(m_typeName.c_str());
			asIScriptModule *mod = asScript->asEngine->GetModule(m_typeName.c_str());
			if (!mod) { conoutf("ERROR!!! no mod"); return ; }
			ot = mod->GetTypeInfoByName(m_typeName.c_str());
		}
		
		if (ot)
		{
			m_typeId = ot->GetTypeId();
			if (m_typeId & asTYPEID_SCRIPTOBJECT )
			{
				m_restorePtr = asScript->asEngine->CreateUninitializedScriptObject(ot);
				asIScriptObject *a = static_cast<asIScriptObject *> (m_restorePtr);
				loopi(m_children.size() < a->GetPropertyCount() ? m_children.size() : a->GetPropertyCount())
				{
					try
					{
						//if (!m_children[i]) { conoutf("noooo"); continue; }
						if (m_children[i]->GetTypeId() <= asTYPEID_DOUBLE && m_children[i]->GetTypeId() != asTYPEID_VOID)
						{
							if ( m_children[i]->m_name == str(a->GetPropertyName(i)) && m_children[i]->GetTypeId() == a->GetPropertyTypeId(i))
							asScript->assigntoptrfromtypeid(a->GetAddressOfProperty(i), m_children[i]->m_restorePtr, m_children[i]->m_typeId);
							//*reinterpret_cast<int*>(a->GetAddressOfProperty(i)) = *(int *) m_children[i]->m_restorePtr; //change this so that it cast based on the primitive type
							//conoutf("the number is: %d %d", 0, *(int *) a->GetAddressOfProperty(i));
						}
					}
					catch (...) {
						conoutf("name here %s %s", m_children[i]->m_name.c_str(), a->GetPropertyName(i));
					}
				}

			}
			else
			{
				if (!m_serializer) { m_serializer = asScript->serializer; if (!m_serializer){ conoutf("error assigning serialize, Please debug"); return; }}
					CUserType *type = m_serializer->m_userTypes[m_typeName];
					if (type)
						type->Create(this);
			}
			
		}
	}
	else //ok it must be a preimitive
	{
		m_typeId = sd.typeID;
		m_restorePtr = asScript->createprivitiveptr(m_typeId);
		memcpy(m_restorePtr, &sd.data[0], sd.data.size());
	}

	return ;
}

void CSerializedValue::save()
{
	if (m_typeId & asTYPEID_OBJHANDLE) {
		m_typeId = 1+asTYPEID_STRING; //set it void for right now
		DataHold *data = new DataHold(getnamespaceid(m_nameSpace), m_typeId, getnameid(m_name));
		data->children = 0;
		saveddata.put(data);
		return;
	}
	DataHold *data = new DataHold(getnamespaceid(m_nameSpace), m_typeId  > asTYPEID_DOUBLE ?  gettypeid(m_typeName) : m_typeId , getnameid(m_name));
	saveddata.put(data);
	if (data->typeID <= asTYPEID_DOUBLE && data->typeID != asTYPEID_VOID)
	{
		int size = m_serializer->m_engine->GetSizeOfPrimitiveType(data->typeID);
		//copy the memory
		if (size > 0)
		{
			data->data.resize(size); data->data = m_mem;
		}
	}
	else if (m_typeId == CSerialstring::serialID)
	{
		std::string *buffer = (std::string*)m_restorePtr;
		if (!buffer){buffer = new str(); }
		data->data.resize(buffer->size());
		memcpy(&data->data[0], buffer->c_str(), data->data.size());
		data->children = data->data.size();
		data->typeID = asTYPEID_STRING;
		return;
		//use the usertype to store for now or find a way to store as an array of data
	}
	//else
	data->children = m_children.size();

	loopi(m_children.size())
	{
		m_children[i]->save();
	}
	
}

void CSerializedValue::getDataTypes()
{
}

void CSerializer::AddUserType(CUserType *ref, const std::string &name)
{
	m_userTypes[name] = ref;
}

CSerializedValue * CSerializer::storenode(node * n)
{
	if (!m_engine)return NULL;
	CSerializedValue *c = new CSerializedValue(&m_root, "SERIAL_OBJECT" , "", n, CSerialnode::getID());
	m_root.m_children.push_back(c);
	return c;
}

void CSerializer::ClearRoot(){ m_root.ClearChildren(); }

int CSerializer::Store(asIScriptModule *mod)
{
	m_mod = mod;

	// The engine must not be destroyed before we're completed, so we'll hold on to a reference
	mod->GetEngine()->AddRef();
	if( m_engine ) m_engine->Release();
	m_engine = mod->GetEngine();

	m_root.m_serializer = this;

	// First store global variables
	asUINT i;
	for( i = 0; i < mod->GetGlobalVarCount(); i++ )
	{
		const char *name, *nameSpace;
		int typeId;
		mod->GetGlobalVar(i, &name, &nameSpace, &typeId);
		m_root.m_children.push_back(new CSerializedValue(&m_root, name, nameSpace, mod->GetAddressOfGlobalVar(i), typeId));
	}

	// Second store extra objects
	for( i = 0; i < m_extraObjects.size(); i++ )
		m_root.m_children.push_back(new CSerializedValue(&m_root, "", "", m_extraObjects[i].originalObject, m_extraObjects[i].originalTypeId));
	m_extraObjects.clear(); //delete the extra objects once we created them
	// For the handles that were stored, we need to substitute the stored pointer
	// that is still pointing to the original object to an internal reference so
	// it can be restored later on.
	m_root.ReplaceHandles();

	return 0;
}
CSerializedValue *CSerializer::Store(asIScriptObject *o)
{
	if (!m_engine)return NULL;
	CSerializedValue *c = new CSerializedValue(&m_root, "", "", o, o->GetTypeId());
	m_root.m_children.push_back(c);
	return c;
}

int CSerializer::Store(asIScriptEngine *e)
{
	e->AddRef();
	if (m_engine) m_engine->Release();
	m_engine = e;

	loopi(m_extraObjects.size())
	{
		m_root.m_children.push_back(new CSerializedValue(&m_root, "", "", m_extraObjects[i].originalObject, m_extraObjects[i].originalTypeId));
	}
	m_extraObjects.clear();
	return 0;
}
void CSerializer::SetEngine(asIScriptEngine *e)
{
	e->AddRef();
	if (m_engine) m_engine->Release();
	m_engine = e;
}
// Retrieve all global variables after reload script.
int CSerializer::Restore(asIScriptModule *mod)
{
	m_mod = mod;

	// The engine must not be destroyed before we're completed, so we'll hold on to a reference
	mod->GetEngine()->AddRef();
	if( m_engine ) m_engine->Release();
	m_engine = mod->GetEngine();

	// First restore extra objects, i.e. the ones that are not directly seen from the module's global variables
	asUINT i;
	for( i = 0; i < m_extraObjects.size(); i++ )
	{
		SExtraObject &o = m_extraObjects[i];
		asITypeInfo *type = m_mod->GetTypeInfoByName( o.originalClassName.c_str() );
		if( type )
		{
			for( size_t i2 = 0; i2 < m_root.m_children.size(); i2++ )
			{
				if( m_root.m_children[i2]->m_originalPtr == o.originalObject )
				{
					// Create a new script object, but don't call its constructor as we will initialize the members.
					// Calling the constructor may have unwanted side effects if for example the constructor changes
					// any outside entities, such as setting global variables to point to new objects, etc.
					void *newPtr = m_engine->CreateUninitializedScriptObject( type );
					m_root.m_children[i2]->Restore( newPtr, type->GetTypeId() ); 
				}
			}
		}
	}

	// Second restore the global variables
	asUINT varCount = mod->GetGlobalVarCount();
	for( i = 0; i < varCount; i++ )
	{
		const char *name, *nameSpace;
		int typeId;
		mod->GetGlobalVar(i, &name, &nameSpace, &typeId);

		CSerializedValue *v = m_root.FindByName(name, nameSpace);
		if( v )
			v->Restore(mod->GetAddressOfGlobalVar(i), typeId);
	}

	// The handles that were restored needs to be 
	// updated to point to their final objects.
	m_root.RestoreHandles();

	return 0;
}

void *CSerializer::GetPointerToRestoredObject(void *ptr)
{
	return m_root.GetPointerToRestoredObject( ptr );
}

void CSerializer::AddExtraObjectToStore( asIScriptObject *object )
{
	if( !object )
		return;

	// Check if the object hasn't been included already
	for( size_t i=0; i < m_extraObjects.size(); i++ )
		if( m_extraObjects[i].originalObject == object )
			return;

	SExtraObject o;
	o.originalObject    = object;
	o.originalClassName = object->GetObjectType()->GetName();
	o.originalTypeId    = object->GetTypeId();

	m_extraObjects.push_back( o );
}


///////////////////////////////////////////////////////////////////////////////////

CSerializedValue::CSerializedValue()
{
	Init();
}

CSerializedValue::CSerializedValue(CSerializedValue *parent, const std::string &name, const std::string &nameSpace, void *ref, int typeId) 
{
	Init();
	m_name       = name;
	m_nameSpace  = nameSpace;
	if (parent) m_serializer = parent->m_serializer;
	Store(ref, typeId);
}

void CSerializedValue::Init()
{
	m_handlePtr   = 0;
	m_restorePtr  = 0;
	m_typeId      = 0;
	m_isInit      = false;
	m_serializer  = 0;
	m_userData    = 0;
	m_originalPtr = 0;
}

void CSerializedValue::Uninit()
{
	m_isInit = false;

	ClearChildren();

	if( m_userData )
	{
		CUserType *type = m_serializer->m_userTypes[m_typeName];
		if( type )
			type->CleanupUserData(this);
		m_userData = 0;
	}
}

void CSerializedValue::ClearChildren()
{
	// If this value is for an object handle that created an object during the restore
	// then it is necessary to release the handle here, so we won't get a memory leak
	if( (m_typeId & asTYPEID_OBJHANDLE) && m_children.size() == 1 && m_children[0]->m_restorePtr )
	{
		m_serializer->m_engine->ReleaseScriptObject(m_children[0]->m_restorePtr, m_serializer->m_engine->GetTypeInfoById(m_children[0]->m_typeId));
	}

	for( size_t n = 0; n < m_children.size(); n++ )
		delete m_children[n];
	m_children.clear();
}

CSerializedValue::~CSerializedValue()
{
	Uninit();
}

CSerializedValue *CSerializedValue::FindByName(const std::string &name, const std::string &nameSpace)
{
	for( size_t i = 0; i < m_children.size(); i++ )
		if( m_children[i]->m_name      == name &&
			m_children[i]->m_nameSpace == nameSpace )
			return m_children[i];

	return 0;
}

void  CSerializedValue::GetAllPointersOfChildren(std::vector<void*> *ptrs)
{
	ptrs->push_back(m_originalPtr);

	for( size_t i = 0; i < m_children.size(); ++i )
		m_children[i]->GetAllPointersOfChildren(ptrs);
}

CSerializedValue *CSerializedValue::FindByPtr(void *ptr)
{
	if( m_originalPtr == ptr )
		return this;

	for( size_t i = 0; i < m_children.size(); i++ )
	{
		CSerializedValue *find = m_children[i]->FindByPtr(ptr);
		if( find )
			return find;
	}

	return 0;
}

void *CSerializedValue::GetPointerToRestoredObject(void *ptr)
{
	if( m_originalPtr == ptr )
		return m_restorePtr;

	for( size_t i = 0; i < m_children.size(); ++i )
	{
		void *ret = m_children[i]->GetPointerToRestoredObject(ptr);
		if( ret )
			return ret;
	}

	return 0;
}

// find variable by ptr but looking only at those in the references, which will create a new object
CSerializedValue *CSerializedValue::FindByPtrInHandles(void *ptr)
{
	// if this handle created object
	if( (m_typeId & asTYPEID_OBJHANDLE) && m_children.size() == 1 )
	{
		if( m_children[0]->m_originalPtr == ptr )
			return this;
	}

	if( !(m_typeId & asTYPEID_OBJHANDLE) )
	{
		for( size_t i = 0; i < m_children.size(); i++ )
		{
			CSerializedValue *find = m_children[i]->FindByPtrInHandles(ptr);
			if( find )
				return find;
		}
	}

	return 0;
}

void CSerializedValue::Create(void *ref, int typeId)
{
	m_isInit = true;
	SetType(typeId);

	
}

void CSerializedValue::Store(void *ref, int typeId)
{
	m_isInit = true;
	if (!m_serializer || !m_serializer->m_engine) return;
	SetType(typeId);
	m_originalPtr = ref;

	if( m_typeId & asTYPEID_OBJHANDLE )
	{
		m_handlePtr = *(void**)ref;
	}
	else if( m_typeId & asTYPEID_SCRIPTOBJECT )
	{
		asIScriptObject *obj = (asIScriptObject *)ref;
		asITypeInfo *type = obj->GetObjectType();
		SetType(type->GetTypeId());

		// Store children 
		for( asUINT i = 0; i < type->GetPropertyCount(); i++ )
		{	
			int childId;
			const char *childName;
			type->GetProperty(i, &childName, &childId);

			m_children.push_back(new CSerializedValue(this, childName, "", obj->GetAddressOfProperty(i), childId));
		}
	}
	else
	{
		int size = m_serializer->m_engine->GetSizeOfPrimitiveType(m_typeId);
		
		if( size == 0 )
		{			
			// if it is user type( string, array, etc ... )
			if( m_serializer->m_userTypes[m_typeName] )
				m_serializer->m_userTypes[m_typeName]->Store(this, m_originalPtr);
			
			// it is script class
			else if (GetType())
				size = GetType()->GetSize();
		}

		if( size )
		{
			m_mem.resize(size);
			memcpy(&m_mem[0], ref, size);
		}
	}
}

void CSerializedValue::Restore(void *ref, int typeId)
{
	if( !this || !m_isInit || !ref )
		return;

	// Verify that the stored type matched the new type of the value being restored
	if( typeId <= asTYPEID_DOUBLE && typeId != m_typeId ) return; // TODO: We may try to do a type conversion for primitives
	if( (typeId & ~asTYPEID_MASK_SEQNBR) ^ (m_typeId & ~asTYPEID_MASK_SEQNBR) ) return;
	asITypeInfo *type = m_serializer->m_engine->GetTypeInfoById(typeId);
	if( type && m_typeName != type->GetName() ) return;

	// Set the new pointer and type
	m_restorePtr = ref;
	SetType(typeId);

	// Restore the value
	if( m_typeId & asTYPEID_OBJHANDLE )
	{
		// if need create objects
		if( m_children.size() == 1 )
		{
			asITypeInfo *type = m_children[0]->GetType();

			if( type->GetFactoryCount() == 0 )
			{
				m_children[0]->m_restorePtr = m_handlePtr;
			}
			else
			{
				// Create a new script object, but don't call its constructor as we will initialize the members. 
				// Calling the constructor may have unwanted side effects if for example the constructor changes
				// any outside entities, such as setting global variables to point to new objects, etc.
				void *newObject = m_serializer->m_engine->CreateUninitializedScriptObject(type);
				m_children[0]->Restore(newObject, type->GetTypeId());
			}
		}
	}
	else if( m_typeId & asTYPEID_SCRIPTOBJECT )
	{
		asIScriptObject *obj = (asIScriptObject *)ref;
		asITypeInfo *type = GetType();
		if (!type) return ;
		// Retrieve children
		for( asUINT i = 0; i < type->GetPropertyCount() ; i++ )
		{	
			const char *nameProperty;
			int typeId;
			type->GetProperty(i, &nameProperty, &typeId);
			
			CSerializedValue *var = FindByName(nameProperty, "");
			if( var )
				var->Restore(obj->GetAddressOfProperty(i), typeId);
		}
	}
	else
	{
		if( m_mem.size() )
		{
			// POD values can be restored with direct copy
			memcpy(ref, &m_mem[0], m_mem.size());
		}
		else if( m_serializer->m_userTypes[m_typeName] )
		{
			// user type restore
			m_serializer->m_userTypes[m_typeName]->Restore(this, m_restorePtr);
		}
		else
		{
			std::string str = "Cannot restore type '";
			str += type->GetName();
			str += "'";
			m_serializer->m_engine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, str.c_str());
		}
	}
}

void CSerializedValue::CancelDuplicates(CSerializedValue *from)
{
	std::vector<void*> ptrs;
	from->GetAllPointersOfChildren(&ptrs);

	for( size_t i = 0; i < ptrs.size(); ++i )
	{
		CSerializedValue *find = m_serializer->m_root.FindByPtrInHandles(ptrs[i]);

		while( find )
		{
			// cancel create object
			find->ClearChildren();

			// Find next link to this ptr
			find = m_serializer->m_root.FindByPtrInHandles(ptrs[i]);
		}
	}
}

void CSerializedValue::ReplaceHandles()
{
	if( m_handlePtr )
	{
		// Find the object that the handle is referring to
		CSerializedValue *handle_to = m_serializer->m_root.FindByPtr(m_handlePtr);
		
		// If the object hasn't been stored yet...
		if( handle_to == 0 )
		{
			// Store the object now
			asITypeInfo *type = GetType();
			CSerializedValue *need_create = new CSerializedValue(this, m_name, m_nameSpace, m_handlePtr, type->GetTypeId()); 

			// Make sure all other handles that point to the same object 
			// are updated, so we don't end up creating duplicates 
			CancelDuplicates(need_create);

			m_children.push_back(need_create);
		}
	}

	// Replace the handles in the children too
	for( size_t i = 0; i < m_children.size(); ++i )
		m_children[i]->ReplaceHandles();
}

void CSerializedValue::RestoreHandles()
{
	if( m_typeId & asTYPEID_OBJHANDLE )
	{
		if( m_handlePtr )
		{
			// Find the object the handle is supposed to point to
			CSerializedValue *handleTo = m_serializer->m_root.FindByPtr(m_handlePtr);

			if( m_restorePtr && handleTo && handleTo->m_restorePtr )
			{
				asITypeInfo *type = m_serializer->m_engine->GetTypeInfoById(m_typeId);

				// If the handle is already pointing to something it must be released first
				if( *(void**)m_restorePtr )
					m_serializer->m_engine->ReleaseScriptObject(*(void**)m_restorePtr, type);

				// Update the internal pointer
				*(void**)m_restorePtr = handleTo->m_restorePtr;

				// Increase the reference
				m_serializer->m_engine->AddRefScriptObject(handleTo->m_restorePtr, type);
			}
		}
		else
		{
			// If the handle is pointing to something, we must release it to restore the null pointer
			if( m_restorePtr && *(void**)m_restorePtr )
			{
				m_serializer->m_engine->ReleaseScriptObject(*(void**)m_restorePtr, m_serializer->m_engine->GetTypeInfoById(m_typeId));
				*(void**)m_restorePtr = 0;
			}
		}
	}

	// Do the same for the children
	for( size_t i = 0; i < m_children.size(); ++i )
		m_children[i]->RestoreHandles();
}

void CSerializedValue::SetType(int typeId)
{
	m_typeId = typeId;

	asITypeInfo *type = m_serializer->m_engine->GetTypeInfoById(typeId);

	if( type )
		m_typeName = type->GetName();
}

asITypeInfo *CSerializedValue::GetType()
{
	if( !m_typeName.empty() )
	{
		int newTypeId = m_serializer->m_mod->GetTypeIdByDecl(m_typeName.c_str());
		return m_serializer->m_engine->GetTypeInfoById(newTypeId);
	}	

	return 0;
}

void CSerializedValue::SetUserData(void *data)
{
	m_userData = data;
}

void CSerializedValue::print(int depth)
{
	std::string a = "";
	loop(depth) a += ">";
	a += m_typeName + m_name;
	conoutf("%s", a.c_str());
	for (uint i = 0; i < m_children.size(); i++)
	{
		m_children[i]->print(depth+1);
	}
}

void *CSerializedValue::GetUserData()
{
	return m_userData;
}

uint CSerializedValue::GetTypeId() { return m_typeId; }

//////////////////////////////////////////////////////
void CSerialnode::Create(CSerializedValue *val)
{
	loopi(val->m_children.size()) {
		if (!val->m_children[i]->m_restorePtr)
		{ //ok something is screwed up we have null pointer so lets do what we can
			if (i < 2)
			{ //Ok if its less then 2 then that means that the vectors for position and rotation got corrupted. So lets try to fix them
				val->m_children[i]->m_restorePtr = new vec(); //create a new pointer and set it to the default (0,0,0);
				val->m_children[i]->ClearChildren(); //kill all the children so nothing bad happens.
			}

			else; //some asobject got corrupted. so lets just remove it cuz we dont know whats wrong (probably just missing the file)
			//we will let this go for now and catch it later
		}
		//now we check to make sure that the objects are the correct thing we need
		if (i < 2) {
			if (val->m_children[i]->GetTypeId() != CSerialvec::getID()) {
				val->m_children[i]->m_restorePtr = new vec();
				val->m_children[i]->ClearChildren(); //kill all the children so nothing bad happens.
			}
		}
		else
			if (!(val->m_children[i]->GetTypeId() & asTYPEID_SCRIPTOBJECT))
				val->m_children[i]->m_restorePtr = 0;
	}
	//now lets create the object
	node *n;
	if (val->m_children.size() < 2)
		n = new node(vec(), vec());
	else {
		n = new node(*(vec *) val->m_children[0]->m_restorePtr, *(vec *) val->m_children[1]->m_restorePtr);
	}
	if (val->m_children.size() > 2)
	{
		n->name = *(std::string*)val->m_children[2]->GetUserData();
		
	}
	if (val->m_children.size() > 3)
	{
		loopk(val->m_children.size() - 3) 
		{
			bool flag = false;
			asIScriptObject *c = (asIScriptObject *) val->m_children[k+3]->m_restorePtr;
			if(c) for (int i = 0; i < c->GetPropertyCount(); i++)
			{
				if (str(c->GetPropertyName(i)) == str("self"))
				{
					// Set the property to point to the node
					//*reinterpret_cast<node**>(n->ctrl->GetAddressOfProperty(i)) = (node *) n;
					asScript->assigntoptr<node>(c->GetAddressOfProperty(i), n, true);
					// Increment the refcount of node since the asobject now holds a reference
					n->ctrl.add(c);
					n->addref();
					flag = true;
				}
			}
			if(!flag)  conoutf("error assigning Self %s", typeidtoname(c->GetTypeId())); //class had no declared self @node we need to remove this object from the node;
		}
	}
	curworld->addnodetoscene(n);
}
void CSerialnode::Store(CSerializedValue *val, void *ptr)
{
	node *g = new node(*((const node *) ptr));
	val->m_children.push_back(new CSerializedValue(val, "pos", "", new vec(g->o), CSerialvec::getID())); //store vec o
	val->m_children.push_back(new CSerializedValue(val, "rot", "", new vec(g->rot), CSerialvec::getID())); // store vec rot
	val->m_children.push_back(new CSerializedValue(val, "name", "", new str(g->name), CSerialstring::getID())); //store the name
	loopv(g->ctrl) if(g->ctrl[i]) val->m_children.push_back(new CSerializedValue(val, "ctrl", "", g->ctrl[i], g->ctrl[i]->GetTypeId()));
	//if(g->ctrl) val->m_children.push_back(new CSerializedValue(val, "ctrl", "", g->ctrl, g->ctrl->GetTypeId())); //store the controler for this element
}
void CSerialnode::Restore(CSerializedValue *val, void *ptr)
{
	node *g = (node *) ptr;
	val->m_children[0]->Restore(&g->o, CSerialvec::getID());
	val->m_children[1]->Restore(&g->rot, CSerialvec::getID()); 
	val->m_children[2]->Restore(&g->name, CSerialstring::getID());
	g->ctrl.growbuf(val->m_children.size() - 3);
	loopi(g->ctrl.length()) val->m_children[2]->Restore(g->ctrl[i], g->ctrl[i]->GetTypeId());
	if (!g->ctrl.length())conoutf("No Ctrls attatched to control");
	//if (val->m_children[2] && g->ctrl) val->m_children[2]->Restore(g->ctrl, g->ctrl->GetTypeId());
}

void CSerialnode::CleanupUserData(CSerializedValue *val)
{
	//std::string *buffer = (std::string*)val->GetUserData();
	//delete buffer;
}
int CSerialnode::serialID = -1;

//////////////////////////////////////////////////////////

void CSerialvec::Create(CSerializedValue *val)
{
	vec v(0);
	
	loopi(val->m_children.size())
	{
		if (i >= 3) break;
		if (!val->m_children[i] || !val->m_children[i]->m_restorePtr) continue;
		v[i] = *(float *) val->m_children[i]->m_restorePtr;
	}
	val->m_restorePtr = new vec(v);
}

void CSerialvec::Store(CSerializedValue *val, void *ptr)
{
	vec *o = new vec(*(vec *) ptr);
	vec *p = (vec *) ptr;
	val->m_children.push_back(new CSerializedValue(val, "x", "", &o->x, asTYPEID_FLOAT));
	val->m_children.push_back(new CSerializedValue(val, "y", "", &o->y, asTYPEID_FLOAT));
	val->m_children.push_back(new CSerializedValue(val, "z", "", &o->z, asTYPEID_FLOAT));
}

void CSerialvec::Restore(CSerializedValue *val, void *ptr)
{
	vec *o = (vec *)ptr;
	val->m_children[0]->Restore(&o->x, asTYPEID_FLOAT);
	val->m_children[1]->Restore(&o->y, asTYPEID_FLOAT);
	val->m_children[2]->Restore(&o->z, asTYPEID_FLOAT);
}

void CSerialvec::CleanupUserData(CSerializedValue *val)
{
	return;
}
int CSerialvec::serialID = -1;

void CSerialstring::Create(CSerializedValue *val)
{
	str buffer = str();
	buffer.resize(val->m_mem.size());
	memcpy(&buffer[0], &val->m_mem[0], val->m_mem.size());
	val->m_restorePtr = new str(buffer);
	//std::string *buffer = (std::string*)val->m_restorePtr;
	val->SetUserData(val->m_restorePtr);
}

void CSerialstring::Store(CSerializedValue *val, void *ptr)
{
	val->m_restorePtr = new str(*(str *) ptr);
}

void CSerialstring::Restore(CSerializedValue *val, void *ptr)
{
	std::string *buffer = (std::string*)val->m_restorePtr;
	*(std::string*)ptr = *buffer;
}

void CSerialstring::CleanupUserData(CSerializedValue *val) 
{
}
int CSerialstring::serialID = -1;

void CSerialvector::Create(CSerializedValue *val)
{
	CScriptArray *buffer = CScriptArray::Create(val->m_children[0]->GetType(), val->m_children.size());
	val->m_restorePtr = buffer;
	val->SetUserData(buffer);
	for (size_t i = 0; i < val->m_children.size(); ++i)
		val->m_children[i]->Restore(buffer->At(asUINT(i)), buffer->GetElementTypeId());
}

void CSerialvector::Store(CSerializedValue *val, void *ptr)
{
	CScriptArray *arr = (CScriptArray*) ptr;
	for (unsigned int i = 0; i < arr->GetSize(); i++)
		val->m_children.push_back(new CSerializedValue(val, "", "", arr->At(i), arr->GetElementTypeId()));
}

void CSerialvector::Restore(CSerializedValue *val, void *ptr)
{
	CScriptArray *arr = (CScriptArray*) ptr;
	arr->Resize(asUINT(val->m_children.size()));
	for (size_t i = 0; i < val->m_children.size(); ++i)
		val->m_children[i]->Restore(arr->At(asUINT(i)), arr->GetElementTypeId());
}

void CSerialvector::CleanupUserData(CSerializedValue *val) 
{
}
int CSerialvector::serialID = -1;

END_AS_NAMESPACE
