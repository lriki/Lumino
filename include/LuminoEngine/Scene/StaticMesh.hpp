﻿
#pragma once
#include "VisualObject.hpp"

namespace ln {
class StaticMeshComponent;

class StaticMesh
	: public VisualObject
{
public:
    static Ref<StaticMesh> create();
    static Ref<StaticMesh> create(const StringRef& filePath, float scale = 1.0f);

    StaticMeshComponent* staticMeshComponent() const;

protected:

LN_CONSTRUCT_ACCESS:
	StaticMesh();
	virtual ~StaticMesh();
	void init();
    void init(const StringRef& filePath, float scale);

private:
    Ref<StaticMeshComponent> m_component;
};

} // namespace ln
