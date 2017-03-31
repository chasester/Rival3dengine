#include <iostream>
#include <string>
#include <string.h> // strstr()
#include <assert.h>
#include <math.h>
#include "scriptstdstring.h"
#include "scripthelper.h"
#include "cube.h"

BEGIN_AS_NAMESPACE

void summonAngel(const char *cmd, const char *arg){

    if (asScript->asEngine == 0){ conoutf("scripting engine failed to launch"); return; }

    // Interpret the command
    if (!strcmp(cmd, "exec"))
        ExecString(asScript->asEngine, arg);
    /*else if (cmd == "addfunc")
        AddFunction(engine, arg);
    else if (cmd == "delfunc")
        DeleteFunction(engine, arg);
    else if (cmd == "addvar")
        AddVariable(engine, arg);
    else if (cmd == "delvar")
        DeleteVariable(engine, arg);
    else if (cmd == "help")
        PrintHelp();
    else if (cmd == "listfuncs")
        ListFunctions(engine);
    else if (cmd == "listvars")
        ListVariables(engine);*/
    else
        conoutf("UKNOWN COMMAND");

    // Shut down the engine


    return;

}

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	conoutf("%s (%d,%d) : %s : %s", msg->section, msg->row, msg->col, type, msg->message);
}

COMMAND(summonAngel, "ss");
void ExecString(asIScriptEngine *engine, const char *arg)
{
	asIScriptModule *mod = engine->GetModule("console", asGM_CREATE_IF_NOT_EXISTS);

	int refTypeId = asTYPEID_VOID;
	defformatstring(funcCode, "%s %s%s%s", engine->GetTypeDeclaration(asTYPEID_VOID, true), "ExecuteString() {\n", arg, ";\n}");

    asITypeInfo *type = 0;

	if (refTypeId & asTYPEID_MASK_OBJECT)
	{
        type = engine->GetObjectTypeByIndex(refTypeId);
		if (type)
			type->AddRef();
	}

	asIScriptModule *execMod = mod ? mod : engine->GetModule("ExecuteString", asGM_ALWAYS_CREATE);

	if (type) type->Release();

	asIScriptFunction *func = 0;
	int r = execMod->CompileFunction("ExecuteString", funcCode, -1, 0, &func);
	if (r < 0){ conoutf("compile error %d", r); return; }
	asIScriptContext *execCtx = engine->RequestContext();
	r = execCtx->Prepare(func);
	if (r < 0){func->Release();execCtx->Release();return;}
	conoutf("prepared");
	r = execCtx->Execute();

	func->Release();
	engine->ReturnContext(execCtx);
	conoutf("executed");
}

void helloworld(){ conoutf("helloWorld"); }



void print(int v){ conoutf("%d", v); }
void print(asUINT v){ conoutf("%d", v); }
void print(bool v){ conoutf("%s", v ? "true" : "false"); }
void print(float v){ conoutf("%f", v); }
void print(double v){ conoutf("%.10f", float(v)); }
//void print(string v){ conoutf("%d", v); }
void print(){}

//static void stringFactoryGeneric(asIScriptGeneric *gen)
//{
//	asUINT length = gen->GetArgDWord(0);
//	const char *s = (const char*)gen->GetArgAddress(1);
//
//	// Return a string value
//	new (gen->GetAddressOfReturnLocation()) string;
//}


void config(){
	int r; //error checker
    //RegisterStdString(asScript->asEngine);
    asScript->asEngine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);
	//string a;
    r = asScript->asEngine->RegisterGlobalFunction("void helloworld()", asFUNCTION(helloworld), asCALL_CDECL);// assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("float sin(float)", asFUNCTION(sinf), asCALL_CDECL);// assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("float cos(float)", asFUNCTION(cosf), asCALL_CDECL);// assert(r >= 0);

    r = asScript->asEngine->RegisterGlobalFunction("void print(bool)", asFUNCTIONPR(print, (bool), void), asCALL_CDECL);// assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("void print(int)", asFUNCTIONPR(print, (int), void), asCALL_CDECL); //assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("void print(uint)", asFUNCTIONPR(print, (asUINT), void), asCALL_CDECL); //assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("void print(float)", asFUNCTIONPR(print, (float), void), asCALL_CDECL); //assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("void print(double)", asFUNCTIONPR(print, (double), void), asCALL_CDECL); //assert(r >= 0);
    r = asScript->asEngine->RegisterGlobalFunction("void print()", asFUNCTIONPR(print, (void), void), asCALL_CDECL); //assert(r >= 0);
	//r = engine->RegisterGlobalFunction("void print(string)", asFUNCTIONPR(print, (string), void), asCALL_CDECL);

    asScript->asEngine->ClearMessageCallback();
}
END_AS_NAMESPACE

//void asEngineShutdown(){ asEngineShutdown(); } //shutdown engine called in main::shutdown

//void asConfigureEngine(){ asConfigureEngine(); } //starts engine up and inits major variables called in main::main



