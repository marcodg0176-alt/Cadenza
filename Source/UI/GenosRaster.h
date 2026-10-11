#pragma once
#include "GenosGeometry.h"
#include <limits>

namespace cadenza::ui::genos
{
// The visible pixel and its control ID share the same depth test. Picking
// therefore cannot select a button hidden behind the chassis or a black key.
struct Raster
{
    int width{}, height{};
    std::vector<std::uint32_t> pixels;
    std::vector<float> depths;
    std::vector<int> controls;
    void clear(int w, int h)
    {
        width=w; height=h;
        pixels.assign(static_cast<std::size_t>(w)*h,0);
        depths.assign(pixels.size(),-std::numeric_limits<float>::infinity());
        controls.assign(pixels.size(),-1);
    }
    void triangle(const std::array<Point,3>& p, std::uint32_t colour, int control)
    {
        const auto edge=[](Point a,Point b,float x,float y) {
            return (x-a.x)*(b.y-a.y)-(y-a.y)*(b.x-a.x);
        };
        const float area=edge(p[0],p[1],p[2].x,p[2].y);
        if(std::abs(area)<0.0001f)return;
        const int left=std::max(0,(int)std::floor(std::min({p[0].x,p[1].x,p[2].x})));
        const int right=std::min(width-1,(int)std::ceil(std::max({p[0].x,p[1].x,p[2].x})));
        const int top=std::max(0,(int)std::floor(std::min({p[0].y,p[1].y,p[2].y})));
        const int bottom=std::min(height-1,(int)std::ceil(std::max({p[0].y,p[1].y,p[2].y})));
        for(int y=top;y<=bottom;++y)for(int x=left;x<=right;++x) {
            const float a=edge(p[1],p[2],x+.5f,y+.5f)/area;
            const float b=edge(p[2],p[0],x+.5f,y+.5f)/area;
            const float c=1-a-b;
            if(a<-.00001f||b<-.00001f||c<-.00001f)continue;
            const float depth=a*p[0].depth+b*p[1].depth+c*p[2].depth;
            const auto i=static_cast<std::size_t>(y)*width+x;
            if(depth<=depths[i])continue;
            depths[i]=depth;pixels[i]=colour;controls[i]=control;
        }
    }
    int at(int x,int y) const
    {
        return x>=0&&x<width&&y>=0&&y<height?controls[static_cast<std::size_t>(y)*width+x]:-1;
    }
};
}
