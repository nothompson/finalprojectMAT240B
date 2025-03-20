#include <JuceHeader.h>
#include "extraDial.h"

extraDial::extraDial() : Slider()
{
    setLookAndFeel(& myExtraLookAndFeel);
}

extraDial::~extraDial()
{
    setLookAndFeel(nullptr);
}

void extraDial::mouseDown(const MouseEvent& event)
{
    Slider::mouseDown(event);
    setMouseCursor(MouseCursor::NoCursor);
    mousePosition = Desktop::getMousePosition();
}

void extraDial::mouseUp(const MouseEvent& event)
{
    Slider::mouseUp(event);
    Desktop::setMousePosition(mousePosition);
    setMouseCursor(MouseCursor::NormalCursor);
}