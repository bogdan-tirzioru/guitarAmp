# guitarAmp schematic to-do list

Reviewed: 5 October 2026. Repository: [bogdan-tirzioru/guitarAmp](https://github.com/bogdan-tirzioru/guitarAmp).

Baseline: `main`, commit [`13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78`](https://github.com/bogdan-tirzioru/guitarAmp/tree/13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78), “setup pots for ANs”, 4 October 2026.

## Review scope

Read the native Altium component, pin, wire, junction and net-label records from all four current `.SchDoc` files. Reconstructed local electrical connections using the stored coordinates, including fractional coordinates, and compared the MCU assignments with `firmwaredsp/firmwaredsp.ioc`, the README, and the codec driver documentation/source. Manufacturer documents were checked for the codec, MCU and analog stages.

This is a source-level schematic review. Altium project compilation/ERC, the compiled cross-sheet netlist, PCB footprints and physical performance have not been validated. Items involving detached labels and connector grouping should be confirmed in Altium. No schematic, IOC or firmware files were changed.

Priority: **P0** = blocking circuit/connectivity fixes; **P1** = complete before schematic freeze; **P2** = optional feature work or release checks.

## Current state

| Sheet | Already present | Main remaining work |
| --- | --- | --- |
| `microcontroller.SchDoc` | STM32H5E5ZJT6, debug/reset/BOOT0, HSE circuit, MCU supply decoupling, memory/codec labels, USB-C, control connector with RC filters | Control wiring/ground/connector numbering, display, LEDs, console connector, power definition and protection |
| `memory.SchDoc` | S70KL1282DPBHI020 HyperRAM, decoupling and reset/CS pull-ups; microSD socket, signal pull-ups and decoupling | Card-detect pull-up/validation, unused reset decision, exact memory requirements and layout constraints |
| `audio.SchDoc` | TLV320AIC3104, OPA2134 guitar preamp/buffer, gain/level trimmers, input/output headers and coupling capacitors | Missing codec supply, detached control labels, auxiliary routing, headphone/line outputs and analog level/protection design |
| `bluetooth.SchDoc` | BM83SM1-00AB symbol | All electrical connections; optional population/clock strategy |

## P0: fix these first

- [ ] **01 — Connect codec DRVDD_1, pin 18.** `audio.SchDoc`: this pin has no wire or supply label. AVDD, IOVDD and DRVDD_2 already connect to +3V3; DVDD connects to +1.8. Connect DRVDD_1 to the intended analog 3.3 V supply and add its local decoupling. Check AVDD/DRVDD supply relationships and startup sequencing against the TI datasheet. **Done when:** both DRVDD pins are powered in the compiled netlist and every codec supply has the specified decoupling.

- [ ] **02 — Reattach the codec SDA and reset labels.** `audio.SchDoc`: `I2C1_SDA` has no local connection to codec SDA pin 9; `SA1_Reset` likewise has no local connection to /RESET pin 31. Their labels sit near the wires but not on their exact stored coordinates. The reset pull-up and capacitor do connect to pin 31. Move both labels onto the wires using the electrical grid. **Done when:** compiled nets show PB7 ↔ codec pin 9 and PE0 ↔ codec pin 31, with the reset RC on that same reset net.

- [ ] **03 — Correct headphone and amplifier output routing.** `audio.SchDoc`: `amp_out` currently connects directly to HPROUT pin 23 and HPLOUT pin 19. `head_out` instead connects through 2.2 µF capacitors to HPRCOM pin 22 and HPLCOM pin 20. The current codec profile enables HPLOUT/HPROUT for headphones and leaves COM outputs disabled. Route the headphone jack from HPLOUT/HPROUT through appropriately sized coupling capacitors. Route the amplifier interface from the intended LEFT/RIGHT line outputs, with a designed single-ended or differential interface and DC blocking as required. **Done when:** the physical output wiring matches the chosen codec profile, with explicit L/R/GND connector labels. Never tie output drivers together.

- [ ] **04 — Replace the 2.2 µF headphone coupling values for the intended load.** The present values are unsuitable for ordinary low-impedance headphones if used in a series AC-coupled path: `fc = 1/(2πRC)` gives approximately 2.26 kHz at 32 Ω and 4.52 kHz at 16 Ω. Select capacitance from the actual minimum headphone impedance and bass-response target; examples are 220 µF at 32 Ω (~22.6 Hz) or 470 µF at 16 Ω (~21.2 Hz). These examples are starting values, not finalized part selections. Check polarity, leakage, voltage rating, driver stability and startup transients. **Done when:** minimum load and calculated cutoff are recorded alongside the selected parts.

- [ ] **05 — Preserve independent guitar and mono auxiliary capture.** Guitar currently reaches LINE1LP pin 10. The two `line_in` signal pins go independently to LINE2R pin 16 and LINE2L pin 14 through 2.2 µF capacitors; there is no L/R mono summing circuit. This conflicts with the README allocation: guitar on ADC left, auxiliary mono on ADC right. Add a proper L/R summing/attenuation circuit feeding one selected right-channel input. Decide between LINE1RP pin 12 and LINE2R pin 16, then align the driver profile: its present default is LINE1 on both ADCs. **Done when:** guitar and auxiliary each have a dedicated ADC channel and external stereo source outputs are never directly shorted together.

- [ ] **06 — Finish the new control connector circuit.** `microcontroller.SchDoc`: eight 100 Ω / 100 nF filter branches are placed, but their MCU sides have no wires/net labels connecting to the ADC pins. The common node of all eight capacitors is also floating; connect it to the intended ground. The ten visible `2510-` connector parts include pin 7 twice and no pin 10; reconcile the multipart connector and annotate it. **Done when:** all connector pins are unique and documented, all eight filters return to ground, and each signal reaches its intended MCU pin. Use the mapping table below.

- [ ] **07 — Define how every power rail is supplied.** The project uses +3V3, +1.8, +12 and -12 power symbols, but the four sheets contain no regulator/power-input implementation that defines their origin. Choose an explicit external supply interface or add the required power circuits. Specify polarity, current budget, analog noise filtering, input protection and startup behavior. Separate the LM1875 power requirement from the preamp supply requirement. **Done when:** every rail has an identified source, a connector or regulator, ratings and a documented return path.

- [ ] **08 — Compile and prove cross-sheet connectivity.** The project has four standalone schematic documents; the structure file lists `audio.SchDoc` as top-level and the other three as `NoMainPathDocument`. There are no sheet-symbol/port records linking the sheets. Confirm the intended flat/global net scope or create an explicit hierarchy with ports. **Done when:** compiled nets demonstrably connect every OCTO, SD, SAI1, I2C1 and codec-reset signal between the correct physical pins. Matching visible label text alone is insufficient.

### Control and interface wiring to complete

| Function | Current CubeMX assignment | Schematic action |
| --- | --- | --- |
| Eight analog controls | PA0, PA1, PC0, PC1, PC2, PC4, PF11, PF12 | Connect one filtered control signal to each; define control names and external potentiometer wiring |
| Encoder A/B | PF0 / PF1, TIM2 CH1/CH2 | Add connector and suitable pull/filter/protection circuits |
| Encoder push button | PF2, `ENC_EXTI2` | Add connector, defined idle level and debounce provision |
| Status LEDs | PC13 `LED0`; PC14 `LED1`; PC15 `LED2` | Add LEDs/resistors or an explicitly defined external LED interface |
| Serial console | PA2 USART2_TX / PA3 USART2_RX | Add 3.3 V UART header with GND and clear TX/RX labels |
| Display SPI | PB13 SCK / PB15 MOSI / PB14 MISO | Add generic display/touch connector |
| Display controls | PB12 LCD_CS; PE14 LCD_RS; PE15 LCD_RESET | Connect to display header with defined inactive/reset levels |
| Touch controls | PB10 TOUCH_CS; PE13 TOUCH_IRQ | Connect to display header; check IRQ pull-up and shared-MISO behavior |

## P1: complete before schematic freeze

- [ ] **09 — Finalize guitar gain and codec headroom.** The current 1 MΩ input bias resistor and 220 nF input capacitor form a plausible high-impedance guitar input. OPA2134 A is non-inverting with a 2 kΩ ground resistor and 0–10 kΩ feedback resistance: nominal gain 1–6. The second 10 kΩ trimmer attenuates the first stage output before the B-channel follower. Record expected passive/active pickup levels, trimmer range and maximum ADC level, including the ±12 V op-amp output during overload. The codec's specified nominal 0 dB full-scale input is 0.707 Vrms at DRVDD1 = 3.3 V; AC coupling does not limit large AC excursions. **Done when:** gain/attenuation and any series protection or limiting keep the codec input within its intended operating and absolute-maximum limits.

- [ ] **10 — Select audio coupling components and input protection.** Choose explicit dielectric/type, voltage rating and part number for the 220 nF guitar input capacitor and the 2.2 µF preamp/auxiliary coupling capacitors. The preamp coupling capacitor's positive terminal faces the approximately ground-biased OPA output; check its DC polarity against the codec-side input bias and change orientation/type if necessary. Verify low-frequency response using the selected codec input impedance/routing. Add appropriate low-leakage guitar-jack ESD/RF protection and protection for line inputs. **Done when:** the audio path has documented capacitor choices, correct polarity and a defined overload/ESD design.

- [ ] **11 — Finish codec analog support and startup behavior.** Review LINE1LM's present 100 nF connection to ground and define every unused input/output disposition for the selected single-ended profile. Verify exposed-pad ground, local bulk/ceramic decoupling, quiet analog rail distribution, MCLK source and optional signal damping. Keep the existing STM32 SAI1 master-clock baseline unless deliberately changed. Define mute/ramp and jack-insertion behavior; add headphone detect or amplifier mute control if required. **Done when:** all codec pins have an intentional function or documented unused treatment and startup has a defined safe output state.

- [ ] **12 — Specify the LM1875 interface.** No LM1875 circuit is present in the four schematic sheets. If it is an external amplifier board, document connector signal level, impedance, AC coupling, grounding, mono/stereo allocation and mute control. If it belongs on this PCB, add a dedicated sheet covering gain/feedback, stability network, supply bypassing, load and thermal design. **Done when:** `amp_out` has a complete electrical contract with the actual amplifier. A speaker must not connect directly to the codec header.

- [ ] **13 — Complete controls mechanically and electrically.** Document the eight potentiometer values/tapers, endpoint supply and ground, cable pinout, connector keying and open-wiper behavior. Add protection for controls connected by exposed cables. Check total source resistance and ADC sample time; the 100 Ω series resistor alone does not describe the source impedance of a potentiometer. Label gain/level trimmers separately from user-facing DSP controls. **Done when:** connector pinout, ADC-channel table and control functions agree.

- [ ] **14 — Add the selected display/touch connector.** Use the generic SPI interface for the Waveshare 3.5inch RPi LCD (A) baseline, with the pin mapping above, power and grounds. Verify the actual module/adapter power and logic requirements. Set inactive CS levels and determine whether MISO is released by each device when deselected; reserve optional series resistors for cable signals. **Done when:** a complete adapter pinout exists and the schematic provides all required LCD/touch signals.

- [ ] **15 — Add status LEDs and console access.** MCU PC13/PC14/PC15 and PA2/PA3 are currently unconnected in the schematic. Add active-high LED circuits matching the existing LED firmware, selecting modest currents appropriate to these MCU pins, or explicitly revise the polarity/pin plan. Add UART access and preserve accessible SWD/NRST/BOOT0. **Done when:** first board bring-up can observe startup/heartbeat and read the serial log without attaching wires to MCU leads.

- [ ] **16 — Finish USB-C protection and power behavior.** The D+/D− routing reaches the dedicated HS PHY pins; both connector orientations are joined, and separate 5.1 kΩ CC pull-downs are already present. Add a suitably low-capacitance USB HS ESD solution and protection for exposed VBUS/CC as appropriate. The current VBUS detection uses a 100 kΩ/100 kΩ divider into PA9: check detection thresholds, leakage, unpowered-MCU behavior and startup. Decide self-powered versus bus-powered behavior and prevent unintended backfeeding. The shield uses `GND` while circuit return uses `DGND`; define the intentional bond/chassis scheme. **Done when:** USB power/detection/protection and shield grounding are explicit.

- [ ] **17 — Verify MCU supplies using the correct H5E5 documentation.** MCU VDD, VDDA, VREF+, VDDIO2 and VDDUSB connect to +3V3; VBAT also uses +3V3. VCAP pins 70/142 and VDD11USB pin 95 share the `VDD11USB` net, with two 2.2 µF capacitors. The VDD11USB-to-VCAP connection is supported for USB HS use; do not treat it as a missing external 1.1 V regulator. Verify the complete LDO-mode circuit, capacitor effective values/ESR and firmware supply configuration. The checked-in `AN/DS_stm32h5f5zj.pdf` targets the H5F family; add the exact H5E5 reference and audit the symbol/footprint pin numbers. **Done when:** a supply/pinout check against STM32H5E5ZJT6 is recorded.

- [ ] **18 — Finalize the 8 MHz HSE implementation.** Q1 is labelled only `ABM3B`; C8/C9 are 15 pF, with a 1 MΩ parallel resistor. Record the exact `ABM3B-8.000MHZ-10-D1G-T` selection if that is the intended part. For its 10 pF load, equal 15 pF capacitors provide 7.5 pF plus parasitic load: 15 pF can be plausible, but calculate with MCU/PCB parasitics, check ESR/startup margin/drive level and the need for the external resistor. Use C0G/NP0 load capacitors. **Done when:** frequency, full part number, load calculation and oscillator checks are documented.

- [ ] **19 — Finish microSD card detection and reset handling.** J1 MP2 is connected to `SD_CARD_DET`/PD0 without a dedicated external pull-up. Verify the socket contact arrangement and define the idle level through an external or explicitly configured internal pull-up. The signal pull-ups are already 47 kΩ on CMD and DAT0–3; CLK has no pull-up. PD1 has an `SD_RESET` label, but it does not reach the memory sheet/socket. A microSD card has no dedicated reset pin: leave PD1 explicitly unused, or use it for a designed card-power switch if power cycling is required. **Done when:** card insertion polarity/idle state and PD1's role are unambiguous.

- [ ] **20 — Close HyperRAM electrical and layout requirements.** The eight DQ nets, RWDS, CK, CS# and RESET# map correctly to the present OCTOSPI assignments; VCC/VCCQ are +3V3, and CS#/RESET# have 10 kΩ pull-ups. Confirm the exact S70KL1282DPBHI020 package, 128 Mbit/16 MiB capacity, 3 V variant, CK# treatment, RFU-pin rules and dual-die latency requirements against the exact part datasheet. Record the initial bus frequency, optional series resistor positions, impedance/length constraints and decoupling placement. **Done when:** the complete bus and its operating assumptions are documented before routing.

## P2: optional Bluetooth and release checks

- [ ] **21 — Decide whether Bluetooth is included in revision 1.** `bluetooth.SchDoc` currently contains only a BM83 symbol: no wires, labels, supplies or passives. Keep it explicitly deferred, or complete it and define a DNP option before layout. An unconnected symbol is not yet an optional populated circuit.

- [ ] **22 — If included, complete BM83 connections.** Provide datasheet-compliant supply/decoupling, ground/pads, reset/MFB drive circuits, firmware/configuration strap access and UART programming access. Connect DT1→PD11, RFS1↔PD12, SCLK1↔PD13, UART_RXD←PD8, UART_TXD→PD9, RST_N←PD10, MFB←PD14 and UART_TX_IND→PD15 through appropriate electrical interfaces. Resolve I/O levels, UART flow control and behavior while unpowered. Leave MCLK1 out of the codec MCLK net. Record antenna keep-out and edge placement.

- [ ] **23 — If included, settle BM83 clock ownership.** The current IOC sets SAI2 as master. Document whether BM83 supplies BCLK/FS with STM32 slave reception and asynchronous rate conversion, or whether a verified BM83 client-mode configuration can follow STM32's 48 kHz clock. Avoid two drivers on the same clock net. **Done when:** schematic signal directions and the IOC agree with the chosen module configuration.

- [ ] **24 — Annotate and correct component metadata.** Many added components remain `R?`, `C?`, `IC?`, `P?`, `J?` or `X?`. Annotate the whole project while preserving intentional multipart grouping. The codec reset capacitor says `Value=10nF` but retains `CC0402KRX7R7BB104`, a 100 nF part number; reconcile value and MPN. Give capacitors explicit values in the displayed comments/BOM, resolve generic audio headers to real connectors, and record footprints, ratings, tolerances and sourcing. **Done when:** every physical part is uniquely identified and the BOM matches intended values.

- [ ] **25 — Run native Altium compilation/ERC and produce review exports.** Resolve unconnected required pins, duplicate multipart pins, detached net identifiers, unexpected one-pin nets, incompatible output drivers, power errors and annotation conflicts. Add no-connect markers only for intentionally unused pins. Export a compiled netlist, schematic PDF and BOM from this same revision. Review the schematic-to-PCB update preview without accepting unintended changes. **Done when:** remaining ERC findings are individually explained and the exports agree with the source files.

- [ ] **26 — Add bring-up access before routing.** Provide practical access to +3V3/+1.8/±12 V and grounds, NRST/BOOT0, codec reset/I2C/MCLK/BCLK/WCLK/DIN/DOUT, ADC input/output nodes, card detect and UART. For USB/HyperRAM high-speed nets, choose probe access that avoids long stubs. Record the board bring-up sequence: supply checks → SWD/UART/LEDs → codec control/clocks → DAC/ADC → controls/display → memory/storage → USB → optional BM83. Hardware verification remains pending until the board exists.

## Recommended work order

1. Fix codec supply, SDA/reset attachment, output routing and auxiliary allocation (01–05).
2. Finish the control connector and power definition, then prove compiled cross-sheet nets (06–08).
3. Close analog levels/protection, amplifier contract, display/LED/UART access and USB protection (09–16).
4. Verify MCU/HSE/microSD/HyperRAM details (17–20).
5. Complete or defer Bluetooth deliberately; annotate, compile, export and review before PCB routing (21–26).

## Source references

- Current project sources: [`layout/guitarDSP`](https://github.com/bogdan-tirzioru/guitarAmp/tree/13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78/layout/guitarDSP), [`firmwaredsp.ioc`](https://github.com/bogdan-tirzioru/guitarAmp/blob/13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78/firmwaredsp/firmwaredsp.ioc), [`README.md`](https://github.com/bogdan-tirzioru/guitarAmp/blob/13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78/README.md), [`docs/tlv320aic3104.md`](https://github.com/bogdan-tirzioru/guitarAmp/blob/13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78/docs/tlv320aic3104.md).
- [TI TLV320AIC3104 datasheet, SLAS510G](https://www.ti.com/lit/ds/symlink/tlv320aic3104.pdf): pin functions, supply limits, analog input levels, AC-coupled headphones, outputs and sequencing.
- [TI OPA2134 datasheet](https://www.ti.com/lit/ds/symlink/opa2134.pdf): supply, input/output limits and analog-stage checks.
- [ST STM32H5Exxx datasheet, DS14971](https://www.st.com/resource/en/datasheet/stm32h5e5zj.pdf): exact MCU pinout, power scheme and USB supply.
- `AN/ABM3B-8.000MHZ-10-D1G-T.pdf`: Abracon family datasheet/order-code explanation; confirm the full ordered part.
- BM83 and S70KL1282 exact-part datasheets should be added/pinned to the hardware reference set before closing their verification items.
