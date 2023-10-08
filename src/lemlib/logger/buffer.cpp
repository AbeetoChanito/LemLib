#include "buffer.hpp"

#include "fmt/core.h"
#include "fmt/color.h"

#include "message.hpp"

namespace lemlib {
Buffer::Buffer(std::function<void(const std::string&)> bufferFunc)
    : bufferFunc(bufferFunc), task([&]() { loggingTask(); }) {}

bool Buffer::buffersEmpty() {
    mutex.take();
    bool status = buffer.size() == 0;
    mutex.give();
    return status;
}

Buffer::~Buffer() {
    // make sure when the destructor is called so all
    // the messages are logged
    while (!buffersEmpty()) { pros::delay(10); }
}

void Buffer::pushToBuffer(const std::string& bufferData) {
    mutex.take();
    buffer.push_back(bufferData);
    mutex.give();
}

void Buffer::setRate(uint32_t rate) { this->rate = rate; }

void Buffer::loggingTask() {
    while (true) {
        mutex.take();
        if (buffer.size() > 0) {
            bufferFunc(buffer.at(0));
            buffer.pop_front();
        }
        mutex.give();
        pros::delay(rate);
    }
}
} // namespace lemlib