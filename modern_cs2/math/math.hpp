#define M_PI       3.14159265358979323846 
struct vec2_t {
	float x, y;

	vec2_t( ) : x( 0 ), y( 0 ) { }
	vec2_t( float x, float y ) : x( x ), y( y ) { }

	vec2_t operator+( const vec2_t& vector ) const { return vec2_t( x + vector.x, y + vector.y ); }
	vec2_t operator-( const vec2_t& v ) const { return vec2_t( x - v.x, y - v.y ); }
	vec2_t operator-( ) const { return vec2_t( -1.f * x, -1.f * y ); }
	vec2_t operator*( float number ) const { return vec2_t( x * number, y * number ); }
	vec2_t operator/( float number ) const { return vec2_t( x / number, y / number ); }

	float length( ) const { return sqrt( ( x * x ) + ( y * y ) ); }
	float distance( vec2_t v ) { return ( *this - v ).length( ); }

	void normalize( ) {
		if ( this->y > 89 )
			this->y = 89;
		if ( this->y < -89 )
			this->y = -89;
		while ( this->x > 180 )
			this->x -= 360;
		while ( this->x < -180 )
			this->x += 360;
	}
};

struct vec3_t {
	float x, y, z;

	vec3_t( ) : x( 0 ), y( 0 ), z( 0 ) { }
	vec3_t( float x, float y, float z ) : x( x ), y( y ), z( z ) { }

	vec3_t operator+( const vec3_t& v ) const { return vec3_t( x + v.x, y + v.y, z + v.z ); }
	vec3_t operator-( const vec3_t& v ) { return vec3_t( x - v.x, y - v.y, z - v.z ); }

	bool operator!=( const vec3_t& vector ) const { return x != vector.x || y != vector.y || z != vector.z; }

	float length( ) { return sqrt( ( x * x ) + ( y * y ) + ( z * z ) ); }
	float distance( vec3_t Vec ) { return ( *this - Vec ).length( ); }

	vec3_t normalized( ) { return vec3_t( x / length( ), y / length( ), z / length( ) ); }
};

struct vec4_t {
	float x, y, z, w;
};

struct view_matrix_t {
	float* operator[ ]( int index ) {
		return matrix[ index ];
	}

	float matrix[ 4 ][ 4 ];
};


bool world_to_screen( const vec3_t& screen_size, const vec3_t& pos, vec3_t& out, view_matrix_t matrix ) {
	out.x = matrix[ 0 ][ 0 ] * pos.x + matrix[ 0 ][ 1 ] * pos.y + matrix[ 0 ][ 2 ] * pos.z + matrix[ 0 ][ 3 ];
	out.y = matrix[ 1 ][ 0 ] * pos.x + matrix[ 1 ][ 1 ] * pos.y + matrix[ 1 ][ 2 ] * pos.z + matrix[ 1 ][ 3 ];

	float w = matrix[ 3 ][ 0 ] * pos.x + matrix[ 3 ][ 1 ] * pos.y + matrix[ 3 ][ 2 ] * pos.z + matrix[ 3 ][ 3 ];

	if ( w < 0.01f )
		return false;

	float inv_w = 1.f / w;
	out.x *= inv_w;
	out.y *= inv_w;

	float x = screen_size.x * .5f;
	float y = screen_size.y * .5f;

	x += 0.5f * out.x * screen_size.x + 0.5f;
	y -= 0.5f * out.y * screen_size.y + 0.5f;

	out.x = x;
	out.y = y;

	return true;
}


namespace math {
	constexpr const double pi = 3.14159265359;
	constexpr const float m_pi = 3.14159265358979323846f;
	constexpr const float m_radpi = 180.0 / m_pi;

	__forceinline float deg_to_rad( float x ) {
		return x * ( ( float )m_pi / 180.f );
	}

	__forceinline float rad_to_deg( float x ) {
		return x * ( 180.f / ( float )m_pi );
	}

	__forceinline vec2_t calc_angle( vec3_t& from, vec3_t& to ) {
		vec3_t delta = from - to;
		float length = delta.length( );
		return vec2_t{ rad_to_deg( -std::atan2( delta.x, -delta.z ) ), rad_to_deg( std::asin( delta.y / length ) ) };
	}

	__forceinline float calc_fov( vec3_t view_angle, vec3_t aim_angle ) {
		vec3_t diff = view_angle - aim_angle;
		if ( diff.x < -180.f )
			diff.x += 360.f;
		if ( diff.x > 180.f )
			diff.x -= 360.f;
		return fabsf( diff.length( ) );
	}

	__forceinline float calc_fov( int screen_width, int screen_height, vec2_t position ) {
		return vec2_t( screen_width / 2.f, screen_height / 2.f ).distance( position );
	}
}