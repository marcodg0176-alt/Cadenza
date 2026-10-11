#include "UI/GenosSurface.h"
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
    struct Note { int pitch; bool on; };
    std::vector<Note> notes;
    int openStyle=0, newSession=0, editor=0, page=-1;
    cadenza::ui::NativePanel::Callbacks callbacks;
    callbacks.onKeyboardNote=[&](int pitch,int,bool on){notes.push_back({pitch,on});};
    callbacks.openStyle=[&]{++openStyle;};
    callbacks.onRecNew=[&](int bars){expect(bars==4,"new style has four bars");++newSession;};
    callbacks.onRecEdit=[&]{++editor;};
    cadenza::ui::GenosSurface view(callbacks,[&](int p){page=p;});
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
    std::cout << "Genos interaction: real host callbacks and note release passed\n";
}
