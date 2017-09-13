#include "StructureFunctions.h"

namespace CepGen
{
  std::ostream&
  operator<<( std::ostream& os, const StructureFunctionsType& sf )
  {
    switch ( sf ) {
      case Electron:            return os << "electron";
      case ElasticProton:       return os << "elastic proton";
      case SuriYennie:          return os << "Suri-Yennie";
      case SuriYennieLowQ2:     return os << "Suri-Yennie;lowQ2";
      case SzczurekUleshchenko: return os << "Szczurek-Uleshchenko";
      case FioreVal:            return os << "Fiore;valence";
      case FioreSea:            return os << "Fiore;sea";
      case Fiore:               return os << "Fiore";
      case ALLM91:              return os << "ALLM;91";
      case ALLM97:              return os << "ALLM;97";
      default: return os;
    }
  }

  StructureFunctions
  StructureFunctions::FioreBrasse( double q2, double xbj )
  {
    const double mp = Particle::massFromPDGId( Particle::Proton ), mp2 = mp*mp;
    const double akin = 1. + 4.*mp2 * xbj*xbj/q2;
    const double prefactor = q2*( 1.-xbj ) / ( 4.*M_PI*Constants::alphaEM*akin );
    const double s = q2*( 1.-xbj )/xbj + mp2;

    FioreBrasseParameterisation p = FioreBrasseParameterisation::standard();
    double ampli_res = 0., ampli_tot = 0.;
    for ( unsigned short i = 0; i < 3; ++i ) { //FIXME 4??
      const FioreBrasseParameterisation::ResonanceParameters res = p.resonances[i];
      if ( !res.enabled ) continue;
      const double sqrts0 = sqrt( p.s0 );

      std::complex<double> alpha;
      if ( s > p.s0 )
        alpha = std::complex<double>( res.alpha0 + res.alpha2*sqrts0 + res.alpha1*s, res.alpha2*sqrt( s-p.s0 ) );
      else
        alpha = std::complex<double>( res.alpha0 + res.alpha1*s + res.alpha2*( sqrts0 - sqrt( p.s0 - s ) ), 0. );

      double formfactor = 1./pow( 1. + q2/res.q02, 2 );
      double denom = pow( res.spin-std::real( alpha ), 2 ) + pow( std::imag( alpha ), 2 );
      double ampli_imag = res.a*formfactor*formfactor*std::imag( alpha )/denom;
      ampli_res += ampli_imag;
    }
    {
      const FioreBrasseParameterisation::ResonanceParameters res = p.resonances[3];
      double sE = res.alpha2, sqrtsE = sqrt( sE );
      std::complex<double> alpha;
      if ( s > sE )
        alpha = std::complex<double>( res.alpha0 + res.alpha1*sqrtsE, res.alpha1*sqrt( s-sE ) );
      else
        alpha = std::complex<double>( res.alpha0 + res.alpha1*( sqrtsE - sqrt( sE-s ) ), 0. );
      double formfactor = 1./pow( 1. + q2/res.q02, 2 );
      double sp = 1.5*res.spin;
      double denom = pow( sp-std::real( alpha ), 2 ) + pow( std::imag( alpha ), 2 );
      double ampli_bg = res.a*formfactor*formfactor*std::imag( alpha )/denom;
      ampli_res += ampli_bg;
    }
    ampli_tot = p.norm*ampli_res;

    StructureFunctions fb;
    fb.F2 = prefactor*ampli_tot;
    return fb;
  }

  StructureFunctions
  StructureFunctions::FioreBrasseOld( double q2, double xbj )
  {
    const double mp = Particle::massFromPDGId( Particle::Proton ), mp2 = mp*mp;
    //const double m_min = Particle::massFromPDGId(Particle::Proton)+0.135;
    const double m_min = mp+Particle::massFromPDGId( Particle::PiZero );

    const double mx2 = mp2 + q2*( 1.-xbj )/xbj, mx = sqrt( mx2 );

    if ( mx < m_min || mx > 1.99 ) {
      InWarning( Form( "Fiore-Brasse form factors to be retrieved for an invalid MX value:\n\t"
                       "%.2e GeV, while allowed range is [1.07, 1.99] GeV", mx ) );
      return StructureFunctions();
    }

    int n_bin;
    double x_bin, dx;
    if ( mx < 1.11 ) {
      n_bin = 0;
      x_bin = mx-m_min;
      dx = 1.11-m_min; // Delta w bin sizes
    }
    else if ( mx < 1.77 ) { // w in [1.11, 1.77[
      dx = 0.015; // Delta w bin sizes
      n_bin = ( mx-1.11 )/dx + 1;
      x_bin = fmod( mx-1.11, dx );
    }
    else { // w in [1.77, 1.99[
      dx = 0.02; // Delta w bin sizes
      n_bin = ( mx-1.77 )/dx + 45;
      x_bin = fmod( mx-1.77, dx );
    }

    // values of a, b, c provided from the fits on ep data and retrieved from
    // http://dx.doi.org/10.1016/0550-3213(76)90231-5 with 1.110 <= w2 <=1.990
    const double a[56] = { 5.045, 5.126, 5.390,5.621, 5.913, 5.955,6.139,6.178,6.125, 5.999,
                           5.769, 5.622, 5.431,5.288, 5.175, 5.131,5.003,5.065,5.045, 5.078,
                           5.145, 5.156, 5.234,5.298, 5.371, 5.457,5.543,5.519,5.465, 5.384,
                           5.341, 5.320, 5.275,5.290, 5.330, 5.375,5.428,5.478,5.443, 5.390,
                           5.333, 5.296, 5.223,5.159, 5.146, 5.143,5.125,5.158,5.159, 5.178,
                           5.182, 5.195, 5.160,5.195, 5.163, 5.172 },
                 b[56] = { 0.798, 1.052, 1.213,1.334,1.397,1.727,1.750,1.878,1.887,1.927,
                           2.041, 2.089, 2.148,2.205,2.344,2.324,2.535,2.464,2.564,2.610,
                           2.609, 2.678, 2.771,2.890,2.982,3.157,3.183,3.315,3.375,3.450,
                           3.477, 3.471, 3.554,3.633,3.695,3.804,3.900,4.047,4.290,4.519,
                           4.709, 4.757, 4.840,5.017,5.015,5.129,5.285,5.322,5.545,5.623,
                           5.775, 5.894, 6.138,6.151,6.301,6.542 },
                 c[56] = { 0.043, 0.024, 0.000,-0.013,-0.023,-0.069,-0.060,-0.080,-0.065,-0.056,
                          -0.065,-0.056,-0.043,-0.034,-0.054,-0.018,-0.046,-0.015,-0.029,-0.048,
                          -0.032,-0.045,-0.084,-0.115,-0.105,-0.159,-0.164,-0.181,-0.203,-0.223,
                          -0.245,-0.254,-0.239,-0.302,-0.299,-0.318,-0.383,-0.393,-0.466,-0.588,
                          -0.622,-0.568,-0.574,-0.727,-0.665,-0.704,-0.856,-0.798,-1.048,-0.980,
                          -1.021,-1.092,-1.313,-1.341,-1.266,-1.473 };

    const double d = 3.0;
    const double nu = 0.5 * ( q2 + mx2 - mp2 ) / mp, nu2 = nu*nu,
                 logqq0 = 0.5 * log( ( nu2+q2 ) / pow( ( mx2-mp2 ) / ( 2.*mp ), 2 ) );
    const double gd2 = pow( 1. / ( 1+q2 / .71 ), 4 ); // dipole form factor of the proton

    const double sigLow = ( n_bin == 0 ) ? 0. :
      gd2 * exp( a[n_bin-1] + b[n_bin-1]*logqq0 + c[n_bin-1]*pow( fabs( logqq0 ), d ) );
    const double sigHigh =
      gd2 * exp( a[n_bin]   + b[n_bin]  *logqq0 + c[n_bin]  *pow( fabs( logqq0 ), d ) );

    const double sigma_t = sigLow + x_bin*( sigHigh-sigLow )/dx;
    const double w1 = ( mx2-mp2 )/( 8.*M_PI*M_PI*mp*Constants::alphaEM )/Constants::GeV2toBarn*1.e6 * sigma_t;
    const double w2 = w1 * q2 / ( q2+nu2 );

    return StructureFunctions( w1, w2 );
  }

  StructureFunctions
  StructureFunctions::SuriYennie( double q2, double xbj, const SuriYennieParameterisation& param )
  {
    const double mp = Particle::massFromPDGId( Particle::Proton ), mp2 = mp*mp;
    const double mx2 = q2 * ( 1.-xbj )/xbj + mp2, // [GeV^2]
                 nu = 0.5 * ( q2 + mx2 - mp2 ) / mp; // [GeV]
    const double dm2 = mx2-mp2, Xpr = q2/( q2+mx2 ), En = dm2+q2, Tau = 0.25 * q2/mp2, MQ = param.rho2+q2;

    StructureFunctions sy;
    sy.FM = ( param.C1*dm2*pow( param.rho2/MQ, 2 ) + ( param.C2*mp2*pow( 1.-Xpr, 4 ) ) / ( 1.+Xpr*( Xpr*param.Cp-2.*param.Bp ) ) )/q2;
    const double FE = ( Tau*sy.FM + param.D1*dm2*q2*param.rho2/mp2*pow( dm2/MQ/En, 2 ) ) / ( 1.+0.25*En*En/mp2/q2 );

    const double w2 = 2.*mp*FE, w1 = 0.5 * sy.FM*q2/mp;

    sy.F2 = nu/mp*w2;
    sy.F1 = 0.5*sy.F2/xbj; // Callan-Gross relation FIXME
    return sy;
  }

  StructureFunctions
  StructureFunctions::SzczurekUleshchenko( double q2, double xbj )
  {
#ifndef GRVPDF
    FatalError( "Szczurek-Uleshchenko structure functions cannot be computed"
                " as GRV PDF set is not linked to this instance!" );
#else
    const float q02 = 0.8;
    float amu2 = q2+q02; // shift the overall scale
    float xuv, xdv, xus, xds, xss, xg;
    float xbj_arg = xbj;

    grv95lo_( xbj_arg, amu2, xuv, xdv, xus, xds, xss, xg );

    DebuggingInsideLoop( Form( "Form factor content at xB = %e (scale = %f GeV^2):\n\t"
                               "  valence quarks: u / d     = %e / %e\n\t"
                               "  sea quarks:     u / d / s = %e / %e / %e\n\t"
                               "  gluons:                   = %e",
                               xbj, amu2, xuv, xdv, xus, xds, xss, xg ) );

    // standard partonic structure function
    const double F2_aux = 4./9.*( xuv + 2.*xus )
                        + 1./9.*( xdv + 2.*xds )
                        + 1./9.*(       2.*xss );

    // F2 corrected for low Q^2 behaviour
    const double F2_corr = F2_aux * q2 / amu2,
                 F1 = 0.5*F2_corr/xbj; // Callan-Gross relation

    return StructureFunctions( F1, F2_corr );
#endif
  }

  StructureFunctions
  StructureFunctions::ALLM( double q2, double xbj, const ALLMParameterisation& param )
  {
    const double factor = q2/( q2+param.m02 );
    const double W2_eff = q2*( 1.-xbj )/xbj;
    const double xp = ( q2+param.mp2 )/( q2+W2_eff+param.mp2 ),
                 xr = ( q2+param.mr2 )/( q2+W2_eff+param.mr2 );

    const double xlog1 = log( ( q2+param.q02 )/ param.lam2 ), xlog2 = log( param.q02/param.lam2 );
    const double t = log( xlog1/xlog2 );

    const double cpom = param.pomeron.c[0] + ( param.pomeron.c[0]-param.pomeron.c[1] )*( 1./( 1.+pow( t, param.pomeron.c[2] ) ) - 1. );
    const double apom = param.pomeron.a[0] + ( param.pomeron.a[0]-param.pomeron.a[1] )*( 1./( 1.+pow( t, param.pomeron.a[2] ) ) - 1. );
    const double creg = param.reggeon.c[0] + param.reggeon.c[1]*pow( t, param.reggeon.c[2] );
    const double areg = param.reggeon.a[0] + param.reggeon.a[1]*pow( t, param.reggeon.a[2] );
    const double bpom = param.pomeron.b[0] + param.pomeron.b[1]*pow( t, param.pomeron.b[2] );
    const double breg = param.reggeon.b[0] + param.reggeon.b[1]*pow( t, param.reggeon.b[2] );

    const double F2_Pom = factor*cpom*pow( xp, apom )*pow( 1.-xbj, bpom ),
                 F2_Reg = factor*creg*pow( xr, areg )*pow( 1.-xbj, breg );

    StructureFunctions allm;
    allm.F2 = F2_Pom + F2_Reg;
    return allm;
  }

  std::ostream&
  operator<<( std::ostream& os, const StructureFunctions& sf )
  {
    return os << "F1 = " << sf.F1 << ", F2 = " << sf.F2;
  }
}
