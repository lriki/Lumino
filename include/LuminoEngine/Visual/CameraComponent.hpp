﻿
#pragma once
#include "VisualComponent.hpp"

namespace ln {
    
/** カメラの投影方法 */
enum class ProjectionMode
{
	Perspective,	/**< 透視投影 */
	Orthographic,	/**< 平行投影 */
};

/**
	@brief
*/
class CameraComponent
	: public VisualComponent
{
public:

	// TODO: tansform に任せたので必要ない
	void setLookAt(const Vector3& position) { m_lookAt = position; }
	const Vector3& getLookAt() const { return m_lookAt; }

	void setUpDirection(const Vector3& up) { m_upDirection = up; }
	const Vector3& getUpDirection() const { return m_upDirection; }

	

	/// Y 方向視野角の設定
	void setFovY(float fov_y) { m_fovY = fov_y; }

	/// Y 方向視野角の取得
	float getFovY() const { return m_fovY; }

	/// 最も近いビュープレーン位置の設定 (0.0 以下にしないこと)
	void setNearClip(float nearClip) { m_nearClip = nearClip; }

	/// 最も近いビュープレーン位置の取得
	float getNearClip() const { return m_nearClip; }

	/// 最も遠いビュープレーン位置の設定
	void setFarClip(float farClip) { m_farClip = farClip; }

	/// 最も遠いビュープレーン位置の取得
	float getFarClip() const { return m_farClip; }

	const ViewFrustum& getViewFrustum() const { return m_viewFrustum; }


	//void setZSortDistanceBase(ZSortDistanceBase type) { m_zSortDistanceBase = type; }
	//ZSortDistanceBase getZSortDistanceBase() const { return m_zSortDistanceBase; }

	//void setCameraBehavior(CameraBehavior* behavior);
	//CameraBehavior* getCameraBehavior() const { return m_cameraBehavior; }

	// 3D→2D
	Vector3 worldToViewportPoint(const Vector3& position) const;
	// 2D→3D
	Vector3 viewportToWorldPoint(const Vector3& position) const;

	void setProjectionMode(ProjectionMode value) { m_projectionMode = value; }

	
	void setOrthographicSize(float value) { m_orthographicSize = value; }

	void setAspect(float value) { m_aspect = value; }


public:	// internal

	/// 各行列を更新する (SceneNode::updateFrameHierarchy() の後で呼び出すこと)
	void updateMatrices(const Size& viewSize);

	// 向きの取得 (シェーダ設定用。updateMatrices() の後で呼び出すこと)
	const Vector4& getDirectionInternal() const { return m_direction; }

	// 行列の取得 (シェーダ設定用。updateMatrices() の後で呼び出すこと)
	const Matrix& getViewMatrix() const { return m_viewMatrix; }
	const Matrix& getProjectionMatrix() const { return m_projMatrix; }
	const Matrix& getViewProjectionMatrix() const { return m_viewProjMatrix; }

protected:

	// Component interface
	//virtual void onUpdate(float deltaSceonds) override;
	//virtual void onUIEvent(UIEventArgs* e) override;

LN_INTERNAL_ACCESS:
	CameraComponent();
	virtual ~CameraComponent();
    void initialize();
	//void initialize(CameraWorld proj);
	//void setCameraDirection(CameraDirection mode) { m_directionMode = mode; }
	//CameraWorld getCameraWorld() const { return m_cameraWorld; }

	//void setReflectionPlane(const Plane& plane) { m_reflectionPlane = plane; }

	//WorldRenderView*	m_ownerLayer;

	//CameraWorld	m_cameraWorld;

private:
	//CameraDirection		m_directionMode;
	ProjectionMode		m_projectionMode;
	Vector3				m_lookAt;
	Vector3				m_upDirection;
	float				m_fovY;
	float				m_aspect;
	float				m_nearClip;
	float				m_farClip;
	float				m_orthographicSize;	// 縦方向のサイズ。横はアスペクト比から求める
	//ZSortDistanceBase	m_zSortDistanceBase;
	//CameraBehavior*		m_cameraBehavior;

	Matrix				m_viewMatrix;		///< ビュー行列
	Matrix				m_projMatrix;		///< プロジェクション行列
	Matrix				m_viewProjMatrix;	///< ビュー行列とプロジェクション行列の積
	Vector4				m_direction;		///< 向き
	ViewFrustum			m_viewFrustum;		// 視錐台カリング用 (3D,2D共用)

	Matrix				m_viewProjMatrixI;

	//Plane				m_reflectionPlane;
};

} // namespace ln