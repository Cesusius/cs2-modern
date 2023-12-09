
vec3_t screen_size( 1920, 1080, 0 );

#define GET_NETVAR(type,name,address)  static const type name = address;
#define DECLARE_MEMBER(type, name, address) auto name() -> type { return driver->read<type>(reinterpret_cast<std::uintptr_t>(this) + address); }

#define DECRYPT_LIST_ENTRY(entity_list, entry)  (entity_list + 0x8 * ( ( entry & 0x7FFF ) >> 9 ) + 16 )

#define DECRYPT_BASE_PLAYER(list_entry, entry)  (list_entry + (120 * (entry & 0x1FF)))



enum bone_index : std::uint64_t {
	head = 6,
	neck_0 = 5,
	spine_1 = 4,
	spine_2 = 2,
	pelvis = 0,
	arm_upper_L = 8,
	arm_lower_L = 9,
	hand_L = 20,
	arm_upper_R = 13,
	arm_lower_R = 14,
	hand_R = 15,
	leg_upper_L = 22,
	leg_lower_L = 23,
	ankle_L = 24,
	leg_upper_R = 25,
	leg_lower_R = 26,
	ankle_R = 27,
};

namespace offsets {
	struct client_dll_t {
		GET_NETVAR( std::uintptr_t, entity_list, 0x178a808 );
		GET_NETVAR( std::uintptr_t, view_matrix, 0x18787e0 );
		GET_NETVAR( std::uintptr_t, local_player_controller, 0x17d9218 ); // 
	};
	struct c_base_player_controller_t {
		GET_NETVAR( std::uintptr_t, pawn_handle, 0x7FC );
	};
	struct base_entity_t {
		GET_NETVAR( std::uintptr_t, health, 0x32C );
		GET_NETVAR( std::uintptr_t, team, 0x3BF );
	};
	struct ccs_player_controller_t {
		GET_NETVAR( std::uintptr_t, name, 0x720 );
	};
	struct c_base_player_pawn_t {
		GET_NETVAR( std::uintptr_t, origin, 0x1214 );
		GET_NETVAR( std::uintptr_t, game_scene_node, 0x310 );
		GET_NETVAR( std::uintptr_t, model_state, 0x160 );
		GET_NETVAR( std::uintptr_t, eye_angles, 0x1500 ); // m_angEyeAngles 
	};
	struct c_bone_data {
		vec3_t Location;
		float Scale;
		float Rotation[ 4 ];
	};

	class c_model_state /* "client" */
	{
	public:
		unsigned char pad_0[ 0x80 ]; // 0x0 - 0xA0
		c_bone_data* m_boneArray;
		unsigned char pad_88[ 0x18 ];
		void* m_hModel; // 0xA0 - 0xA8
		const char* m_ModelName; // 0xA8 - 0xB0
		unsigned char pad_B0[ 0x38 ]; // 0xB0 - 0xE8
		bool m_bClientClothCreationSuppressed; // 0xE8 - 0xE9
		unsigned char pad_E9[ 0x97 ]; // 0xE9 - 0x180
		uint64_t m_MeshGroupMask; // 0x180 - 0x188
		unsigned char pad_188[ 0x9A ]; // 0x188 - 0x222
		int8_t m_nIdealMotionType; // 0x222 - 0x223
		int8_t m_nForceLOD; // 0x223 - 0x224
		int8_t m_nClothUpdateFlags; // 0x224 - 0x225
		unsigned char pad_225[ 0xB ]; // 0x225 - 0x230
	}; // size - 0x2
}




class c_local_entity {
public:
	DECLARE_MEMBER( std::uint32_t, get_local_health, offsets::base_entity_t::health );
	DECLARE_MEMBER( std::uint32_t, get_local_team, offsets::base_entity_t::team );
	DECLARE_MEMBER( vec3_t, get_origin, offsets::c_base_player_pawn_t::origin );
};

class c_base_player_pawn {
public:		
	DECLARE_MEMBER( std::uint32_t, get_health, offsets::base_entity_t::health );
	DECLARE_MEMBER( std::uint32_t, get_team, offsets::base_entity_t::team );
	DECLARE_MEMBER( vec3_t, get_origin, offsets::c_base_player_pawn_t::origin );
	DECLARE_MEMBER( vec3_t, eye_angles, offsets::c_base_player_pawn_t::eye_angles );  

	auto get_bone_id( int bone_id ) -> vec3_t {
		auto skeleton_instance = driver->read<std::uintptr_t>( reinterpret_cast<std::uintptr_t>( this ) + offsets::c_base_player_pawn_t::game_scene_node );

		auto model_state = driver->read<offsets::c_model_state>( skeleton_instance + offsets::c_base_player_pawn_t::model_state );
		if ( !model_state.m_boneArray )
			return vec3_t( 0.f, 0.f, 0.f );

		auto bone_data = driver->read<offsets::c_bone_data>( reinterpret_cast< std::uintptr_t >( &model_state.m_boneArray[ bone_id ] ) );
		return bone_data.Location;
	}
};

class c_base_player_controller {
public:	
	DECLARE_MEMBER( c_base_player_pawn*, get_pawn_handle, offsets::c_base_player_controller_t::pawn_handle );
	DECLARE_MEMBER( std::uintptr_t, get_name, offsets::ccs_player_controller_t::name );
};

struct info_t {
	c_base_player_pawn* pawn;
	c_local_entity* local_entity;
	c_base_player_controller* base_player;
};

struct game_t {
	std::vector<info_t>entitys;
	std::vector<info_t>temp_entitys;
	std::mutex sync{ };
};

game_t c_game;

auto read_entitys( ) -> void {
		c_game.temp_entitys.clear( );
			
		view_matrix_t view_matrix = driver->read<view_matrix_t>( driver->client_dll + offsets::client_dll_t::view_matrix );

		const auto entity_list = driver->read<std::uintptr_t>( driver->client_dll + offsets::client_dll_t::entity_list );

		auto local_player_controller = driver->read<c_base_player_controller*>( driver->client_dll + offsets::client_dll_t::local_player_controller );
		if ( !local_player_controller )
			return;
	
		auto local_pawn_handle = reinterpret_cast< std::uint32_t>( local_player_controller->get_pawn_handle( ) );

		auto local_list_entry = driver->read<std::uintptr_t>( DECRYPT_LIST_ENTRY( entity_list, local_pawn_handle ) );
		auto local_player = driver->read<c_local_entity*>( DECRYPT_BASE_PLAYER( local_list_entry, local_pawn_handle ) );
		
		for ( auto i = 0; i < 64; ++i ) {
			auto list_entry = driver->read<std::uintptr_t>( DECRYPT_LIST_ENTRY( entity_list, i) );
			if ( !list_entry )
				continue;
			
			auto base_player_controller = driver->read<c_base_player_controller*>( DECRYPT_BASE_PLAYER( list_entry, i ) );
			if ( !base_player_controller )
				continue;
			
			auto pawn_handle = reinterpret_cast<std::uint32_t>( base_player_controller->get_pawn_handle( ) );
			

			auto pawn_list_entry = driver->read<std::uintptr_t>( DECRYPT_LIST_ENTRY( entity_list, pawn_handle ) );
			auto pawn = driver->read<c_base_player_pawn*>( DECRYPT_BASE_PLAYER( pawn_list_entry, pawn_handle  ) );
			if ( !pawn )
				continue;



			info_t ent;
			c_game.sync.lock( );
			ent.base_player = base_player_controller;
			ent.local_entity = local_player;
			ent.pawn = pawn;
			c_game.sync.unlock( );
			c_game.temp_entitys.push_back( ent );
		}
		c_game.entitys = c_game.temp_entitys;
		std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
}

auto esp_loop( ) -> void {
	c_game.sync.lock( );
	for ( auto players : c_game.entitys ) {
		auto pawn_health = players.pawn->get_health( );

		view_matrix_t view_matrix = driver->read<view_matrix_t>( driver->client_dll + offsets::client_dll_t::view_matrix );

		if ( !pawn_health )
			continue;

		if ( reinterpret_cast< std::uintptr_t >( players.pawn ) == reinterpret_cast< std::uintptr_t >( players.local_entity ) )
			continue;

		if ( CONFIG_GET( _( "stellarix:player_team" ), bool ) ) {
			/* ddd */
		}
		else if ( players.pawn->get_team( ) == players.local_entity->get_local_team( ) ) {
			continue;
		}
			
		
		std::string player_name = "empty";
		auto player_name_addr = players.base_player->get_name( );
		if ( player_name_addr ) {
			char buffer[ 256 ];
			driver->read( player_name_addr, buffer, sizeof( buffer ) );
			player_name = std::string( buffer );
		}


		auto origin = players.pawn->get_origin( );
		auto head_pos = players.pawn->get_bone_id( 6 );
		auto eye_angle = players.pawn->eye_angles( );

		vec3_t screen_top, screen_bottom;

		float Length = 50.f;
		float LineLength = cos( eye_angle.x * M_PI / 180 ) * Length;
		vec3_t LoSEndPoint;
		LoSEndPoint.x = head_pos.x + cos( eye_angle.y * M_PI / 180 ) * LineLength;
		LoSEndPoint.y = head_pos.y + sin( eye_angle.y * M_PI / 180 ) * LineLength;
		LoSEndPoint.z = head_pos.z - sin( eye_angle.x * M_PI / 180 ) * Length;

		vec3_t screen_head, screen_endpoint;
		if ( world_to_screen( screen_size, head_pos, screen_head, view_matrix ) &&
			world_to_screen( screen_size, LoSEndPoint, screen_endpoint, view_matrix ) ) {
			
			if ( CONFIG_GET( _( "stellarix:player_los" ), bool ) ) {
				renderer::line( screen_head.x, screen_head.y, screen_endpoint.x, screen_endpoint.y, ImColor( 255, 0, 0, 255 ) );
			}
		}


		if ( !world_to_screen( screen_size, origin, screen_bottom, view_matrix ) || !world_to_screen( screen_size, head_pos, screen_top, view_matrix ) )
			continue;

		int box_height = screen_bottom.y - screen_top.y;
		int box_width = box_height / 2;

		int main_box_width = box_width;
		int main_box_height = box_height;
		int main_box_left = screen_top.x - ( main_box_width / 2 );
		int main_box_top = screen_top.y;


		int head_box_size = box_height / 8;
		int head_box_left = screen_top.x - ( head_box_size / 2 );
		int head_box_top = screen_top.y;
		int head_box_width = head_box_size;
		int head_box_height = head_box_size;


		float max_health = 100.0f;
		float health = pawn_health;
		float health_precent = ( health / max_health ) * 100.0f;

		// Draw health bar
		int healthBarWidth = box_width / 50;
		int healthBarHeight = main_box_height;
		int healthBarLeft = main_box_left - healthBarWidth;
		int healthBarTop = main_box_top;


		ImColor healthBarColor(
			static_cast< int >( 255 * ( 1.0f - ( health_precent / 100.0f ) ) ),
			static_cast< int >( 255 * ( health_precent / 100.0f ) ),
			0,
			255
		);

		if ( CONFIG_GET( _( "stellarix:player_box" ), bool ) ) {
			renderer::rect( main_box_left, main_box_top, main_box_width, main_box_height, ImColor( 134, 247, 225, 255 ) );
		}

		if ( CONFIG_GET( _( "stellarix:player_headbox" ), bool ) ) {
			renderer::text( screen_top.x, screen_top.y, "x", ImColor( 255, 0, 0, 255 ), false );
		}
		
		if ( CONFIG_GET( _( "stellarix:player_name" ), bool ) ) {
			renderer::text( screen_top.x + ( box_width / 2 + 10 ), screen_top.y, player_name.c_str( ), ImColor( 255, 255, 0 ), false );
		}

		if ( CONFIG_GET( _( "stellarix:player_health" ), bool ) ) {
			renderer::rect( healthBarLeft, healthBarTop, healthBarWidth, healthBarHeight, healthBarColor );
		}
		
	}
	c_game.sync.unlock( );
}