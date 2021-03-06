/**
 * test_landmark.cpp
 *
 * \date 12/03/2010
 * \author jsola@laas.fr
 *
 *  \file test_landmark.cpp
 *
 *  ## Add a description here ##
 *
 * \ingroup rtslam
 */

// boost unit test includes
#include <boost/test/auto_unit_test.hpp>

// jafar debug include
#include "kernel/jafarDebug.hpp"

#include <iostream>
#include "jmath/matlab.hpp"
#include "jmath/random.hpp"

#include "jmath/jblas.hpp"
#include "jmath/indirectArray.hpp"

#include "rtslam/quatTools.hpp"

#include "rtslam/landmarkAnchoredHomogeneousPoint.hpp"
#include "kernel/IdFactory.hpp"

using namespace jafar::rtslam;
using namespace jblas;
using namespace jafar::jmath;
using namespace jafar::jmath::ublasExtra;
using namespace std;

void test_landmark01(void) {

	IdFactory IdFac;

	size_t size_map = 300;

	map_ptr_t mapPtr(new MapAbstract(size_map));
	randVector(mapPtr->x());
	randMatrix(mapPtr->P());

	ublas::vector<LandmarkAbstract *> Lmks(5);

	cout << "\n% LANDMARK CREATION AND PRINT \n%===========" << endl;

	for (size_t lmk = 0; lmk < 3; lmk++) {
		jblas::ind_array ial = ia_pushfront(mapPtr->used_states, LandmarkAnchoredHomogeneousPoint::size());
		Lmks.insert_element(lmk, new LandmarkAnchoredHomogeneousPoint(mapPtr));
		Lmks(lmk)->id(lmk);
	}
	for (size_t lmk = 0; lmk < 3; lmk++) {
		cout << *Lmks(lmk) << endl;
	}

	cout << "\n% LMK TRANSFORMATIONS \n%========================" << endl;
	jblas::vec7 F;
	jblas::vec3 T;
	jblas::vec3 E;
	for (size_t i = 0; i < 3; i++) {
		E(i) = 0.1 * (i + 1);
		T(i) = i + 1;
	}
	LandmarkAnchoredHomogeneousPoint ahp1(mapPtr);
	subrange(F, 0, 3) = T;
	subrange(F, 3, 7) = quaternion::e2q(E);
	cout << "F = " << F << endl;
	cout << "ahpf= " << ahp1.state.x() << endl;

	cout << "\n% FROM FRAME \n%==================" << endl;
	jblas::vec ahp(7), ahpf(7);
	jblas::mat AHP_f(7, 7), AHP_ahpf(7, 7);
	ahpf = ahp1.state.x();
	ahp = lmkAHP::fromFrame(F, ahpf);
	cout << "ahp_namespace : " << (MATLAB) ahp << endl;
	ahp = lmkAHP::fromFrame(F,ahpf);
	cout << "ahp_class : " << (MATLAB) ahp << endl;
	lmkAHP::fromFrame(F, ahpf, ahp, AHP_f, AHP_ahpf);

	cout << "F.x = " << MATLAB(F) << endl;
	cout << "F = updateFrame(F);" << endl;
	cout << "ahpf = " << (MATLAB) (vec) ahp1.state.x() << endl;
	cout << "ahp = " << (MATLAB) ahp << endl;
	cout << "AHP_f = " << (MATLAB) AHP_f << endl;
	cout << "AHP_ahpf = " << (MATLAB) AHP_ahpf << endl;
	cout << "[ahp_m,AHP_f_m,AHP_ahpf_m] = fromFrameAhm(F,ahpf);" << endl;
	cout << "ahp_err = norm(ahp-ahp_m)" << endl;
	cout << "AHP_f_err = norm(AHP_f-AHP_f_m)" << endl;
	cout << "AHP_ahpf_err = norm(AHP_ahpf-AHP_ahpf_m)" << endl;

	cout << "\n% TO FRAME \n%==================" << endl;
//	ahp1.state.x(ahp);
	jblas::mat AHPF_f(7, 7), AHPF_ahp(7, 7);
	lmkAHP::toFrame(F, ahp, ahpf, AHPF_f, AHPF_ahp);
	//	ahpf = ahp1.toFrame(F, ahp);
	cout << "ahp = " << (MATLAB) ahp1.state.x() << endl;
	cout << "ahpf = " << (MATLAB) ahpf << endl;
	cout << "AHPF_f = " << (MATLAB) AHPF_f << endl;
	cout << "AHPF_ahp = " << (MATLAB) AHPF_ahp << endl;
	cout << "[ahpf_m,AHPF_f_m,AHPF_ahp_m] = toFrameAhm(F,ahp);" << endl;
	cout << "ahpf_err = norm(ahpf-ahpf_m)" << endl;
	cout << "AHPF_f_err = norm(AHPF_f-AHPF_f_m)" << endl;
	cout << "AHPF_ahp_err = norm(AHPF_ahp-AHPF_ahp_m)" << endl;

	cout << "\n% REPARAMETRIZATION \n%==================" << endl;
	cout << "ahp = " << (MATLAB) ahp1.state.x() << endl;
	jblas::vec3 euc;
	jblas::mat EUC_ahp(3, 7);
	//	euc = LandmarkAnchoredHomogeneousPoint::ahp2euc(ahp);
	lmkAHP::ahp2euc(ahp1.state.x(),euc,EUC_ahp);
	cout << "euc = " << (MATLAB) euc << endl;
	cout << "EUC_ahp = " << (MATLAB) EUC_ahp << endl;
	cout << "[euc_m, EUC_ahp_m] = ahm2euc(ahp);" << endl;
	cout << "euc_err = norm(euc - euc_m)" << endl;
	cout << "EUC_ahp_err = norm(EUC_ahp-EUC_ahp_m)" << endl;

}

BOOST_AUTO_TEST_CASE( test_landmark )
{
	test_landmark01();
}

