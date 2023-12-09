class c_framework {
public:
    void begin_window( std::string title, vec2_t window_size );
    void end_window( );

    int add_tabs( std::vector<std::string> tab_names );
    int add_sub_tabs( std::vector<std::string> tab_names );

    // comps
    void add_combobox( const std::string& title, const vec2_t& pos, const vec2_t& size );
    void checkbox( const std::string& title, const std::string& key );

    void slider( const std::string& title, const std::string& key, int min, int max, const std::string& suffix );
    void slider( const std::string& title, const std::string& key, float min, float max, const std::string& suffix );

    void dropdown( const std::string& title, const std::vector<std::string> items, int& value, bool& is_open );
    void button( const std::string& title, std::function<void( void )> callback );

    //void multi_dropdown( const std::string& title, const std::vector<std::string> items, std::vector<bool>& value, bool& is_open );
    //void colorpicker( const std::string& title, engine::color_t& value, bool& is_open, bool inlined );

    vec2_t menu_pos{ 50, 50 }, menu_size{ };
    vec2_t next_item_pos;
    std::string m_footer_text;

    // animation vars
    float m_stripe_width{ 0.f }, m_old_stripe_width{ 0.f };
    float m_menu_opacity{ 0.f }, m_old_menu_opacity{ 0.f };

    bool m_main_window_open{ true };
    bool m_insert_state, m_old_insert_state;
    bool m_mouse_state, m_old_mouse_state;

    vec2_t m_mouse_pos, m_old_mouse_pos;

    bool is_mouse_in_box( const vec2_t& p1, const vec2_t& p2 );
};

inline c_framework framework;

float inQuad( float t, float b, float c, float d ) {
    t = t / d;
    return c * powf( t, 2 ) + b;
}

float outQuad( float t, float b, float c, float d ) {
    t = t / d;
    return -c * t * ( t - 2 ) + b;
}

float lerp( float s, float e, float t ) {
    return ( s + ( e - s ) * t );
}

void c_framework::begin_window( std::string title, vec2_t window_size ) {
    menu_size = window_size;

    m_insert_state = GetAsyncKeyState( VK_INSERT );
    m_mouse_state = GetAsyncKeyState( VK_LBUTTON ) & 0x8000;

    POINT m_mouse_point;
    GetCursorPos( &m_mouse_point );

    m_mouse_pos.x = m_mouse_point.x - std::abs( overlay::m_window_pos.x );
    m_mouse_pos.y = m_mouse_point.y - std::abs( overlay::m_window_pos.y );

    m_menu_opacity = m_main_window_open ? 255.f : 0.f;
    m_menu_opacity = inQuad( 1.0f, m_old_menu_opacity, m_menu_opacity - m_old_menu_opacity, 6.f );

    auto i_menu_opacity = static_cast< int >( m_menu_opacity );
    if ( m_insert_state && !m_old_insert_state )
        m_main_window_open = !m_main_window_open;

    if ( !m_main_window_open ) {
        m_stripe_width = 0.f;
        m_menu_opacity = max( m_menu_opacity - 15.f, 0.f );
        if ( m_menu_opacity == 0 )
            return;
    }

    bool hovered = is_mouse_in_box( { menu_pos.x, menu_pos.y }, { menu_pos.x + window_size.x, menu_pos.y + 25.f } );
    if ( hovered && m_mouse_state && m_menu_opacity >= 254 ) {
        menu_pos.x += m_mouse_pos.x - m_old_mouse_pos.x;
        menu_pos.y += m_mouse_pos.y - m_old_mouse_pos.y;
    }

    m_footer_text = "build: " + std::string( __DATE__ );

    std::transform( m_footer_text.begin( ), m_footer_text.end( ), m_footer_text.begin( ), std::tolower );

    renderer::rounded_rect( { menu_pos.x - 1, menu_pos.y - 1, window_size.x + 2, window_size.y + 2 }, ImColor( 11, 11, 11, i_menu_opacity ) );
    renderer::filled_rounded_rect( { menu_pos.x, menu_pos.y, window_size.x, window_size.y }, ImColor( 11, 11, 11, i_menu_opacity ) );
    renderer::rounded_rect( { menu_pos.x + 1, menu_pos.y + 1, window_size.x - 2, window_size.y - 2 }, ImColor( 32, 32, 32, i_menu_opacity ) );

    renderer::filled_rect( menu_pos.x + 1, menu_pos.y + 1, window_size.x - 2, 20, ImColor( 8, 8, 8, i_menu_opacity ) );
    renderer::rounded_rect( { menu_pos.x + 1, menu_pos.y + 1, window_size.x - 2, 22 }, ImColor( 32, 32, 32, i_menu_opacity ) );

    renderer::rounded_rect( { menu_pos.x + 2, menu_pos.y + 2, window_size.x - 4, 20 }, ImColor( 0, 0, 0, i_menu_opacity ) );
    renderer::line( menu_pos.x + 2, menu_pos.y + 19, menu_pos.x + 2 + window_size.x - 4, menu_pos.y + 19, ImColor( 0, 0, 0, i_menu_opacity ) );

    //renderer::filled_rounded_rect( { menu_pos.x + 1, menu_pos.y + 1 - window_size.y - 24, window_size.x - 2, 22 }, ImColor( 255, 255, 255, i_menu_opacity ) );

    renderer::text( menu_pos.x + 7, menu_pos.y + 6, title, ImColor( 0, 0, 0, i_menu_opacity ), false );
    renderer::text( menu_pos.x + 6, menu_pos.y + 5, title, ImColor( 150, 150, 150, i_menu_opacity ), false );

    renderer::rounded_rect( { menu_pos.x + 2, menu_pos.y + window_size.y - 23, window_size.x - 4, 21 }, ImColor( 0, 0, 0, i_menu_opacity ) );
    renderer::filled_rounded_rect( { menu_pos.x + 2, menu_pos.y + window_size.y - 23, window_size.x - 4, 21 }, ImColor( 8, 8, 8, i_menu_opacity ) );

    renderer::filled_rect( menu_pos.x + 1, menu_pos.y + 20, window_size.x - 2, window_size.y - 40, ImColor( 11, 11, 11, i_menu_opacity ) );

    m_stripe_width = window_size.x / 1.5f - 2;
    m_stripe_width = inQuad( 1.f, m_old_stripe_width, m_stripe_width - m_old_stripe_width, 5.f );

    renderer::line( menu_pos.x + 2, menu_pos.y + window_size.y - 20, menu_pos.x + 2 + window_size.x - 4, menu_pos.y + window_size.y - 20, ImColor( 0, 0, 0, i_menu_opacity ) );

    renderer::rect( { menu_pos.x + 1, menu_pos.y + 20, window_size.x - 2, window_size.y - 40 }, ImColor( 32, 32, 32, i_menu_opacity ) );

    renderer::gradient_filled_rect( { menu_pos.x + window_size.x - window_size.x / 1.5f, menu_pos.y + 22, m_stripe_width, 1 }, ImColor( 0, 0, 0, 0 ), ImColor( 107, 107, 107, i_menu_opacity ) );

    auto footer_size = ImGui::CalcTextSize( m_footer_text.c_str( ) );

    renderer::text( menu_pos.x + 7, menu_pos.y + window_size.y - footer_size.y - 3, this->m_footer_text, ImColor( 0, 0, 0, i_menu_opacity ), false );
    renderer::text( menu_pos.x + 6, menu_pos.y + window_size.y - footer_size.y - 4, this->m_footer_text, ImColor( 150, 150, 150, i_menu_opacity ), false );

    //renderer::filled_rect( m_mouse_pos.x, m_mouse_pos.y, 8, 8, ImColor( 255, 0, 0, 255 ) );
}

void c_framework::end_window( ) {
    m_old_stripe_width = m_stripe_width;
    m_old_insert_state = m_insert_state;
    m_old_menu_opacity = m_menu_opacity;
    m_old_mouse_pos = m_mouse_pos;
    m_old_mouse_state = m_mouse_state;
}

int c_framework::add_tabs( std::vector<std::string> tab_names ) {
    static int selected_tab{ 0 };
    static float m_old_x{ menu_pos.x + 20 };
    float x{ menu_pos.x + 80 };

    auto i_menu_opacity = static_cast< int >( m_menu_opacity );
    if ( i_menu_opacity <= 0 )
        m_old_x = menu_pos.x + 20;

    if ( i_menu_opacity < 254 )
        x = inQuad( 1.f, m_old_x, x - m_old_x, 7.f );

    float draw_start = x;

    for ( int idx{ 0 }; idx < tab_names.size( ); idx++ ) {
        auto text_size = ImGui::CalcTextSize( tab_names[ idx ].c_str( ) );

        text_size.x += 8;

        bool hovered = is_mouse_in_box( { draw_start - 2, menu_pos.y + 3 }, { draw_start - 2 + text_size.x + 2, menu_pos.y + 12 } );

        if ( hovered && m_mouse_state )
            selected_tab = idx;

        auto col = hovered ? ImColor( 180, 180, 180, i_menu_opacity ) : ImColor( 75, 75, 75, i_menu_opacity );

        if ( idx == selected_tab ) {
            renderer::filled_rounded_rect( { draw_start - 2, menu_pos.y + 3, text_size.x + 4, 18 }, ImColor( 32, 32, 32, i_menu_opacity ) );
            renderer::filled_rect( draw_start - 2, menu_pos.y + 5, text_size.x + 4, 16, ImColor( 32, 32, 32, i_menu_opacity ) );
            renderer::rounded_rect( { draw_start - 2, menu_pos.y + 3, text_size.x + 4, 18 }, ImColor( 0, 0, 0, i_menu_opacity ) );

            renderer::filled_rect( draw_start - 2, menu_pos.y + 18, text_size.x + 4, 3, ImColor( 32, 32, 32, i_menu_opacity ) );

            renderer::line(
                draw_start - 2, menu_pos.y + 18,
                draw_start - 2, menu_pos.y + 20,
                ImColor( 0, 0, 0, i_menu_opacity )
            );

            renderer::line(
                draw_start - 2 + text_size.x + 3, menu_pos.y + 18,
                draw_start - 2 + text_size.x + 3, menu_pos.y + 20,
                ImColor( 0, 0, 0, i_menu_opacity )
            );

            renderer::line( draw_start - 2, menu_pos.y + 20, draw_start - 2 + text_size.x + 4, menu_pos.y + 20, ImColor( 32, 32, 32, i_menu_opacity ) );

            col = ImColor( 255, 255, 255, i_menu_opacity );
        }

        //renderer::text( draw_start - 1 + ( text_size.x + 4 ) / 2, menu_pos.y + 6, tab_names[ idx ], ImColor( 0, 0, 0, i_menu_opacity ), true );
        renderer::text( draw_start - 2 + ( text_size.x + 4 ) / 2, menu_pos.y + 5, tab_names[ idx ], col, true );

        draw_start += text_size.x + 10;
    }

    m_old_x = x;

    return selected_tab;
}

int c_framework::add_sub_tabs( std::vector<std::string> tab_names ) {
    static int selected_tab{ 0 };

    auto i_menu_opacity = static_cast< int >( m_menu_opacity );
    auto tab_size = ( menu_size.x / tab_names.size( ) ) - 50;

    auto offset = tab_names.size( ) <= 1 ? 25 : 50;

    for ( int idx{ 0 }; idx < tab_names.size( ); idx++ ) {
        auto hovered = is_mouse_in_box( { menu_pos.x + ( idx * tab_size ) + offset, menu_pos.y + 30 }, { menu_pos.x + ( idx * tab_size ) + tab_size + offset, menu_pos.y + 45 } );

        auto col = hovered ? ImColor( 150, 150, 150, i_menu_opacity ) : ImColor( 75, 75, 75, i_menu_opacity );

        if ( hovered && m_mouse_state )
            selected_tab = idx;

        if ( selected_tab == idx )
            col = ImColor( 255, 255, 255, i_menu_opacity );

        renderer::gradient_filled_rect( { menu_pos.x + ( idx * tab_size ) + offset + tab_size / 2, menu_pos.y + 45, tab_size / 2, 1 }, selected_tab == idx ? ImColor( 255, 255, 255, i_menu_opacity ) : ImColor( 52, 52, 52, i_menu_opacity ), ImColor( 0, 0, 0, 0 ) );
        renderer::gradient_filled_rect( { menu_pos.x + ( idx * tab_size ) + offset , menu_pos.y + 45, tab_size / 2, 1 }, ImColor( 0, 0, 0, 0 ), selected_tab == idx ? ImColor( 255, 255, 255, i_menu_opacity ) : ImColor( 52, 52, 52, i_menu_opacity ) );

        renderer::text( menu_pos.x + ( idx * tab_size ) + tab_size / 2 + offset, menu_pos.y + 27, tab_names[ idx ], col, true );
    }

    return selected_tab;
}

void c_framework::add_combobox( const std::string& title, const vec2_t& pos, const vec2_t& size ) {
    auto i_menu_opacity = static_cast< int >( m_menu_opacity );

    renderer::rect( menu_pos.x + pos.x, menu_pos.y + pos.y, size.x, size.y, ImColor( 32, 32, 32, i_menu_opacity ) );

    renderer::vert_gradient_filled_rect( { menu_pos.x + pos.x + 1, menu_pos.y + pos.y + 1, size.x - 2, 25 }, ImColor( 16, 16, 16, i_menu_opacity ), ImColor( 8, 8, 8, i_menu_opacity ) );

    renderer::text( menu_pos.x + pos.x + 6.f, menu_pos.y + pos.y + 6.f, title, ImColor( 75, 75, 75, i_menu_opacity ), false );

    renderer::filled_rect( menu_pos.x + pos.x + 1, menu_pos.y + pos.y + 25, size.x - 2, size.y - 26, ImColor( 8, 8, 8, i_menu_opacity ) );

    renderer::gradient_filled_rect( { menu_pos.x + pos.x + size.x / 2, menu_pos.y + pos.y + 25, size.x / 2, 1 }, ImColor( 0, 0, 0, 0 ), ImColor( 52, 52, 52, i_menu_opacity ) );
    renderer::gradient_filled_rect( { menu_pos.x + pos.x, menu_pos.y + pos.y + 25, size.x / 2, 1 }, ImColor( 52, 52, 52, i_menu_opacity ), ImColor( 0, 0, 0, 0 ) );

    next_item_pos = { menu_pos.x + pos.x + 15, menu_pos.y + pos.y + 40 };
}

void c_framework::checkbox( const std::string& title, const std::string& key ) {
    auto i_menu_opacity = static_cast< int >( m_menu_opacity );

    auto text_size = ImGui::CalcTextSize( title.c_str( ) );
    auto hovered = is_mouse_in_box( { next_item_pos.x, next_item_pos.y }, { next_item_pos.x + text_size.x + 8, next_item_pos.y + 8 } );

    if ( hovered && m_mouse_state && !m_old_mouse_state )
        g_cfg[ key ].set<bool>( !g_cfg[ key ].get<bool>( ) );

    auto col = hovered ? ImColor( 150, 150, 150, i_menu_opacity ) : ImColor( 75, 75, 75, i_menu_opacity );

    renderer::filled_rect( next_item_pos.x, next_item_pos.y + 1, 7, 7, ImColor( 32, 32, 32, i_menu_opacity ) );
    if ( g_cfg[ key ].get<bool>( ) ) {
        renderer::vert_gradient_filled_rect( { next_item_pos.x, next_item_pos.y, 7, 7 }, ImColor( 171, 171, 171, i_menu_opacity ), ImColor( 141, 141, 141, i_menu_opacity ) );
        col = ImColor( 255, 255, 255, i_menu_opacity );
    }

    renderer::rect( next_item_pos.x - 1, next_item_pos.y, 8, 8, ImColor( 0, 0, 0, i_menu_opacity ) );
    renderer::text( next_item_pos.x + 15.f, next_item_pos.y - text_size.y / 2 + 3, title, col, false );

    next_item_pos.y += 15;
}

template <typename t>
void clamp( t& x, t min, t max ) {
    if ( x < min )
        x = min;
    if ( x > max )
        x = max;
}

void c_framework::slider( const std::string& title, const std::string& key, int min, int max, const std::string& suffix ) {
    next_item_pos.x += 15;

    auto i_menu_opacity = static_cast< int >( m_menu_opacity );

    static bool was_hovering = false;

    if ( m_mouse_state && !was_hovering )
        was_hovering = is_mouse_in_box( { next_item_pos.x + 1, next_item_pos.y + 16 }, { next_item_pos.x + 166, next_item_pos.y + 22 } );
    else
        was_hovering = false;

    if ( was_hovering && m_mouse_state ) {
        g_cfg[ key ].set<int>( int( ( m_old_mouse_pos.x - ( next_item_pos.x + 1 ) ) * ( max - min ) / 165 ) );
        g_cfg[ key ].set<int>( std::clamp( g_cfg[ key ].get<int>( ), min, max ) );
    }

    auto col = g_cfg[ key ].get<int>( ) > 0 ? ImColor( 255, 255, 255, i_menu_opacity ) : ImColor( 75, 75, 75, i_menu_opacity );

    auto value_str = std::to_string( g_cfg[ key ].get<int>( ) ) + suffix;
    auto value_str_size = ImGui::CalcTextSize( value_str.c_str( ) );

    renderer::text( next_item_pos.x + 1, next_item_pos.y, title, col, false );
    renderer::text( next_item_pos.x + 1 + 165 - value_str_size.x, next_item_pos.y, value_str, col, false );

    renderer::filled_rect( next_item_pos.x, next_item_pos.y + 17, 165, 6, ImColor( 32, 32, 32, i_menu_opacity ) );
    renderer::vert_gradient_filled_rect( { next_item_pos.x + 1, next_item_pos.y + 17, static_cast< float >( ( g_cfg[ key ].get<int>( ) * 165 ) / max ) - 1, 6 }, ImColor( 171, 171, 171, i_menu_opacity ), ImColor( 75, 75, 75, i_menu_opacity ) );

    renderer::rect( { next_item_pos.x, next_item_pos.y + 16, 165, 8 }, ImColor( 0, 0, 0, i_menu_opacity ) );

    next_item_pos.x -= 15;
    next_item_pos.y += 35;
}

void c_framework::slider( const std::string& title, const std::string& key, float min, float max, const std::string& suffix ) {
    next_item_pos.x += 15;

    auto i_menu_opacity = static_cast< int >( m_menu_opacity );

    static bool was_hovering = false;

    if ( m_mouse_state && !was_hovering )
        was_hovering = is_mouse_in_box( { next_item_pos.x + 1, next_item_pos.y + 16 }, { next_item_pos.x + 166, next_item_pos.y + 22 } );
    else
        was_hovering = false;

    if ( was_hovering && m_mouse_state ) {
        g_cfg[ key ].set<float>( float( ( m_old_mouse_pos.x - ( next_item_pos.x + 1 ) ) * ( max - min ) / 165 ) );
        g_cfg[ key ].set<float>( std::clamp( g_cfg[ key ].get<float>( ), min, max ) );
    }

    auto col = g_cfg[ key ].get<float>( ) > 0 ? ImColor( 255, 255, 255, i_menu_opacity ) : ImColor( 75, 75, 75, i_menu_opacity );

    auto value_str = tfm::format( "%.2f%s", g_cfg[ key ].get<float>( ), suffix );
    auto value_str_size = ImGui::CalcTextSize( value_str.c_str( ) );

    renderer::text( next_item_pos.x + 1, next_item_pos.y, title, col, false );
    renderer::text( next_item_pos.x + 1 + 165 - value_str_size.x, next_item_pos.y, value_str, col, false );

    renderer::filled_rect( next_item_pos.x, next_item_pos.y + 17, 165, 6, ImColor( 32, 32, 32, i_menu_opacity ) );
    renderer::vert_gradient_filled_rect( { next_item_pos.x + 1, next_item_pos.y + 17, static_cast< float >( ( g_cfg[ key ].get<float>( ) * 165 ) / max ) - 1, 6 }, ImColor( 171, 171, 171, i_menu_opacity ), ImColor( 75, 75, 75, i_menu_opacity ) );

    renderer::rect( { next_item_pos.x, next_item_pos.y + 16, 165, 8 }, ImColor( 0, 0, 0, i_menu_opacity ) );

    next_item_pos.x -= 15;
    next_item_pos.y += 35;
}

void c_framework::button( const std::string& title, std::function<void( void )> callback ) {
    auto i_menu_opacity = static_cast< int >( m_menu_opacity );

    renderer::vert_gradient_filled_rect( { next_item_pos.x + 11, next_item_pos.y, 167, 24 }, ImColor( 24, 24, 24, i_menu_opacity ), ImColor( 16, 16, 16, i_menu_opacity ) );

    auto hovered = is_mouse_in_box( { next_item_pos.x + 11, next_item_pos.y }, { next_item_pos.x + 11 + 167, next_item_pos.y + 24 } );
    auto col = hovered ? ImColor( 150, 150, 150, i_menu_opacity ) : ImColor( 75, 75, 75, i_menu_opacity );
    auto border_col = ImColor( 0, 0, 0, i_menu_opacity );

    if ( hovered && m_mouse_state ) {
        renderer::filled_rect( next_item_pos.x + 11, next_item_pos.y, 167, 24, ImColor( 23, 23, 23, i_menu_opacity ) );

        border_col = ImColor( 255, 255, 255, i_menu_opacity );
        col = ImColor( 255, 255, 255, i_menu_opacity );

        callback( );
    }

    renderer::rect( { next_item_pos.x + 11, next_item_pos.y, 167, 24 }, border_col );

    renderer::text( next_item_pos.x + 11 + 167 / 2, next_item_pos.y + 5, title, col, true );

    next_item_pos.y += 30;
}

bool c_framework::is_mouse_in_box( const vec2_t& p1, const vec2_t& p2 ) {
    if ( m_old_mouse_pos.x < p1.x || m_old_mouse_pos.y < p1.y )
        return false;

    if ( m_old_mouse_pos.x > p2.x || m_old_mouse_pos.y > p2.y )
        return false;

    return true;
}
