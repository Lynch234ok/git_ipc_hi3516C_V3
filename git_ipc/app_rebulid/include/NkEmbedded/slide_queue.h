/**
 *
 */

#include <NkUtils/allocator.h>
#include <NkEmbedded/stdlib.h>


#if !defined(NK_SLIDE_QUEUE_H_)
#define NK_SLIDE_QUEUE_H_
NK_CPP_EXTERN_BEGIN


/**
 * SlideQ 滑动队列缓冲写用户。
 */
struct Nk_SlideQWriter;
typedef struct Nk_SlideQWriter NK_SlideQWriter;

/**
 * SlideQ 滑动队列缓冲读用户。
 */
struct Nk_SlideQReader;
typedef struct Nk_SlideQReader NK_SlideQReader;


/**
 * SlideQ 滑动队列缓冲句柄。
 *
 */
typedef struct Nk_SlideQ {
#define NK_This struct Nk_SlideQ *const

	/**
	 * 模块基础类。
	 *
	 */
	NK_Object Object;

	/**
	 * 获取缓冲写入流量。
	 * @param hPool
	 * @return
	 */
	NK_Size
	(*speed)(NK_This);


	/**
	 * 清空队列的所有数据节点。
	 */
	NK_Void
	(*empty)(NK_This);

	/**
	 * 获取一个写用户。
	 *
	 * @return	获取成功返回写用户句柄，否则返回 Nil。
	 */
	NK_SlideQWriter *
	(*getWriter)(NK_This, NK_Size size, NK_Boolean seekable, NK_UInt64 timestamp, NK_Boolean block);

	/**
	 * 释放一个写用户。
	 *
	 * @return	释放成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*releaseWriter)(NK_This, NK_SlideQWriter **Writer_r);

	/**
	 * 获取一个读用户。
	 *
	 * @return	获取成功返回读用户句柄，否则返回 Nil。
	 */
	NK_SlideQReader *
	(*getReader)(NK_This);

	/**
	 * 释放一个读用户。
	 *
	 * @return	释放成功返回 0，失败返回 -1。
	 */
	NK_Int
	(*releaseReader)(NK_This, NK_SlideQReader **Reader_r);


	/**
	 * @brief
	 *  获取缓冲的流量。
	 * @details
	 *  模块内部通过 @ref NK_SlideQWriter 模块写入的数据进行流量统计，\n
	 *  以调用 @ref getWriter() 时传入的时间戳作为参考依据，计算周期在两个 seekable 数据之间。\n
	 *
	 * @return
	 *  返回缓冲流量，单位字节每秒（kB/s）。
	 */
	NK_Size
	(*kBytesPS)(NK_This);

	/**
	 * @brief
	 *  获取最后一帧数据的时间戳。
	 * @details
	 *  每次缓冲接收到一帧数据会记录其时间戳（单位：微秒）。
	 *
	 * @return
	 *  返回最后一帧数据的时间戳（单位：微秒）。
	 */
	NK_UInt64
	(*latestTimestamp)(NK_This);

#undef NK_This
} NK_SlideQ;

/**
 * @brief
 *  创建滑动队列句柄。
 *
 * @param Alloctr [in]
 *  模块内使用的内存分配器。
 * @param maxSeekable [in]
 *  可检索数据节点数，此值会限制队列内最大的可检索节点数。
 * @param maxReaders [in]
 *  队列最大读用户数。
 *
 * @return
 *  成功返回队列句柄，失败返回 Nil。
 */
NK_API NK_SlideQ *
NK_SlideQ_CreateV2(NK_Allocator *Alloctr, NK_UInt32 maxSeekable, NK_UInt32 maxReaders);

/**
 * @macro
 *  旧版本兼容。
 */
#define NK_SlideQ_Create(__Alloctr, __maxFrams, __maxSeekable, ___maxReaders) NK_SlideQ_CreateV2(__Alloctr, __maxSeekable, ___maxReaders)

/**
 * 释放滑动队列句柄。
 *
 * @param[in]		Queue_r							队列句柄。
 *
 * @return 		成功返回 0，并把 @ref Queue_r 引用的指针至 Nil，否则返回 -1。
 */
NK_API NK_Int
NK_SlideQ_Free(NK_SlideQ **Queue_r);


/**
 * SlideQ 写用户数据结构句柄。
 *
 */
struct Nk_SlideQWriter {
#define NK_This struct Nk_SlideQWriter *const

	/**
	 * 通过写用户写入数据，应用时可以多次写入也可以一次写入，
	 * 但写入的总大小必须小于等于 @ref NK_SlideQ::getWriter() 方法调用时申请的内存大小。
	 *
	 */
	NK_Int
	(*write)(NK_This, NK_PVoid data, NK_Size len);

#undef NK_This
};


/**
 * SlideQ 读用户数据结构句柄。
 *
 */
struct Nk_SlideQReader {
#define NK_This struct Nk_SlideQReader *const

	/**
	 * 同步当前用户到缓冲池最新位置。
	 *
	 * @param[in] hUser 模块句柄
	 *
	 * @retval 0 同步成功。
	 * @retval -1 同步失败。
	 *
	 */
	NK_Int
	(*seek)(NK_This);

	/**
	 * 从队列中读取数据。\n
	 * 根据当前状态从缓冲池中读取一块数据单元，\n
	 * 读取完成后模块会将数据单元位置往后调整，以便下一次读取到往后的数据。\n
	 * 用户可通过传入dataBuf，模块将会往数据缓冲中填充所需要的数据，\n
	 * 用户也可以传入dataRef直接引用模块内部的内存数据，这样做可以减少一次内存拷贝节省处理资源，\n
	 * 但要注意非必要情况下不要改写引用的内存数据，否则有可能造成严重的后果。
	 *
	 * @param[out]			data_ref		数据的内存区域引用地址。
	 *
	 * @retval	>0	读取数据成功，返回数据的长度，获取数据后必须调用 @release 接口把数据释放，否则不能继续读取。
	 * @retval	0	队列空。
	 * @retval	-1	读取失败。
	 *
	 */
	NK_SSize
	(*read)(NK_This, NK_UInt64 *timestamp, NK_PVoid *data_ref);

	/**
	 * 预览队列数据。\n
	 * 区别于 @ref read，此接口调用后模块不会偏移下一个可读数据的位置，\n
	 * 多次重复调用此接口，除非该数据单元已被模块回收，否则将读取到同一块数据。
	 *
	 * @param[out]			data_ref		数据的内存区域引用地址。
	 *
	 * @retval	>0	读取数据成功，返回数据的长度，获取数据后必须调用 @release 接口把数据释放，否则不能继续读取。
	 * @retval	0	队列空。
	 * @retval	-1	读取失败。
	 */
	NK_SSize
	(*peek)(NK_This, NK_UInt64 *timestamp, NK_PVoid *data_ref);

	/**
	 * 去锁定队列。\n
	 * 当调用完deQueue或者peekQueue方法前，调用此接口去锁定队列，退出临界状态。
	 *
	 * @param[in] hUser 模块句柄
	 *
	 * @retval	0	解锁成功。
	 * @retval	-1	解锁失败。
	 *
	 */
	NK_Int
	(*release)(NK_This, NK_PVoid data);


#undef NK_This
};

/**
 * @brief
 *  创建组合读用户句柄。
 * @details
 *  当调用这须要从多个滑动序列中读取数据，并按照时间戳顺序以此读取的时候，\n
 *  可以通过创建协读用户方式实现，调用这须要把多个滑动队列以此传入并创建句柄。\n
 *  组合读方式可以有准同步和完全同步两种模式，见参数 @ref quasi_sync，\n
 *  当参数 @ref quasi_sync 为 True 时则工作在准同步模式，\n
 *  在这个模式下，多个 SlideQ 中只要有一个存在数据，@ref NK_SlideQReader::read() 方法就可以获取数据。\n
 *  而反之在完全同步模式下，模块内部会判断传入的所有 SlideQ 均存在数据时 @ref NK_SlideQReader::read() 方法才能正常获取数据。
 *
 * @param Alloctr [in]
 *  模块内存分配器。
 * @param quasi_sync [in]
 *  准同步标识，当标识为 True 时为准同步，否则为完全同步。
 * @param SlideQ [in]
 *  一个或多个滑动队列，最多传入 4 个，必须以 Nil 结束。
 *
 * @return
 *  成功返回读用户句柄，失败返回 Nil。
 */
NK_API NK_SlideQReader *
NK_SlideQ_Group(NK_Allocator *Alloctr, NK_Boolean quasi_sync, NK_SlideQ *SlideQ, ...);




/**
 * 释放协读用户句柄。
 *
 * @param[in]				SlideQ_r		模块句柄。
 *
 * @return		成功返回 0，失败返回 -1。
 */
NK_API NK_Int
NK_SlideQ_UnGroup(NK_SlideQReader **SlideQ_r);


/**
 * @macro
 *  旧版本兼容。
 */
#define NK_SlideQ_CreateCoReader   NK_SlideQ_Group

/**
 * @macro
 *  旧版本兼容。
 */
#define NK_SlideQ_FreeCoReader     NK_SlideQ_UnGroup


NK_CPP_EXTERN_END
#endif /* NK_SLIDE_QUEUE_H_ */

