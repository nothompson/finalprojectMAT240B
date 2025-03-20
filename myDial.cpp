#include <JuceHeader.h>
#include "myDial.h"

myDial::myDial() : Slider()
{
    setLookAndFeel(& myDialLookAndFeel);
}

myDial::~myDial()
{
    setLookAndFeel(nullptr);
}

void myDial::mouseDown(const MouseEvent& event)
{
    Slider::mouseDown(event);
    setMouseCursor(MouseCursor::NoCursor);
    mousePosition = Desktop::getMousePosition();
}

void myDial::mouseUp(const MouseEvent& event)
{
    Slider::mouseUp(event);
    Desktop::setMousePosition(mousePosition);
    setMouseCursor(MouseCursor::NormalCursor);
}