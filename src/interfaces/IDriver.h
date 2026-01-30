#pragma once
/**
 * IDriver.h
 * 
 * Interfaces for input/output drivers.
 * Abstracts the Interception library for testability.
 */

#include <optional>
#include <cstdint>
#include "../core/KeyEvent.h"

namespace capsicain {

/**
 * Raw key stroke from hardware
 */
struct RawKeyStroke {
    uint16_t scancode = 0;
    uint16_t flags = 0;     // Direction, extended key flags, etc.
    
    bool isKeyUp() const { return (flags & 0x01) != 0; }
    bool isExtended() const { return (flags & 0x02) != 0; }
};

/**
 * Device handle type
 */
using DeviceHandle = int;
constexpr DeviceHandle INVALID_DEVICE = 0;

/**
 * Interface for keyboard input driver
 */
class IInputDriver {
public:
    virtual ~IInputDriver() = default;
    
    /**
     * Initialize the driver
     * @return true on success
     */
    virtual bool initialize() = 0;
    
    /**
     * Shutdown the driver and release resources
     */
    virtual void shutdown() = 0;
    
    /**
     * Wait for the next key event
     * @param timeoutMs Timeout in milliseconds (-1 for infinite)
     * @return Key stroke if available, nullopt on timeout
     */
    virtual std::optional<RawKeyStroke> waitForKey(int timeoutMs = -1) = 0;
    
    /**
     * Get the device that produced the last key event
     */
    virtual DeviceHandle getCurrentDevice() const = 0;
    
    /**
     * Check if a device is a keyboard
     */
    virtual bool isKeyboard(DeviceHandle device) const = 0;
    
    /**
     * Get device identifier string
     */
    virtual const char* getDeviceId(DeviceHandle device) const = 0;
};

/**
 * Interface for keyboard output driver
 */
class IOutputDriver {
public:
    virtual ~IOutputDriver() = default;
    
    /**
     * Send a key event
     * @param event The key event to send
     * @return true on success
     */
    virtual bool sendKey(const core::VKeyEvent& event) = 0;
    
    /**
     * Send multiple key events
     * @param events Vector of key events
     * @return true if all succeeded
     */
    virtual bool sendKeys(const std::vector<core::VKeyEvent>& events) = 0;
    
    /**
     * Block/consume the current input key (don't pass through)
     */
    virtual void blockCurrentKey() = 0;
    
    /**
     * Pass through the current input key unchanged
     */
    virtual void passThrough() = 0;
};

} // namespace capsicain
