#ifndef HB_TRACE_ERROR
#define HB_TRACE_ERROR

#define TRACE_ERROR(msg)   TRACE("%s(%i): Error %s\n"   , __FILE__, __LINE__, (msg));
#define TRACE_WARNING(msg) TRACE("%s(%i): Warning: %s\n", __FILE__, __LINE__, (msg));

#endif