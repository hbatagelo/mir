/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored By: Alan Griffiths <alan@octopull.co.uk>
 */

#ifndef MIR_FRONTEND_SURFACE_MODIFICATIONS_H_
#define MIR_FRONTEND_SURFACE_MODIFICATIONS_H_

#include "mir/optional_value.h"

#include <string>

namespace mir
{
namespace frontend
{
struct SurfaceModifications
{
    optional_value<std::string> name;
    // TODO: type/state/size etc (LP: #1422522) (LP: #1420573)
};
}
}

#endif /* MIR_FRONTEND_SURFACE_MODIFICATIONS_H_ */
