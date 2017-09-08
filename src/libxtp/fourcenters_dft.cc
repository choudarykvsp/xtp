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
 *Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#include <votca/xtp/threecenters.h>
#include <new>


using namespace votca::tools;

namespace votca {
    namespace xtp {
        namespace ub = boost::numeric::ublas;

        /*
         * Cleaning FCMatrix_dft data and free memory
         */
       








        void FCMatrix_dft::Fill_4c_small_molecule(const AOBasis& dftbasis) {

          //cout << endl;
          //cout << "fourcenters_dft.cc FCMatrix_dft::Fill_4c_small_molecule" << endl;
          int dftBasisSize = dftbasis.AOBasisSize();
          int vectorSize = (dftBasisSize*(dftBasisSize+1))/2;
          
          try{
          _4c_vector = ub::zero_vector<double>((vectorSize*(vectorSize+1))/2);
          }
          catch(std::bad_alloc& ba){
            std::cerr << "Basisset too large for 4c calculation. Not enough RAM. Caught bad alloc: " << ba.what() << endl;
            exit(0);
          }
          int shellsize=dftbasis.getNumofShells();
          #pragma omp parallel for
          for(int i=0;i<shellsize;++i){
          
            const AOShell* _shell_3 = dftbasis.getShell(i);
            int _start_3 = _shell_3->getStartIndex();
            int NumFunc_3 = _shell_3->getNumFunc();
   

            for (int j=i;j<shellsize;++j) {
              const AOShell* _shell_4 = dftbasis.getShell(j);
              int _start_4 = _shell_4->getStartIndex();
              int NumFunc_4 = _shell_4->getNumFunc();                
       
              for (int k=i;k<shellsize;++k) {
                const AOShell* _shell_1 = dftbasis.getShell(k);
                int _start_1 = _shell_1->getStartIndex();
                int NumFunc_1 = _shell_1->getNumFunc();
         
               for (int l=k;l<shellsize;++l)  {
                  const AOShell* _shell_2 = dftbasis.getShell(l);
                  int _start_2 = _shell_2->getStartIndex();
                  int NumFunc_2 = _shell_2->getNumFunc();
                  // get 4-center directly as _subvector
                  ub::matrix<double> _subvector = ub::zero_matrix<double>(NumFunc_1 * NumFunc_2, NumFunc_3 * NumFunc_4);
                  
                  bool nonzero=FillFourCenterRepBlock(_subvector, _shell_1, _shell_2, _shell_3, _shell_4);

                  if (nonzero) {

                    for (int _i_3 = 0; _i_3 < NumFunc_3; _i_3++) {
                      int ind_3 = _start_3 + _i_3;
                      int sum_ind_3 = (ind_3*(ind_3+1))/2;
                      for (int _i_4 = 0; _i_4 < NumFunc_4; _i_4++) {
                        int ind_4 = _start_4 + _i_4;
                        if (ind_3 > ind_4) continue;
                        int _index_subv_34 = NumFunc_3 * _i_4 + _i_3;
                        int _index_34 = dftBasisSize * ind_3 - sum_ind_3 + ind_4;
                        int _index_34_12_a = vectorSize * _index_34 - (_index_34*(_index_34+1))/2;
                        for (int _i_1 = 0; _i_1 < NumFunc_1; _i_1++) {
                          int ind_1 = _start_1 + _i_1;
                          int sum_ind_1 = (ind_1*(ind_1+1))/2;
                          for (int _i_2 = 0; _i_2 < NumFunc_2; _i_2++) {
                            int ind_2 = _start_2 + _i_2;
                            if (ind_1 > ind_2) continue;
                            int _index_12 = dftBasisSize * ind_1 - sum_ind_1 + ind_2;
                            if (_index_34 > _index_12) continue;
                            _4c_vector(_index_34_12_a + _index_12) = _subvector(NumFunc_1 * _i_2 + _i_1, _index_subv_34);

                          } // _i_2
                        } // _i_1
                      } // _i_4
                    } // _i_3
                    
                  } // end if
                } // DFT shell_2
              } // DFT shell_1
            } // DFT shell_4
          } // DFT shell_3

          return;
        } // FCMatrix_dft::Fill_4c_small_molecule
        
       







 


    }
}
