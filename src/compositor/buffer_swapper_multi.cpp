/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/compositor/buffer_swapper_multi.h"
#include "mir/compositor/buffer.h"
#include "mir/compositor/buffer_id.h"

namespace mc = mir::compositor;

mc::BufferSwapperMulti::BufferSwapperMulti(std::shared_ptr<Buffer> buffer_a,
                                           BufferID id_a,
                                           std::shared_ptr<Buffer> buffer_b,
                                           BufferID id_b)
    : in_use_by_client{0}
{
    buffers[id_a] = buffer_a;
    buffers[id_b] = buffer_b;

    client_queue.push_back(id_a);
    client_queue.push_back(id_b);
}

mc::BufferSwapperMulti::BufferSwapperMulti(std::shared_ptr<Buffer> buffer_a,
                       BufferID id_a,
                       std::shared_ptr<Buffer> buffer_b,
                       BufferID id_b,
                       std::shared_ptr<Buffer> buffer_c,
                       BufferID id_c)
    : in_use_by_client{0}
{
    buffers[id_a] = buffer_a;
    buffers[id_b] = buffer_b;
    buffers[id_c] = buffer_c;

    client_queue.push_back(id_a);
    client_queue.push_back(id_b);
    client_queue.push_back(id_c);
}

void mc::BufferSwapperMulti::client_acquire(std::weak_ptr<mc::Buffer>& buffer_reference, BufferID& dequeued_buffer)
{
    std::unique_lock<std::mutex> lk(swapper_mutex);

    /*
     * Don't allow the client to acquire all the buffers, because then the
     * compositor won't have a buffer to display.
     */
    while (client_queue.empty() || in_use_by_client == buffers.size() - 1)
    {
        client_available_cv.wait(lk);
    }

    dequeued_buffer = client_queue.front();
    client_queue.pop_front();
    buffer_reference = buffers[dequeued_buffer]; 
    in_use_by_client++;
}

void mc::BufferSwapperMulti::client_release(BufferID& queued_buffer)
{
    std::unique_lock<std::mutex> lk(swapper_mutex);

    compositor_queue.push_back(queued_buffer);
    in_use_by_client--;

    /*
     * At this point we could signal the client_available_cv.  However, we
     * won't do so, because the cv will get signaled after the next
     * compositor_release anyway, and we want to avoid the overhead of
     * signaling at every client_release just to ensure quicker client
     * notification in an abnormal situation.
     */
}

void mc::BufferSwapperMulti::compositor_acquire(std::weak_ptr<mc::Buffer>& buffer_reference, BufferID& dequeued_buffer)
{
    std::unique_lock<std::mutex> lk(swapper_mutex);

    if (compositor_queue.empty())
    {
        dequeued_buffer = client_queue.back();
        client_queue.pop_back();
    }
    else
    {
        dequeued_buffer = compositor_queue.front();
        compositor_queue.pop_front();
    }

    buffer_reference = buffers[dequeued_buffer]; 
}

void mc::BufferSwapperMulti::compositor_release(BufferID& released_buffer)
{
    std::unique_lock<std::mutex> lk(swapper_mutex);
    client_queue.push_back(released_buffer);
    client_available_cv.notify_one();
}

void mc::BufferSwapperMulti::shutdown()
{
    std::unique_lock<std::mutex> lk(swapper_mutex);

    if (client_queue.empty())
    {
        auto dequeued_buffer = compositor_queue.front();
        compositor_queue.pop_front();
        client_queue.push_back(dequeued_buffer);
    }

    client_available_cv.notify_all();
}
