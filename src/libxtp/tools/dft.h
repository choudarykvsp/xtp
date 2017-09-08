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

#ifndef _VOTCA_XTP_DFT_H
#define _VOTCA_XTP_DFT_H

#include <stdio.h>

#include <votca/ctp/logger.h>
#include <votca/xtp/dftengine.h>
#include <votca/xtp/qmpackagefactory.h>
#include <votca/ctp/atom.h>
#include <votca/ctp/segment.h>
#include <votca/ctp/apolarsite.h>

namespace votca {
    namespace xtp {
        using namespace std;

        class DFT : public ctp::QMTool {
        public:

            DFT() {
            };

            ~DFT() {
            };

            string Identify() {
                return "dft";
            }

            void Initialize(Property *options);
            bool Evaluate();

            DFTENGINE _dftengine;


        private:

            string _orbfile;
            string _xyzfile;

            string _logfile;
            string _guess_file;
            bool _do_guess;
            string _package;
            Property _package_options;
            Property _dftengine_options;

            string _output_file;

            string _reporting;
            ctp::Logger _log;

            string _mpsfile;
            bool _do_external;

            void XYZ2Orbitals(Orbitals* _orbitals, string filename);


        };

        void DFT::Initialize(Property* options) {

            // update options with the VOTCASHARE defaults   
            UpdateWithDefaults(options, "xtp");

            /*    _do_dft_input = false;
            _do_dft_run = false;
            _do_dft_parse = false;
            _do_gwbse = false;
            _do_optimize = false;
            _restart_opt = false;
             */

            string key = "options." + Identify();
            _output_file = options->get(key + ".archive").as<string>();
            _reporting = options->get(key + ".reporting").as<string> ();

            // options for dft package

            //cout << endl << "... ... Parsing " << _package_xml << endl ;




            // options for dftengine
            key = "options." + Identify();



            if (options->exists(key + ".guess")) {
                _do_guess = true;
                _guess_file = options->get(key + ".guess").as<string> ();

            } else {
                _do_guess = false;
            }
            if (options->exists(key + ".dftengine")) {
                string _dftengine_xml = options->get(key + ".dftengine").as<string> ();
                load_property_from_xml(_dftengine_options, _dftengine_xml.c_str());
            } else if (options->exists(key + ".package")) {
                string _package_xml = options->get(key + ".package").as<string> ();
                key = "package";
                _package = _package_options.get(key + ".name").as<string> ();
                load_property_from_xml(_package_options, _package_xml.c_str());
                key = "options." + Identify();
                _logfile = options->get(key + ".molecule.log").as<string> ();
                _orbfile = options->get(key + ".molecule.orbitals").as<string> ();
            }


            if (options->exists(key + ".mpsfile")) {
                _do_external = true;
                _mpsfile = options->get(key + ".mpsfile").as<string> ();
            } else {
                _do_external = false;
            }


            // initial coordinates
            _xyzfile = options->get(key + ".xyz").as<string>();


            // get the path to the shared folders with xml files
            char *votca_share = getenv("VOTCASHARE");
            if (votca_share == NULL) throw std::runtime_error("VOTCASHARE not set, cannot open help files.");
            string xmlFile = string(getenv("VOTCASHARE")) + string("/xtp/packages/") + _package + string("_idft_pair.xml");
            //load_property_from_xml(_package_options, xmlFile);

            // register all QM packages (Gaussian, TURBOMOLE, etc)
            QMPackageFactory::RegisterAll();




        }

        bool DFT::Evaluate() {


            if (_reporting == "silent") _log.setReportLevel(ctp::logERROR); // only output ERRORS, GEOOPT info, and excited state info for trial geometry
            if (_reporting == "noisy") _log.setReportLevel(ctp::logDEBUG); // OUTPUT ALL THE THINGS
            if (_reporting == "default") _log.setReportLevel(ctp::logINFO); // 

            _log.setMultithreading(true);
            _log.setPreface(ctp::logINFO, "\n... ...");
            _log.setPreface(ctp::logERROR, "\n... ...");
            _log.setPreface(ctp::logWARNING, "\n... ...");
            _log.setPreface(ctp::logDEBUG, "\n... ...");

            //TLogLevel _ReportLevel = _log.getReportLevel( ); // backup report level

            // Create new orbitals object and fill with atom coordinates
            Orbitals _orbitals;

            if (_do_guess) {
                CTP_LOG(ctp::logDEBUG, _log) << "Reading guess from " << _guess_file << flush;
                _orbitals.Load(_guess_file);

            } else {
                CTP_LOG(ctp::logDEBUG, _log) << "Reading structure from " << _xyzfile << flush;
                _orbitals.LoadFromXYZ(_xyzfile);
            }



            // initialize the DFTENGINE
            DFTENGINE _dft;
            _dft.Initialize(&_dftengine_options);
            _dft.setLogger(&_log);
            ;

            if (_do_external) {
                CTP_LOG (ctp::logDEBUG, _log) << " Let's create the background "  << flush; 
                vector<ctp::APolarSite*> sites = ctp::APS_FROM_MPS(_mpsfile, 0);
                std::vector<ctp::PolarSeg*> polar_segments;
                //ctp::PolarSeg *thisPolarSegment = NULL;
                ctp::PolarSeg *newPolarSegment = new ctp::PolarSeg(0, sites);
                polar_segments.push_back(newPolarSegment);
                polar_segments[0]->WriteMPS("test.mps", "test");
                _dft.setExternalcharges(polar_segments);
            }

            // RUN
            _dft.Prepare(&_orbitals);
            _dft.Evaluate(&_orbitals);


            CTP_LOG(ctp::logDEBUG, _log) << "Saving data to " << _output_file << flush;
            std::ofstream ofs((_output_file).c_str());
            boost::archive::binary_oarchive oa(ofs);



            oa << _orbitals;
            ofs.close();

            return true;
        }

    }
}


#endif
