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
        for(const auto& point:p)
            if(!std::isfinite(point.x)||!std::isfinite(point.y)||!std::isfinite(point.depth))return;
        const auto edge=[](Point a,Point b,float x,float y) {
            return (x-a.x)*(b.y-a.y)-(y-a.y)*(b.x-a.x);
        };
        const float area=edge(p[0],p[1],p[2].x,p[2].y);
        if(!std::isfinite(area)||std::abs(area)<0.0001f)return;
        const float inverseArea=1.0f/area;
        const int left=std::max(0,(int)std::floor(std::min({p[0].x,p[1].x,p[2].x})));
        const int right=std::min(width-1,(int)std::ceil(std::max({p[0].x,p[1].x,p[2].x})));
        const float minimumY=std::min({p[0].y,p[1].y,p[2].y});
        const float maximumY=std::max({p[0].y,p[1].y,p[2].y});
        const float edgeTolerance=(maximumY-minimumY)*.00002f+.000001f;
        const int top=std::max(0,(int)std::floor(minimumY));
        const int bottom=std::min(height-1,(int)std::ceil(maximumY));
        for(int y=top;y<=bottom;++y) {
            // Restrict work to the triangle's span on this row. Large rounded
            // chassis triangles otherwise test millions of empty pixels.
            float spanLeft=std::numeric_limits<float>::infinity(),spanRight=-spanLeft;
            for(int i=0;i<3;++i) {
                const auto& a=p[(std::size_t)i];const auto& b=p[(std::size_t)((i+1)%3)];
                const float dy=b.y-a.y;
                if(std::abs(dy)<.000001f)continue;
                const float t=(y+.5f-a.y)/dy;
                if(t<0||t>1)continue;
                const float x=a.x+t*(b.x-a.x);
                spanLeft=std::min(spanLeft,x);spanRight=std::max(spanRight,x);
            }
            // At a subpixel horizontal edge the barycentric tolerance can
            // accept a pixel just outside the exact span. Preserve that case.
            if(!std::isfinite(spanLeft) && (y+.5f<minimumY-edgeTolerance || y+.5f>maximumY+edgeTolerance))continue;
            const int rowLeft=std::isfinite(spanLeft)?std::max(left,(int)std::floor(spanLeft)-1):left;
            const int rowRight=std::isfinite(spanRight)?std::min(right,(int)std::ceil(spanRight)+1):right;
            for(int x=rowLeft;x<=rowRight;++x) {
            const float a=edge(p[1],p[2],x+.5f,y+.5f)*inverseArea;
            const float b=edge(p[2],p[0],x+.5f,y+.5f)*inverseArea;
            const float c=1-a-b;
            if(a<-.00001f||b<-.00001f||c<-.00001f)continue;
            const float depth=a*p[0].depth+b*p[1].depth+c*p[2].depth;
            const auto i=static_cast<std::size_t>(y)*width+x;
            if(depth<=depths[i])continue;
            depths[i]=depth;pixels[i]=colour;controls[i]=control;
            }
        }
    }
    int at(int x,int y) const
    {
        return x>=0&&x<width&&y>=0&&y<height?controls[static_cast<std::size_t>(y)*width+x]:-1;
    }
};
}
