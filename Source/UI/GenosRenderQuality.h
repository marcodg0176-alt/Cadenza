#pragma once
#include <algorithm>
#include <cmath>

namespace cadenza::ui::genos
{
struct RenderSize { int width{}, height{}; };

// Render real geometry above the display resolution, then filter it down.
// The 8K envelope bounds the colour, depth and picking buffers together.
inline RenderSize renderSize(int logicalWidth, int logicalHeight, float displayScale=1.0f)
{
    if(logicalWidth<=0 || logicalHeight<=0)return {};
    if(!std::isfinite(displayScale) || displayScale<=0)displayScale=1.0f;
    const double scale=std::min(
        std::max(2.0*displayScale,3840.0/logicalWidth),
        std::min(7680.0/logicalWidth,4320.0/logicalHeight));
    return {std::max(1,(int)std::lround(logicalWidth*scale)),
            std::max(1,(int)std::lround(logicalHeight*scale))};
}
}
