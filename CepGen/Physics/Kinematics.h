#ifndef CepGen_Physics_Kinematics_h
#define CepGen_Physics_Kinematics_h

#include <iosfwd>
#include <memory>
#include <vector>

#include "CepGen/Core/ParametersList.h"
#include "CepGen/Physics/Cuts.h"
#include "CepGen/Physics/IncomingBeams.h"

namespace cepgen {
  enum class KTFlux;
  namespace strfun {
    class Parameterisation;
  }
  namespace formfac {
    class Parameterisation;
  }
  /// List of kinematic constraints to apply on the process phase space.
  class Kinematics {
  public:
    Kinematics() = default;
    explicit Kinematics(const ParametersList&);
    ~Kinematics() = default;

    /// Minimal diffractive mass for dissociative proton treatment
    static const double MX_MIN;

    /// Set a collection of kinematics parameters
    void setParameters(const ParametersList&);
    /// List containing all parameters handled
    ParametersList parameters() const;

    /// Set the incoming particles' momenta (if the collision is symmetric)
    Kinematics& setSqrtS(double sqrts) {
      incoming_beams.setSqrtS(sqrts);
      return *this;
    }
    /// Process centre of mass energy
    double sqrtS() const { return incoming_beams.sqrtS(); }
    /// Beam/primary particle's kinematics
    IncomingBeams incoming_beams;

    /// Minimum list of central particles required
    std::vector<pdgid_t> minimum_final_state;

    /// A collection of cuts to apply on the physical phase space
    struct CutsList {
      CutsList();
      cuts::Initial initial;        ///< Cuts on the initial particles kinematics
      cuts::Central central;        ///< Cuts on the central system produced
      PerIdCuts central_particles;  ///< Cuts on the central individual particles
      cuts::Remnants remnants;      ///< Cuts on the beam remnants system
    } cuts;                         ///< Phase space cuts
    /// Human-readable description of a full kinematics cuts definition
    friend std::ostream& operator<<(std::ostream&, const CutsList&);
  };
}  // namespace cepgen

#endif
