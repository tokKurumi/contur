/// @file test_syscall_table.cpp
/// @brief Unit tests for SyscallTable.

#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "contur/core/error.h"

#include "contur/arch/instruction.h"
#include "contur/process/process_image.h"
#include "contur/syscall/syscall_table.h"

using namespace contur;

namespace {

    ProcessImage makeCaller(ProcessId pid = 1)
    {
        return ProcessImage(pid, "caller", {{Instruction::Nop, 0, 0, 0}});
    }

    class FakeSyscallHandler final : public ISyscallHandler
    {
        public:
        Result<RegisterValue> handle(SyscallId id, std::span<const RegisterValue> args, ProcessImage &caller) override
        {
            lastId = id;
            lastArgCount = args.size();
            lastCaller = caller.id();
            return Result<RegisterValue>::ok(77);
        }

        SyscallId lastId = SyscallId::Exit;
        std::size_t lastArgCount = 0;
        ProcessId lastCaller = INVALID_PID;
    };

} // namespace

TEST(SyscallTableTest, RegisterAndDispatchFunctionHandler)
{
    SyscallTable table;
    auto caller = makeCaller();

    ASSERT_TRUE(table
                    .registerHandler(
                        SyscallId::GetPid,
                        [](std::span<const RegisterValue> args, ProcessImage &process) {
                            (void)args;
                            return Result<RegisterValue>::ok(static_cast<RegisterValue>(process.id()));
                        }
                    )
                    .isOk());

    auto result = table.dispatch(SyscallId::GetPid, {}, caller);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 1);
    EXPECT_TRUE(table.hasHandler(SyscallId::GetPid));
    EXPECT_EQ(table.handlerCount(), 1u);
}

TEST(SyscallTableTest, RegisterAndDispatchInterfaceHandler)
{
    SyscallTable table;
    FakeSyscallHandler handler;
    auto caller = makeCaller(42);

    ASSERT_TRUE(table.registerHandler(SyscallId::Write, handler).isOk());

    const std::vector<RegisterValue> args = {1, 2, 3};
    auto result = table.dispatch(SyscallId::Write, args, caller);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value(), 77);
    EXPECT_EQ(handler.lastId, SyscallId::Write);
    EXPECT_EQ(handler.lastArgCount, 3u);
    EXPECT_EQ(handler.lastCaller, 42u);
}

TEST(SyscallTableTest, RegisterRejectsEmptyFunction)
{
    SyscallTable table;

    auto result = table.registerHandler(SyscallId::Read, SyscallTable::HandlerFn{});

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::InvalidArgument);
}

TEST(SyscallTableTest, DispatchUnknownSyscallReturnsNotFound)
{
    SyscallTable table;
    auto caller = makeCaller();

    auto result = table.dispatch(SyscallId::Read, {}, caller);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::NotFound);
}

TEST(SyscallTableTest, UnregisterRemovesHandler)
{
    SyscallTable table;
    auto caller = makeCaller();

    ASSERT_TRUE(table
                    .registerHandler(
                        SyscallId::Yield,
                        [](std::span<const RegisterValue> args, ProcessImage &process) {
                            (void)args;
                            (void)process;
                            return Result<RegisterValue>::ok(1);
                        }
                    )
                    .isOk());

    ASSERT_TRUE(table.unregisterHandler(SyscallId::Yield).isOk());
    EXPECT_FALSE(table.hasHandler(SyscallId::Yield));
    EXPECT_EQ(table.handlerCount(), 0u);

    auto dispatch = table.dispatch(SyscallId::Yield, {}, caller);
    EXPECT_TRUE(dispatch.isError());
    EXPECT_EQ(dispatch.errorCode(), ErrorCode::NotFound);
}

TEST(SyscallTableTest, UnregisterUnknownReturnsNotFound)
{
    SyscallTable table;

    auto result = table.unregisterHandler(SyscallId::Exec);

    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorCode(), ErrorCode::NotFound);
}
