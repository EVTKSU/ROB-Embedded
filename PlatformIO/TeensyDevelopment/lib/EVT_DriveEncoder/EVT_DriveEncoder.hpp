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

#include <IOConstants.hpp>

namespace MotorControls {
	class DriveEncoder {
		private: 
			static DriveEncoder * instance;

			volatile int32_t count;
			volatile uint8_t lastState;

			static constexpr double countsPerRotation =
				static_cast<double>(Constants::IOConstants::driveEncoderPulsesPerRev) * 4.0;

			static void updateISR();
			void updateFromPins();
		public:
			/**
			 * @brief Construct a new Drive Encoder object
			 * 
			 */
			DriveEncoder();

			/**
			 * @brief Initializes the encoder hardware.
			 */
			void setup();

			/**
			 * @brief Kept for compatibility. Position is updated by pin-change interrupts.
			 * 
			 */
			void feed();


			/**
			 * @brief Returns the position of the drive encoder in rotations.
			 *
			 * @return The position of the drive encoder in rotations.
			 */
			double getPosition();

			/**
			 * @brief Returns accumulated drive encoder revolutions since firmware startup.
			 *
			 * @return The accumulated drive encoder revolutions since startup.
			 */
			double getRevolutionsFromStart();

			/**
			 * @brief Returns the raw accumulated quadrature count since firmware startup.
			 *
			 * @return The raw accumulated quadrature count since startup.
			 */
			int32_t getCount();
	};
}

#endif // DRIVE_ENCODER
