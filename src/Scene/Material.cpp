/*
	[2015/5/11] SceneGraph に合わせた種類の Material はどう表現する？

		- コンポーネント
		値の get が文字列検索になる。毎フレームそれはちょっと・・・。
		ユーザーが値にアクセスするときも文字列でプロパティ名を指定する。タイプセーフじゃない。

		- 継承
			内部的にはこれで行こうか・・・。

			もしUnityみたいなエディタから入力するなら
			- オブジェクトにコンポーネントとして追加するマテリアルはあくまでデータモデルクラスとなる。
			- Forward用マテリアル、MMD用マテリアル、など。こいつらは基本的に値を map で持つ。
			- Node を作るとき、そのデータモデルから、現在の Scene 種類に合わせた Material クラスを作る。
			  この Material クラスには基本的にどんなデータモデルクラスを入力してもよいことにする。

			なお、Scene と Material が対応する都合上、Node は作成時に必ず何らかの SceneGraph に属することになる。


	[2016/5/9] 
		- これまでのレガシーなプロパティ (Diffuse/Ambient/Emissive/Speculer など)
		- 遅延シェーディング特有のプロパティ
		- Lumino 特有のゲーム向けプロパティ (BlendColor/Tone など)
		- MMD 特有のプロパティ (スフィアマップ など)
		
		
		
		シェーダの持ついくつかのプロパティ (主にセマンティクスとかついてるもの) は、
		SceneGraph から値をとってくる。
		
		基本的には「どんなプロパティが必要か？」はマテリアル側ではなく、シェーダ側から
		セマンティクスなどで指定することになる。
		
		MMD 用シーンを使っている場合などは、プロパティをC++側で覚えておく必要がある。
		シェーダプロパティに覚えておく、だと、シェーダを切り替えたときに Diffuse 値などが失われる。
		
		いままで使っていた SceneShader が Material の役割となる。
		
		BlendColor/Tone などは全てのマテリアルに影響する。
		
		
		
		
		セマンティクス
		
		SetVector("_Color", "DIFFUSE", color);
		とかにする場合、きっと map に変数名と値を覚えておく必要がある。
		でも、シェーダ側がセマンティクスで要求している場合、どんなセマンティクスにも対応できるように
		様々な値を map 似いれておかなければならない。(特に変換行列が多い。world, view, proj の転置、逆、など)
		これまでどおり、Apply するときあたりで全変数をなめて、変数がセマンティクスで要求している値を
		SceneGraph なり Node なり Material なりから取り出すのがメモリ効率的によい。
		
		
		特有のプロパティを持つマテリアルは派生クラスにする？
		全て1つのマテリアルクラスに持たせると無駄が多すぎる。
		また、メンバ変数に持たせず map 管理にすると設定側がセマンティクス名などのルールを守る必要がある。管理も少し複雑。
		まずは派生クラスとして作ってみる。
		
		
		SubMaterial は必ず1つ以上つくるべき？
			そもそも、
			- SubMaterial は 3Dモデルにくっついて Asset からロードされる共有マテリアル。基本的に Static。
			- MainMaterial は Node にくっつくルートのマテリアル。基本的に Dynamic。
			と整理するほうがわかりやすい。
			そうすると、Sub[0] = Main にしようとするのは間違いな気がする。
			
			SubMaterial 1個以上、MainMaterial 無し → 完全なstaticメッシュ。
			SubMaterial 0個、MainMaterial 有り → Asset 使わず、コード上で Sprite::Create() とかした場合。
			SubMaterial 0個、MainMaterial 無し、というパターンもありえる。その場合はデフォルトのマテリアルを使うようにする。
			
			
		Color は uint なのに opacity は float?
			WPF はこんなかんじ。
		
		
		
		
		UE4 - UMaterial
		https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/Materials/UMaterial/index.html
		
		Uity - Material
		http://docs.unity3d.com/ja/current/ScriptReference/Material.html
		
	
	RenderState は Node に持たせるべきか Material に持たせるべきか
		- Unity → ShaderのPassから
		- Ogre → Material
		
		Material に持たせる場合、「持っていない」という情報も必要。
		ほとんどの場合は Node に持たせておけば用は足りる。
*/

#include "Internal.h"
#include <Lumino/Graphics/Texture.h>
#include <Lumino/Graphics/Shader.h>
#include <Lumino/Scene/Material.h>
#include <Lumino/Scene/SceneGraph.h>
#include "MME/MmdMaterial.h"	// TODO

LN_NAMESPACE_BEGIN

//==============================================================================
//// Material
////==============================================================================
//
////------------------------------------------------------------------------------
//Material2::Material2(int materialTypeId)
//	: m_materialTypeId(materialTypeId)
//	, m_opacity(1.0f)
//	, m_colorScale(1, 1, 1, 1)
//	, m_blendColor(0, 0, 0, 0)
//	, m_tone()
//	, m_texture(nullptr)
//	, m_shader(nullptr)
//{
//}
//
////------------------------------------------------------------------------------
//Material2::~Material2()
//{
//	LN_SAFE_RELEASE(m_texture);
//	LN_SAFE_RELEASE(m_shader);
//}
//
////------------------------------------------------------------------------------
//void Material2::SetTexture(Texture* texture)
//{
//	LN_REFOBJ_SET(m_texture, texture);
//}
//
////------------------------------------------------------------------------------
//void Material2::SetShader(Shader* shader)
//{
//	LN_REFOBJ_SET(m_shader, shader);
//}

namespace detail
{

MaterialInstance::MaterialInstance(int materialTypeId)
	: m_materialTypeId(materialTypeId)
{

}

MaterialInstance::~MaterialInstance()
{

}

void MaterialInstance::Combine(Material3* owner, Material3* parent)
{
	if (m_owner == nullptr || owner != m_owner || m_owner->m_modifiedForMaterialInstance)
	{
		// set
		m_owner = owner;
		OnCombine(owner, parent);
		m_owner->m_modifiedForMaterialInstance = false;
	}
}

}

//==============================================================================
// MaterialList2
//==============================================================================

//------------------------------------------------------------------------------
MaterialList2::MaterialList2()
{
}

//------------------------------------------------------------------------------
MaterialList2::~MaterialList2()
{
}

//------------------------------------------------------------------------------
void MaterialList2::Initialize(int subMaterialCount, bool createMainMaterial)
{
	Resize(subMaterialCount);
	for (int i = 0; i < subMaterialCount; ++i)
	{
		auto m = RefPtr<Material3>::MakeRef();	// TODO
		SetAt(i, m);
	}

	if (createMainMaterial)
	{
		m_mainMaterial = RefPtr<Material3>::MakeRef();	// TODO
	}
}

//------------------------------------------------------------------------------
void MaterialList2::UpdateMaterialInstances(SceneGraph* sceneGraph)
{
	// m_mainMaterial は親として使える？
	Material3* parent = nullptr;
	if (m_mainMaterial != nullptr)
	{
		parent = m_mainMaterial;
	}

	// m_instanceList のサイズをそろえる
	int subCount = GetCount();
	if (m_instanceList.GetCount() != subCount)
	{
		int d = subCount - m_instanceList.GetCount();
		m_instanceList.Resize(subCount);
		if (d > 0)
		{
			for (int i = 0; i < d; ++i)
			{
				m_instanceList.Add(sceneGraph->CreateMaterialInstance());
			}
		}
	}

	// m_instanceList の内容を作っていく
	if (subCount > 0)
	{
		for (int i = 0; i < subCount; ++i)
		{
			m_instanceList[i]->Combine(GetAt(i), parent);
		}
	}
	else if (parent != nullptr)
	{
		// parent はあるけど SubMaterial が1つも無い。parent を使う。
		if (m_instanceList.GetCount() != 1) m_instanceList.Resize(1);
		m_instanceList[0]->Combine(parent, nullptr);
	}
	else
	{
		// parent も SubMaterial も無い。デフォルトのを使う。
		if (m_instanceList.GetCount() != 1) m_instanceList.Resize(1);
		LN_NOTIMPLEMENTED();
		//m_instanceList[0].Combine(parent, nullptr);
	}
}

LN_NAMESPACE_END
