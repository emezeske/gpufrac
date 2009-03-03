#ifndef CEGUI_UTILITIES_H
#define CEGUI_UTILITIES_H

#include <CEGUI.h>

#include "utilities.h"

template <typename BimapType>
void configure_dropdown_box( cstring& window_name, const BimapType& map, const CEGUI::Event::Subscriber& subscriber )
{
    using namespace CEGUI;
    Combobox* combobox = static_cast<Combobox*>( WindowManager::getSingleton().getWindow( window_name.c_str() ) );
    combobox->setReadOnly( true );

    for ( typename BimapType::left_const_iterator it = map.left.begin(); it != map.left.end(); ++it )
    {
        combobox->addItem( new ListboxTextItem( ( utf8* ) it->first.c_str() ) );
    }

    combobox->subscribeEvent( Combobox::EventListSelectionAccepted, subscriber );
}

void configure_slider( cstring& window_name, const float initial_value, const CEGUI::Event::Subscriber& subscriber );

void set_window_text( cstring& window_name, float value, int precision );

#endif // CEGUI_UTILITIES_H
