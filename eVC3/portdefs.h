#ifdef _WIN32_WCE
#  define VERSION "PocketPC experimental version"
#  undef WIN32
#  define snprintf _snprintf
   char* getenv (char* name);
   void mkdir (char* dirname, int mode);
#endif

