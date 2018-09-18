﻿#pragma once
#include "../Graphics/ColorStructs.hpp"
#include "../Graphics/RenderState.hpp"
#include "../Shader/Shader.hpp"
#include "../Shader/ShaderInterfaceFramework.hpp"

namespace ln {
namespace detail {

enum class MaterialType : uint8_t
{
	PBR,
	Phong,
};

} // namespace detail

/**
	@brief
*/
class AbstractMaterial
	: public Object
{
	//LN_OBJECT;
//public:
//	static const String DiffuseParameter;
//	static const String AmbientParameter;
//	static const String SpecularParameter;
//	static const String EmissiveParameter;
//	static const String PowerParameter;
//	static const String MaterialTextureParameter;

public:
	void setMainTexture(Texture* value);
	Texture* mainTexture() const;

	void setShader(Shader* shader);
	Shader* shader() const;

	void setInt(const StringRef& name, int value);
	void setFloat(const StringRef& name, float value);
	void setVector(const StringRef& name, const Vector4& value);
	void setMatrix(const StringRef& name, const Matrix& value);
	void setTexture(const StringRef& name, Texture* value);
	void setColor(const StringRef& name, const Color& value);

	//--------------------------------------------------------------------------
	/** @name RenderState */
	/** @{ */

private:
	Optional<BlendMode>		blendMode;
	Optional<CullingMode>	cullingMode;
	Optional<bool>			depthTestEnabled;
	Optional<bool>			depthWriteEnabled;

public:
	ShadingModel			shadingModel = ShadingModel::Default;

	void setBlendMode(Optional<BlendMode> mode);
	Optional<BlendMode> getBlendMode() const { return blendMode; }

	void setCullingMode(Optional<CullingMode> mode);
	Optional<CullingMode> getCullingMode() const { return cullingMode; }

	void setDepthTestEnabled(Optional<bool> enabled);
	Optional<bool> isDepthTestEnabled() const { return depthTestEnabled; }

	void setDepthWriteEnabled(Optional<bool> enabled);
	Optional<bool> isDepthWriteEnabled() const { return depthWriteEnabled; }


protected:
	AbstractMaterial(detail::MaterialType type);
	virtual ~AbstractMaterial();
	void initialize();
	virtual void translateToPBRMaterialData(detail::PbrMaterialData* outData) = 0;
	virtual void translateToPhongMaterialData(detail::PhongMaterialData* outData) = 0;

//LN_INTERNAL_ACCESS:
//	void reset();
//
//	void setBuiltinIntParameter(const StringRef& name, int value);
//	void setBuiltinFloatParameter(const StringRef& name, float value);
//	void setBuiltinVectorParameter(const StringRef& name, const Vector4& value);
//	void setBuiltinMatrixParameter(const StringRef& name, const Matrix& value);
//	void setBuiltinTextureParameter(const StringRef& name, Texture* value);
//	void setBuiltinColorParameter(const StringRef& name, const Color& value);
//	void setBuiltinColorParameter(const StringRef& name, float r, float g, float b, float a);

LN_PROTECTED_INTERNAL_ACCESS:

LN_INTERNAL_ACCESS:
	//using ShaderValuePtr = std::shared_ptr<ShaderValue>;

	//struct ValuePair
	//{
	//	ShaderVariable*	variable;
	//	ShaderValuePtr	value;
	//};

	//const List<ValuePair>& GetLinkedVariableList() { return m_linkedVariableList; }

	//Ref<CommonMaterial> copyShared() const;

	//void ResolveCombinedMaterial();
	//detail::CombinedMaterial* getCombinedMaterial() const;

public:	// TODO:

	// TODO: 他の Builtin パラーメータを追い出したのでこれだけになってしまった。普通のメンバ変数でいいのでは？
	//void setMaterialTexture(Texture* v);
	//Texture* getMaterialTexture(Texture* defaultValue) const;

	//void setOpacity(float v);
	//float getOpacity() const;

	//void setColorScale(const Color& v);
	//Color getColorScale() const;

	//void setBlendColor(const Color& v);
	//Color getBlendColor() const;

	//void setTone(const ToneF& v);
	//ToneF getTone() const;

	//Matrix GetUVTransform() const { /*auto* v = FindShaderValueConst(_T("UVTransform")); return (v) ? v->getMatrix() : */ return Matrix::Identity; }

	//Color getColor(const StringRef& name, const Color& defaultValue) const { auto* v = FindShaderValueConst(name); return (v) ? Color(v->getVector()) : defaultValue; }
	//float GetFloat(const StringRef& name, float defaultValue) const { auto* v = FindShaderValueConst(name); return (v) ? v->GetFloat() : defaultValue; }
	//Texture* getTexture(const StringRef& name, Texture* defaultValue) const { auto* v = FindShaderValueConst(name); return (v) ? v->getManagedTexture() : defaultValue; }
	//int GetInt(const StringRef& name, int defaultValue) const { auto* v = FindShaderValueConst(name); return (v) ? v->GetInt() : defaultValue; }


	//static const Color DefaultDiffuse;	// (1.0f, 1.0f, 1.0f, 1.0f)
	//static const Color DefaultAmbient;	// (0.0f, 0.0f, 0.0f, 0.0f)
	//static const Color DefaultSpecular;	// (0.5f, 0.5f, 0.5f, 0.5f)
	//static const Color DefaultEmissive;	// (0.0f, 0.0f, 0.0f, 0.0f)
	//static const float DefaultPower;	// (50.0f)

private:
	//void LinkVariables();
	//ShaderValue* FindShaderValue(const StringRef& name);
	//ShaderValue* FindShaderValueConst(const StringRef& name) const;

	//static void onRenderStateChanged(Object* obj);

	detail::MaterialType m_type;
	Ref<Shader> m_shader;
	Ref<Texture> m_mainTexture;
	std::unordered_map<uint32_t, Variant> m_valueMap;

	//std::unordered_map<uint32_t, ShaderValue>	m_builtinValueMap;


LN_INTERNAL_ACCESS:
	//int									m_revisionCount;
	//uint32_t							m_hashCode;


	//const std::unordered_map<uint32_t, ShaderValue>& getUserValueMap() const { return m_userValueMap; }
	//ShaderValue* findAndCreateUserShaderValue(uint32_t hashKey);
	//const ShaderValue* findUserShaderValueConst(uint32_t hashKey) const;
	////const Color& getColor(uint32_t hashKey, const Color& defaultValue) const { auto* v = findUserShaderValueConst(hashKey); return (v) ? static_cast<const Color&>(v->getVector()) : defaultValue; }
	////float GetFloat(uint32_t hashKey, float defaultValue) const { auto* v = findUserShaderValueConst(hashKey); return (v) ? v->GetFloat() : defaultValue; }
	////Texture* getTexture(uint32_t hashKey, Texture* defaultValue) const { auto* v = findUserShaderValueConst(hashKey); return (v) ? v->getManagedTexture() : defaultValue; }

	//const Color& getBuiltinColor(uint32_t hashKey, const Color& defaultValue) const { auto itr = m_builtinValueMap.find(hashKey); return (itr != m_builtinValueMap.end()) ? static_cast<const Color&>(itr->second.getVector()) : defaultValue; }
	//float getBuiltinFloat(uint32_t hashKey, float defaultValue) const { auto itr = m_builtinValueMap.find(hashKey); return (itr != m_builtinValueMap.end()) ? itr->second.getFloat() : defaultValue; }
	//Texture* getBuiltinTexture(uint32_t hashKey, Texture* defaultValue) const { auto itr = m_builtinValueMap.find(hashKey); return (itr != m_builtinValueMap.end()) ? itr->second.getManagedTexture() : defaultValue; }

	//void applyUserShaderValeues(Shader* targetShader);

	//uint32_t getHashCode();
};

/**
 * 標準的な物理ベースレンダリングのマテリアルです。
 */
class Material
	: public AbstractMaterial
{
	//LN_OBJECT;
public:
	static Ref<Material> create();

public:
	void setColor(const Color& value);
	void setRoughness(float value);
	void setMetallic(float value);
	void setSpecular(float value);

	// TODO: 自己発光。必要かな？
	//void setEmissive(const Color& value);

protected:
	virtual void translateToPBRMaterialData(detail::PbrMaterialData* outData) override;
	virtual void translateToPhongMaterialData(detail::PhongMaterialData* outData) override;

LN_CONSTRUCT_ACCESS:
	Material();
	virtual ~Material();
	void initialize();

private:
	detail::PbrMaterialData m_data;
};

} // namespace ln
