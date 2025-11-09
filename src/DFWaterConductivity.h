/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ArduinoJSON: https://arduinojson.org/
 * DFRobot_EC_Pro: https://github.com/cdjq/DFRobot_ECPRO
 * EC Sensor: https://www.dfrobot.com/product-2565.html
 *
 * Contributors: Sam Groveman
 */

#pragma once
#include <GenericAnalogInput.h>
#include <Storage.h>
#include <ArduinoJson.h>
#include <DFRobot_ECPRO.h>

class DFWaterConductivity : public GenericAnalogInput {
	public:
		DFWaterConductivity(String Name, int EC_Pin = 36, int TEMP_Pin = 39, String ConfigFile = "DFWaterConductivity.json");
		bool begin();
		bool takeMeasurement();
		String getConfig();
		bool setConfig(String config, bool save);
		std::tuple<Sensor::calibration_response, String> calibrate(int step);
		
	protected:
		/// @brief Soil moisture sensor configuration
		struct {
			/// @brief The analog pin to use for the EC probe
			int EC_Pin;

			/// @brief The analog pin to use for the temperature probe
			int Temp_Pin;

			/// @brief The K-value of the sensor
			float K_Value = 1.0;

			/// @brief The value of the calibration solution used
			float Calibration_Value = 1413.0;
		} current_config;

		/// @brief Path to settings file
		String config_path;

		/// @brief Water conductivity probe
		DFRobot_ECPRO ec;
		
		/// @brief Water temperature probe
		DFRobot_ECPRO_PT1000 ec_temp;

		// EC probe pin
		int EC_PIN;

		// Temperature sensor pin
		int TEMP_PIN;

		// EC EEPROM address for calibration
		const int EC_ADDR = 0x20;

		JsonDocument addAdditionalConfig();
};
