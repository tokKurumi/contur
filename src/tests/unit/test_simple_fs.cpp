/// @file test_simple_fs.cpp
/// @brief Unit tests for SimpleFS.

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "contur/fs/simple_fs.h"

using namespace contur;

namespace {

    auto asBytes(const std::vector<std::uint8_t> &src) -> std::vector<std::byte>
    {
        std::vector<std::byte> out;
        out.reserve(src.size());
        for (auto v : src)
        {
            out.push_back(static_cast<std::byte>(v));
        }
        return out;
    }

    auto asInts(const std::vector<std::byte> &src) -> std::vector<std::uint8_t>
    {
        std::vector<std::uint8_t> out;
        out.reserve(src.size());
        for (auto b : src)
        {
            out.push_back(static_cast<std::uint8_t>(b));
        }
        return out;
    }

} // namespace

class SimpleFSTest : public ::testing::Test
{
    protected:
    SimpleFS fs{32, 8};
};

TEST_F(SimpleFSTest, CreateWriteReadRoundTrip)
{
    auto openWrite = fs.open("/hello.bin", OpenMode::Create | OpenMode::Write);
    ASSERT_TRUE(openWrite.isOk());

    const auto payload = asBytes({1, 2, 3, 4, 5, 6});
    auto wrote = fs.write(openWrite.value(), std::span<const std::byte>(payload.data(), payload.size()));
    ASSERT_TRUE(wrote.isOk());
    EXPECT_EQ(wrote.value(), payload.size());
    ASSERT_TRUE(fs.close(openWrite.value()).isOk());

    auto openRead = fs.open("/hello.bin", OpenMode::Read);
    ASSERT_TRUE(openRead.isOk());

    std::vector<std::byte> buffer(payload.size(), std::byte{0});
    auto read = fs.read(openRead.value(), std::span<std::byte>(buffer.data(), buffer.size()));
    ASSERT_TRUE(read.isOk());
    EXPECT_EQ(read.value(), payload.size());
    EXPECT_EQ(asInts(buffer), std::vector<std::uint8_t>({1, 2, 3, 4, 5, 6}));
}

TEST_F(SimpleFSTest, MkdirAndListDir)
{
    ASSERT_TRUE(fs.mkdir("/etc").isOk());
    ASSERT_TRUE(fs.mkdir("/tmp").isOk());

    auto root = fs.listDir("/");
    ASSERT_TRUE(root.isOk());
    ASSERT_EQ(root.value().size(), 2u);
    EXPECT_EQ(root.value()[0].name, "etc");
    EXPECT_EQ(root.value()[1].name, "tmp");
    EXPECT_EQ(root.value()[0].type, InodeType::Directory);
}

TEST_F(SimpleFSTest, RemoveFileThenOpenFails)
{
    auto fd = fs.open("/data.bin", OpenMode::Create | OpenMode::Write);
    ASSERT_TRUE(fd.isOk());
    ASSERT_TRUE(fs.close(fd.value()).isOk());

    ASSERT_TRUE(fs.remove("/data.bin").isOk());

    auto openRead = fs.open("/data.bin", OpenMode::Read);
    ASSERT_TRUE(openRead.isError());
    EXPECT_EQ(openRead.errorCode(), ErrorCode::NotFound);
}

TEST_F(SimpleFSTest, RemoveNonEmptyDirectoryFails)
{
    ASSERT_TRUE(fs.mkdir("/logs").isOk());
    auto fd = fs.open("/logs/a.txt", OpenMode::Create | OpenMode::Write);
    ASSERT_TRUE(fd.isOk());
    ASSERT_TRUE(fs.close(fd.value()).isOk());

    auto removeDir = fs.remove("/logs");
    ASSERT_TRUE(removeDir.isError());
    EXPECT_EQ(removeDir.errorCode(), ErrorCode::InvalidState);
}

TEST_F(SimpleFSTest, StatReturnsSizeAndType)
{
    auto fd = fs.open("/note.txt", OpenMode::Create | OpenMode::Write);
    ASSERT_TRUE(fd.isOk());

    const auto payload = asBytes({9, 8, 7, 6, 5});
    ASSERT_TRUE(fs.write(fd.value(), std::span<const std::byte>(payload.data(), payload.size())).isOk());
    ASSERT_TRUE(fs.close(fd.value()).isOk());

    auto info = fs.stat("/note.txt");
    ASSERT_TRUE(info.isOk());
    EXPECT_EQ(info.value().type, InodeType::File);
    EXPECT_EQ(info.value().size, payload.size());
    EXPECT_GE(info.value().blockCount, 1u);
}

TEST_F(SimpleFSTest, AppendModeWritesToEnd)
{
    auto fdWrite = fs.open("/append.txt", OpenMode::Create | OpenMode::Write);
    ASSERT_TRUE(fdWrite.isOk());

    auto first = asBytes({1, 2, 3});
    ASSERT_TRUE(fs.write(fdWrite.value(), std::span<const std::byte>(first.data(), first.size())).isOk());
    ASSERT_TRUE(fs.close(fdWrite.value()).isOk());

    auto fdAppend = fs.open("/append.txt", OpenMode::Write | OpenMode::Append);
    ASSERT_TRUE(fdAppend.isOk());
    auto second = asBytes({4, 5});
    ASSERT_TRUE(fs.write(fdAppend.value(), std::span<const std::byte>(second.data(), second.size())).isOk());
    ASSERT_TRUE(fs.close(fdAppend.value()).isOk());

    auto fdRead = fs.open("/append.txt", OpenMode::Read);
    ASSERT_TRUE(fdRead.isOk());

    std::vector<std::byte> buffer(5, std::byte{0});
    auto read = fs.read(fdRead.value(), std::span<std::byte>(buffer.data(), buffer.size()));
    ASSERT_TRUE(read.isOk());
    EXPECT_EQ(asInts(buffer), std::vector<std::uint8_t>({1, 2, 3, 4, 5}));
}

TEST_F(SimpleFSTest, OpenMissingWithoutCreateFails)
{
    auto fd = fs.open("/missing.txt", OpenMode::Read);
    ASSERT_TRUE(fd.isError());
    EXPECT_EQ(fd.errorCode(), ErrorCode::NotFound);
}

TEST_F(SimpleFSTest, ReadFromWriteOnlyDescriptorFails)
{
    auto fd = fs.open("/wonly.txt", OpenMode::Create | OpenMode::Write);
    ASSERT_TRUE(fd.isOk());

    std::vector<std::byte> buffer(4, std::byte{0});
    auto read = fs.read(fd.value(), std::span<std::byte>(buffer.data(), buffer.size()));
    ASSERT_TRUE(read.isError());
    EXPECT_EQ(read.errorCode(), ErrorCode::PermissionDenied);
}
