// See the file "COPYING" in the main distribution directory for copyright.

#ifndef util_h
#define util_h

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "config.h"

// Expose C99 functionality from inttypes.h, which would otherwise not be
// available in C++.
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#if __STDC__
#define myattribute __attribute__
#else
#define myattribute(x)
#endif

#ifdef DEBUG

#include <assert.h>

#define ASSERT(x)	assert(x)
#define DEBUG_MSG(x...)	fprintf(stderr, x)
#define DEBUG_fputs	fputs

#else

#define ASSERT(x)
#define DEBUG_MSG(x...)
#define DEBUG_fputs(x...)

#endif

#ifdef USE_PERFTOOLS
#include <google/heap-checker.h>
#include <google/heap-profiler.h>
extern HeapLeakChecker* heap_checker;
#endif

#include <stdint.h>

typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef int64 bro_int_t;
typedef uint64 bro_uint_t;

// "ptr_compat_uint" and "ptr_compat_int" are (un)signed integers of
// pointer size. They can be cast safely to a pointer, e.g. in Lists,
// which represent their entities as void* pointers.
//
#if SIZEOF_VOID_P == 8
typedef uint64 ptr_compat_uint;
typedef int64 ptr_compat_int;
#define PRI_PTR_COMPAT_INT PRId64 // Format to use with printf.
#define PRI_PTR_COMPAT_UINT PRIu64
#elif SIZEOF_VOID_P == 4
typedef uint32 ptr_compat_uint;
typedef int32 ptr_compat_int;
#define PRI_PTR_COMPAT_INT PRId32
#define PRI_PTR_COMPAT_UINT PRIu32
#else
# error "Unusual pointer size. Please report to bro@bro-ids.org."
#endif

template <class T>
void delete_each(T* t)
	{
	typedef typename T::iterator iterator;
	for ( iterator it = t->begin(); it != t->end(); ++it )
		delete *it;
	}

extern char* copy_string(const char* s);
extern int streq(const char* s1, const char* s2);

// Returns the character corresponding to the given escape sequence (s points
// just past the '\'), and updates s to point just beyond the last character
// of the sequence.
extern int expand_escape(const char*& s);

extern char* skip_whitespace(char* s);
extern const char* skip_whitespace(const char* s);
extern char* skip_whitespace(char* s, char* end_of_s);
extern const char* skip_whitespace(const char* s, const char* end_of_s);
extern char* skip_digits(char* s);
extern char* get_word(char*& s);
extern void get_word(int length, const char* s, int& pwlen, const char*& pw);
extern void to_upper(char* s);
extern const char* strchr_n(const char* s, const char* end_of_s, char ch);
extern const char* strrchr_n(const char* s, const char* end_of_s, char ch);
extern int decode_hex(char ch);
extern unsigned char encode_hex(int h);
extern int strcasecmp_n(int s_len, const char* s, const char* t);
#ifndef HAVE_STRCASESTR
extern char* strcasestr(const char* s, const char* find);
#endif
extern const char* strpbrk_n(size_t len, const char* s, const char* charset);
template<class T> int atoi_n(int len, const char* s, const char** end, int base, T& result);
extern char* uitoa_n(uint64 value, char* str, int n, int base, const char* prefix=0);
int strstr_n(const int big_len, const unsigned char* big,
		const int little_len, const unsigned char* little);
extern int fputs(int len, const char* s, FILE* fp);
extern bool is_printable(const char* s, int len);

extern const char* fmt_bytes(const char* data, int len);

// Note: returns a pointer into a shared buffer.
extern const char* fmt(const char* format, ...)
	myattribute((format (printf, 1, 2)));
extern const char* fmt_access_time(double time);

extern bool ensure_dir(const char *dirname);

// Returns true if path exists and is a directory.
bool is_dir(const char* path); 

extern uint8 shared_hmac_md5_key[16];
extern void hash_md5(size_t size, const unsigned char* bytes,
			unsigned char digest[16]);

extern int hmac_key_set;
extern unsigned char shared_hmac_md5_key[16];
extern void hmac_md5(size_t size, const unsigned char* bytes,
			unsigned char digest[16]);

extern const char* md5_digest_print(const unsigned char digest[16]);

// Initializes RNGs for bro_random() and MD5 usage.  If seed is given, then
// it is used (to provide determinism).  If load_file is given, the seeds
// (both random & MD5) are loaded from that file.  This takes precedence
// over the "seed" argument.  If write_file is given, the seeds are written
// to that file.
//
extern void init_random_seed(uint32 seed, const char* load_file,
				const char* write_file);

// Returns true if the user explicitly set a seed via init_random_seed();
extern bool have_random_seed();

// Replacement for the system random(), to which is normally falls back
// except when a seed has been given. In that case, we use our own
// predictable PRNG.
long int bro_random();

extern uint64 rand64bit();

// Each event source that may generate events gets an internally unique ID.
// This is always LOCAL for a local Bro. For remote event sources, it gets
// assigned by the RemoteSerializer.
//
// FIXME: Find a nicer place for this type definition.
// Unfortunately, it introduces circular dependencies when defined in one of
// the obvious places (like Event.h or RemoteSerializer.h)

typedef ptr_compat_uint SourceID;
#define PRI_SOURCE_ID PRI_PTR_COMPAT_UINT
static const SourceID SOURCE_LOCAL = 0;

extern void pinpoint();
extern int int_list_cmp(const void* v1, const void* v2);

extern const char* bro_path();
extern const char* bro_prefixes();
std::string dot_canon(std::string path, std::string file, std::string prefix = "");
const char* normalize_path(const char* path);
void get_script_subpath(const std::string& full_filename, const char** subpath);
extern FILE* search_for_file(const char* filename, const char* ext,
	const char** full_filename, bool load_pkgs, const char** bropath_subpath);

// Renames the given file to a new temporary name, and opens a new file with
// the original name. Returns new file or NULL on error. Inits rotate_info if
// given (open time is set network time).
class RecordVal;
extern FILE* rotate_file(const char* name, RecordVal* rotate_info);

// This mimics the script-level function with the same name.
const char* log_file_name(const char* tag);

// Calculate the duration until the next time a file is to be rotated, based
// on the given rotate_interval and rotate_base_time.
double calc_next_rotate(double rotate_interval, const char* rotate_base_time);

// Terminates processing gracefully, similar to pressing CTRL-C.
void terminate_processing();

// Sets the current status of the Bro process to the given string.
// If the option --status-file has been set, this is written into
// the the corresponding file.  Otherwise, the function is a no-op.
#define set_processing_status(status, location) \
	_set_processing_status(status " [" location "]\n");
void _set_processing_status(const char* status);

// Current timestamp, from a networking perspective, not a wall-clock
// perspective.  In particular, if we're reading from a savefile this
// is the time of the most recent packet, not the time returned by
// gettimeofday().
extern double network_time;

// Returns the current time.
// (In pseudo-realtime mode this is faked to be the start time of the
// trace plus the time interval Bro has been running. To avoid this,
// call with real=true).
extern double current_time(bool real=false);

// Convert a time represented as a double to a timeval struct.
extern struct timeval double_to_timeval(double t);

// Return > 0 if tv_a > tv_b, 0 if equal, < 0 if tv_a < tv_b.
extern int time_compare(struct timeval* tv_a, struct timeval* tv_b);

// Returns an integer that's very likely to be unique, even across Bro
// instances. The integer can be drawn from different pools, which is helpful
// when the randon number generator is seeded to be deterministic. In that
// case, the same sequence of integers is generated per pool.
#define UID_POOL_DEFAULT_INTERNAL 1
#define UID_POOL_DEFAULT_SCRIPT   2
#define UID_POOL_CUSTOM_SCRIPT    10 // First available custom script level pool.
extern uint64 calculate_unique_id();
extern uint64 calculate_unique_id(const size_t pool);

// For now, don't use hash_maps - they're not fully portable.
#if 0
// Use for hash_map's string keys.
struct eqstr {
	bool operator()(const char* s1, const char* s2) const
		{
		return strcmp(s1, s2) == 0;
		}
};
#endif

// Use for map's string keys.
struct ltstr {
	bool operator()(const char* s1, const char* s2) const
	{
	return strcmp(s1, s2) < 0;
	}
};

// Versions of realloc/malloc which abort() on out of memory

inline size_t pad_size(size_t size)
	{
	// We emulate glibc here (values measured on Linux i386).
	// FIXME: We should better copy the portable value definitions from glibc.
	if ( size == 0 )
		return 0;	// glibc allocated 16 bytes anyway.

	const int pad = 8;
	if ( size < 12 )
		return 2 * pad;

	return ((size+3) / pad + 1) * pad;
	}

#define padded_sizeof(x) (pad_size(sizeof(x)))

extern void out_of_memory(const char* where);

inline void* safe_realloc(void* ptr, size_t size)
	{
	ptr = realloc(ptr, size);
	if ( size && ! ptr )
		out_of_memory("realloc");

	return ptr;
	}

inline void* safe_malloc(size_t size)
	{
	void* ptr = malloc(size);
	if ( ! ptr )
		out_of_memory("malloc");

	return ptr;
	}

inline char* safe_strncpy(char* dest, const char* src, size_t n)
	{
	char* result = strncpy(dest, src, n);
	dest[n-1] = '\0';
	return result;
	}

inline int safe_snprintf(char* str, size_t size, const char* format, ...)
	{
	va_list al;
	va_start(al, format);
	int result = vsnprintf(str, size, format, al);
	va_end(al);
	str[size-1] = '\0';

	return result;
	}

inline int safe_vsnprintf(char* str, size_t size, const char* format, va_list al)
	{
	int result = vsnprintf(str, size, format, al);
	str[size-1] = '\0';
	return result;
	}

// Returns total memory allocations and (if available) amount actually
// handed out by malloc.
extern void get_memory_usage(unsigned int* total,
			     unsigned int* malloced);
#endif
