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

#ifndef __VOTCA_XTP_NWCHEM_H
#define	__VOTCA_XTP_NWCHEM_H


#include <votca/ctp/apolarsite.h>
#include <votca/xtp/qmpackage.h>

#include <string>



namespace votca { namespace xtp {
/**
    \brief Wrapper for the Gaussian program

    The Gaussian class executes the Gaussian package
    and extracts information from its log and io files

*/
class NWChem : public QMPackage
{
public:

   std::string getPackageName() { return "nwchem"; }

   void Initialize( Property *options );

   /* Writes Gaussian input file with coordinates of segments
    * and a guess for the dimer (if requested) constructed from the
    * monomer orbitals
    */
   bool WriteInputFile( std::vector< ctp::Segment* > segments, Orbitals* orbitals_guess = NULL, std::vector<ctp::PolarSeg*> PolarSegments = {});

   bool WriteShellScript();

   bool Run( Orbitals* _orbitals = NULL );

   void CleanUp();

   bool CheckLogFile();

   bool ParseLogFile( Orbitals* _orbitals );

   bool ParseOrbitalsFile( Orbitals* _orbitals );

   bool setMultipoleBackground( std::vector<ctp::PolarSeg*> multipoles){ return true; };


   std::string getScratchDir( ) { return _scratch_dir; }

private:

    std::string                              _shell_file_name;
    std::string                              _chk_file_name;
    std::string                              _scratch_dir;
    bool                                _is_optimization;

    std::string                              _cleanup;

    int NumberOfElectrons( std::string _line );
    int BasisSetSize( std::string _line );
    int EnergiesFromLog( std::string _line, ifstream inputfile );
    std::string FortranFormat( const double &number );
    void WriteBackgroundCharges(ofstream& _com_file,std::vector<ctp::PolarSeg*> PolarSegments);

};


}}

#endif	/* __VOTCA_XTP_NWCHEM_H */
