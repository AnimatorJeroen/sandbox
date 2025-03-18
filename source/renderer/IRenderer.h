#pragma once

#include "BezierRenderPath.h"

class IRenderer
{
	virtual void DrawPath(const BezierRenderPath* path) = 0;

};