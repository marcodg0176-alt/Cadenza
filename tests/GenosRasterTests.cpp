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
    std::cout<<"Genos depth: visible surface and picking agree, including intersections\n";
}
