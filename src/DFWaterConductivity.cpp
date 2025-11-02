#include "DFWaterConductivity.h"

/// @brief Creates a soil moisture sensor
/// @param Name The device name
/// @param EC_Pin The analog pin to use for the EC probe
/// @param Temp_Pin The analog pin to use for the temp probe
/// @param ConfigFile The name of the configuration file to use
DFWaterConductivity::DFWaterConductivity(String Name, int EC_Pin, int Temp_Pin, String ConfigFile) : Sensor(Name) {
	config_path = "/settings/sen/" + ConfigFile;
	current_config.EC_Pin = EC_Pin;
	current_config.Temp_Pin = Temp_Pin;
}

/// @brief Starts a Data Template object
/// @return True on success
bool DFWaterConductivity::begin() {
	Description.parameterQuantity = 1;
	Description.type = "Water Sensor";
	Description.parameters = {"Conductivity", "Temperature"};
	Description.units = {"uS/cm", "C"};
	values.resize(Description.parameterQuantity);
	bool result = false;
	// Create settings directory if necessary
	if (!checkConfig(config_path)){
		// Set defaults
		current_config.ADC_Volatage_mv = 3300;
		current_config.ADC_Resolution = 4096;
		result = saveConfig(config_path, getConfig());
	} else {
		// Load settings
		result = setConfig(Storage::readFile(config_path), false);
	}
	return result;
}

/// @brief Takes a measurement
/// @return True on success
bool DFWaterConductivity::takeMeasurement() {
	float Temp_Voltage = getAnalogMV(current_config.Temp_Pin);
	float EC_Voltage = getAnalogMV(EC_PIN);
	for (int i = 0; i < 9; i++) {
		Temp_Voltage += getAnalogMV(current_config.Temp_Pin);
		EC_Voltage += getAnalogMV(EC_PIN);
		delay(5);
	}
	Temp_Voltage /= 10;
	EC_Voltage /= 10;
	float temp = ec_temp.convVoltagetoTemperature_C((float)Temp_Voltage/1000);
	values[0] = ec.getEC_us_cm(EC_Voltage, temp);
	values[1] = temp;
	return true;
}

/// @brief Gets the current config
/// @return A JSON string of the config
String DFWaterConductivity::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["Name"] = Description.name;
	doc["EC_Pin"] = current_config.EC_Pin;
	doc["Temp_Pin"] = current_config.Temp_Pin;
	doc["ADC_Volatage_mv"] = current_config.ADC_Volatage_mv;
	doc["ADC_Resolution"] = current_config.ADC_Resolution;

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
	Description.name = doc["Name"].as<String>();
	current_config.EC_Pin = doc["EC_Pin"].as<int>();
	current_config.Temp_Pin = doc["Temp_Pin"].as<int>();
	current_config.ADC_Volatage_mv = doc["ADC_Volatage_mv"].as<float>();
	current_config.ADC_Resolution = doc["ADC_Resolution"].as<int>();
	pinMode(current_config.EC_Pin, INPUT);
	pinMode(current_config.Temp_Pin, INPUT);
	if (save) {
		return saveConfig(config_path, config);
	}
	return true;
}

/// @brief Converts an analog reading to mV
/// @param pin The pin number to read
/// @return The reading in millivolts
uint32_t DFWaterConductivity::getAnalogMV(int pin) {
	return analogRead(pin) * current_config.ADC_Volatage_mv / current_config.ADC_Resolution;
}