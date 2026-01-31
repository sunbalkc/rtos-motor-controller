# Hardware Schematic - Motor Controller

## Block Diagram

```
                                    12-48V DC Input
                                         │
                                         ├───────┐
                                         │       │
                                    ┌────▼────┐  │
                                    │  Buck   │  │
                                    │  5V/3A  │  │
                                    └────┬────┘  │
                                         │       │
                                    ┌────▼────┐  │
                                    │  LDO    │  │
                                    │  3.3V   │  │
                                    └────┬────┘  │
                                         │       │
        ┌────────────────────────────────┼───────┼──────────────────┐
        │                                │       │                  │
        │  STM32F407VGT6                 │       │                  │
        │  ┌──────────────────────────┐  │       │                  │
        │  │  ARM Cortex-M4F          │  │       │                  │
        │  │  168 MHz, FPU            │  │       │                  │
        │  │  512KB Flash, 192KB RAM  │  │       │                  │
        │  └──────────────────────────┘  │       │                  │
        │                                │       │                  │
        │  SPI1 ────────────────────────┼───────┼──────────┐       │
        │  (10MHz)    PA5,6,7,4         │       │          │       │
        │                                │       │          ▼       │
        │  I2C1 ─────────────────────────┼───────┼────┬─────────┐  │
        │  (400kHz)   PB6,7              │       │    │         │  │
        │                                │       │    ▼         ▼  │
        │  USART1 ────────────────────┐  │       │  ┌────┐  ┌────┐│
        │  (115200)   PA9,10          │  │       │  │OLED│  │EEP-││
        │                             │  │       │  │SSD │  │ROM ││
        │  TIM1 PWM ──────────────────┼──┼───────┼─ │1306│  │24LC││
        │  (20kHz)    PE8-13          │  │       │  └────┘  │256 ││
        │                             │  │       │          └────┘│
        │  ADC1 ──────────────────────┼──┼───────┼──────────┐     │
        │  (12-bit)   PA0,1,2         │  │       │          │     │
        │                             │  │       │          ▼     │
        │  GPIO ──────────────────────┼──┼───────┼────  ┌────────┐│
        │  E-Stop     PC13            │  │       │      │Current ││
        │                             │  │       │      │Sense   ││
        └─────────────────────────────┼──┼───────┼──────│INA240  ││
                                      │  │       │      │ x3     ││
                                      │  │       │      └───┬────┘│
                                      │  │       │          │     │
                                      │  │       │      ┌───▼─────▼─┐
                                      │  │       └──────►           │
                                      │  │              │  DRV8323  │
                                      │  │              │  3-Phase  │
                                      │  │              │  Driver   │
                                      │  └──────────────►           │
                                      │                 └─────┬─────┘
                                      │                       │
                                      ▼                       │
                                ┌──────────┐                 │
                                │ AS5047P  │                 │
                                │ Magnetic │                 │
                                │ Encoder  │                 │
                                │ (14-bit) │                 │
                                └────┬─────┘                 │
                                     │                       │
                                     └───────┐       ┌───────┘
                                             │       │
                                        ┌────▼───────▼────┐
                                        │                 │
                                        │   BLDC Motor    │
                                        │   (3-phase)     │
                                        │                 │
                                        └─────────────────┘
```

## Detailed Schematics

### Power Supply Section

```
VIN (12-48V) ──┬── D1 (Reverse Protection) ──┬── C1 (100uF) ──┬── L1 ──┐
               │                              │                │        │
               │                              ├── R1 (10K) ────┤        │
               │                              │                │        │
               └── Fuse (5A) ─────────────────┘                │        │
                                                                │        │
                                                          ┌─────▼─────┐  │
                                                          │  TPS54331 │  │
                                                          │  Buck     │  │
                                                          │  Conv.    │  │
                                                          └─────┬─────┘  │
                                                                │        │
                                                          C2 (22uF) ────┤
                                                                │        │
                                                          5V (3A) ──────┤
                                                                │        │
                                                          ┌─────▼─────┐  │
                                                          │  AMS1117  │  │
                                                          │  3.3V LDO │  │
                                                          └─────┬─────┘  │
                                                                │        │
                                                          C3 (10uF) ────┤
                                                                │        │
                                                          3.3V (1A) ────┘

Decoupling Capacitors:
- C1: 100µF/63V electrolytic (input)
- C2: 22µF/10V ceramic + 100µF/10V electrolytic (5V rail)
- C3: 10µF/10V ceramic + 100nF ceramic (3.3V rail)
- C4-C10: 100nF ceramics at each IC power pin
```

### MCU Core Section

```
                     STM32F407VGT6
                  ┌──────────────────┐
       BOOT0 ─────┤1  BOOT0      VDD ├──── 3.3V
                  │                  │
       NRST  ─────┤7  NRST      VSS ├──── GND
             10K↓ │                  │
                  │                  │
   8MHz Crystal ──┤5,6 OSC_IN/OUT   │
   ┌──┐ ┌───────┐ │                  │
   │  ├─┤ 22pF  ├─┤                  │
   └──┘ └───────┘ │                  │
                  │                  │
   SWD Connector: │                  │
   SWDIO ─────────┤ PA13 (SWDIO)     │
   SWCLK ─────────┤ PA14 (SWCLK)     │
   GND ───────────┤ GND              │
   3.3V ──────────┤ VDD              │
                  │                  │
   ST-Link V2     │                  │
                  └──────────────────┘

Bypass Capacitors:
- 100nF ceramic at each VDD pin (x4)
- 10µF tantalum at VCAP pins (x2)
- 4.7µF ceramic at VDDA (analog supply)
```

### SPI Encoder Interface

```
    STM32F407               AS5047P Encoder
    ┌─────┐                 ┌─────┐
PA5 ├─────┼── SCK ──────────┤ CLK │ 10MHz
    │     │                 │     │
PA6 ├─────┼── MISO ─────────┤ DO  │
    │     │                 │     │
PA7 ├─────┼── MOSI ─────────┤ DI  │
    │     │                 │     │
PA4 ├─────┼── CS ───────────┤ CSN │
    │     │        10K↑     │     │
    └─────┘                 │ VDD ├── 5V (from level shifter)
                            │     │
                            │ VSS ├── GND
                            └─────┘

Level Shifter (3.3V ↔ 5V):
- TXS0108E 8-channel bidirectional
- VCCA = 3.3V (STM32 side)
- VCCB = 5V (Encoder side)
```

### I2C Display Interface

```
    STM32F407           SSD1306 OLED         24LC256 EEPROM
    ┌─────┐            ┌─────┐               ┌─────┐
PB6 ├─────┼── SCL ─┬───┤ SCL │               │ SCL ├──┬── 4.7K ↑ 3.3V
    │     │  400k  │   │     │               │     │  │
PB7 ├─────┼── SDA ─┼───┤ SDA │               │ SDA ├──┘
    │     │        │   │     │               │     │
    └─────┘        │   │ VDD ├── 3.3V        │ VCC ├── 3.3V
                   │   │ GND ├── GND         │ GND ├── GND
                   │   └─────┘               │     │
                   │                         │ A0-2├── GND (addr 0x50)
                   │                         │ WP  ├── GND
                   │                         └─────┘
                   └─── Bus pullups: 4.7K each to 3.3V
```

### UART Debug Interface

```
    STM32F407               USB-UART Bridge
    ┌─────┐                (CP2102 / FT232)
    │     │                ┌─────┐
PA9 ├─────┼── TX ──────────┤ RXD │
    │     │                │     │
PA10├─────┼── RX ──────────┤ TXD │
    │     │                │     │
    └─────┘                │ VCC ├── 5V USB
                           │ GND ├── GND
                           │ USB+├── USB D+
                           │ USB-├── USB D-
                           └─────┘

Settings: 115200-8-N-1
```

### PWM Motor Drive

```
    STM32F407                     DRV8323RS
    ┌─────┐                       ┌─────┐
PE9 ├─────┼── PWM_AH ─────────────┤ INHA│──┐
    │     │                       │     │  │  Gate Driver A
PE8 ├─────┼── PWM_AL ─────────────┤ INLA│──┘
    │     │                       │     │
PE11├─────┼── PWM_BH ─────────────┤ INHB│──┐
    │     │                       │     │  │  Gate Driver B
PE10├─────┼── PWM_BL ─────────────┤ INLB│──┘
    │     │                       │     │
PE13├─────┼── PWM_CH ─────────────┤ INHC│──┐
    │     │                       │     │  │  Gate Driver C
PE12├─────┼── PWM_CL ─────────────┤ INLC│──┘
    │     │                       │     │
    │     │                       │ VCC ├── 5V
    │     │                       │ GND ├── GND
    │     │                       │ VM  ├── 12-48V (Motor power)
    │     │                       │     │
    │     │                       │ GHA │──┐
    │     │                       │ GLA │  │ To Motor Phase A
    │     │                       │ SHA │──┘
    │     │                       │     │
    │     │                       │ GHB │──┐
    │     │                       │ GLB │  │ To Motor Phase B
    │     │                       │ SHB │──┘
    │     │                       │     │
    │     │                       │ GHC │──┐
    │     │                       │ GLC │  │ To Motor Phase C
    │     │                       │ SHC │──┘
    └─────┘                       └─────┘

PWM Settings:
- Frequency: 20kHz
- Dead-time: 500ns
- Complementary outputs with auto-disable
```

### Current Sensing

```
    Motor Phase A/B/C          INA240A3              STM32 ADC
    ┌─────┐                   ┌─────┐               ┌─────┐
    │     ├── ┬── Rshunt ─┬───┤ IN+ │               │     │
    │Motor│   │   0.1Ω    │   │     │               │     │
    │     │   └───────────┼───┤ IN- │               │     │
    └─────┘               │   │     │               │     │
                          │   │ OUT ├───── 100Ω ────┤ PA0 │ (Phase A)
                     GND ─┘   │     │       ┌──────┤     │
                              │ VCC ├── 3.3V│  10nF│     │
                              │ GND ├── GND └──────┤ PA1 │ (Phase B)
                              └─────┘              │     │
                                                   │ PA2 │ (Phase C)
Gain: 20 V/V                                       └─────┘
Imax = 3.3V / (20 × 0.1Ω) = 1.65A per sense
Total: ±10A with proper scaling
```

### Emergency Stop Circuit

```
    E-Stop Button (NC)           STM32F407
    ┌─────┐                      ┌─────┐
    │  o  │                      │     │
    │ / \ │────┬─── 3.3V         │     │
    │     │    │                 │     │
    └─────┘    │                 │     │
               ├── 10K           │     │
               │                 │     │
               └─────────────────┤ PC13│── Interrupt (falling edge)
                       ┌─────────┤     │
                       │  100nF  │     │
                   GND─┘         └─────┘

Logic: Button pressed = GND = Emergency stop
       Normal = 3.3V
```

## Bill of Materials (BOM)

| Ref Des | Part Number | Description | Qty | Est. Cost |
|---------|-------------|-------------|-----|-----------|
| U1 | STM32F407VGT6 | MCU, ARM Cortex-M4F, 168MHz | 1 | $10.00 |
| U2 | DRV8323RS | 3-Phase Gate Driver | 1 | $5.50 |
| U3 | AS5047P | 14-bit Magnetic Encoder | 1 | $8.00 |
| U4-U6 | INA240A3 | High-Side Current Sense | 3 | $4.50 |
| U7 | SSD1306 | OLED Display Module, 128x64 | 1 | $3.00 |
| U8 | 24LC256 | I2C EEPROM, 256Kbit | 1 | $0.50 |
| U9 | TPS54331 | Buck Converter, 3A | 1 | $2.00 |
| U10 | AMS1117-3.3 | LDO Regulator, 3.3V, 1A | 1 | $0.30 |
| U11 | TXS0108E | 8-Ch Level Shifter | 1 | $1.20 |
| U12 | CP2102 | USB-UART Bridge | 1 | $1.50 |
| Q1-Q6 | FQD2N60C | N-Channel MOSFET (if discrete) | 6 | $3.00 |
| D1 | SS34 | Schottky Diode, 3A | 1 | $0.20 |
| R1-R20 | Various | Resistors (0805) | 20 | $0.20 |
| C1-C30 | Various | Capacitors (0805, electrolytic) | 30 | $3.00 |
| L1 | 47µH | Power Inductor, 3A | 1 | $0.80 |
| Y1 | 8MHz | Crystal Oscillator | 1 | $0.50 |
| SW1 | Emergency Stop Button (NC) | - | 1 | $5.00 |
| J1-J5 | Pin Headers | Various connectors | 5 | $2.00 |
| PCB | 4-layer PCB | 100mm x 80mm | 1 | $20.00 |
| | | | **Total** | **~$70.70** |

## PCB Layout Guidelines

### Layer Stackup (4-layer)
1. **Top Layer**: Signal routing, components
2. **Layer 2 (GND)**: Solid ground plane
3. **Layer 3 (Power)**: Split power planes (3.3V, 5V, Motor power)
4. **Bottom Layer**: Signal routing, return paths

### Critical Design Rules
- **Trace Width**: 
  - Power (Motor): 2mm (50mil) min
  - Signal: 0.2mm (8mil) typical
  - High-speed (SPI): 0.15mm (6mil), controlled impedance
  
- **Clearances**:
  - High voltage (Motor power): 1mm
  - Normal: 0.2mm
  
- **Via Stitching**:
  - Ground plane: every 5mm along high-speed traces
  - Thermal vias under ICs: 4x 0.3mm
  
- **Grounding**:
  - Star ground topology for analog (ADC)
  - Separate digital and power grounds, join at single point
  - Keep motor power return separate from control ground

### Component Placement
- MCU: Center of board
- Power section: Input side, separate from sensitive analog
- Motor driver: Output side, short traces to connectors
- Encoder interface: Away from switching noise sources
- Display: Edge for easy viewing