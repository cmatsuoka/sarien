#ifdef _WIN32_WCE

#  include <stdlib.h>

#  define HOMEDIR "USERPROFILE"
#  define DATA_DIR "Sarien"
#  define VERSION "PocketPC experimental version"

#  undef USE_CONSOLE
#  define PATCH_LOGIC

typedef unsigned char	UINT8;
typedef signed char		SINT8;
typedef unsigned short	UINT16;
typedef signed short	SINT16;
typedef unsigned int	UINT32;
typedef signed int		SINT32;

#  undef WIN32
#  define snprintf _snprintf
   char* getenv (char* name);
   void mkdir (char* dirname, int mode);

#  define assert(a) ;

#endif
