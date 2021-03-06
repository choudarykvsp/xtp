/* 
 *            Copyright 2009-2017 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _VOTCA_XTP_SPECTRUM_H
#define _VOTCA_XTP_SPECTRUM_H

#include <stdio.h>
#include <boost/math/constants/constants.hpp>
#include <votca/tools/constants.h>
#include <votca/ctp/logger.h>
#include <votca/ctp/qmtool.h>


namespace votca { namespace xtp {
    using namespace std;
    namespace ub = boost::numeric::ublas;

class Spectrum : public ctp::QMTool
{
public:

    Spectrum() { };
   ~Spectrum() { };

    string Identify() { return "spectrum"; }

    void   Initialize(Property *options);
    bool   Evaluate();


private:
    
    string      _orbfile;
    string      _output_file;
    
    ctp::Logger      _log;
    
    void CheckContent(  Orbitals& _orbitals );
    
    double evtonm( double eV );
    double evtoinvcm( double eV );
    double nmtoinvcm( double nm );
    double invcmtonm( double invcm );
    double nmtoev( double nm );

    
    double _lower; // in eV
    double _upper; // in eV
    int _n_pt;

    int _minexc; // in eV
    int _maxexc; // in eV
    
    double _fwhm; // in eV
    double _shiftby; 
    
    
    string _spectrum_type;
    // lineshape functions
    double Gaussian( double _x, double _center, double _fwhm );
    double Lorentzian( double _x, double _center, double _fwhm );
    double TruncatedLorentzian( double _x, double _center, double _fwhm );
    
};

void Spectrum::Initialize(Property* options) {

            // update options with the VOTCASHARE defaults   
    UpdateWithDefaults( options, "xtp" );
 

            string key = "options." + Identify();
            // _jobfile = options->get(key + ".file").as<string>();

            // key = "options." + Identify();
            
       
           // orbitals file or pure DFT output
           _orbfile      = options->get(key + ".input").as<string> ();
           _output_file  = options->get(key + ".output").as<string> ();
           _n_pt  = options->get(key + ".points").as<int> ();
           _lower  = options->get(key + ".lower").as<double> ();
           _upper  = options->get(key + ".upper").as<double> ();
           _fwhm  = options->get(key + ".fwhm").as<double>();
           _spectrum_type = options->get(key + ".type").as<string> ();
           _minexc  = options->get(key + ".minexc").as<int> ();
	   _maxexc  = options->get(key + ".maxexc").as<int> ();
           _shiftby = options->get(key + ".shift").as<double> ();
    
  
    // get the path to the shared folders with xml files
    char *votca_share = getenv("VOTCASHARE");    
    if(votca_share == NULL) throw std::runtime_error("VOTCASHARE not set, cannot open help files.");
    // string xmlFile = string(getenv("VOTCASHARE")) + string("/xtp/packages/") + _package + string("_idft_pair.xml");
    // load_property_from_xml( _package_options, xmlFile );    

    // register all QM packages (Gaussian, TURBOMOLE, etc)
    // QMPackageFactory::RegisterAll();
    
    
    
    
    
}

bool Spectrum::Evaluate() {

    _log.setReportLevel( ctp::logDEBUG );
    _log.setMultithreading( true );
    
    _log.setPreface(ctp::logINFO,    "\n... ...");
    _log.setPreface(ctp::logERROR,   "\n... ...");
    _log.setPreface(ctp::logWARNING, "\n... ...");
    _log.setPreface(ctp::logDEBUG,   "\n... ..."); 

    CTP_LOG(ctp::logDEBUG, _log) << "Calculating absorption spectrum plot " << _orbfile << flush;

    Orbitals _orbitals;
    // load the QM data from serialized orbitals object

    std::ifstream ifs( (_orbfile).c_str());
    CTP_LOG(ctp::logDEBUG, _log) << " Loading QM data from " << _orbfile << flush;
    boost::archive::binary_iarchive ia(ifs);
    ia >> _orbitals;
    ifs.close();
    
    // output info about contents of serialized data
    // CheckContent( _orbitals );
    
    // check if orbitals contains singlet energies and transition dipoles
    if ( !_orbitals.hasBSESinglets() ){
        throw std::runtime_error("BSE singlet energies not stored in QM data file!");
    } 
    
    if ( !_orbitals.hasTransitionDipoles() ){
        throw std::runtime_error("BSE transition dipoles not stored in QM data file!");
    } 
    
    const ub::vector<real_gwbse>& BSESingletEnergies = _orbitals.BSESingletEnergies();
    const std::vector<tools::vec >& TransitionDipoles = _orbitals.TransitionDipoles();
    

    //int _n_exc = TransitionDipoles.size();
    int _n_exc = _maxexc - _minexc +1;

    if ( _maxexc > int(TransitionDipoles.size()) ) {
      CTP_LOG(ctp::logDEBUG, _log) << " Transition dipoles for some excitations missing! " << flush;
      exit(1);
    }
 
  
    CTP_LOG(ctp::logDEBUG, _log) << " Considering " << _n_exc << " excitation with max energy " << BSESingletEnergies(_maxexc) * tools::conv::hrt2ev << " eV / min wave length " <<  evtonm(BSESingletEnergies[_maxexc-1] * tools::conv::hrt2ev) << " nm" << flush;
    
    /*
     * 
     * For a single excitation, broaden by Lineshape function L(v-W)
     *    eps(v) = f * L(v-W)
     *
     * where
     *       v: energy
     *       f: oscillator strength in dipole-length gauge   
     *       W: excitation energy      
     * 
     * Lineshape function depend on FWHM and can be
     * 
     *      Gaussian
     *          L(v-W) = 1/(sqrt(2pi)sigma) * exp(-0.5 (v-W)^2/sigma^2
     * 
     * 
     *            with sigma: derived from FWHM (FWHM/2.3548)
     * 
     *     Lorentzian
     *          L(v-W) = 1/pi * 0.5 FWHM/( (v-w)^2 + 0.25*FWHM^2 )
     * 
     * Full spectrum is superposition of individual spectra.
     * 
     *  Alternatively, one can calculate the imaginary part of the 
     *  frequency-dependent dielectric function
     * 
     *   IM(eps(v)) ~ 1/v^2 * W^2 * |td|^2 * L(v-W)
     *              = 1/v^2 * W   * f      * L(v-W) 
     *
     * 
     */

    
    // get averaged oscillator strength for each excitation
    // f = 2/3 * Omega(Hrt) * |td|^2
    std::vector<double> _osc;
    for ( int _i_exc = _minexc ; _i_exc <= _maxexc; _i_exc++ ){
        _osc.push_back(  2*BSESingletEnergies(_i_exc ) * (TransitionDipoles[_i_exc]*TransitionDipoles[_i_exc]) / 3.0  );
    }
    
        
        
    std::ofstream ofs (_output_file.c_str(), std::ofstream::out);
    
    
    if ( _spectrum_type == "energy"){
        _fwhm = _fwhm / tools::conv::hrt2ev;
        ofs << "# E(eV)    epsGaussian    IM(eps)Gaussian   epsLorentz    Im(esp)Lorentz\n";
        for ( int _i_pt = 0 ; _i_pt <= _n_pt; _i_pt++ ){
    
           double _e = (_lower + _i_pt * ( _upper - _lower)/_n_pt)/tools::conv::hrt2ev;
        
           double _eps_Gaussian     = 0.0;
           double _imeps_Gaussian   = 0.0;
           double _eps_Lorentzian   = 0.0;
           double _imeps_Lorentzian = 0.0;
           double _eps_TruncLorentzian   = 0.0;
           double _imeps_TruncLorentzian = 0.0;
           
           for ( int _i_exc = _minexc ; _i_exc <= _maxexc ; _i_exc++){
              _eps_Gaussian     +=  _osc[_i_exc-_minexc] * Gaussian(_e, BSESingletEnergies(_i_exc)+_shiftby/tools::conv::hrt2ev, _fwhm);
              _imeps_Gaussian   +=  _osc[_i_exc-_minexc] *  BSESingletEnergies(_i_exc) * Gaussian(_e, BSESingletEnergies(_i_exc), _fwhm);
              _eps_Lorentzian   +=  _osc[_i_exc-_minexc] * Lorentzian(_e, BSESingletEnergies(_i_exc), _fwhm);
              _imeps_Lorentzian +=  _osc[_i_exc-_minexc] *  BSESingletEnergies(_i_exc) * Lorentzian(_e, BSESingletEnergies(_i_exc), _fwhm);
              _eps_TruncLorentzian   +=  _osc[_i_exc-_minexc] * TruncatedLorentzian(_e, BSESingletEnergies(_i_exc), _fwhm);
              _imeps_TruncLorentzian +=  _osc[_i_exc-_minexc] *  BSESingletEnergies(_i_exc) * TruncatedLorentzian(_e, BSESingletEnergies(_i_exc), _fwhm);
           }
        
           ofs << _e*tools::conv::hrt2ev << "    " << _eps_Gaussian << "   " << _imeps_Gaussian << "   " << _eps_Lorentzian << "   " << _imeps_Lorentzian << "  " << _eps_TruncLorentzian << "   " << _imeps_TruncLorentzian  << endl;
    
       }
        
        CTP_LOG(ctp::logDEBUG, _log) << " Spectrum in energy range from  " << _lower << " to " << _upper << " eV and with broadening of FWHM " << _fwhm*tools::conv::hrt2ev << " eV written to file  " << _output_file << flush;
    }
    
    if ( _spectrum_type == "wavelength"){
        
        
    
        ofs << "# lambda(nm)    epsGaussian    IM(eps)Gaussian   epsLorentz    Im(esp)Lorentz\n";
        for ( int _i_pt = 0 ; _i_pt <= _n_pt; _i_pt++ ){
        
            double _lambda =(_lower + _i_pt * ( _upper - _lower)/_n_pt) ;

            double _eps_Gaussian     = 0.0;
            double _imeps_Gaussian   = 0.0;
            double _eps_Lorentzian   = 0.0;
            double _imeps_Lorentzian = 0.0;
            double _eps_TruncLorentzian   = 0.0;
            double _imeps_TruncLorentzian = 0.0;
            
            for ( int _i_exc = _minexc ; _i_exc <= _maxexc ; _i_exc++){
              double _exc_lambda = nmtoev(BSESingletEnergies(_i_exc)*tools::conv::hrt2ev + _shiftby);
              _eps_Gaussian     +=  _osc[_i_exc-_minexc] * Gaussian(_lambda, _exc_lambda, _fwhm);
              _imeps_Gaussian   +=  _osc[_i_exc-_minexc] *  _exc_lambda * Gaussian(_lambda, _exc_lambda, _fwhm);
              _eps_Lorentzian   +=  _osc[_i_exc-_minexc] * Lorentzian(_lambda, _exc_lambda, _fwhm);
              _imeps_Lorentzian +=  _osc[_i_exc-_minexc] *  _exc_lambda * Lorentzian(_lambda, _exc_lambda, _fwhm);
              _eps_TruncLorentzian   +=  _osc[_i_exc-_minexc] * TruncatedLorentzian(_lambda, _exc_lambda, _fwhm);
              _imeps_TruncLorentzian +=  _osc[_i_exc-_minexc] *  BSESingletEnergies(_i_exc) * TruncatedLorentzian(_lambda, _exc_lambda, _fwhm);
            }

            ofs << _lambda << "    " << _eps_Gaussian << "   " << _imeps_Gaussian << "   " << _eps_Lorentzian << "   " << _imeps_Lorentzian << "   " << _eps_TruncLorentzian << "   " << _imeps_TruncLorentzian << endl;
        }    
    CTP_LOG(ctp::logDEBUG, _log) << " Spectrum in wavelength range from  " << _lower << " to " << _upper << " nm and with broadening of FWHM " << _fwhm << " nm written to file  " << _output_file << flush;        
    }


    ofs.close();
    


    
    return true;
}


double Spectrum::TruncatedLorentzian(double _x, double _center, double _fwhm){
    
    double _result;
    double _abs_diff = std::abs(_x-_center) ; 
    if ( _abs_diff > 0.5*_fwhm &&  _abs_diff < _fwhm ){
        _result = 1.0/(0.25*_fwhm*_fwhm) - 1.0/( pow(_abs_diff - _fwhm,2) + 0.25*_fwhm*_fwhm );
    } else if ( _abs_diff < 0.5*_fwhm ) {
        _result = 1.0/( pow(_x-_center,2) + 0.25*_fwhm*_fwhm  );
    } else {
        _result = 0.0;
    }
    
    
    return 0.5*_fwhm * _result / boost::math::constants::pi<double>();
}

double Spectrum::Lorentzian(double _x, double _center, double _fwhm){
    
    
    
    
    
    return 0.5*_fwhm/( pow(_x-_center,2) + 0.25*_fwhm*_fwhm  ) / boost::math::constants::pi<double>();
}

double Spectrum::Gaussian(double _x, double _center, double _fwhm){

    // FWHM = 2*sqrt(2 ln2) sigma = 2.3548 sigma
    double _sigma = _fwhm/2.3548;
    return exp(-0.5*pow(( _x - _center )/_sigma,2))/_sigma/sqrt(2.0*boost::math::constants::pi<double>());
    
}



double Spectrum::evtonm( double eV ){
    return 1241.0/eV;
}


double Spectrum::evtoinvcm(double eV ){
    return 8065.73 * eV;
}

double Spectrum::nmtoinvcm(double nm){
    return 1241.0*8065.73/nm;
}

double Spectrum::invcmtonm( double invcm ){
    return 1.0e7/invcm;
}

double Spectrum::nmtoev( double nm ){
    return 1241.0/nm;
}

}}


#endif
