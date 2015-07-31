﻿
#pragma once

#include <vector>
#include <Lumino/Base/Delegate.h>
#include <Lumino/Base/Rect.h>
#include "Device/DeviceInterface.h"
#include <Lumino/Graphics/RenderState.h>
#include <Lumino/Graphics/SamplerState.h>

#include <Lumino/Graphics/SwapChain.h>
#include <Lumino/Graphics/VertexBuffer.h>
#include <Lumino/Graphics/IndexBuffer.h>
#include <Lumino/Graphics/Texture.h>
#include <Lumino/Graphics/Utils.h>

#define LN_RC_TRACE /*printf*/

namespace Lumino
{
namespace Graphics
{
class SwapChain;
class Texture;
class VertexBuffer;
class IndexBuffer;
class ShaderPass;
class RenderingCommandList;

/// 
///		×必ずメインスレッド (描画スレッド以外) で生成されて、描画スレッドで実行される。
///		×operator new 等、C++ のコアな機能を使っているが、これはサブクラスとして定義するコマンドの実装を簡単にするため。
///		×メモリ確保と m_commandList の初期化を operator new で行うことで、サブクラスにアロケータ処理を書く必要が無くなる。
///
///		Alloc は Create 内でのみ可
/**
	描画コマンドの基底クラス

	以前は operator new をオーバーロードし、コンストラクタで初期化を行えるようにしていた。
	しかし、コマンドは RenderingCommandList::Alloc() が呼ばれると再配置される可能性がある。
	そのため、コンストラクタ内で Alloc() すると this が不正なポインタになることがある。

	対策として、初期化は static 関数である Create() で行うようにした。
	ユーティリティ関数を用意したものの、キャストなどで冗長になってしまうのはやむなし。
	(急いで作ったから、もう少し改善の余地はあるかも)

	必ず守らなければならないのは、HandleCast() と Alloc() を同一式の中に書いてはならないこと。
	例えば以下のようにしてはならない。

		HandleCast<Command>(cmd)->m_data = Alloc(cmd, 10, ...);

	Alloc() で再配置の可能性があるので、格納先である m_data が不正なメモリ位置になってしまう。
*/
struct RenderingCommand
{
public:
	typedef size_t	DataHandle;

	virtual void Execute() = 0;

protected:
	inline Device::IRenderer* GetRenderer() const;
	inline DataHandle AllocExtData(size_t byteCount, const void* copyData);
	inline void* GetExtData(DataHandle handle);
	inline void MarkGC(RefObject* obj);

private:
	friend class RenderingCommandList;
	RenderingCommandList*	m_commandList;
	//size_t					m_dataHandle;

	//struct CmdInfo
	//{
	//	RenderingCommandList*	m_commandList;
	//	size_t					m_dataHandle;
	//};



	//RenderingCommand() {}
	//virtual ~RenderingCommand() {}

	//virtual void Execute() = 0;
	//virtual void Release(RenderingCommandList* commandList) {}	// 削除予定。デストラクタに任せる

	//template<class CommandT>
	//static CommandT* HandleCast(CmdInfo& cmd)
	//{
	//	return reinterpret_cast<CommandT*>(cmd.m_commandList->GetExtData(cmd.m_dataHandle));
	//}

	//inline static size_t AllocExtData(size_t byteCount, const void* copyData);

	//inline void MarkGC(RefObject* obj);

//private:
	//friend class RenderingCommandList;
	//static void* operator new(size_t size, RenderingCommandList* cmmandList);
	//static void operator delete(void* ptr, RenderingCommandList* cmmandList);	// 一応書いているが、直接 delete しようとするとコンパイルエラー「非置換 delete が定義されていません」になるはず
};

//#define LN_RENDERING_COMMAND_IMPLEMENT(cmdClass) \
	
//	static void ExecuteInternal(RenderingCommand* cmd) { Execute(static_cast<cmdClass*>(cmd)); }



class RenderingCommandList
	: public RefObject
{
public:
	typedef size_t	DataHandle;

	RenderingCommandList();
	virtual ~RenderingCommandList();

public:
	void ClearCommands();

	/// すべてのコマンドを実行する (描画スレッドから呼ばれる)
	void Execute(Device::IRenderer* renderer);

	/// 後処理 (描画スレッドから呼ばれる)
	void PostExecute();

	/// 描画キューに入っているか
	bool IsRunning() { return m_running.IsTrue(); }

	/// アイドル状態になるまで待つ
	void WaitForIdle() { m_idling.Wait(); }


private:

	static const size_t DataBufferReserve = 20;


private:
	DataHandle AllocCommand(size_t byteCount, const void* copyData);

	RenderingCommand* GetCommand(DataHandle bufferIndex) { return (RenderingCommand*)&(m_commandDataBuffer.GetData()[bufferIndex]); }

	template<typename T>
	DataHandle CreateCommand()
	{
		size_t dataHandle = AllocCommand(sizeof(T), NULL);
		T* t = new (GetCommand(dataHandle))T();
		t->m_commandList = this;
		//t->m_dataHandle = dataHandle;
		return dataHandle;
	}



public:

	template<typename T, typename... TArgs>
	void AddCommand(TArgs... args)
	{
		DataHandle h = CreateCommand<T>();
		T* cmd = static_cast<T*>(GetCommand(h));
		//RenderingCommand::CmdInfo cmd;
		//cmd->m_commandList = this;
		//cmd.m_dataHandle = t->m_dataHandle;
		cmd->Create(args...);
		LN_RC_TRACE("RenderingCommandList::AddCommand 0() s %p\n", this);
		m_commandList.Add(h);
	}

#if 0
	template<typename T, typename A1>
	void AddCommand(const A1& a1)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1);
		LN_RC_TRACE("RenderingCommandList::AddCommand 0() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2>
	void AddCommand(const A1& a1, const A2& a2)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2);
		LN_RC_TRACE("RenderingCommandList::AddCommand 1() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2, typename A3>
	void AddCommand(const A1& a1, const A2& a2, const A3& a3)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2, a3);
		LN_RC_TRACE("RenderingCommandList::AddCommand 2() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2, typename A3, typename A4>
	void AddCommand(const A1& a1, const A2& a2, const A3& a3, const A4& a4)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2, a3, a4);
		LN_RC_TRACE("RenderingCommandList::AddCommand 3() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5>
	void AddCommand(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2, a3, a4, a5);
		LN_RC_TRACE("RenderingCommandList::AddCommand 4() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void AddCommand(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2, a3, a4, a5, a6);
		LN_RC_TRACE("RenderingCommandList::AddCommand 5() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void AddCommand(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2, a3, a4, a5, a6, a7);
		LN_RC_TRACE("RenderingCommandList::AddCommand 6() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
	template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void AddCommand(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
	{
		T* t = CreateCommand<T>();
		RenderingCommand::CmdInfo cmd;
		cmd.m_commandList = this;
		cmd.m_dataHandle = t->m_dataHandle;
		T::Create(cmd, a1, a2, a3, a4, a5, a6, a7, a8);
		LN_RC_TRACE("RenderingCommandList::AddCommand 8() s %p\n", this);
		m_commandList.Add(cmd.m_dataHandle);
	}
#endif

public:
	DataHandle AllocExtData(size_t byteCount, const void* copyData);
	void* GetExtData(DataHandle bufferIndex);
	void MarkGC(RefObject* obj) 
	{ 
		obj->AddRef();
		m_markGCList.Add(obj);
	}

private:
	Array<size_t>			m_commandList;
	ByteBuffer				m_commandDataBuffer;
	size_t					m_commandDataBufferUsed;
	ByteBuffer				m_extDataBuffer;
	size_t					m_extDataBufferUsed;
	Array<RefObject*>		m_markGCList;
	Device::IRenderer*		m_currentRenderer;	///< 描画実行中の IRenderer

	friend class RenderingThread;
	friend class UserRenderingCommand;
	friend struct RenderingCommand;
	Threading::EventFlag	m_running;	///< 描画キューに入っているか
	Threading::EventFlag	m_idling;
};

inline Device::IRenderer* RenderingCommand::GetRenderer() const
{
	return m_commandList->m_currentRenderer;
}
inline RenderingCommand::DataHandle RenderingCommand::AllocExtData(size_t byteCount, const void* copyData)
{ 
	return m_commandList->AllocExtData(byteCount, copyData);
}
inline void* RenderingCommand::GetExtData(DataHandle handle)
{
	return m_commandList->GetExtData(handle);
}
inline void RenderingCommand::MarkGC(RefObject* obj)
{
	m_commandList->MarkGC(obj);
}



//=============================================================================
struct SetRenderStateCommand : public RenderingCommand
{
	RenderState	m_state;
	void Create(const RenderState& state) { m_state = state; }
	void Execute() { GetRenderer()->SetRenderState(m_state); }
};

//=============================================================================
struct SetDepthStencilStateCommand : public RenderingCommand
{
	DepthStencilState	m_state;
	void Create(const DepthStencilState& state) { m_state = state; }
	void Execute() { GetRenderer()->SetDepthStencilState(m_state); }
};

//=============================================================================
struct SetRenderTargetCommand : public RenderingCommand
{
	int m_index;
	Texture* m_sourceTexture;
	void Create(int index, Texture* texture)	// TODO: Device::Itexture の方が良い気がする
	{
		m_index = index;
		m_sourceTexture = texture;
		MarkGC(texture);
	}
	void Execute()
	{
		GetRenderer()->SetRenderTarget(m_index, (m_sourceTexture != NULL) ? m_sourceTexture->m_deviceObj : NULL);
	}
};

//=============================================================================
struct SetDepthBufferCommand : public RenderingCommand
{
	Texture* m_sourceTexture;
	void Create(Texture* texture)	// TODO: Device::Itexture の方が良い気がする
	{
		m_sourceTexture = texture;
		MarkGC(texture);
	}
	void Execute()
	{
		GetRenderer()->SetDepthBuffer((m_sourceTexture != NULL) ? m_sourceTexture->m_deviceObj : NULL);
	}
};

//=============================================================================
struct SetViewportCommand : public RenderingCommand
{
	Rect m_viewportRect;
	void Create(const Rect& rect) { m_viewportRect = rect; }
	void Execute() { GetRenderer()->SetViewport(m_viewportRect); }
};

//=============================================================================
struct SetVertexBufferCommand : public RenderingCommand
{
	VertexBuffer* m_sourceVertexBuffer;
	void Create(VertexBuffer* vertexBuffer)// TODO: Device::IVertexBuffer の方が良い気がする
	{
		m_sourceVertexBuffer = vertexBuffer;
		MarkGC(vertexBuffer);
	}
	void Execute()
	{
		GetRenderer()->SetVertexBuffer((m_sourceVertexBuffer != NULL) ? m_sourceVertexBuffer->m_deviceObj : NULL);
	}
};

//=============================================================================
struct SetIndexBufferCommand : public RenderingCommand
{
	IndexBuffer* m_sourceIndexBuffer;
	void Create(IndexBuffer* indexBuffer)// TODO: Device::IIndexBuffer の方が良い気がする
	{
		m_sourceIndexBuffer = indexBuffer;
		MarkGC(indexBuffer);
	}
	void Execute()
	{
		GetRenderer()->SetIndexBuffer((m_sourceIndexBuffer != NULL) ? m_sourceIndexBuffer->m_deviceObj : NULL);
	}
};

//=============================================================================
struct ClearCommand : public RenderingCommand
{
	ClearFlags m_flags;
	ColorF m_clearColor;
	float m_z;
	uint8_t m_stencil;
	void Create(ClearFlags flags, const ColorF& color, float z, uint8_t stencil)
	{
		m_flags = flags;
		m_clearColor = color;
		m_z = z;
		m_stencil = stencil;
	}
	void Execute()
	{
		GetRenderer()->Clear(m_flags, m_clearColor, m_z, m_stencil);
	}
};

//=============================================================================
struct DrawPrimitiveCommand : public RenderingCommand
{
	PrimitiveType m_primitive;
	int m_startVertex;
	int m_primitiveCount;
	void Create(PrimitiveType primitive, int startVertex, int primitiveCount)
	{
		m_primitive = primitive;
		m_startVertex = startVertex;
		m_primitiveCount = primitiveCount;
	}
	void Execute()
	{
		GetRenderer()->DrawPrimitive(m_primitive, m_startVertex, m_primitiveCount);
	}
};

//=============================================================================
struct DrawPrimitiveIndexedCommand : public RenderingCommand
{
	PrimitiveType m_primitive;
	int m_startIndex;
	int m_primitiveCount;
	void Create(PrimitiveType primitive, int startIndex, int primitiveCount)
	{
		m_primitive = primitive;
		m_startIndex = startIndex;
		m_primitiveCount = primitiveCount;
	}
	void Execute()
	{
		GetRenderer()->DrawPrimitiveIndexed(m_primitive, m_startIndex, m_primitiveCount);
	}
};

//=============================================================================
struct SetSamplerStateCommand : public RenderingCommand
{
	Device::ITexture* m_targetTexture;
	SamplerState m_state;
	void Create(Device::ITexture* texture, const SamplerState& state)
	{
		m_targetTexture = texture;
		m_state = state;
		MarkGC(m_targetTexture);
	}
	void Execute()
	{
		m_targetTexture->SetSamplerState(m_state);
	}
};

//=============================================================================
//class SetTextureSubDataCommand : public RenderingCommand
//{
//	Device::ITexture*		m_targetTexture;
//	size_t					m_sourceBitmapDataHandle;
//	Size					m_size;
//	Imaging::PixelFormat	m_format;
//
//public:
//	static void Create(Device::ITexture* texture, Imaging::Bitmap* bitmap)
//	{
//		// メモリ確保は一度ハンドルをローカル変数に置く。1つの式の中で複数回 HandleCast を使用してはならないため。
//		size_t tmpData = Alloc(cmd, bitmap->GetBitmapBuffer()->GetSize(), bitmap->GetBitmapBuffer()->GetData());
//
//		HandleCast<SetTextureSubDataCommand>(cmd)->m_targetTexture = texture;
//		HandleCast<SetTextureSubDataCommand>(cmd)->m_sourceBitmapDataHandle = tmpData;
//		HandleCast<SetTextureSubDataCommand>(cmd)->m_size = bitmap->GetSize();
//		HandleCast<SetTextureSubDataCommand>(cmd)->m_format = bitmap->GetPixelFormat();
//	}
//
//private:
//	virtual void Execute()
//	{
//		// 参照モードで一時メモリを Bitmap 化する (メモリコピーを行わない)
//		ByteBuffer refData(commandList->GetBuffer(m_sourceBitmapDataHandle), Imaging::Bitmap::GetPixelFormatByteCount(m_format, m_size), true);
//		Imaging::Bitmap lockedBmp(&refData, m_size, m_format);
//
//		m_targetTexture->SetSubData(&lockedBmp);
//	}
//};

//=============================================================================
struct SetShaderVariableCommand : public RenderingCommand
{
	union
	{
		bool				BoolVal;	// X11 では "Bool" が定義済みマクロなので Val を付けて逃げる
		int					Int;
		float				Float;
		size_t				VectorsBufferIndex;
		//Vector4*			Vector;		///< 単一 Vector または VectorArray
		//Lumino::Matrix*		Matrix;		///< 単一 Matrix または MatrixArray
		Device::ITexture*	Texture;
	};
	DataHandle					m_arrayLength;
	ShaderVariableType			m_variableType;
	Device::IShaderVariable*	m_target;

	void Create(Device::IShaderVariable* target, bool value)
	{
		m_target = target;
		m_variableType = ShaderVariableType_Bool;
		BoolVal = value;
		MarkGC(target);
	}
	void Create(Device::IShaderVariable* target, int value)
	{
		m_target = target;
		m_variableType = ShaderVariableType_Int;
		Int = value;
		MarkGC(target);
	}
	void Create(Device::IShaderVariable* target, float value)
	{
		m_target = target;
		m_variableType = ShaderVariableType_Float;
		Float = value;
		MarkGC(target);
	}
	void Create(Device::IShaderVariable* target, const Lumino::Vector4* vectors, size_t count)
	{
		size_t tmpData = AllocExtData(sizeof(Vector4) * count, vectors);
		m_target = target;
		m_arrayLength = count;
		m_variableType = (count == 1) ? ShaderVariableType_Vector : ShaderVariableType_VectorArray;
		VectorsBufferIndex = tmpData;
		MarkGC(target);
	}
	void Create(Device::IShaderVariable* target, const Lumino::Matrix* matrices, size_t count)
	{
		size_t tmpData = AllocExtData(sizeof(Matrix) * count, matrices);
		m_target = target;
		m_arrayLength = count;
		m_variableType = (count == 1) ? ShaderVariableType_Matrix : ShaderVariableType_MatrixArray;
		VectorsBufferIndex = tmpData;
		MarkGC(target);
	}
	void Create(Device::IShaderVariable* target, Device::ITexture* value)
	{
		m_target = target;
		m_variableType = ShaderVariableType_Texture;
		Texture = value;
		MarkGC(target);
		MarkGC(value);
	}

	void Execute()
	{
		switch (m_variableType)
		{
		case ShaderVariableType_Bool:
			m_target->SetBool(BoolVal);
			break;
		case ShaderVariableType_Int:
			m_target->SetInt(Int);
			break;
		case ShaderVariableType_Float:
			m_target->SetFloat(Float);
			break;
		case ShaderVariableType_Vector:
			m_target->SetVector(*((Vector4*)GetExtData(VectorsBufferIndex)));
			break;
		case ShaderVariableType_VectorArray:
			m_target->SetVectorArray((Vector4*)GetExtData(VectorsBufferIndex), m_arrayLength);
			break;
		case ShaderVariableType_Matrix:
			m_target->SetMatrix(*((Matrix*)GetExtData(VectorsBufferIndex)));
			break;
		case ShaderVariableType_MatrixArray:
			m_target->SetMatrixArray((Matrix*)GetExtData(VectorsBufferIndex), m_arrayLength);
			break;
		case ShaderVariableType_Texture:
			m_target->SetTexture(Texture);
			break;
		//case ShaderVariableType_String:
		default:
			break;
		}
	}
};

//=============================================================================
struct ApplyShaderPassCommand : public RenderingCommand
{
	Device::IShaderPass* m_pass;
	void Create(Device::IShaderPass* pass)
	{
		m_pass = pass;
		MarkGC(pass);
	}
	void Execute() { m_pass->Apply(); }
};

//=============================================================================
struct PresentCommand : public RenderingCommand
{
	SwapChain* m_targetSwapChain;

	void Create(SwapChain* swapChain)
	{
		m_targetSwapChain = swapChain;
		MarkGC(swapChain);
	}

	void Execute()
	{
		m_targetSwapChain->m_deviceObj->Present(m_targetSwapChain->m_backColorBuffer->m_deviceObj);


		// 実行完了。m_waiting を ture にすることで、メインスレッドからはこのスワップチェインをキューに追加できるようになる。
		// コマンドの成否にかかわらず true にしないと、例外した後にデッドロックが発生する。

		// TODO: ポインタが fefefefe とかなってたことがあった。メモリバリア張っておくこと。
		m_targetSwapChain->m_waiting.SetTrue();
		//LN_SAFE_RELEASE(m_targetSwapChain);
		// TODO: ↑これは CommandList::Execute 側でやるべきなきがする
	}
};
	
//=============================================================================
struct SetSubDataTextureCommand : public RenderingCommand	// TODO: ↑似たようなコマンドが残っている
{
	Device::ITexture*		m_targetTexture;
	Point					m_offset;
	size_t					m_bmpDataIndex;
	Size					m_bmpSize;
	// ↑エラーチェックは Texture で行い、フォーマットは既に決まっていることを前提とするため、コマンドに乗せるデータはこれだけでOK。

	void Create(Device::ITexture* texture, const Point& offset, const void* data, size_t dataSize, const Size& bmpSize)
	{
		m_targetTexture = texture;
		m_offset = offset;
		m_bmpDataIndex = AllocExtData(dataSize, data);
		m_bmpSize = bmpSize;
		MarkGC(texture);
	}

	void Execute()
	{
		m_targetTexture->SetSubData(m_offset, GetExtData(m_bmpDataIndex), m_bmpSize);
	}
};

//=============================================================================
struct Texture_SetSubDataBitmapCommand : public RenderingCommand
{
	//RefPtr<Device::ITexture> m_targetTexture;
	Device::ITexture* m_targetTexture;
	Point m_offset;

	size_t m_bmpDataIndex;
	Size m_size;
	int m_pitch;
	Imaging::PixelFormat m_format;
	bool m_upFlow;

	void Create(Device::ITexture* texture, const Point& offset, Imaging::Bitmap* bmp)
	{
		m_targetTexture = texture;
		m_offset = offset;
		m_bmpDataIndex = AllocExtData(bmp->GetBitmapBuffer()->GetSize(), bmp->GetBitmapBuffer()->GetConstData());
		m_size = bmp->GetSize();
		m_pitch = bmp->GetPitch();
		m_format = bmp->GetPixelFormat();
		m_upFlow = bmp->IsUpFlow();
		MarkGC(m_targetTexture);
	}

	void Execute()
	{
		if (m_format == Utils::TranslatePixelFormat(m_targetTexture->GetTextureFormat()))
		{
			m_targetTexture->SetSubData(m_offset, GetExtData(m_bmpDataIndex), m_size);
		}
		else {
			LN_THROW(0, NotImplementedException);
		}
	}
};

//=============================================================================
struct ReadLockTextureCommand : public RenderingCommand
{
	Texture*	m_targetTexture;
	void Create(Texture* texture)
	{
		m_targetTexture = texture;
		MarkGC(texture);
	}
	void Execute()
	{
		m_targetTexture->m_primarySurface = m_targetTexture->m_deviceObj->Lock();
		// Texture::Lock() はこの後コマンドリストが空になるまで待機する
		// (実際のところ、このコマンドが最後のコマンドのはず)
	}
};

//=============================================================================
struct ReadUnlockTextureCommand : public RenderingCommand
{
	Texture*	m_targetTexture;
	void Create(Texture* texture)
	{
		m_targetTexture = texture;
		MarkGC(texture);
	}
	void Execute()
	{
		m_targetTexture->m_deviceObj->Unlock();
		m_targetTexture->m_primarySurface = NULL;
		// ReadLockTextureCommand と同じように、Texture::Unlock() で待機している。
		// (でも、ここまで待機することも無いかも？)
	}
};

} // namespace Graphics
} // namespace Lumino
