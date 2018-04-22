﻿#pragma once

namespace ln {
namespace detail {

class GraphicsManager
	: public RefObject
{
public:
	GraphicsManager();
	virtual ~GraphicsManager();

	void initialize();
	void dispose();
};

} // namespace detail
} // namespace ln

