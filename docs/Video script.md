# Video Demonstration Script
## Real-Time Motor Controller with FreeRTOS

**Total Duration**: 4 minutes 30 seconds  
**Target Audience**: Hiring managers, technical recruiters, fellow engineers  
**Equipment**: Camera, tripod, demonstration board, oscilloscope, logic analyzer, laptop

---

## Pre-Production Checklist

### Hardware Setup
- [ ] Motor controller board powered and functional
- [ ] Motor securely mounted with safety guard
- [ ] All test equipment calibrated and connected
- [ ] Laptop showing serial terminal and IDE
- [ ] Good lighting on demonstration area
- [ ] Clear background (no clutter)

### Software Preparation
- [ ] Firmware flashed and verified
- [ ] Debug messages enabled
- [ ] Oscilloscope triggers configured
- [ ] Logic analyzer capture settings ready
- [ ] Screen recording software running

### Camera Setup
- [ ] Camera positioned for overview shot
- [ ] Close-up lens ready for detail shots
- [ ] Microphone tested (clear audio)
- [ ] Frame rate: 60fps for smooth motor rotation capture

---

## Scene-by-Scene Script

### SCENE 1: Introduction (0:00 - 0:30)

**Visual**: 
- Wide shot of completed motor controller on bench
- Motor, display, and test equipment visible
- Professional but approachable setup

**Audio/Narration**:
> "Hi, I'm [Your Name], and this is my real-time motor controller project. This firmware demonstrates professional-level embedded systems development using FreeRTOS, implementing precise motor control with sub-millisecond timing guarantees. Let's dive into how it works."

**On-Screen Text**:
```
BLDC Motor Controller
- FreeRTOS Task Management
- 1kHz PID Control Loop
- SPI, I²C, UART Protocols
- Real-Time Constraints
```

**Camera Work**:
- Start with wide establishing shot (3 seconds)
- Slow zoom into board details (2 seconds)
- Cut to close-up of motor spinning slowly (2 seconds)

---

### SCENE 2: Hardware Tour (0:30 - 1:00)

**Visual**:
- Close-up panning shot across the board
- Point to each component with on-screen labels

**Audio/Narration**:
> "The brain of this system is an STM32F407 Cortex-M4 running at 168 MHz. It communicates with a 14-bit magnetic encoder over SPI for position feedback, drives a three-phase motor driver using complementary PWM outputs, and monitors phase currents through dedicated sense amplifiers. There's also an I²C OLED display for real-time telemetry and a UART interface for debugging."

**On-Screen Labels** (appear as narration mentions each):
```
→ STM32F407 MCU
→ AS5047P Encoder (SPI)
→ DRV8323 Gate Driver
→ INA240 Current Sensors
→ SSD1306 OLED Display (I²C)
→ USB-UART Bridge
```

**Camera Work**:
- Smooth left-to-right pan (8 seconds)
- Hold on MCU (2 seconds)
- Quick cut to spinning motor (2 seconds)

---

### SCENE 3: Power-Up Sequence (1:00 - 1:30)

**Visual**:
- Screen recording of serial terminal
- Split screen: terminal + board close-up

**Audio/Narration**:
> "When powered on, the firmware initializes all peripherals, creates three FreeRTOS tasks, and performs a self-test. The OLED display shows system status, while the UART console provides detailed telemetry."

**Serial Terminal Output** (visible on screen):
```
=====================================
Motor Controller v1.0.0
=====================================
[INIT] Configuring peripherals...
[INIT] SPI1: Encoder interface OK
[INIT] I2C1: Display found (0x3C)
[INIT] TIM1: PWM @20kHz configured
[INIT] ADC1: Current sensing ready
[OK] Hardware initialization complete

[RTOS] Creating tasks...
[RTOS] PID_Task      (Priority 3) - 512 bytes
[RTOS] Safety_Task   (Priority 4) - 256 bytes
[RTOS] Telemetry_Task(Priority 1) - 1024 bytes
[OK] Task creation complete

[ENCODER] Reading position... 127.8°
[SYSTEM] Self-test PASSED
[READY] Waiting for commands...
>
```

**Camera Work**:
- Focus on terminal (15 seconds)
- Cut to OLED display showing startup (5 seconds)
- Wide shot showing powered system (10 seconds)

---

### SCENE 4: Motor Control Demonstration (1:30 - 2:15)

**Visual**:
- Split screen: Motor + Oscilloscope
- Real-time data overlays

**Audio/Narration**:
> "Let's command the motor to spin at 2000 RPM. Watch how quickly it responds to the step input. The PID controller achieves target speed in under 50 milliseconds with minimal overshoot. The oscilloscope shows the PWM waveforms driving the motor phases at 20 kilohertz."

**Commands** (typed in terminal, visible):
```
> speed 2000
[CMD] Setting target speed: 2000 RPM
[PID] Ramping up...

RPM: 450.2/2000.0  | Duty: 15.3% | I: 0.82A
RPM: 1205.8/2000.0 | Duty: 32.1% | I: 1.54A
RPM: 1876.4/2000.0 | Duty: 44.8% | I: 2.18A
RPM: 2012.3/2000.0 | Duty: 45.2% | I: 2.23A
RPM: 2000.5/2000.0 | Duty: 45.0% | I: 2.20A
RPM: 1999.8/2000.0 | Duty: 45.1% | I: 2.21A

[OK] Target reached (42ms settling time)
```

**Oscilloscope Display**:
```
Ch1 (PWM Phase A):  20kHz square wave, 45% duty
Ch2 (Phase Current): Sinusoidal, 2.2A peak
```

**Camera Work**:
- Close-up on motor accelerating (10 seconds)
- Cut to oscilloscope traces (5 seconds)
- Back to motor at steady state (5 seconds)
- Terminal data scrolling (5 seconds)

---

### SCENE 5: Load Disturbance Test (2:15 - 2:45)

**Visual**:
- Motor running steadily
- Apply mechanical brake manually
- Show system response

**Audio/Narration**:
> "Now I'll apply a sudden load to the motor. Notice how the controller immediately compensates by increasing current. The speed drops momentarily but recovers within 65 milliseconds. This demonstrates excellent disturbance rejection."

**Serial Output**:
```
RPM: 2000.2/2000.0 | Duty: 45.0% | I: 2.20A
RPM: 2000.5/2000.0 | Duty: 45.1% | I: 2.21A

[Load Applied!]
RPM: 1882.3/2000.0 | Duty: 52.8% | I: 3.15A
RPM: 1945.7/2000.0 | Duty: 50.2% | I: 2.84A
RPM: 1988.4/2000.0 | Duty: 46.3% | I: 2.35A
RPM: 2000.1/2000.0 | Duty: 45.2% | I: 2.22A

[Recovery complete: 65ms]
```

**Camera Work**:
- Wide shot showing hand applying brake (5 seconds)
- Quick cut to current meter showing spike (3 seconds)
- Terminal showing recovery (7 seconds)

---

### SCENE 6: Real-Time Performance Analysis (2:45 - 3:15)

**Visual**:
- Logic analyzer screen capture
- Timing diagrams with measurements

**Audio/Narration**:
> "The logic analyzer reveals the real-time behavior. The PID task executes exactly every millisecond with only 23 microseconds of jitter. Each iteration completes in 249 microseconds, leaving substantial margin. This is deterministic real-time performance."

**Logic Analyzer Capture**:
```
Channel 1: PID_Task_Pin
  ┌─────────────────────────────────────┐
  │ Period: 1.000ms ± 0.023ms           │
  │ Pulse Width: 249µs                  │
  │ Frequency: 1000.0 Hz                │
  └─────────────────────────────────────┘

Channel 2: SPI_CS (Encoder)
  ┌─────────────────────────────────────┐
  │ Transaction Time: 78µs              │
  │ Clock: 10.02 MHz                    │
  └─────────────────────────────────────┘

Timing Analysis:
✓ Zero deadline misses (10,000 samples)
✓ Worst-case jitter: 23.4µs
✓ Context switch: 5.8µs average
```

**Camera Work**:
- Screen recording of logic analyzer (20 seconds)
- Zoom into timing measurements (5 seconds)
- Show statistics panel (5 seconds)

---

### SCENE 7: Safety Systems Demo (3:15 - 3:45)

**Visual**:
- Emergency stop button press
- Motor stopping immediately
- System status on display

**Audio/Narration**:
> "Safety is critical. When the emergency stop button is pressed, the system responds in under 10 microseconds electronically, disabling PWM outputs. The motor coasts to a complete stop in 68 milliseconds. Overcurrent protection is also active—if phase current exceeds 10 amps, the system shuts down automatically."

**Actions**:
1. Motor running at 4000 RPM
2. Press large red E-stop button (dramatic)
3. PWM immediately stops (scope shows)
4. Motor decelerates to zero
5. Display shows "E-STOP ACTIVE"

**Serial Output**:
```
RPM: 4002.1/4000.0 | Duty: 68.5% | I: 4.12A

[!!! EMERGENCY STOP ACTIVATED !!!]
[0.0µs ] Interrupt triggered
[5.3µs ] Event flag set
[8.6µs ] PWM outputs disabled
[42ms  ] Motor < 100 RPM
[68ms  ] Motor stopped

[SYSTEM] Safe state achieved
[SYSTEM] Press RESET to clear fault
```

**Camera Work**:
- Wide shot showing E-stop press (5 seconds)
- Cut to scope showing PWM disable (3 seconds)
- Motor stopping (slow-motion, 7 seconds)
- Display showing fault status (5 seconds)

---

### SCENE 8: Code Walkthrough (3:45 - 4:15)

**Visual**:
- IDE screen showing key code sections
- Syntax highlighting visible

**Audio/Narration**:
> "Let's look at the code. Here's the PID control task. It runs at 1 kilohertz using `vTaskDelayUntil` for precise timing. The task reads the encoder via SPI, calculates velocity, executes the PID algorithm with anti-windup, and updates the PWM outputs. All critical sections are protected by FreeRTOS mutexes to ensure thread safety."

**Code Sections to Show** (scroll through with highlights):

```c
// PID Control Task (main.c, line 145)
static void vPIDTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1);
    
    xLastWakeTime = xTaskGetTickCount();
    
    for(;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);  // ← Precise 1ms timing
        
        // Read encoder via SPI
        uint16_t angle = Encoder_Read_Angle();        // ← SPI transaction
        
        // Calculate velocity
        g_motor_state.current_rpm = ...;              // ← Delta position → RPM
        
        // Execute PID
        float output = PID_Update(&g_pid, 
                                  setpoint, 
                                  measured);          // ← Control algorithm
        
        // Update motor PWM
        Motor_Set_Duty_Cycle(output);                 // ← Actuate
    }
}
```

**Camera Work**:
- Screen recording, slow scroll (20 seconds)
- Pause on key functions (2-3 seconds each)
- Show task creation code briefly (5 seconds)

---

### SCENE 9: Performance Summary (4:15 - 4:30)

**Visual**:
- Animated performance metrics
- Final shot of working system

**Audio/Narration**:
> "This project demonstrates production-ready firmware with sub-millisecond control loops, robust error handling, and efficient multi-protocol communication—all skills directly applicable to professional embedded development. Thanks for watching! Code and documentation are available on my GitHub."

**On-Screen Graphics** (animated):
```
┌─────────────────────────────────────────┐
│   PERFORMANCE HIGHLIGHTS                │
├─────────────────────────────────────────┤
│ ✓ 1kHz PID Loop (±23µs jitter)         │
│ ✓ 42ms Settling Time                    │
│ ✓ 31% CPU Usage                         │
│ ✓ Zero Deadline Misses                  │
│ ✓ 72-Hour Stress Test PASSED            │
│                                          │
│ Technologies Demonstrated:               │
│  • FreeRTOS Task Management             │
│  • Real-Time Control Algorithms         │
│  • SPI, I²C, UART Protocols             │
│  • Robust Safety Systems                │
│  • Professional Code Structure          │
└─────────────────────────────────────────┘

GitHub: github.com/[username]/motor-controller
LinkedIn: linkedin.com/in/[yourname]
```

**Camera Work**:
- Pull back to wide shot (5 seconds)
- Motor still running smoothly in background
- Fade to GitHub QR code (3 seconds)

---

## Post-Production Editing Notes

### Graphics to Add
1. **Intro Graphic** (0:00-0:05): Project title, your name
2. **Component Labels** (0:30-1:00): Overlay arrows/text on hardware
3. **Data Overlays** (1:30-2:15): Real-time RPM, current, duty cycle
4. **Timing Diagrams** (2:45-3:15): Animated logic traces
5. **Performance Metrics** (4:15-4:30): Final summary card

### Music
- **Intro**: Upbeat tech background music (low volume)
- **Demo Sections**: Ambient electronic (subtle, not distracting)
- **Conclusion**: Same as intro (callback effect)

### Transitions
- Clean cuts (no fancy effects)
- 1-second cross-dissolve between major sections
- Quick cuts for action sequences (motor start/stop)

### Color Grading
- Bright, professional look
- Enhance oscilloscope/display contrast
- Consistent white balance throughout

### Text Overlays
- Font: Roboto Mono (code sections), Roboto (narration)
- Size: Large enough to read on mobile
- Position: Lower third (don't cover hardware)
- Duration: 3-5 seconds per graphic

---

## Alternative B-Roll Shots (Optional)

If time permits, capture these additional shots:

1. **Closeup of PWM Signals on Scope**
   - Show complementary outputs with dead-time
   - Zoom into edge transitions

2. **Thermal Camera View**
   - Show even heat distribution during operation
   - Highlight no hot spots

3. **Encoder Mounting Detail**
   - Show mechanical coupling to motor shaft
   - Explain alignment importance

4. **Circuit Board Manufacturing**
   - Populated vs. bare PCB comparison
   - Highlight 4-layer stackup (if cross-section available)

5. **Development Environment**
   - IDE with multiple files open
   - Version control (Git) showing commit history
   - Unit test execution

---

## YouTube Upload Checklist

### Title
"Real-Time BLDC Motor Controller | FreeRTOS | STM32 | Firmware Engineering Portfolio"

### Description
```
Professional-grade brushless DC motor controller firmware demonstrating:
• FreeRTOS real-time task management
• 1kHz PID control loop with <50µs jitter
• Multi-protocol communication (SPI, I²C, UART)
• Robust safety systems and fault handling
• Production-ready code quality

Hardware:
- STM32F407 ARM Cortex-M4F @ 168MHz
- AS5047P 14-bit magnetic encoder (SPI)
- DRV8323 3-phase gate driver
- Custom PCB with isolated power supplies

Performance:
- 42ms step response settling time
- 8.3% overshoot
- 31% CPU utilization
- 72-hour endurance test passed

📁 Full project documentation, schematics, and code:
https://github.com/[username]/motor-controller

🔗 Connect with me:
LinkedIn: https://linkedin.com/in/[yourname]
Portfolio: https://yourwebsite.com

#EmbeddedSystems #FirmwareEngineering #FreeRTOS #STM32 #MotorControl #RealTime
```

### Tags
```
embedded systems, firmware engineer, FreeRTOS, STM32, motor control, PID controller, 
real-time operating system, BLDC motor, SPI, I2C, UART, ARM Cortex-M4, 
embedded linux, portfolio project, electrical engineering, computer engineering
```

### Thumbnail Design
- High-contrast image of motor controller board
- Large text: "1kHz REAL-TIME CONTROL"
- Subtitle: "FreeRTOS | STM32"
- Your name in corner
- Bright colors (avoid reds/blues, use yellows/greens)

### Chapters (Timestamps)
```
0:00 Introduction
0:30 Hardware Tour
1:00 System Initialization
1:30 Motor Control Demo
2:15 Load Disturbance Test
2:45 Real-Time Performance
3:15 Safety Systems
3:45 Code Walkthrough
4:15 Summary & Conclusion
```

---

## Equipment Rental/Purchase List

If you don't have all test equipment:

### Essential (Must Have)
- **Oscilloscope**: Rigol DS1054Z (~$350) or borrow from makerspace
- **Logic Analyzer**: Saleae Logic 8 (~$400) or cheaper clone (~$10)
- **Camera**: Smartphone (modern phones are sufficient)
- **Tripod**: $20-30

### Nice to Have
- **Macro Lens**: For closeup board shots (~$50)
- **External Microphone**: Lavalier mic for clear audio (~$25)
- **Ring Light**: Better illumination (~$30)

### Software (Free)
- **Screen Recording**: OBS Studio (free)
- **Video Editing**: DaVinci Resolve (free version)
- **Graphics**: Canva (free tier sufficient)

**Total Budget Estimate**: $500-900 (if buying everything new)
**Realistic Cost**: $50-100 (if borrowing equipment)

---

## Final Checklist Before Filming

- [ ] All hardware tested and working
- [ ] Firmware debugged (no live debugging on camera!)
- [ ] Script memorized (natural delivery, not reading)
- [ ] Backup power supply connected
- [ ] Extra SD cards for camera
- [ ] Test shots reviewed (lighting, focus, audio)
- [ ] Quiet environment (no background noise)
- [ ] Professional appearance (clean workspace)
- [ ] Time allocated: 3-4 hours for filming
- [ ] Post-production time: 4-6 hours editing

Good luck with your video! This will be an impressive portfolio piece.