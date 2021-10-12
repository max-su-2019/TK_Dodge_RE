#ifndef MYFP_VERSION_INCLUDED
#define MYFP_VERSION_INCLUDED

#define MAKE_STR_HELPER(a_str) #a_str
#define MAKE_STR(a_str) MAKE_STR_HELPER(a_str)

#define MYFP_VERSION_MAJOR 0
#define MYFP_VERSION_MINOR 4
#define MYFP_VERSION_PATCH 2
#define MYFP_VERSION_BETA 1
#define MYFP_VERSION_VERSTRING   \
	MAKE_STR(MYFP_VERSION_MAJOR) \
	"." MAKE_STR(MYFP_VERSION_MINOR) "." MAKE_STR(MYFP_VERSION_PATCH) "." MAKE_STR(MYFP_VERSION_BETA)

#endif
