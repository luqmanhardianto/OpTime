# Event System

## Overview

The event system provides a fixed-size FIFO queue for inter-module communication without introducing direct dependencies across layers.

## Architecture

```mermaid
flowchart LR
    ButtonDriver --> EventSystem
    RtcDriver --> EventSystem
    TimeService --> EventSystem
    UIController --> EventSystem
    EventSystem --> EventProcessor
    EventProcessor --> ModeManager
    EventProcessor --> NotificationManager
    EventProcessor --> UIController
```

## Queue Model

```mermaid
flowchart LR
    Publish --> Head
    Head --> RingBuffer
    RingBuffer --> Tail
    Tail --> Consume
```

## Characteristics

- Fixed-size ring buffer
- FIFO ordering
- O(1) publish and consume
- No dynamic allocation
- Atomic protection for queue operations
- Overflow count available for diagnostics
