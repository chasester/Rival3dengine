#ifndef HALO_H
#define HALO_H

#include "angelscript.h" //base angelscirpt include

#include "scriptstdstring.h" //string

void print(int);
void print(asUINT);
void print(bool);
void print(float);
void print(double);
void print(str);
void print(void);

struct node;

//include various data type
#include "scripthandle.h" //handles
#include "scriptbuilder.h" //compiling functionality
#include "weakref.h" //weak ref<>
#include "scriptarray.h" //template style arrays<>
#include "serializer.h" //serializer


extern int addfunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall = 0);

struct funcstore;

extern vector<funcstore *> functions;

extern bool addfunc(asSFuncPtr f, str d, str _class = "");

#define asGCOMMMAND(ret, n, args) bool __dummyprop__##n = addfunc(asFUNCTIONPR(n,args,ret), (str(#ret) + " " + str(#n) + str(#args))); 

//set up COMMAND ICOMMAND VAR etc like interfacing, The following interface saves back the functional data so that it cant be inted inside of the engine config()
//bool addstruct() { return true; }
//bool addproperty()
//
//
//#define asStruct(n) static bool __dummystruct__##n = addstruct(str(#n)); struct n 
//#define asAddProp(t,n,o) bool __dummyprop__##n = addprop(type, name, obj, val); //t = type, n = name, o = object (null if global)
//#define asAddPropVal(type, name, obj);
//asStruct(n){};
//#define asCOMMMAND(ret, name, args)
// Function prototypes
//asIScriptFunction createFunction(asIScriptEngine *engine, string code, int refTypeId);
void ExecString(asIScriptEngine *engine, const char *arg);
void writeerror(int r);



//asIScriptEngine *asEngine = 0;
void config(asIScriptEngine *);

//print functions



void hello();
//asGCOMMMAND(void, hello, (void));


//errorhandling for inline functions and vars
bool FATALERROR(int r, int line, const char *file);
#define asFATAL if (r<0)FATALERROR(r, __LINE__,  __FILE__);
 
//exection functions
#define asCOMMAND(ret,name, args) static bool _dummy_##name = addfunction( (str(#ret) + " " + str(#name) + str(#args)).c_str() , asFUNCTIONPR(name, args, ret), asCALL_CDECL, 0) < 0 ? !FATALERROR(0, __LINE__,  __FILE__)  : true;
//#define asVAR(type, name) asEngine->RegisterGlobalProperty((str(#type) + " " + str(#name)).c_str(), name);
//#define asVARC(type, name) asEngine->RegisterGlobalProperty(("const " + str(#type) + " " + str(#name)).c_str(), &#type (name) );


END_AS_NAMESPACE
void asConfigureEngine();
void asEngineShutdown();

struct IController
{
	int a;
	node *self;
};



struct node;
struct ScriptManager 
{
private:
	struct ScriptControler 
	{

		str module;
		asITypeInfo     *type;
		//store a link to the action functions
		asIScriptFunction *onMessageMethod;
		asIScriptFunction *onCreate;
		asIScriptFunction *onAwake;
		asIScriptFunction *onUpdate;
		asIScriptFunction *onRender;
		asIScriptFunction *onGui;
		asIScriptFunction *onDestroy;

		void reload();
		void copy(ScriptControler *ctrl);
	};
	struct prefab
	{
		str name;
		//vector<asITypeInfo *> types;
		vector<ScriptControler *> objects;
		//vector<const char *> paths;
		prefab(str n, vector<str> paths);
	};
	
public:
	int r;
	bool hasCompileErrors;
	asIScriptEngine *asEngine;
	vector<asIScriptContext *> contexts;
	vector<ScriptControler *> ctrls;
	vector<prefab *> prefabs;
	CSerializer *serializer;

	ScriptManager();

	//prefabs
	//serializer
	void setupserializer(CSerializer *s);
	void setupdatatypeasserializedtype();
	void createSerialized(asIScriptModule * mod);
	void createSerialized(str mod);
	void restoreSerialized(asIScriptModule *mod);
	void restoreSerialized(str mod);
	void resetserializer();

	int ExecuteCall(asIScriptContext *ctx);
	void recompilemodule(str script);
	str getprimitivename(int id);
	asIScriptObject *CreateController(const str &script, node *gameObj);

	//setup action functions
#define CREATEACTIONTYPESDEF(call) void do##call(asIScriptObject *object);
	CREATEACTIONTYPESDEF(Awake);
	CREATEACTIONTYPESDEF(Create);
	CREATEACTIONTYPESDEF(Update);
	CREATEACTIONTYPESDEF(Render);
	CREATEACTIONTYPESDEF(Gui);
	CREATEACTIONTYPESDEF(Destroy);
	
	#define CODE_TO_STRING(C) #C

template<typename T> void assigntoptr(void *a, void *b, bool handle = false)
{
	try
	{
		if (handle) *reinterpret_cast<T**>(a) = (T *) (b);
		else *reinterpret_cast<T*>(a) = *(T *) (b);
	}
	catch (...)
	{
		conoutf("Not a script object of primitive " );
	}
}
template<typename T> void changerefptr(void *b, void *v)
{
	//*reinterpret_cast<T *>(b) = (T (*v));
	
}
void assigntoptrfromtypeid(void *a, void *b, int id);
void *createprivitiveptr(int id);
void *convertPrimitive(void * v, int type);

private:
	//setup context and context pool
	ScriptControler *getctrlscript(const str &script, bool checkctrl = true);
	asIScriptContext *PrepareContextFromPool(asIScriptFunction *func);
	void ReturnContextToPool(asIScriptContext *ctx);

	//debug print data so that we can see what kind of data is being held here
	void printinstancedata(asITypeInfo *typ, int level);
	
};
extern ScriptManager *asScript;

#include "node.h"

#endif
