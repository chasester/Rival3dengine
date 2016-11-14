//CSerializer::CSerializer()
//{
//	m_engine = 0;
//}
//
//CSerializer::~CSerializer()
//{
//	// Clean the serialized values before we remove the user types
//	m_root.Uninit();
//
//	// Delete the user types
//	std::map<std::string, CUserType*>::iterator it;
//	for (it = m_userTypes.begin(); it != m_userTypes.end(); it++)
//		delete it->second;
//
//	if (m_engine)
//		m_engine->Release();
//}
#include "cube.h";