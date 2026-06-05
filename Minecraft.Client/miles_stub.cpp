// Miles Sound System stub implementation
// Provides empty no-op versions of all MSS functions so the linker
// is satisfied without needing the real mss64.lib
// The real Miles (RAD Game Tools MSS 9.3m) is only used in:
//   Common/Audio/SoundEngine.cpp
//   Various UIController files
// For learning/build purposes these stubs are sufficient.

#include <windows.h>
#include <stdint.h>

// Opaque handle typedefs matching mss.h
typedef void* HPROVIDER;
typedef void* HDIGDRIVER;
typedef void* HSAMPLE;
typedef void* HSTREAM;
typedef void* HASISTREAM;
typedef void* HMDIDRIVER;
typedef void* HSEQUENCE;
typedef void* H3DPOBJECT;
typedef void* H3DSAMPLE;
typedef void* HLISTENER;
typedef void* HDLSDEVICE;
typedef void* HBINK;
typedef void* HAUDIOBINK;
typedef void* MSS_FILE_CALLBACKS;

typedef int32_t  S32;
typedef uint32_t U32;
typedef int16_t  S16;
typedef uint16_t U16;
typedef int8_t   S8;
typedef uint8_t  U8;
typedef float    F32;
typedef double   F64;

extern "C" {

// --- Init / shutdown ---
S32   AIL_startup(void)                            { return 1; }
void  AIL_shutdown(void)                           {}
char* AIL_last_error(void)                         { return (char*)""; }
void  AIL_set_error(const char*)                   {}
S32   AIL_get_preference(U32)                      { return 0; }
S32   AIL_set_preference(U32, S32)                 { return 0; }

// --- Digital driver ---
HDIGDRIVER AIL_open_digital_driver(U32,S32,S32,U32){ return nullptr; }
void        AIL_close_digital_driver(HDIGDRIVER)   {}

// --- Samples ---
HSAMPLE AIL_allocate_sample_handle(HDIGDRIVER)     { return nullptr; }
void    AIL_release_sample_handle(HSAMPLE)         {}
void    AIL_init_sample(HSAMPLE, S32, S32)         {}
void    AIL_set_sample_address(HSAMPLE,void*,U32)  {}
void    AIL_set_sample_type(HSAMPLE,S32,U32)       {}
void    AIL_set_sample_volume_levels(HSAMPLE,F32,F32){}
void    AIL_set_sample_volume_pan(HSAMPLE,F32,F32) {}
void    AIL_set_sample_loop_count(HSAMPLE,S32)     {}
void    AIL_start_sample(HSAMPLE)                  {}
void    AIL_stop_sample(HSAMPLE)                   {}
void    AIL_resume_sample(HSAMPLE)                 {}
void    AIL_end_sample(HSAMPLE)                    {}
S32     AIL_sample_status(HSAMPLE)                 { return 4; } // SMP_DONE

// --- Streams ---
HSTREAM AIL_open_stream(HDIGDRIVER,const char*,S32){ return nullptr; }
void    AIL_close_stream(HSTREAM)                  {}
void    AIL_start_stream(HSTREAM)                  {}
void    AIL_stop_stream(HSTREAM)                   {}
void    AIL_pause_stream(HSTREAM,S32)              {}
void    AIL_set_stream_volume_levels(HSTREAM,F32,F32){}
void    AIL_set_stream_loop_count(HSTREAM,S32)     {}
S32     AIL_stream_status(HSTREAM)                 { return 4; }
S32     AIL_stream_position(HSTREAM)               { return 0; }
void    AIL_set_stream_position(HSTREAM,S32)       {}
S32     AIL_stream_length(HSTREAM)                 { return 0; }

// --- 3D ---
H3DSAMPLE AIL_allocate_3D_sample_handle(HPROVIDER){ return nullptr; }
void      AIL_release_3D_sample_handle(H3DSAMPLE) {}
void      AIL_set_3D_position(H3DPOBJECT,F32,F32,F32){}
void      AIL_set_3D_velocity(H3DPOBJECT,F32,F32,F32,F32,F32,F32){}
void      AIL_set_3D_orientation(H3DPOBJECT,F32,F32,F32,F32,F32,F32){}
void      AIL_set_3D_distance_factor(HPROVIDER,F32){}
void      AIL_set_3D_rolloff_factor(HPROVIDER,F32) {}
void      AIL_set_3D_doppler_factor(HPROVIDER,F32) {}
HLISTENER AIL_open_3D_listener(HPROVIDER)          { return nullptr; }
void      AIL_close_3D_listener(HLISTENER)         {}
HPROVIDER AIL_open_3D_provider(U32)                { return nullptr; }
void      AIL_close_3D_provider(HPROVIDER)         {}
U32       AIL_enumerate_3D_providers(HPROVIDER*,S32*)  { return 0; }

// --- Memory / file ---
void* AIL_mem_alloc_lock(U32)                      { return nullptr; }
void  AIL_mem_free_lock(void*)                     {}
S32   AIL_file_write(const char*,void*,U32)        { return 0; }
void* AIL_file_read(const char*,void*)             { return nullptr; }
U32   AIL_file_size(const char*)                   { return 0; }

// --- Misc ---
void  AIL_serve(void)                              {}
U32   AIL_ms_count(void)                           { return 0; }
void  AIL_delay(S32)                               {}
S32   AIL_background(void)                         { return 0; }
void  AIL_set_redist_directory(const char*)        {}
char* AIL_redist_directory(void)                   { return (char*)""; }

} // extern "C"
