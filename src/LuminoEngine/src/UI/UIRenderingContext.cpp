﻿
#include "Internal.hpp"
#include <LuminoEngine/Graphics/GraphicsContext.hpp>
#include <LuminoEngine/Rendering/Material.hpp>
#include <LuminoEngine/UI/UIRenderingContext.hpp>
#include <LuminoEngine/UI/UIElement.hpp>
#include "../Rendering/RenderStage.hpp"
#include "../Rendering/DrawElementListBuilder.hpp"
#include "../Rendering/FrameRectRenderFeature.hpp"
#include "../Rendering/ShapesRenderFeature.hpp"

namespace ln {

//==============================================================================
// UIRenderingContext

UIRenderingContext::UIRenderingContext()
	: m_elementList(makeRef<detail::DrawElementList>(detail::EngineDomain::renderingManager()))
{
	setDrawElementList(m_elementList);
}

void UIRenderingContext::drawSolidRectangle(const Rect& rect, const Color& color)
{
    auto* element = m_builder->addNewDrawElement<detail::DrawShapesElement>(
        m_manager->shapesRenderFeature(),
        m_builder->shapesRenderFeatureStageParameters());

    element->commandList.addDrawBoxBackground(m_builder->targetList()->dataAllocator(), element->combinedWorldMatrix(), rect, CornerRadius(), color);
}

void UIRenderingContext::drawImageBox(const Rect& rect, BrushImageDrawMode mode, const Rect& textureSourceRect, const Thickness& borderThickness, const Color& color)
{
    if (mode == BrushImageDrawMode::Image) {
        drawSolidRectangle(rect, color);
    }
    else {
        auto* element = m_builder->addNewDrawElement<detail::DrawFrameRectElement>(
            m_manager->frameRectRenderFeature(),
            m_builder->frameRectRenderFeatureStageParameters());

        element->rect = rect;
        element->transform = element->combinedWorldMatrix();
        element->imageDrawMode = mode;
        element->borderThickness = borderThickness;
        element->srcRect = textureSourceRect;
        element->wrapMode = BrushWrapMode::Stretch;
    }
}

//void UIRenderingContext::drawBoxBackground(const Rect& rect, const CornerRadius& cornerRadius, BrushImageDrawMode mode/*, AbstractMaterial* material*/, const Rect& textureSourceRect, const Color& color)
//{
//    //m_builder->setMaterial(material);
//
//    if (m_builder->material() && !m_builder->material()->mainTexture()) {
//        mode = BrushImageDrawMode::Image;
//    }
//
//	if (mode == BrushImageDrawMode::Image)
//	{
//        auto* element = m_builder->addNewDrawElement<detail::DrawShapesElement>(
//            m_manager->shapesRenderFeature(),
//            m_builder->shapesRenderFeatureStageParameters());
//		
//        element->commandList.addDrawBoxBackground(m_builder->targetList()->dataAllocator(), element->combinedWorldMatrix(), rect, cornerRadius, color);
//	}
//	else
//	{
//		auto* element = m_builder->addNewDrawElement<detail::DrawFrameRectElement>(
//			m_manager->frameRectRenderFeature(),
//			m_builder->frameRectRenderFeatureStageParameters());
//
//		element->rect = rect;
//		element->transform = element->combinedWorldMatrix();
//		element->imageDrawMode = mode;
//		element->borderThickness = Thickness();	// TODO: //borderThickness;
//		element->srcRect = textureSourceRect;
//		element->wrapMode = BrushWrapMode::Stretch;
//	}
//
//    // TODO: bounding box
//}

void UIRenderingContext::drawBoxBorderLine(const Rect& rect, const Thickness& thickness, const Color& leftColor, const Color& topColor, const Color& rightColor, const Color& bottomColor, const CornerRadius& cornerRadius, bool borderInset)
{
    auto* element = m_builder->addNewDrawElement<detail::DrawShapesElement>(
        m_manager->shapesRenderFeature(),
        m_builder->shapesRenderFeatureStageParameters());

    element->commandList.drawBoxBorderLine(m_builder->targetList()->dataAllocator(), element->combinedWorldMatrix(), rect, thickness, leftColor, topColor, rightColor, bottomColor, cornerRadius, borderInset);


    // TODO: bounding box
}

void UIRenderingContext::drawBoxBorderLine(const Rect& rect, float thickness, const Color& color, bool borderInset)
{
	drawBoxBorderLine(rect, Thickness(thickness), color, color, color, color, CornerRadius(), borderInset);
}

void UIRenderingContext::drawBoxShadow(const Rect& rect, const CornerRadius& cornerRadius, const Vector2& offset, const Color& color, float blur, float width, bool inset)
{
	auto* element = m_builder->addNewDrawElement<detail::DrawShapesElement>(
		m_manager->shapesRenderFeature(),
		m_builder->shapesRenderFeatureStageParameters());

	element->commandList.addDrawBoxShadow(m_builder->targetList()->dataAllocator(), element->combinedWorldMatrix(), rect, cornerRadius, offset, color, blur, width, inset);


	// TODO: bounding box

}

void UIRenderingContext::drawImage(const Rect& destinationRect, AbstractMaterial* material)
{
    auto texture = material->mainTexture();
    drawSprite(
        Matrix::makeTranslation(destinationRect.x, destinationRect.y, 0),
        destinationRect.getSize(),
        Vector2::Zero,
        Rect(0, 0, 1, 1),
        Color::White,
        SpriteBaseDirection::Basic2D,
        BillboardType::None,
        detail::SpriteFlipFlags::None,
        material);
}

void UIRenderingContext::drawVisual(UIElement* element, const Matrix& transform)
{
	//pushState();
	element->renderClient(this, m_builder->baseTransform() * element->m_localTransform * transform);
	//popState();
}

void UIRenderingContext::resetForBeginRendering()
{
	RenderingContext::resetForBeginRendering();
	m_elementList->clear();
}

} // namespace ln

