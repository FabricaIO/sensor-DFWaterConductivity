/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * External libraries needed:
 * ArduinoJSON: https://arduinojson.org/
 * DFRobot_EC_Pro: https://github.com/cdjq/DFRobot_ECPRO
 *
 * EC Sensor: https://www.dfrobot.com/product-2565.html
 *
 * Contributors: Sam Groveman
 */

#pragma once
#include <Sensor.h>
#include <Storage.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <DFRobot_ECPRO.h>

class DFWaterConductivity : public Sensor {
	protected:
		DFWaterConductivity(int EC_Pin = 36, int TEMP_Pin = 39, String ConfigFile = "DFWaterConductivity.json");
		bool begin();
		bool takeMeasurement();
		String getConfig();
		bool setConfig(String config, bool save);
		
	private:
		/// @brief Soil moisture sensor configuration
		struct {
			/// @brief The analog pin to use for the EC probe
			int EC_Pin;

			/// @brief The analog pin to use for the temperature probe
			int Temp_Pin;

			/// @brief The voltage used by the ADC in mv
			int ADC_Volatage_mv;

			/// @brief The resolution of the ADC
			int ADC_Resolution;
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

		uint32_t getAnalogMV(int pin);
};