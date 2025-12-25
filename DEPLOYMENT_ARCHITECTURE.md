# E-Paper Display - Deployment Architecture

## Table of Contents
1. [Physical Deployment](#physical-deployment)
2. [Software Stack](#software-stack)
3. [Process Architecture](#process-architecture)
4. [Communication Protocols](#communication-protocols)
5. [System Integration](#system-integration)
6. [Resource Management](#resource-management)

---

## Physical Deployment

### Hardware Deployment Diagram

```mermaid
graph TB
    subgraph "Raspberry Pi Hardware"
        CPU[ARM CPU<br/>Cortex-A72]
        RAM[LPDDR4 RAM<br/>1-8 GB]

        subgraph "BCM2835/BCM2711 SoC"
            GPIO_CTRL[GPIO Controller]
            SPI_CTRL[SPI Controller]
            PWM[PWM Controller]
        end

        subgraph "Physical Pins"
            PIN_VCC[Pin 1: 3.3V]
            PIN_GND[Pin 6/9: GND]
            PIN_GPIO17[Pin 11: GPIO 17]
            PIN_GPIO24[Pin 18: GPIO 24]
            PIN_GPIO25[Pin 22: GPIO 25]
            PIN_GPIO8[Pin 24: GPIO 8]
            PIN_GPIO10[Pin 19: GPIO 10]
            PIN_GPIO11[Pin 23: GPIO 11]
        end
    end

    subgraph "E-Paper Display Module"
        subgraph "Display Controller"
            CTRL_IC[IL0373/IL91874<br/>Display IC]
            CTRL_RAM[Display RAM]
            CTRL_LUT[LUT Storage]
        end

        subgraph "E-Ink Panel"
            EPANEL[E-Ink Display<br/>176x264 pixels<br/>2.7 inches]
        end

        subgraph "Interface Pins"
            DISP_VCC[VCC]
            DISP_GND[GND]
            DISP_RST[RST]
            DISP_DC[DC]
            DISP_CS[CS]
            DISP_BUSY[BUSY]
            DISP_DIN[DIN]
            DISP_CLK[CLK]
        end
    end

    CPU --> GPIO_CTRL
    CPU --> SPI_CTRL

    GPIO_CTRL --> PIN_GPIO17
    GPIO_CTRL --> PIN_GPIO24
    GPIO_CTRL --> PIN_GPIO25
    GPIO_CTRL --> PIN_GPIO8

    SPI_CTRL --> PIN_GPIO10
    SPI_CTRL --> PIN_GPIO11

    PIN_VCC -.->|Power 3.3V| DISP_VCC
    PIN_GND -.->|Ground| DISP_GND
    PIN_GPIO17 -.->|Reset Signal| DISP_RST
    PIN_GPIO25 -.->|Data/Cmd| DISP_DC
    PIN_GPIO8 -.->|Chip Select| DISP_CS
    PIN_GPIO24 -.->|Status| DISP_BUSY
    PIN_GPIO10 -.->|SPI Data| DISP_DIN
    PIN_GPIO11 -.->|SPI Clock| DISP_CLK

    DISP_RST --> CTRL_IC
    DISP_DC --> CTRL_IC
    DISP_CS --> CTRL_IC
    DISP_BUSY --> CTRL_IC
    DISP_DIN --> CTRL_IC
    DISP_CLK --> CTRL_IC

    CTRL_IC --> CTRL_RAM
    CTRL_IC --> CTRL_LUT
    CTRL_IC --> EPANEL

    style CPU fill:#ffccbc
    style GPIO_CTRL fill:#fff9c4
    style SPI_CTRL fill:#fff9c4
    style CTRL_IC fill:#c8e6c9
    style EPANEL fill:#e3f2fd
```

### Physical Connection Details

| Connection | Type | Speed | Voltage | Purpose |
|------------|------|-------|---------|---------|
| **Power (VCC/GND)** | DC Power | N/A | 3.3V | Display power supply |
| **RST (GPIO 17)** | Digital Output | 1 MHz | 3.3V | Hardware reset control |
| **DC (GPIO 25)** | Digital Output | 1 MHz | 3.3V | Data/Command mode select |
| **CS (GPIO 8)** | Digital Output | 1 MHz | 3.3V | SPI chip select |
| **BUSY (GPIO 24)** | Digital Input | 1 MHz | 3.3V | Display busy status |
| **DIN/MOSI (GPIO 10)** | SPI | 4-16 MHz | 3.3V | Serial data transmission |
| **CLK/SCLK (GPIO 11)** | SPI | 4-16 MHz | 3.3V | Serial clock |

---

## Software Stack

### Full Software Stack Diagram

```mermaid
graph TB
    subgraph "User Space - Application Layer"
        APP[E-Paper Application<br/>demo.cpp]
    end

    subgraph "User Space - Library Layer"
        subgraph "E-Paper Library (libepaper.a)"
            DRAW[Draw API<br/>draw.cpp]
            FONT[Font System<br/>font.cpp]
            SCREEN[Screen Manager<br/>screen.cpp]
            EPD27[EPD27 Driver<br/>epd27.cpp]
            DEVICE[Device Wrapper<br/>device.cpp]
        end

        BCM_LIB[BCM2835 Library<br/>libbcm2835.so]
    end

    subgraph "Kernel Space"
        subgraph "Linux Kernel Drivers"
            GPIO_DRV[GPIO Driver<br/>/dev/gpiomem]
            SPI_DRV[SPI Driver<br/>/dev/spidev0.0]
        end

        KERNEL[Linux Kernel<br/>6.x]
    end

    subgraph "Hardware Space"
        GPIO_HW[GPIO Hardware]
        SPI_HW[SPI Hardware]
        DISPLAY_HW[E-Paper Display]
    end

    APP --> DRAW
    APP --> SCREEN
    APP --> EPD27
    APP --> DEVICE

    DRAW --> FONT
    DRAW --> SCREEN
    SCREEN --> EPD27
    EPD27 --> DEVICE

    DEVICE --> BCM_LIB

    BCM_LIB --> GPIO_DRV
    BCM_LIB --> SPI_DRV

    GPIO_DRV --> KERNEL
    SPI_DRV --> KERNEL

    KERNEL --> GPIO_HW
    KERNEL --> SPI_HW

    GPIO_HW --> DISPLAY_HW
    SPI_HW --> DISPLAY_HW

    style APP fill:#e3f2fd
    style DRAW fill:#fff9c4
    style FONT fill:#fff9c4
    style SCREEN fill:#c8e6c9
    style EPD27 fill:#ffccbc
    style DEVICE fill:#f8bbd0
    style BCM_LIB fill:#ffe0b2
    style GPIO_DRV fill:#b2dfdb
    style SPI_DRV fill:#b2dfdb
    style KERNEL fill:#90a4ae
    style GPIO_HW fill:#cfd8dc
    style SPI_HW fill:#cfd8dc
    style DISPLAY_HW fill:#cfd8dc
```

### Layer Responsibilities

| Layer | Component | Privileges | Function |
|-------|-----------|------------|----------|
| **Application** | demo.cpp | User | Business logic, display content |
| **High-Level API** | Draw, Font | User | Graphics primitives |
| **Buffer Management** | Screen | User | Framebuffer operations |
| **Driver Logic** | EPD27 | User | Display protocol implementation |
| **HAL** | Device, BCM2835 | Root/GPIO group | Hardware access abstraction |
| **Kernel** | GPIO/SPI drivers | Kernel | Device drivers |
| **Hardware** | BCM2835 SoC | N/A | Physical hardware |

---

## Process Architecture

### Runtime Process Structure

```mermaid
graph TB
    subgraph "Process: epaper_demo"
        MAIN[Main Thread]

        subgraph "Memory Segments"
            TEXT[.text<br/>Code Segment<br/>~100 KB]
            DATA[.data/.bss<br/>Static Data<br/>~20 KB]
            HEAP[Heap<br/>Framebuffer: 5-12 KB<br/>Objects: ~1 KB]
            STACK[Stack<br/>~8 MB default]
        end

        subgraph "Memory Mapped I/O"
            GPIO_MEM[/dev/gpiomem<br/>GPIO registers]
            SPI_MEM[/dev/spidev0.0<br/>SPI interface]
        end
    end

    MAIN --> TEXT
    MAIN --> DATA
    MAIN --> HEAP
    MAIN --> STACK

    MAIN --> GPIO_MEM
    MAIN --> SPI_MEM

    GPIO_MEM -.->|mmap| GPIO_REG[Physical GPIO<br/>0x3F200000]
    SPI_MEM -.->|ioctl| SPI_REG[Physical SPI<br/>0x3F204000]

    style MAIN fill:#e3f2fd
    style TEXT fill:#fff9c4
    style HEAP fill:#c8e6c9
    style GPIO_MEM fill:#ffccbc
    style SPI_MEM fill:#ffccbc
```

### Memory Map

| Segment | Size | Content | Allocation |
|---------|------|---------|------------|
| **Text** | ~100 KB | Compiled code | Static |
| **Data** | ~20 KB | Global variables, font tables | Static |
| **Heap** | 6-13 KB | Framebuffer (5.8-11.6 KB), objects (~1 KB) | Dynamic |
| **Stack** | 8 MB | Function calls, local variables | Dynamic |
| **MMIO** | 4 KB pages | GPIO/SPI register access | Kernel mapped |

---

## Communication Protocols

### SPI Communication Protocol

```mermaid
sequenceDiagram
    participant App as Application
    participant Dev as Device Class
    participant Drv as Kernel Driver
    participant SPI as SPI Hardware
    participant Disp as Display IC

    Note over App,Disp: Command Transmission Sequence

    App->>Dev: send_command(0x24)

    Dev->>Dev: write_pin(DC, LOW)
    Note right of Dev: DC=0: Command mode

    Dev->>Dev: write_pin(CS, LOW)
    Note right of Dev: Enable chip select

    Dev->>Drv: ioctl(SPI_IOC_MESSAGE)
    Drv->>SPI: Configure SPI registers

    loop Send byte
        SPI->>Disp: Clock out bit
        SPI->>Disp: Data on MOSI
    end

    Dev->>Dev: write_pin(CS, HIGH)
    Note right of Dev: Disable chip select

    Note over App,Disp: Data Transmission Sequence

    App->>Dev: send_data(buffer)

    Dev->>Dev: write_pin(DC, HIGH)
    Note right of Dev: DC=1: Data mode

    Dev->>Dev: write_pin(CS, LOW)

    loop For each byte in buffer
        Dev->>Drv: ioctl(SPI_IOC_MESSAGE)
        Drv->>SPI: Transfer byte
        SPI->>Disp: Clock + Data
    end

    Dev->>Dev: write_pin(CS, HIGH)
```

### GPIO Control Flow

```mermaid
flowchart TD
    A[Application] --> B[Device::write_pin]
    B --> C[BCM2835 Library]
    C --> D[/dev/gpiomem]
    D --> E[Memory Mapped Registers]
    E --> F[GPIO Controller]
    F --> G[Physical Pin]
    G --> H[Display Pin]

    I[Display Controller] --> J[BUSY Pin Output]
    J --> K[Physical Pin]
    K --> L[GPIO Controller]
    L --> M[Memory Mapped Registers]
    M --> N[/dev/gpiomem]
    N --> O[BCM2835 Library]
    O --> P[Device::read_pin]
    P --> Q[Application]

    style A fill:#e3f2fd
    style H fill:#cfd8dc
    style I fill:#cfd8dc
    style Q fill:#e3f2fd
```

### Timing Diagram

```mermaid
gantt
    title SPI Transaction Timing (Command + Data)
    dateFormat X
    axisFormat %L μs

    section GPIO
    DC = LOW (Cmd)     :0, 1
    CS = LOW           :1, 1
    CS = HIGH          :10, 1
    DC = HIGH (Data)   :11, 1
    CS = LOW           :12, 1
    CS = HIGH          :5010, 1

    section SPI
    Send Command Byte  :2, 8
    Send Data (5KB)    :13, 5000

    section Display
    Process Command    :11, 10
    Store Data in RAM  :13, 5000
    BUSY = HIGH        :5011, 2000000
```

**Notes**:
- Command transmission: ~10 μs
- Data transmission (5KB): ~5 ms @ 8 MHz SPI clock
- Display refresh: ~2-3 seconds (BUSY signal HIGH)

---

## System Integration

### Deployment Context

```mermaid
C4Context
    title System Context Diagram

    Person(user, "User", "Developer/End User")
    System(app, "E-Paper Application", "Displays graphics and text")
    System_Ext(rpi, "Raspberry Pi OS", "Linux-based operating system")
    System_Ext(hardware, "E-Paper Display", "Physical display hardware")

    Rel(user, app, "Runs", "SSH/Console")
    Rel(app, rpi, "Uses", "System calls")
    Rel(rpi, hardware, "Controls", "GPIO/SPI")
    Rel(hardware, user, "Shows", "Visual output")
```

### Container Diagram

```mermaid
C4Container
    title Container Diagram - E-Paper System

    Container(demo, "Demo Application", "C++23", "Example application")
    Container(draw, "Draw Library", "C++23", "Graphics primitives")
    Container(screen, "Screen Library", "C++23", "Framebuffer management")
    Container(driver, "EPD27 Driver", "C++23", "Display protocol")
    Container(hal, "Device HAL", "C++23", "Hardware abstraction")
    Container_Ext(bcm, "BCM2835 Library", "C", "Low-level GPIO/SPI")
    Container_Ext(kernel, "Linux Kernel", "C", "Hardware drivers")

    Rel(demo, draw, "Uses", "Function calls")
    Rel(demo, screen, "Uses", "Function calls")
    Rel(draw, screen, "Uses", "Function calls")
    Rel(screen, driver, "Uses", "Function calls")
    Rel(driver, hal, "Uses", "Function calls")
    Rel(hal, bcm, "Uses", "Function calls")
    Rel(bcm, kernel, "Uses", "System calls")
```

### Component Interactions

```mermaid
sequenceDiagram
    participant U as User
    participant A as Application
    participant L as E-Paper Library
    participant K as Kernel
    participant H as Hardware

    U->>A: Launch program
    A->>K: Request GPIO/SPI access
    K-->>A: Grant access (if root)

    A->>L: Initialize (Device, EPD27)
    L->>K: mmap /dev/gpiomem
    L->>K: open /dev/spidev0.0
    K-->>L: File descriptors
    L->>H: Hardware reset (RST pin)
    H-->>L: Ready
    L-->>A: Initialized

    A->>L: Drawing operations
    Note over L: Update framebuffer in memory
    L-->>A: Operations complete

    A->>L: Refresh display
    L->>K: SPI write (ioctl)
    K->>H: Data transfer via SPI
    H->>H: Update display (2-3s)
    H-->>K: BUSY = LOW
    K-->>L: Transfer complete
    L-->>A: Refresh complete

    U->>A: View display
    Note over H: Visual output visible

    U->>A: Exit program
    A->>L: Cleanup (sleep)
    L->>H: Sleep command
    L->>K: Close file descriptors
    K-->>L: Resources freed
    L-->>A: Cleanup complete
    A-->>U: Program exits
```

---

## Resource Management

### Resource Lifecycle

```mermaid
flowchart TD
    Start([Program Start]) --> Init1[Create Device]

    Init1 --> Init2[bcm2835_init]
    Init2 --> Init3{Success?}
    Init3 -->|No| Error1[Throw/Return Error]
    Init3 -->|Yes| Init4[bcm2835_spi_begin]

    Init4 --> Init5{Success?}
    Init5 -->|No| Error2[Cleanup BCM2835<br/>Return Error]
    Init5 -->|Yes| Init6[Configure GPIO pins]

    Init6 --> Create1[Create EPD27]
    Create1 --> Create2[Create Screen<br/>Allocate buffer]
    Create2 --> Create3[Create Draw]

    Create3 --> Run[Main Loop:<br/>Draw and Refresh]

    Run --> Cleanup1[Exit Scope]
    Cleanup1 --> Cleanup2[~Draw destructor]
    Cleanup2 --> Cleanup3[~Screen destructor<br/>Free buffer]
    Cleanup3 --> Cleanup4[~EPD27 destructor]
    Cleanup4 --> Cleanup5[~Device destructor<br/>bcm2835_spi_end<br/>bcm2835_close]

    Cleanup5 --> End([Program End])

    Error1 --> End
    Error2 --> End

    style Start fill:#c8e6c9
    style End fill:#c8e6c9
    style Error1 fill:#ffcdd2
    style Error2 fill:#ffcdd2
    style Run fill:#e3f2fd
```

### Resource Ownership

```mermaid
graph TD
    subgraph "Application Scope"
        APP_MAIN[main function]
    end

    subgraph "Owned Resources"
        OWN_DEV[Device<br/>Owns: BCM2835 init]
        OWN_EPD[EPD27<br/>Owns: Display state]
        OWN_SCR[Screen<br/>Owns: Buffer memory]
        OWN_DRW[Draw<br/>Owns: Nothing]
    end

    subgraph "Referenced Resources"
        REF_DEV[Device& reference]
        REF_EPD[Driver& reference]
        REF_SCR[Screen& reference]
    end

    subgraph "System Resources"
        SYS_GPIO[/dev/gpiomem]
        SYS_SPI[/dev/spidev0.0]
        SYS_MEM[Heap Memory]
    end

    APP_MAIN -->|owns| OWN_DEV
    APP_MAIN -->|owns| OWN_EPD
    APP_MAIN -->|owns| OWN_SCR
    APP_MAIN -->|owns| OWN_DRW

    OWN_EPD -->|references| REF_DEV
    OWN_SCR -->|references| REF_EPD
    OWN_DRW -->|references| REF_SCR

    OWN_DEV -.->|acquires| SYS_GPIO
    OWN_DEV -.->|acquires| SYS_SPI
    OWN_SCR -.->|allocates| SYS_MEM

    style APP_MAIN fill:#e3f2fd
    style OWN_DEV fill:#c8e6c9
    style OWN_EPD fill:#c8e6c9
    style OWN_SCR fill:#c8e6c9
    style OWN_DRW fill:#c8e6c9
    style SYS_GPIO fill:#ffccbc
    style SYS_SPI fill:#ffccbc
    style SYS_MEM fill:#ffccbc
```

### Memory Allocation Timeline

```mermaid
gantt
    title Memory Allocation During Execution
    dateFormat X
    axisFormat %L ms

    section Initialization
    BCM2835 init         :0, 50
    GPIO configuration   :50, 20
    SPI configuration    :70, 20

    section Buffer Allocation
    Screen buffer (5.8KB) :90, 5

    section Execution
    Drawing operations    :100, 10

    section Display
    SPI transfer (5KB)    :110, 5
    Display refresh       :115, 2000

    section Cleanup
    Free buffer           :2120, 1
    Close SPI             :2121, 10
    Close BCM2835         :2131, 20
```

---

## Deployment Scenarios

### Scenario 1: Development Environment

```mermaid
graph LR
    subgraph "Development Machine"
        IDE[IDE/Editor<br/>VSCode/Vim]
        SSH[SSH Client]
    end

    subgraph "Raspberry Pi"
        SSHD[SSH Daemon]
        BUILD[Build System<br/>CMake + GCC]
        RUN[Run Application<br/>sudo ./demo]
        DISPLAY[E-Paper Display]
    end

    IDE -->|Edit code| SSH
    SSH -->|SSH connection| SSHD
    SSHD --> BUILD
    BUILD -->|Compile| RUN
    RUN -->|Control| DISPLAY
    DISPLAY -->|Visual feedback| IDE

    style IDE fill:#e3f2fd
    style BUILD fill:#fff9c4
    style RUN fill:#c8e6c9
    style DISPLAY fill:#ffccbc
```

### Scenario 2: Production Deployment

```mermaid
graph TB
    subgraph "Boot Process"
        BOOT[System Boot]
        SYSTEMD[systemd]
        SERVICE[epaper.service]
    end

    subgraph "Application"
        APP[E-Paper App]
        LIB[libepaper.a]
        BCM[libbcm2835.so]
    end

    subgraph "Display"
        HW[E-Paper Display]
    end

    BOOT --> SYSTEMD
    SYSTEMD --> SERVICE
    SERVICE --> APP
    APP --> LIB
    LIB --> BCM
    BCM --> HW

    style BOOT fill:#c8e6c9
    style APP fill:#e3f2fd
    style HW fill:#ffccbc
```

**systemd service example**:

```ini
[Unit]
Description=E-Paper Display Service
After=multi-user.target

[Service]
Type=simple
User=root
WorkingDirectory=/opt/epaper
ExecStart=/opt/epaper/bin/epaper_app
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

### Scenario 3: Embedded System

```mermaid
graph TD
    subgraph "Raspberry Pi Zero W"
        CPU[ARM CPU<br/>Single Core]
        RAM[512 MB RAM]
        STORAGE[SD Card<br/>8-16 GB]
    end

    subgraph "Software"
        OS[Raspberry Pi OS Lite<br/>Minimal install]
        APP[E-Paper Application<br/>Auto-start on boot]
    end

    subgraph "Hardware"
        DISPLAY[E-Paper Display<br/>Low power]
        BATTERY[Battery/Solar<br/>Power source]
    end

    STORAGE --> OS
    CPU --> OS
    RAM --> APP
    OS --> APP
    APP --> DISPLAY
    BATTERY --> CPU
    BATTERY --> DISPLAY

    style CPU fill:#ffccbc
    style APP fill:#e3f2fd
    style DISPLAY fill:#c8e6c9
```

---

## Performance Characteristics

### System Resource Usage

```mermaid
pie title CPU Usage During Typical Operation
    "Idle/Waiting" : 95
    "Drawing Operations" : 1
    "SPI Transfer" : 2
    "Display Refresh Wait" : 2
```

```mermaid
pie title Memory Distribution
    "Code (.text)" : 40
    "Static Data" : 8
    "Framebuffer (Heap)" : 48
    "Objects (Heap)" : 2
    "Stack" : 2
```

### Bottleneck Analysis

| Component | Performance | Bottleneck? |
|-----------|-------------|-------------|
| **Drawing Operations** | <1ms for typical scene | ❌ No |
| **Framebuffer Update** | <1μs per pixel | ❌ No |
| **SPI Transfer** | ~5ms for 5KB @ 8MHz | ⚠️ Minor |
| **Display Refresh** | 2-3 seconds | ✅ **Major** |
| **Memory Bandwidth** | >1 GB/s | ❌ No |
| **CPU Utilization** | <5% during drawing | ❌ No |

**Conclusion**: The physical e-ink display refresh time (2-3 seconds) is the primary bottleneck. This is a hardware limitation, not software.

---

## Security Considerations

### Privilege Requirements

```mermaid
flowchart TD
    A[Application Start] --> B{Running as root?}
    B -->|Yes| C[Full GPIO/SPI access]
    B -->|No| D{User in gpio group?}

    D -->|Yes| E[Limited GPIO access via /dev/gpiomem]
    D -->|No| F[Permission Denied]

    C --> G[Application runs normally]
    E --> H{SPI access?}

    H -->|Yes| G
    H -->|No| I[Add user to spi group<br/>or run with sudo]

    F --> J[Error: Cannot access hardware]
    I --> J

    style G fill:#c8e6c9
    style F fill:#ffcdd2
    style J fill:#ffcdd2
```

### Best Practices

| Practice | Recommendation | Rationale |
|----------|----------------|-----------|
| **Development** | Use `sudo` | Full access, easier debugging |
| **Production** | Add user to `gpio` and `spi` groups | Principle of least privilege |
| **Service** | Run as dedicated user with group permissions | Security isolation |
| **Testing** | Use mock hardware layer | No root required for unit tests |

---

## Monitoring and Debugging

### Debug Information Flow

```mermaid
flowchart LR
    APP[Application] -->|stdout| LOG1[Console Output]
    APP -->|stderr| LOG2[Error Output]
    APP -->|Debug symbols| GDB[GDB Debugger]

    KERNEL[Kernel] -->|dmesg| LOG3[Kernel Log]
    KERNEL -->|/sys/kernel/debug| LOG4[Debug FS]

    BCM[BCM2835 Lib] -->|GPIO state| SYS1[/sys/class/gpio/]
    BCM -->|SPI config| SYS2[/sys/class/spi/]

    LOG1 --> MONITOR[Monitoring Tools]
    LOG2 --> MONITOR
    LOG3 --> MONITOR

    style APP fill:#e3f2fd
    style MONITOR fill:#fff9c4
```

### Diagnostic Commands

```bash
# Check GPIO state
cat /sys/kernel/debug/gpio

# Check SPI devices
ls -l /dev/spi*

# Monitor system calls
strace -e open,ioctl,mmap ./epaper_demo

# Profile application
perf record ./epaper_demo
perf report

# Memory debugging
valgrind --leak-check=full ./epaper_demo

# Monitor hardware signals (requires logic analyzer)
# Connect to GPIO pins and observe SPI/GPIO signals
```

---

## Scalability and Extensions

### Multi-Display Support

```mermaid
graph TB
    APP[Application]

    subgraph "Display 1"
        D1_DEV[Device 1]
        D1_EPD[EPD27 1]
        D1_SCR[Screen 1]
        D1_HW[E-Paper 1<br/>CS: GPIO 8]
    end

    subgraph "Display 2"
        D2_DEV[Device 2]
        D2_EPD[EPD27 2]
        D2_SCR[Screen 2]
        D2_HW[E-Paper 2<br/>CS: GPIO 7]
    end

    APP --> D1_SCR
    APP --> D2_SCR

    D1_SCR --> D1_EPD
    D1_EPD --> D1_DEV
    D1_DEV --> D1_HW

    D2_SCR --> D2_EPD
    D2_EPD --> D2_DEV
    D2_DEV --> D2_HW

    style APP fill:#e3f2fd
```

**Note**: Each display requires separate chip select (CS) pin and Device instance.

---

## Summary

This deployment architecture demonstrates:

1. **Clear separation** between software layers and hardware
2. **Efficient resource management** through RAII and ownership
3. **System integration** via standard Linux interfaces
4. **Performance characteristics** dominated by display hardware
5. **Security considerations** for GPIO/SPI access
6. **Extensibility** for multiple displays or new hardware

The architecture is suitable for:
- **Development**: Rapid prototyping and debugging
- **Production**: Embedded systems and IoT devices
- **Education**: Learning embedded Linux and hardware interfacing

---

*For detailed component architecture, see `ARCHITECTURE.md`*
*For quick API reference, see `ARCHITECTURE_QUICK_REFERENCE.md`*

