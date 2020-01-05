﻿
#pragma once
#include "Common.hpp"
#include "../Rendering/RenderView.hpp"
#include "TransformControls.hpp"

namespace ln {
class Material;
class World;
class Camera;
class RenderingContext;
class StaticMeshModel;
namespace detail {
class SceneRenderingPipeline;
}

/**
 *
 * ClearMode のデフォルトは ColorAndDepth です。
 */
class WorldRenderView
	: public RenderView
{
public:
    void setTargetWorld(World* world);
	void setCamera(Camera* camera);

	void setDebugGridEnabled(bool value) { m_visibleGridPlane = value; }
	bool debugGridEnabled() const { return m_visibleGridPlane; }

    void setPhysicsDebugDrawEnabled(bool value) { m_physicsDebugDrawEnabled = value; }
	bool physicsDebugDrawEnabled() const { return m_physicsDebugDrawEnabled; }

    void setGizmoEnabled(bool value) { m_gizmoEnabled = value; }
    bool gizmoEnabled() const { return m_gizmoEnabled; }

    const Ref<TransformControls>& transformControls() const{ return m_transformControls; }

    // TODO: internal
    virtual void render(GraphicsContext* graphicsContext, RenderTargetTexture* renderTarget) override;

protected:
    virtual void onRoutedEvent(UIEventArgs* e) override;

LN_CONSTRUCT_ACCESS:
    WorldRenderView();
	virtual ~WorldRenderView();
	void init();

private:
    void createGridPlane();
    void renderGridPlane(RenderingContext* renderingContext, RenderView* renderView);
    void adjustGridPlane(const ViewFrustum& viewFrustum, RenderView* renderView);

    Ref<detail::SceneRenderingPipeline> m_sceneRenderingPipeline;
    Ref<detail::DrawElementListCollector> m_drawElementListCollector;
    Ref<World> m_targetWorld;
	Ref<Camera> m_camera;
    Ref<RenderViewPoint> m_viewPoint;
    Ref<Material> m_clearMaterial;
	Ref<StaticMeshModel> m_skyProjectionPlane;

    Ref<StaticMeshModel> m_gridPlane;
    bool m_visibleGridPlane;
    bool m_physicsDebugDrawEnabled;
    bool m_gizmoEnabled;

    Ref<TransformControls> m_transformControls; // TODO: gizmo でまとめる？
};

} // namespace ln
