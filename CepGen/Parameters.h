#ifndef Parameters_h
#define Parameters_h

#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <string>
#include <memory>

#include "CepGen/Physics/Kinematics.h"
#include "CepGen/Processes/GamGamLL.h"
#include "CepGen/Processes/PPtoLL.h"

#include "CepGen/Hadronisers/Pythia6Hadroniser.h"

namespace CepGen
{
  /// List of parameters used to start and run the simulation job
  /// \note The default parameters are derived from GMUINI in LPAIR
  class Parameters {
    public:
      Parameters();
      /// Copy constructor (transfers ownership to the process!)
      Parameters( const Parameters& );
      ~Parameters();
      /// Set the polar angle range for the produced leptons
      /// \param[in] thetamin The minimal value of \f$\theta\f$ for the outgoing leptons
      /// \param[in] thetamax The maximal value of \f$\theta\f$ for the outgoing leptons
      void setThetaRange( float thetamin, float thetamax );
      /// Dump the input parameters in the console
      void dump( std::ostream& os=Logger::get().outputStream, bool pretty=true ) const;

      //----- process to compute

      /// Process for which the cross-section will be computed and the events will be generated
      std::shared_ptr<Process::GenericProcess> process;
      void setProcess( Process::GenericProcess* proc ) { process.reset( proc ); }
      /// Type of outgoing state to consider for the incoming primary particles
      Kinematics::ProcessMode process_mode;

      /// Type of remnant fragmentation algorithm to use
      /// \note Was named PMOD/EMOD in ILPAIR
      StructureFunctions remnant_mode;

      //----- events kinematics

      Kinematics kinematics;

      //----- VEGAS

      struct Vegas
      {
        Vegas() : ncvg( 100000 ), itvg( 10 ), npoints( 5000 ), first_run( true ) {}
        unsigned int ncvg; // ??
        /// Maximal number of iterations to perform by VEGAS
        unsigned int itvg;
        /// Number of points to "shoot" in each integration bin by the algorithm
        unsigned int npoints;
        /// Is it the first time the integrator is run?
        bool first_run;
      };
      Vegas vegas;

      //----- events generation

      /// Are we generating events ? (true) or are we only computing the cross-section ? (false)
      bool generation;
      /// Are the events generated in this run to be stored in the output file ?
      bool store;
      /// Maximal number of events to generate in this run
      unsigned int maxgen;
      /// Pointer to the last event produced in this run
      std::shared_ptr<Event> last_event;
      /// Do we want the events to be symmetrised with respect to the \f$z\f$-axis ?
      bool symmetrise;

      //----- PDFLIB information
      /// Number of events already generated in this run
      unsigned int ngen;
      /// PDFLIB group to use
      unsigned int gpdf;
      /// PDFLIB set to use
      unsigned int spdf;
      /// Number of quarks to consider in the hadronisation part
      unsigned int qpdf;

      //----- hadronisation

      /// Hadronisation algorithm to use for the proton(s) fragmentation
      std::shared_ptr<Hadroniser::GenericHadroniser> hadroniser;
      void setHadroniser( Hadroniser::GenericHadroniser* hadr ) { hadroniser.reset( hadr ); }
      /// Maximal number of trials for the hadronisation of the proton(s) remnants
      unsigned int hadroniser_max_trials;
  };
}

#endif
