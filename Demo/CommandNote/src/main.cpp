#include "Notepad.h"
#include "ConsoleInterface.h"

int main() {
    my::notepad::Notepad notepad;
    my::ui::ConsoleInterface interface;
    interface.run(notepad);
    return 0;
}