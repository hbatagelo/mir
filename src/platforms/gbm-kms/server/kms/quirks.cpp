/*
 * Copyright © 2021 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quirks.h"

#include "mir/log.h"
#include "mir/options/option.h"
#include "mir/udev/wrapper.h"

#include <vector>
#include <unordered_set>

namespace mgg = mir::graphics::gbm;
namespace mo = mir::options;

namespace
{
char const* quirks_option_name = "driver-quirks";
}

namespace
{
auto value_or(char const* maybe_null_string, char const* value_if_null) -> char const*
{
    if (maybe_null_string)
    {
        return maybe_null_string;
    }
    else
    {
        return value_if_null;
    }
}
}

class mgg::Quirks::Impl
{
public:
    explicit Impl(mo::Option const& options)
    {
        if (!options.is_set(quirks_option_name))
        {
            return;
        }

        for (auto const& quirk : options.get<std::vector<std::string>>(quirks_option_name))
        {
            auto const disable_kms_probe = "disable-kms-probe:";

            if (quirk.starts_with("skip:"))
            {
                // Quirk format is skip:(devnode|driver):value
                auto type_delimeter_pos = quirk.find(':', 5);
                auto const type = quirk.substr(5, type_delimeter_pos - 5);
                auto const value = quirk.substr(type_delimeter_pos + 1);
                if (type == "devnode")
                {
                    devnodes_to_skip.insert(value);
                    continue;
                }
                else if (type == "driver")
                {
                    drivers_to_skip.insert(value);
                    continue;
                }
            }
            else if (quirk.starts_with(disable_kms_probe))
            {
                // Quirk format is disable-kms-probe:value
                skip_modesetting_support.emplace(quirk.substr(strlen(disable_kms_probe)));
                continue;
            }

            // If we didn't `continue` above, we're ignoring...
            mir::log_warning(
                "Ignoring unexpected value for %s option: %s "
                "(expects value of the form “skip:<type>:<value>” or ”disable-kms-probe:<value>”)",
                quirks_option_name,
                quirk.c_str());
        }
    }

    auto should_skip(udev::Device const& device) const -> bool
    {
        auto const devnode = value_or(device.devnode(), "");
        auto const parent_device = device.parent();
        auto const driver =
            [&]()
            {
                if (parent_device)
                {
                    return value_or(parent_device->driver(), "");
                }
                mir::log_warning("udev device has no parent! Unable to determine driver for quirks.");
                return "<UNKNOWN>";
            }();
        mir::log_debug("Quirks: checking device with devnode: %s, driver %s", device.devnode(), driver);
        bool const should_skip_driver = drivers_to_skip.count(driver);
        bool const should_skip_devnode = devnodes_to_skip.count(devnode);
        if (should_skip_driver)
        {
            mir::log_info("Quirks: skipping device %s (matches driver quirk %s)", devnode, driver);
        }
        if (should_skip_devnode)
        {
            mir::log_info("Quirks: skipping device %s (matches devnode quirk %s)", devnode, devnode);
        }
        return should_skip_driver || should_skip_devnode;
    }

    auto require_modesetting_support(mir::udev::Device const& device) const -> bool
    {
        auto const devnode = value_or(device.devnode(), "");
        auto const parent_device = device.parent();
        auto const driver =
            [&]()
            {
                if (parent_device)
                {
                    return value_or(parent_device->driver(), "");
                }
                mir::log_warning("udev device has no parent! Unable to determine driver for quirks.");
                return "<UNKNOWN>";
            }();
        mir::log_debug("Quirks: checking device with devnode: %s, driver %s", device.devnode(), driver);

        bool const should_skip_modesetting_support = skip_modesetting_support.count(driver);
        if (should_skip_modesetting_support)
        {
            mir::log_info("Quirks: skipping modesetting check %s (matches driver quirk %s)", devnode, driver);
        }
        return !should_skip_modesetting_support;
    }

private:
    // Mir's gbm-kms support isn't (yet) working with nvidia
    // Mir's gbm-kms support isn't (yet) working with evdi
    std::unordered_set<std::string> drivers_to_skip = { "nvidia", "evdi" };
    std::unordered_set<std::string> devnodes_to_skip;
    // We know this is currently useful for virtio_gpu
    std::unordered_set<std::string> skip_modesetting_support = { "virtio_gpu" };
};

mgg::Quirks::Quirks(const options::Option& options)
    : impl{std::make_unique<Impl>(options)}
{
}

mgg::Quirks::~Quirks() = default;

auto mgg::Quirks::should_skip(udev::Device const& device) const -> bool
{
    return impl->should_skip(device);
}

void mgg::Quirks::add_quirks_option(boost::program_options::options_description& config)
{
    config.add_options()
        (quirks_option_name,
         boost::program_options::value<std::vector<std::string>>(),
         "[platform-specific] Driver quirks to apply (may be specified multiple times; multiple quirks are combined)");
}

auto mir::graphics::gbm::Quirks::require_modesetting_support(mir::udev::Device const& device) const -> bool
{
    if (getenv("MIR_MESA_KMS_DISABLE_MODESET_PROBE") != nullptr)
    {
        mir::log_debug("MIR_MESA_KMS_DISABLE_MODESET_PROBE is set");
        return false;
    }
    else if (getenv("MIR_GBM_KMS_DISABLE_MODESET_PROBE")  != nullptr)
    {
        mir::log_debug("MIR_GBM_KMS_DISABLE_MODESET_PROBE is set");
        return false;
    }
    else
    {
        return impl->require_modesetting_support(device);
    }
}
