#pragma once

#include <chrono>

namespace capsicain {
namespace services {

/**
 * @brief ProfilingService encapsulates performance profiling functionality
 *
 * Header-only service for minimal overhead. Tracks timing and statistics
 * for keyboard event processing.
 *
 * Usage:
 *   ProfilingService profiler;
 *   profiler.stopwatchRestart();
 *   // ... do work ...
 *   unsigned long elapsed = profiler.stopwatchReadUS();
 */
class ProfilingService {
public:
    ProfilingService() = default;

    /**
     * @brief Get current time point
     * @return Current steady clock time
     */
    [[nodiscard]] std::chrono::steady_clock::time_point getTimepointNow() const noexcept {
        return std::chrono::steady_clock::now();
    }

    /**
     * @brief Restart stopwatch timer
     * @return Elapsed microseconds since last restart/read
     */
    unsigned long stopwatchRestart() noexcept {
        unsigned long duration = stopwatchReadUS();
        timepointStopwatch_ = std::chrono::steady_clock::now();
        return duration;
    }

    /**
     * @brief Read stopwatch without restarting
     * @return Elapsed microseconds since last restart
     */
    [[nodiscard]] unsigned long stopwatchReadUS() const noexcept {
        return static_cast<unsigned long>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - timepointStopwatch_
            ).count()
        );
    }

    // Profiling statistics
    [[nodiscard]] int getCountIncoming() const noexcept { return countIncoming_; }
    [[nodiscard]] int getCountOutgoing() const noexcept { return countOutgoing_; }
    [[nodiscard]] unsigned long getTotalMappingTimeUS() const noexcept { return totalMappingTimeUS_; }
    [[nodiscard]] unsigned long getTotalSendingTimeUS() const noexcept { return totalSendingTimeUS_; }
    [[nodiscard]] unsigned long getWorstMappingTimeUS() const noexcept { return worstMappingTimeUS_; }
    [[nodiscard]] unsigned long getWorstSendingTimeUS() const noexcept { return worstSendingTimeUS_; }

    void incrementIncoming() noexcept { ++countIncoming_; }
    void incrementOutgoing() noexcept { ++countOutgoing_; }
    void addMappingTime(unsigned long us) noexcept {
        totalMappingTimeUS_ += us;
        if (us > worstMappingTimeUS_) worstMappingTimeUS_ = us;
    }
    void addSendingTime(unsigned long us) noexcept {
        totalSendingTimeUS_ += us;
        if (us > worstSendingTimeUS_) worstSendingTimeUS_ = us;
    }

    void reset() noexcept {
        timepointStopwatch_ = std::chrono::steady_clock::now();
        timepointPreviousKeyEvent_ = std::chrono::steady_clock::now();
        timepointLoopStart_ = std::chrono::steady_clock::now();
        countIncoming_ = 0;
        countOutgoing_ = 0;
        totalMappingTimeUS_ = 0;
        totalSendingTimeUS_ = 0;
        worstMappingTimeUS_ = 0;
        worstSendingTimeUS_ = 0;
    }

    // Public members (for legacy compatibility)
    std::chrono::steady_clock::time_point timepointPreviousKeyEvent_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point timepointLoopStart_ = std::chrono::steady_clock::now();

private:
    std::chrono::steady_clock::time_point timepointStopwatch_ = std::chrono::steady_clock::now();

    int countIncoming_ = 0;
    int countOutgoing_ = 0;
    unsigned long totalMappingTimeUS_ = 0;
    unsigned long totalSendingTimeUS_ = 0;
    unsigned long worstMappingTimeUS_ = 0;
    unsigned long worstSendingTimeUS_ = 0;
};

} // namespace services
} // namespace capsicain
