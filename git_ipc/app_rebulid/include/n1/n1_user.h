
/**
 * N1 用户认真体系接口。
 */

#include <NkUtils/n1_def.h>

#ifndef NK_N1_DEVICE_H_
#define NK_N1_DEVICE_H_
NK_CPP_EXTERN_BEGIN



typedef struct Nk_N1User
{
	/**
	 * 用户名称。
	 */
	NK_Char username[128];

	/**
	 * 用户密码。
	 */
	NK_Char password[128];

	/**
	 * 禁止标识，位有效（One Bit Hot）标识，最多传入 32 个标识，。
	 */
	//NK_UInt32 forbidden;

} NK_N1User;


/**
 * @brief N1 用户集合句柄。
 */
typedef NK_PVoid NK_N1UserSet;

/**
 *
 */
extern NK_N1UserSet
NK_N1User_Create();


/**
 *
 */
extern NK_Int
NK_N1User_Free(NK_N1UserSet *UserSet_r);


/**
 *
 */
extern NK_Int
NK_N1User_Add(NK_N1User *User);

/**
 *
 */
extern NK_Int
NK_N1User_Delete(NK_N1User *User);

/**
 *
 */
extern NK_Int
NK_N1User_Find(NK_PChar username, NK_N1User *User);

/**
 *
 */
extern NK_Int
NK_N1User_Count(NK_Int id, NK_N1User *User);

/**
 *
 */
extern NK_Int
NK_N1User_IndexOf(NK_Int id, NK_N1User *User);




NK_CPP_EXTERN_END
#endif /* NK_N1_DEVICE_H_ */
