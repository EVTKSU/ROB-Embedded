#ifndef DRIVE_ENCODER
#define DRIVE_ENCODER

/*-----------------------------------------------------------------------------*/
/** 
 * @file   ModuleConstants.hpp
 * @brief  Header for ModuleConstants struct 
 * 
 * The ModuleConstants struct is used for the definition of various constants 
 * that are used throughout the entire codebase for module drivers
 * 
 * @author Austin Sheppard
 * @date   March 18, 2026
*//*---------------------------------------------------------------------------*/

#include <AS5X47.h>

#include <IOConstants.hpp>

namespace MotorControls {
	// TODO: Implement velocity filter. IDK how reliable to real time thing is rn.
	class DriveEncoder {
		private: 
			AS5X47 encoder {Constants::IOConstants::driveEncoderCS};

			double position;  // Current position of the drive motor in degrees 
			double lastAngle; // Previous position of the drive motor 
		public:
			/**
			 * @brief Construct a new Drive Encoder object
			 * 
			 */
			DriveEncoder();


			/**
			 * @brief Updates the internal position of the motor
			 * 
			 * @note This method must be called at least once per half rotation of the encoder
			 */
			void feed();


			/**
			 * @brief Returns the position of the drive encoder in rotations.
			 *
			 * @return The position of the drive encoder in rotations.
			 */
			double getPosition();
	};
}

#endif // DRIVE_ENCODER