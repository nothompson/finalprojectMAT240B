#pragma once

#include <JuceHeader.h>
#include "extraLookAndFeel.h"

class extraDial : public Slider
{
public: 
    extraDial();
    ~extraDial();

    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override; 

private: 
    extraLookAndFeel myExtraLookAndFeel;
    Point<int> mousePosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (extraDial)
};