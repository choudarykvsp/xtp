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

#ifndef __VOTCA_XTP_XTPDFT_H
#define __VOTCA_XTP_XTPDFT_H


#include <votca/ctp/apolarsite.h>
#include <votca/xtp/qmpackage.h>
#include <votca/xtp/dftengine.h>

#include <string>



namespace votca {
    namespace xtp {

        /**
            \brief Wrapper for the internal XTP DFT engine


         */
        class XTPDFT : public QMPackage {
        public:

            std::string getPackageName() {
                return "xtp";
            }

            void Initialize(Property *options);

            bool WriteInputFile(std::vector< ctp::Segment* > segments, Orbitals* orbitals_guess = NULL, std::vector<ctp::PolarSeg*> PolarSegments = {});

            bool Run(Orbitals* _orbitals = NULL);

            void CleanUp();

            bool CheckLogFile();

            bool ParseLogFile(Orbitals* _orbitals);

            bool ParseOrbitalsFile(Orbitals* _orbitals);
            
            bool setMultipoleBackground( std::vector<ctp::PolarSeg*> multipoles);

        private:

            DFTENGINE _xtpdft;
            Property _xtpdft_options;

            std::string _cleanup;

            
        };


    }
}

#endif /* __VOTCA_XTP_XTPDFT_H */
