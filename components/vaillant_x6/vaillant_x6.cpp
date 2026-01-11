#include "vaillant_x6.h"
#include "esphome/core/application.h"
#include "response_decoder.h"

namespace esphome {
namespace vaillant_x6 {

void VaillantX6Component::setup() {
    request_response_handler = new RequestResponseHandler(
        std::bind(&uart::UARTDevice::read, this),
        std::bind(&uart::UARTDevice::write, this, std::placeholders::_1),
        std::bind(&uart::UARTDevice::available, this),
        std::bind(&VaillantX6Component::is_response_complete_, this),
        std::bind(&VaillantX6Component::is_response_valid, this)
    );
}

void VaillantX6Component::add_binary_sensor(
    binary_sensor::BinarySensor* sensor,
    std::string response_type,
    std::vector<uint8_t> request_bytes,
    int poll_interval) {

    GetOnOffStatusCommand* cmd;
    if (response_type == "Status01") {
        cmd = new GetOnOffStatusCommand();
    } else if (response_type == "Status0f") {
        cmd = new GetOnOffStatusCommand();
        cmd->on_value = 0x0f;
    } else {
        ESP_LOGE(TAG, "Unknown response_type: %s", response_type.c_str());
        return;
    }
    
    cmd->name = "Get " + sensor->get_name();
    cmd->request_bytes = std::move(request_bytes);
    cmd->sensor = sensor;
    
    // PollingComponent is configured to be invoked every 10s (see __init__.py)
    cmd->interval = poll_interval / 10;
    commands.push_back(cmd);
}

void VaillantX6Component::add_sensor(
    sensor::Sensor* sensor,
    std::string response_type,
    std::vector<uint8_t> request_bytes,
    int poll_interval) {

    VaillantX6Command* cmd = nullptr;
    
    if (response_type == "AnalogueValue2Bytes") {
        auto* c = new GetAnalogueValue2BytesCommand();
        c->sensor = sensor;
        cmd = c;
    } else if (response_type == "AnalogueValue1Byte") {
        auto* c = new GetAnalogueValue1ByteCommand();
        c->sensor = sensor;
        cmd = c;
    } else {
        ESP_LOGE(TAG, "Unknown response_type: %s", response_type.c_str());
        return;
    }
    
    cmd->name = "Get " + sensor->get_name();
    cmd->request_bytes = std::move(request_bytes);
    
    // PollingComponent is configured to be invoked every 10s (see __init__.py)
    cmd->interval = poll_interval / 10;
    commands.push_back(cmd);
}

void VaillantX6Component::update() {
    current_command_idx = -1;
    seek_to_next_command();
    update_counter += 1;
}

void VaillantX6Component::loop() {
    auto response_available = request_response_handler->loop();

    if (response_available) {
        auto command = commands[current_command_idx];

        if (request_response_handler->bytes_read_count != command->get_expected_response_length()) {
            ESP_LOGW(TAG, "Unexpected response length. Was %d, required %d",
                    request_response_handler->bytes_read_count, command->get_expected_response_length());
            return;
        }

        command->process_response(request_response_handler->response_buffer);

        seek_to_next_command();
    }
}

void VaillantX6Component::seek_to_next_command() {
    current_command_idx++;
    while (current_command_idx < commands.size()) {
        auto& command = commands[current_command_idx];
        if ((update_counter % command->get_interval()) == 0) {
            request_response_handler->set_next_command(command);
            break;
        }
        current_command_idx++;
    }
}

bool VaillantX6Component::is_response_complete_() {
    uint8_t response_length = request_response_handler->response_buffer[0];

    return request_response_handler->bytes_read_count >= response_length;
}

bool VaillantX6Component::is_response_valid() {
    if (request_response_handler->response_buffer[1] != 0x00) {
        ESP_LOGW(TAG, "Second byte in response is not 0x00");
        return false;
    }

    uint8_t checksum = request_response_handler->response_buffer[request_response_handler->bytes_read_count - 1];
    if (calc_checksum_of_response() != checksum) {
        ESP_LOGW(TAG, "Checksum mismatch");
        return false;
    }

    return true;
}

uint8_t VaillantX6Component::calc_checksum_of_response() {
    uint8_t checksum = 0;

    for (int i = 0; i < request_response_handler->bytes_read_count - 1; i++) {
        uint8_t byte = request_response_handler->response_buffer[i];
    
        if (checksum & 0x80) {
            che
