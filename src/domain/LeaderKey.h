#pragma once
/**
 * LeaderKey.h
 * 
 * QMK-inspired Leader Key implementation for Interception.
 * 
 * A leader key is a special key that, when pressed, starts a "listening mode"
 * where the next sequence of keys triggers an action.
 * 
 * Example:
 * - Press Leader, then G, then C = git commit
 * - Press Leader, then E, then M = open email client
 * - Press Leader, then D, then D = delete line
 * 
 * This is similar to Vim's leader key concept.
 */

#include <cstdint>
#include <vector>
#include <string>
#include <chrono>
#include <functional>
#include <unordered_map>

namespace capsicain {
namespace domain {

/**
 * A leader key sequence with its action
 */
struct LeaderSequence {
    std::vector<uint16_t> keys;                     // Sequence of keys after leader
    std::function<void()> action = nullptr;         // Custom action callback
    std::vector<uint16_t> outputKeys;               // Keys to send (if no custom action)
    std::string description;                        // Human-readable description
};

/**
 * Result of leader key processing
 */
struct LeaderResult {
    bool consumed = false;              // Key was consumed by leader mode
    bool sequenceMatched = false;       // A valid sequence was completed
    bool sequenceFailed = false;        // Sequence didn't match anything
    bool leaderActive = false;          // Leader mode is currently active
    std::vector<uint16_t> outputKeys;   // Keys to send
};

/**
 * LeaderKeyEngine - Manages leader key sequences
 */
class LeaderKeyEngine {
public:
    // Rule of 5: Explicitly defaulted (has vector members)
    LeaderKeyEngine() noexcept = default;
    ~LeaderKeyEngine() = default;
    LeaderKeyEngine(const LeaderKeyEngine&) = default;
    LeaderKeyEngine& operator=(const LeaderKeyEngine&) = default;
    LeaderKeyEngine(LeaderKeyEngine&&) noexcept = default;
    LeaderKeyEngine& operator=(LeaderKeyEngine&&) noexcept = default;


    using TimePoint = std::chrono::steady_clock::time_point;
    
    /**
     * Configure the leader key
     */
    void setLeaderKey(uint16_t keycode) noexcept {
        m_leaderKey = keycode;
    }
    
    /**
     * Set timeout for leader sequences (default 1000ms)
     */
    void setTimeoutMs(uint16_t ms) {
        m_timeoutMs = ms;
    }
    
    /**
     * Add a leader sequence
     */
    void addSequence(const LeaderSequence& seq) {
        m_sequences.push_back(seq);
    }
    
    /**
     * Add a simple key-to-keys sequence
     */
    void addSequence(const std::vector<uint16_t>& keys, 
                     const std::vector<uint16_t>& output,
                     const std::string& desc = "") {
        LeaderSequence seq;
        seq.keys = keys;
        seq.outputKeys = output;
        seq.description = desc;
        m_sequences.push_back(seq);
    }
    
    /**
     * Process a key event
     */
    LeaderResult process(uint16_t keycode, bool isDown, TimePoint now) {
        LeaderResult result;
        
        // Only process key downs for leader sequences
        if (!isDown) {
            return result;
        }
        
        // Check if this is the leader key
        if (keycode == m_leaderKey && !m_isActive) {
            m_isActive = true;
            m_currentSequence.clear();
            m_startTime = now;
            result.consumed = true;
            result.leaderActive = true;
            return result;
        }
        
        // If not in leader mode, pass through
        if (!m_isActive) {
            return result;
        }
        
        result.consumed = true;
        result.leaderActive = true;
        
        // Check for timeout
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_startTime
        ).count();
        
        if (elapsed > m_timeoutMs) {
            // Timeout - cancel leader mode
            m_isActive = false;
            m_currentSequence.clear();
            result.sequenceFailed = true;
            result.leaderActive = false;
            return result;
        }
        
        // Add key to current sequence
        m_currentSequence.push_back(keycode);
        
        // Check for matches
        bool exactMatch = false;
        bool prefixMatch = false;
        
        for (const auto& seq : m_sequences) {
            if (sequenceEquals(m_currentSequence, seq.keys)) {
                // Exact match!
                exactMatch = true;
                result.sequenceMatched = true;
                result.leaderActive = false;
                m_isActive = false;
                
                if (seq.action) {
                    seq.action();
                } else {
                    result.outputKeys = seq.outputKeys;
                }
                break;
            } else if (isPrefix(m_currentSequence, seq.keys)) {
                prefixMatch = true;
            }
        }
        
        if (!exactMatch && !prefixMatch) {
            // No match possible - cancel
            m_isActive = false;
            m_currentSequence.clear();
            result.sequenceFailed = true;
            result.leaderActive = false;
        }
        
        return result;
    }
    
    /**
     * Check if leader mode is active
     */
    bool isActive() const noexcept {
        return m_isActive;
    }
    
    /**
     * Cancel leader mode
     */
    void cancel() {
        m_isActive = false;
        m_currentSequence.clear();
    }
    
private:
    bool sequenceEquals(const std::vector<uint16_t>& a, 
                        const std::vector<uint16_t>& b) const {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) return false;
        }
        return true;
    }
    
    bool isPrefix(const std::vector<uint16_t>& prefix,
                  const std::vector<uint16_t>& full) const {
        if (prefix.size() >= full.size()) return false;
        for (size_t i = 0; i < prefix.size(); ++i) {
            if (prefix[i] != full[i]) return false;
        }
        return true;
    }
    
    uint16_t m_leaderKey = 0;
    uint16_t m_timeoutMs = 1000;
    bool m_isActive = false;
    std::vector<uint16_t> m_currentSequence;
    std::vector<LeaderSequence> m_sequences;
    TimePoint m_startTime;
};

} // namespace domain
} // namespace capsicain
