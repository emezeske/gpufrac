#include "cegui_utilities.h"

void configure_slider( cstring& window_name, const float initial_value, const CEGUI::Event::Subscriber& subscriber )
{
    using namespace CEGUI;
    Slider* slider = static_cast<Slider*>( WindowManager::getSingleton().getWindow( window_name.c_str() ) );
    slider->subscribeEvent( Slider::EventValueChanged, subscriber );
    slider->setCurrentValue( initial_value );
}

void set_window_text( cstring& window_name, float value, int precision )
{
    using namespace CEGUI;
    std::ostringstream stream;
    stream.precision( precision );
    stream << value;
    Window* w = WindowManager::getSingleton().getWindow( window_name.c_str() );
    w->setText( stream.str().c_str() );
}
