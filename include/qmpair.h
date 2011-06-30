/* 
 * File:   qmbeadpair.h
 * Author: james
 *
 * Created on 14 December 2009, 08:47
 */

#ifndef _QMPAIR_H
#define	_QMPAIR_H

#include "qmcrgunit.h"
#include <utility>

class QMTopology;

class QMPair :
    public std::pair<QMCrgUnit *, QMCrgUnit *>
{
public:
    QMPair(): _r(0.,0.,0.), _rate_12(0.),_rate_21(0.),_lambda_outer(0.),_ghost(NULL),_crg2(NULL) {}

    QMPair(QMCrgUnit *crg1, QMCrgUnit *crg2, QMTopology * top);

    ~QMPair(){
        if(_ghost != NULL)
            delete _ghost;
    }
    /**
     * \brief the vector connecting two beads
     * @return pbc correct distance vector
     */
    vec &r() { return _r; }

    /**
     * \brief the distance of the beads
     * @return pbc correct distance
     */
    double dist() { return abs(_r); }

    /**
     * \brief vector of transfer integrals (vector because of degenerate orbitals)
     * @return vector of transfer integrals
     */
    vector <double> &Js() { return _Js; }

    /**
     * \brief transfer rate from first to second
     * @return transfer rate from first to second
     */

    double &rate12(){return _rate_12;}

    /**
     * \brief transfer rate from second to first
     * @return transfer rate from second to first
     */     
    double &rate21(){return _rate_21;}

    /**
     * \brief set the transfer integral
     * @param js vector with transfer integrals
     */
    void setJs(const vector <double> &js){_Js=js;}

    /** \brief calculate the effective transfer integral as the rms of the transfer integrals
     * @return effective tranfer integral squared
     */
    double calcJeff2();

    /**
     *  \brief set transfer rate from first to second
     * @param rate  forward rate
     */
    void setRate12(double rate) {_rate_12=rate;}
    
    /**
     * \brief set transfer rate from second to first
     * @param r backward rate
     */
    void setRate21(double rate) {_rate_21=rate;}

     /**
     * \brief set outer sphere lambda
     */
    void setLambdaOuter(double lambda) {_lambda_outer=lambda;}

    /**
     * \brief outer sphere lambda
     * @return outer sphere lambda
     */
    const double &getLambdaOuter() const {return _lambda_outer;}
    //double &getLambdaOuter() {return _lambda_outer;}

    /**
     * \brief first crg unit (might be ghost copy for pbc image)
     *
     * first and second are original crg units, crg1 and crg2 take into account
     * pbc and might be ghost copies
     * @return crg unit 1
     */
    QMCrgUnit *Crg1() {return first;}

    /**
     * \brief second crg unit (might be ghost copy for pbc image)
     *
     * first and second are original crg units, crg1 and crg2 take into account
     * pbc and might be ghost copies
     * @return crg unit 2
     */
    QMCrgUnit *Crg2() {return _crg2;}

protected:
    /// vector connecting the two beads
    vec _r;
    /// transfer integrals, multiple entries in case of degeneracy
    vector <double> _Js;
    /// transfer rate from first to second
    double _rate_12;
    /// transfer rate from second to first
    double _rate_21;
    /// outer sphere reorganization energy
    double _lambda_outer;
    /// ghost atom in case the molecules are neighbors across a boundary
    QMCrgUnit * _ghost;

    QMCrgUnit *_crg2;
};

#endif	/* _QMBEADPAIR_H */

