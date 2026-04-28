#include "EVT_DriveEncoder.hpp"

#include "ControlConstants.hpp"
#include "ConversionConstants.hpp"
using namespace Constants;

#include <cmath>

namespace MotorControls {
	DriveEncoder::DriveEncoder() {
		Settings1 settings1 {};

		// Set the inversion of the encoder.
		settings1.values.dir = ControlConstants::isDriveEncoderInverted;

		// Make sure it is abi mode.
		settings1.values.uvw_abi = 0;
		
		// Use the binary abi mode.
		settings1.values.abibin = 1;

		encoder.writeSettings1(settings1);
		
		Settings2 settings2 {};

		// Use the highest resolution
		settings2.values.abires = 0;

		encoder.writeSettings2(settings2);

		lastAngle = encoder.readAngle() * ConversionConstants::degToTurns;
		position = lastAngle;
	}


	void DriveEncoder::feed() {
		double delta = (encoder.readAngle() * ConversionConstants::degToTurns) - lastAngle;

		if (std::abs(delta) > 0.5) {
			position += std::copysign(1, -delta);
		}

		position += delta;
		lastAngle += delta;
	}

	
	double DriveEncoder::getPosition() {
		return position;
	}
}