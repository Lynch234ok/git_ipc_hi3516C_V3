/**
 * 网络流缓冲，通过对数据进行然后放入缓冲。
 * 通过缓冲一次性提取。
 *
 */


#include <NkUtils/object.h>
#include <NkUtils/allocator.h>


#if !defined(NK_UTILS_BUFIO_H_)
#define NK_UTILS_BUFIO_H_
NK_CPP_EXTERN_BEGIN

/**
 * 缓冲上下文，记录当前缓冲数据状况。
 * 数据结构内容只读。
 * 通过 @ref NK_BufIO_Put*() 进行编辑。
 */
typedef struct Nk_BufIO {
	NK_PByte mem, mem_end;
	NK_Size mem_len;
	NK_PByte ptr;
    NK_UInt32 pos;
	NK_UInt32 actlen;

} NK_BufIO;

/**
 * @brief
 *  复位 @ref NK_BufIO 缓冲结构。
 *
 * @param mem [in]
 *  使用固定缓冲的内存起始地址。
 * @param len [in]
 *  使用固定缓冲的内存长度。
 * @param BufIO [out]
 *  返回的缓冲上下文。
 *
 * @return
 *  复位成功返回 0，否则返回 -1。
 */
NK_API NK_Int
NK_BufIO_Reset(NK_PByte mem, NK_Size len, NK_BufIO *BufIO);

/**
 * @brief
 *  打印输出缓冲状态。
 * @details
 *  略。
 *
 * @param BufIO [in]
 *  缓冲上下文，调用 @ref NK_BufIO_Reset() 方法方法返回。
 *
 */
NK_API NK_Void
NK_BufIO_Dump(NK_BufIO *BufIO);

/**
 * @brief
 *  缓冲追加一个字节。
 */
NK_API NK_Int
NK_BufIO_PutByte(NK_BufIO *BufIO, NK_Byte abyte);

/**
 * @brief
 *  追加长度为 @ref len 个字节。
 */
NK_API NK_Int
NK_BufIO_PutBytes(NK_BufIO *BufIO, const NK_PVoid buf, NK_Size len);

/**
 * @brief
 *  获取当前缓冲长度。
 */
NK_API NK_SSize
NK_BufIO_Size(NK_BufIO *BufIO);

/**
 * @brief
 *  判断缓冲是否已经满。
 */
NK_API NK_Boolean
NK_BufIO_Endof(NK_BufIO *BufIO);

/**
 * @brief
 *  追加字符串到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutString(NK_BufIO *BufIO, NK_PChar str);

/**
 * @brief
 *  小端模式追加 32 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutLE32(NK_BufIO *BufIO, NK_UInt32 val);

/**
 * @brief
 *  大端模式追加 32 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutBE32(NK_BufIO *BufIO, NK_UInt32 val);

/**
 * @brief
 *  小端模式追加 64 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutLE64(NK_BufIO *BufIO, NK_UInt64 val);

/**
 * @brief
 *  大端模式追加 64 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutBE64(NK_BufIO *BufIO, NK_UInt64 val);

/**
 * @brief
 *  小端模式追加 16 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutLE16(NK_BufIO *BufIO, NK_UInt16 val);

/**
 * @brief
 *  大端模式追加 16 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutBE16(NK_BufIO *BufIO, NK_UInt16 val);

/**
 * @brief
 *  小端模式追加 24 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutLE24(NK_BufIO *BufIO, NK_UInt32 val);

/**
 * @brief
 *  大端模式追加 24 位整数到缓冲。
 */
NK_API NK_Int
NK_BufIO_PutBE24(NK_BufIO *BufIO, NK_UInt32 val);


NK_CPP_EXTERN_END
#endif /* NK_UTILS_BUFIO_H_ */
