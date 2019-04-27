#ifndef _FARAGE_API_
#define _FARAGE_API_

#include "api/basics.h"
#include "api/handle.h"
#include "api/link.h"

/*#define READY_CAST                      Farage::Ready*
#define SERVER_CAST                     Farage::Server*
#define MESSAGE_CAST                    Farage::Message*
#define USER_CAST                       Farage::User*
#define STRING_CAST                     std::string*
#define VOID_CAST                       void*
#define INT_CAST                        int*
#define GET_CAST(t,a,v)                 t v = reinterpret_cast<t>(a)*/
/*#define READY_BITS                      2
#define SERVER_BITS                     4
#define MESSAGE_BITS                    516192
#define BAN_BITS                        24
#define ERROR_BITS                      512
#define USER_BITS                       24*/
/*#define GET_TYPE1(e)                    ((e) & (READY_BITS) ? (READY_CAST) : (e) & (SERVER_BITS) ? (SERVER_CAST) : (e) & (BAN_BITS) ? (STRING_CAST) : (e) & (MESSAGE_BITS) ? (MESSAGE_CAST) : (e) & (ERROR_BITS) ? (INT_CAST) : (VOID_CAST))
#define GET_TYPE2(e)                    ((e) & (BAN_BITS) ? (USER_CAST) : (e) & (ERROR_BITS) ? (STRING_CAST) : (VOID_CAST))
#define GET_EVENT_ARG1(e,a,v)           GET_CAST(GET_TYPE1(e),a,v)
#define GET_EVENT_ARG2(e,a,v)           GET_CAST(GET_TYPE2(e),a,v)*/

//#define GET_EVENT_ARG1(e,a,v)           if(e&READY_BITS) READY_CAST v = reinterpret_cast<READY_CAST>(a);else if(e&SERVER_BITS) SERVER_CAST v = reinterpret_cast<SERVER_CAST>(a);else if(e&BAN_BITS) STRING_CAST v = reinterpret_cast<STRING_CAST>(a);else if(e&MESSAGE_BITS) MESSAGE_CAST v = reinterpret_cast<MESSAGE_CAST>(a);else if(e&ERROR_BITS) INT_CAST v = reinterpret_cast<INT_CAST>(a);else VOID_CAST v = a;
//#define GET_EVENT_ARG2(e,a,v)           if(e&BAN_BITS) USER_CAST v = reinterpret_cast<USER_CAST>(a);else if(e&ERROR_BITS) STRING_CAST v = reinterpret_cast<STRING_CAST>(a);else VOID_CAST* v = a;

//#define GET_EVENT_ARG1(e,a,v)           if(e&READY_BITS) goto READY_CREATE;else if(e&SERVER_BITS) goto SERVER_CREATE;else if(e&BAN_BITS) goto BAN_CREATE;else if(e&MESSAGE_BITS) goto MESSAGE_CREATE;else if(e&ERROR_BITS) goto ERROR_CREATE;VOID_CAST v = a;goto CREATED;READY_CREATE: GET_CAST(READY_CAST,a,v);goto CREATED;SERVER_CREATE: GET_CAST(SERVER_CAST,a,v);goto CREATED;BAN_CREATE: GET_CAST(STRING_CAST,a,v);goto CREATED;MESSAGE_CREATE: GET_CAST(MESSAGE_CAST,a,v);goto CREATED;ERROR_CREATE: GET_CAST(INT_CAST,a,v);CREATED:
//#define GET_EVENT_ARG2(e,a,v)           if(e&BAN_BITS) goto BAN_CREATE2;else if(e&ERROR_BITS) goto ERROR_CREATE2;VOID_CAST v = a;goto CREATED2;BAN_CREATE2: GET_CAST(USER_CAST,a,v);goto CREATED2;ERROR_CREATE2: GET_CAST(STRING_CAST,a,v);CREATED2:

//#define GET_EVENT_ARGS(e,a1,a2,v1,v2)   if(e&READY_BITS) goto READY_CREATE;else if(e&SERVER_BITS) goto SERVER_CREATE;else if(e&BAN_BITS) goto BAN_CREATE;else if(e&MESSAGE_BITS) goto MESSAGE_CREATE;else if(e&ERROR_BITS) goto ERROR_CREATE;VOID_CAST v1 = a1;VOID_CAST v2 = a2;goto CREATED;READY_CREATE: GET_CAST(READY_CAST,a1,v1);goto CREATED;SERVER_CREATE: GET_CAST(SERVER_CAST,a1,v1);goto CREATED;BAN_CREATE: GET_CAST(STRING_CAST,a1,v1);GET_CAST(USER_CAST,a2,v2);goto CREATED;MESSAGE_CREATE: GET_CAST(MESSAGE_CAST,a1,v1);goto CREATED;ERROR_CREATE: GET_CAST(INT_CAST,a1,v1);GET_CAST(STRING_CAST,a2,v2);CREATED:

#endif

