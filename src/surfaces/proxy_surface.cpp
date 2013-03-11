/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "proxy_surface.h"

#include "mir/surfaces/surface_stack_model.h"
#include "mir/input/input_channel.h"

#include <boost/throw_exception.hpp>

#include <stdexcept>

namespace ms = mir::surfaces;
namespace mc = mir::compositor;
namespace mi = mir::input;

ms::BasicProxySurface::BasicProxySurface(std::weak_ptr<mir::surfaces::Surface> const& surface,
                                         std::shared_ptr<input::InputChannel> const& input_package)
  : surface(surface),
    input_package(input_package)
{
}

void ms::BasicProxySurface::hide()
{
    if (auto const& s = surface.lock())
    {
        s->set_hidden(true);
    }
}

void ms::BasicProxySurface::show()
{
    if (auto const& s = surface.lock())
    {
        s->set_hidden(false);
    }
}

void ms::BasicProxySurface::destroy()
{
}

void ms::BasicProxySurface::shutdown()
{
    if (auto const& s = surface.lock())
    {
        s->shutdown();
    }
}

mir::geometry::Size ms::BasicProxySurface::size() const
{
    if (auto const& s = surface.lock())
    {
        return s->size();
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid surface"));
    }
}

mir::geometry::PixelFormat ms::BasicProxySurface::pixel_format() const
{
    if (auto const& s = surface.lock())
    {
        return s->pixel_format();
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid surface"));
    }
}

void ms::BasicProxySurface::advance_client_buffer()
{
    if (auto const& s = surface.lock())
    {
        s->advance_client_buffer();
    }
}

std::shared_ptr<mc::Buffer> ms::BasicProxySurface::client_buffer() const
{
    if (auto const& s = surface.lock())
    {
        return s->client_buffer();
    }
    else
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("Invalid surface"));
    }
}

void ms::BasicProxySurface::destroy_surface(SurfaceStackModel* const surface_stack) const
{
    surface_stack->destroy_surface(surface);
}

bool ms::BasicProxySurface::supports_input() const
{
    if (input_package)
        return true;
    return false;
}

int ms::BasicProxySurface::client_input_fd() const
{
    if (!supports_input())
        BOOST_THROW_EXCEPTION(std::logic_error("Surface does not support input"));
    return input_package->client_fd();
}

ms::ProxySurface::ProxySurface(
        SurfaceStackModel* const surface_stack_,
        std::shared_ptr<input::InputChannel> const& input_package,
        shell::SurfaceCreationParameters const& params) :
    BasicProxySurface(surface_stack_->create_surface(params), input_package),
    surface_stack(surface_stack_)
{
}

void ms::ProxySurface::destroy()
{
    destroy_surface(surface_stack);
}

ms::ProxySurface::~ProxySurface()
{
    destroy();
}
