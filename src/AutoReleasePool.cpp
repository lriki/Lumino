
#include "Internal.h"
#include <Lumino/AutoReleasePool.h>

namespace Lumino
{

//=============================================================================
// AutoReleasePool
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AutoReleasePool::AutoReleasePool()
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AutoReleasePool::~AutoReleasePool()
{

}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AutoReleasePool* AutoReleasePool::GetCurrent()
{
	return NULL;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AutoReleasePool::AddObject(RefObject* obj)
{
	m_objectList.push_back(obj);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AutoReleasePool::Clear()
{
	std::vector<RefObject*> releaseList;
	releaseList.swap(m_objectList);
	for (RefObject* obj : releaseList) {
		obj->Release();
	}
}


} // namespace Lumino
