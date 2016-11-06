#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

//#include "LinearMath/btQuickprof.h"
//#include "LinearMath/btIDebugDraw.h"


#include "LinearMath/btDefaultMotionState.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btDefaultMotionState.h"


extern void addridgidbody(ridgidbody &r, node *g);


class btBroadphaseInterface;
class btCollisionShape;
class btOverlappingPairCache;
class btCollisionDispatcher;
class btConstraintSolver;
struct btCollisionAlgorithmCreateFunc;
class btDefaultCollisionConfiguration;
class btTriangleIndexVertexArray;
//btDiscreteDynamicsWorld* m_dynamicsWorld;

extern int lastphysicframe;


extern int PHYSDebugDraw;

btRigidBody *setcbfrommodel(vec &o, const char *mdl, vec dir, int weight);//attatch collision box to model

extern void buildLevelTriCol();// build level collision mesh

extern void PHYSInit();
extern void PHYSKill();
extern void PHYSStep();
extern void PHYSrebuildLevel();// rebuild level collision mesh
struct btTEMPMELOOKATLATER
{
	vector<btRigidBody *> bodies;
	vector<btTypedConstraint *> contraints;
};