
#include <Workspace.hpp>
#include <Project.hpp>
#include <AssetDatabase.hpp>
#include <PluginManager.hpp>
#include "../App/EditorContext.hpp"
#include "TilemapSceneEditorModel.hpp"
#include "TilemapSceneModePane.hpp"

namespace lna {

//==============================================================================
// TilemapBrush

void TilemapBrush::setNormalTiles(ln::Tileset* tileset, const ln::RectI& range)
{
	if (LN_REQUIRE(tileset)) return;

	int beginX = range.x;
	int beginY = range.y;
	int endX = range.x + range.width;
	int endY = range.y + range.height;

	m_tiles.clear();
	for (int y = beginY; y < endY; y++) {
		for (int x = beginX; x < endX; x++) {
			int id = y * TilemapSceneModePane::DisplayTilesX + x;
			m_tiles.push_back(id);
		}
	}

	m_source = TilemapBrushSource::Tileset;
	m_range.x = beginX;
	m_range.y = beginY;
	m_range.width = endX - beginX;
	m_range.height = endY - beginY;
}

//==============================================================================
// TilemapSceneEditorModel

TilemapSceneEditorModel::TilemapSceneEditorModel()
	: m_tilemapBrush(ln::makeObject<TilemapBrush>())
{
}

ln::Result TilemapSceneEditorModel::createNewTilemapSceneAsset(lna::EditorContext* context, const ln::Path& filePath)
{

	auto tilesetAsset = context->assetDatabase()->openAsset(u"D:/Proj/LN/PrivateProjects/HC0/Assets/Tilesets/Tileset-1");

	auto mainWorld = ln::makeObject<ln::World>();

	auto tilemap = ln::makeObject<ln::Tilemap>();
	tilemap->setShadingModel(ln::ShadingModel::UnLighting);
	mainWorld->addObject(tilemap);

	//tilesetMaterial->setMainTexture(ln::Texture2D::create((u"D:/Proj/LN/PrivateProjects/HC0/Assets/Tilesets/BaseChip_pipo.png")));
	auto tilesetTexture = ln::Assets::loadTexture(u"32066696-1621-4EED-820D-535BB2F22A9D");
	auto tilesetMaterial = ln::makeObject<ln::Material>();
	tilesetMaterial->setMainTexture(tilesetTexture);

	auto tileset = dynamic_cast<ln::Tileset*>(tilesetAsset->target()); //ln::makeObject<ln::Tileset>();
	tileset->reset(tilesetMaterial, 16, 16);

	auto layer = ln::makeObject<ln::TilemapLayer>();
	layer->resize(5, 5);
	layer->setTileSize(ln::Size(1, 1));
	layer->setOrientation(ln::TilemapOrientation::UpFlow);
	layer->setTileId(0, 0, 1);
	layer->setTileId(1, 1, 1);
	layer->setTileId(0, 4, 1);
	layer->setTileId(1, 4, 1);
	layer->setTileId(2, 4, 1);
	layer->setTileId(3, 4, 1);
	layer->setTileId(4, 4, 1);
	auto tilemapModel = ln::makeObject<ln::TilemapModel>();
	tilemapModel->addTileset(tileset);
	tilemapModel->addLayer(layer);
	tilemap->setTilemapModel(tilemapModel);

	return context->mainProject()->assetDatabase()->createAsset(mainWorld, filePath);
}

} // namespace lna

