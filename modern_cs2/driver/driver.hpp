
#define IoSetupDriver			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x850, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IoGetModule				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IoGetPeb				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IoSetTargetCr3			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IoReadPhysicalMemory	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IoWritePhysicalMemory	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#pragma once

typedef struct operation_module {
	ULONG			module_pid;
	const char* module_name;
	ULONG			module_size;
	PVOID			module_base;
};

typedef struct operation_peb {
	ULONG			peb_pid;
	PVOID			peb_base;
};

typedef struct operation_rw {
	uintptr_t		rw_address;
	PVOID			rw_buffer;
	size_t			rw_size;
};

typedef struct operation_cr3 {
	ULONG			cr3_pid;
	PVOID			cr3_result;
};

class Driver {
private:

	HANDLE driver_handle = NULL;
	ULONG process_id = 0;

	bool IsKernelAddress( void* address ) {
		return reinterpret_cast< size_t >( address ) >= ( static_cast< size_t >( 1 ) << ( 8 * sizeof( size_t ) - 1 ) );
	}

	bool IsUserAddress( void* address ) {
		return reinterpret_cast< size_t >( address ) < ( static_cast< size_t >( 1 ) << ( 8 * sizeof( size_t ) - 1 ) );
	}

	bool IsValidAddress( uint64_t address ) {
		if ( !address || !sizeof( address ) )
			return false;

		if ( address < 0xFFFFFF || address > 0x7FFFFFFF0000 )
			return false;

		if ( IsKernelAddress( reinterpret_cast< void* >( address ) ) || !IsUserAddress( reinterpret_cast< void* >( address ) ) )
			return false;

		return true;
	}

	uint32_t GetProcessIdByName( const std::string processName ) {
		PROCESSENTRY32 processInfo;
		processInfo.dwSize = sizeof( processInfo );

		HANDLE processesSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, NULL );
		if ( processesSnapshot == INVALID_HANDLE_VALUE ) {
			return 0;
		}

		Process32First( processesSnapshot, &processInfo );
		if ( !processName.compare( processInfo.szExeFile ) ) {
			CloseHandle( processesSnapshot );
			return processInfo.th32ProcessID;
		}

		while ( Process32Next( processesSnapshot, &processInfo ) ) {
			if ( !processName.compare( processInfo.szExeFile ) ) {
				CloseHandle( processesSnapshot );
				return processInfo.th32ProcessID;
			}
		}

		CloseHandle( processesSnapshot );
		return 0;
	}

public:

	Driver( const std::string ProcessName ) {
		UNICODE_STRING ObjectName;
		OBJECT_ATTRIBUTES ObjectAttributes;
		IO_STATUS_BLOCK IoStatusBlock;

		RtlInitUnicodeString( &ObjectName, const_cast< wchar_t* >( L"\\Device\\Afd" ) );
		InitializeObjectAttributes( &ObjectAttributes, &ObjectName, OBJ_CASE_INSENSITIVE, NULL, NULL );

		HANDLE Handle;
		if ( !NT_SUCCESS( NtOpenFile( &Handle, FILE_GENERIC_READ | FILE_GENERIC_WRITE, &ObjectAttributes, &IoStatusBlock, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_NON_DIRECTORY_FILE ) ) )
			exit( 1000 );

		driver_handle = Handle;
		printf( "Handle: %p\n", driver_handle );;

		operation_module req = {};

		if ( !DeviceIoControl( driver_handle, IoSetupDriver, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			exit( 1000 );

		process_id = GetProcessIdByName( ProcessName );
	}

	uint64_t GetModuleInformation_x64( const char* ModuleName, ULONG* ModuleSize = 0 ) {
		operation_module req = {};

		req.module_pid = process_id;
		req.module_name = ModuleName;

		if ( !DeviceIoControl( driver_handle, IoGetModule, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			return false;

		if ( ModuleSize )
			*ModuleSize = req.module_size;

		return reinterpret_cast< uint64_t >( req.module_base );
	}

	uint64_t GetPebAddress( ) {
		if ( !process_id )
			return 0;

		operation_peb req = {};

		req.peb_pid = process_id;

		if ( !DeviceIoControl( driver_handle, IoGetPeb, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			return false;

		return reinterpret_cast< uint64_t >( req.peb_base );
	}

	uint64_t SetTargetCr3( ) {
		if ( !process_id )
			return 0;

		operation_cr3 req = {};

		req.cr3_pid = process_id;

		if ( !DeviceIoControl( driver_handle, IoSetTargetCr3, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			return false;

		return reinterpret_cast< uint64_t >( req.cr3_result );
	}

	bool ReadPhysicalMemory( uint64_t Address, PVOID Buffer, DWORD Size ) {
		if ( !Address || !IsValidAddress( Address ) )
			return false;

		operation_rw req = { 0 };

		req.rw_address = Address;
		req.rw_buffer = Buffer;
		req.rw_size = Size;

		if ( !DeviceIoControl( driver_handle, IoReadPhysicalMemory, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			return false;

		return true;
	}

	bool WritePhysicalMemory( uint64_t Address, PVOID Buffer, DWORD Size ) {
		if ( !Address || !IsValidAddress( Address ) )
			return false;

		operation_rw req = { 0 };

		req.rw_address = Address;
		req.rw_buffer = Buffer;
		req.rw_size = Size;

		if ( !DeviceIoControl( driver_handle, IoWritePhysicalMemory, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			return false;

		return true;
	}

	bool read( uint64_t Address, PVOID Buffer, DWORD Size ) {
		if ( !Address || !IsValidAddress( Address ) )
			return false;

		operation_rw req = { 0 };

		req.rw_address = Address;
		req.rw_buffer = Buffer;
		req.rw_size = Size;

		if ( !DeviceIoControl( driver_handle, IoReadPhysicalMemory, &req, sizeof( req ), &req, sizeof( req ), 0, 0 ) )
			return false;

		return true;
	}

	template <typename T>
	T read( uint64_t Address, DWORD Size = sizeof( T ) ) {
		T buffer;
		ReadPhysicalMemory( Address, ( PVOID )&buffer, Size );
		return buffer;
	}

	template <typename t>
	auto write( const std::uintptr_t address, t value ) -> bool {
		return this->WritePhysicalMemory( address, &value, sizeof( t ) );
	}
	
	template<typename t>
	auto read_chain( const std::uintptr_t address, const std::vector<std::uintptr_t> chain ) -> t {
		auto current = address;

		for ( int i = 0; i < chain.size( ) - 1; i++ ) {
			current = this->read<std::uintptr_t>( current + chain[ i ] );
		}
		return this->read<t>( current + chain[ chain.size( ) - 1 ] );
	};

	auto read_string( std::uintptr_t addr ) -> std::string {
		char buffer[ 128 ];
		if ( this->ReadPhysicalMemory( addr, ( std::uint8_t* )&buffer, sizeof( buffer ) ) ) return std::string( buffer );
		else return ( "None" );
	}

	auto read_wstring( std::uintptr_t addr ) -> std::wstring {
		wchar_t buffer[ 128 ];
		if ( this->ReadPhysicalMemory( addr, ( std::uint8_t* )&buffer, sizeof( buffer ) ) ) return std::wstring( buffer, wcslen( buffer ) );
		else return ( L"None" );
	}

	auto get_process( LPCTSTR ProcessName ) -> uintptr_t// non-conflicting function name
	{
		PROCESSENTRY32 pt;
		HANDLE hsnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
		pt.dwSize = sizeof( PROCESSENTRY32 );
		if ( Process32First( hsnap, &pt ) ) { // must call this first
			do {
				if ( !lstrcmpi( pt.szExeFile, ProcessName ) ) {
					CloseHandle( hsnap );
					return pt.th32ProcessID;
				}
			} while ( Process32Next( hsnap, &pt ) );
		}
		CloseHandle( hsnap ); // close handle on failure
		return 0;
	}

	std::uintptr_t client_dll{ };
};

std::unique_ptr<Driver>driver;