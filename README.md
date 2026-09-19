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
Bluetooth is deferred.

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

These are planning assignments from the current `.ioc`, not a validated codec
configuration. The file currently reports a 30.2% SAI audio-frequency error and a
250.0 kHz real audio frequency; clock setup must be corrected before audio testing.

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
2. Choose one coherent audio clock arrangement: STM32-generated MCLK or an
   external oscillator. The ReSpeaker 24.576 MHz source is a reference option,
   not yet the selected guitarAmp clock. Define master/slave roles accordingly.
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
