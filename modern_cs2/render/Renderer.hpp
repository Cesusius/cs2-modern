constexpr const double pi = 3.14159265359;
constexpr const float m_pi = 3.14159265358979323846f;
constexpr const float m_radpi = 180.0 / m_pi;

namespace renderer {
	struct rect_t {
		float x, y, w, h;
	};

	inline void rect( rect_t pos, ImColor col ) {
		ImGui::GetForegroundDrawList( )->AddRect( ImVec2( pos.x, pos.y ), ImVec2( pos.x + pos.w, pos.y + pos.h ), col );
	}

	inline void rounded_rect( rect_t pos, ImColor col ) {
		ImGui::GetForegroundDrawList( )->AddRect( ImVec2( pos.x, pos.y ), ImVec2( pos.x + pos.w, pos.y + pos.h ), col, 3.f );
	}

	inline void vert_gradient_filled_rect( rect_t pos, ImColor col1, ImColor col2 ) {
		ImGui::GetForegroundDrawList( )->AddRectFilledMultiColor( ImVec2( pos.x, pos.y ), ImVec2( pos.x + pos.w, pos.y + pos.h ), col1, col1, col2, col2 );
	}

	inline void gradient_filled_rect( rect_t pos, ImColor col1, ImColor col2 ) {
		ImGui::GetForegroundDrawList( )->AddRectFilledMultiColor( ImVec2( pos.x, pos.y ), ImVec2( pos.x + pos.w, pos.y + pos.h ), col1, col2, col2, col1 );
	}

	inline void filled_rounded_rect( rect_t pos, ImColor col ) {
		ImGui::GetForegroundDrawList( )->AddRectFilled( ImVec2( pos.x, pos.y ), ImVec2( pos.x + pos.w, pos.y + pos.h ), col, 3.f );
	}

	inline void filled_rect( float x, float y, float w, float h, ImColor col ) {
		ImGui::GetForegroundDrawList( )->AddRectFilled( ImVec2( x, y ), ImVec2( x + w, y + h ), col );
	}

	inline void rect( float x, float y, float w, float h, ImColor col ) {
		ImGui::GetForegroundDrawList( )->AddRect( ImVec2( x, y ), ImVec2( x + w, y + h ), col );
	}

	inline void line( float x1, float y1, float x2, float y2, ImColor col ) {
		ImGui::GetForegroundDrawList( )->AddLine( ImVec2( x1, y1 ), ImVec2( x2, y2 ), col );
	}

	inline void text( float x, float y, std::string text, ImColor col, bool centered ) {

		auto text_sz = ImGui::CalcTextSize( text.c_str( ) );
		ImGui::GetForegroundDrawList( )->AddText( ImVec2( x -= centered ? text_sz.x / 2 : 0, y ), col, text.c_str( ) );
	}

	inline void outline_text( float x, float y, std::string text, ImColor col, bool centered ) {
		renderer::text( x + 1, y + 1, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );
		renderer::text( x - 1, y - 1, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );
		renderer::text( x + 1, y - 1, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );
		renderer::text( x - 1, y + 1, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );

		renderer::text( x + 1, y, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );
		renderer::text( x - 1, y, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );
		renderer::text( x, y - 1, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );
		renderer::text( x, y + 1, text.c_str( ), ImColor( 0, 0, 0, static_cast< int >( col.Value.w * 255 ) ), centered );

		renderer::text( x, y, text, col, centered );
	}

	inline void draw_arc( float x, float y, float radius, float start_angle, float percent, float thickness, ImColor color ) {
		auto precision = ( 2.f * pi ) / 30.f;
		auto step = pi / 180.f;
		auto inner = radius - thickness;
		auto end_angle = ( start_angle + percent ) * step;
		auto start_angles = ( start_angle * pi ) / 180.f;

		for ( ; radius > inner; --radius ) {
			for ( auto angle = start_angles; angle < end_angle; angle += precision ) {
				auto cx = std::round( x + radius * std::cos( angle ) );
				auto cy = std::round( y + radius * std::sin( angle ) );

				auto cx2 = std::round( x + radius * std::cos( angle + precision ) );
				auto cy2 = std::round( y + radius * std::sin( angle + precision ) );

				ImGui::GetForegroundDrawList( )->AddLine( ImVec2( cx, cy ), ImVec2( cx2, cy2 ), color );
			}
		}
	}
}