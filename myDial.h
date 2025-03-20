#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

class myDial : public Slider
{
public: 
    myDial();
    ~myDial();

    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override; 

private: 
    myLookAndFeel myDialLookAndFeel;
    Point<int> mousePosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (myDial)
};