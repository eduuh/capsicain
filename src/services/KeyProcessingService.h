#pragma once

#include <vector>
#include "legacy/configUtils.h"  // For legacy ::VKeyEvent
#include "core/Types.h"
#include "core/KeyEvent.h"
#include "domain/TapDetector.h"
#include "domain/ModifierTracker.h"
#include "domain/KeyMapper.h"
#include "domain/ComboMatcher.h"

// Forward declarations
namespace capsicain {
namespace services {
    class ConfigurationService;
    class RuntimeStateService;
    class HardwareService;
    class UIService;
}
}

namespace capsicain {
namespace services {

/**
 * @brief Result of processing a keystroke
 */
struct ProcessResult {
    bool shouldForward = false;        ///< Forward original key to OS
    bool consumed = false;             ///< Key was consumed (no output)
    std::vector<::VKeyEvent> outputEvents;  ///< Replacement key events to send (legacy VKeyEvent)
};

/**
 * @brief KeyProcessingService - Core keystroke processing pipeline
 *
 * This service encapsulates the entire keystroke processing logic,
 * replacing the monolithic main loop with a clean service architecture.
 *
 * **Replaces:**
 * - `loopState` global (now private `ProcessingContext`)
 * - `modifierState` global (now private `ModifierTracker` instance)
 * - 550+ lines of inline processing logic in capsicain_main_impl()
 *
 * **Orchestrates:**
 * - TapDetector: Detects tap vs hold behavior
 * - ModifierTracker: Tracks modifier key state
 * - KeyMapper: Applies rewire and alpha mappings
 * - ComboMatcher: Matches key combinations
 *
 * **Processing Pipeline (10 stages):**
 * 1. Reset context
 * 2. Check capsicain on/off
 * 3. Device filtering
 * 4. Apple keyboard normalization
 * 5. Messy keys (Print, Pause, Break)
 * 6. Tap detection
 * 7. Rewire mapping
 * 8. Modifier update
 * 9. Combo matching
 * 10. Alpha mapping
 *
 * **Usage:**
 * ```cpp
 * KeyProcessingService processor(config, runtime, hardware, ui);
 * auto keystroke = hardware.waitForKey(2);
 * if (keystroke) {
 *     auto result = processor.processKeystroke(*keystroke);
 *     if (result.shouldForward) {
 *         hardware.sendCurrentKey();
 *     } else if (!result.consumed) {
 *         hardware.sendKeys(result.outputEvents);
 *     }
 * }
 * ```
 */
class KeyProcessingService {
public:
    /**
     * @brief Construct with service dependencies
     *
     * @param config Configuration service (mappings, options)
     * @param runtime Runtime state service (capsicain on/off, config switching, etc.)
     * @param hardware Hardware service (device info, previous strokes for tap detection)
     * @param ui UI service (labels for debugging)
     */
    KeyProcessingService(
        ConfigurationService& config,
        RuntimeStateService& runtime,
        HardwareService& hardware,
        UIService& ui
    );

    /**
     * @brief Process a single keystroke through the full pipeline
     *
     * This is the main entry point that replaces the inline processing
     * logic in capsicain_main_impl().
     *
     * @param keystroke Hardware keystroke from Interception
     * @return ProcessResult indicating what to do with the key
     */
    ProcessResult processKeystroke(const struct HardwareKeystroke& keystroke);

    /**
     * @brief Reset all processing state
     *
     * Called when user presses ESC+Backspace to clear state.
     */
    void reset();

private:
    // ========================================================================
    // Processing Context (replaces loopState)
    // ========================================================================

    /**
     * @brief Per-keystroke processing context
     *
     * This struct encapsulates all the temporary state needed while
     * processing a single keystroke. It replaces the global `loopState`.
     *
     * Reset at the start of each keystroke via reset().
     */
    struct ProcessingContext {
        uint8_t scancode = 0;        ///< Hardware scancode from Interception
        int vcode = -1;              ///< Internal virtual key code
        bool isDownstroke = false;   ///< true = press, false = release
        bool isModifier = false;     ///< Is this a modifier key?

        // Tap detection flags
        bool tapped = false;         ///< Key was tapped (quick press/release)
        bool tappedSlow = false;     ///< Tap with autorepeat before release
        bool tapHoldMake = false;    ///< Tap-and-hold action triggered
        bool repeat = false;         ///< Key is auto-repeating

        // Output
        std::vector<VKeyEvent> resultingVKeyEventSequence;

        void reset();
    };

    // ========================================================================
    // Service Dependencies
    // ========================================================================

    ConfigurationService& config_;
    RuntimeStateService& runtime_;
    HardwareService& hardware_;
    UIService& ui_;

    // ========================================================================
    // Domain Components
    // ========================================================================

    TapDetector tapDetector_;
    ModifierTracker modifierTracker_;  ///< Replaces modifierState global
    domain::KeyMapper keyMapper_;
    domain::ComboMatcher comboMatcher_;

    // ========================================================================
    // Processing State
    // ========================================================================

    ProcessingContext context_;  ///< Replaces loopState global

    // ========================================================================
    // Pipeline Stages (internal methods)
    // ========================================================================

    /**
     * @brief Stage 1: Reset context for new keystroke
     */
    void stage1_ResetContext();

    /**
     * @brief Stage 2: Check if capsicain is on/off
     * @return true to continue processing, false to forward key unmodified
     */
    bool stage2_CheckCapsicainOn();

    /**
     * @brief Stage 3: Apply device filtering
     * @param deviceId Interception device ID
     * @return true to continue, false to ignore this device
     */
    bool stage3_DeviceFiltering(int deviceId);

    /**
     * @brief Stage 4: Normalize Apple keyboard keys (swap Alt<>Win if needed)
     */
    void stage4_AppleKeyboardNormalization();

    /**
     * @brief Stage 5: Handle messy keys (Print, Pause, Break)
     * @return true to continue, false if key was handled
     */
    bool stage5_MessyKeys();

    /**
     * @brief Stage 6: Detect tap vs hold behavior
     * @param stroke Current keystroke with timing info
     */
    void stage6_TapDetection(const struct HardwareKeystroke& stroke);

    /**
     * @brief Stage 7: Apply rewire mapping
     * @return true to continue, false if key was consumed
     */
    bool stage7_RewireMapping();

    /**
     * @brief Stage 8: Update modifier tracker state
     */
    void stage8_ModifierUpdate();

    /**
     * @brief Stage 9: Match key combinations
     * @return true to continue, false if combo matched
     */
    bool stage9_ComboMatching();

    /**
     * @brief Stage 10: Apply alpha mapping
     * @return true if mapping applied
     */
    bool stage10_AlphaMapping();
};

/**
 * @brief Hardware keystroke with timing information
 *
 * Extended version of HardwareService::KeyStroke that includes
 * timing info needed for tap detection.
 */
struct HardwareKeystroke {
    int deviceId = 0;
    uint8_t scancode = 0;
    uint16_t state = 0;  // Interception state flags
    uint32_t information = 0;
};

} // namespace services
} // namespace capsicain
