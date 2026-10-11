#include "UI/GenosSurface.h"
#include "UI/CadenzaLookAndFeel.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{
void expect(bool condition,const char* message)
{
    if(!condition) throw std::runtime_error(message);
}
}
int main()
{
    juce::ScopedJuceInitialiser_GUI initialise;
    cadenza::ui::CadenzaLookAndFeel theme;
    struct Note { int pitch; bool on; };
    std::vector<Note> notes;
    int openStyle=0, newSession=0, editor=0, page=-1;
    cadenza::ui::NativePanel::Callbacks callbacks;
    callbacks.onKeyboardNote=[&](int pitch,int,bool on){notes.push_back({pitch,on});};
    callbacks.openStyle=[&]{++openStyle;};
    callbacks.onRecNew=[&](int bars){expect(bars==4,"new style has four bars");++newSession;};
    callbacks.onRecEdit=[&]{++editor;};
    cadenza::ui::GenosSurface view(callbacks,[&](int p){page=p;});
    view.setLookAndFeel(&theme);
    view.setSize(1280,800);
    view.setVisible(true);
    expect(view.keyPressed(juce::KeyPress('Q',{},'q')),"melody keyboard key handled");
    expect(notes.size()==1 && notes.back().pitch==60 && notes.back().on,"melody note reaches host callback");
    view.keyPressed(juce::KeyPress('Q',{},'q'));
    expect(notes.size()==1,"key repeat does not retrigger held note");
    view.focusLost(juce::Component::focusChangedDirectly);
    expect(notes.size()==2 && !notes.back().on,"focus loss releases held note");
    view.keyPressed(juce::KeyPress('W',{},'w'));
    expect(notes.back().pitch==48 && notes.back().on,"lower keyboard row supplies chord-zone notes");
    view.setVisible(false);
    expect(notes.back().pitch==48 && !notes.back().on,"hiding the 3D view releases notes");
    const auto noteCount=notes.size();
    expect(!view.keyPressed(juce::KeyPress('S',juce::ModifierKeys::ctrlModifier,'s')),
           "system modifier shortcuts are not musical input");
    expect(notes.size()==noteCount,"shortcut produces no note");
    auto click=[&](const juce::String& text) {
        for(int i=0;i<view.getNumChildComponents();++i)
            if(auto* b=dynamic_cast<juce::TextButton*>(view.getChildComponent(i));b && b->getButtonText()==text){
                if(b->onClick)b->onClick(); return;
            }
        throw std::runtime_error("missing accessible toolbar button");
    };
    click("Ouvrir un style");
    expect(openStyle==1,"open style button reaches host callback");
    click("Style Creator");
    expect(newSession==1 && editor==1 && page==7,"creator opens real editor through host callbacks");

    // A rear control must stay on top of the chassis for both painting and
    // picking. Average-depth sorting previously made this button disappear.
    cadenza::ui::genos::Camera camera;
    for(const auto& control:cadenza::ui::genos::makeControls()) {
        if(control.id!="style" && control.id!="midi" && control.id!="knob0")continue;
        auto top=control.centre;top.y+=control.height;
        const auto point=camera.project(top,1256,596);
        expect(view.controlIdAt({point.x+12,point.y+104})==control.id,"rear control survives chassis occlusion");
    }

    view.setVisible(true);
    view.setRuntime("8 Beat Pop","C","mainA",110,false,
        {"mainA","mainB","mainC","mainD","intro","ending","fill"},
        {104,86,101,87,80,86,78,78,100},true,true,true,false,0,
        false,false,{}, {true,false,false},false,30,15,{0,0,0});
    auto render=[&](const char* name) {
        juce::Image image(juce::Image::ARGB,1280,800,true);
        juce::Graphics graphics(image);
        view.paintEntireComponent(graphics,true);
        juce::File file=juce::File::getCurrentWorkingDirectory().getChildFile(name);
        auto stream=file.createOutputStream();
        expect(stream!=nullptr,"preview output opened");
        stream->setPosition(0);stream->truncate();
        expect(juce::PNGImageFormat{}.writeImageToStream(image,*stream),"preview rendered from the actual component");
    };
    render("genos2-3d.png");
    click("Vue de dessus");
    render("genos2-top.png");
    view.setLookAndFeel(nullptr);
    std::cout << "Genos interaction: real host callbacks and note release passed\n";
}
