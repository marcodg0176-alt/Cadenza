#include "UI/GenosGeometry.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <set>
#include <iostream>

int main()
{
    using namespace cadenza::ui::genos;
    const auto controls=makeControls();
    std::set<int> notes; std::set<std::string> ids;
    for (const auto& c:controls) {
        assert(ids.insert(c.id).second);
        if(c.kind==Control::Kind::key) {
            assert(notes.insert(c.value).second);
            assert(c.black==isBlack(c.value));
        }
    }
    assert(notes.size()==76 && *notes.begin()==28 && *notes.rbegin()==103);
    Camera camera;
    for (const auto& c:controls) {
        const auto point=camera.project(c.centre,1280,600);
        assert(std::isfinite(point.x) && std::isfinite(point.y));
        assert(point.x>0 && point.x<1280 && point.y>0 && point.y<600);
    }
    std::vector<Face> faces;
    box(faces,{0,0,0},2,2,1,0xffffffff,4);
    assert(faces.size()==5 && faces.front().top);
    std::array<Point,4> polygon{};
    for (std::size_t i=0;i<4;++i) polygon[i]=camera.project(faces.front().vertices[i],1280,600);
    auto centre=camera.project({0,1,0},1280,600);
    assert(contains(polygon,centre.x,centre.y));
    assert(!contains(polygon,-100,-100));
    // Projection remains pickable at both extremes of the allowed camera orbit.
    for(float yaw:{-.48f,.48f}) for(float pitch:{.72f,1.56f}) {
        camera.yaw=yaw;camera.pitch=pitch;
        for(std::size_t i=0;i<4;++i) polygon[i]=camera.project(faces.front().vertices[i],1280,600);
        centre=camera.project({0,1,0},1280,600);
        assert(contains(polygon,centre.x,centre.y));
    }
    std::cout << "Genos model: 76 unique keys, camera projection and picking passed\n";
}
