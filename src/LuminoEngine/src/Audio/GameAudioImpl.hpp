﻿#pragma once

namespace ln {
namespace detail {
class AudioManager;

class GameAudioImpl
	: public RefObject
{
public:
    GameAudioImpl(AudioManager* mamager);
	virtual ~GameAudioImpl();

    void playBGM(const StringRef& filePath, float volume, float pitch, double fadeTime);
    void stopBGM(double fadeTime);
    void playBGS(const StringRef& filePath, float volume, float pitch, double fadeTime);
    void stopBGS(double fadeTime);
    void playME(const StringRef& filePath, float volume, float pitch);
    void stopME();
    void playSE(const StringRef& filePath, float volume, float pitch);
    void playSE3D(const StringRef& filePath, const Vector3& position, float distance, float volume, float pitch);
    void stopSE();
    void setMEFadeState(double begin, double end);
    void setBGMVolume(float volume, double fadeTime);
    void setBGSVolume(float volume, double fadeTime);
    void setEnableBGMRestart(bool enabled);
    void setEnableBGSRestart(bool enabled);

private:
};

} // namespace detail
} // namespace ln
