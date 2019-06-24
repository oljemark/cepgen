#include "CepGen/Hadronisers/GenericHadroniser.h"
#include "CepGen/Hadronisers/HadronisersHandler.h"

#include "CepGen/Event/Event.h"
#include "CepGen/Event/Particle.h"

#include "CepGen/Physics/PDG.h"

#include "CepGen/Core/ParametersList.h" //FIXME
#include "CepGen/Core/Exception.h"
#include "CepGen/Core/utils.h"

#include <algorithm>

/// Maximal number of characters to fetch for the particle's name
#define NAME_CHR 16

extern "C"
{
  /// Get the particle's mass in GeV from the Pythia6 module
  extern double pymass_( int& );
  /// Get the resonant particle's width in GeV from the Pythia6 module
  //extern double pywidt_( int& );
  /// Launch the Pythia6 fragmentation
  extern void pyexec_();
  /// Set a parameter value to the Pythia6 module
  extern void pygive_( const char*, int );
  extern void pyckbd_();
  /// List all the particles in the event in a human-readable format
  extern void pylist_( int& );
  /// Join two coloured particles in a colour singlet
  extern void pyjoin_( int&, int& );
  /// Get a particle's human-readable name from the Pythia6 module
  extern void pyname_( int&, char*, int );
  /// Get kinematic information on a particle from the Pythia6 module
  extern double pyp_( int&, int& );
  /// Store one parton/particle in the PYJETS common block
  extern void py1ent_( int&, int&, double&, double&, double& );
  void pystop_() {}

  /// Particles content of the event
  extern struct
  {
    /// Number of particles in the event
    int n;
    int npad;
    /// Particles' general information (status, PDG id, mother, daughter 1, daughter 2)
    int k[5][4000];
    /// Particles' kinematics, in GeV (px, py, pz, E, M)
    double p[5][4000];
    /// Primary vertex for the particles
    double v[5][4000];
  } pyjets_;
}

namespace cepgen
{
  namespace hadr
  {
    /**
     * Full interface to the Pythia6 @cite Sjostrand:2006za algorithm. It can be used in a single particle decay mode as well as a full event hadronisation using the string model, as in Jetset.
     * @brief Pythia6 hadronisation algorithm
     */
    class Pythia6Hadroniser : public GenericHadroniser
    {
      public:
        Pythia6Hadroniser( const ParametersList& );

        void setParameters( const Parameters& ) override {}
        void readString( const char* param ) override;
        void init() override {}
        bool run( Event& ev, double& weight, bool full ) override;

        void setCrossSection( double xsec, double xsec_err ) override {}
        //bool hadronise( const Particle* );

      private:
        static constexpr unsigned short MAX_PART_STRING = 3;
        static constexpr unsigned short MAX_STRING_EVENT = 2;

        inline static double pymass(int pdgid_) { return pymass_(pdgid_); }
        //inline static double pywidt(int pdgid_) { return pywidt_(pdgid_); }
        inline static void pyexec() { pyexec_(); }
        inline static void pyckbd() { pyckbd_(); }
        inline static void pygive( const std::string& line ) { pygive_( line.c_str(), line.length() ); }
        inline static void pylist( int mlist ) { pylist_( mlist ); }
        inline static double pyp( int role, int qty ) { return pyp_( role, qty ); }
        //inline static void py1ent( int* kf, double* pe, double theta, double phi ) { int one=1; py1ent_( &one, kf, pe, theta, phi ); }
        //inline static void py1ent( int* kf, double* pe, double theta, double phi ) { py1ent_( 1, kf, pe, theta, phi ); }
        inline static std::string pyname( int pdgid ) {
          char out[NAME_CHR];
          std::string s;
          pyname_( pdgid, out, NAME_CHR );
          s = std::string( out, NAME_CHR );
          //s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
          s.erase( remove( s.begin(), s.end(), ' ' ), s.end() );
          return s;
        }
        /**
         * \brief Connect entries with colour flow information
         * \param[in] njoin_ Number of particles to join in the colour flow
         * \param[in] ijoin_ List of particles unique identifier to join in the colour flow
         */
        inline static void pyjoin( int njoin, int ijoin[2] ) { return pyjoin_( njoin, *ijoin ); }
        bool prepareHadronisation( Event& );
        std::pair<short,short> pickPartonsContent() const;
        struct EventProperties
        {
          unsigned int str_in_evt = 0;
          unsigned int num_part_in_str[MAX_STRING_EVENT] = { 0 };
          int jlrole[MAX_STRING_EVENT] = { -1 };
          int jlpsf[MAX_STRING_EVENT][MAX_PART_STRING] = { 0 };
        };
        EventProperties fillParticles( const Event& ) const;
    };

    Pythia6Hadroniser::Pythia6Hadroniser( const ParametersList& plist ) :
      GenericHadroniser( plist, "pythia6" )
    {}

    void
    Pythia6Hadroniser::readString( const char* param )
    {
      pygive( param );
    }

    bool
    Pythia6Hadroniser::run( Event& ev, double& weight, bool full )
    {
      weight = 1.;
      //if ( full )
        prepareHadronisation( ev );

      if ( utils::Logger::get().level >= utils::Logger::Level::debug ) {
        CG_DEBUG( "Pythia6Hadroniser" ) << "Dump of the event before the hadronisation:";
        ev.dump();
      }

      //--- fill Pythia 6 common blocks
      auto prop = fillParticles( ev );

      CG_DEBUG( "Pythia6Hadroniser" )
        << "Passed the string construction stage.\n\t"
        << " " << prop.str_in_evt << " string objects were identified and constructed.";

      unsigned int oldnpart = pyjets_.n;

      for ( unsigned short i = 0; i < prop.str_in_evt; ++i ) {
        if ( prop.num_part_in_str[i] < 2 )
          continue;

        std::ostringstream dbg;
        for ( unsigned short j = 0; j < prop.num_part_in_str[i]; ++j )
          if ( prop.jlpsf[i][j] != -1 )
            dbg << Form( "\n\t * %2d (pdgId=%4d)", prop.jlpsf[i][j], pyjets_.k[1][prop.jlpsf[i][j]-1] );

        CG_INFO( "Pythia6Hadroniser" )
          << "Joining " << prop.num_part_in_str[i] << " particle" << utils::s( prop.num_part_in_str[i] )
          << " in a same string (" << i << ")"
          << dbg.str();

        pyjoin( prop.num_part_in_str[i], prop.jlpsf[i] );
      }
      pyexec();

      int criteria = oldnpart+1;
      for ( unsigned int i = 0; i < MAX_STRING_EVENT; ++i )
        criteria += prop.num_part_in_str[i];

      if ( pyjets_.k[1][criteria] == 2212
        && pyjets_.k[0][criteria] == 1 ) {
        CG_WARNING( "Pythia6Hadroniser" ) << "System is non-inelastic.";
        return false;
      }

      // We filter the first particles already present in the event
      for ( unsigned int p = oldnpart; p < (unsigned int)pyjets_.n; ++p ) {
        const Particle::Role role = pyjets_.k[2][p] != 0
          ? ev[pyjets_.k[2][p]-1].role() // child particle inherits its mother's role
          : Particle::Role::UnknownRole;

        auto& pa = ev.addParticle( role );
        pa.setId( p );
        pa.setPdgId( (short)pyjets_.k[1][p] );
        pa.setStatus( static_cast<Particle::Status>( pyjets_.k[0][p] ) );
        pa.setMomentum( Particle::Momentum( pyjets_.p[0][p], pyjets_.p[1][p], pyjets_.p[2][p], pyjets_.p[3][p] ) );
        pa.setMass( pyjets_.p[4][p] );
        //pa.setCharge( (float)pyp( p+1, 6 ) );
        if ( role != Particle::Role::UnknownRole ) {
          auto& moth = ev[pyjets_.k[2][p]-1];
          moth.setStatus( role == Particle::Role::CentralSystem
                          ? Particle::Status::Resonance
                          : Particle::Status::Fragmented );
          pa.addMother( moth );
        }
      }
      return true;
    }

    bool
    Pythia6Hadroniser::prepareHadronisation( Event& ev )
    {
      CG_DEBUG( "Pythia6Hadroniser" ) << "Hadronisation preparation called.";

      for ( const auto& part : ev.particles() ) {
        if ( part.status() != Particle::Status::Unfragmented )
          continue;
        //--- only loop over all protons to be fragmented

part.dump();

        const auto partons = pickPartonsContent();
        const double mx2 = part.mass2();
        const double mq = pymass( partons.first ), mq2 = mq*mq;
        const double mdq = pymass( partons.second ), mdq2 = mdq*mdq;

        //--- choose random direction in MX frame
        const double phi = 2.*M_PI*drand(), theta = acos( 2.*drand()-1. ); // theta angle

        //--- compute momentum of decay particles from MX
        const double px = std::sqrt( std::pow( mx2-mdq2+mq2, 2 ) / ( 4.*mx2 ) - mq2 );

        //--- build 4-vectors and boost decay particles
        const auto pq = Particle::Momentum::fromPThetaPhi( px, theta, phi, std::hypot( px, mq ) );

        //--- singlet
        auto singl_mom = part.momentum();
        singl_mom.lorentzBoost( pq );

        auto& quark = ev.addParticle( part.role(), false );
        quark.setPdgId( partons.first );
        quark.setStatus( Particle::Status::DebugResonance );
        quark.setMomentum( singl_mom );

        //--- doublet
        auto doubl_mom = part.momentum();
        doubl_mom.lorentzBoost( -pq );

        auto& diquark = ev.addParticle( part.role(), false );
        diquark.setPdgId( partons.second );
        diquark.setStatus( Particle::Status::DebugResonance );
        diquark.setMomentum( doubl_mom );

        std::cout << (singl_mom+doubl_mom)-part.momentum() << "|" << part.numDaughters() << std::endl;
        /*if ( part.numDaughters() == 0 ) {
          quark.addMother( ev[part.id()] );
          diquark.addMother( ev[part.id()] );
          CG_DEBUG( "Pythia6Hadroniser" )
            << "Quark/diquark content succesfully added to the event.";
        }
        else { // Quark/diquark content already present in the event
          CG_WARNING( "Pythia6Hadroniser" )
            << "Quark/diquark content already present in the event!\n\t"
            << "Role of these particles: " << part.role() << ".";

          for ( const auto& daug : part.daughters() ) {
            if ( ev[daug].pdgId() == PDG::up || ev[daug].pdgId() == PDG::down ) {
              //--- quark
              quark.addMother( ev[part.id()] );
              ev[daug] = quark;
              CG_DEBUG( "Pythia6Hadroniser" ) << "Singlet replaced.";
            }
            else {
              //--- diquark
              diquark.addMother( ev[part.id()] );
              ev[daug] = diquark;
              CG_DEBUG( "Pythia6Hadroniser" ) << "Doublet replaced";
            }
          }
        }*/
        ev[part.id()].setStatus( Particle::Status::Fragmented );
      }
ev.dump();

      return true;
    }

    Pythia6Hadroniser::EventProperties
    Pythia6Hadroniser::fillParticles( const Event& ev ) const
    {
      pyjets_.n = 0;

      //--- initialising the string fragmentation variables
      EventProperties out;
      out.str_in_evt = 0;

      for ( const auto& role : ev.roles() ) { // loop on roles
        unsigned int part_in_str = 0;
        for ( const auto& part : ev[role] ) {
          unsigned int np = part.id();

          pyjets_.p[0][np] = part.momentum().px();
          pyjets_.p[1][np] = part.momentum().py();
          pyjets_.p[2][np] = part.momentum().pz();
          pyjets_.p[3][np] = part.energy();
          pyjets_.p[4][np] = part.mass();
          pyjets_.k[0][np] = part.status() <= Particle::Status::Undefined
            ? 21 // incoming beam
            : (int)part.status();
          pyjets_.k[1][np] = part.integerPdgId();
          pyjets_.k[2][np] = part.mothers().empty()
            ? 0 // no mother
            : *( part.mothers().begin() )+1; // mother
          const auto& daug = part.daughters();
          if ( daug.empty() )
            pyjets_.k[3][np] = pyjets_.k[4][np] = 0; // no daughters
          else {
            pyjets_.k[3][np] = *daug.begin()+1; // daughter 1
            pyjets_.k[4][np] = *daug.rbegin()+1; // daughter 2
          }
          for ( int i = 0; i < 5; ++i )
            pyjets_.v[i][np] = 0.;

          if ( part.status() == Particle::Status::DebugResonance ) {
            pyjets_.k[0][np] = 1; //FIXME PYTHIA/JETSET workaround
            out.jlpsf[out.str_in_evt][part_in_str] = part.id()+1;
            out.num_part_in_str[out.str_in_evt]++;
            part_in_str++;
          }
          pyjets_.n++;
        }
        if ( out.jlrole[out.str_in_evt] > 0 )
          out.str_in_evt++;
      }
      return out;
    }

    std::pair<short,short>
    Pythia6Hadroniser::pickPartonsContent() const
    {
      const double ranudq = drand();
      if ( ranudq < 1./9. )
        return { PDG::down, 2203 }; // (d,uu1)
      if ( ranudq < 5./9. )
        return { PDG::up, 2101 };   // (u,ud0)
      return { PDG::up, 2103 };     // (u,ud1)
    }

  }
}

// register hadroniser and define alias
REGISTER_HADRONISER( pythia6, Pythia6Hadroniser )

