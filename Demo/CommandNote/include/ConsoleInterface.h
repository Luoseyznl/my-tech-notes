#ifndef MY_NOTEPAD_CONSOLEINTERFACE_H
#define MY_NOTEPAD_CONSOLEINTERFACE_H

#include "INotepadInterface.h"

namespace my::ui {

// ConsoleInterface 类实现了 INotepadInterface 接口，用于在控制台中运行记事本
class ConsoleInterface : public INotepadInterface {
public:
    void run(my::notepad::Notepad& notepad) override;
};

} // namespace xai::ui

#endif // MY_NOTEPAD_CONSOLEINTERFACE_H