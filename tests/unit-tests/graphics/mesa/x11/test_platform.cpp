/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Cemil Azizoglu <cemil.azizoglu@canonical.com>
 */

#include "src/platforms/mesa/server/x11/platform.h"

#include "mir/test/doubles/platform_factory.h"
#include "mir/test/doubles/mock_drm.h"
#include "mir/test/doubles/mock_gbm.h"
#include "mir/test/doubles/mock_x11.h"
#include "mir/shared_library.h"
#include "mir_test_framework/executable_path.h"
#include "mir_test_framework/udev_environment.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace mg = mir::graphics;
namespace mtd = mir::test::doubles;
namespace mtf = mir_test_framework;

namespace
{
const char probe_platform[] = "probe_graphics_platform";

class X11GraphicsPlatformTest : public ::testing::Test
{
public:
    void SetUp()
    {
        ::testing::Mock::VerifyAndClearExpectations(&mock_drm);
        ::testing::Mock::VerifyAndClearExpectations(&mock_gbm);
//        fake_devices.add_standard_device("standard-drm-devices");
    }

    std::shared_ptr<mg::Platform> create_platform()
    {
        return mtd::create_platform_with_null_dependencies();
    }

    ::testing::NiceMock<mtd::MockDRM> mock_drm;
    ::testing::NiceMock<mtd::MockGBM> mock_gbm;
    ::testing::NiceMock<mtd::MockX11> mock_x11;
//    mtf::UdevEnvironment fake_devices;
};
}

TEST_F(X11GraphicsPlatformTest, failure_to_open_x11_display_results_in_an_error)
{
    using namespace ::testing;

    EXPECT_CALL(mock_x11, XOpenDisplay(_))
        .WillRepeatedly(Return(nullptr));

    try
    {
        auto platform = create_platform();
    } catch(...)
    {
        return;
    }

    FAIL() << "Expected an exception to be thrown.";
}

TEST_F(X11GraphicsPlatformTest, failure_to_open_drm_results_in_an_error)
{
    using namespace ::testing;

    EXPECT_CALL(mock_drm, open(_,_,_))
            .WillRepeatedly(Return(-1));

    try
    {
        auto platform = create_platform();
    } catch(...)
    {
        return;
    }

    FAIL() << "Expected an exception to be thrown.";
}

TEST_F(X11GraphicsPlatformTest, failure_to_create_gbm_device_results_in_an_error)
{
    using namespace ::testing;

    EXPECT_CALL(mock_gbm, gbm_create_device(mock_drm.fake_drm.fd()))
        .WillRepeatedly(Return(nullptr));

    try
    {
        auto platform = create_platform();
    } catch(...)
    {
        return;
    }

    FAIL() << "Expected an exception to be thrown.";
}

TEST_F(X11GraphicsPlatformTest, probe_returns_unsupported_when_no_drm_udev_devices)
{
    mtf::UdevEnvironment udev_environment;

    mir::SharedLibrary platform_lib{mtf::server_platform("server-mesa-x11")};
    auto probe = platform_lib.load_function<mg::PlatformProbe>(probe_platform);
    EXPECT_EQ(mg::PlatformPriority::unsupported, probe());
}

TEST_F(X11GraphicsPlatformTest, probe_returns_unsupported_when_x_cannot_open_display)
{
    using namespace ::testing;

    EXPECT_CALL(mock_x11, XOpenDisplay(_))
        .WillRepeatedly(Return(nullptr));

    mir::SharedLibrary platform_lib{mtf::server_platform("server-mesa-x11")};
    auto probe = platform_lib.load_function<mg::PlatformProbe>(probe_platform);
    EXPECT_EQ(mg::PlatformPriority::unsupported, probe());
}

TEST_F(X11GraphicsPlatformTest, probe_returns_best_when_drm_render_nodes_exist)
{
    mtf::UdevEnvironment udev_environment;

    udev_environment.add_standard_device("standard-drm-render-nodes");

    mir::SharedLibrary platform_lib{mtf::server_platform("server-mesa-x11")};
    auto probe = platform_lib.load_function<mg::PlatformProbe>(probe_platform);
    EXPECT_EQ(mg::PlatformPriority::best, probe());
}
