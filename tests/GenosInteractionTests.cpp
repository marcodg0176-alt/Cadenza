#include "UI/GenosSurface.h"
#include "UI/CadenzaLookAndFeel.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {
void expect(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
juce::MouseEvent event(juce::Component& view, juce::Point<float> p,
                       juce::Point<float> down, bool held, bool dragged = false) {
    auto now = juce::Time::getCurrentTime();
    return {juce::Desktop::getInstance().getMainMouseSource(), p,
            juce::ModifierKeys(held ? juce::ModifierKeys::leftButtonModifier : 0),
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, &view, &view, now, down, now, 1, dragged};
}
}
int runTests() {
    juce::ScopedJuceInitialiser_GUI initialise;
    cadenza::ui::CadenzaLookAndFeel theme;
    const auto modelStart=std::chrono::steady_clock::now();
    const auto model=cadenza::ui::genos::Model::load();
    std::cout<<"Reference model load: "
             <<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-modelStart).count()
             <<" ms, "<<model.triangles.size()<<" triangles, "<<model.controls.size()<<" controls\n";
    expect(model.triangles.size()>100000,"final Blender mesh loaded, rather than prior simplified geometry");
    expect(model.textures.size()==2,"both source texture resources loaded");
    for(const auto& texture:model.textures)
        expect(texture.valid()&&texture.width==2000&&texture.height==2000,"source texture decoded at its original resolution");
    std::set<std::string> controlIds;
    std::set<int> modelNotes;
    int modelButtons=0,modelFaders=0,modelKnobs=0,modelJoysticks=0;
    for(const auto& c:model.controls) {
        expect(controlIds.insert(c.id).second,"unique imported command ID: "+c.id);
        expect(c.hasHitPoint,"imported command has an explicit picking point: "+c.id);
        if(c.kind==cadenza::ui::genos::Control::Kind::key) {
            expect(modelNotes.insert(c.value).second,"unique imported MIDI pitch");
            expect(c.black==cadenza::ui::genos::isBlack(c.value),"imported black/white key matches MIDI pitch");
        }
        if(c.kind==cadenza::ui::genos::Control::Kind::button)++modelButtons;
        if(c.kind==cadenza::ui::genos::Control::Kind::fader)++modelFaders;
        if(c.kind==cadenza::ui::genos::Control::Kind::knob)++modelKnobs;
        if(c.kind==cadenza::ui::genos::Control::Kind::joystick)++modelJoysticks;
    }
    expect(modelNotes.size()==76&&*modelNotes.begin()==28&&*modelNotes.rbegin()==103,"final model preserves all 76 notes E1-G7");
    expect(modelButtons==29&&modelFaders==9&&modelKnobs==7&&modelJoysticks==1,"final model has only its confirmed physical command mapping");
    std::set<std::string> expectedIds{
        "style","ots0","ots1","ots2","ots3","left","right0","right1","right2","rec",
        "acmp","autofill","break","endingA","endingB","endingC","introA","introB","introC",
        "mainA","mainB","mainC","mainD","syncstop","play","tempo-","tempo+","transpose-","transpose+",
        "master","pitch"
    };
    for(int n=28;n<=103;++n)expectedIds.insert("note"+std::to_string(n));
    for(int i=0;i<9;++i)expectedIds.insert("fader"+std::to_string(i));
    for(int i=0;i<6;++i)expectedIds.insert("knob"+std::to_string(i));
    expect(controlIds==expectedIds,"export binds exactly the confirmed physical controls without fictitious commands");
    for(const char* id:{"creator","midi","audio","native","import","soundfont","save","fill","memory"})
        expect(controlIds.count(id)==0,"legitimate software-only action remains outside physical mesh: "+std::string(id));
    const auto metadata=juce::JSON::parse(juce::String::fromUTF8(GenosModelData::geometry_json,GenosModelData::geometry_jsonSize));
    expect(metadata["sourceSha256"].toString().equalsIgnoreCase("09D140A95059B1A05504D4BD60E96ECD581D2854F34729947407095E938ED553"),
           "embedded export comes from the user's final Yamaha_Genos2.blend source");
    expect((int)metadata["triangleCount"]==(int)model.triangles.size(),"binary mesh loader preserves exported triangle count");
    std::set<std::string> omittedLcdMeshes;
    if(const auto* omitted=metadata["omittedRuntimeMeshes"].getArray())
        for(const auto& name:*omitted)omittedLcdMeshes.insert(name.toString().toStdString());
    expect(omittedLcdMeshes==std::set<std::string>({"Dalle LCD interface originale","LIVE CONTROL VIEW ecran secondaire"}),
           "both static reference LCD artworks are explicitly omitted from the runtime export");
    bool inclinedMainScreen=false,texturedFaces=false;
    for(const auto& screen:model.screens) {
        expect(screen.hasCorners,"actual screen exports four oriented corners");
        for(const auto& p:screen.corners)
            expect(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z),"finite imported screen corner");
        if(screen.id=="main")inclinedMainScreen=std::abs(screen.corners[0].y-screen.corners[3].y)>.0001f;
    }
    expect(inclinedMainScreen,"main display uses the source model's inclined plane");
    for(const auto& face:model.triangles) {
        expect(face.control>=-1&&face.control<(int)model.controls.size(),"binary triangle has a valid command binding");
        expect(face.texture>=-1&&face.texture<(int)model.textures.size(),"binary triangle has a valid texture binding");
        if(face.texture>=0)texturedFaces=true;
        for(const auto& p:face.vertices)
            expect(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z),"finite imported vertex");
        for(const auto& uv:face.uv)
            expect(std::isfinite(uv.x)&&std::isfinite(uv.y),"finite imported UV");
    }
    expect(texturedFaces,"actual mesh uses the imported texture artwork");
    struct Note { int pitch, velocity; bool on; };
    std::vector<Note> notes;
    std::vector<std::string> sections;
    std::vector<std::pair<int,int>> mixer;
    std::vector<int> master, reverb, compressor, pitch, modulation;
    std::vector<std::array<int,3>> eq;
    int openStyle=0, newSession=0, makeEditable=0, editor=0, page=-1, play=0;
    cadenza::ui::NativePanel::Callbacks cb;
    cb.onKeyboardNote=[&](int n,int v,bool on){notes.push_back({n,v,on});};
    cb.openStyle=[&]{++openStyle;}; cb.togglePlay=[&]{++play;};
    cb.selectSection=[&](const std::string& id){sections.push_back(id);};
    cb.onMixerVolume=[&](int ch,int v){mixer.emplace_back(ch,v);};
    cb.onMasterChanged=[&](int v){master.push_back(v);};
    cb.onReverbChanged=[&](int v){reverb.push_back(v);};
    cb.onCompChanged=[&](int v){compressor.push_back(v);};
    cb.onEqChanged=[&](int l,int m,int h){eq.push_back({l,m,h});};
    cb.onPitchBend=[&](int v){pitch.push_back(v);};
    cb.onModulation=[&](int v){modulation.push_back(v);};
    cb.onRecNew=[&](int bars){expect(bars==4,"new style has four bars");++newSession;};
    cb.onMakeEditable=[&]{++makeEditable;}; cb.onRecEdit=[&]{++editor;};
    const auto surfaceStart=std::chrono::steady_clock::now();
    cadenza::ui::GenosSurface view(cb,[&](int p){page=p;});
    std::cout<<"Genos surface construction: "
             <<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-surfaceStart).count()<<" ms\n";
    view.setLookAndFeel(&theme); view.setSize(1280,800); view.setVisible(true);
    bool frenchTooltip=false;
    for(int i=0;i<view.getNumChildComponents();++i)
        if(auto* b=dynamic_cast<juce::TextButton*>(view.getChildComponent(i));b && b->getTooltip().contains("Espace")) {
            frenchTooltip=true;
            expect(b->getTooltip().containsChar(0x00c9)&&b->getTooltip().containsChar(0x00ea)
                   &&!b->getTooltip().containsChar(0x00c3),"French tooltip uses Unicode accents, not misdecoded UTF-8 bytes");
        }
    expect(frenchTooltip,"French keyboard help is available");
    auto toolbar=[&](const juce::String& text) {
        for(int i=0;i<view.getNumChildComponents();++i)
            if(auto* b=dynamic_cast<juce::TextButton*>(view.getChildComponent(i));b && b->getButtonText()==text) {
                expect(bool(b->onClick),"toolbar callback exists"); b->onClick(); return;
            }
        throw std::runtime_error("missing accessible toolbar: "+text.toStdString());
    };
    auto centre=[&](const std::string& id) {
        auto p=view.controlScreenCentre(id);
        expect(std::isfinite(p.x)&&std::isfinite(p.y),"finite control position: "+id);
        expect(p.x>=0&&p.x<view.getWidth()&&p.y>=0&&p.y<view.getHeight(),"within viewport: "+id);
        expect(view.controlIdAt(p)==id,"mesh and picking agree: "+id); return p;
    };
    auto down=[&](juce::Point<float> p){view.mouseDown(event(view,p,p,true));};
    auto up=[&](juce::Point<float> p){view.mouseUp(event(view,p,p,false));};
    auto click=[&](const std::string& id){auto p=centre(id);down(p);up(p);};
    auto drag=[&](const std::string& id,juce::Point<float> d){auto p=centre(id);down(p);view.mouseDrag(event(view,p+d,p,true,true));up(p+d);};
    auto runtime=[&](std::vector<std::string> available,const char* chord="C#m") {
        view.setRuntime(juce::String::fromUTF8("Été à Montréal — Répétition"),chord,"mainA",110,false,
            available,{104,86,101,87,80,86,78,78,100},true,true,true,false,0,
            false,false,{}, {true,false,false},false,30,15,{0,0,0});
    };
    auto picking=[&] {
        int keys=0;
        for(const auto& c:model.controls) {
            centre(c.id); if(c.kind==cadenza::ui::genos::Control::Kind::key)++keys;
        }
        expect(keys==76,"all 76 physical keys independently pickable");
    };
    auto note=[&](std::size_t i,int n,bool on) {
        expect(i<notes.size(),"note callback present");
        expect(notes[i].pitch==n&&notes[i].on==on,"correct pitch and note direction");
        expect(notes[i].velocity==(on?100:0),"correct attack/release velocity");
    };
    expect(view.keyPressed(juce::KeyPress('Q',{},'q')),"melody key handled");
    expect(notes.size()==1,"melody reaches host once");note(0,60,true);
    view.keyPressed(juce::KeyPress('Q',{},'q'));expect(notes.size()==1,"repeat does not retrigger");
    view.focusLost(juce::Component::focusChangedDirectly);expect(notes.size()==2,"focus releases note");note(1,60,false);
    view.keyPressed(juce::KeyPress('W',{},'w'));note(2,48,true);
    view.setVisible(false);note(3,48,false);
    auto count=notes.size();
    expect(!view.keyPressed(juce::KeyPress('S',juce::ModifierKeys::ctrlModifier,'s')),"modifier shortcuts excluded");
    expect(notes.size()==count,"shortcut produces no note");view.setVisible(true);
    toolbar("Ouvrir un style");expect(openStyle==1,"open style toolbar reaches host");
    toolbar("Style Creator");expect(newSession==1&&editor==1&&page==7,"empty creator opens real editor/session");
    const std::vector<std::string> initialSections{"mainA","mainB","mainC","mainD","intro","ending","fill"};
    runtime(initialSections);picking();
    click("style");expect(openStyle==2,"physical STYLE gateway reaches the existing style picker callback");
    auto lcdSnapshot=[&] {
        juce::Image image(juce::Image::ARGB,view.getWidth(),view.getHeight(),true,juce::SoftwareImageType());
        { juce::Graphics graphics(image);view.paint(graphics); }
        return image;
    };
    const auto lcdBefore=lcdSnapshot();
    runtime(initialSections,"F#m");
    const auto lcdAfter=lcdSnapshot();
    bool liveLcdChanged=false;
    const auto bounds=view.getLocalBounds().withTrimmedTop(104).withTrimmedBottom(100).reduced(12,0);
    cadenza::ui::genos::Camera defaultCamera;
    for(const auto& screen:model.screens)if(screen.id=="main") {
        std::array<cadenza::ui::genos::Point,4> polygon;
        for(std::size_t i=0;i<4;++i) {
            polygon[i]=defaultCamera.project(screen.corners[i],(float)bounds.getWidth(),(float)bounds.getHeight());
            polygon[i].x+=bounds.getX();polygon[i].y+=bounds.getY();
        }
        for(int y=0;y<view.getHeight()&&!liveLcdChanged;++y)
            for(int x=0;x<view.getWidth()&&!liveLcdChanged;++x)
                if(cadenza::ui::genos::contains(polygon,x+.5f,y+.5f)
                   &&lcdBefore.getPixelAt(x,y)!=lcdAfter.getPixelAt(x,y))liveLcdChanged=true;
    }
    expect(liveLcdChanged,"inclined LCD displays changing host chord, rather than a static reference photograph");
    runtime(initialSections);
    for(int n=28;n<=103;++n) {
        count=notes.size();click("note"+std::to_string(n));
        expect(notes.size()==count+2,"pointer plays/releases every key");note(count,n,true);note(count+1,n,false);
    }
    auto p=centre("note60"),q=centre("note62");count=notes.size();down(p);
    view.mouseDrag(event(view,q,p,true,true));up(q);
    expect(notes.size()==count+4,"glissando paired attacks/releases");
    note(count,60,true);note(count+1,60,false);note(count+2,62,true);note(count+3,62,false);
    count=notes.size();down(p);view.mouseDrag(event(view,{-20,-20},p,true,true));up({-20,-20});
    expect(notes.size()==count+2,"leaving keyboard and outside mouse-up release once");note(count+1,60,false);
    count=notes.size();down(p);view.mouseExit(event(view,{-20,-20},p,false));up(p);
    expect(notes.size()==count+2,"mouse exit without held button releases once");
    for(int release=0;release<3;++release) {
        count=notes.size();down(p);
        if(release==0)view.focusLost(juce::Component::focusChangedDirectly);
        if(release==1){view.setVisible(false);view.setVisible(true);}
        if(release==2)expect(view.keyPressed(juce::KeyPress(juce::KeyPress::escapeKey)),"Escape handled");
        up(p);expect(notes.size()==count+2,"safety release exactly one note-off");note(count+1,60,false);
    }
    count=notes.size();view.keyPressed(juce::KeyPress('Q',{},'q'));p=centre("note60");down(p);up(p);
    expect(notes.size()==count+1,"pointer release preserves computer-owned same note");
    view.keyStateChanged(false);expect(notes.size()==count+2,"last owner releases shared note");note(count+1,60,false);
    count=notes.size();down(p);view.keyPressed(juce::KeyPress('Q',{},'q'));view.keyStateChanged(false);
    expect(notes.size()==count+1,"computer release preserves pointer-owned note");up(p);
    expect(notes.size()==count+2,"pointer releases shared note last");
    runtime({"mainA","intro","ending","fill"});click("mainB");expect(sections.empty(),"absent variation disabled");
    click("mainA");click("introA");click("endingA");
    expect(sections==std::vector<std::string>({"mainA","intro","ending"}),"physical section aliases resolve to real sections");
    using cadenza::ui::genos::resolveSection;
    expect(resolveSection("fill",{"mainA","fillAA","fillA","fill"},"mainA")=="fillAA","specific fill precedes generic fill without inventing a physical button");
    expect(resolveSection("fill",{"mainB","fillBB","fill"},"mainB")=="fillBB","fill follows the current variation");
    expect(resolveSection("fill",{"mainA","fillA","fill"},"mainA")=="fillA","short fill alias fallback");
    expect(resolveSection("fill",{"mainA","fill"},"mainA")=="fill","generic fill fallback");
    expect(resolveSection("fill",{"mainA"},"mainA").empty(),"unavailable fill resolves to no action");
    runtime({"mainA","mainB","fillAA","fillA","fill"});click("mainB");
    expect(sections==std::vector<std::string>({"mainA","intro","ending","mainB"}),"available physical variation dispatches");
    click("play");expect(play==1,"physical start/stop reaches host");
    view.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));view.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));
    expect(play==2,"held Space toggles once");view.keyStateChanged(false);
    view.keyPressed(juce::KeyPress(juce::KeyPress::spaceKey));expect(play==3,"released Space toggles again");view.keyStateChanged(false);
    drag("fader0",{0,-500});drag("fader0",{0,500});
    expect(mixer==std::vector<std::pair<int,int>>({{10,127},{10,0}}),"first fader channel10 with both limits");
    drag("fader8",{0,-500});drag("fader8",{0,500});
    expect(master==std::vector<int>({127,0})&&mixer.size()==2,"ninth fader master only");
    drag("knob0",{0,-500});drag("knob0",{0,500});expect(reverb==std::vector<int>({100,0}),"reverb percentage endpoints");
    drag("knob1",{0,-500});drag("knob1",{0,500});expect(compressor==std::vector<int>({100,0}),"compressor percentage endpoints");
    drag("knob2",{0,-500});drag("knob2",{0,500});expect(master==std::vector<int>({127,0,127,0}),"master knob shared callback");
    drag("master",{0,-500});drag("master",{0,500});
    expect(master.size()==6&&master[4]==127&&master[5]==0,"physical MASTER shares real gain callback");
    drag("knob3",{0,-500});drag("knob3",{0,500});
    expect(eq==std::vector<std::array<int,3>>({{12,0,0},{-12,0,0}}),"EQ endpoints preserve other bands");
    p=centre("pitch");down(p);view.mouseDrag(event(view,p+juce::Point<float>{100,-100},p,true,true));
    expect(!pitch.empty()&&pitch.back()==16383,"joystick right maximum bend");
    expect(!modulation.empty()&&modulation.back()==127,"joystick up maximum modulation");
    view.mouseDrag(event(view,p+juce::Point<float>{-100,100},p,true,true));
    expect(pitch.back()==0&&modulation.back()==0,"joystick opposite limits");up(p);
    expect(pitch.back()==8192,"pitch recentres on release");
    down(p);view.mouseDrag(event(view,p+juce::Point<float>{20,-20},p,true,true));
    view.focusLost(juce::Component::focusChangedDirectly);expect(pitch.back()==8192,"pitch recentres on focus loss");
    const auto heldModulation=modulation.back();
    p=centre("pitch");down(p);view.mouseDrag(event(view,p+juce::Point<float>{1,-1},p,true,true));up(p);
    expect(modulation.back()==heldModulation+1,"new joystick gesture preserves held modulation without jumping");
    runtime({"mainA","mainB","mainC","mainD","intro","ending","fill"});toolbar("Style Creator");
    expect(newSession==1&&makeEditable==1&&editor==2&&page==7,"loaded creator makes editable and opens editor");
    count=notes.size();down(centre("note60"));toolbar("Vue arrangeur");
    expect(page==0,"Native opens Cadenza page");expect(notes.size()==count+2&&!notes.back().on,"Native releases held note");
    auto render=[&](const char* name,float pixelScale=1.0f) {
        view.setVisible(true);
        juce::Image image(juce::Image::ARGB,(int)std::lround(view.getWidth()*pixelScale),
                         (int)std::lround(view.getHeight()*pixelScale),true,juce::SoftwareImageType());
        const auto renderStart=std::chrono::steady_clock::now();
        {
        juce::Graphics graphics(image);
        graphics.addTransform(juce::AffineTransform::scale(pixelScale));
        // Paint the real component directly: standalone test components have no window peer.
        view.paint(graphics);
        for(int i=0;i<view.getNumChildComponents();++i) {
            auto* child=view.getChildComponent(i);if(!child->isVisible())continue;
            juce::Graphics::ScopedSaveState saved(graphics);
            graphics.setOrigin(child->getPosition());graphics.reduceClipRegion(child->getLocalBounds());
            child->paintEntireComponent(graphics,true);
        }
        } // Flush all painting before inspecting or exporting the pixels.
        std::cout<<"Actual JUCE render "<<name<<" ("<<image.getWidth()<<"x"<<image.getHeight()<<"): "
                 <<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-renderStart).count()<<" ms\n";
        const auto scene=view.renderedSceneSize();
        expect(scene.x>=3840,"scene is rendered from geometry at 4K or higher, rather than stretching a low-resolution bitmap");
        expect(scene.x<=7680&&scene.y<=4320,"supersampling buffers stay within their memory envelope");
        std::cout<<"Scene raster "<<scene.x<<"x"<<scene.y<<", display scale "<<pixelScale<<"\n";
        expect(view.controlIdAt({-1,-1}).empty(),"supersampled picking rejects outside-window coordinates");
        expect(view.controlIdAt({24.0f,75.0f}).empty(),"supersampled picking does not extend into the toolbar");
        int darkest=255,brightest=0,lit=0,samples=0;
        for(int y=0;y<image.getHeight();y+=4)for(int x=0;x<image.getWidth();x+=4) {
            auto c=image.getPixelAt(x,y);
            int v=(int)c.getRed()+(int)c.getGreen()+(int)c.getBlue();
            darkest=std::min(darkest,v/3);brightest=std::max(brightest,v/3);
            if(c.getAlpha()!=0&&v>90)++lit;++samples;
        }
        auto stream=juce::File::getCurrentWorkingDirectory().getChildFile(name).createOutputStream();
        expect(stream!=nullptr,"preview output opened");stream->setPosition(0);stream->truncate();
        expect(juce::PNGImageFormat{}.writeImageToStream(image,*stream),"actual component preview rendered");
        expect(brightest-darkest>80&&lit>samples/10,"capture is nonblank with visible instrument: "+std::string(name));
    };
    view.setSize(1280,800);toolbar("Vue 3D");picking();render("genos2-3d.png");
    toolbar("Vue de dessus");picking();render("genos2-top.png");
    view.setSize(1600,900);picking();render("genos2-top-resized.png");
    toolbar("Vue 3D");picking();render("genos2-3d-resized.png");
    view.setSize(900,650);picking();
    for(int i=0;i<view.getNumChildComponents();++i)
        expect(view.getLocalBounds().contains(view.getChildComponent(i)->getBounds()),"all toolbar buttons fit at minimum window size");
    render("genos2-minimum.png");
    view.setSize(3840,2160);toolbar("Vue 3D");picking();
    runtime(initialSections,"C7"); // Force a full geometry/LCD render for the 4K timing.
    render("genos2-4k.png");
    toolbar("Vue de dessus");picking();runtime(initialSections,"F#m");
    render("genos2-4k-top.png");
    view.setSize(1920,1080);toolbar("Vue 3D");runtime(initialSections);
    render("genos2-4k-hidpi.png",2.0f);picking();
    expect(view.renderedSceneSize().x>=4*(1920-24),"200-percent DPI retains supersampling above native physical pixels");
    lcdSnapshot(); // Returning to a 100-percent graphics target must invalidate the DPI cache.
    expect(view.renderedSceneSize().x==3840,"moving back from 200-percent DPI updates the cached raster resolution");
    picking();
    view.releaseNotes();view.setLookAndFeel(nullptr);
    std::cout<<"Genos interaction: pointer notes/releases, sections, mixer, knobs, joystick, Native/editor and resize passed\n";
    return 0;
}

int main() {
    try { return runTests(); }
    catch(const std::exception& error) {
        std::cerr<<"Genos interaction failure: "<<error.what()<<std::endl;
        return 1;
    }
}
