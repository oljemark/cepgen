/*
 *  CepGen: a central exclusive processes event generator
 *  Copyright (C) 2013-2025  Laurent Forthomme
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

#ifndef CepGen_Integration_ProcessIntegrand_h
#define CepGen_Integration_ProcessIntegrand_h

#include <memory>

#include "CepGen/EventFilter/EventBrowser.h"
#include "CepGen/Integration/Integrand.h"

namespace cepgen {
  class RunParameters;
}
namespace cepgen::proc {
  class Process;
}
namespace cepgen::utils {
  class Timer;
}

namespace cepgen {
  /// Wrapper to the function to be integrated
  class ProcessIntegrand final : public Integrand {
  public:
    explicit ProcessIntegrand(const proc::Process&);
    explicit ProcessIntegrand(const RunParameters*);

    /// Compute the integrand for a given phase space point (or “event”)
    /// \param[in] x Phase space point coordinates
    /// \note The weight includes the matrix element of the process considered, along with all the kinematic factors,
    ///  and the cut restrictions imposed on this phase space.
    ///  \f${\bf x}=\{x_1,\ldots,x_N\}\f$ is therefore an array of random numbers defined inside its boundaries
    ///  (as normalised so that \f$\forall i=1,\ldots,N\f$, \f$0<x_i<1\f$).
    double eval(const std::vector<double>& x) override;
    size_t size() const override;  ///< Phase space dimension
    bool hasProcess() const override { return true; }

    proc::Process& process();              ///< Thread-local physics process
    const proc::Process& process() const;  ///< Thread-local physics process

    void setStorage(bool store) { storage_ = store; }  ///< Specify if the generated events will be stored
    bool storage() const { return storage_; }          ///< Store the events generated in this run?

  private:
    void setProcess(const proc::Process&);

    std::unique_ptr<proc::Process> process_;        ///< Local instance of the physics process
    const RunParameters* run_parameters_{nullptr};  ///< Generator-owned runtime parameters
    const std::unique_ptr<utils::Timer> timer_;     ///< Timekeeper for event generation
    utils::EventBrowser bws_;                       ///< Event browser
    bool storage_{false};                           ///< Will the next event generated be stored?
  };
}  // namespace cepgen

#endif
