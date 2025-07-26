#ifndef MY_NOTEPAD_INOTEPADINTERFACE_H
#define MY_NOTEPAD_INOTEPADINTERFACE_H

/*
 *********************** 界面接口 ***********************

*/

namespace my::notepad
{
    class Notepad;
}

namespace my::ui
{

    class INotepadInterface
    {
    public:
        virtual ~INotepadInterface() = default;
        virtual void run(my::notepad::Notepad &notepad) = 0;
    };

} // namespace my::ui

#endif // MY_NOTEPAD_INOTEPADINTERFACE_H