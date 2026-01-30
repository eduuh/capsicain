#include "services/HardwareService.h"
#include "legacy/scancodes.h"

namespace capsicain {
namespace services {

bool HardwareService::initialize()
{
    // Create Interception context
    context_ = interception_create_context();
    if (!context_) {
        return false;
    }

    // Set default filters (keyboard only initially)
    interception_set_filter(context_, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);
    interception_set_filter(context_, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_NONE);

    return true;
}

void HardwareService::shutdown()
{
    if (context_) {
        interception_destroy_context(context_);
        context_ = nullptr;
    }
}

std::optional<HardwareService::KeyStroke> HardwareService::waitForKey(int timeoutMS)
{
    if (!context_) {
        return std::nullopt;
    }

    InterceptionDevice device = 0;  // Initialize to 0
    InterceptionStroke stroke;

    int result = interception_receive(context_, device, &stroke, 1);
    if (device && result > 0) {
        // Update device tracking
        previousDevice_ = currentDevice_;
        currentDevice_ = device;

        // Determine device type
        if (interception_is_keyboard(device)) {
            lastKeyboard_ = device;
        } else if (interception_is_mouse(device)) {
            lastMouse_ = device;
        }

        // Update keystroke history
        InterceptionKeyStroke* ks = (InterceptionKeyStroke*)&stroke;
        updateHistory(*ks);

        return KeyStroke{ device, *ks };
    }

    return std::nullopt;
}

void HardwareService::sendCurrentKey()
{
    if (context_ && currentDevice_) {
        interception_send(context_, currentDevice_, (InterceptionStroke*)&currentStroke_, 1);
    }
}

void HardwareService::sendKey(InterceptionDevice device, const InterceptionKeyStroke& stroke)
{
    if (context_ && device) {
        interception_send(context_, device, (InterceptionStroke*)&stroke, 1);
    }
}

void HardwareService::setMouseFilter(bool enable)
{
    if (context_) {
        if (enable) {
            interception_set_filter(context_, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL);
        } else {
            interception_set_filter(context_, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_NONE);
        }
    }
}

void HardwareService::updateHistory(const InterceptionKeyStroke& stroke)
{
    // Shift history
    previousStroke2_ = previousStroke1_;
    previousStroke1_ = currentStroke_;
    currentStroke_ = stroke;
}

} // namespace services
} // namespace capsicain
