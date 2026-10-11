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
    if (requested == "fill") {
        const char variation = current.size() == 5 && current.rfind("main",0) == 0 ? current.back() : 'A';
        for (const auto& s : {std::string("fill")+variation+variation,
                              std::string("fill")+variation, std::string("fill")})
            if (exists(s)) return s;
    }
    if (exists(requested)) return requested;
    if (requested == "introA" && exists("intro")) return "intro";
    if (requested == "endingA" && exists("ending")) return "ending";
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
                    &m_audioButton,&m_topButton,&m_angleButton,&m_nativeButton}) {
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
    m_topButton.onClick = [this] { m_camera.yaw=0; m_camera.pitch=1.55f; m_camera.zoom=1; m_frameDirty=true; repaint(); };
    m_nativeButton.onClick = [this] { releaseNotes(); if(m_openPage)m_openPage(0); };
    m_angleButton.onClick = [this] { m_camera={}; m_frameDirty=true; repaint(); };
    m_hint = juce::String::fromUTF8("Cliquez les touches et commandes · glissez le fond pour orienter · molette pour zoomer");
    m_playButton.setTooltip(juce::String::fromUTF8("Espace : lecture / arrêt. Échap : relâcher les notes du clavier à l'écran."));
    m_editorButton.setTooltip(juce::String::fromUTF8("Ouvrir l'atelier Cadenza sur une copie éditable du style chargé."));
    m_topButton.setTooltip(juce::String::fromUTF8("Recentrer le modèle en vue de dessus."));
    m_angleButton.setTooltip(juce::String::fromUTF8("Recentrer le modèle en perspective 3D."));
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
    m_style=style; m_chord=chord.isEmpty() ? juce::String::fromUTF8("—") : chord; m_section=section;
    m_bpm=bpm; m_playing=playing; m_sections=available; m_volumes=volumes;
    m_arranger=arranger; m_memory=memory; m_autoFill=autoFill;
    m_syncStop=syncStop; m_transpose=transpose;
    m_recorderActive=recorderActive; m_recording=recording; m_ots=ots;
    m_rightEnabled=rightEnabled; m_leftEnabled=leftEnabled;
    m_reverb=reverb; m_compressor=compressor; m_eq=eq;
    m_playButton.setButtonText(playing ? juce::String::fromUTF8("Arrêter") : "Lecture");
    if (changed) {
        m_frameDirty=true;
        if(isVisible())repaint();
    }
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

genos::Vec3 GenosSurface::movedVertex(genos::Vec3 p, int index) const
{
    if(index<0)return p;
    const auto& c=m_controls[(std::size_t)index];
    if(c.kind==genos::Control::Kind::key && m_notes.count(c.value))p.y-=.035f;
    else if(c.kind==genos::Control::Kind::fader)
        p.z+=(.5f-m_volumes[(std::size_t)c.value]/127.0f)*.48f;
    else if(c.kind==genos::Control::Kind::knob) {
        const float angle=-2.2f+4.4f*knobValue(c.value)/127.0f;
        const float x=p.x-c.centre.x,z=p.z-c.centre.z;
        p.x=c.centre.x+x*std::cos(angle)-z*std::sin(angle);
        p.z=c.centre.z+x*std::sin(angle)+z*std::cos(angle);
    } else if(c.kind==genos::Control::Kind::joystick) {
        p.x+=(m_pitch-8192)/8192.0f*.07f;
        p.z-=m_modulation/127.0f*.06f;
    }
    return p;
}

juce::Point<float> GenosSurface::project(genos::Vec3 p) const
{
    const auto b=modelBounds();
    const auto q=m_camera.project(p,(float)b.getWidth(),(float)b.getHeight());
    return {q.x+b.getX(),q.y+b.getY()};
}

void GenosSurface::ensureFrame() const
{
    const auto bounds=modelBounds();
    if(!m_frameDirty && m_frame.getWidth()==bounds.getWidth() && m_frame.getHeight()==bounds.getHeight())return;
    const int w=bounds.getWidth(),h=bounds.getHeight();
    if(w<=0 || h<=0)return;
    m_raster.clear(w,h);
    for(const auto& face:m_model.triangles) {
        std::array<genos::Point,3> points;
        for(std::size_t i=0;i<3;++i)
            points[i]=m_camera.project(face.moving?movedVertex(face.vertices[i],face.control):face.vertices[i],
                                       (float)w,(float)h);
        auto colour=juce::Colour(face.colour);
        if(face.control>=0) {
            const auto& c=m_controls[(std::size_t)face.control];
            if(c.kind==genos::Control::Kind::key) {
                if(illuminated(c))colour=colour.interpolatedWith(juce::Colour(0xff70b6d5),.45f);
            } else if(c.kind==genos::Control::Kind::button) {
                if(!enabled(c))colour=colour.withMultipliedBrightness(.52f);
                else if(illuminated(c))colour=colour.interpolatedWith(juce::Colour(0xff368dcc),.65f);
            }
            if(m_hover==face.control && enabled(c))colour=colour.brighter(.14f);
        }
        m_raster.triangle(points,colour.getARGB(),face.control);
    }
    m_frame=juce::Image(juce::Image::ARGB,w,h,true,juce::SoftwareImageType());
    juce::Image::BitmapData data(m_frame,juce::Image::BitmapData::writeOnly);
    for(int y=0;y<h;++y)for(int x=0;x<w;++x) {
        const auto c=m_raster.pixels[(std::size_t)y*w+x];
        auto* pixel=reinterpret_cast<juce::PixelARGB*>(data.getPixelPointer(x,y));
        pixel->setARGB((juce::uint8)(c>>24),(juce::uint8)(c>>16),(juce::uint8)(c>>8),(juce::uint8)c);
    }
    m_frameDirty=false;
}

juce::Point<float> GenosSurface::controlScreenCentre(const std::string& id) const
{
    for(std::size_t i=0;i<m_controls.size();++i)if(m_controls[i].id==id) {
        const auto& c=m_controls[i];
        auto p=c.centre;p.y+=c.height*.8f;
        if(c.kind==genos::Control::Kind::fader){p.z-=.08f;p.y=.55f;}
        return project(movedVertex(p,(int)i));
    }
    return {-1,-1};
}

void GenosSurface::paintScreen(juce::Graphics& g,const genos::ModelScreen& screen)
{
    const bool main=screen.id=="main";
    const int w=main?600:600,h=main?310:64;
    juce::Image image(juce::Image::RGB,w,h,true,juce::SoftwareImageType());
    {
    juce::Graphics lcd(image);
    lcd.fillAll(juce::Colour(0xff142737));
    if(main) {
        lcd.setColour(juce::Colour(0xff22455c));lcd.fillRect(0,0,w,42);
        lcd.setFont(juce::Font(juce::FontOptions(20.0f,juce::Font::bold)));
        lcd.setColour(juce::Colours::white);
        lcd.drawText("CADENZA",14,0,250,42,juce::Justification::centredLeft);
        lcd.drawText(juce::String(m_bpm)+" BPM",400,0,185,42,juce::Justification::centredRight);
        lcd.setFont(juce::Font(juce::FontOptions(25.0f,juce::Font::bold)));
        lcd.drawFittedText(m_style,{16,52,w-32,46},juce::Justification::centredLeft,1);
        lcd.setColour(juce::Colour(0xff7cd0ee));lcd.setFont(juce::Font(juce::FontOptions(42.0f,juce::Font::bold)));
        lcd.drawText(m_chord,18,110,180,55,juce::Justification::centredLeft);
        lcd.setFont(juce::Font(juce::FontOptions(23.0f)));
        lcd.drawText(juce::String(m_section),230,110,350,55,juce::Justification::centredRight);
        for(int i=0;i<9;++i) {
            lcd.setColour(juce::Colour(0xff234658));lcd.fillRect(18+i*64,189,48,32);
            lcd.setColour(juce::Colour(0xff5faacc));lcd.fillRect(18+i*64,221-m_volumes[(std::size_t)i]*32/127,48,m_volumes[(std::size_t)i]*32/127);
        }
        lcd.setColour(juce::Colour(0xffdf903f));lcd.fillRect(0,246,w,42);
        lcd.setColour(juce::Colour(0xff142737));lcd.setFont(juce::Font(juce::FontOptions(22.0f,juce::Font::bold)));
        lcd.drawText(m_playing?"LECTURE":juce::String::fromUTF8("PRÊT"),18,246,230,42,juce::Justification::centredLeft);
        lcd.drawText(juce::String::fromUTF8("STYLE · MIXER · ÉDITEUR"),250,246,335,42,juce::Justification::centredRight);
    } else {
        constexpr const char* names[]={"REV","COMP","VOL","LOW","MID","HIGH"};
        for(int i=0;i<6;++i) {
            lcd.setColour(juce::Colour(0xff7cd0ee));lcd.setFont(juce::Font(juce::FontOptions(18.0f)));
            lcd.drawText(names[i],i*100,0,100,28,juce::Justification::centred);
            lcd.setColour(juce::Colours::white);
            lcd.drawText(juce::String(knobValue(i)),i*100,27,100,32,juce::Justification::centred);
        }
    }
    } // Complete the display image before the host graphics reads it.
    auto c=screen.centre;c.y+=.036f;
    const auto a=project({c.x-screen.width/2,c.y,c.z-screen.depth/2});
    const auto b=project({c.x+screen.width/2,c.y,c.z-screen.depth/2});
    const auto d=project({c.x-screen.width/2,c.y,c.z+screen.depth/2});
    g.drawImageTransformed(image,juce::AffineTransform::fromTargetPoints(0,0,a.x,a.y,
        (float)w,0,b.x,b.y,0,(float)h,d.x,d.y),false);
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
    {
    juce::Graphics::ScopedSaveState sceneState(g);
    g.reduceClipRegion(bounds);
    ensureFrame();
    g.drawImageAt(m_frame,bounds.getX(),bounds.getY());
    const float scale=std::min(bounds.getWidth()/13.7f,bounds.getHeight()/5.5f)*m_camera.zoom;
    for(const auto& label:m_model.labels) {
        // Live display artwork is supplied by the host, never baked as a photo.
        bool inScreen=false;
        for(const auto& screen:m_model.screens)
            if(std::abs(label.position.x-screen.centre.x)<screen.width/2 &&
               std::abs(label.position.z-screen.centre.z)<screen.depth/2)inScreen=true;
        if(inScreen)continue;
        const auto p=project(label.position);
        const float size=std::max(5.5f,label.size*scale);
        g.setFont(juce::Font(juce::FontOptions(size)));
        g.setColour(juce::Colour(label.colour));
        const float width=std::max(30.0f,size*label.text.length()*.8f);
        g.drawText(label.text,juce::Rectangle<float>(p.x-width/2,p.y-size*.55f,width,size*1.2f),
                   juce::Justification::centred,false);
    }
    for(const auto& screen:m_model.screens)paintScreen(g,screen);
    }
    const auto band=getLocalBounds().removeFromBottom(88).reduced(24,7);
    g.setColour(juce::Colour(0xff192735)); g.fillRoundedRectangle(band.toFloat(),10);
    g.setFont(juce::Font(juce::FontOptions(15.0f,juce::Font::bold)));
    g.setColour(juce::Colour(0xffdcebf6));
    g.drawText(m_style+juce::String::fromUTF8("    ·    ")+juce::String(m_bpm)+juce::String::fromUTF8(" BPM    ·    Transposition ")+juce::String(m_transpose),
               band.reduced(15,0).removeFromTop(33),juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(12.5f)));
    g.setColour(juce::Colour(0xff9cb7cb));
    g.drawFittedText(m_hint,band.reduced(15,5).withTrimmedTop(30),juce::Justification::centredLeft,2);
}

void GenosSurface::resized()
{
    m_frameDirty=true;
    const int width=std::max(70,(getWidth()-90)/8);
    int x=24;
    for (auto* b : {&m_styleButton,&m_playButton,&m_editorButton,&m_midiButton,&m_audioButton,&m_topButton,&m_angleButton,&m_nativeButton}) {
        b->setBounds(x,62,width,34); x+=width+6;
    }
}

int GenosSurface::hit(juce::Point<float> p) const
{
    ensureFrame();
    const auto b=modelBounds();
    return m_raster.at((int)std::floor(p.x)-b.getX(),(int)std::floor(p.y)-b.getY());
}

std::string GenosSurface::controlIdAt(juce::Point<float> point) const
{
    const int index=hit(point);
    return index>=0?m_controls[(std::size_t)index].id:std::string{};
}

void GenosSurface::mouseMove(const juce::MouseEvent& e)
{
    const int next=hit(e.position);
    if (next==m_hover) return;
    m_hover=next;
    if (next>=0) {
        const auto& c=m_controls[(std::size_t)next];
        m_hint=juce::String(c.hint.empty()?c.label:c.hint);
        if (!enabled(c)) m_hint=juce::String::fromUTF8("Cette section n'existe pas dans le style chargé.");
        setTooltip(m_hint);
        setMouseCursor(enabled(c)?juce::MouseCursor::PointingHandCursor:juce::MouseCursor::NormalCursor);
    } else { setTooltip({}); setMouseCursor(juce::MouseCursor::NormalCursor); }
    m_frameDirty=true; repaint();
}

void GenosSurface::dispatch(const genos::Control& c)
{
    if (!enabled(c)) return;
    const auto& id=c.id;
    if (sectionControl(id)) { if (m_cb.selectSection) m_cb.selectSection(resolveSection(id,m_sections,m_section)); }
    else if (id=="play") { if (m_cb.togglePlay) m_cb.togglePlay(); }
    else if (id=="style") m_styleButton.onClick();
    else if (id=="creator") m_editorButton.onClick();
    else if (id=="midi") m_midiButton.onClick();
    else if (id=="audio") m_audioButton.onClick();
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
    m_frameDirty=true; repaint();
}
void GenosSurface::releasePointerNote()
{
    if (m_pointerNote<0) return;
    const int n=m_pointerNote; m_pointerNote=-1;
    if (!m_computerNotes.count(n)) {
        m_notes.erase(n); if (m_cb.onKeyboardNote) m_cb.onKeyboardNote(n,0,false);
    }
    m_frameDirty=true; repaint();
}
void GenosSurface::releaseNotes()
{
    for (int n:m_notes) if (m_cb.onKeyboardNote) m_cb.onKeyboardNote(n,0,false);
    m_notes.clear(); m_computerNotes.clear(); m_pointerNote=-1;
    if (m_drag>=0 && m_controls[(std::size_t)m_drag].id=="pitch") {
        m_pitch=8192;if(m_cb.onPitchBend)m_cb.onPitchBend(8192);
    }
    m_drag=-1; m_frameDirty=true; repaint();
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
    else if (c.kind==genos::Control::Kind::joystick) m_startValue=m_modulation;
    else if (c.kind==genos::Control::Kind::button) { dispatch(c);m_frameDirty=true;repaint(); }
}
void GenosSurface::mouseDrag(const juce::MouseEvent& e)
{
    if (m_drag<0) {
        m_camera.yaw=juce::jlimit(-0.48f,.48f,m_startYaw+(e.position.x-m_dragStart.x)*.002f);
        m_camera.pitch=juce::jlimit(.72f,1.56f,m_startPitch+(e.position.y-m_dragStart.y)*.003f);
        m_frameDirty=true; repaint(); return;
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
        m_volumes[c.value]=value; m_hint="Volume partie "+juce::String(c.value+1)+" : "+juce::String(value); m_frameDirty=true; repaint();
    } else if(c.kind==genos::Control::Kind::knob) {
        const int value=juce::jlimit(0,127,m_startValue+(int)((m_dragStart.y-e.position.y)*2));
        if(c.value==0) {m_reverb=value*100/127;if(m_cb.onReverbChanged)m_cb.onReverbChanged(m_reverb);}
        else if(c.value==1) {m_compressor=value*100/127;if(m_cb.onCompChanged)m_cb.onCompChanged(m_compressor);}
        else if(c.value==2) {m_volumes[8]=value;if(m_cb.onMasterChanged)m_cb.onMasterChanged(value);}
        else {m_eq[c.value-3]=value*24/127-12;if(m_cb.onEqChanged)m_cb.onEqChanged(m_eq[0],m_eq[1],m_eq[2]);}
        m_hint=juce::String(c.label)+" : "+juce::String(value);m_frameDirty=true; repaint();
    } else if(c.kind==genos::Control::Kind::joystick) {
        m_pitch=juce::jlimit(0,16383,8192+(int)((e.position.x-m_dragStart.x)*130));
        m_modulation=juce::jlimit(0,127,m_startValue+(int)((m_dragStart.y-e.position.y)*1.3f));
        if(m_cb.onPitchBend)m_cb.onPitchBend(m_pitch);
        if(m_cb.onModulation)m_cb.onModulation(m_modulation);
        m_hint=juce::String::fromUTF8("Joystick · pitch ")+juce::String(m_pitch)+juce::String::fromUTF8(" · modulation ")+juce::String(m_modulation);
        m_frameDirty=true;repaint();
    } else if (c.kind==genos::Control::Kind::wheel) {
        const int delta=(int)((m_dragStart.y-e.position.y)*130);
        if(c.id=="pitch" && m_cb.onPitchBend) m_cb.onPitchBend(juce::jlimit(0,16383,8192+delta));
        if(c.id=="mod" && m_cb.onModulation) m_cb.onModulation(juce::jlimit(0,127,delta/100));
    }
}
void GenosSurface::mouseUp(const juce::MouseEvent&)
{
    releasePointerNote();
    if (m_drag>=0 && m_controls[(std::size_t)m_drag].id=="pitch") {m_pitch=8192;if(m_cb.onPitchBend)m_cb.onPitchBend(8192);m_frameDirty=true;repaint();}
    m_drag=-1;
}
void GenosSurface::mouseExit(const juce::MouseEvent& e)
{
    if (!e.mods.isAnyMouseButtonDown()) releasePointerNote();
    m_hover=-1; m_frameDirty=true; repaint();
}
void GenosSurface::mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails& wheel)
{
    m_camera.zoom=juce::jlimit(.65f,1.45f,m_camera.zoom+wheel.deltaY*.20f); m_frameDirty=true; repaint();
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
    m_frameDirty=true; repaint(); return true;
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
