/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef MIR_TEST_DOUBLES_STUB_CLIENT_BUFFER_H_
#define MIR_TEST_DOUBLES_STUB_CLIENT_BUFFER_H_

#include "src/include/client/mir/client_buffer.h"
#include <unistd.h>

namespace mir
{
namespace test
{
namespace doubles
{

struct StubClientBuffer : client::ClientBuffer
{
    StubClientBuffer(
        std::shared_ptr<MirBufferPackage> const& package,
        geometry::Size size, MirPixelFormat pf,
        std::shared_ptr<graphics::NativeBuffer> const& buffer) :
        package{package}, size_{size}, pf_{pf}, buffer{buffer}
    {
    }

    StubClientBuffer(
        std::shared_ptr<MirBufferPackage> const& package,
        geometry::Size size, MirPixelFormat pf) :
        StubClientBuffer(package, size, pf, nullptr)
    {
    }

    ~StubClientBuffer()
    {
        for (int i = 0; i < package->fd_items; i++)
            ::close(package->fd[i]);
    }

    std::shared_ptr<client::MemoryRegion> secure_for_cpu_write() override
    {
        auto raw = new client::MemoryRegion{
            size().width,
            size().height,
            stride(),
            pixel_format(),
            std::shared_ptr<char>(new char[stride().as_int()*size().height.as_int()], [](char arr[]) {delete[] arr;})};

        return std::shared_ptr<client::MemoryRegion>(raw);
    }

    geometry::Size size() const override { return size_; }
    geometry::Stride stride() const override { return geometry::Stride{package->stride}; }
    MirPixelFormat pixel_format() const override { return pf_; }
    uint32_t age() const override { return 0; }
    void increment_age() override {}
    void mark_as_submitted() override {}

    std::shared_ptr<graphics::NativeBuffer> native_buffer_handle() const override
    {
        return buffer;
    }
    void update_from(MirBufferPackage const&) override {}
    void fill_update_msg(MirBufferPackage&)  override{}

    MirNativeBuffer* as_mir_native_buffer() const { return nullptr; }
    void set_fence(MirNativeFence, MirBufferAccess) {}
    MirNativeFence get_fence() const { return nullptr; }
    bool wait_fence(MirBufferAccess, std::chrono::nanoseconds) { return true; }

    std::shared_ptr<MirBufferPackage> const package;
    geometry::Size size_;
    MirPixelFormat pf_;
    std::shared_ptr<graphics::NativeBuffer> buffer;
};

}
}
}

#endif
