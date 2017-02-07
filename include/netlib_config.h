#ifndef __NETLIB_CONF__H__
#define __NETLIB_CONF__H__

//Basic defines for processing other defines
#define HBC_xstr(a) HBC_str(a)
#define HBC_str(a) #a

//The current version
#define netlib_VERSION_trash v0.5.1-26-gbf817a8

#define netlib_VERSION HBC_xstr(netlib_VERSION_trash)

//Is netlib debug on?
#define NETLIB_DEBUG

#endif
