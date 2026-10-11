#include "GenosSurface.h"
#include <numeric>

namespace cadenza::ui
{
namespace
{
constexpr std::array<int,8> mixerChannels{10,9,11,12,13,14,15,16};
constexpr std::array<int,18> computerPitches{60,62,64,65,67,69,71,72,61,63,66,68,48,50,52,53,55,57};
constexpr char computerKeys[] = "qsdfghjkzertwxcvbn";

std::string resolveSection(const std::string& requested,
                           const std::vector<std::string>& sections,
                           const std::string& current)
{
    auto exists = [&](const std::string& s) {
        return std::find(sections.begin(), sections.end(), s) != sections.end();
    };
    if (exists(requested)) return requested;
    if (requested == "introA" && exists("intro")) return "intro";
    if (requested == "endingA" && exists("ending")) return "ending";
    if (requested == "fill") {
        const char variation = current.size() == 5 && current.rfind("main",0) == 0 ? current.back() : 'A';
        for (const auto& s : {std::string("fill")+variation+variation,
                              std::string("fill")+variation, std::string("fill")})
            if (exists(s)) return s;
    }
    return {};
}
bool sectionControl(const std::string& id)
{
    return id.rfind("intro",0)==0 || id.rfind("main",0)==0
        || id.rfind("ending",0)==0 || id=="break" || id=="fill";
}
}

GenosSurface::GenosSurface(NativePanel::Callbacks cb, std::function<void(int)> openPage)
    : m_cb(std::move(cb)), m_openPage(std::move(openPage))
{
    setWantsKeyboardFocus(true);
    setTitle("Genos2 personnel en 3D");
    setDescription("Commandes de l'arrangeur Cadenza, clavier et vue 3D orientable");
    for (auto* b : {&m_styleButton,&m_playButton,&m_editorButton,&m_midiButton,
                    &m_audioButton,&m_topButton,&m_angleButton}) {
        addAndMakeVisible(*b);
        b->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff182938));
        b->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffeef4fa));
    }
    m_styleButton.onClick = [this] { if (m_cb.openStyle) m_cb.openStyle(); };
    m_playButton.onClick = [this] { if (m_cb.togglePlay) m_cb.togglePlay(); };
    m_editorButton.onClick = [this] {
        releaseNotes();
        if (m_sections.empty()) { if (m_cb.onRecNew) m_cb.onRecNew(4); }
        else if (m_cb.onMakeEditable) m_cb.onMakeEditable();
        if (m_cb.onRecEdit) m_cb.onRecEdit();
        if (m_openPage) m_openPage(NativePanel::kEditorPage);
    };
    m_midiButton.onClick = [this] { if (m_cb.openMidiSettings) m_cb.openMidiSettings(); };
    m_audioButton.onClick = [this] { if (m_cb.openAudioSettings) m_cb.openAudioSettings(); };
    m_topButton.onClick = [this] { m_camera.yaw=0; m_camera.pitch=1.55f; m_camera.zoom=1; repaint(); };
    m_angleButton.onClick = [this] { m_camera={}; repaint(); };
    m_hint = "Cliquez les touches et commandes · glissez le fond pour orienter · molette pour zoomer";
    m_playButton.setTooltip("Espace : lecture / arrêt. Échap : relâcher les notes du clavier à l'écran.");
    m_editorButton.setTooltip("Ouvrir l'atelier Cadenza sur une copie éditable du style chargé.");
    m_topButton.setTooltip("Recentrer le modèle en vue de dessus.");
    m_angleButton.setTooltip("Recentrer le modèle en perspective 3D.");
}

GenosSurface::~GenosSurface() { releaseNotes(); }

void GenosSurface::setRuntime(const juce::String& style, const juce::String& chord,
                              const std::string& section, int bpm, bool playing,
                              const std::vector<std::string>& available,
                              const std::array<int,9>& volumes, bool arranger,
                              bool memory, bool autoFill, bool syncStop, int transpose,
                              bool recorderActive, bool recording, const std::array<bool,4>& ots,
                              const std::array<bool,3>& rightEnabled, bool leftEnabled,
                              int reverb, int compressor, const std::array<int,3>& eq)
{
    const bool changed = m_style != style || m_chord != chord || m_section != section
        || m_bpm != bpm || m_playing != playing || m_sections != available
        || m_volumes != volumes || m_arranger != arranger || m_memory != memory
        || m_autoFill != autoFill || m_syncStop != syncStop || m_transpose != transpose
        || m_recorderActive != recorderActive || m_recording != recording || m_ots != ots
        || m_rightEnabled != rightEnabled || m_leftEnabled != leftEnabled
        || m_reverb!=reverb || m_compressor!=compressor || m_eq!=eq;
    m_style=style; m_chord=chord.isEmpty() ? "—" : chord; m_section=section;
    m_bpm=bpm; m_playing=playing; m_sections=available; m_volumes=volumes;
    m_arranger=arranger; m_memory=memory; m_autoFill=autoFill;
    m_syncStop=syncStop; m_transpose=transpose;
    m_recorderActive=recorderActive; m_recording=recording; m_ots=ots;
    m_rightEnabled=rightEnabled; m_leftEnabled=leftEnabled;
    m_reverb=reverb; m_compressor=compressor; m_eq=eq;
    m_playButton.setButtonText(playing ? "Arrêter" : "Lancer la lecture");
    if (changed && isVisible()) repaint();
}

juce::Rectangle<int> GenosSurface::modelBounds() const
{
    return getLocalBounds().withTrimmedTop(104).withTrimmedBottom(100).reduced(12,0);
}

bool GenosSurface::enabled(const genos::Control& c) const
{
    if (sectionControl(c.id)) return !resolveSection(c.id,m_sections,m_section).empty();
    if (c.id.rfind("ots",0)==0) return m_ots[(std::size_t)(c.id.back()-'0')];
    if (c.id=="rec" || c.id=="save") return m_recorderActive;
    return true;
}

bool GenosSurface::illuminated(const genos::Control& c) const
{
    if (c.kind == genos::Control::Kind::key) return m_notes.count(c.value) != 0;
    if (sectionControl(c.id)) return !m_section.empty() && resolveSection(c.id,m_sections,m_section)==m_section;
    if (c.id=="play") return m_playing;
    if (c.id=="acmp") return m_arranger;
    if (c.id=="memory") return m_memory;
    if (c.id=="autofill") return m_autoFill;
    if (c.id=="syncstop") return m_syncStop;
    if (c.id=="rec") return m_recording;
    if (c.id=="left") return m_leftEnabled;
    if (c.id.rfind("right",0)==0) return m_rightEnabled[(std::size_t)(c.id.back()-'0')];
    return false;
}

std::vector<GenosSurface::ProjectedFace> GenosSurface::buildFrame() const
{
    using namespace genos;
    std::vector<Face> faces;
    box(faces,{0,-0.22f,0},12.34f,4.56f,0.54f,0xff252a30);
    box(faces,{0,0.32f,-0.50f},11.82f,2.82f,0.045f,0xff20252b);
    box(faces,{0,0.39f,-1.10f},2.21f,1.58f,0.16f,0xff111920);
    box(faces,{0,0.55f,-1.10f},1.98f,1.36f,0.018f,0xff0b2438);
    box(faces,{-2.71f,0.39f,-1.82f},2.27f,0.25f,0.045f,0xff071926);
    for (int i=0;i<9;++i)
        box(faces,{-3.54f+i*.28f,.38f,-1.05f},.055f,.78f,.008f,0xff080b10);

    for (int i=0; i<static_cast<int>(m_controls.size()); ++i) {
        const auto& c=m_controls[static_cast<std::size_t>(i)];
        auto centre=c.centre;
        std::uint32_t colour=0xff4b5660;
        if (c.kind==Control::Kind::key) {
            colour=c.black ? 0xff16191e : 0xffe4e7e8;
            if (illuminated(c)) { centre.y-=0.035f; colour=0xff79b8df; }
        } else if (c.kind==Control::Kind::fader) {
            centre.z += (0.5f-static_cast<float>(m_volumes[c.value])/127.0f)*0.61f;
            box(faces,centre,c.width,0.17f,c.height,0xff697784,i);
            continue;
        } else if (c.kind==Control::Kind::knob) {
            // Eight-sided cylindrical knob, not a flat decoration.
            const float y0=centre.y,y1=y0+c.height,r=c.width/2;
            for(int side=0;side<8;++side) {
                const float a=side*juce::MathConstants<float>::twoPi/8;
                const float b=(side+1)*juce::MathConstants<float>::twoPi/8;
                const Vec3 p{centre.x+r*std::cos(a),y1,centre.z+r*std::sin(a)};
                const Vec3 q{centre.x+r*std::cos(b),y1,centre.z+r*std::sin(b)};
                faces.push_back({{{{centre.x,y1,centre.z},p,q,q}},0xff59636e,i,true});
                faces.push_back({{{p,{p.x,y0,p.z},{q.x,y0,q.z},q}},0xff343d47,i,false});
            }
            continue;
        } else if (c.kind==Control::Kind::wheel) colour=0xff13191f;
        else {
            if (!enabled(c)) colour=0xff2b3036;
            else if (illuminated(c)) colour=0xff197bb7;
            else if (c.id=="creator" || c.id=="play") colour=0xff235776;
            if (m_hover==i && enabled(c)) colour=juce::Colour(colour).brighter(.17f).getARGB();
        }
        box(faces,centre,c.width,c.depth,c.height,colour,i);
    }
    std::vector<ProjectedFace> projected;
    const auto bounds=modelBounds();
    projected.reserve(faces.size());
    for (const auto& f:faces) {
        ProjectedFace p;
        p.colour=juce::Colour(f.colour).withMultipliedBrightness(f.top ? 1.0f : 0.65f);
        p.control=f.control; p.top=f.top;
        for (std::size_t i=0;i<4;++i) {
            p.points[i]=m_camera.project(f.vertices[i],static_cast<float>(bounds.getWidth()),static_cast<float>(bounds.getHeight()));
            p.points[i].x+=bounds.getX(); p.points[i].y+=bounds.getY();
            p.depth+=p.points[i].depth/4;
        }
        projected.push_back(p);
    }
    std::stable_sort(projected.begin(),projected.end(),[](const auto& a,const auto& b){return a.depth<b.depth;});
    return projected;
}

void GenosSurface::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0c131b));
    g.setColour(juce::Colour(0xffedf3f8));
    g.setFont(juce::Font(juce::FontOptions(25.0f,juce::Font::bold)));
    g.drawText("Genos2",24,12,250,34,juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.setColour(juce::Colour(0xff9fb4c6));
    g.drawText("ARRANGEUR PERSONNEL  /  MOTEUR CADENZA",160,17,getWidth()-360,26,juce::Justification::centredLeft);
    const auto bounds=modelBounds();
    g.setColour(juce::Colour(0xff05090e));
    g.fillEllipse(bounds.toFloat().reduced(28.0f,40.0f).translated(0,25));
    for (const auto& f:buildFrame()) {
        juce::Path p; p.startNewSubPath(f.points[0].x,f.points[0].y);
        for (int i=1;i<4;++i) p.lineTo(f.points[i].x,f.points[i].y);
        p.closeSubPath(); g.setColour(f.colour); g.fillPath(p);
        g.setColour(juce::Colours::black.withAlpha(.20f)); g.strokePath(p,juce::PathStrokeType(.7f));
        if (f.top && f.control>=0) {
            const auto& c=m_controls[static_cast<std::size_t>(f.control)];
            if (!c.label.empty() && c.kind!=genos::Control::Kind::knob) {
                const auto r=p.getBounds();
                g.setFont(juce::Font(juce::FontOptions(juce::jlimit(7.0f,12.0f,r.getHeight()*.66f))));
                g.setColour(enabled(c)?juce::Colour(0xfff3f7fa):juce::Colour(0xff86939e));
                g.drawFittedText(juce::String(c.label),r.toNearestInt(),juce::Justification::centred,1,.60f);
            }
        }
    }
    auto point=[&](genos::Vec3 p) {
        const auto s=m_camera.project(p,(float)bounds.getWidth(),(float)bounds.getHeight());
        return juce::Point<float>(s.x+bounds.getX(),s.y+bounds.getY());
    };
    for(const auto& c:m_controls) if(c.kind==genos::Control::Kind::knob) {
        auto centre=c.centre; centre.y+=c.height+.005f;
        const float angle=-2.2f+4.4f*knobValue(c.value)/127.0f;
        const auto start=point(centre);
        const auto end=point({centre.x+.085f*std::sin(angle),centre.y,
                              centre.z-.085f*std::cos(angle)});
        g.setColour(juce::Colour(0xffefaa56));g.drawLine({start,end},2.0f);
    }
    const auto lcd=point({0,.58f,-1.1f});
    const float scale=std::min(bounds.getWidth()/13.7f,bounds.getHeight()/5.5f)*m_camera.zoom;
    const int lw=(int)(1.86f*scale), lh=(int)(.98f*scale);
    auto screen=juce::Rectangle<int>((int)lcd.x-lw/2,(int)lcd.y-lh/2,lw,lh);
    g.setColour(juce::Colour(0xffe3eff8));
    g.setFont(juce::Font(juce::FontOptions(std::max(11.0f,scale*.15f),juce::Font::bold)));
    g.drawFittedText(m_style,screen.removeFromTop(lh/3).reduced(6,1),juce::Justification::centredLeft,1);
    g.setFont(juce::Font(juce::FontOptions(std::max(18.0f,scale*.30f),juce::Font::bold)));
    g.setColour(juce::Colour(0xff80d4ff));
    g.drawFittedText(m_chord,screen.removeFromTop(lh/3).reduced(6,0),juce::Justification::centredLeft,1);
    g.setFont(juce::Font(juce::FontOptions(std::max(10.0f,scale*.125f))));
    g.setColour(juce::Colour(0xffb4cadd));
    g.drawFittedText(juce::String(m_bpm)+" BPM  ·  "+juce::String(m_section),screen.reduced(6,0),juce::Justification::centredLeft,1);
    const auto logo=point({4.88f,.42f,-.98f});
    g.setFont(juce::Font(juce::FontOptions(std::max(16.0f,scale*.26f))));
    g.setColour(juce::Colour(0xffaab1ba));
    g.drawText("Genos2",(int)logo.x-55,(int)logo.y-12,110,25,juce::Justification::centred);
    const auto band=getLocalBounds().removeFromBottom(88).reduced(24,7);
    g.setColour(juce::Colour(0xff192735)); g.fillRoundedRectangle(band.toFloat(),10);
    g.setFont(juce::Font(juce::FontOptions(15.0f,juce::Font::bold)));
    g.setColour(juce::Colour(0xffdcebf6));
    g.drawText(m_style+"    ·    "+juce::String(m_bpm)+" BPM    ·    Transposition "+juce::String(m_transpose),
               band.reduced(15,0).removeFromTop(33),juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(12.5f)));
    g.setColour(juce::Colour(0xff9cb7cb));
    g.drawFittedText(m_hint,band.reduced(15,5).withTrimmedTop(30),juce::Justification::centredLeft,2);
}

void GenosSurface::resized()
{
    const int width=juce::jlimit(75,145,(getWidth()-60)/7);
    int x=24;
    for (auto* b : {&m_styleButton,&m_playButton,&m_editorButton,&m_midiButton,&m_audioButton,&m_topButton,&m_angleButton}) {
        b->setBounds(x,62,width,34); x+=width+6;
    }
}

int GenosSurface::hit(juce::Point<float> p) const
{
    const auto frame=buildFrame();
    for (auto it=frame.rbegin();it!=frame.rend();++it)
        if (genos::contains(it->points,p.x,p.y)) return it->control;
    return -1;
}

void GenosSurface::mouseMove(const juce::MouseEvent& e)
{
    const int next=hit(e.position);
    if (next==m_hover) return;
    m_hover=next;
    if (next>=0) {
        const auto& c=m_controls[(std::size_t)next];
        m_hint=juce::String(c.hint.empty()?c.label:c.hint);
        if (!enabled(c)) m_hint="Cette section n'existe pas dans le style chargé.";
        setTooltip(m_hint);
        setMouseCursor(enabled(c)?juce::MouseCursor::PointingHandCursor:juce::MouseCursor::NormalCursor);
    } else { setTooltip({}); setMouseCursor(juce::MouseCursor::NormalCursor); }
    repaint();
}

void GenosSurface::dispatch(const genos::Control& c)
{
    if (!enabled(c)) return;
    const auto& id=c.id;
    if (sectionControl(id)) { if (m_cb.selectSection) m_cb.selectSection(resolveSection(id,m_sections,m_section)); }
    else if (id=="play") { if (m_cb.togglePlay) m_cb.togglePlay(); }
    else if (id=="style") m_styleButton.triggerClick();
    else if (id=="creator") m_editorButton.triggerClick();
    else if (id=="midi") m_midiButton.triggerClick();
    else if (id=="audio") m_audioButton.triggerClick();
    else if (id=="soundfont") { if (m_cb.openSoundFont) m_cb.openSoundFont(); }
    else if (id=="import") { if (m_cb.importMidiStyle) m_cb.importMidiStyle(); }
    else if (id=="native") { releaseNotes(); if (m_openPage) m_openPage(0); }
    else if (id=="tempo-") { if (m_cb.nudgeTempo) m_cb.nudgeTempo(-1); }
    else if (id=="tempo+") { if (m_cb.nudgeTempo) m_cb.nudgeTempo(1); }
    else if (id=="transpose-") { if (m_cb.nudgeTranspose) m_cb.nudgeTranspose(-1); }
    else if (id=="transpose+") { if (m_cb.nudgeTranspose) m_cb.nudgeTranspose(1); }
    else if (id=="acmp") { if (m_cb.setArranger) m_cb.setArranger(!m_arranger); }
    else if (id=="memory") { if (m_cb.setChordMemory) m_cb.setChordMemory(!m_memory); }
    else if (id=="autofill") { if (m_cb.setAutoFill) m_cb.setAutoFill(!m_autoFill); }
    else if (id=="syncstop") { if (m_cb.setSyncroStop) m_cb.setSyncroStop(!m_syncStop); }
    else if (id.rfind("ots",0)==0) { if (m_cb.onOts) m_cb.onOts(id.back()-'0'); }
    else if (id.rfind("right",0)==0) {
        const int layer=id.back()-'0';
        if (m_cb.onRightEnabled) m_cb.onRightEnabled(layer,!m_rightEnabled[(std::size_t)layer]);
    }
    else if (id=="left") { if(m_cb.onLeftEnabled)m_cb.onLeftEnabled(!m_leftEnabled); }
    else if (id=="rec") { if(m_cb.onRecArm)m_cb.onRecArm(!m_recording); }
    else if (id=="save") { if(m_cb.onRecSave)m_cb.onRecSave(); }
}

void GenosSurface::pressNote(int note)
{
    if (!m_notes.insert(note).second) return;
    if (m_cb.onKeyboardNote) m_cb.onKeyboardNote(note,100,true);
    repaint();
}
void GenosSurface::releasePointerNote()
{
    if (m_pointerNote<0) return;
    const int n=m_pointerNote; m_pointerNote=-1;
    if (!m_computerNotes.count(n)) {
        m_notes.erase(n); if (m_cb.onKeyboardNote) m_cb.onKeyboardNote(n,0,false);
    }
    repaint();
}
void GenosSurface::releaseNotes()
{
    for (int n:m_notes) if (m_cb.onKeyboardNote) m_cb.onKeyboardNote(n,0,false);
    m_notes.clear(); m_computerNotes.clear(); m_pointerNote=-1;
    if (m_drag>=0 && m_controls[(std::size_t)m_drag].id=="pitch" && m_cb.onPitchBend)
        m_cb.onPitchBend(8192);
    m_drag=-1; repaint();
}
void GenosSurface::mouseDown(const juce::MouseEvent& e)
{
    grabKeyboardFocus(); m_drag=hit(e.position); m_dragStart=e.position;
    m_startYaw=m_camera.yaw; m_startPitch=m_camera.pitch;
    if (m_drag<0) return;
    const auto& c=m_controls[(std::size_t)m_drag];
    if (c.kind==genos::Control::Kind::key) { m_pointerNote=c.value; pressNote(c.value); }
    else if (c.kind==genos::Control::Kind::fader) m_startValue=m_volumes[c.value];
    else if (c.kind==genos::Control::Kind::knob) m_startValue=knobValue(c.value);
    else if (c.kind==genos::Control::Kind::button) dispatch(c);
}
void GenosSurface::mouseDrag(const juce::MouseEvent& e)
{
    if (m_drag<0) {
        m_camera.yaw=juce::jlimit(-0.48f,.48f,m_startYaw+(e.position.x-m_dragStart.x)*.002f);
        m_camera.pitch=juce::jlimit(.72f,1.56f,m_startPitch+(e.position.y-m_dragStart.y)*.003f);
        repaint(); return;
    }
    const auto& c=m_controls[(std::size_t)m_drag];
    if (c.kind==genos::Control::Kind::key) {
        const int index=hit(e.position);
        if (index>=0 && m_controls[(std::size_t)index].kind==genos::Control::Kind::key) {
            const int note=m_controls[(std::size_t)index].value;
            if (note!=m_pointerNote) {releasePointerNote();m_pointerNote=note;pressNote(note);}
        } else releasePointerNote();
    } else if (c.kind==genos::Control::Kind::fader) {
        const int value=juce::jlimit(0,127,m_startValue+(int)((m_dragStart.y-e.position.y)*2));
        if(c.value==8) {if(m_cb.onMasterChanged)m_cb.onMasterChanged(value);}
        else if (m_cb.onMixerVolume) m_cb.onMixerVolume(mixerChannels[c.value],value);
        m_volumes[c.value]=value; m_hint="Volume partie "+juce::String(c.value+1)+" : "+juce::String(value); repaint();
    } else if(c.kind==genos::Control::Kind::knob) {
        const int value=juce::jlimit(0,127,m_startValue+(int)((m_dragStart.y-e.position.y)*2));
        if(c.value==0) {m_reverb=value*100/127;if(m_cb.onReverbChanged)m_cb.onReverbChanged(m_reverb);}
        else if(c.value==1) {m_compressor=value*100/127;if(m_cb.onCompChanged)m_cb.onCompChanged(m_compressor);}
        else if(c.value==2) {m_volumes[8]=value;if(m_cb.onMasterChanged)m_cb.onMasterChanged(value);}
        else {m_eq[c.value-3]=value*24/127-12;if(m_cb.onEqChanged)m_cb.onEqChanged(m_eq[0],m_eq[1],m_eq[2]);}
        m_hint=juce::String(c.label)+" : "+juce::String(value);repaint();
    } else if (c.kind==genos::Control::Kind::wheel) {
        const int delta=(int)((m_dragStart.y-e.position.y)*130);
        if(c.id=="pitch" && m_cb.onPitchBend) m_cb.onPitchBend(juce::jlimit(0,16383,8192+delta));
        if(c.id=="mod" && m_cb.onModulation) m_cb.onModulation(juce::jlimit(0,127,delta/100));
    }
}
void GenosSurface::mouseUp(const juce::MouseEvent&)
{
    releasePointerNote();
    if (m_drag>=0 && m_controls[(std::size_t)m_drag].id=="pitch" && m_cb.onPitchBend) m_cb.onPitchBend(8192);
    m_drag=-1;
}
void GenosSurface::mouseExit(const juce::MouseEvent& e)
{
    if (!e.mods.isAnyMouseButtonDown()) releasePointerNote();
    m_hover=-1; repaint();
}
void GenosSurface::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& wheel)
{
    m_camera.zoom=juce::jlimit(.65f,1.45f,m_camera.zoom+wheel.deltaY*.20f); repaint();
}
bool GenosSurface::keyPressed(const juce::KeyPress& key)
{
    if (key.getModifiers().isCtrlDown() || key.getModifiers().isAltDown()
        || key.getModifiers().isCommandDown()) return false;
    if (key.getKeyCode()==juce::KeyPress::spaceKey) {
        if (!m_computerNotes.count(-1)) {m_computerNotes.insert(-1);if(m_cb.togglePlay)m_cb.togglePlay();}
        return true;
    }
    if (key.getKeyCode()==juce::KeyPress::escapeKey) {releaseNotes();return true;}
    const char ch=(char)juce::CharacterFunctions::toLowerCase(key.getTextCharacter());
    for (std::size_t i=0;i<computerPitches.size();++i) if(ch==computerKeys[i]) {
        m_computerNotes.insert(computerPitches[i]); pressNote(computerPitches[i]); return true;
    }
    return false;
}
bool GenosSurface::keyStateChanged(bool)
{
    if (!juce::KeyPress::isKeyCurrentlyDown(juce::KeyPress::spaceKey)) m_computerNotes.erase(-1);
    for (std::size_t i=0;i<computerPitches.size();++i) {
        const int n=computerPitches[i];
        if (m_computerNotes.count(n) && !juce::KeyPress::isKeyCurrentlyDown(juce::CharacterFunctions::toUpperCase((juce::juce_wchar)computerKeys[i]))) {
            m_computerNotes.erase(n);
            if (m_pointerNote!=n) {m_notes.erase(n);if(m_cb.onKeyboardNote)m_cb.onKeyboardNote(n,0,false);}
        }
    }
    repaint(); return true;
}
void GenosSurface::focusLost(FocusChangeType) { releaseNotes(); }
void GenosSurface::visibilityChanged() { if (!isVisible()) releaseNotes(); }
int GenosSurface::knobValue(int i) const
{
    if(i==0)return m_reverb*127/100;
    if(i==1)return m_compressor*127/100;
    if(i==2)return m_volumes[8];
    return juce::jlimit(0,127,(m_eq[(std::size_t)i-3]+12)*127/24);
}
}
