inline int random_seed;
inline int recreated_rand( ) {
	random_seed = ( ( random_seed * 1103515245 + 12345 ) & 0x7fffffff );
	return random_seed;
};


#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shlwapi.lib")
//winmm.lib
//shlwapi.lib

inline ImFont* m_drawing_font;
inline ImFont* m_menu_font;
inline ImFont* m_icon_font;

bool wants_unload = false;

namespace overlay {
	inline HWND m_overlay_hwnd;

	inline vec3_t m_w2s_size;

	inline vec3_t m_game_size;
	inline vec3_t m_game_pos;

	inline vec3_t m_window_pos;
	inline vec3_t m_window_size;

	inline IDirect3D9Ex* m_d3d;
	inline IDirect3DDevice9* m_d3d_device;
	inline D3DPRESENT_PARAMETERS m_present_params;

	bool setup( bool using_wmp );
	void run_loop( std::function<void( void )> callback );
}

static bool setup_viewport{ false };

bool overlay::setup( bool using_wmp ) {
	if ( using_wmp ) {
		auto target_wnd = FindWindowA( ( "ms_sqlce_se_notify_wndproc" ), nullptr ); // win media player...
		if ( !target_wnd )
			target_wnd = FindWindowA( ( "WMPlayerApp" ), nullptr );

		m_overlay_hwnd = target_wnd;
	}
	else {
		return false;
		//auto target_wnd = FindWindowA( _( "" ), nullptr ); // use nvidia.
		//if ( !target_wnd )
		//	return false;
	}

	RECT desktop_rect{ };
	GetWindowRect( GetDesktopWindow( ), &desktop_rect );
	//
	overlay::m_window_pos = vec3_t( 0.f,0.f,0.f );
	overlay::m_window_size.x = desktop_rect.right - desktop_rect.left;
	overlay::m_window_size.y = desktop_rect.bottom - desktop_rect.top;

	if ( !SetMenu( m_overlay_hwnd, nullptr ) )
		return false;

	if ( !SetWindowLongPtrA( m_overlay_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE ) )
		return false;

	if ( !SetWindowLongPtrA( m_overlay_hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT ) )
		return false;

	if ( !SetWindowPos( m_overlay_hwnd, HWND_TOPMOST, overlay::m_window_pos.x, overlay::m_window_pos.y, overlay::m_window_size.x, overlay::m_window_size.y, 0 ) )
		return false;

	if ( !SetLayeredWindowAttributes( m_overlay_hwnd, 0, 255, LWA_ALPHA ) )
		return false;

	MARGINS margin = { 0, 0, desktop_rect.bottom, desktop_rect.right };
	if ( DwmExtendFrameIntoClientArea( m_overlay_hwnd, &margin ) != S_OK )
		return false;

	if ( FAILED( Direct3DCreate9Ex( D3D_SDK_VERSION, &overlay::m_d3d ) ) )
		return false;

	overlay::m_present_params = { 0 };
	overlay::m_present_params.Windowed = TRUE;
	overlay::m_present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	overlay::m_present_params.hDeviceWindow = overlay::m_overlay_hwnd;
	overlay::m_present_params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	overlay::m_present_params.BackBufferFormat = D3DFMT_A8R8G8B8;
	overlay::m_present_params.BackBufferWidth = overlay::m_window_size.x;
	overlay::m_present_params.BackBufferHeight = overlay::m_window_size.y;
	overlay::m_present_params.EnableAutoDepthStencil = TRUE;
	overlay::m_present_params.AutoDepthStencilFormat = D3DFMT_D16;
	overlay::m_present_params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	overlay::m_present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	auto res = overlay::m_d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, overlay::m_overlay_hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &overlay::m_present_params, &overlay::m_d3d_device );

	if ( FAILED( res ) ) {
		overlay::m_d3d_device->Release( );
		return false;
	}

	ImGui::CreateContext( );

	m_drawing_font = ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( drawing_font, sizeof( drawing_font ), 10.f );
	m_menu_font = ImGui::GetIO( ).Fonts->AddFontFromFileTTF( "C:\\Windows\\Fonts\\Verdana.ttf", 12.f );
	//ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( menu_font, sizeof( menu_font ), 16.f );
	m_icon_font = ImGui::GetIO( ).Fonts->AddFontFromMemoryTTF( icon_font, sizeof( icon_font ), 28.f );

	ImGui_ImplWin32_Init( overlay::m_overlay_hwnd );
	ImGui_ImplDX9_Init( overlay::m_d3d_device );
	ImGui_ImplDX9_CreateDeviceObjects( );

	overlay::m_d3d->Release( );

	return true;
}

void overlay::run_loop( std::function<void( void )> callback ) {
	while ( FindWindowA( "SDL_app", 0 ) ) {
		if ( wants_unload )
			break;

		if ( GetAsyncKeyState( VK_F6 ) & 1 ) {
			wants_unload = true;
			return;
		}

		auto frame = GetSystemMetrics( SM_CXSIZEFRAME );
		auto y_border = GetSystemMetrics( SM_CYBORDER );
		auto x_modifier = ( GetSystemMetrics( SM_CXBORDER ) + frame );
		auto y_modifier = ( y_border + GetSystemMetrics( SM_CYCAPTION ) + frame );

		RECT rect;
		GetWindowRect( FindWindowA( "SDL_app", 0), &rect );
		overlay::m_game_pos.x = 0 - overlay::m_window_pos.x;
		overlay::m_game_pos.y = 0 - overlay::m_window_pos.y;
		overlay::m_game_size.x = 1920;
		overlay::m_game_size.y = 1080;

		overlay::m_w2s_size = vec3_t{
			( static_cast< float >( overlay::m_game_size.x ) / 2.0f ),
			( static_cast< float >( overlay::m_game_size.y ) / 2.0f ),
			( static_cast< float >( overlay::m_game_size.z ) / 2.0f )
		};

		auto& io = ImGui::GetIO( );
		io.ImeWindowHandle = overlay::m_overlay_hwnd;

		ImGui_ImplWin32_NewFrame( );
		ImGui_ImplDX9_NewFrame( );

		ImGui::NewFrame( );

		callback( );

		UpdateWindow( overlay::m_overlay_hwnd );

		ImGui::EndFrame( );

		overlay::m_d3d_device->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 );
		overlay::m_d3d_device->BeginScene( );

		ImGui::Render( );

		ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );

		overlay::m_d3d_device->SetRenderState( D3DRS_SCISSORTESTENABLE, false );
		overlay::m_d3d_device->SetRenderState( D3DRS_ZENABLE, false );
		overlay::m_d3d_device->SetRenderState( D3DRS_ALPHABLENDENABLE, false );

		overlay::m_d3d_device->EndScene( );

		auto result = overlay::m_d3d_device->Present( NULL, NULL, NULL, NULL );
		if ( result == D3DERR_DEVICELOST && overlay::m_d3d_device->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET ) {
			ImGui_ImplDX9_InvalidateDeviceObjects( );
			overlay::m_d3d_device->Reset( &overlay::m_present_params );
			ImGui_ImplDX9_CreateDeviceObjects( );
		}
	}
}

namespace process {
	

	HANDLE start_process( HANDLE parent, const char* path ) {
		SIZE_T size;
		STARTUPINFOEX si = {};
		PROCESS_INFORMATION pi = {};

		InitializeProcThreadAttributeList( nullptr, 1, 0, &size );
		si.lpAttributeList = reinterpret_cast< LPPROC_THREAD_ATTRIBUTE_LIST >( HeapAlloc( GetProcessHeap( ), 0, size ) );
		InitializeProcThreadAttributeList( si.lpAttributeList, 1, 0, &size );
		UpdateProcThreadAttribute( si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &parent, sizeof( parent ), nullptr, nullptr );

		si.StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		si.StartupInfo.wShowWindow = SW_HIDE;
		si.StartupInfo.cb = sizeof( STARTUPINFO );
		CreateProcessA( path, nullptr, nullptr, nullptr, false, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, reinterpret_cast<STARTUPINFO*>( &si ), &pi );
		Sleep( 500 );

		HeapFree( GetProcessHeap( ), 0, si.lpAttributeList );
		CloseHandle( pi.hThread );
		return pi.hProcess;
	}
}