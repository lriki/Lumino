
#pragma once
#include <glues/source/glues.h>
#include "FreeTypeFont.h"

LN_NAMESPACE_BEGIN
namespace detail {

	//struct VertexInfo {
	//	double v_[3]; //!< Why is this double precision? Because the second
	//	

	//	//! Default constructor just initializes Vertex to zero.
	//	//! \param color_tess optional color tesselation object.
	//	//! \param texture_tess optional texture tesselation object.
	//	VertexInfo(ColorTess* color_tess = 0, TextureTess* texture_tess = 0)
	//		: color_tess_(color_tess), texture_tess_(texture_tess)
	//	{
	//		v_[X] = v_[Y] = v_[Z] = 0.;
	//	}

	//	/*!
	//	* Construct a Vertex from a point in a FreeType contour.
	//	* \param ft_v a FreeType FT_Vector, normally passed into the
	//	* the decomposition callbacks.
	//	* \param color_tess optional color tesselation object.
	//	* \param texture_tess optional texture tesselation object.
	//	*/
	//	VertexInfo(FT_Vector* ft_v, ColorTess* color_tess = 0,
	//		TextureTess* texture_tess = 0)
	//		: color_tess_(color_tess), texture_tess_(texture_tess)
	//	{
	//		v_[X] = (double)(ft_v->x / 64) + (double)(ft_v->x % 64) / 64.;
	//		v_[Y] = (double)(ft_v->y / 64) + (double)(ft_v->y % 64) / 64.;
	//		v_[Z] = 0.;
	//	}

	//	/*!
	//	* Construct a Vertex from a 2D point.
	//	* \param p 2D array of doubles.
	//	* \param color_tess optional color tesselation object.
	//	* \param texture_tess optional texture tesselation object.
	//	*/
	//	VertexInfo(double p[2], ColorTess* color_tess = 0,
	//		TextureTess* texture_tess = 0)
	//		: color_tess_(color_tess), texture_tess_(texture_tess)
	//	{
	//		v_[X] = p[X];
	//		v_[Y] = p[Y];
	//		v_[Z] = 0.;
	//	}

	//	/*!
	//	* Construct a Vertex from a 2D point.
	//	* \param x the X coordinate.
	//	* \param y the Y coordinate.
	//	* \param color_tess optional color tesselation object.
	//	* \param texture_tess optional texture tesselation object.
	//	*/
	//	VertexInfo(double x, double y, ColorTess* color_tess = 0,
	//		TextureTess* texture_tess = 0)
	//		: color_tess_(color_tess), texture_tess_(texture_tess)
	//	{
	//		v_[X] = x;
	//		v_[Y] = y;
	//		v_[Z] = 0.;
	//	}

	//	//! Treat the Vertex like a vector: Normalize its length in the
	//	//! usual way.
	//	void normalize(void)
	//	{
	//		double length = sqrt(v_[X] * v_[X] + v_[Y] * v_[Y] + v_[Z] * v_[Z]);
	//		v_[X] /= length;
	//		v_[Y] /= length;
	//		v_[Z] /= length;
	//	}
	//};

class Filled
{
public:
	void Initialize();

	void DecomposeOutlineVertices(FreeTypeFont* font, UTF32 utf32code);

	void CalculateExtrusion();

	void Tessellate();

	void MakeEdgeStroke();


	void renderGlyph(FreeTypeFont* font, UTF32 ch);

public:
	enum Coordinates {
		X, //!< The X component of space
		Y, //!< The Y component of space
		Z, //!< The Z component of space
		W  //!< The projection component of space
	};

	typedef void(*GLUTessCallback)();

	FT_Outline_Funcs	m_ftOutlineFuncs;
	GLUtesselator*		m_gluTesselator;
	float				m_pointSize = 12;
	float				m_resolution = 100;		// DPI

	bool contour_open_;	// 新しい輪郭が始まったかどうか。


	//using VertexInfo = Vector3;
	struct VertexInfo
	{
		Vector2	pos;
		Vector2	extrusion;	// 押し出し方向
		float	alpha;

		VertexInfo(const Vector2& pos_)
			: pos(pos_)
			, alpha(1.0f)
		{}
	};
	
	Vector2		m_lastVertex;	// テッセレーションプロセス内で最後に処理した制御点
	float		m_vectorScale;



	int m_tessellationStep;	// なめらかさ

	float	m_delta1;
	float	m_delta2;
	float	m_delta3;

	static Vector2 FTVectorToLNVector(const FT_Vector* ftVec);

	void setTessellationSteps(int steps)
	{
		m_tessellationStep = steps;
		m_delta1 = 1. / (double)m_tessellationStep;
		m_delta2 = m_delta1 * m_delta1;
		m_delta3 = m_delta2 * m_delta1;
	}


	struct ContourOutline
	{
		int	startIndex = 0;
		int	indexCount = 0;
	};

	List<ContourOutline>	m_contourOutlineList;
	List<VertexInfo>		m_vertexList;


	struct Contour
	{
		int	primitiveType;	// GLenum (GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, or GL_TRIANGLES)

		// TriangleFan と、TriangleStrip の後続頂点のインデックス。三角形を構成する1つめの頂点番号。
		int	intermediateVertexIndex1;

		// TriangleFan と、TriangleStrip の後続頂点のインデックス。三角形を構成する2つめの頂点番号。
		int	intermediateVertexIndex2;

		int faceCount;
	};
	List<Contour>	m_contourList;
	List<uint16_t>	m_triangleIndexList;	// 要素数は3の倍数となる



	//! A place to store any extra vertices generated by the Combine callback
	//VertexInfoList extra_vertices_;

protected:
	//! Offset the glyph in the Z direction. Solely for the Solid subclass.
	//! Until I can figure out how to shift the glyph outside the context
	//! of this class, I guess this has got to stay (but it is redundant
	//! to extrusion_.depth_)
	//GLfloat depth_offset_;

public:
	///*!
	//* \param filename the filename which contains the font face.
	//* \param point_size the initial point size of the font to generate. A point
	//* is essentially 1/72th of an inch. Defaults to 12.
	//* \param resolution the pixel density of the display in dots per inch (DPI).
	//* Defaults to 100 DPI.
	//*/
	//Filled(const char* filename, float point_size = 12,
	//	FT_UInt resolution = 100);
	///*!
	//* \param face open FreeType FT_Face.
	//* \param point_size the initial point size of the font to generate. A point
	//* is essentially 1/72th of an inch. Defaults to 12.
	//* \param resolution the pixel density of the display in dots per inch (DPI).
	//* Defaults to 100 DPI.
	//*/
	//Filled(FT_Face face, float point_size = 12, FT_UInt resolution = 100);
	///*!
	//* The destructor deletes the GLU tessellation object allocated in
	//* in the constructor.
	//*/
	virtual ~Filled(void);

	/*!
	* \return the list of extra vertices created by the GLU tessellation
	* combine callback.
	*/
	//VertexInfoList& extraVertices(void) { return extra_vertices_; }

protected:
	
private:
	static int ftMoveToCallback(FT_Vector* to, Filled* thisData);
	static int ftLineToCallback(FT_Vector* to, Filled* thisData);
	static int ftConicToCallback(FT_Vector* control, FT_Vector* to, Filled* thisData);
	static int ftCubicToCallback(FT_Vector* control1, FT_Vector* control2, FT_Vector* to, Filled* thisData);

	static void tessBeginCallback(GLenum primitiveType, Filled* thisData);
	static void tessEndCallback(Filled* thisData);
	static void vertexDataCallback(void* vertexData, Filled* thisData);
	static void combineCallback(GLfloat coords[3], void* vertex_data[4], GLfloat weight[4], void** out_data, Filled* thisData);
	static void errorCallback(GLenum error_code);
};




class FontOutlineTessellator
{
public:
	FontOutlineTessellator();
	~FontOutlineTessellator();

	void Tessellate(RawFont::VectorGlyphInfo* info);

private:
	typedef void(*GLUTessCallback)();

	struct Contour
	{
		int	primitiveType;	// GLenum (GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, or GL_TRIANGLES)

							// TriangleFan と、TriangleStrip の後続頂点のインデックス。三角形を構成する1つめの頂点番号。
		int	intermediateVertexIndex1;

		// TriangleFan と、TriangleStrip の後続頂点のインデックス。三角形を構成する2つめの頂点番号。
		int	intermediateVertexIndex2;

		int faceCount;

	};

	struct TessellatingState
	{
		FontOutlineTessellator*		thisPtr;
		RawFont::VectorGlyphInfo*	glyphInfo;
		List<Contour>				contourList;
	};


	static void BeginCallback(GLenum primitiveType, TessellatingState* state);
	static void EndCallback(TessellatingState* state);
	static void VertexDataCallback(void* vertexData, TessellatingState* state);
	static void CombineCallback(GLfloat coords[3], void* vertex_data[4], GLfloat weight[4], void** out_data, TessellatingState* state);
	static void ErrorCallback(GLenum error_code);



	GLUtesselator*		m_gluTesselator;
};

class FontOutlineStroker
{
public:

	void MakeStroke(RawFont::VectorGlyphInfo* info);

private:
	void CalculateExtrusion();
	void MakeAntiAliasStroke();
	
	RawFont::VectorGlyphInfo* m_info;
};

} // namespace detail
LN_NAMESPACE_END
