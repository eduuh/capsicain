#pragma once

#include <string>
#include "platform/interception.h"  // For InterceptionKeyStroke

/**
 * ConsoleUI - Handles all console output and user interface
 *
 * Centralizes console printing logic that was scattered throughout capsicain.cpp.
 * This class is responsible for displaying status, help, debug info, and other
 * console-based user interface elements.
 */
class ConsoleUI
{
public:
    ConsoleUI();

    // Application startup/status
    void printHeader();
    void printStatus();
    void printOptions();

    // Help and information
    void printHelp();
    void printKeylabels();

    // Debug output (called during key processing loop)
    void printIKStrokeState(InterceptionKeyStroke iks);
    void printLoopState1Input();
    void printLoopState2Modifier();
    void printLoopStateMappingTime(long us);
    void printLoopState4TapState();
};
