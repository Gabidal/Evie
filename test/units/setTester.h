#ifndef _SET_TESTER_H_
#define _SET_TESTER_H_

#include "utils.h"
#include "../../src/utils/utils.h"

#include <vector>
#include <stdexcept>
#include <cstddef>

namespace tester {

	class setTester : public utils::TestSuite {
	public:
		setTester() : utils::TestSuite("set Tester") {
			
		}

	private:
		
	};
}

#endif