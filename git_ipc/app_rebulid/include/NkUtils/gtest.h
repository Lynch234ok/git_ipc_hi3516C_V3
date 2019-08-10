/**
 * Google Test 单元测时抽象宏。
 */

#include <gtest/gtest.h>
#include <NkUtils/allocator.h>
#include <NkUtils/log.h>

#ifndef NK_UTILS_GTEST_H_
#define NK_UTILS_GTEST_H_

/**
 * @macro
 *
 * Google Test 用例抽象。
 * 如果须要调用 Google Test 相关的测试接口。
 * 在包含此文件之前须要包含 Google Test 头文件 gtest.h。
 */
#if defined(GTEST_INCLUDE_GTEST_GTEST_H_)

#if !defined(NK_GTEST_CASE)

/**
 * 单元测试定义。
 */
typedef NK_Void (*NK_GTestCase)(NK_Allocator *Alloctr);

/**
 * 基于 Google Test 带内存泄漏单元测试。
 */
#define NK_GTEST_CASE(__module_name, __mem_size, __Test_Case) \
	GTEST_TEST(__module_name, __Test_Case) {\
		NK_Allocator *Alloctr = NK_Nil;\
		NK_Size const mem_size = __mem_size;\
		NK_PByte mem_ptr = NK_Nil;\
		NK_PByte mem = NK_Nil;\
		\
		mem = (NK_PByte)(calloc(mem_size, 1));\
		\
		NK_Log()->setTerminalLevel(NK_LOG_ALL);\
		ASSERT_TRUE(mem) \
			<< (NK_Log()->alert("Test: Case Memory( Size = %u ) Error.", mem_size), "\r\n");\
		NK_Log()->info("Test: Case Memory( Size = %u ) Setup.", mem_size);\
		Alloctr = NK_Alloc_Create(mem, mem_size);\
		ASSERT_TRUE(Alloctr) \
			<< (NK_Log()->alert("Allocator Create Error."), "\r\n");\
		ASSERT_EQ(0, Alloctr->usage(Alloctr)) \
			<< (NK_Log()->alert("Allocator Error."), "\r\n")\
			<< (Alloctr->Object.dump(&Alloctr->Object), "\r\n");\
		\
		/** 测试用例实现。 */\
		__Test_Case(Alloctr);\
		\
		/** 检查内存泄漏。*/\
		ASSERT_EQ(0, Alloctr->usage(Alloctr)) \
			<< (NK_Log()->alert("Memory Leak? Usage %u.", Alloctr->usage(Alloctr)), "\r\n");\
		do {\
			/** 检查内存分配器参数。*/\
			ASSERT_EQ(mem_size, NK_Alloc_Free(&Alloctr, &mem_ptr));\
			ASSERT_FALSE(Alloctr);\
			ASSERT_EQ(mem, mem_ptr);\
		} while(0);\
		\
		free(mem);\
		mem = NK_Nil;\
		\
	}

#define NK_DEFINE_GTEST_CASE(__Case_Name) \
	static NK_Void __Case_Name(NK_Allocator *Alloctr)

#endif /* NK_GTEST_CASE */
#endif /* GTEST_INCLUDE_GTEST_GTEST_H_ */

#endif /* NK_UTILS_GTEST_H_ */
