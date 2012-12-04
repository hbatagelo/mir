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
#ifndef MIR_DRAW_PATTERNS_H
#define MIR_DRAW_PATTERNS_H

#include "mir_client/mir_client_library.h"

#include <memory>
/* todo: replace with color value types */
#include <stdint.h>

namespace mir
{
namespace draw
{

class DrawPattern
{
public:
    virtual ~DrawPattern() {};
    virtual void draw(std::shared_ptr<MirGraphicsRegion>& region) const = 0;
    virtual bool check(const std::shared_ptr<MirGraphicsRegion>& region) const = 0;

};

class DrawPatternSolid : public DrawPattern
{
public:
    /* todo: should construct with a color value type, not an uint32 */
    DrawPatternSolid(uint32_t color_value);

    void draw(std::shared_ptr<MirGraphicsRegion>& region) const;
    bool check(const std::shared_ptr<MirGraphicsRegion>& region) const;

private:
    const uint32_t color_value;
};
 
}
}

#endif /*MIR_DRAW_PATTERNS_H */
