#include "UI/GenosSurface.h"
#include "UI/CadenzaLookAndFeel.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
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
    cadenza::ui::GenosSurface view(cb,[&](int p){page=p;});
    view.setLookAndFeel(&theme); view.setSize(1280,800); view.setVisible(true);
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
    auto runtime=[&](std::vector<std::string> available) {
        view.setRuntime(juce::String::fromUTF8("Été à Montréal — Répétition"),"C#m","mainA",110,false,
            available,{104,86,101,87,80,86,78,78,100},true,true,true,false,0,
            false,false,{}, {true,false,false},false,30,15,{0,0,0});
    };
    auto picking=[&] {
        int keys=0;
        for(const auto& c:cadenza::ui::genos::makeControls()) {
            if(c.id=="mod")continue; // The imported instrument has one two-axis joystick.
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
    runtime({"mainA","mainB","mainC","mainD","intro","ending","fill"});picking();
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
    count=notes.size();view.keyPressed(juce::KeyPress('Q',{},'q'));down(p);up(p);
    expect(notes.size()==count+1,"pointer release preserves computer-owned same note");
    view.keyStateChanged(false);expect(notes.size()==count+2,"last owner releases shared note");note(count+1,60,false);
    count=notes.size();down(p);view.keyPressed(juce::KeyPress('Q',{},'q'));view.keyStateChanged(false);
    expect(notes.size()==count+1,"computer release preserves pointer-owned note");up(p);
    expect(notes.size()==count+2,"pointer releases shared note last");
    runtime({"mainA","intro","ending","fill"});click("mainB");expect(sections.empty(),"absent variation disabled");
    click("mainA");click("introA");click("endingA");click("fill");
    expect(sections==std::vector<std::string>({"mainA","intro","ending","fill"}),"section aliases resolve to real sections");
    runtime({"mainA","mainB","fillAA","fillA","fill"});click("mainB");click("fill");
    expect(sections[4]=="mainB"&&sections[5]=="fillAA","variation and specific fill dispatch");
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
    count=notes.size();down(centre("note60"));toolbar("Vue Cadenza");
    expect(page==0,"Native opens Cadenza page");expect(notes.size()==count+2&&!notes.back().on,"Native releases held note");
    auto render=[&](const char* name) {
        view.setVisible(true);
        juce::Image image(juce::Image::ARGB,view.getWidth(),view.getHeight(),true);
        juce::Graphics graphics(image);
        // Paint the real component directly: standalone test components have no window peer.
        view.paint(graphics);
        for(int i=0;i<view.getNumChildComponents();++i) {
            auto* child=view.getChildComponent(i);if(!child->isVisible())continue;
            juce::Graphics::ScopedSaveState saved(graphics);
            graphics.setOrigin(child->getPosition());graphics.reduceClipRegion(child->getLocalBounds());
            child->paintEntireComponent(graphics,true);
        }
        int darkest=255,brightest=0,lit=0,samples=0;
        for(int y=0;y<image.getHeight();y+=4)for(int x=0;x<image.getWidth();x+=4) {
            auto c=image.getPixelAt(x,y);
            int v=(int)c.getRed()+(int)c.getGreen()+(int)c.getBlue();
            darkest=std::min(darkest,v/3);brightest=std::max(brightest,v/3);
            if(c.getAlpha()!=0&&v>90)++lit;++samples;
        }
        expect(brightest-darkest>80&&lit>samples/10,"capture is nonblank with visible instrument: "+std::string(name));
        auto stream=juce::File::getCurrentWorkingDirectory().getChildFile(name).createOutputStream();
        expect(stream!=nullptr,"preview output opened");stream->setPosition(0);stream->truncate();
        expect(juce::PNGImageFormat{}.writeImageToStream(image,*stream),"actual component preview rendered");
    };
    view.setSize(1280,800);toolbar("Vue 3D");picking();render("genos2-3d.png");
    toolbar("Vue de dessus");picking();render("genos2-top.png");
    view.setSize(1600,900);picking();render("genos2-top-resized.png");
    toolbar("Vue 3D");picking();render("genos2-3d-resized.png");
    view.setSize(900,650);picking();
    for(int i=0;i<view.getNumChildComponents();++i)
        expect(view.getLocalBounds().contains(view.getChildComponent(i)->getBounds()),"all toolbar buttons fit at minimum window size");
    render("genos2-minimum.png");
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
