﻿
#include "Internal.hpp"
#include <LuminoEngine/Audio/AudioContext.hpp>
#include <LuminoEngine/Audio/AudioProcessorNode.hpp>
#include "CAProcessorNode.hpp"
#include "AudioManager.hpp"

namespace ln {

//==============================================================================
// AudioProcessorNode

AudioProcessorNode::AudioProcessorNode()
{
}

AudioProcessorNode::~AudioProcessorNode()
{
}

void AudioProcessorNode::initialize(int numberOfInputChannels, int numberOfOutputChannels)
{
    // TODO: AudioNode::initialize() の後にしたい。
    // detail::EngineDomain::audioManager()->primaryContext() じゃなくて context() にしたい
    m_coreObject = makeRef<detail::CAProcessorNode>(detail::EngineDomain::audioManager()->primaryContext()->coreObject(), this);
    m_coreObject->initialize(numberOfInputChannels, numberOfOutputChannels);

	AudioNode::initialize();
}

detail::CoreAudioNode* AudioProcessorNode::coreNode()
{
	return m_coreObject;
}

void AudioProcessorNode::commit()
{
    AudioNode::commit();

    detail::ScopedReadLock lock(propertyMutex());
    //m_coreObject->setGain(m_gain);
}

void AudioProcessorNode::onAudioProcess(AudioBus* input, AudioBus* output)
{
}

} // namespace ln
