#pragma once
#include "GenosGeometry.h"
#include "GenosRaster.h"
#include "GenosModelData.h"
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <unordered_map>
#include <cstring>

namespace cadenza::ui::genos
{
struct ModelTriangle {
    std::array<Vec3,3> vertices;
    std::array<Vec2,3> uv{};
    std::uint32_t colour{};
    int control=-1, texture=-1;
    bool moving=true;
};
struct ModelLabel { juce::String text; Vec3 position; float size{}; std::uint32_t colour{}; };
struct ModelScreen { juce::String id; Vec3 centre; float width{}, depth{}; std::array<Vec3,4> corners{}; bool hasCorners=false; };
struct Model
{
    std::vector<Control> controls;
    std::vector<ModelTriangle> triangles;
    std::vector<ModelLabel> labels;
    std::vector<ModelScreen> screens;
    std::vector<RasterTexture> textures;
    static RasterTexture textureFromImage(const juce::Image& image)
    {
        RasterTexture texture;
        if(!image.isValid())return texture;
        texture.width=image.getWidth();texture.height=image.getHeight();
        texture.bilinear=true;
        texture.pixels.resize(static_cast<std::size_t>(texture.width)*texture.height);
        const juce::Image::BitmapData data(image,juce::Image::BitmapData::readOnly);
        for(int y=0;y<texture.height;++y)for(int x=0;x<texture.width;++x)
            texture.pixels[static_cast<std::size_t>(y)*texture.width+x]=data.getPixelColour(x,y).getARGB();
        return texture;
    }
    static Model load()
    {
        Model result;
        const auto json=juce::JSON::parse(juce::String::fromUTF8(GenosModelData::geometry_json,
                                                              GenosModelData::geometry_jsonSize));
        auto vector=[](const juce::var& a) {
            return Vec3{(float)a[0],(float)a[1],(float)a[2]};
        };
        std::unordered_map<std::string,int> ids;
        if(auto* array=json["controls"].getArray())for(const auto& item:*array) {
            const auto kind=item["kind"].toString();
            Control c;
            c.kind=kind=="key"?Control::Kind::key:kind=="fader"?Control::Kind::fader:
                   kind=="knob"?Control::Kind::knob:kind=="joystick"?Control::Kind::joystick:Control::Kind::button;
            c.id=item["id"].toString().toStdString();c.label=item["label"].toString().toStdString();
            c.hint=item["hint"].toString().toStdString();c.centre=vector(item["centre"]);
            c.width=(float)item["width"];c.depth=(float)item["depth"];c.height=(float)item["height"];
            c.value=(int)item["value"];c.black=(bool)item["black"];
            c.hasHitPoint=item.hasProperty("hitPoint");if(c.hasHitPoint)c.hitPoint=vector(item["hitPoint"]);
            c.hasTravel=item.hasProperty("travel");if(c.hasTravel)c.travel=vector(item["travel"]);
            if(item.hasProperty("neutralValue"))c.neutralValue=(float)item["neutralValue"];
            ids.emplace(c.id,(int)result.controls.size());result.controls.push_back(std::move(c));
        }
        if(auto* array=json["faces"].getArray())for(const auto& item:*array) {
            ModelTriangle face;
            for(int i=0;i<3;++i)face.vertices[(std::size_t)i]=vector(item["vertices"][i]);
            face.colour=(std::uint32_t)(juce::int64)item["colour"];
            face.moving=item.hasProperty("moving")?(bool)item["moving"]:true;
            if(item.hasProperty("texture"))face.texture=(int)item["texture"];
            if(item.hasProperty("uv"))for(int i=0;i<3;++i)
                face.uv[(std::size_t)i]={(float)item["uv"][i][0],(float)item["uv"][i][1]};
            const auto found=ids.find(item["control"].toString().toStdString());
            if(found!=ids.end())face.control=found->second;
            result.triangles.push_back(face);
        }
        if(json.hasProperty("triangleResource")) {
            int size=0;
            const auto* bytes=GenosModelData::getNamedResource(json["triangleResource"].toString().toRawUTF8(),size);
            if(bytes && size>=12 && std::memcmp(bytes,"GNOS2M01",8)==0) {
                juce::MemoryInputStream stream(bytes,(std::size_t)size,false);
                stream.setPosition(8);
                const auto count=static_cast<std::uint32_t>(stream.readInt());
                if(count<=2000000 && static_cast<std::uint64_t>(count)*76+12==(std::uint64_t)size) {
                    result.triangles.reserve(count);
                    for(std::uint32_t i=0;i<count;++i) {
                        ModelTriangle face;
                        for(auto& p:face.vertices)p={stream.readFloat(),stream.readFloat(),stream.readFloat()};
                        for(auto& p:face.uv)p={stream.readFloat(),stream.readFloat()};
                        face.colour=static_cast<std::uint32_t>(stream.readInt());
                        face.control=stream.readInt();face.texture=stream.readInt();
                        face.moving=(stream.readInt()&1)!=0;
                        if(face.control<0 || face.control>=(int)result.controls.size())face.control=-1;
                        result.triangles.push_back(face);
                    }
                }
            }
        }
        if(auto* array=json["labels"].getArray())for(const auto& item:*array)
            result.labels.push_back({item["text"].toString(),vector(item["position"]),
                (float)item["size"],(std::uint32_t)(juce::int64)item["colour"]});
        if(auto* array=json["screens"].getArray())for(const auto& item:*array) {
            ModelScreen screen{item["id"].toString(),vector(item["centre"]),
                               (float)item["width"],(float)item["depth"]};
            screen.hasCorners=item.hasProperty("corners");
            if(screen.hasCorners)for(int i=0;i<4;++i)screen.corners[(std::size_t)i]=vector(item["corners"][i]);
            result.screens.push_back(screen);
        }
        if(auto* array=json["textures"].getArray())for(const auto& item:*array) {
            int size=0;
            auto* bytes=GenosModelData::getNamedResource(item["resource"].toString().toRawUTF8(),size);
            const auto image=bytes && size>0?juce::ImageFileFormat::loadFrom(bytes,(std::size_t)size):juce::Image{};
            result.textures.push_back(textureFromImage(image));
            jassert(result.textures.back().valid());
        }
        jassert(result.controls.size()>=76 && result.triangles.size()>1000);
        return result;
    }
};
}
