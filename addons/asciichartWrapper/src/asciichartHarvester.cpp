/*
 *  CepGen: a central exclusive processes event generator
 *  Copyright (C) 2024-2025  Laurent Forthomme
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CepGen/EventFilter/EventHarvester.h"
#include "CepGen/Modules/EventExporterFactory.h"

using namespace std::string_literals;

namespace cepgen {
  struct asciichartHarvester final : EventHarvester {
    using EventHarvester::EventHarvester;
    static ParametersDescription description() {
      auto desc = EventHarvester::description();
      desc.setDescription("asciichart event harvester");
      desc.add("plotter", "asciichart"s);
      return desc;
    }
  };
}  // namespace cepgen
REGISTER_EXPORTER("asciichart", asciichartHarvester);
