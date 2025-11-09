#include "DFWaterConductivity.h"

/// @brief Creates a water conductivity sensor
/// @param Name The device name
/// @param EC_Pin The analog pin to use for the EC probe
/// @param Temp_Pin The analog pin to use for the temp probe
/// @param ConfigFile The name of the configuration file to use
DFWaterConductivity::DFWaterConductivity(String Name, int EC_Pin, int Temp_Pin, String ConfigFile) : GenericAnalogInput(Name, EC_Pin, ConfigFile) {
	config_path = "/settings/sen/" + ConfigFile;
	current_config.EC_Pin = EC_Pin;
	current_config.Temp_Pin = Temp_Pin;
}

/// @brief Starts a Data Template object
/// @return True on success
bool DFWaterConductivity::begin() {
	bool result = false;
	if (GenericAnalogInput::begin()) {
		Description.parameterQuantity = 1;
		Description.type = "Water Sensor";
		Description.parameters = {"Conductivity", "Temperature"};
		Description.units = {"uS/cm", "C"};
		values.resize(Description.parameterQuantity);
		// Create settings directory if necessary
		if (!checkConfig(config_path)){
			// Set defaults
			result = saveConfig(config_path, getConfig());
		} else {
			// Load settings
			result = setConfig(Storage::readFile(config_path), false);
		}
	}
	return result;
}

/// @brief Takes a measurement
/// @return True on success
bool DFWaterConductivity::takeMeasurement() {
	analog_config.Pin = current_config.Temp_Pin;
	float Temp_Voltage = getMVValue();

	analog_config.Pin = current_config.EC_Pin;
	float EC_Voltage = getMVValue();

	float temp = ec_temp.convVoltagetoTemperature_C((float)Temp_Voltage/1000);
	values[0] = ec.getEC_us_cm(EC_Voltage, temp);
	values[1] = temp;
	return true;
}

/// @brief Used to calibrate sensor
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> DFWaterConductivity::calibrate(int step) {
	Logger.println("Calibrating EC sensor, step " + String(step));
	std::tuple<Sensor::calibration_response, String> response;
	int new_value;
	// Disable averaging for calibration
	bool average = analog_config.RollingAverage;
	analog_config.RollingAverage = false;
	switch (step) {
		case 0:
			response = { Sensor::calibration_response::NEXT, "Place sensor in calibration solution, then click next" };
			break;
		case 1:
			uint16_t new_value = getMVValue();
			for (int i = 0; i < 9; i++) {
				new_value += getMVValue();
				delay(50);
			}
			new_value /= 10;
			current_config.K_Value = ec.calibrate(new_value, current_config.Calibration_Value);
			// Apply calibration value
			ec.~DFRobot_ECPRO();
			ec = DFRobot_ECPRO(current_config.K_Value);
			response = { Sensor::calibration_response::DONE, "Calibration complete, new K-value is: " + String(current_config.K_Value) };
			break;
		default:
			response = { Sensor::calibration_response::ERROR, "No such calibration step: " + String(step) };
			break;
	}
	// Re-enable averaging if needed
	analog_config.RollingAverage = average;
	return response;
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFWaterConductivity::getConfig() {
	// Allocate the JSON document
	JsonDocument doc = addAdditionalConfig();

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config A JSON string of the configuration settings
/// @param save If the configuration should be saved to a file
/// @return True on success
bool DFWaterConductivity::setConfig(String config, bool save) {
	if (GenericAnalogInput::setConfig(config, false)) {
		// Allocate the JSON document
		JsonDocument doc;
		// Deserialize file contents
		DeserializationError error = deserializeJson(doc, config);
		// Test if parsing succeeds.
		if (error) {
			Serial.print(F("Deserialization failed: "));
			Serial.println(error.f_str());
			return false;
		}
		// Assign loaded values
		current_config.EC_Pin = doc["Pin"].as<int>();
		current_config.Temp_Pin = doc["Temp_Pin"].as<int>();
		current_config.K_Value = doc["K_Value"].as<float>();
		current_config.Calibration_Value = doc["Calibration_Value"].as<float>();

		pinMode(current_config.Temp_Pin, INPUT);

		// Apply calibration value
		ec.~DFRobot_ECPRO();
		ec = DFRobot_ECPRO(current_config.K_Value);

		if (save) {
			return saveConfig(config_path, config);
		}
		return true;
	}
	return false;
}

/// @brief Collects all the base class parameters and additional parameters
/// @return a JSON document with all the parameters
JsonDocument DFWaterConductivity::addAdditionalConfig() {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, GenericAnalogInput::getConfig());
	// Test if parsing succeeds.
	if (error) {
		Logger.print(F("Deserialization failed: "));
		Logger.println(error.f_str());
		return doc;
	}
	doc["Temp_Pin"] = current_config.Temp_Pin;
	doc["K_Value"] = current_config.K_Value;
	doc["Calibration_Value"] = current_config.Calibration_Value;
	return doc;
}