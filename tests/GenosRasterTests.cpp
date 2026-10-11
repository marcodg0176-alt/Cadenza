#include "UI/GenosRaster.h"
#include <stdexcept>
#include <iostream>
int main()
{
    using namespace cadenza::ui::genos;
    Raster image;image.clear(32,32);
    const std::array<Point,3> chassis{{{0,0,0},{31,0,0},{0,31,0}}};
    const std::array<Point,3> button{{{5,5,2},{20,5,2},{5,20,2}}};
    image.triangle(button,0xff0000ff,7);
    image.triangle(chassis,0xff222222,-1);
    if(image.at(8,8)!=7||image.pixels[8*32+8]!=0xff0000ff)
        throw std::runtime_error("chassis overwrote closer command");
    const std::array<Point,3> key{{{5,5,3},{20,5,3},{5,20,3}}};
    image.triangle(key,0xffffffff,28);
    image.triangle(button,0xff0000ff,7);
    if(image.at(8,8)!=28)throw std::runtime_error("hidden command selected through closer key");
    // A large sloped face has nearer and farther pixels. Average-depth sorting
    // would produce one wrong result regardless of which triangle came last.
    image.clear(32,32);
    const std::array<Point,3> slope{{{0,0,-2},{31,0,4},{0,31,-2}}};
    image.triangle(chassis,0xff222222,1);
    image.triangle(slope,0xffeeeeee,2);
    if(image.at(2,2)!=1||image.at(20,2)!=2||image.at(-1,2)!=-1)
        throw std::runtime_error("per-pixel sloped surface depth or bounds incorrect");
    image.triangle({{{1,1,8},{1,1,8},{1,1,8}}},0xffffffff,99);
    if(image.at(2,2)!=1)throw std::runtime_error("degenerate triangle changed picking");
    image.clear(32,32);
    // A clipped quad made of opposite windings must cover every visible pixel,
    // including its shared diagonal and its horizontal top/bottom edges.
    image.triangle({{{-8,-8,1},{40,-8,1},{40,40,1}}},0xffffffff,4);
    image.triangle({{{-8,40,1},{40,40,1},{-8,-8,1}}},0xffffffff,5);
    for(int y=0;y<32;++y)for(int x=0;x<32;++x)
        if(image.at(x,y)<0||image.pixels[(std::size_t)y*32+x]!=0xffffffff)
            throw std::runtime_error("scanline clipping or winding left a visible seam");
    image.clear(32,32);
    image.triangle({{{-8,.499999f,1},{40,.499999f,1},{0,-8,1}}},0xffffffff,7);
    for(int x=0;x<32;++x)
        if(image.at(x,0)!=7)throw std::runtime_error("subpixel horizontal edge lost its tolerance coverage");
    const auto before=image.pixels;
    image.triangle({{{0,0,1},{31,std::numeric_limits<float>::quiet_NaN(),1},{0,31,1}}},0xff000000,9);
    if(image.pixels!=before)throw std::runtime_error("non-finite triangle affected the frame");
    std::cout<<"Genos depth: visible surface and picking agree, including intersections\n";
}
