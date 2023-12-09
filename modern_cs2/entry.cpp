#include <vector>
#include <Windows.h>
#include <functional>
#include <string>
#include <TlHelp32.h>
#include <iostream>
#include <d3d9.h>
#include <dwmapi.h>
#include <d3dx9math.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>
#include <winternl.h>

#pragma comment (lib, "ntdll.lib")

#include "driver/driver.hpp"
#include "math/math.hpp"

#include "render/Overlay/tinyformat.hpp"

#include "render/Fonts/drawing_font.hpp"
#include "render/Fonts/icon_font.hpp"
#include "render/Fonts/menu_font.hpp"

#include "render/Overlay/xorstr.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_win32.h"

#include "render/Config/config.hpp"

#include "render/Renderer.hpp"
#include "render/Overlay/Overlay.hpp"

#include "render/menu/ui.hpp"
#include "render/menu/menu.hpp"

#include "source_engine/engine.hpp"
bool should_use_wmp = true;
DWORD process_id{ };
bool show_menu;
enum class Tab { Aimbot, Visuals, Misc };
Tab currentTab = Tab::Aimbot; 


auto overlay_callback( ) -> void {

	renderer::outline_text( 10, 10, tfm::format( "Stellarix -- fps: %.2f", ImGui::GetIO( ).Framerate ), ImColor( 255, 255, 255, 255 ), false );

	read_entitys( );;
	esp_loop( );
render_menu:

	ImGui::PushFont( m_menu_font );
	{

		framework.begin_window( "Stellarix", { 600, 430 } );
		auto tab = framework.add_tabs( { "combat", "visuals", "misc", "config" } );
		if ( tab == 0 ) {
			framework.add_sub_tabs( { "Aimbot" } );

			framework.add_combobox( "General", { 15, 60 }, { 280, 340 } );
			{
				framework.checkbox( "Aimbot", _( "stellarix:aimbot_movement" ) );
				framework.slider( "Field of view", _( "stellarix:aimbot_fov" ), 0, 1000, "px" );
				
			}

			framework.add_combobox( "Indicators", { 305, 250 }, { 280, 150 } );
			{
				framework.checkbox( "Draw fov", _( "stellarix:aimbot_draw_fov" ) );
			}
		}

		if ( tab == 1 ) {
			int sub_tab = framework.add_sub_tabs( { "Players", "World" } );
			if ( sub_tab == 0 ) {
				framework.add_combobox( "Main", { 15, 60 }, { 280, 340 } );
				{
					framework.checkbox( "Bounding box", _( "stellarix:player_box" ) );
					framework.checkbox( "Name", _( "stellarix:player_name" ) );
					framework.checkbox( "Draw Team", _( "stellarix:player_team" ) );
					framework.checkbox( "Health", _( "stellarix:player_health" ) );
					framework.checkbox( "Head Box", _( "stellarix:player_headbox" ) );
					framework.checkbox( "Line Of Sight", _( "stellarix:player_los" ) );

					framework.slider( "Max player distance", _( "stellarix:player_distance" ), 0, 1000, "m" );
				}

				framework.add_combobox( "Radar", { 305, 60 }, { 280, 340 } );
				{

				}
			}
		
		}

		if ( tab == 2 ) {
			framework.add_sub_tabs( { "Miscellaneous" } );

		}

		if ( tab == 3 ) {
			framework.add_sub_tabs( { _( "" ) } );

			framework.add_combobox( _( "Config" ), { 15, 60 }, { 219, 180 } );
			{
				framework.button( _( "Save config" ), [ ]( ) {
					g_config_sys.save( );
					} );

				framework.button( _( "Load config" ), [ ]( ) {
					g_config_sys.load( );
					} );
			}
		}

		framework.end_window( );

		//gui::render( );
	} ImGui::PopFont( );

	return;
}

auto real_entry( ) -> void {
	HWND hWnd = GetConsoleWindow( );
	ShowWindow( hWnd, SW_SHOW );
	driver = std::make_unique<Driver>( "cs2.exe" );
	{
		driver->SetTargetCr3( );
		driver->client_dll = driver->GetModuleInformation_x64( "client.dll" );
		std::cout << driver->client_dll << std::endl; 

		char program_files[ MAX_PATH ], windows_directory[ MAX_PATH ];

		random_seed = GetTickCount( );

		auto directory_exists = [ & ]( const std::string& dir_path ) -> bool {
			auto attr = GetFileAttributesA( dir_path.c_str( ) );
			if ( attr == INVALID_FILE_ATTRIBUTES )
				return false;

			if ( attr & FILE_ATTRIBUTE_DIRECTORY )
				return true;

			return false;
		};

		if ( SHGetSpecialFolderPathA( 0, program_files, CSIDL_PROGRAM_FILES, FALSE ) ) {
			if ( directory_exists( std::string( program_files ) + ( "\\Windows Media Player\\" ) ) )
				printf( "1\n" );
			should_use_wmp = true;
		}

		auto explorer = OpenProcess( PROCESS_ALL_ACCESS, false, driver->get_process( "explorer.exe" ) );

		if ( should_use_wmp ) {
			auto path = std::string( program_files ) + ( "\\Windows Media Player\\wmplayer.exe" );
			auto wmplayer = process::start_process( explorer, path.c_str( ) );
			printf( "2\n" );
		}

		while ( !GetAsyncKeyState( VK_F1 ) )
			std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );

		auto target_hwnd = FindWindowA( ( "SDL_app" ), nullptr );
		while ( !target_hwnd )
			target_hwnd = FindWindowA( ( "SDL_app" ), nullptr );

		GetWindowThreadProcessId( target_hwnd, &process_id );

		if ( overlay::setup( should_use_wmp ) ) {
			overlay::run_loop( overlay_callback );
		}

		if ( overlay::m_d3d_device != NULL ) {
			overlay::m_d3d_device->EndScene( );
			overlay::m_d3d_device->Release( );
		}

		if ( overlay::m_d3d != NULL )
			overlay::m_d3d->Release( );

		DWORD process_id{ };
		GetWindowThreadProcessId( overlay::m_overlay_hwnd, &process_id );
		const auto proc = OpenProcess( PROCESS_TERMINATE, false, process_id );
		TerminateProcess( proc, 1 );
		CloseHandle( proc );

	}
	std::cin.get( );
}

int main( ) {
	real_entry( );
	return 0;
}
