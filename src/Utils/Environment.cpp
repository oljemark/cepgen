/*
 *  CepGen: a central exclusive processes event generator
 *  Copyright (C) 2020-2025  Laurent Forthomme
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

#include <cstdlib>

#include "CepGen/Utils/Collections.h"
#include "CepGen/Utils/Environment.h"
#include "CepGen/Utils/Filesystem.h"
#include "CepGen/Utils/String.h"

namespace cepgen::utils::env {
  std::string get(const std::string& var, const std::string& def) {
    if (const auto out = std::getenv(var.c_str()); out != nullptr)
      return std::string(out);
    return def;
  }

  std::vector<std::string> searchPaths() {
    const auto cepgen_path = fs::path(get("CEPGEN_PATH", "."));
    return std::vector<std::string>{fs::current_path(),
                                    fs::current_path().parent_path(),
                                    cepgen_path,
                                    cepgen_path / "CepGen",
                                    cepgen_path / "lib",
                                    cepgen_path / "lib64",
                                    cepgen_path / "share" / "CepGen",
                                    // additional paths for local builds
                                    cepgen_path / "External",
                                    cepgen_path / "build"};
  }

  void set(const std::string& var, const std::string& value) { setenv(var.c_str(), value.c_str(), 1); }

#ifdef _WIN32
  static constexpr char PATH_DELIM = ';';
#else
  static constexpr char PATH_DELIM = ':';
#endif

  void append(const std::string& var, const std::string& value) {
    auto environment_variables = split(env::get(var, ""), PATH_DELIM);
    environment_variables.emplace_back(value);
    normalise(environment_variables);
    setenv(var.c_str(), utils::merge(environment_variables, std::string(1, PATH_DELIM)).c_str(), 1);
  }

  void unset(const std::string& var) { unsetenv(var.c_str()); }
}  // namespace cepgen::utils::env
