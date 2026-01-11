#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include <vector>

namespace esphome {
namespace vaillant_x6 {

static const char *const TAG = "vaillant_x6";

class VaillantX6Command {
  public:
    std::string name;
    std::vector<uint8_t> request_bytes;
    int interval{1};
    
    virtual void process_response(uint8_t* response) = 0;
    virtual int get_expected_response_length();
    int get_interval() { return interval; }
};

// -------------------------------------------- 

class GetAnalogueValue2BytesCommand : public VaillantX6Command {
  public:
    sensor::Sensor* sensor;
    void process_response(uint8_t* response) override;
};

// -------------------------------------------- 
// NEUE KLASSE HINZUFÜGEN:

class GetAnalogueValue1ByteCommand : public VaillantX6Command {
  public:
    sensor::Sensor* sensor;
    void process_response(uint8_t* response) override;
};

// -------------------------------------------- 

class GetOnOffStatusCommand : public VaillantX6Command {
  public:
    binary_sensor::BinarySensor* sensor;
    uint8_t on_value{0x01};
    void process_response(uint8_t* response) override;
};

} // namespace vaillant_x6
} // namespace esphome
