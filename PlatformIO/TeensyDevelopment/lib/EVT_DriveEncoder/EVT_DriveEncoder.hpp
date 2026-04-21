#pragma once

#include <AS5X47.h>

#include <IOConstants.hpp>

// TODO: Implement velocity filter. IDK how reliable to real time thing is rn.
class DriveEncoder {

	AS5X47 encoder = AS5X47(Constants::IOConstants::driveEncoderCS);
	double position;
	double lastAngle;
public:
	DriveEncoder();

	/*
	 * Updates the internal position of the motor. Must be called at least once
	 * per half rotation of the encoder. Will also be required for velocity
	 * calculation at which point it will need to be called a the frequency of
	 * position samples in the filter.
	 */
	void feed();

	/**
	 * Returns the position of the drive encoder in rotations.
	 *
	 * @return The position of the drive encoder in rotations.
	 */
	double getPosition();
};
