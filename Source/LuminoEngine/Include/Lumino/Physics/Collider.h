﻿
#pragma once
#include "Common.h"
#include "BodyBase.h"

LN_NAMESPACE_BEGIN
class CollisionShape;

/**
	@brief	
*/
class Collider
	: public PhysicsObject
{
	LN_TR_REFLECTION_TYPEINFO_DECLARE();
public:

	/**
		@brief		RigidBody オブジェクトを作成します。
		@param[in]	shape	: 衝突判定形状
	*/
	static RefPtr<Collider> Create(CollisionShape* shape);


	/** 現在の姿勢を取得します。*/
	const Matrix& GetTransform() const;

	/** 衝突判定形状を追加します。*/
	void AddShape(CollisionShape* shape);

	/** この Collider が衝突判定のためのトリガーであるかを設定します。初期値は false です。*/
	void SetTrigger(bool enabled);

	/** この Collider が衝突判定のためのトリガーであるかを取得します。*/
	bool IsTrigger() const;


	/** OnTriggerEnter イベントの通知を受け取るコールバックを登録します。*/
	void ConnectOnTriggerEnter(std::function<void(PhysicsObject*)> handler);

	/** OnTriggerLeave イベントの通知を受け取るコールバックを登録します。*/
	void ConnectOnTriggerLeave(std::function<void(PhysicsObject*)> handler);

	/** OnTriggerStay イベントの通知を受け取るコールバックを登録します。*/
	void ConnectOnTriggerStay(std::function<void(PhysicsObject*)> handler);

protected:
	virtual void OnBeforeStepSimulation() override;
	virtual void OnAfterStepSimulation() override;
	virtual void OnRemovedFromWorld() override;

	/** 他の Collider または RigidBody が、この Collider との接触を開始したときに呼び出されます。*/
	virtual void OnTriggerEnter(PhysicsObject* otherObject);

	/** 他の Collider または RigidBody が、この Collider との接触を終了したときに呼び出されます。*/
	virtual void OnTriggerLeave(PhysicsObject* otherObject);

	/** 他の Collider または RigidBody が、この Collider との接触している間呼び出されます。*/
	virtual void OnTriggerStay(PhysicsObject* otherObject);

LN_CONSTRUCT_ACCESS:
	Collider();
	virtual ~Collider();
	void Initialize();

private:
	void CreateInternalObject();
	void DeleteInternalObject();

	class LocalGhostObject;

	LocalGhostObject*		m_btGhostObject;
	RefPtr<CollisionShape>	m_shape;
	Matrix					m_transform;
	bool					m_isTrigger;
	bool					m_initialUpdate;

	Event<void(PhysicsObject*)>	onTriggerEnter;
	Event<void(PhysicsObject*)>	onTriggerLeave;
	Event<void(PhysicsObject*)>	onTriggerStay;
};

LN_NAMESPACE_END
