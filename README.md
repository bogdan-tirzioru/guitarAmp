# guitarAmp

STM32-based guitar effects and amplifier project. The first design focuses on a
working guitar audio path, headphone listening, a backing-track input, and USB
recording. Initial effects are overdrive and delay.

Status: hardware planning and CubeMX peripheral/pin allocation. Requirements below
describe the intended design, not completed or validated functionality.

## Agreed requirements

| Area | Requirement |
| --- | --- |
| Guitar input | Mono instrument input with approximately 1 MΩ input impedance, protection, and suitable analog gain/headroom. |
| Backing-track input | Auxiliary line input. Accept stereo L/R at the connector and sum to mono before the codec using a proper summing circuit; never short source outputs together. |
| Conversion | One codec IC with two ADC channels and two DAC channels. Separate ADC/DAC ICs are outside the first-design scope. |
| Channel allocation | ADC left: guitar. ADC right: mono auxiliary. Keep sources separate until digital processing/mixing. |
| Bluetooth option | BM83SM1-00AB A2DP receiver; I²S into STM32, independent backing-track level, bypass guitar effects, included in the USB recording mix. |
| Effects | Process guitar through STM32 overdrive/delay; auxiliary audio bypasses guitar effects. |
| Mixing | Independent guitar and auxiliary levels, followed by output level control. Mix in STM32 so the backing track is included in USB recording. |
| Headphones | Headphone output is required, using the selected codec's integrated drivers. |
| Speaker output | External LM1875 power-amplifier stage. Final speaker channel count, power supply, load, and cooling remain open. |
| USB | High-speed USB planned. Audio recording of the digital mix is required; audio playback, HID, and mass-storage functions are planned, with implementation stages still to be defined. |
| Simplicity | Prioritize achievable analog hardware and bring-up over additional channels or maximum converter specifications. |
| Budget | Initial project target: approximately EUR 100; complete BOM cost has not been validated. |

Stereo auxiliary capture has been dropped for the first design. The stereo DAC
and headphone outputs remain available for stereo guitar effects or a mono mix
sent to both ears; stereo speaker amplification is not a fixed requirement.
Bluetooth backing-track reception is now an optional peripheral: BM83SM1-00AB,
with decoded digital audio mixed in STM32. The module may be fitted later.

## Selected codec: TI TLV320AIC3104

Selected orderable part: **TLV320AIC3104IRHBR**, as used by the ReSpeaker 2-Mics Pi
HAT V2.0 reference schematic.

Reasons for selection:

- Two ADCs and two DACs in one IC match the simplified audio architecture.
- Integrated headphone drivers and separate differential line outputs.
- Programmable input gain and output volume.
- I²C configuration and I²S-compatible audio connection to STM32 SAI.
- A concrete reference circuit is available in the ReSpeaker V2.0 design.

The device's multiple analog input pins feed only two ADC channels. They do not
provide additional simultaneous digital capture channels. The guitar still needs
an external high-impedance buffer/preamp; codec gain does not replace that stage.

Codec reference supplies are 3.3 V analog/I/O and 1.8 V digital core. Final
regulators, decoupling, power sequencing, input levels, and output coupling must be
checked against the datasheet before layout. Keep AGC disabled for the initial
guitar path to preserve playing dynamics.

## Audio routing

| Path | Routing |
| --- | --- |
| Guitar | Input jack → protection/high-impedance preamp → codec ADC left → STM32 effects |
| Auxiliary | Stereo input jack → L/R mono summing and level conditioning → codec ADC right → STM32 clean path |
| Mix | Processed guitar + clean auxiliary → digital output level → playback and USB recording |
| Headphones | STM32 mix → codec DACs/headphone drivers → coupling/protection → headphone jack |
| Speaker | STM32 mix → codec DACs/line outputs → analog interface → LM1875 → speaker |
| USB recording | STM32 digital mix → USB audio capture stream to host |

Separate dry guitar/auxiliary USB capture channels are an optional extension.
Do not mix auxiliary audio only after the DAC: that would omit it from the USB
recording path. For mono speaker playback, sum digitally or with a designed analog
stage; do not tie codec outputs together.

## MCU and digital hardware

The current [CubeMX project](firmwaredsp/firmwaredsp.ioc) targets
**STM32H5E5ZJT6, LQFP144**. Earlier STM32H7 options were explored; the checked-in
configuration is the current hardware-planning baseline.

| Function | Current allocation / intent |
| --- | --- |
| Codec audio | SAI1 block A master with clock; block B synchronous slave. Final TX/RX roles, framing, DMA, and clock values require configuration. |
| Codec control | I2C1, with a dedicated reset GPIO. |
| External RAM | OCTOSPI1 configured for HyperBus; HyperRAM is planned for audio buffers. Final memory part/capacity remains to be confirmed. |
| Removable storage | SDMMC1 configured for 4-bit SD; microSD planned. |
| Onboard storage | SDMMC2 configured for 4-bit MMC; eMMC planned alongside microSD. |
| USB | USB_OTG_HS configured as a device with internal PHY. |
| Bluetooth (planned) | BM83 via SAI2 block A receive, USART3 control, and three control/status GPIOs; see proposed allocation below. |
| Diagnostics | USART2 and SWD. |
| User interface | Input/output levels and effect controls required; buttons, encoders/potentiometers, LEDs, and any display remain to be selected. |

External NOR was considered earlier; its need and connection are not finalized.
Memory and storage interfaces are allocated, not yet proof of working hardware.

### Current codec-related pin allocation

| STM32 pin | CubeMX signal / intended purpose |
| --- | --- |
| PE2 | SAI1_MCLK_A → codec MCLK, if STM32 supplies the master clock |
| PE4 | SAI1_FS_A ↔ codec WCLK |
| PE5 | SAI1_SCK_A ↔ codec BCLK |
| PE6 | SAI1_SD_A; proposed transmit data to codec DIN |
| PE3 | SAI1_SD_B; proposed receive data from codec DOUT |
| PB6 | I2C1_SCL |
| PB7 | I2C1_SDA |
| PE0 | GPIO labelled SAI_reset; intended codec reset |

The latest `.ioc` selects 48 kHz / 24-bit audio and reports 47.999 kHz with
0.0% rounded error. PLL2P supplies approximately 49.152 MHz to SAI; PLL3Q remains
dedicated to the USB PHY at 16 MHz. Preserve this allocation when adding Bluetooth.
These settings establish the hardware-planning baseline, not hardware validation.

## Optional Bluetooth peripheral and CubeMX pin plan

Selected candidate: **Microchip BM83SM1-00AB**, Mouser **579-BM83SM1-00AB**.
Fit the module directly on the PCB, with the option to leave it unpopulated.
Use A2DP reception and digital I²S output; retain TLV320AIC3104 as the analog codec.

Routing: phone → BM83 → SAI2 receive → sample-rate alignment → independent
backing-track gain → STM32 mixer after guitar effects → codec and USB recording.
The wired auxiliary path remains available. Stereo Bluetooth does not require
extra ADC channels; define stereo/headphone and mono-speaker mixing in the DSP.

### Proposed pin reservations (not yet applied to the .ioc)

These eight pins are unused in the current checked-in configuration. Alternate
functions were checked against STM32H5Exxx DS14971, tables 14 and 15; all are
available on the STM32H5E5 LQFP144 package.

| STM32 pin | CubeMX function / label | BM83 connection |
| --- | --- | --- |
| PD11 | SAI2_SD_A, AF10, receive | DT1, pin 4 → MCU |
| PD12 | SAI2_FS_A, AF10 | RFS1, pin 2 |
| PD13 | SAI2_SCK_A, AF10 | SCLK1, pin 3 |
| PD8 | USART3_TX, AF7 | MCU → UART_RXD, pin 29 |
| PD9 | USART3_RX, AF7 | UART_TXD, pin 30 → MCU |
| PD10 | GPIO output, BT_RESET_N | RST_N, pin 43, via suitable interface |
| PD14 | GPIO output, BT_WAKE | MFB, pin 26, via suitable interface |
| PD15 | GPIO input / optional EXTI15, BT_UART_IND | P0_0 / UART_TX_IND, pin 49 |

In CubeMX, reserve SAI2 block A for asynchronous slave receive with external
BCLK/FS as the initial independent-clock option; leave block B unused. Select
I²S framing; final data width and slot length must match the BM83 configuration.
Use USART3 asynchronous TX/RX; retain USART2 for diagnostics. Control GPIO drive
type, pulls, and reset states depend on the schematic voltage/interface design.

### Clock decision before schematic freeze

The conservative option is BM83 as I²S clock master and SAI2 as slave receiver,
with asynchronous sample-rate conversion into the local 48 kHz audio domain.
This keeps codec SAI1/PLL2 and USB PLL3 independent. Reserve processing budget;
FIFO buffering alone cannot handle continuous clock drift or 44.1-to-48 kHz conversion.

BM83 also supports I²S client mode. A shared-clock alternative may avoid an MCU
ASRC, but requires confirmation that the selected module firmware handles source
rate conversion and radio clock drift at a fixed external 48 kHz rate. Do not
assume client-mode support alone proves this. The same BCLK/FS pins allow revisiting
clock direction without reallocating the existing codec pins.

BM83 MCLK1 is an **output**, not an MCU clock input to the module. Do not connect
it to codec MCLK/PE2. No MCU MCLK pin is reserved for this receive-only option.
Leave DR1 unused unless transmit functionality is added.

### Schematic provisions still required

- Check BM83 supply and I/O levels, reset and MFB drive requirements, and
  unpowered-module isolation before choosing direct wiring or level translation.
- Keep the specified antenna clearance and place the antenna at the PCB edge.
- Expose UART and the module application/test strap for configuration access.
- Resolve UART hardware-flow-control needs before layout. PD11/PD12 are used
  for audio, so they cannot also serve USART3 CTS/RTS; use alternative AF pins
  if the selected configuration requires them.
- Configure the module for MCU control and the selected digital-audio mode;
  factory configuration is not proof of the intended interface behavior.

Scope: peripheral and schematic planning only. No Bluetooth firmware, generated
code, or CubeMX configuration changes are included in this plan.

References:
- [BM83 datasheet, sections 2.2, 6.5 and 6.6](https://ww1.microchip.com/downloads/aemDocuments/documents/WSG/ProductDocuments/DataSheets/BM83-Bluetooth-Stereo-Audio-Module-Data-Sheet-DS70005402.pdf)
- [STM32H5Exxx datasheet](https://www.st.com/resource/en/datasheet/stm32h5e5zj.pdf)

## ReSpeaker V2.0 reference

Reference document: `202004059_ReSpeaker-2-Mics-Pi-HAT-V2.0_SCH_PDF_241121(1).pdf`,
revision V2.0 dated 2024-11-15, supplied during design review. The PDF is not
currently included in this repository.

Page 4 provides the audio circuit; page 2 confirms the change from WM8960 to
TLV320AIC3104. Useful reference elements include:

- 3.3 V rails and a separate 1.8 V core regulator.
- A 24.576 MHz active oscillator, with alternative clock-source resistor options.
- I²C, I²S, and reset connections.
- Headphone outputs using 47 µF coupling capacitors and filtering/protection.
- A separate TPA2005D1 speaker amplifier fed from codec line outputs.

Adapt the microphone circuits to our guitar and auxiliary inputs, and replace the
reference speaker amplifier with the planned LM1875 stage. Verify headphone
left/right mapping: the reference HPLOUT/HPROUT net labels appear swapped after
the coupling capacitors. Size coupling capacitors for the selected headphone load
and bass response rather than copying 47 µF without checking.

## Open decisions and bring-up plan

1. Finish guitar preamp and auxiliary summing/attenuation circuits, including
   clipping headroom, input protection, and AC coupling.
2. Retain the current STM32/PLL2 codec-clock baseline. Resolve the optional
   BM83 clock-domain strategy before freezing its schematic; PLL3 remains for USB.
3. Use 48 kHz with 24-bit samples in 32-bit slots as the proposed initial firmware
   target; verify codec registers and SAI timing together.
4. Finalize headphone impedance/output level, jack wiring, volume ramp/mute, and
   speaker muting when headphones are inserted.
5. Finalize LM1875 power supply, speaker configuration, gain, and cooling.
6. Bring up rails/reset/I²C first, then DAC test tone, ADC capture, clean
   pass-through, effects/mixing, and USB recording of both sources.
7. Measure round-trip latency, clipping/noise, and sustained DMA/USB operation.
   No numeric end-to-end latency target has yet been agreed.
8. Add storage functions and the final controls/display after the audio path works.

## Repository contents and references

- [`firmwaredsp/firmwaredsp.ioc`](firmwaredsp/firmwaredsp.ioc): CubeMX configuration.
- [`AN/`](AN/): MCU reference documents.
- [TI TLV320AIC3104 datasheet](https://www.ti.com/lit/ds/symlink/tlv320aic3104.pdf).

Requirements recorded from design discussions through 2026-09-19. Component
selection and peripheral allocation do not imply that hardware or firmware has
been validated.
