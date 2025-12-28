#pragma once
#include "core/renderer/DrawCommandRecorder.h"
#include <cereal/archives/binary.hpp>

class IShape
{
public:
	const virtual void Draw(Core::DrawCommandRecorder& recorder) = 0;
	virtual ~IShape() = default;

};