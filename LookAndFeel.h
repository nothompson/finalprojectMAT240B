#pragma once

#include <JuceHeader.h>


class myLookAndFeel : public juce::LookAndFeel_V4
{
public:
    myLookAndFeel();

    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, Slider& slider)override;
    void drawLabel(Graphics& g, Label& label);

private:
    Image knobRelease;
    
};
