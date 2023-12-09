template<typename T>
struct vector2 {
	T x;
	T y;

	vector2( T x, T y ) : x( x ), y( y ) { }
	vector2( ) = default;
};

enum class item_type : int {
	toggle,
	int_value,
	float_value,
	tab,
};

struct sub_tab_t {
	const char* name;
	std::string var_name;

	item_type type;

	sub_tab_t( const char* name, std::string var_name, item_type type ) : name( name ), var_name( var_name ), type( type ) { }
};

struct tab_t {
	const char* name;
	std::string var_name;

	std::vector< sub_tab_t > subtabs;

	tab_t( const char* name, std::string var_name, std::vector< sub_tab_t > subtabs ) : name( name ), var_name( var_name ), subtabs( subtabs ) { }

	bool opened = false;

	void add_sub_tab( sub_tab_t sub_tab ) {
		subtabs.emplace_back( sub_tab );
	}

	void render_sub_tabs( int x, int y, int selected_sub_tab ) {
		auto height = subtabs.size( ) * 18.f;

		int max_width = ImGui::CalcTextSize( subtabs[ 0 ].name ).x;
		for ( int idx = 0; idx < subtabs.size( ); idx++ ) {
			auto width = ImGui::CalcTextSize( subtabs[ idx ].name ).x;
			if ( max_width < width )
				max_width = width;
		}

		max_width += 40;

		renderer::filled_rect( x, y, max_width, height, ImColor( 10, 10, 10, 255 ) );
		renderer::rect( x, y, max_width, height, ImColor( 158, 158, 158, 255 ) );

		for ( int idx = 0; idx < subtabs.size( ); idx++ ) {
			if ( selected_sub_tab == idx ) {
				renderer::filled_rect( x, y + idx * 18.f, max_width, 18.f, ImColor( 155, 155, 155, 255 ) );
			}

			renderer::text( x + 2.f, y + idx * 18.f, subtabs[ idx ].name, { 255, 255, 255, 255 }, false );

			if ( subtabs[ idx ].type == item_type::toggle ) {
				const ImVec2 check_mark_pts[ ] = {
					ImVec2 { x + ( max_width * 0.95f ), ( static_cast< float >( y ) - 3.f ) + idx * 18.f + 9.f},
					ImVec2 { x + ( max_width * 0.95f ) - 8, ( static_cast< float >( y ) + 4.f ) + idx * 18.f + 9.f},
					ImVec2 { x + ( max_width * 0.95f ) - 12,  ( static_cast< float >( y ) ) + idx * 18.f + 9.f}
				};

				if ( CONFIG_GET( subtabs[ idx ].var_name, bool ) )
					ImGui::GetForegroundDrawList( )->AddPolyline( check_mark_pts, 3, ImColor( 255, 255, 255, 255 ), false, 1.0f );
			}

			else if ( subtabs[ idx ].type == item_type::int_value )
				renderer::text( x + max_width * 0.80f, y + idx * 18.f, tfm::format( "%d", CONFIG_GET( subtabs[ idx ].var_name, int ) ), ImColor( 255, 255, 255, 255 ), false );
			else if ( subtabs[ idx ].type == item_type::float_value )
				renderer::text( x + max_width * 0.70f, y + idx * 18.f, tfm::format( "%.2f", CONFIG_GET( subtabs[ idx ].var_name, float ) ), ImColor( 255, 255, 255, 255 ), false );
		}
	}
};

class c_menu {
public:
	c_menu( ) = default;

	bool window( const char* title, vector2<float> window_pos );

	std::vector< tab_t > m_tabs;

	void poll_input( );
	void render_children( );

	void add_tab( tab_t tab ) {
		m_tabs.emplace_back( tab );
	}

	bool is_menu_open{ true };

	vector2<float> window_pos{ 50, 50 }, window_size{ 250, 85 };
	bool insert_state, old_insert_state;
private:
	int selected_index, selected_subtab_index;
};

extern c_menu menu;

namespace gui {
	void render( );
}

static bool wakeup{ false };

bool c_menu::window( const char* title, vector2<float> window_pos ) {
	if ( !wakeup ) {
		this->window_pos = window_pos;
		this->window_size = { 230, 85 };
		this->selected_subtab_index = -1;
		wakeup = true;
	}

	this->insert_state = GetAsyncKeyState( VK_INSERT );

	if ( this->insert_state && !this->old_insert_state )
		this->is_menu_open = !this->is_menu_open;

	if ( !this->is_menu_open )
		return false;

	renderer::filled_rect( this->window_pos.x, this->window_pos.y, this->window_size.x, this->window_size.y, ImColor( 10, 10, 10, 255 ) );
	renderer::filled_rect( this->window_pos.x, this->window_pos.y, this->window_size.x, 18.f, ImColor( 0, 0, 0, 150 ) );

	renderer::text( this->window_pos.x + 2.f, this->window_pos.y, title, ImColor( 255, 255, 255, 255 ), false );
	renderer::filled_rect( this->window_pos.x, this->window_pos.y + 18.f + this->selected_index * 18.f, this->window_size.x, 18.f, ImColor( 150, 150, 150, 220 ) );

	renderer::rect( this->window_pos.x, this->window_pos.y, this->window_size.x, this->window_size.y, ImColor( 158, 158, 158, 255 ) );

	return true;
}

void c_menu::poll_input( ) {
	if ( !is_menu_open )
		return;

	if ( selected_subtab_index == -1 ) {
		if ( GetAsyncKeyState( VK_UP ) & 1 ) {
			if ( selected_index < 1 )
				selected_index = m_tabs.size( ) - 1;
			else
				selected_index -= 1;
		}

		if ( GetAsyncKeyState( VK_DOWN ) & 1 ) {
			if ( selected_index > m_tabs.size( ) - 2 )
				selected_index = 0;
			else
				selected_index += 1;
		}
	}
	else {
		if ( GetAsyncKeyState( VK_UP ) & 1 ) {
			if ( selected_subtab_index < 1 )
				selected_subtab_index = m_tabs[ selected_index ].subtabs.size( ) - 1;
			else
				selected_subtab_index -= 1;
		}

		if ( GetAsyncKeyState( VK_DOWN ) & 1 ) {
			if ( selected_subtab_index > m_tabs[ selected_index ].subtabs.size( ) - 2 )
				selected_subtab_index = 0;
			else
				selected_subtab_index += 1;
		}

		if ( GetAsyncKeyState( VK_LEFT ) & 1 ) {
			selected_subtab_index = -1;
			g_cfg[ m_tabs[ selected_index ].var_name ].set<bool>( false );
		}

		if ( selected_subtab_index == -1 )
			return;

		auto safety_net = selected_subtab_index;

		if ( m_tabs[ selected_index ].subtabs[ safety_net ].type == item_type::toggle ) {
			if ( GetAsyncKeyState( VK_RIGHT ) & 1 ) {
				g_cfg[ m_tabs[ selected_index ].subtabs[ safety_net ].var_name ].set<bool>( !CONFIG_GET( m_tabs[ selected_index ].subtabs[ safety_net ].var_name, bool ) );
			}
		}
		else if ( m_tabs[ selected_index ].subtabs[ safety_net ].type == item_type::int_value ) {
			if ( GetAsyncKeyState( VK_PRIOR ) & 1 ) {
				g_cfg[ m_tabs[ selected_index ].subtabs[ safety_net ].var_name ].set<int>( CONFIG_GET( m_tabs[ selected_index ].subtabs[ safety_net ].var_name, int ) + 1 );
			}

			if ( GetAsyncKeyState( VK_NEXT ) & 1 ) {
				g_cfg[ m_tabs[ selected_index ].subtabs[ safety_net ].var_name ].set<int>( CONFIG_GET( m_tabs[ selected_index ].subtabs[ safety_net ].var_name, int ) - 1 );
			}
		}
		else if ( m_tabs[ selected_index ].subtabs[ safety_net ].type == item_type::float_value ) {
			if ( GetAsyncKeyState( VK_PRIOR ) & 1 ) {
				g_cfg[ m_tabs[ selected_index ].subtabs[ safety_net ].var_name ].set<float>( CONFIG_GET( m_tabs[ selected_index ].subtabs[ safety_net ].var_name, float ) + .1f );
			}

			if ( GetAsyncKeyState( VK_NEXT ) & 1 ) {
				g_cfg[ m_tabs[ selected_index ].subtabs[ safety_net ].var_name ].set<float>( CONFIG_GET( m_tabs[ selected_index ].subtabs[ safety_net ].var_name, float ) - .1f );
			}
		}
	}

	for ( auto idx{ 0 }; idx < m_tabs.size( ); idx++ ) {
		auto item = m_tabs[ idx ];

		if ( selected_subtab_index == -1 ) {
			if ( selected_index == idx && GetAsyncKeyState( VK_RIGHT ) & 1 ) {
				g_cfg[ item.var_name ].set<bool>( !g_cfg[ item.var_name ].get<bool>( ) );
				g_cfg[ item.var_name ].get<bool>( ) ? selected_subtab_index = idx : selected_subtab_index = -1;
			}
		}
	}
}

void c_menu::render_children( ) {
	if ( !is_menu_open )
		return;

	if ( m_tabs.empty( ) )
		return;

	constexpr const float text_y_offset = 18.f;

	for ( auto idx{ 0 }; idx < m_tabs.size( ); idx++ ) {
		auto item = m_tabs[ idx ];
		if ( g_cfg[ item.var_name ].get<bool>( ) ) {
			item.render_sub_tabs( this->window_pos.x + this->window_size.x, this->window_pos.y + text_y_offset + idx * 18.f, selected_subtab_index );
		}

		auto opened = CONFIG_GET( item.var_name, bool ) ? "+" : "-";
		renderer::text( window_pos.x + 2, window_pos.y + text_y_offset + idx * 18.f, tfm::format( "%s", item.name ), { 255, 255, 255, 255 }, false );
	}
}

c_menu menu;

//void render_active_items( ) {
//	const ImColor tab_colors[ ] = {
//		ImColor( 209, 209, 209, 255 ),
//		ImColor( 161, 161, 161, 255 ),
//		ImColor( 99, 99, 99, 255 ),
//		ImColor( 171, 171, 171, 255 ),
//		ImColor( 71, 71, 71, 255 ),
//	};
//
//	int y = 0.f;
//
//	for ( int idx{ 0 }; idx < menu.m_tabs.size( ); idx++ ) {
//		auto color = tab_colors[ idx ];
//		auto tab = menu.m_tabs[ idx ];
//
//		for ( int jdx{ 0 }; jdx < tab.subtabs.size( ); jdx++ ) {
//			auto sub_tab_item = tab.subtabs[ jdx ];
//			if ( sub_tab_item.type != item_type::toggle )
//				continue;
//
//			if ( CONFIG_GET( sub_tab_item.var_name, bool ) ) {
//				auto text_len = ImGui::CalcTextSize( sub_tab_item.name );
//				renderer::text( overlay::sc - text_len.x - 2, y, sub_tab_item.name, color, false );
//
//				y += 18.f;
//			}
//		}
//	}
//}

void gui::render( ) {

	auto menu_open = menu.window( _( "stellarix" ), vector2<float>( 50, 50 ) );

	menu.add_tab( tab_t( _( "aimbot" ), _( "stellarix:tab_aimbot" ), {
			sub_tab_t( _( "aimbot" ), _( "stellarix:aimbot_movement" ), item_type::toggle ),

			sub_tab_t( _( "aimbot fov: " ), _( "stellarix:aimbot_fov" ), item_type::int_value ),
			sub_tab_t( _( "ignore teammates" ), _( "stellarix:aimbot_ignore_team" ), item_type::toggle ),
			sub_tab_t( _( "draw fov" ), _( "stellarix:aimbot_draw_fov" ), item_type::toggle ),
		} ) );

	menu.add_tab( tab_t( _( "visuals" ), _( "stellarix:tab_esp" ), {
			sub_tab_t( _( "player name" ), _( "stellarix:player_name" ), item_type::toggle ),
			sub_tab_t( _( "player box" ), _( "stellarix:player_box" ), item_type::toggle ),
			sub_tab_t( _( "player health" ), _( "stellarix:player_health" ), item_type::toggle ),
			sub_tab_t( _( "player team" ), _( "stellarix:player_team" ), item_type::toggle ),
			sub_tab_t( _( "max player distance: " ), _( "stellarix:player_distance" ), item_type::int_value ),
		} ) );

	menu.add_tab( tab_t( _( "world" ), _( "stellarix:tab_item_esp" ), {
			
		} ) );

	menu.add_tab( tab_t( _( "misc" ), _( "stellarix:tab_misc" ), {
		
		} ) );

	//g_cfg[ _( "stellarix:aimbot_fov" ) ].set<int>( std::clamp( CONFIG_GET( _( "stellarix:aimbot_fov" ), int ), 0, 500 ) );
	//g_cfg[ _( "stellarix:player_distance" ) ].set<int>( std::clamp( CONFIG_GET( _( "stellarix:player_distance" ), int ), 0, 1000 ) );
	//g_cfg[ _( "stellarix:esp_items_min_price" ) ].set<int>( std::clamp( CONFIG_GET( _( "stellarix:esp_items_min_price" ), int ), 0, 2000 ) );
	//g_cfg[ _( "stellarix:esp_items_max_distance" ) ].set<int>( std::clamp( CONFIG_GET( _( "stellarix:esp_items_max_distance" ), int ), 0, 1000 ) );
	//g_cfg[ _( "stellarix:misc_timescale_modifier_val" ) ].set<float>( std::clamp( CONFIG_GET( _( "stellarix:misc_timescale_modifier_val" ), float ), 0.0f, 1.8f ) );
	//g_cfg[ _( "stellarix:misc_high_jump_height" ) ].set<float>( std::clamp( CONFIG_GET( _( "stellarix:misc_high_jump_height" ), float ), 0.0f, 9.0f ) );

	menu.poll_input( );
	menu.render_children( );

	int max_width = ImGui::CalcTextSize( menu.m_tabs[ 0 ].name ).x;
	for ( int idx = 0; idx < menu.m_tabs.size( ); idx++ ) {
		auto width = ImGui::CalcTextSize( menu.m_tabs[ idx ].name ).x;
		if ( max_width < width )
			max_width = width;
	}

	menu.window_size.x = max_width + 40;
	menu.window_size.y = 18.f + menu.m_tabs.size( ) * 18.f;

	menu.old_insert_state = menu.insert_state;

	if ( GetAsyncKeyState( VK_F2 ) )
		g_config_sys.save( );

	if ( GetAsyncKeyState( VK_F3 ) )
		g_config_sys.load( );

	//render_active_items( );

	menu.m_tabs.clear( );
}