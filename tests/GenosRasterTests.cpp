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
    const RasterTexture quadrants{2,2,{0xffff0000,0xff00ff00,0xff0000ff,0xffffff00}};
    auto pixelAt=[&](int x,int y) { return image.pixels[(std::size_t)y*image.width+x]; };
    auto checkPixel=[&](int x,int y,std::uint32_t colour,int id,const char* message) {
        if(pixelAt(x,y)!=colour || image.at(x,y)!=id)throw std::runtime_error(message);
    };
    auto texturedQuad=[&](float depth,std::uint32_t colour,int id,const RasterTexture* texture) {
        image.triangle({{{0,0,depth},{8,0,depth},{8,8,depth}}},colour,id,texture,
                       {{{0,0},{1,0},{1,1}}});
        // Reverse winding on the other half, with UVs following their vertices.
        image.triangle({{{0,8,depth},{8,8,depth},{0,0,depth}}},colour,id,texture,
                       {{{0,1},{1,1},{0,0}}});
    };
    image.clear(8,8);texturedQuad(1,0xffffffff,7,&quadrants);
    checkPixel(1,1,0xffff0000,7,"UV origin did not sample the image top-left");
    checkPixel(6,1,0xff00ff00,7,"increasing U did not sample image right");
    checkPixel(1,6,0xff0000ff,7,"increasing V did not sample image down");
    checkPixel(6,6,0xffffff00,7,"UV bottom-right or reversed winding was incorrect");
    for(int y=0;y<8;++y)for(int x=0;x<8;++x) {
        const auto expected=quadrants.pixels[(y>=4?2:0)+(x>=4?1:0)];
        checkPixel(x,y,expected,7,"textured quad left a seam or interpolated the wrong texel");
    }
    auto filtered=quadrants;filtered.bilinear=true;
    const std::array<Point,3> filteredPatch{{{0,0,1},{8,0,1},{0,8,1}}};
    image.clear(8,8);
    image.triangle(filteredPatch,0xffffffff,21,&filtered,{{{.5f,.5f},{.5f,.5f},{.5f,.5f}}});
    checkPixel(1,1,0xff808040,21,"bilinear centre did not average and round the four source texels");
    image.clear(8,8);
    image.triangle(filteredPatch,0xffffffff,22,&filtered,{{{-1,-1},{-1,-1},{-1,-1}}});
    checkPixel(1,1,0xffff0000,22,"bilinear filter failed to clamp at image top-left edge");
    image.clear(8,8);
    image.triangle(filteredPatch,0xffffffff,23,&filtered,{{{2,2},{2,2},{2,2}}});
    checkPixel(1,1,0xffffff00,23,"bilinear filter failed to clamp at image bottom-right edge");

    // Texture colour modulates material RGB; it does not replace the material.
    const RasterTexture pigment{1,1,{0xff80c040}};
    image.clear(8,8);texturedQuad(1,0xff804020,8,&pigment);
    checkPixel(1,1,0xff403008,8,"texture/material RGB modulation was incorrect");

    const std::array<Point,3> patch{{{0,0,1},{8,0,1},{0,8,1}}};
    image.clear(8,8);
    image.triangle(patch,0xffffffff,9,&quadrants,{{{-5,-5},{-5,-5},{-5,-5}}});
    checkPixel(1,1,0xffff0000,9,"negative UV did not clamp to first texel");
    image.clear(8,8);
    image.triangle(patch,0xffffffff,10,&quadrants,{{{5,5},{5,5},{5,5}}});
    checkPixel(1,1,0xffffff00,10,"UV greater than one did not clamp to last texel");
    image.clear(8,8);
    image.triangle(patch,0xffffffff,11,&quadrants,{{{1,1},{1,1},{1,1}}});
    checkPixel(1,1,0xffffff00,11,"UV exactly one read past the image boundary");

    image.clear(8,8);texturedQuad(2,0xffffffff,12,&quadrants);
    texturedQuad(3,0xff224466,13,nullptr);
    checkPixel(1,1,0xff224466,13,"closer untextured chassis failed to occlude textured screen");
    texturedQuad(1,0xffffffff,14,&quadrants);
    checkPixel(1,1,0xff224466,13,"far texture replaced visible colour or control ID");
    texturedQuad(4,0xffffffff,15,&quadrants);
    checkPixel(1,1,0xffff0000,15,"closer textured command failed to update colour and picking together");

    // Invalid texture metadata/data and non-finite UVs use the untextured path.
    const std::array<RasterTexture,5> invalidTextures{{
        {0,2,{}},{2,0,{}},{2,2,{}},{2,2,{0xffffffff}},
        {1,1,{0xffffffff,0xffffffff}}
    }};
    for(const auto& invalid:invalidTextures) {
        image.clear(8,8);texturedQuad(1,0xff123456,16,&invalid);
        checkPixel(1,1,0xff123456,16,"invalid texture did not fall back safely to material colour");
    }
    for(float invalid:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()}) {
        image.clear(8,8);
        image.triangle(patch,0xff123456,17,&quadrants,{{{invalid,0},{1,0},{0,1}}});
        checkPixel(1,1,0xff123456,17,"non-finite UV did not fall back safely");
    }
    const RasterTexture transparent{1,1,{0x00112233}};
    image.clear(8,8);texturedQuad(1,0xff123456,18,nullptr);
    texturedQuad(5,0xffffffff,19,&transparent);
    checkPixel(1,1,0xff123456,18,"transparent texel blocked visible surface or picking");
    texturedQuad(2,0xff654321,20,nullptr);
    checkPixel(1,1,0xff654321,20,"transparent texel wrote depth and occluded a later visible surface");
    std::cout<<"Genos raster: depth, picking, clipping, UV orientation, material modulation and safe texture fallback passed\n";
}
