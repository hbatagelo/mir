/*
 * Copyright © 2018 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3 as
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
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "wayland_shm.h"

#include <mir/fd.h>
#include <boost/throw_exception.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <system_error>

struct wl_shm_pool* make_shm_pool(struct wl_shm* shm, int size, void **data)
{
    static auto (*open_shm_file)() -> mir::Fd = []
        {
            static char const* shm_dir;
            open_shm_file = []{ return mir::Fd{open(shm_dir, O_TMPFILE | O_RDWR | O_EXCL, S_IRWXU)}; };

            // Wayland based toolkits typically use $XDG_RUNTIME_DIR to open shm pools
            // so we try that before "/dev/shm". But confined snaps can't access "/dev/shm"
            // so we try "/tmp" if both of the above fail.
            for (auto dir : {const_cast<const char*>(getenv("XDG_RUNTIME_DIR")), "/dev/shm", "/tmp" })
            {
                if (dir)
                {
                    shm_dir = dir;
                    auto fd = open_shm_file();
                    if (fd >= 0)
                        return fd;
                }
            }

            // Workaround for filesystems that don't support O_TMPFILE (E.g. phones based on 3.4 or 3.10
            open_shm_file = []
            {
                char template_filename[] = "/dev/shm/mir-buffer-XXXXXX";
                mir::Fd fd{mkostemp(template_filename, O_CLOEXEC)};
                if (fd != -1)
                {
                    if (unlink(template_filename) < 0)
                    {
                        return mir::Fd{};
                    }
                }
                return fd;
            };

            return open_shm_file();
        };

    auto fd = open_shm_file();

    if (fd < 0) {
        BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to open shm buffer"}));
    }

    if (auto error = posix_fallocate(fd, 0, size))
    {
        BOOST_THROW_EXCEPTION((std::system_error{error, std::system_category(), "Failed to allocate shm buffer"}));
    }

    if ((*data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        BOOST_THROW_EXCEPTION((std::system_error{errno, std::system_category(), "Failed to mmap buffer"}));
    }

    return wl_shm_create_pool(shm, fd, size);
}
