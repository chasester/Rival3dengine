#ifndef HALO_H
#define HALO_H

#include "angelscript.h" //base angelscirpt include

#include "scriptstdstring.h" //string

//and basic print functionality
//re write to allow for templating from any class. If class has a to_str() then call that otherwise just print the [classname]
void print(int);
void print(asUINT);
void print(bool);
void print(float);
void print(double);
void print(str);
void print(void);

struct node; //define nodes here so we can use them later

//include various data type
#include "scripthandle.h" //handles
#include "scriptbuilder.h" //compiling functionality
#include "weakref.h" //weak ref<>
#include "scriptarray.h" //template style arrays<>
#include "serializer.h" //serializer


#pragma region asICOMMANDANDVARANDCLASSFUNCTIONALITY


//TODO: add in functionallity for ICOMMAND like calls
extern int addfunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall = 0);

//structure for above functionality
struct funcstore;

extern vector<funcstore *> functions;

extern bool addfunc(asSFuncPtr f, str d, str _class = "");

//macro for above functionality
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
#define asCOMMAND(ret,name, args) static bool _dummy_##name = addfunction( (str(#ret) + " " + str(#name) + str(#args)).c_str() , asFUNCTIONPR(name, args, ret), asCALL_CDECL, 0) < 0 ? !FATALERROR(0, __LINE__,  __FILE__)  : true;
//#define asVAR(type, name) asEngine->RegisterGlobalProperty((str(#type) + " " + str(#name)).c_str(), name);
//#define asVARC(type, name) asEngine->RegisterGlobalProperty(("const " + str(#type) + " " + str(#name)).c_str(), &#type (name) );
#pragma endregion

void ExecString(asIScriptEngine *engine, const char *arg);
void writeerror(int r);



//sets up the engine
void config(asIScriptEngine *); 


//errorhandling for inline functions and vars
bool FATALERROR(int r, int line, const char *file);
#define asFATAL if (r<0)FATALERROR(r, __LINE__,  __FILE__); //this doesnt work correctly trie and fix later this is soley for debuging and shouldnt be a release from. This info can be sent to an error handling server later

END_AS_NAMESPACE //unused but important later if we impliment more namespaced approach (this macro may be in the wrong spot though.

void asConfigureEngine(); //this sets up the engine with a  set of default configuration, latter add peramaters to allow for more control over the script engine
void asEngineShutdown(); //this ends the engine, is called on crash or exit of the program, this just releases memory 

//Base controler for all objects, this is not implimented yet, need to move from an interface system to a c++ inheritence.
struct IController
{
	int a;
	node *self;
};

struct ScriptManager  //handles all interfacing between asEngine and corisponding parts (user input, script file loading, and serializer)
{
private:
	struct ScriptControler  //this is a base for all controlers to hold onto ptrs to there main called functions, this is to speed up the time it takes to call these functions. may need changed
	{

		str module; //string name of the file path to the script file (and name)
		asITypeInfo     *type; // type info for object this stores all needed info of the control class (inlcuding things like name, typeID member varianbles and functions, etc) see as Documentation for more info
		//store a link to the action functions
		asIScriptFunction *onMessageMethod;
		asIScriptFunction *onCreate;
		asIScriptFunction *onAwake;
		asIScriptFunction *onUpdate;
		asIScriptFunction *onRender;
		asIScriptFunction *onGui;
		asIScriptFunction *onDestroy;

		void reload(); //function that re runs the function code if requested by command reloadmodule (or for any other reason) this will change the code and pointer functions and typeInfo  allowing for realtime changes to reflect
		void copy(ScriptControler *ctrl); //simple copy code
	};
	struct prefab //prefabs are not implimented but will be a preset set of striptcontrolers to allow for quick modulization in the editor
	{
		str name; 
		//vector<asITypeInfo *> types;
		vector<ScriptControler *> objects;
		//vector<const char *> paths;
		prefab(str n, vector<str> paths);
	};
	
public:
	int r; //r is a test variable to see if any thing crashes if r < 0 then we are crashing and we need to look at error codes to see why
	bool hasCompileErrors; //a quick check to see if we are crashing
	asIScriptEngine *asEngine; //a ptr to the engine we are using (should be a get, variable so we dont get hairy issues)
	vector<asIScriptContext *> contexts; //contexts are used for compileing script objects this is just a pool so we can reuse them, right now we hardly have more than one element but this could allow for multiple objects to load at once and/or multithreading
	vector<ScriptControler *> ctrls; //list of ctrl objects (these are like base classes that control nodes, we can reuse these over and over)
	vector<prefab *> prefabs; //list of prefabs so we can have the ablitiy to list the preset sets of controls
	CSerializer *serializer; //the serializer :)

	ScriptManager(); //contructor, not really important

	//prefabs
	//serializer
	void setupserializer(CSerializer *s); //Init the serializer and set the engine and other nessary variables, Keep in mind the engine has to exist for this to run correctly
	void setupdatatypeasserializedtype(); //this does nothing do not investigate :)
	void createSerialized(asIScriptModule * mod); //this creates the serializer tree from a module (look at as documention for definition of mod)
	void createSerialized(str mod); //this creates a serializer tree from a name of a module (same as above)
	void restoreSerialized(asIScriptModule *mod); //this restores from a mod
	void restoreSerialized(str mod); //this restores from a name that gets the mod
	void resetserializer(); //this kills the serializer and starts over

	int ExecuteCall(asIScriptContext *ctx); // this allows you t run a context (look at as documentation for definition of a context)
	void recompilemodule(str script); //this recompliles a module that errored or that already has been compiled (this is the call that allows realtime changes to code while running the game)
	str getprimitivename(int id); //this takes in a primitive that is less or equal to asTYPEID_DOUBLE, else it returns nothing, this is used for print calls and general debuging
	asIScriptObject *CreateController(const str &script, node *gameObj); //this takes a stript name (which is same as path for now, and attatches this controler to the node if no controler exist or the context returns an error node never gets a controler and returns void
	//setup action functions
#define CREATEACTIONTYPESDEF(call) void do##call(asIScriptObject *object); //Macro for ptr function calls.
	CREATEACTIONTYPESDEF(Awake);
	CREATEACTIONTYPESDEF(Create);
	CREATEACTIONTYPESDEF(Update);
	CREATEACTIONTYPESDEF(Render);
	CREATEACTIONTYPESDEF(Gui);
	CREATEACTIONTYPESDEF(Destroy);
	
	#define CODE_TO_STRING(C) #C //this macro takes code and turns into string

template<typename T> void assigntoptr(void *a, void *b, bool handle = false) // assigns a ptr to a ptr of the same type (intreral do not use directly)
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
template<typename T> void changerefptr(void *b, void *v) //did something once :p but needs removed
{
	//*reinterpret_cast<T *>(b) = (T (*v));
	
}
void assigntoptrfromtypeid(void *a, void *b, int id); //this is the public side of assigntoptr(), used to take a ptr and assign another ptr of the same type to eachother using an as ID to relate them
void *createprivitiveptr(int id); //creates a new T for any perimitive type use ASTYPEID_ to set the type
void *convertPrimitive(void * v, int type); // doesnt work yet so dont use

private:
	//setup context and context pool
	ScriptControler *getctrlscript(const str &script, bool checkctrl = true); //intreral call for creating ctrl and attatching to node
	asIScriptContext *PrepareContextFromPool(asIScriptFunction *func); //interal pool function
	void ReturnContextToPool(asIScriptContext *ctx); //internal pool function

	//debug print data so that we can see what kind of data is being held here
	void printinstancedata(asITypeInfo *typ, int level); //this is not really used
	
};
extern ScriptManager *asScript;

#include "node.h"

#endif
