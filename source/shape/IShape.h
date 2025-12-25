#pragma once

#include "../renderer/DrawCommandRecorder.h"


class IShape
{

public:
	const virtual void Draw(DrawCommandRecorder& recorder) = 0;

};