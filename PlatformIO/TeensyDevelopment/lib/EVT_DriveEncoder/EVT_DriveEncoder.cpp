#include "EVT_DriveEncoder.hpp"

#include "ControlConstants.hpp"
using namespace Constants;

namespace MotorControls {
	DriveEncoder * DriveEncoder::instance = nullptr;

	DriveEncoder::DriveEncoder() {
		count = 0;
		lastState = 0;
	}


	void DriveEncoder::setup() {
		instance = this;

		pinMode(IOConstants::driveEncoderPinA, INPUT_PULLUP);
		pinMode(IOConstants::driveEncoderPinB, INPUT_PULLUP);

		lastState = (digitalReadFast(IOConstants::driveEncoderPinA) << 1) |
							 digitalReadFast(IOConstants::driveEncoderPinB);

		attachInterrupt(digitalPinToInterrupt(IOConstants::driveEncoderPinA), updateISR, CHANGE);
		attachInterrupt(digitalPinToInterrupt(IOConstants::driveEncoderPinB), updateISR, CHANGE);
	}


	void DriveEncoder::feed() {
	}


	double DriveEncoder::getPosition() {
		return getRevolutionsFromStart();
	}


	double DriveEncoder::getRevolutionsFromStart() {
		return static_cast<double>(getCount()) / countsPerRotation;
	}


	int32_t DriveEncoder::getCount() {
		noInterrupts();
		int32_t countSnapshot = count;
		interrupts();

		if (ControlConstants::isDriveEncoderInverted) {
			countSnapshot = -countSnapshot;
		}

		return countSnapshot;
	}


	void DriveEncoder::updateISR() {
		if (instance != nullptr) {
			instance->updateFromPins();
		}
	}


	void DriveEncoder::updateFromPins() {
		const uint8_t newState = (digitalReadFast(IOConstants::driveEncoderPinA) << 1) |
														 digitalReadFast(IOConstants::driveEncoderPinB);
		const uint8_t transition = (lastState << 2) | newState;

		switch (transition) {
			case 0b0001:
			case 0b0111:
			case 0b1110:
			case 0b1000:
				count++;
				break;

			case 0b0010:
			case 0b1011:
			case 0b1101:
			case 0b0100:
				count--;
				break;

			default:
				break;
		}

		lastState = newState;
	}
}
