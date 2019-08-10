
#ifndef _TFCARD_TYPE_H_
#define _TFCARD_TYPE_H_

#ifdef __cplusplus
extern "C"{
#endif


#include <stdint.h>
#if 0
#ifndef NK_Char
typedef  char           NK_Char;
#endif

#ifndef NK_PChar
typedef  NK_Char*       NK_PChar;
#endif

#ifndef NK_Int8
typedef  char           NK_Int8;
#endif

#ifndef NK_UInt8
typedef  unsigned char  NK_UInt8;
#endif

#ifndef NK_Int
typedef  int            NK_Int;
#endif

#ifndef NK_Int16
typedef int16_t			NK_Int16;
#endif

#ifndef NK_UInt16
typedef uint16_t		NK_UInt16;
#endif

#ifndef NK_Int32
typedef  int32_t        NK_Int32;
#endif

#ifndef NK_UInt32
typedef uint32_t	    NK_UInt32;
#endif


#ifndef NK_Int64
typedef  int64_t        NK_Int64;
#endif

#ifndef NK_UInt64
typedef  uint64_t       NK_UInt64;
#endif

#ifndef NK_SSize
typedef  NK_Int32		NK_SSize;
#endif

#ifndef NK_Size
typedef  NK_UInt32		NK_Size;
#endif

#ifndef NK_SSize64
typedef  NK_Int64 		NK_SSize64;
#endif

#ifndef NK_Size64
typedef  NK_UInt64		NK_Size64;
#endif

#ifndef NK_Void
typedef  void           NK_Void;
#endif

#ifndef NK_PVoid
typedef  void*          NK_PVoid;
#endif

#ifndef NK_Boolean
typedef  NK_Int         NK_Boolean;
#endif

#ifndef NK_Bool
typedef  NK_Int         NK_Bool;
#endif

#ifndef NK_Bool
typedef  NK_Int         NK_Bool;
#endif

#ifndef NK_Float
typedef  float         NK_Float;
#endif

#ifndef NK_Double
typedef	 double		NK_Double;
#endif

#ifndef NK_False
#define		NK_False	(0)
#endif
#ifndef NK_True
#define 	NK_True		(!NK_False)
#endif
#ifndef NK_FALSE
#define		NK_FALSE	(NK_False)
#endif
#ifndef NK_TRUE
#define		NK_TRUE		(NK_True)
#endif

#ifndef NK_Nil
#define   NK_Nil      (NK_PVoid)(0)
#endif

#ifndef NK_NULL
#define   NK_NULL      (NK_PVoid)(0)
#endif
#endif
#ifdef __cplusplus
}
#endif

#endif


