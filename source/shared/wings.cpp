
#include "cube.h"
#include "engine.h"

//#include "scripthelper.h"
asIScriptContext *execCtx; //global reference to the context controls so we dont have to pass it to and from the engine when running one set of code. Remember to always return or error will run;

//using namespace std;

ScriptManager *asScript = new ScriptManager();
#define ASEngine asScript->asEngine

//commandline angel script calling
void summonAngel(const char *arg){ if (ASEngine == 0){ conoutf("scripting engine failed to launch"); return; } ExecString(ASEngine, arg); }
COMMAND(summonAngel, "s");
asGCOMMMAND(void, buildLevelTriCol, (void))
void execangelfile(const char *file){};
COMMAND(execangelfile, "s");

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	int mgtype = CON_ERROR;
	const char *type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING){
		type = "WARN";
		mgtype = CON_WARN;
	}
	else if (msg->type == asMSGTYPE_INFORMATION){
		type = "INFO";
		mgtype = CON_INFO;
	}
	conoutf(mgtype, "%s (%d,%d) : %s : %s", msg->section, msg->row, msg->col, type, msg->message);
}

void hello(){ print(str("HELLo")); }

asGCOMMMAND(void, hello, (void))
struct funcstore
{
private:
	asSFuncPtr func;
	str _class;
public:
	str decl;

	funcstore(asSFuncPtr f, str d, str c) : func(f), decl(d), _class(c){}
	void regiserfunction() { if (_class == "") addfunction(decl.c_str(), func, asCALL_CDECL, 0); }
};

vector<funcstore *> functions;

bool addfunc(asSFuncPtr f, str d, str _class){ functions.add(new funcstore(f, d, _class)); return true; }


void compilefunction(asIScriptEngine * engine, const char *arg){}


asIScriptFunction *createFunction(asIScriptEngine *engine, str code, int refTypeId)
{
	
	asIScriptModule *mod = engine->GetModule("console", asGM_CREATE_IF_NOT_EXISTS);

	asITypeInfo *type = 0;

	if (refTypeId & asTYPEID_MASK_OBJECT)
	{
		type = engine->GetTypeInfoById(refTypeId);
		if (type)
			type->AddRef();
	}
	asIScriptModule *execMod = mod ? mod : engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);
	if (type) type->Release();

	asIScriptFunction *func = 0;
	int r = execMod->CompileFunction("ExecuteString", code.c_str(), -1, 0, &func);
	if (r < 0){ conoutf("compile error %d", r); return NULL; }
	return func;
}
asIScriptFunction *createExecFunction(asIScriptEngine *engine, str code, int refTypeId)
{
	int r;
	asIScriptFunction *func = createFunction(engine, code, refTypeId);
	asIScriptContext *execCtx = engine->RequestContext();
	r = execCtx->Execute();
	if (r < 0){ writeerror(r); return  NULL; }
	engine->ReturnContext(execCtx);
	return func;
}


void ExecString(asIScriptEngine *engine, const char *arg)
{
	//fix the str so that it has void ExecuteString(){ in front and ;} behind it
	str funcCode = str(engine->GetTypeDeclaration(asTYPEID_VOID, true)) + " ExecuteString() {\n" + str(arg) + ";\n}";

	int r;
	asIScriptFunction *func = createFunction(engine, funcCode, asTYPEID_VOID);
	if (!func)return;
	asIScriptContext *execCtx = engine->RequestContext();
	r = execCtx->Prepare(func);
	if (r < 0){func->Release(); execCtx->Release(); return;}
	
	r = asScript->ExecuteCall(execCtx);
	//if (r < 0){ writeerror(r); return; }
	func->Release(); //remove the func so that we dont throw an error if we want to run different code through the commandline
	engine->ReturnContext(execCtx);

	//remove
}


bool FATALERROR(int r, int line, const char *file)
{
	writeerror(r);
	conoutf("\
		ANGEL SCRIPT ENGINE HAS RUN AN ERROR IN THE INTERAL CODE\n \
		IF ERROR CONTINUES PLEASE REDOWNLOAD ENGINE \n \
		Error in file: %s on line: %d",\
		file, line);
	return true;
}

void print(vec &v){ conoutf("x:%f y:%f z:%f",v.x, v.y, v.z); }
void print(int v){ conoutf("%d", v); }
void print(asUINT v){ conoutf("%d", v); }
void print(bool v){ conoutf("%s", v ? "true" : "false"); }
void print(float v){ conoutf("%f", v); }
void print(double v){ conoutf("%.10f", float(v)); }
void print(str v){ conoutf("%s", v.c_str()); }
void print(const char *v){ conoutf("%s", v); }
void print() {};

//static void strFactoryGeneric(asIScriptGeneric *gen)
//{
//	asUINT length = gen->GetArgDWord(0);
//	const char *s = (const char*)gen->GetArgAddress(1);
//
//	// Return a str value
//	new (gen->GetAddressOfReturnLocation()) str;
//}
//int a = asEngine->RegisterGlobalFunction("void print(bool)", asFUNCTIONPR(print, (bool), void), asCALL_CDECL);
void writeerror(int r) { conoutf(CON_ERROR, "%d SHUTDOWN ENGINE", r); }


//wrapper functions for adding global functions through ICOMMAND COMMAND and VAR macros
int addfunction(const char *declaration, const asSFuncPtr &funcPointer, asDWORD callConv, void *objForThiscall)
{
	int r = ASEngine->RegisterGlobalFunction(declaration, funcPointer, callConv, objForThiscall); return r;
}
#define SETUPMATHFUNC(type) {	asCOMMAND(type, min, (type, type)) asCOMMAND(type, max, (type, type)) asCOMMAND(type, clamp, (type, type, type));\
						 asCOMMAND(type, cos, (type)); asCOMMAND(type, sin, (type));}
void addvarible(const char *declaration, void *pointer){ int r = ASEngine->RegisterGlobalProperty(declaration, pointer);  }

//#define BASECONSTUCTOR(type) static void BaseConstructor##type(asIScriptGeneric * gen){  }
//#define COPYCONSTUCTOR(type) static void CopyConstructor##type(asIScriptGeneric * gen){ new (gen->GetObject()) type (*static_cast<type *>(gen->GetArgObject(0))); }
//#define DECONSTUCTOR(type) static void CopyConstructor##type(asIScriptGeneric * gen){ vec * ptr = static_cast<vec *>(gen->GetObject());ptr->~vec(); }
//#define CUSTOMCONSTRUCTOR(type, name, args) static void Construct##name##type(asIScriptGeneric * gen)

#define FUNCSET(name) void name(asIScriptGeneric * gen)
#define TEMP0ARG  template<typename C>
#define TEMP1ARG  template<typename C, typename A1>
#define TEMP2ARG  template<typename C, typename A1, typename A2>
#define TEMP3ARG  template<typename C, typename A1, typename A2, typename A3>
#define TEMP4ARG  template<typename C, typename A1, typename A2, typename A3, typename A4>
#define TEMP5ARG  template<typename C, typename A1, typename A2, typename A3, typename A4, typename A5> 
#define TEMP6ARG  template<typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
#define TEMP7ARG  template<typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
#define TEMP8ARG  template<typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
#define TEMP9ARG  template<typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
#define TEMP10ARG template<typename C, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9,typename A10>

#define GETDECONSTRUCTOR(c) ~##c

//set all functions here
TEMP0ARG FUNCSET(BaseConstructor) { new (gen->GetObject()) C(); }
TEMP1ARG FUNCSET(ConstructorArg1) { new (gen->GetObject()) C(*static_cast<A1 *>(gen->GetAddressOfArg(0))); }
TEMP2ARG FUNCSET(ConstructorArg2) { new (gen->GetObject()) C(*static_cast<A1 *>(gen->GetAddressOfArg(0)), *static_cast<A2 *>(gen->GetAddressOfArg(1))); }
TEMP3ARG FUNCSET(ConstructorArg3) { new (gen->GetObject()) C(*static_cast<A1 *>(gen->GetAddressOfArg(0)), *static_cast<A2 *>(gen->GetAddressOfArg(1)), *static_cast<A3 *>(gen->GetAddressOfArg(2)));}
TEMP0ARG FUNCSET(DeConstructor)   { (static_cast<C *>(gen->GetObject()))->GETDECONSTRUCTOR(C)(); }

//opperator types here
//inclusive opperators add to the object and return the object with new value.. 
TEMP1ARG FUNCSET(AsnOperator) { C *a = static_cast<C*>(gen->GetObject()); *a = C(*static_cast<A1 *>(gen->GetAddressOfArg(0))); gen->SetReturnAddress(a); } //assign from
TEMP1ARG FUNCSET(AddOperator) {C ret_val = (*static_cast<C *>(gen->GetObject())) + C(*static_cast<A1 *>(gen->GetArgAddress(0))); gen->SetReturnObject(&ret_val);} //add from
TEMP1ARG FUNCSET(SubOperator) {C ret_val = *static_cast<C *>(gen->GetObject()) - C(*static_cast<A1 *>(gen->GetArgAddress(0))); gen->SetReturnObject(&ret_val); } //subtract form
TEMP1ARG FUNCSET(DivOperator) {C ret_val = *static_cast<C *>(gen->GetObject()) / C(*static_cast<A1 *>(gen->GetArgAddress(0))); gen->SetReturnObject(&ret_val); } //divid by
TEMP1ARG FUNCSET(MulOperator) {C ret_val = *static_cast<C *>(gen->GetObject()) * C(*static_cast<A1 *>(gen->GetArgAddress(0))); gen->SetReturnObject(&ret_val); } //muliply by


//TEMP1ARG FUNCSET(RAsnOperator) {C ret_val = C(*static_cast<A1 *>(gen->GetArgAddress(0))) + *static_cast<C *>(gen->GetObject()); gen->SetReturnObject(&ret_val); } //assign to
TEMP1ARG FUNCSET(RAddOperator) { C ret_val = C(*static_cast<A1 *>(gen->GetArgAddress(0))) + *static_cast<C *>(gen->GetObject()); gen->SetReturnObject(&ret_val); } //add to
TEMP1ARG FUNCSET(RSubOperator) {C ret_val = C(*static_cast<A1 *>(gen->GetArgAddress(0))) - *static_cast<C *>(gen->GetObject()); gen->SetReturnObject(&ret_val); } //substract to
TEMP1ARG FUNCSET(RDivOperator) {C ret_val = C(*static_cast<A1 *>(gen->GetArgAddress(0))) / *static_cast<C *>(gen->GetObject()); gen->SetReturnObject(&ret_val); } // divide to
TEMP1ARG FUNCSET(RMulOperator) {C ret_val = C(*static_cast<A1 *>(gen->GetArgAddress(0))) * *static_cast<C *>(gen->GetObject()); gen->SetReturnObject(&ret_val); } //multipy to

TEMP1ARG FUNCSET(AddAsnOperator) { C ret_val = (*static_cast<C *>(gen->GetObject())) + C(*static_cast<A1 *>(gen->GetArgAddress(0))); new (gen->GetObject()) C(ret_val); gen->SetReturnObject(&ret_val); } //add assign
TEMP1ARG FUNCSET(SubAsnOperator) { C ret_val = (*static_cast<C *>(gen->GetObject())) - C(*static_cast<A1 *>(gen->GetArgAddress(0))); new (gen->GetObject()) C(ret_val); gen->SetReturnObject(&ret_val); } //add assign
TEMP1ARG FUNCSET(DivAsnOperator) { C ret_val = (*static_cast<C *>(gen->GetObject())) / C(*static_cast<A1 *>(gen->GetArgAddress(0))); new (gen->GetObject()) C(ret_val); gen->SetReturnObject(&ret_val); } //add assign
TEMP1ARG FUNCSET(MulAsnOperator) { C ret_val = (*static_cast<C *>(gen->GetObject())) * C(*static_cast<A1 *>(gen->GetArgAddress(0))); new (gen->GetObject()) C(ret_val); gen->SetReturnObject(&ret_val); } //add assign

void CopyConstructor(asIScriptGeneric *gen){ vec *a = static_cast<vec *>(gen->GetArgObject(0)); new (gen->GetObject()) vec(*a); };
//define use type defs here
template void BaseConstructor<vec>(asIScriptGeneric * gen); //base constructor
template void ConstructorArg1<vec, vec>(asIScriptGeneric * gen); //copy constructor
template void ConstructorArg1<vec, float>(asIScriptGeneric * gen); //constructor float to vec
template void ConstructorArg3<vec, float, float,float>(asIScriptGeneric * gen); //constructor float3 to vec
template void DeConstructor  <vec>(asIScriptGeneric * gen);

//define opperational functions
//assigin
 template void AsnOperator<vec, float>(asIScriptGeneric *gen);
 template void AsnOperator<vec, vec>(asIScriptGeneric *gen);
//add
 template void AddOperator<vec, float>(asIScriptGeneric *gen);
 template void AddOperator<vec, vec>(asIScriptGeneric *gen);
 template void RAddOperator<vec, float>(asIScriptGeneric *gen);
 template void RAddOperator<vec, vec>(asIScriptGeneric *gen);
 template void AddAsnOperator<vec, float>(asIScriptGeneric *gen);
 template void AddAsnOperator<vec, vec>(asIScriptGeneric *gen);
//sub
 template void SubOperator<vec, float>(asIScriptGeneric *gen);
 template void SubOperator<vec, vec>(asIScriptGeneric *gen);
 template void RSubOperator<vec, float>(asIScriptGeneric *gen);
 template void RSubOperator<vec, vec>(asIScriptGeneric *gen);
 template void SubAsnOperator<vec, float>(asIScriptGeneric *gen);
 template void SubAsnOperator<vec, vec>(asIScriptGeneric *gen);
//div
 template void DivOperator<vec, float>(asIScriptGeneric *gen);
 template void DivOperator<vec, vec>(asIScriptGeneric *gen);
 template void RDivOperator<vec, float>(asIScriptGeneric *gen);
 template void RDivOperator<vec, vec>(asIScriptGeneric *gen);
 template void DivAsnOperator<vec, float>(asIScriptGeneric *gen);
 template void DivAsnOperator<vec, vec>(asIScriptGeneric *gen);
//mul
 template void MulOperator<vec, float>(asIScriptGeneric *gen);
 template void MulOperator<vec, vec>(asIScriptGeneric *gen);
 template void MulAsnOperator<vec, float>(asIScriptGeneric *gen);
 template void MulAsnOperator<vec, vec>(asIScriptGeneric *gen);
 template void RMulOperator<vec, float>(asIScriptGeneric *gen);
 template void RMulOperator<vec, vec>(asIScriptGeneric *gen);

//
//CUSTOMCONSTRUCTOR(vec, float){ new(gen->GetObject()) vec(*static_cast<float *>(gen->GetAddressOfArg(0))); };
//CUSTOMCONSTRUCTOR(vec, float3){ new(gen->GetObject()) vec(*static_cast<float *>(gen->GetAddressOfArg(0)), *static_cast<float *>(gen->GetAddressOfArg(1)), *static_cast<float *>(gen->GetAddressOfArg(2)));}

//static void float3ConstructVec3(asIScriptGeneric * gen){ }
//static void floatConstuctVec3(asIScriptGeneric * gen){ }
//static void CopyConstructVec3(asIScriptGeneric * gen){ }
//static void DestructVec3(asIScriptGeneric * gen){}
//static void AddVecToFloat(asIScriptGeneric * gen){ (static_cast<vec *>(gen->GetObject))->add(*(static_cast<float *>(gen->GetAddressOfArg(0))));}
//static void AddFloatToVec(asIScriptGeneric * gen){ gen->SetReturnAddress((static_cast<vec *>(gen->GetObject))->add(*(static_cast<float *>(gen->GetAddressOfArg(0))))); }

void RegisterVec3(asIScriptEngine *engine){
	int r;
	r = engine->RegisterObjectType("vec", sizeof(vec), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(r >= 0);
	asScript->serializer->AddUserType(new CSerialvec(r), "vec");
	//set up all constructors
	r = engine->RegisterObjectBehaviour("vec", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(BaseConstructor<vec>), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("vec", asBEHAVE_CONSTRUCT, "void f(float a)", asFUNCTION((ConstructorArg1<vec,float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("vec", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z)", asFUNCTION((ConstructorArg3<vec,float,float,float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("vec", asBEHAVE_CONSTRUCT, "void f(const vec &in)", asFUNCTION((CopyConstructor)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("vec", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DeConstructor<vec>), asCALL_GENERIC); assert(r >= 0);
	
	//setup operators
	//assign
	r = engine->RegisterObjectMethod("vec", "vec &opAssign(float)", asFUNCTION((AsnOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec &opAssign(vec)", asFUNCTION((AsnOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	//math functions
	r = engine->RegisterObjectMethod("vec", "vec opAdd(const vec &in)", asFUNCTION((AddOperator<vec,vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opAdd(const float &in)", asFUNCTION((AddOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opSub(const vec &in)", asFUNCTION((SubOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opSub(const float &in)", asFUNCTION((SubOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opDiv(const vec &in)", asFUNCTION((DivOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opDiv(const float &in)", asFUNCTION((DivOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opMul(const vec &in)", asFUNCTION((MulOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opMul(const float &in)", asFUNCTION((MulOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	//reverse math functions
	r = engine->RegisterObjectMethod("vec", "vec opAdd_r(const vec &in)", asFUNCTION((RAddOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opAdd_r(const float &in)", asFUNCTION((RAddOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opSub_r(const vec &in)", asFUNCTION((RSubOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opSub_r(const float &in)", asFUNCTION((RSubOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opDiv_r(const vec &in)", asFUNCTION((RDivOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opDiv_r(const float &in)", asFUNCTION((RDivOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opMul_r(const vec &in)", asFUNCTION((RMulOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opMul_r(const float &in)", asFUNCTION((RMulOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	//math assign functions
	r = engine->RegisterObjectMethod("vec", "vec opAddAssign(const vec &in)", asFUNCTION((AddAsnOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opAddAssign(const float &in)", asFUNCTION((AddAsnOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opSubAssign(const vec &in)", asFUNCTION((SubAsnOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opSubAssign(const float &in)", asFUNCTION((SubAsnOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opDivAssign(const vec &in)", asFUNCTION((DivAsnOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opDivAssign(const float &in)", asFUNCTION((DivAsnOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opMulAssign(const vec &in)", asFUNCTION((MulAsnOperator<vec, vec>)), asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("vec", "vec opMulAssign(const float &in)", asFUNCTION((MulAsnOperator<vec, float>)), asCALL_GENERIC); assert(r >= 0);

	//vec properties
	r = engine->RegisterObjectProperty("vec", "float x", asOFFSET(vec, x));
	r = engine->RegisterObjectProperty("vec", "float y", asOFFSET(vec, y));
	r = engine->RegisterObjectProperty("vec", "float z", asOFFSET(vec, z));
	//using union to declar another way
	r = engine->RegisterObjectProperty("vec", "float r", asOFFSET(vec, r));
	r = engine->RegisterObjectProperty("vec", "float g", asOFFSET(vec, g));
	r = engine->RegisterObjectProperty("vec", "float b", asOFFSET(vec, b));
}

void wrapper_adddynamiclight(const vec &o, float rad, const vec &color, int fade=0, int peak =0,float initrad = 0, const vec &initcolor = vec(0,0,0))
{
	adddynlight(o, rad, color,fade,peak,0,initrad, initcolor);
}
void wrapper_addlight(const vec &o, const vec &color, int radius, int type)
{
	curworld->addlight(o, color, radius, type % 5);
}
void wrapper_rendermapmodel(int ind, int ani, const vec &o, const vec &rot)
{
	const char *mdlname = mapmodelname(ind);
	if (!mdlname) return;
	rendermodel(mdlname, ANIM_MAPMODEL | ANIM_LOOP, o, rot.x, rot.y, rot.z, MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED|MDL_CENTER_ALIGNED);
} //wrapper because there are of default arguments that should not be used

void createobject(str s, vec &o, vec &rot)
{
	node *g  = newasent(s, o, false);
	if (!g) return;
}

void rebuildmodule(const char * script) { asScript->recompilemodule(script); curworld->doAwake(); }
COMMAND(rebuildmodule, "s");

int randomint(int i){ return i > 0 ? rand() % i : 0; /*fixes error if i = 0;*/}

void config(asIScriptEngine *asEngine){
	int r; //error checker
	asEngine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); 
	asScript->setupserializer(new CSerializer());
	RegisterStdString(asEngine);
	//RegisterScriptArray(asEngine, true);
	RegisterScriptHandle(asEngine);
	RegisterScriptWeakRef(asEngine);
	RegisterScriptArray(asEngine, true);
	loopv(functions)
	{ 
		if (!asEngine->GetGlobalFunctionByDecl(functions[i]->decl.c_str())) //check to make sure some one didnt use this function twice with the same declaration
			functions[i]->regiserfunction();
	}
	//add math funcitons
	SETUPMATHFUNC(float);
	//SETUPMATHFUNC(double);
	//asEngine->RegisterGlobalProperty("const float PI", &int(pi));
	//asEngine->RegisterGlobalProperty("const float RAD", &int(rad));
	asEngine->RegisterGlobalProperty("const int lastmillis", &lastmillis);

	RegisterVec3(asEngine);

	r = asEngine->RegisterGlobalFunction("void print(bool)", asFUNCTIONPR(print, (bool), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print(int)", asFUNCTIONPR(print, (int), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print(uint)", asFUNCTIONPR(print, (asUINT), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print(float)", asFUNCTIONPR(print, (float), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print(double)", asFUNCTIONPR(print, (double), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print()", asFUNCTIONPR(print, (void), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print(string)", asFUNCTIONPR(print, (str), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("void print(const vec &in)", asFUNCTIONPR(print, (vec&), void), asCALL_CDECL); asFATAL(r);
	r = asEngine->RegisterGlobalFunction("int randomint(int i)", asFUNCTIONPR(randomint, (int), int), asCALL_CDECL); asFATAL(r);
	//r = asEngine->RegisterGlobalFunction("void print(array)", )

	//r = asEngine->RegisterGlobalFunction("void rebuildmodule(string a)", asFUNCTIONPR(rebuildmodule, (str), void), asCALL_CDECL); asFATAL(r);

	r = asEngine->RegisterGlobalFunction("void adddynamiclight(const vec &in, float radius, const vec &in, int fade=0, int peak=0, float initrad=0.0f, const vec &in = vec(0,0,0))", asFUNCTIONPR(wrapper_adddynamiclight, (const vec&, float, const vec&, int, int, float, const vec&), void), asCALL_CDECL);
	r = asEngine->RegisterGlobalFunction("void rendermapmodel(int index,int ani, const vec &in, const vec &in)", asFUNCTIONPR(wrapper_rendermapmodel, (int, int, const vec&, const vec&), void), asCALL_CDECL);
	r = asEngine->RegisterGlobalFunction("void addlight(const vec &in, const vec &in, int radius, int type)", asFUNCTIONPR(wrapper_addlight, (const vec&, const vec&, int, int), void), asCALL_CDECL);
#define setupobjectpropert(o,t, p) asEngine->RegisterObjectProperty( str(#o).c_str() ,  str(str(#t) + " " +  str(#p)).c_str(), asOFFSET(o, p)); assert(r >= 0);

	//setup type light
	//r = asEngine->RegisterObjectType("light", sizeof(light), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(r >= 0);
	//r = setupobjectpropert(light, vec, color);
	//r = setupobjectpropert(light, vec, o);
	//r = setupobjectpropert(light, int, type);
	//r = setupobjectpropert(light, int, radius);

	//setup ridgid body
	r = asEngine->RegisterObjectType("ridgidbody", sizeof(ridgidbody), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(r >= 0);
	r = setupobjectpropert(ridgidbody, float, friction);
	r = setupobjectpropert(ridgidbody, vec, gravity); 
	r = setupobjectpropert(ridgidbody, float, weight);
	r = setupobjectpropert(ridgidbody, float, restitution);
	r = setupobjectpropert(ridgidbody, float, ldamp);
	r = setupobjectpropert(ridgidbody, float, adamp);
	r = setupobjectpropert(ridgidbody, vec, afactor);
	r = setupobjectpropert(ridgidbody, vec, lfactor);
	r = setupobjectpropert(ridgidbody, int, type); 
	r = setupobjectpropert(ridgidbody, vec, o); 
	r = setupobjectpropert(ridgidbody, vec, rot); 
	r = setupobjectpropert(ridgidbody, vec, center);
	r = setupobjectpropert(ridgidbody, vec, radius);


	//set up the game object temperary varible for test later replace with entity
	r = asEngine->RegisterObjectType("node", 0, asOBJ_REF ); assert(r >= 0);
	asScript->serializer->AddUserType(new CSerialnode(r), "node");
	

	

	r = asEngine->RegisterInterface("IController");

	r = asEngine->RegisterObjectBehaviour("node", asBEHAVE_ADDREF, "void f()", asMETHOD(node, addref), asCALL_THISCALL); assert(r >= 0);
	r = asEngine->RegisterObjectBehaviour("node", asBEHAVE_RELEASE, "void f()", asMETHOD(node, release), asCALL_THISCALL); assert(r >= 0);
	r = asEngine->RegisterObjectBehaviour("node", asBEHAVE_GET_WEAKREF_FLAG, "int &f()", asMETHOD(node, getweakflagref), asCALL_THISCALL); assert(r >= 0);

	r = asEngine->RegisterObjectProperty("node", "string name", asOFFSET(node, name));
	r = asEngine->RegisterObjectProperty("node", "vec o", asOFFSET(node, o));
	r = asEngine->RegisterObjectProperty("node", "vec rot", asOFFSET(node, rot));
	//r = asEngine->RegisterObjectMethod("node", "void seteditbox(const vec &in)", asMETHOD(node, seteditbox), asCALL_THISCALL); assert(r >= 0);
	r = asEngine->RegisterObjectMethod("node", "void seteditbox(int i)", asMETHOD(node, seteditbox), asCALL_THISCALL); assert(r >= 0);
	//rigid body and physics
	r = asEngine->RegisterObjectType("ridgidbody", sizeof(ridgidbody), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert(r >= 0);
	r = asEngine->RegisterObjectBehaviour("ridgidbody", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(BaseConstructor<ridgidbody>), asCALL_GENERIC); assert(r >= 0);
	r = asEngine->RegisterObjectBehaviour("ridgidbody", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DeConstructor<ridgidbody>), asCALL_GENERIC); assert(r >= 0);
	r = asEngine->RegisterObjectMethod("ridgidbody", "void aabbfrommodel(int index)", asMETHOD(ridgidbody, aabbfrommodel), asCALL_THISCALL); assert(r >= 0);
	r = asEngine->RegisterGlobalFunction("void addridgidbody(const ridgidbody &in, const node @+ to)", asFUNCTIONPR(addridgidbody, (ridgidbody&, node*), void), asCALL_CDECL); asFATAL(r); 
	r = asEngine->RegisterGlobalFunction("void createobject(string s, const vec &in, const vec &in)", asFUNCTIONPR(createobject, (str, vec&, vec&), void), asCALL_CDECL); asFATAL(r);
	asEngine->ClearMessageCallback();
}
END_AS_NAMESPACE

void asEngineShutdown(){ ASEngine->ShutDownAndRelease(); } //shutdown asEngine called in main::shutdown

void asConfigureEngine() {
	ASEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION); if (ASEngine) config(ASEngine); else conoutf("asengine didnt build");
} //starts asEngine up and inits major variables called in main::main

//////////////////////
ScriptManager::ScriptManager() : asEngine(NULL), serializer(NULL){ }

ScriptManager::ScriptControler *ScriptManager::getctrlscript(const str &script, bool checkctrl){
		//if we allready made the module then lets try  and find it	
		if (checkctrl) loopv(ctrls)
		if (ctrls[i]->module == script) return ctrls[i];

		//no controller created yet, ok see if we loaded it before but could not initialize it
		if (asEngine->GetModule(script.c_str(), asGM_ONLY_IF_EXISTS)){ print("Module already had an error"); return NULL; }
		//ok no controler and no module lets create one 
		CScriptBuilder builder;
		r = builder.StartNewModule(asEngine, script.c_str());
		if (r < 0){ print("module not created, Interal error please try restarting or reinstalling"); return 0; }

		FILE *f;
		if ((f = fopen((script + ".halo").c_str(), "r")) == 0){ print("There is no file " + script + ".halo"); return 0; }
		fclose(f);

		r = builder.AddSectionFromFile((script + ".halo").c_str());
		if (r == 0)print(str("WARNING: File \"" + script + ".halo\" has already been included"));
		else if (r < 0){ print(str("ERR: File \"" + script + ".halo\" Has ran into error")); }
		r = builder.BuildModule(); if (r < 0) { print(str("failed to init module: Syntax error" )); print(r); return 0; }

		ScriptControler *ctrl = new ScriptControler();
		ctrl->module = script;

		asIScriptModule *mod = asEngine->GetModule(script.c_str(), asGM_ONLY_IF_EXISTS);
		if (!mod){ return 0; }
		asITypeInfo *type = 0;
		int tc = mod->GetObjectTypeCount();
		loopi(tc){
			bool found = false;
			type = mod->GetObjectTypeByIndex(i);
			int ic = type->GetInterfaceCount();
			loopj(ic)
			if (strcmp(type->GetInterface(i)->GetName(), "IController") == 0)
			{
				found = true; break;
			}

			if (found == true) { ctrl->type = type; break; }
		}
		if (ctrl->type == 0)
		{
			print("Couldn't find the controller class for the type " + script); delete ctrl; return 0;
		}

		ctrls.add(ctrl);
		ctrl->onCreate = type->GetMethodByDecl("void Create()");
		ctrl->onAwake = type->GetMethodByDecl("void Awake()");
		ctrl->onUpdate = type->GetMethodByDecl("void Update()");
		ctrl->onRender = type->GetMethodByDecl("void Render()");
		ctrl->onGui = type->GetMethodByDecl("void Gui()");
		ctrl->onDestroy = type->GetMethodByDecl("void Destroy()");
		//ctrl->onMessageMethod = type->GetMethodByDecl("void OnMessage(ref @msg, const node @sender)");
		type->SetUserData(ctrl);

		serializer->Store(asEngine->GetModule(script.c_str(), asGM_ONLY_IF_EXISTS));
		return ctrl;
	}

asIScriptContext *ScriptManager::PrepareContextFromPool(asIScriptFunction *func)
	{
		if (!func) return 0;
		asIScriptContext *ctx = 0;
		if (contexts.length())
		{
			ctx = contexts.pop();
		}
		else
			ctx = asEngine->CreateContext();

		int r = ctx->Prepare(func); assert(r >= 0);

		return ctx;
	}

void ScriptManager::ReturnContextToPool(asIScriptContext *ctx) { contexts.add(ctx); ctx->Unprepare(); }

void ScriptManager::setupserializer(CSerializer *s){ serializer = s; s->SetEngine(asEngine); }

void ScriptManager::setupdatatypeasserializedtype(){}

void ScriptManager::createSerialized(asIScriptModule * mod){ serializer->Store(mod); }

void ScriptManager::createSerialized(str mod)
	{
		asIScriptModule *m = asEngine->GetModule(mod.c_str(), asGM_ONLY_IF_EXISTS);
		if (m) return createSerialized(m); // the module has already been loaded YAYA
		asEngine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, "Error loading module: have you loaded it?");
		return;
		//later add a load functionality here
	}

void ScriptManager::restoreSerialized(asIScriptModule *mod){ serializer->Restore(mod); }

void ScriptManager::restoreSerialized(str mod)
	{
		asIScriptModule *m = asEngine->GetModule(mod.c_str(), asGM_ONLY_IF_EXISTS);
		if (m)return restoreSerialized(m);
		asEngine->WriteMessage("", 0, 0, asMSGTYPE_ERROR, "Error loading module: have you loaded it?");
		return;
	}

void ScriptManager::resetserializer(){ serializer->ClearRoot(); }

int ScriptManager::ExecuteCall(asIScriptContext *ctx)
	{
		int r = ctx->Execute();
		if (r != asEXECUTION_FINISHED)
		{
			if (r == asEXECUTION_EXCEPTION)
			{
				print("Exception: " + str(ctx->GetExceptionString()));
				print("Function: " + str(ctx->GetExceptionFunction()->GetDeclaration()));
				print(str("Line: " + ctx->GetExceptionLineNumber()));

				// It is possible to print more information about the location of the 
				// exception, for example the call stack, values of variables, etc if 
				// that is of interest.
			}
		}
		return r;
	}

void ScriptManager::recompilemodule(str script)
	{
		createSerialized(script);
		serializer->print();
		asEngine->DiscardModule(script.c_str());
		ScriptControler *ctrl = getctrlscript(script, false);
		if (!ctrl){ print(str("could not get control script"));  return; }
		loopv(ctrls)
		{
			if (ctrls[i]->module == script)
			{
				ctrls[i]->copy(ctrl);
				return;
			}
		}
		restoreSerialized(script);
	}

str ScriptManager::getprimitivename(int id)
	{
		switch (id)
		{
		case asTYPEID_VOID:
			return "void";
		case asTYPEID_BOOL:
			return "bool";
		case asTYPEID_INT8:
		case asTYPEID_INT16:
		case asTYPEID_INT32:
		case asTYPEID_INT64:
			return "int";
		case asTYPEID_UINT8:
		case asTYPEID_UINT16:
		case asTYPEID_UINT32:
		case asTYPEID_UINT64:
			return "uint";
		case asTYPEID_FLOAT:
			return "float";
		case asTYPEID_DOUBLE:
			return "double";
		}
	}

asIScriptObject *ScriptManager::CreateController(const str &script, node *gameObj)
	{
		if (script == "") return 0;
		asIScriptObject *instance;
		 ScriptControler*ctrl = getctrlscript(script);
		if (!ctrl || !ctrl->type)return 0;
		instance = (asIScriptObject *) (asScript->asEngine->CreateUninitializedScriptObject(ctrl->type));
		for (int i = 0; i < instance->GetPropertyCount(); i++)
		{
			if (str(instance->GetPropertyName(i)) == "self")
			{
				// Set the property to point to the node
				*reinterpret_cast<node**>(instance->GetAddressOfProperty(i)) = gameObj;

				// Increment the refcount of node since the asobject now holds a reference
				gameObj->addref();
				//doCreate(instance);
				return instance;
			}
		}
		print("Could not find node \"self\". All controler types must have a node @self declared");
		return 0;
		//asIScriptContext *ctx = PrepareContextFromPool(ctrl->factoryFunc);

		//// Pass the object pointer to the script function. With this call the 
		//// context will automatically increase the reference count for the object.
		//ctx->SetArgObject(0, gameObj);

		//// Make the call and take care of any errors that may happen
		//r = ExecuteCall(ctx);

		//if (r == asEXECUTION_FINISHED)
		//{
		//	// Get the newly created object
		//	instance = *((asIScriptObject**)ctx->GetAddressOfReturnValue());

		//	// Since a reference will be kept to this object 
		//	// it is necessary to increase the ref count
		//	instance->AddRef();		
		//}
		//ReturnContextToPool(ctx);
		//doCreate(instance);
		////call the default create function to replace the constructor
		//
		////serializer->AddExtraObjectToStore(instance);
		//// Return the context to the pool so it can be reused
		////printinstancedata(instance->GetObjectType(), 0);
		//return instance;
	}

void ScriptManager::printinstancedata(asITypeInfo *typ, int level)
	{
		if (!typ) return;
		bool pri, prot, ref;
		int offset, typeId;
		const char *name;
		if (level == 0){ level++; print(str(typ->GetName())); }
		loopi(typ->GetPropertyCount()){
			str indent;
			str typeName;
			asITypeInfo *temp = 0;
			int index = typ->GetProperty(i, &name, &typeId, &pri, &prot, &offset, &ref);
			if (index < 0)return;
			if (typeId <= asTYPEID_DOUBLE) typeName = getprimitivename(typeId);
			else
			{
				temp = asEngine->GetTypeInfoById(typeId);
				if (temp)typeName = temp->GetName();
				else typeName = "unknown";
			}
			loopi(level)indent += ">";
			print(indent + str(prot ? "protected " : pri ? "private " : "public ") + typeName + " " + str(typeId & asTYPEID_OBJHANDLE ? "@" : "") + str(name));
			if (temp)
			{
				printinstancedata(temp, level + 1);
			}
		}
	}

#define CREATEACTIONTYPESDEC(call)\
	void ScriptManager::do##call(asIScriptObject *object)\
	{ \
	ScriptControler *ctrl = reinterpret_cast<ScriptControler *>(object->GetObjectType()->GetUserData()); \
	if (!ctrl) return; \
	if (ctrl->on##call == 0) return; \
	asIScriptContext *ctx = PrepareContextFromPool(ctrl->on##call); \
	ctx->SetObject(object); \
	ExecuteCall(ctx); \
	ReturnContextToPool(ctx); \
	};

	CREATEACTIONTYPESDEC(Awake)
	CREATEACTIONTYPESDEC(Create)
	CREATEACTIONTYPESDEC(Update)
	CREATEACTIONTYPESDEC(Render)
	CREATEACTIONTYPESDEC(Gui)
	CREATEACTIONTYPESDEC(Destroy)

	
	void ScriptManager::assigntoptrfromtypeid(void *a, void *b, int id)
	{
		bool handle = false;// id & asTYPEID_OBJHANDLE;
		if (id < asTYPEID_DOUBLE + 1) {
			switch (id)
			{
			case(asTYPEID_VOID):
				return;
			case(asTYPEID_BOOL):
				assigntoptr<bool>(a, b, handle);
				return;
			case(asTYPEID_INT8):
			case(asTYPEID_UINT8):
				assigntoptr<__int8>(a, b, handle);
				return;
			case(asTYPEID_INT16):
			case(asTYPEID_UINT16):
				assigntoptr<__int16>(a, b, handle);
				return;
			case(asTYPEID_INT32):
			case(asTYPEID_UINT32):
				assigntoptr<__int32>(a, b, handle);
				return;
			case(asTYPEID_INT64):
			case(asTYPEID_UINT64):
				assigntoptr<__int64>(a, b, handle);
				return;
			case(asTYPEID_FLOAT):
				assigntoptr<float>(a, b, handle);
				return;
			case(asTYPEID_DOUBLE):
				assigntoptr<double>(a, b, handle);
				return;
			default:
				conoutf("ERROR IN ASSIGNING");
				return;
			}
		}
		else return;
			//assigntoptr<asIScriptObject>(a, b, handle);
	}

	void *ScriptManager::createprivitiveptr(int id)
	{
		if (id > asTYPEID_DOUBLE) return nullptr;
			switch (id)
			{
			case(asTYPEID_VOID):
				return nullptr;
			case(asTYPEID_BOOL):
				return new bool;
			case(asTYPEID_INT8):
			case(asTYPEID_UINT8):
				return new __int8;
			case(asTYPEID_INT16):
			case(asTYPEID_UINT16):
				return new __int16;
			case(asTYPEID_INT32):
			case(asTYPEID_UINT32):
				return new __int32;
			case(asTYPEID_INT64):
			case(asTYPEID_UINT64):
				return new __int64;
			case(asTYPEID_FLOAT):
				return new float;
			case(asTYPEID_DOUBLE):
				return new double;
			default:
				conoutf("ERROR IN ASSIGNING");
				return nullptr;
			}
	}
	 void *ScriptManager::convertPrimitive(void *v, int type)
	{
		void *b = asScript->createprivitiveptr(type);
		if (!b) return nullptr;
		switch (type)
		{
		case(asTYPEID_VOID):
			return nullptr;
		case(asTYPEID_BOOL):
			changerefptr<bool>(b, v);
			return b;
		case(asTYPEID_INT8):
		case(asTYPEID_UINT8):
			changerefptr<__int8>(b, v);
			return b;
		case(asTYPEID_INT16):
		case(asTYPEID_UINT16):
			changerefptr<__int16>(b, v);
			return b;
		case(asTYPEID_INT32):
		case(asTYPEID_UINT32):
			changerefptr<__int32>(b, v);
			return b;
		case(asTYPEID_INT64):
		case(asTYPEID_UINT64):
			changerefptr<__int64>(b, v);
			return b;
		case(asTYPEID_FLOAT):
			changerefptr<float>(b, v);			
			return b;
		case(asTYPEID_DOUBLE):
			changerefptr<double>(b, v);
			return b;
		default:
			conoutf("ERROR IN ASSIGNING");
			return nullptr;
		}
	}
	void ScriptManager::ScriptControler::copy(ScriptControler *ctrl)
	{
		module = ctrl->module;
		type = ctrl->type;
		onMessageMethod = ctrl->onMessageMethod;
		onCreate = ctrl->onCreate;
		onAwake = ctrl->onAwake;
		onUpdate = ctrl->onUpdate;
		onRender = ctrl->onRender;
		onDestroy = ctrl->onDestroy;
	}

	void ScriptManager::ScriptControler::reload(){}

