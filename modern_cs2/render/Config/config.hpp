#pragma once
#include <map>
#include <string>

#define CONFIG_GET( token, type ) g_cfg[ token ].get<type>()

struct cfg_value_t {
	template<typename t = bool>
	t get( ) {
		return ( t )( m_value );
	}

	template <typename t = bool>
	void set( t in ) {
		m_value = ( double )( in );
	}

	double m_value = 0.0; // for size
};

extern std::map< std::string, cfg_value_t > g_cfg;

class cfg_system {
public:
	void init( );

	void save( std::string name = "config.ini" );
	void load( std::string name = "config.ini" );
private:
	bool m_setup{ };
	std::string m_path{ };
};

inline cfg_system g_config_sys;

cfg_value_t pre_defined;

std::map< std::string, cfg_value_t > g_cfg{
	{ _( "stellarix:tab_aimbot" ), pre_defined },
	{ _("stellarix:aimbot_movement"), pre_defined },

	{ _( "stellarix:aimbot_fov" ), pre_defined },
	{ _( "stellarix:aimbot_ignore_team" ), pre_defined },
	{ _( "stellarix:aimbot_draw_fov" ), pre_defined },

	{ _( "stellarix:tab_esp" ), pre_defined },
	{ _( "stellarix:player_name" ), pre_defined },
	{ _( "stellarix:player_headbox" ), pre_defined },
	{ _( "stellarix:player_los" ), pre_defined },
	{ _( "stellarix:player_team" ), pre_defined },
	{ _( "stellarix:player_health" ), pre_defined },
	{ _( "stellarix:player_box" ), pre_defined },
	{ _( "stellarix:player_distance" ), pre_defined },

};

void cfg_system::init( ) {
	m_setup = false;

	m_path.resize( MAX_PATH + 1 );

	if ( !SUCCEEDED( SHGetFolderPathA( 0, CSIDL_PERSONAL, 0, SHGFP_TYPE_CURRENT, ( LPSTR )m_path.data( ) ) ) )
		return;

	PathAppendA( ( char* )m_path.c_str( ), _( "stellarix" ) );

	CreateDirectoryA( m_path.c_str( ), 0 );

	m_setup = true;
}

void cfg_system::save( std::string name ) {
	if ( !m_setup )
		return;

	std::string file;
	file = tfm::format( _( "%s\\%s" ), m_path.data( ), name.data( ) );

	for ( auto& e : g_cfg )
		WritePrivateProfileStringA( _( "ot" ), e.first.c_str( ), std::to_string( e.second.get<double>( ) ).c_str( ), file.c_str( ) );
}

void cfg_system::load( std::string name ) {
	if ( !m_setup )
		return;

	std::string file;
	file = tfm::format( _( "%s\\%s" ), m_path.data( ), name.data( ) );

	for ( auto& e : g_cfg ) {
		char value[ 64 ] = { '\0' };

		GetPrivateProfileStringA( _( "ot" ), e.first.c_str( ), "", value, 64, file.c_str( ) );

		e.second.set<double>( atof( value ) );
	}
}