#pragma once
#include "GenosGeometry.h"
#include "NativePanel.h"
#include "GenosModel.h"
#include "GenosRaster.h"
#include <set>

namespace cadenza::ui
{
// Native parametric 3D surface. It shares the existing panel callbacks; it does
// not implement a second arranger or pretend that a hardware device is present.
class GenosSurface final : public juce::Component,
                           public juce::SettableTooltipClient
{
public:
    explicit GenosSurface(NativePanel::Callbacks callbacks, std::function<void(int)> openPage);
    ~GenosSurface() override;
    void setRuntime(const juce::String& style, const juce::String& chord,
                    const std::string& section, int bpm, bool playing,
                    const std::vector<std::string>& available,
                    const std::array<int, 9>& volumes, bool arranger,
                    bool memory, bool autoFill, bool syncStop, int transpose,
                    bool recorderActive, bool recording, const std::array<bool,4>& ots,
                    const std::array<bool,3>& rightEnabled, bool leftEnabled,
                    int reverb, int compressor, const std::array<int,3>& eq);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed(const juce::KeyPress&) override;
    bool keyStateChanged(bool) override;
    void focusLost(FocusChangeType) override;
    void visibilityChanged() override;
    void releaseNotes();
    std::string controlIdAt(juce::Point<float> point) const;
    juce::Point<float> controlScreenCentre(const std::string& id) const;

private:
    void ensureFrame() const;
    genos::Vec3 movedVertex(genos::Vec3, int control) const;
    juce::Point<float> project(genos::Vec3) const;
    void paintScreen(juce::Graphics&, const genos::ModelScreen&);
    int hit(juce::Point<float>) const;
    bool enabled(const genos::Control&) const;
    bool illuminated(const genos::Control&) const;
    void dispatch(const genos::Control&);
    void pressNote(int);
    void releasePointerNote();
    juce::Rectangle<int> modelBounds() const;
    NativePanel::Callbacks m_cb;
    std::function<void(int)> m_openPage;
    genos::Model m_model = genos::Model::load();
    std::vector<genos::Control> m_controls = m_model.controls;
    mutable genos::Raster m_raster;
    mutable juce::Image m_frame;
    mutable bool m_frameDirty = true;
    genos::Camera m_camera;
    juce::String m_style = juce::String::fromUTF8("Aucun style chargé"), m_chord = juce::String::fromUTF8("—"), m_hint;
    std::string m_section;
    std::vector<std::string> m_sections;
    std::array<int,9> m_volumes{100,100,100,100,100,100,100,100,100};
    std::array<int,3> m_eq{};
    int m_reverb=30, m_compressor=0;
    int m_bpm = 110, m_transpose = 0, m_hover = -1, m_drag = -1;
    int m_pointerNote = -1;
    bool m_playing = false, m_arranger = true, m_memory = true;
    bool m_autoFill = true, m_syncStop = false;
    bool m_recorderActive = false, m_recording = false, m_leftEnabled = false;
    std::array<bool,4> m_ots{};
    std::array<bool,3> m_rightEnabled{};
    std::set<int> m_notes;
    std::set<int> m_computerNotes;
    juce::Point<float> m_dragStart;
    float m_startYaw{}, m_startPitch{};
    int m_startValue{};
    int m_pitch=8192, m_modulation=0;
    int knobValue(int index) const;
    juce::TextButton m_styleButton{"Ouvrir un style"}, m_playButton{"Lecture"};
    juce::TextButton m_editorButton{"Style Creator"}, m_midiButton{"Connexion MIDI"};
    juce::TextButton m_audioButton{"Sortie audio"}, m_topButton{"Vue de dessus"};
    juce::TextButton m_angleButton{"Vue 3D"};
    juce::TextButton m_nativeButton{"Vue Cadenza"};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenosSurface)
};
}
