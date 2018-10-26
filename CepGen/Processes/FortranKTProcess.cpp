#include "CepGen/Processes/FortranKTProcess.h"
#include "CepGen/Processes/Fortran/KTStructures.h"

#include "CepGen/Core/ParametersList.h"

#include "CepGen/StructureFunctions/StructureFunctions.h"
#include "CepGen/Event/Event.h"

#include "CepGen/Physics/KTFlux.h"
#include "CepGen/Physics/Constants.h"
#include "CepGen/Physics/PDG.h"

extern "C"
{
  extern cepgen::ktblock::Constants constants_;
  extern cepgen::ktblock::Parameters params_;
  extern cepgen::ktblock::KTKinematics ktkin_;
  extern cepgen::ktblock::Cuts kincuts_;
  extern cepgen::ktblock::Event evtkin_;
}

namespace cepgen
{
  namespace proc
  {
    FortranKTProcess::FortranKTProcess( const ParametersList& params, const char* name, const char* descr, std::function<void( double& )> func ) :
      GenericKTProcess( params, name, descr, { { PDG::photon, PDG::photon } }, { PDG::muon, PDG::muon } ),
      pair_( params.get<int>( "pair", 13 ) ),
      method_( params.get<int>( "method", 1 ) ),
      func_( func )
    {
      constants_.m_p = GenericProcess::mp_;
      constants_.units = constants::GEV2_TO_BARN;
      constants_.pi = M_PI;
      constants_.alpha_em = constants::ALPHA_EM;
    }

    void
    FortranKTProcess::preparePhaseSpace()
    {
      mom_ip1_ = event_->getOneByRole( Particle::IncomingBeam1 ).momentum();
      mom_ip2_ = event_->getOneByRole( Particle::IncomingBeam2 ).momentum();

      registerVariable( y1_, Mapping::linear, kin_.cuts.central.rapidity_single, { -6., 6. }, "First central particle rapidity" );
      registerVariable( y2_, Mapping::linear, kin_.cuts.central.rapidity_single, { -6., 6. }, "Second central particle rapidity" );
      registerVariable( pt_diff_, Mapping::linear, kin_.cuts.central.pt_diff, { 0., 50. }, "Transverse momentum difference between central particles" );
      registerVariable( phi_pt_diff_, Mapping::linear, kin_.cuts.central.phi_pt_diff, { 0., 2.*M_PI }, "Central particles azimuthal angle difference" );

      //===========================================================================================
      // feed phase space cuts to the common block
      //===========================================================================================

      kin_.cuts.central.pt_single.save( (bool&)kincuts_.ipt, kincuts_.pt_min, kincuts_.pt_max );
      kin_.cuts.central.energy_single.save( (bool&)kincuts_.iene, kincuts_.ene_min, kincuts_.ene_max );
      kin_.cuts.central.eta_single.save( (bool&)kincuts_.ieta, kincuts_.eta_min, kincuts_.eta_max );
      kin_.cuts.central.mass_sum.save( (bool&)kincuts_.iinvm, kincuts_.invm_min, kincuts_.invm_max );
      kin_.cuts.central.pt_sum.save( (bool&)kincuts_.iptsum, kincuts_.ptsum_min, kincuts_.ptsum_max );
      kin_.cuts.central.rapidity_diff.save( (bool&)kincuts_.idely, kincuts_.dely_min, kincuts_.dely_max );

      //===========================================================================================
      // feed run parameters to the common block
      //===========================================================================================

      params_.icontri = (int)kin_.mode;
      params_.imethod = method_;
      params_.sfmod = (int)kin_.structure_functions->type;
      params_.pdg_l = pair_;

      //-------------------------------------------------------------------------------------------
      // incoming beams information
      //-------------------------------------------------------------------------------------------

      params_.inp1 = kin_.incoming_beams.first.pz;
      params_.inp2 = kin_.incoming_beams.second.pz;
      const HeavyIon in1 = (HeavyIon)kin_.incoming_beams.first.pdg;
      if ( in1 ) {
        params_.a_nuc1 = in1.A;
        params_.z_nuc1 = (unsigned short)in1.Z;
        if ( params_.z_nuc1 > 1 ) {
          event_->getOneByRole( Particle::IncomingBeam1 ).setPdgId( (PDG)in1 );
          event_->getOneByRole( Particle::OutgoingBeam1 ).setPdgId( (PDG)in1 );
        }
      }
      else
        params_.a_nuc1 = params_.z_nuc1 = 1;

      const HeavyIon in2 = (HeavyIon)kin_.incoming_beams.second.pdg;
      if ( in2 ) {
        params_.a_nuc2 = in2.A;
        params_.z_nuc2 = (unsigned short)in2.Z;
        if ( params_.z_nuc2 > 1 ) {
          event_->getOneByRole( Particle::IncomingBeam2 ).setPdgId( (PDG)in2 );
          event_->getOneByRole( Particle::OutgoingBeam2 ).setPdgId( (PDG)in2 );
        }
      }
      else
        params_.a_nuc2 = params_.z_nuc2 = 1;

      //-------------------------------------------------------------------------------------------
      // intermediate partons information
      //-------------------------------------------------------------------------------------------

      params_.iflux1 = (int)kin_.incoming_beams.first.kt_flux;
      params_.iflux2 = (int)kin_.incoming_beams.second.kt_flux;
      if ( (KTFlux)params_.iflux1 == KTFlux::P_Gluon_KMR )
        event_->getOneByRole( Particle::Parton1 ).setPdgId( PDG::gluon );
      if ( (KTFlux)params_.iflux2 == KTFlux::P_Gluon_KMR )
        event_->getOneByRole( Particle::Parton2 ).setPdgId( PDG::gluon );
    }

    double
    FortranKTProcess::computeKTFactorisedMatrixElement()
    {
      ktkin_.q1t = qt1_;
      ktkin_.q2t = qt2_;
      ktkin_.phiq1t = phi_qt1_;
      ktkin_.phiq2t = phi_qt2_;
      ktkin_.y1 = y1_;
      ktkin_.y2 = y2_;
      ktkin_.ptdiff = pt_diff_;
      ktkin_.phiptdiff = phi_pt_diff_;
      ktkin_.m_x = MX_;
      ktkin_.m_y = MY_;

      //--- compute the event weight
      double weight = 0.;
      func_( weight );
      return weight;
    }

    void
    FortranKTProcess::fillCentralParticlesKinematics()
    {
      //===========================================================================================
      // outgoing beam remnants
      //===========================================================================================

      PX_ = Particle::Momentum( evtkin_.px );
      PY_ = Particle::Momentum( evtkin_.py );
      // express these momenta per nucleon
      PX_ *= 1./params_.a_nuc1;
      PY_ *= 1./params_.a_nuc2;

      //===========================================================================================
      // intermediate partons
      //===========================================================================================

      const Particle::Momentum mom_par1 = mom_ip1_-PX_, mom_par2 = mom_ip2_-PY_;
      event_->getOneByRole( Particle::Parton1 ).setMomentum( mom_par1 );
      event_->getOneByRole( Particle::Parton2 ).setMomentum( mom_par2 );
      event_->getOneByRole( Particle::Intermediate ).setMomentum( mom_par1+mom_par2 );

      //===========================================================================================
      // central system
      //===========================================================================================

      Particles& oc = event_->operator[]( Particle::CentralSystem );
      for ( int i = 0; i < evtkin_.nout; ++i ) {
        Particle& p = oc[i];
        p.setPdgId( evtkin_.pdg[i] );
        p.setStatus( Particle::Status::FinalState );
        p.setMomentum( Particle::Momentum( evtkin_.pc[i] ) );
      }
    }
  }
}
