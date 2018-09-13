﻿#pragma once

namespace ln {
namespace detail {


/* 
 * RenderingManager
 * 
 * 
 * 主要クラスについて
 * --------
 * 
 * ### RenderStage
 * State と Command(DrawElement) を固めて持っておくクラス。
 * Unity でいうところの CommandBuffer に相当する。
 * SceneNode 1つ分と考えてもよい。
 * 基本的にどんなタイミングでも、「RenderStage を描画」すれば同じジオメトリが表示される。
 * 
 * ### RenderingContext
 * ユーザーが何か直接描画したいときに主に使うことになるクラス。
 * Unity でいうところの Graphics クラス。
 * コマンドリスト (RenderStage の集合) を構築する。
 * 
 * ### SceneRenderer
 * コマンドリスト (RenderStage の集合) をどのように描画するかを決めるクラス。
 * 3D なら ForwardShading だったり、2D なら NoShading だったり。
 * RenderStage の集合に対して遅延描画などの描画方式の都合や最適化のための並び替え、カリングなどを行い正確に効率よく描画する。
 * 実際の描画は RenderFeature に任せる。
 * インスタンスは RenderView が保持する。
 * 
 * ### RenderView
 * シーンレンダリングが行われるフレームバッファと考えてよい。
 * RenderStage の集合 を入力して描画を行う、シーンレンダリングのエントリーポイント。
 * SceneRenderer のインスタンスを持つ。（SceneRendererとは 1:1 の関係）
 * RenderStage の集合 に対しては n:n。
 * これは主にエディタ上で、シーンのプレビューを異なるウィンドウで見れるようにするための仕組み。(通常レンダリングと深度マップを同時表示など)
 * オフスクリーンレンダリングでも使用される。
 * 
 * ###　RenderFerture
 * Sprite や Text など、様々なコンポーネントの描画に特化した派生クラスがある。
 * これの種類によっても State が異なることになるため RenderStage は拡張性が重要となる。
 * またそれ以上に特にユーザーカスタマイズを考慮する必要がある。
 * 例えばユーザーが外部のパーティクルエンジンを組み込みたいと思ったら・・・
 * - ExParticleRenderFeature のような描画を行うクラスを作る (state less)
 * - ExParticleRenderFeatureState のような State 保持クラスを作る
 * - ExParticleRenderCommand のようなジオメトリ形状保持クラスを作る (0.4.0 のころの DrawElement)
 * - RenderStage へステートとして、
 *   - ExParticleRenderFeature のポインタ
 *   - ExParticleRenderFeatureState
 *   をセットする。
 * - 描画時、ExParticleRenderCommand を add する
 * 
 * 
 * World と UI
 * --------
 * それぞれ State として必要なものが違うので、RenderStage を継承した
 * WorldRenderStage や UIRenderStage を作ることになる。
 * 
 * WorldRenderStage は Material などを持ち、
 * UIRenderStage は Brush や Pen などを持つ。
 * 
 */
class RenderingManager
	: public RefObject
{
public:
	struct Settings
	{
		GraphicsManager* graphicsManager;
	};

	RenderingManager();
	void initialize(const Settings& settings);
	void dispose();

private:
};

} // namespace detail
} // namespace ln
