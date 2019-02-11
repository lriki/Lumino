﻿#pragma once

namespace ln {
class DiagnosticsManager;

namespace detail {

enum class PCMFormat
{
	U8,
	S8,
	S16L,	// L = little endian
	S24L,
	S32L,
	S64L,
	Float,
	Double,
};

struct AudioDataInfo
{
	int channelCount;
	int sampleRate;
	double totalSeconds;
	uint32_t frameSize; // samples の、1フレームの要素数 (channelCount * byteParSample)
    uint32_t totalSamples;	// 全サンプル数。frameCount * channelCount
	//std::vector<float> samples;
	PCMFormat sourceFormat;
    uint32_t byteParSample;

    uint32_t totalFrameCount() const { return totalSamples / channelCount; }
};

// TODO: できるだけキャッシュして共有して使いたい。シークは read でやる。できるだけmutable な状態を持たない。
//
// リサンプリングはされないので注意。
// 例えば以下のようなフレーム数となる。
// - 3s,22050Hz=66150
// - 3s,32000Hz=96000
class AudioDecoder
	: public RefObject
{
public:
	virtual ~AudioDecoder() = default;
	virtual const AudioDataInfo& audioDataInfo() const = 0;
	virtual void seekToFrame(size_t frameNumber) = 0;
    // チャンネル数2の時に frames=1 で呼び出すと、2サンプル取り出す。ret は読んだフレーム数
	virtual uint32_t read2(float* buffer, uint32_t requestFrames) = 0;
	virtual void reset() = 0;

	static void convertToFloat32(float* dst, const void* src, const size_t frameLength, PCMFormat format);
	static void convertFromFloat32(void * dst, const float * src, const size_t frameLength, PCMFormat format);

	//virtual void getLoopState(uint32_t* begin, uint32_t* length) const = 0;
private:
};

class WaveDecoder
	: public AudioDecoder
{
public:
	virtual ~WaveDecoder() = default;
    bool init(Stream* stream, DiagnosticsManager* diag);
	virtual const AudioDataInfo& audioDataInfo() const override;
	//virtual void seekToSample(uint32_t sampleNumber) override;
    virtual void seekToFrame(size_t frameNumber) override;
	virtual uint32_t read2(float* buffer, uint32_t requestFrames) override;
	virtual void reset() override;

    std::vector<float> readAllSamples();

private:
	Ref<Stream> m_stream;
	AudioDataInfo m_info;
	size_t m_pcmDataOffset;
	size_t m_pcmDataLength;
    size_t m_pcmDataPos;
	size_t m_bitDepth;
	size_t m_seekSample;
	std::vector<byte_t> m_workBuffer;
};

} // namespace detail
} // namespace ln

