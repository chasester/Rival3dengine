//#include "cube.h"
//struct rawdata;
//struct userserialize;
//struct blueprint;
//
//struct serializer
//{
//	serializer();
//	~serializer();
//
//	rawdata *storenode(node *g);
//	blueprint *getobjectlayout(asIScriptObject *o);
//
//	vector<userserialize *> usertypes;
//	
//	
//	
//};
//
//struct rawdata
//{
//	rawdata *parent;
//	rawdata *fchild;
//	rawdata *nsib;
//	int typei;
//	str typen;
//	str name;
//	void *data;
//
//	rawdata(int id, str tn, str n, rawdata *parent = NULL, rawdata *fchild = NULL, rawdata *nsib = NULL) : typei(id), typen(tn), name(n), parent(parent), fchild(fchild), nsib(nsib){}
//	~rawdata();
//};