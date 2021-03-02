#pragma once


#include <AUI/Platform/AWindow.h>
#include <AUI/View/ARadioButton.h>

class ImportVersionWindow: public AWindow {
private:
    _<ARadioButton::Group> mRadioGroup = _new<ARadioButton::Group>();

public:
    ImportVersionWindow();
};

