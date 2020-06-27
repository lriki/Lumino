﻿
#include "Internal.hpp"
#include <LuminoEngine/Asset/Assets.hpp>
#include <LuminoEngine/Animation/AnimationCurve.hpp>
#include <LuminoEngine/Graphics/SamplerState.hpp>
#include <LuminoEngine/Shader/Shader.hpp>
#include <LuminoEngine/Mesh/Mesh.hpp>
#include <LuminoEngine/Rendering/Material.hpp>
#include <LuminoEngine/Rendering/RenderingContext.hpp>
#include <LuminoEngine/Rendering/RenderView.hpp>
#include "../Rendering/RenderingManager.hpp"
#include "InternalSkyBox.hpp"

namespace ln {
namespace detail {

//==============================================================================
// InternalSkyBox

InternalSkyBox::InternalSkyBox()
{
}

void InternalSkyBox::init()
{
	Object::init();
	auto shader = EngineDomain::renderingManager()->builtinShader(BuiltinShader::SkyLowAltitudeOptimized);

	m_material = makeObject<Material>();
	m_material->setShader(shader);
    m_material->shadingModel = ShadingModel::Unlit;

    m_lightDirection = Vector3::UnitY;
}

void InternalSkyBox::setLightDirection(const Vector3& value)
{
    m_lightDirection = value;
}

void InternalSkyBox::render(RenderingContext* context, const RenderViewPoint* viewPoint)
{
	const float sunDistance = 400000;

    
    //m_material->setVector(u"_RayleighColorScale", Vector4(0.1, 0.2, 0.6, 0.0));
    m_material->setVector(u"_RayleighColorScale", Color(0.5, 0.2, 0.5).toVector4());
	//m_material->setFloat(u"turbidity", 10.0f);
    m_material->setFloat(u"turbidity", 1.0f);
    //m_material->setFloat(u"rayleigh", 2.0f);
	m_material->setFloat(u"rayleigh", 0.5f);
	m_material->setFloat(u"mieCoefficient", 0.005f);
	m_material->setFloat(u"mieDirectionalG", 0.8f);
	m_material->setFloat(u"luminance", 1.0f);
	m_material->setVector(u"up", Vector4(0, 1, 0, 0));
	m_material->setVector(u"sunPosition", Vector4(m_lightDirection * sunDistance, 0));

	context->pushState();
	context->setBlendMode(BlendMode::Normal);
	context->setAdditionalElementFlags(detail::RenderDrawElementTypeFlags::BackgroundSky);
    //context->setRenderPhase(RenderPhaseClass::BackgroundSky);
    //context->setDepthTestEnabled(false);
	context->setDepthWriteEnabled(false);
    context->setBaseTransfrom(Matrix::makeTranslation(viewPoint->viewPosition));
	context->setMaterial(m_material);
	context->setCullingMode(CullMode::Front);
    context->drawBox(Box(Vector3::Zero, 100));// , Color::White, Matrix::makeTranslation(viewPoint->viewPosition));
	context->popState();
}

//==============================================================================
// InternalSkyDome

#include "SkyDomeMesh.inl"

InternalSkyDome::InternalSkyDome()
{
}

bool InternalSkyDome::init()
{
#if 0 // Dump
    m_model = StaticMeshModel::load(u"Sphere.glb");
    auto mesh = m_model->meshContainers()[0]->mesh();
    {
        StreamWriter writer(u"vertices.txt");
        for (int i = 0; i < mesh->vertexCount(); i++) {
            auto& v = mesh->vertex(i);
            //printf("{{%f, %f, %f}, {%f, %f, %f}, {%f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f}}",
            //writer.writeLineFormat(u"{{{{{0}, {1}, {2}}}, {{{3}, {4}, {5}}}, {{{6}, {7}}}, {{{8}, {9}, {10}, {11}}}, {{{12}, {13}, {14}, {15}}}}}",
            //    v.position.x, v.position.y, v.position.z,
            //    v.normal.x, v.normal.y, v.normal.z,
            //    v.uv.x, v.uv.y,
            //    v.color.r, v.color.g, v.color.b, v.color.a,
            //    v.tangent.x, v.tangent.y, v.tangent.z, v.tangent.w);
            writer.writeLineFormat(u"{{{{{0}, {1}, {2}}}, {{{3}, {4}, {5}}}, {{{6}, {7}}}}}",
                v.position.x, v.position.y, v.position.z,
                v.normal.x, v.normal.y, v.normal.z,
                v.uv.x, v.uv.y);
        }

        StreamWriter writer2(u"indices.txt");
        for (int i = 0; i < mesh->indexCount() / 3; i++) {
            int index = i * 3;
            writer2.writeLineFormat(
                u"{0}, {1}, {2},",
                mesh->index(index + 0),
                mesh->index(index + 1),
                mesh->index(index + 2));
        }
    }
#endif
    {
        const int vertexCount = LN_ARRAY_SIZE_OF(s_skyDomeVertices);
        const int indexCount = LN_ARRAY_SIZE_OF(s_skyDomeIndices);
        auto mesh = makeObject<Mesh>(
            vertexCount, indexCount,
            IndexBufferFormat::UInt16, GraphicsResourceUsage::Static);

        auto* vertices = static_cast<Vertex*>(mesh->acquireMappedVertexBuffer(InterleavedVertexGroup::Main));
        for (int i = 0; i < vertexCount; i++) {
            vertices[i].position = s_skyDomeVertices[i].pos / 10;
            vertices[i].normal = s_skyDomeVertices[i].normal;
            vertices[i].uv = s_skyDomeVertices[i].uv;
            vertices[i].color = Color::White;
            vertices[i].tangent = Vector4(1, 0, 0, 1);
        }

        auto* indices = static_cast<uint16_t*>(mesh->acquireMappedIndexBuffer());
        memcpy(indices, s_skyDomeIndices, sizeof(uint16_t) * indexCount);

        mesh->addSection(0, indexCount / 3, 0, PrimitiveTopology::TriangleList);

        m_model = makeObject<StaticMeshModel>();
        auto node = m_model->addMeshContainerNode(mesh);
        m_model->addRootNode(0);

        m_material = makeObject<Material>();
        m_model->addMaterial(m_material);
        m_model->updateNodeTransforms();
    }




    auto samperState = ln::SamplerState::create(ln::TextureFilterMode::Linear, ln::TextureAddressMode::Repeat, true);

    auto _mainCloudsTexture = ln::Texture2D::load(Path(Assets::engineAssetsDirectory(), u"SkydomeCloudA.png"));
    _mainCloudsTexture->setSamplerState(samperState);
    auto _secondCloudsTexture = ln::Texture2D::load(Path(Assets::engineAssetsDirectory(), u"SkydomeCloudB.png"));
    _secondCloudsTexture->setSamplerState(samperState);
    auto _detailCloudTexture = ln::Texture2D::load(Path(Assets::engineAssetsDirectory(), u"SkydomeCloudC.png"));
    _detailCloudTexture->setSamplerState(samperState);
    auto _secondCloudPowerMap = ln::Texture2D::load(Path(Assets::engineAssetsDirectory(), u"SkydomeCloudR.png"));
    _secondCloudPowerMap->setSamplerState(samperState);

    m_material->setShader(ln::Shader::create(u"C:/Proj/LN/Lumino/src/LuminoEngine/src/Scene/Resource/SkyDome.fx"));
    m_material->setTexture(u"_thirdCloudTexture", _mainCloudsTexture);
    m_material->setTexture(u"_detailCloudTexture", _detailCloudTexture);
    m_material->setTexture(u"_secondCloudPowerMap", _secondCloudPowerMap);
    m_material->setTexture(u"_mainCloudsTexture", _mainCloudsTexture);
    m_material->setTexture(u"_secondCloudsTexture", _secondCloudsTexture);
    m_material->setMainTexture(_secondCloudsTexture);


    {
        auto r = ln::KeyFrameAnimationCurve::create();
        r->addKeyFrame(0.0, 0.49134);
        r->addKeyFrame(5.0, 0.49134);
        r->addKeyFrame(6.5, 0.819287);
        r->addKeyFrame(9.086636, 0.507721);
        r->addKeyFrame(12.054127, 0.466585);
        r->addKeyFrame(16.886023, 0.50004);
        r->addKeyFrame(17.854126, 0.573377);
        r->addKeyFrame(18.687864, 0.454085);
        r->addKeyFrame(19.871872, 0.49134);
        r->addKeyFrame(24.0, 0.49134);
        m_backGroundSkyDomeColorR = r;

        auto g = ln::KeyFrameAnimationCurve::create();
        g->addKeyFrame(0.0, 0.369745);
        g->addKeyFrame(5.0, 0.369745);
        g->addKeyFrame(9.032509, 0.683189);
        g->addKeyFrame(16.831896, 0.720503);
        g->addKeyFrame(17.799999, 0.316957);
        g->addKeyFrame(18.633738, 0.185302);
        g->addKeyFrame(19.817745, 0.413315);
        g->addKeyFrame(24.0, 0.413315);
        m_backGroundSkyDomeColorG = g;

        auto b = ln::KeyFrameAnimationCurve::create();
        b->addKeyFrame(0.0, 0.441597);
        b->addKeyFrame(5.0, 0.441597);
        b->addKeyFrame(6.5, 0.839636);
        b->addKeyFrame(9.032509, 0.972142);
        b->addKeyFrame(12.0, 1.0);
        b->addKeyFrame(17.022861, 0.94726);
        b->addKeyFrame(18.633738, 0.75);
        b->addKeyFrame(19.817745, 0.441597);
        b->addKeyFrame(24.0, 0.441597);
        m_backGroundSkyDomeColorB = b;
    }

    {
        // とりあえず線形補間で近似
        auto r = ln::KeyFrameAnimationCurve::create();
        r->addKeyFrame(4.5, 2.0);
        r->addKeyFrame(5.53, 8.0);
        r->addKeyFrame(8.5, 2.82);
        r->addKeyFrame(12.0, 1.5);
        r->addKeyFrame(16.0, 3.0);
        r->addKeyFrame(17.0, 8.0);
        r->addKeyFrame(18.4, 5.6);
        r->addKeyFrame(19.2, 2.0);
        m_backGroundHorizonColorR = r;

        // とりあえず線形補間で近似
        auto g = ln::KeyFrameAnimationCurve::create();
        g->addKeyFrame(4.5, 1.9);
        g->addKeyFrame(5.021666, 0.74472);
        g->addKeyFrame(5.53, 3.0);
        g->addKeyFrame(8.5, 2.5);
        g->addKeyFrame(12.0, 2.7);
        g->addKeyFrame(16.0, 2.23);
        g->addKeyFrame(17.53, 1.345);
        g->addKeyFrame(18.34, 2.7);
        g->addKeyFrame(19.2, 1.9);
        m_backGroundHorizonColorG = g;

        // とりあえず線形補間で近似
        auto b = ln::KeyFrameAnimationCurve::create();
        b->addKeyFrame(4.6, 2.5);
        b->addKeyFrame(5.0, 0.828287);
        b->addKeyFrame(5.53, 0.549693);
        b->addKeyFrame(8.5, 3.0);
        b->addKeyFrame(12.0, 3.0);
        b->addKeyFrame(16.0, 2.5);
        b->addKeyFrame(17.53, 1.0);
        b->addKeyFrame(18.5, 0.76);
        b->addKeyFrame(19.2, 2.5);
        m_backGroundHorizonColorB = b;
    }

    {
        auto r = ln::KeyFrameAnimationCurve::create();
        r->addKeyFrame(5.0, 0.137255);
        r->addKeyFrame(5.592618, 0.75);
        r->addKeyFrame(6.225262, 0.95);
        r->addKeyFrame(8.996424, 0.674617);
        r->addKeyFrame(12.0, 0.633211);
        r->addKeyFrame(15.900076, 1.0);
        r->addKeyFrame(17.87228, 1.0);
        r->addKeyFrame(18.647556, 0.743711);
        r->addKeyFrame(19.5, 0.137255);
        m_allOverlayColorR = r;

        auto g = ln::KeyFrameAnimationCurve::create();
        g->addKeyFrame(5.0, 0.137255);
        g->addKeyFrame(5.592618, 0.18825);
        g->addKeyFrame(6.225262, 0.641275);
        g->addKeyFrame(8.996424, 0.762539);
        g->addKeyFrame(12.0, 0.808692);
        g->addKeyFrame(16.245581, 0.901319);
        g->addKeyFrame(17.87228, 0.47);
        g->addKeyFrame(18.647556, 0.108993);
        g->addKeyFrame(19.5, 0.137255);
        m_allOverlayColorG = g;

        auto b = ln::KeyFrameAnimationCurve::create();
        b->addKeyFrame(5.0, 0.156863);
        b->addKeyFrame(5.592618, 0.18825);
        b->addKeyFrame(6.225262, 0.35);
        b->addKeyFrame(9.05025, 0.843588);
        b->addKeyFrame(12.0, 1.0);
        b->addKeyFrame(16.245581, 1.0);
        b->addKeyFrame(17.87228, 0.3);
        b->addKeyFrame(18.647556, 0.10006);
        b->addKeyFrame(19.5, 0.156863);
        m_allOverlayColorB = b;

        // とりあえず線形補間で近似
        auto a = ln::KeyFrameAnimationCurve::create();
        a->addKeyFrame(6.5, 0.0);
        a->addKeyFrame(12.0, 0.65);
        a->addKeyFrame(18.0, 0.0);
        m_allOverlayColorA = a;
    }

    {
        auto r = ln::KeyFrameAnimationCurve::create();
        r->addKeyFrame(5.0, 0.12499);
        r->addKeyFrame(6.025473, 0.736755);
        r->addKeyFrame(8.295567, 0.936754);
        r->addKeyFrame(11.02476, 0.564322);
        r->addKeyFrame(14.645027, 0.565347);
        r->addKeyFrame(17.025473, 0.552442);
        r->addKeyFrame(17.768421, 0.736755);
        r->addKeyFrame(19.707409, 0.12499);
        m_baseCloudColorAndIntensityR = r;

        auto g = ln::KeyFrameAnimationCurve::create();
        g->addKeyFrame(5.0, 0.105382);
        g->addKeyFrame(6.025473, 0.133787);
        g->addKeyFrame(8.295567, 0.162505);
        g->addKeyFrame(11.02476, 0.70059);
        g->addKeyFrame(14.645027, 0.700712);
        g->addKeyFrame(17.025473, 0.657623);
        g->addKeyFrame(17.768421, 0.136755);
        g->addKeyFrame(19.707409, 0.105382);
        m_baseCloudColorAndIntensityG = g;

        auto b = ln::KeyFrameAnimationCurve::create();
        b->addKeyFrame(5.0, 0.285774);
        b->addKeyFrame(6.025473, 0.430111);
        b->addKeyFrame(8.295567, 0.256381);
        b->addKeyFrame(11.02476, 0.986755);
        b->addKeyFrame(14.645027, 0.986755);
        b->addKeyFrame(17.025473, 0.936754);
        b->addKeyFrame(17.768421, 0.578202);
        b->addKeyFrame(19.707409, 0.285774);
        m_baseCloudColorAndIntensityB = b;

        // とりあえず線形補間で近似
        auto a = ln::KeyFrameAnimationCurve::create();
        a->addKeyFrame(0.5, 0.6);
        a->addKeyFrame(12.0, 0.22);
        a->addKeyFrame(20.0, 0.6);
        m_baseCloudColorAndIntensityA = a;
    }

    m_timeOfDay = 12.0;

    return true;
}

void InternalSkyDome::render(RenderingContext* context, const RenderViewPoint* viewPoint)
{
    m_timeOfDay += 0.016;
    m_timeOfDay = std::fmod(m_timeOfDay, 24.0f);

    {
        float timeOfDay = m_timeOfDay;

        auto BackGroundSkyDomeColor = ln::Color(
            m_backGroundSkyDomeColorR->evaluate(timeOfDay),
            m_backGroundSkyDomeColorG->evaluate(timeOfDay),
            m_backGroundSkyDomeColorB->evaluate(timeOfDay),
            1.0f);
        auto BackGroundHorizonColor = ln::Color(
            m_backGroundHorizonColorR->evaluate(timeOfDay),
            m_backGroundHorizonColorG->evaluate(timeOfDay),
            m_backGroundHorizonColorB->evaluate(timeOfDay),
            1.0f);
        auto AllOverlayColor = ln::Color(
            m_allOverlayColorR->evaluate(timeOfDay),
            m_allOverlayColorG->evaluate(timeOfDay),
            m_allOverlayColorB->evaluate(timeOfDay),
            m_allOverlayColorA->evaluate(timeOfDay));
        auto BaseCloudColorAndIntensity = ln::Color(
            m_baseCloudColorAndIntensityR->evaluate(timeOfDay),
            m_baseCloudColorAndIntensityG->evaluate(timeOfDay),
            m_baseCloudColorAndIntensityB->evaluate(timeOfDay),
            m_baseCloudColorAndIntensityA->evaluate(timeOfDay));

        //BackGroundSkyDomeColor = Color::Gray;
        //BackGroundHorizonColor = Color::Gray;
        //AllOverlayColor = Color::Gray;
        //BaseCloudColorAndIntensity = Color::Gray;

        m_material->setColor(u"_Curve_BackGroundSkyDomeColor", BackGroundSkyDomeColor);
        m_material->setColor(u"_Curve_BackGroundHorizonColor", BackGroundHorizonColor);
        m_material->setColor(u"_Curve_AllOverlayColor", AllOverlayColor);
        m_material->setColor(u"_Curve_BaseCloudColorAndIntensity", BaseCloudColorAndIntensity);

        //m_material->setFloat(u"_Main_Clouds_Falloff_Intensity", 3.0);
        //m_material->setFloat(u"_Second_Clouds_Falloff_Intensity", 4.0);
        //m_material->setFloat(u"_AllCloudsFalloffIntensity", 1.15);
        //m_material->setFloat(u"_AllCloudsIntensity", 0.85);
        m_material->setFloat(u"_Main_Clouds_Falloff_Intensity", 0.8);
        m_material->setFloat(u"_Second_Clouds_Falloff_Intensity", 0.8);
        m_material->setFloat(u"_ThirdCloudsIntensity", 0.25);
        m_material->setFloat(u"_AllCloudsFalloffIntensity", 0.95);
        m_material->setFloat(u"_AllCloudsIntensity", 1.1);
        

        /*
            Super Heavy:
                Main Clouds Falloff Intensity: 0.8
                Second Clouds Falloff Intensity: 0.8
                Third Clouds  Intensity: 0.25
                All Clouds Falloff Intensity: 0.95
                All Clouds Intensity: 1.1

            Middle:
                Main Clouds Falloff Intensity: 3.0
                Second Clouds Falloff Intensity: 4.0
                Third Clouds  Intensity: 0.0
                All Clouds Falloff Intensity: 1.15
                All Clouds Intensity: 0.85

            Slight:
                Main Clouds Falloff Intensity: 4.0
                Second Clouds Falloff Intensity: 4.0
                Third Clouds  Intensity: 0.0
                All Clouds Falloff Intensity: 1.4
                All Clouds Intensity: 0.6

            Super Slight:
              Main Clouds Falloff Intensity: 5.0
              Second Clouds Falloff Intensity: 5.0
              All Clouds Falloff Intensity: 2.0
              Third Clouds Intensity: 0.0
              All Clouds Intensity: 0.35

            Clear:
              All Clouds Intensity: 0.0
        */
    }

    Matrix transform = //Matrix::makeScaling(100);
        transform.makeTranslation(viewPoint->viewPosition);

    for (const auto& node : m_model->meshNodes()) {
        if (node->meshContainerIndex() >= 0) {
            //context->setTransfrom(m_model->nodeGlobalTransform(node->index()));
            context->setTransfrom(transform);
            const auto& meshContainer = m_model->meshContainers()[node->meshContainerIndex()];

            Mesh* mesh = meshContainer->mesh();
            if (mesh) {
                for (int iSection = 0; iSection < mesh->sections().size(); iSection++) {
                    context->setMaterial(m_model->materials()[mesh->sections()[iSection].materialIndex]);
                    context->drawMesh(mesh, iSection);
                }
            }
        }
    }
}


} // namespace detail
} // namespace ln

