#pragma once
#include "core/renderer/DrawCommandRecorder.h"

class IShape
{

public:
	const virtual void Draw(Core::DrawCommandRecorder& recorder) = 0;

};