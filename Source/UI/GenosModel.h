#pragma once
#include "GenosGeometry.h"
#include "GenosModelData.h"
#include <juce_core/juce_core.h>
#include <unordered_map>

namespace cadenza::ui::genos
{
struct ModelTriangle { std::array<Vec3,3> vertices; std::uint32_t colour{}; int control=-1; bool moving=true; };
struct ModelLabel { juce::String text; Vec3 position; float size{}; std::uint32_t colour{}; };
struct ModelScreen { juce::String id; Vec3 centre; float width{}, depth{}; };
struct Model
{
    std::vector<Control> controls;
    std::vector<ModelTriangle> triangles;
    std::vector<ModelLabel> labels;
    std::vector<ModelScreen> screens;
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
            ids.emplace(c.id,(int)result.controls.size());result.controls.push_back(std::move(c));
        }
        if(auto* array=json["faces"].getArray())for(const auto& item:*array) {
            ModelTriangle face;
            for(int i=0;i<3;++i)face.vertices[(std::size_t)i]=vector(item["vertices"][i]);
            face.colour=(std::uint32_t)(juce::int64)item["colour"];
            face.moving=item.hasProperty("moving")?(bool)item["moving"]:true;
            const auto found=ids.find(item["control"].toString().toStdString());
            if(found!=ids.end())face.control=found->second;
            result.triangles.push_back(face);
        }
        if(auto* array=json["labels"].getArray())for(const auto& item:*array)
            result.labels.push_back({item["text"].toString(),vector(item["position"]),
                (float)item["size"],(std::uint32_t)(juce::int64)item["colour"]});
        if(auto* array=json["screens"].getArray())for(const auto& item:*array)
            result.screens.push_back({item["id"].toString(),vector(item["centre"]),
                                      (float)item["width"],(float)item["depth"]});
        jassert(result.controls.size()>=76 && result.triangles.size()>1000);
        return result;
    }
};
}
