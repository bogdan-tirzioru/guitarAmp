# guitarAmp schematic to-do list

Re-reviewed: 10 October 2026. Previous detailed review: 9 October 2026. Repository: [bogdan-tirzioru/guitarAmp](https://github.com/bogdan-tirzioru/guitarAmp).

Current baseline: `main`, commit [`c599cde4b6e07f61a3ce9b34e6d7a115bc9e0c7e`](https://github.com/bogdan-tirzioru/guitarAmp/tree/c599cde4b6e07f61a3ce9b34e6d7a115bc9e0c7e). Compared with the 9 October review baseline, `97976bce0f3fed0a7a3f9e05cf19d3cab5d09502`, main now includes SD-card detect pull-up, display/UART interface, and USB-shield updates.

## Review scope and limits

The current `.SchDoc` files and recent schematic updates were checked against the previous native-record connectivity review, CubeMX pin allocation, codec driver profile, and the decisions recorded in the project discussion. The 10 October changes include commits `25ee012` (SD detect pull-up), `0734f77` (LCD connector and serial access), and `c599cde` (USB shield). Their resulting open items are captured below.

This is a source-level review. Altium compilation/ERC, compiled cross-sheet netlisting, footprint checks, and physical measurements have not been run. A status marked resolved means the source drawing reflects the correction; it does not prove the compiled project or hardware.

Priority: **P0** = blocking circuit/connectivity fixes; **P1** = complete before schematic freeze; **P2** = optional feature work or release checks.

## Current review summary

| Area | Current source status | Remaining action |
| --- | --- | --- |
| Codec power | DRVDD_1 pin 18 remains unconnected | Connect to the intended +3V3 analog rail and add local decoupling |
| Guitar input | 1 MΩ input bias is restored | Confirm the resistor value is retained in the BOM and input protection/headroom are adequate |
| Auxiliary input | Stereo L/R is resistor-summed to mono on codec ADC right; guitar remains on ADC left | Correct the three polarized 2.2 µF input capacitor orientations; complete overload/protection analysis |
| Controls | All eight ADC connector paths, all 14 connector pins, PF12 and encoder pull-ups are present | Ground the common returns of the eight filter capacitors; finish encoder push idle/bounce and confirm TIM2_CH1 attachment |
| Storage | SD-card detect now has a pull-up | Confirm insertion polarity; decide the unused SD_RESET GPIO role |
| LCD and console | Generic LCD/touch interface and UART access were added | Specify connector rails/pinout, including 5 V module power and 3.3 V logic; verify inactive CS, shared MISO and IRQ behavior |
| USB | Shield connection was updated; USB is self-powered design | Keep USB VBUS as detection only, review divider/backfeed, add ESD protection, and document shield/chassis bond |
| Power tree | System is intended to be self-powered from ±12 V shared with the reused LM1875 board | Add/confirm power input, rail conversion to +5 V, +3V3, +1.8 V, ratings, returns and sequencing |
| Optional features | BM83 sheet still only has a symbol | Defer it explicitly or finish the circuit and clock plan before layout |

The external LM1875 board is intentionally reused; its amplifier circuit is not part of the guitarAmp schematic. The guitarAmp board must still document the shared ±12 V input and audio-ground connection.

## Current sheet state

| Sheet | Present in the current design | Main remaining work |
| --- | --- | --- |
| `microcontroller.SchDoc` | STM32H5E5ZJT6, reset/BOOT0/SWD, HSE, MCU supply decoupling, USB-C and shield update, memory/codec signals, complete 14-pin control connector, LCD/touch and UART interfaces | Power entry/regulators and returns, ADC-filter capacitor ground, encoder switch behavior, LEDs, USB ESD/VBUS details, compiled connectivity |
| `memory.SchDoc` | S70KL1282 HyperRAM; microSD socket, signal pull-ups and card-detect pull-up | Confirm detect polarity; decide SD_RESET use; document memory constraints |
| `audio.SchDoc` | TLV320AIC3104, OPA2134 guitar preamp/buffer, 1 MΩ guitar input, stereo-to-mono auxiliary summing, independent codec ADC routing, separate line-output channels, 220 µF headphone coupling | DRVDD_1 power, three reversed polarized input capacitors, overload/protection, capacitor/BOM metadata and analog headroom |
| `bluetooth.SchDoc` | BM83SM1-00AB symbol | All electrical connections, population decision and clock strategy |

## P0: fix these first

- [ ] **01 — Connect codec DRVDD_1, pin 18.** `audio.SchDoc`: this pin has no wire or supply label; it supplies the analog ADC as well as output-driver circuitry. AVDD, IOVDD and DRVDD_2 already connect to +3V3; DVDD connects to +1.8. Connect DRVDD_1 to the intended analog 3.3 V supply and add its local decoupling. Check AVDD/DRVDD supply relationships and startup sequencing against the TI datasheet. **Done when:** both DRVDD pins are powered in the compiled netlist and every codec supply has the specified decoupling.

- [x] **02 — Reattach the codec SDA and reset labels — resolved in source.** `audio.SchDoc`: `I2C1_SDA` now has an exact local connection to SDA pin 9; `SA1_Reset` attaches to /RESET pin 31 and the existing reset RC. On the MCU sheet, the matching labels reach PB7 pin 137 and PE0 pin 141. The detached-label fault is corrected. Native compiled cross-sheet proof remains under 08; reset capacitor value/MPN consistency remains under 24.

- [ ] **03 — Finalize headphone and amplifier output interfaces — routing corrected, interface partial.** `head_out` now comes from HPROUT pin 23 and HPLOUT pin 19 through 220 µF capacitors. HPRCOM/HPLCOM are unconnected, matching the current disabled COM profile. `amp_out` now comes directly from RIGHT_LOP pin 29 and LEFT_LOP pin 27; RIGHT/LEFT_LOM remain unused. This fixes the previous output selection. The LM1875 is an existing external board reused from the previous project. The reused board's `PowerAmp.SchDoc` now confirms **C1 = 2.2 µF** between input connector P2 pin 2 and LM1875 +IN, with its positive terminal toward P2. This supplies the required DC blocking for the codec's positive output common mode; a second series capacitor on the DSP board is not required for this documented path. Record the chosen single codec output and ground connection as shown below, and confirm the fitted board matches this schematic. Record the single-ended line-output configuration, level and impedance. **Current pinout:** `head_out` 1=R, 2=L, 3=DGND; `amp_out` 1=R_LOP, 2=L_LOP, 3=DGND. **Done when:** those names appear on the schematic and the completed amplifier interface handles codec output common mode and startup transients. Never tie output drivers together.

- [ ] **04 — Finalize headphone capacitor and load specification — values corrected, selection partial.** Both series capacitors now read **220 µF**, with positive terminals toward HPLOUT/HPROUT. The previous 2.2 µF bass-loss issue is corrected for a 32 Ω baseline: `fc = 1/(2πRC)` is approximately **22.6 Hz at 32 Ω** and **45.2 Hz at 16 Ω**, ignoring driver impedance and capacitor tolerance. Specify the minimum headphone impedance and acceptable bass response; if approximately 20 Hz cutoff at 16 Ω is required, 470 µF gives approximately 21.2 Hz. Finalize MPN, voltage rating, tolerance, leakage, effective capacitance and startup behavior. **Done when:** the supported minimum load and cutoff target are recorded and the chosen parts meet them.

- [x] **05 — Preserve independent guitar and mono auxiliary capture — resolved in source.** Guitar still reaches LINE1LP pin 10. `line_in` pin 2 and pin 3 now each pass through a separate **10 kΩ resistor and 2.2 µF capacitor** before joining at LINE1RP pin 12. LINE2L/LINE2R are unused; LINE1RM pin 13 now has a 100 nF AC connection to DGND, like LINE1LM. The isolation resistors prevent directly shorting the source L/R outputs, and the allocation now matches the firmware default LINE1/LINE1 inputs. For low-impedance source outputs and the datasheet nominal 20 kΩ input at 0 dB routing, the midband approximation is `Vaux ≈ 0.4 × (VL + VR)`; equal in-phase L/R gives about 0.8 times one channel, not unity gain. Final levels/protection and the reversed polarized input-capacitor orientation remain under 09–10.

- [ ] **06 — Close the control/encoder return and switch behavior.** The eight analog controls now reach their MCU ADC inputs, including PF12/ADC1_INP6, and the 14-pin connector is complete. However, the common return for the eight 100 nF filter capacitors still needs a direct DGND connection; connector pin 13 does not ground this separate capacitor node. Encoder A/B have 10 kΩ pull-ups and the push switch reaches PF2, but define its idle bias/debounce and confirm the `TIM2_CH1` label is attached to the PF0 net in Altium. **Done when:** the filter capacitors return to DGND, the switch has a defined idle state, and the compiled connector/net mapping is correct.

- [x] **27 — Restore the guitar input impedance — resolved in the current source.** The latest reviewed audio sheet shows the agreed **1 MΩ** bias resistor at the OPA2134 input, restoring the intended high-impedance passive-guitar input. Confirm the annotated value and part number during BOM cleanup.

- [ ] **07 — Define the power entry and rail generation.** The accepted architecture is self-powered from an external ±12 V supply shared with the reused LM1875 board; USB must never power the guitarAmp board. The schematic still needs to show the power connector/interface, current budget, polarity protection, return path and regulator implementation for +5 V (LCD module), +3V3 (MCU/codec/display logic) and +1.8 V (codec core). Check analog rail noise, startup sequencing and regulator thermal limits. **Done when:** every input and rail has a documented source, rating, protection and return path.

- [ ] **08 — Compile and prove cross-sheet connectivity.** The project has four standalone schematic documents; the structure file lists `audio.SchDoc` as top-level and the other three as `NoMainPathDocument`. There are no sheet-symbol/port records linking the sheets. Confirm the intended flat/global net scope or create an explicit hierarchy with ports. **Done when:** compiled nets demonstrably connect every OCTO, SD, SAI1, I2C1 and codec-reset signal between the correct physical pins. Matching visible label text alone is insufficient.

### Reused power-amplifier connection

The power-amplifier schematic already contains the required input coupling. Example mapping using the **left** codec channel; the right channel can be chosen instead. Confirm physical connector pin numbering on the built boards.

| DSP board | Reused power-amplifier board | Function |
| --- | --- | --- |
| `amp_out` pin 2, LEFT_LOP | P2 pin 2 | Mono audio signal through existing C1 |
| `amp_out` pin 3, DGND | P2 pin 1, GND | Signal return |
| `amp_out` pin 1, RIGHT_LOP | Unconnected in this example | Second line-output channel |

Produce the guitar/auxiliary mono mix in STM32 and send it to the selected DAC channel. Never wire LEFT_LOP and RIGHT_LOP together. P1 is the amplifier's supply connector; P3 is its speaker connector. Keep speaker-current return wiring out of the codec signal-return path. The source-level check confirms P2/C1/R3/feedback/supply connectivity; the existing assembly, PCB correspondence, actual supply voltage and performance have not been validated here.

### Current control connector pinout

The `2514-` multipart connector is still `X?`. Pin numbers below come from the native visible pin records; compile and compare with the actual footprint before layout.

| Connector pin | Function / intended MCU pin | Source-level status |
| --- | --- | --- |
| 1 | Encoder A → PF0, MCU pin 10 | Connected directly; 10 kΩ pull-up present; reattach `TIM2_CH1` label |
| 2 | Encoder B → PF1, MCU pin 11 | Connected; `TIM2_CH2` label and 10 kΩ pull-up present |
| 3 | Encoder push → PF2, MCU pin 12 | Connected; `ENC_EXTI2`; add defined idle bias/debounce |
| 4 | Analog control → PF12, MCU pin 50, ADC1_INP6 | **Open at MCU side of filter**; add matching wire/labels |
| 5 | Analog control → PC0, MCU pin 26, ADC1_INP10 | Signal connected |
| 6 | Analog control → PC1, MCU pin 27, ADC1_INP11 | Signal connected |
| 7 | Analog control → PC2, MCU pin 28, ADC1_INP12 | Signal connected |
| 8 | Analog control → PA0, MCU pin 34, ADC1_INP0 | Signal connected |
| 9 | Analog control → PA1, MCU pin 35, ADC1_INP1 | Signal connected |
| 10 | Analog control → PC4, MCU pin 44, ADC1_INP4 | Signal connected |
| 11 | Analog control → PF11, MCU pin 49, ADC1_INP2 | Signal connected |
| 12 | +3V3 | Connected |
| 13 | DGND | Connected; **does not connect the capacitor return node** |
| 14 | Unassigned | No placed visible part/pin; add or explicitly document intended treatment |

Every analog filter still needs its capacitor return connected to DGND. Assign user-facing control names and document external pot/encoder wiring.

### Other interfaces to complete

| Function | Current CubeMX assignment | Schematic action |
| --- | --- | --- |
| Status LEDs | PC13 `LED0`; PC14 `LED1`; PC15 `LED2` | Add LEDs/resistors or an explicitly defined external LED interface |
| Serial console | PA2 USART2_TX / PA3 USART2_RX | Add 3.3 V UART header with GND and clear TX/RX labels |
| Display SPI | PB13 SCK / PB15 MOSI / PB14 MISO | Add generic display/touch connector |
| Display controls | PB12 LCD_CS; PE14 LCD_RS; PE15 LCD_RESET | Connect to display header with defined inactive/reset levels |
| Touch controls | PB10 TOUCH_CS; PE13 TOUCH_IRQ | Connect to display header; check IRQ pull-up and shared-MISO behavior |

## P1: complete before schematic freeze

- [ ] **09 — Finalize guitar/auxiliary gain and codec headroom.** Restore the guitar bias resistor first (27). The OPA2134 A non-inverting stage retains a 2 kΩ ground resistor and 0–10 kΩ feedback resistance: nominal gain 1–6. The second 10 kΩ trimmer attenuates its output before the B-channel follower. Record expected passive/active pickup and auxiliary source levels, trimmer range, summing attenuation and maximum ADC level, including the ±12 V op-amp output during overload. The codec nominal 0 dB full-scale input is 0.707 Vrms at DRVDD1 = 3.3 V; AC coupling does not limit excessive AC excursions. **Done when:** gain/attenuation and any series protection or limiting keep both inputs within intended operating and absolute-maximum limits.

- [ ] **10 — Select audio coupling components and input protection.** Choose explicit dielectric/type, voltage rating and MPN for the 220 nF guitar input capacitor and the three 2.2 µF preamp/auxiliary coupling capacitors. All three polarized input capacitors currently have **positive toward the approximately ground-centered source and negative toward the codec**; with the codec internal positive input bias, this is the opposite DC polarity to the normal operating condition. Reverse them with positive toward the codec, or choose an appropriate non-polar alternative after checking DC/transient conditions. The two new 10 kΩ auxiliary resistors do not correct that polarity. The headphone capacitors have the correct positive-toward-codec orientation. Verify low-frequency response with the selected input impedance/routing and source/summing resistances. Add suitable low-leakage guitar-jack ESD/RF protection and line-input protection. **Done when:** capacitor choices/polarity and overload/ESD design are documented.

- [ ] **11 — Finish codec analog support and startup behavior.** Review LINE1LM and the newly added LINE1RM 100 nF connections to ground and define every unused input/output disposition for the selected single-ended profile. Verify exposed-pad ground, local bulk/ceramic decoupling, quiet analog rail distribution, MCLK source and optional signal damping. Keep the existing STM32 SAI1 master-clock baseline unless deliberately changed. Define mute/ramp and jack-insertion behavior; add headphone detect or amplifier mute control if required. **Done when:** all codec pins have an intentional function or documented unused treatment and startup has a defined safe output state.

- [ ] **12 — Finalize wiring to the reused LM1875 board — schematic interface verified.** Reference: [`guitaramp/PCB_Project/PowerAmp.SchDoc`](https://github.com/bogdan-tirzioru/amplifier/blob/b777c9ec8a359d5651b260d3cbb7d02f36bd271f/guitaramp/PCB_Project/PowerAmp.SchDoc). This is one mono LM1875 channel with **P2 pin 2 = audio input, P2 pin 1 = GND**. **C1 = 2.2 µF** provides input AC coupling, positive toward P2; **R3 = 22 kΩ** biases the amplifier side to ground. With a low-impedance codec source, the input coupling cutoff is approximately **3.3 Hz**. **R2 = 1 MΩ** is an additional shunt on the connector side of C1, not the amplifier's audio-band input impedance; the codec sees approximately `22 kΩ || 1 MΩ ≈ 21.5 kΩ` in the audio midband. **R1 = 20 kΩ**, **R5 = 1 kΩ** give audio-band non-inverting gain **1 + 20k/1k = 21** (26.4 dB), with C6 = 22 µF in the feedback ground leg. **P1 = +12 / GND / -12 on pins 1/2/3** as labelled in the schematic. **P3 pins 1/2 = speaker output / GND**. No enable/mute control is present in this circuit. Use codec mute/ramp for startup and start with the existing -12 dB DAC setting; set the final level from actual supply rails, load and measured unclipped output. Do not infer 20 W output from the part name at the schematic ±12 V supply. Send the mono DSP mix to one selected DAC/line-output channel and wire it directly to P2, bypassing the old analog preamp. **Done when:** the fitted board/connector orientation and actual supply match the reference, the selected mono channel/interconnect are recorded on the DSP schematic, and startup/level behavior is checked on hardware.

- [ ] **13 — Complete controls mechanically and electrically.** Document the eight potentiometer values/tapers, endpoint supply and ground, cable pinout, connector keying and open-wiper behavior. A/B pull-ups are now present; finish encoder push bias and debounce, confirm contact-common wiring and selected interrupt edge. Add protection for controls connected by exposed cables. Check total source resistance and ADC sample time; the 100 Ω series resistor alone does not describe the source impedance of a potentiometer. Label gain/level trimmers separately from user-facing DSP controls. **Done when:** connector pinout, ADC-channel table and control functions agree.

- [ ] **14 — Verify the LCD/touch interface and its supply.** A generic SPI LCD/touch interface is now present. For the Waveshare 3.5inch RPi LCD (A) baseline, provide the module's 5 V supply and the 3.3 V logic rail/reference at the connector; use a common ground. Verify the adapter pinout against the actual Rev 4.0 module, define inactive LCD/touch CS levels, and confirm touch IRQ pull-up and shared-MISO release behavior. **Done when:** the connector pinout, supply current budget and signal levels are recorded.

- [ ] **15 — Complete status LED and service access.** UART console access is now present, alongside the existing SWD/NRST/BOOT0 access. Add status LED circuitry for the configured PC13/PC14/PC15 functions (or revise the firmware/pin plan) and ensure the UART connector has 3.3 V logic, ground, and clearly identified TX/RX directions. **Done when:** bring-up status and serial access are available from documented connector pins.

- [ ] **16 — Finish USB protection and verify data-only VBUS behavior.** The design is self-powered; USB VBUS is for detection and must not feed the board rails. D+/D− reach the dedicated HS PHY pins, Type-C CC pull-downs are present, and the shield connection has been updated. Add suitable low-capacitance HS ESD protection; verify the PA9 VBUS-divider threshold/leakage and behavior when the MCU is unpowered; document how connector shield, chassis and DGND are bonded. **Done when:** there is no USB-to-board power path and the protection, detection and shield scheme are explicit.

- [ ] **17 — Verify MCU supplies using the correct H5E5 documentation.** MCU VDD, VDDA, VREF+, VDDIO2 and VDDUSB connect to +3V3; VBAT also uses +3V3. VCAP pins 70/142 and VDD11USB pin 95 share the `VDD11USB` net, with two 2.2 µF capacitors. The VDD11USB-to-VCAP connection is supported for USB HS use; do not treat it as a missing external 1.1 V regulator. Verify the complete LDO-mode circuit, capacitor effective values/ESR and firmware supply configuration. The checked-in `AN/DS_stm32h5f5zj.pdf` targets the H5F family; add the exact H5E5 reference and audit the symbol/footprint pin numbers. **Done when:** a supply/pinout check against STM32H5E5ZJT6 is recorded.

- [ ] **18 — Finalize the 8 MHz HSE implementation.** Q1 is labelled only `ABM3B`; C8/C9 are 15 pF, with a 1 MΩ parallel resistor. Record the exact `ABM3B-8.000MHZ-10-D1G-T` selection if that is the intended part. For its 10 pF load, equal 15 pF capacitors provide 7.5 pF plus parasitic load: 15 pF can be plausible, but calculate with MCU/PCB parasitics, check ESR/startup margin/drive level and the need for the external resistor. Use C0G/NP0 load capacitors. **Done when:** frequency, full part number, load calculation and oscillator checks are documented.

- [ ] **19 — Close microSD detect and reset handling.** A pull-up is now present on `SD_CARD_DET`/PD0. Confirm the socket contact's insertion polarity and the resulting idle/inserted levels. PD1's `SD_RESET` label does not reach a card reset pin; define PD1 as intentionally unused or redesign it as a controlled card-power switch. **Done when:** card-detect behavior is documented and PD1's role is unambiguous.

- [ ] **20 — Close HyperRAM electrical and layout requirements.** The eight DQ nets, RWDS, CK, CS# and RESET# map correctly to the present OCTOSPI assignments; VCC/VCCQ are +3V3, and CS#/RESET# have 10 kΩ pull-ups. Confirm the exact S70KL1282DPBHI020 package, 128 Mbit/16 MiB capacity, 3 V variant, CK# treatment, RFU-pin rules and dual-die latency requirements against the exact part datasheet. Record the initial bus frequency, optional series resistor positions, impedance/length constraints and decoupling placement. **Done when:** the complete bus and its operating assumptions are documented before routing.

## P2: optional Bluetooth and release checks

- [ ] **21 — Decide whether Bluetooth is included in revision 1.** `bluetooth.SchDoc` currently contains only a BM83 symbol: no wires, labels, supplies or passives. Keep it explicitly deferred, or complete it and define a DNP option before layout. An unconnected symbol is not yet an optional populated circuit.

- [ ] **22 — If included, complete BM83 connections.** Provide datasheet-compliant supply/decoupling, ground/pads, reset/MFB drive circuits, firmware/configuration strap access and UART programming access. Connect DT1→PD11, RFS1↔PD12, SCLK1↔PD13, UART_RXD←PD8, UART_TXD→PD9, RST_N←PD10, MFB←PD14 and UART_TX_IND→PD15 through appropriate electrical interfaces. Resolve I/O levels, UART flow control and behavior while unpowered. Leave MCLK1 out of the codec MCLK net. Record antenna keep-out and edge placement.

- [ ] **23 — If included, settle BM83 clock ownership.** The current IOC sets SAI2 as master. Document whether BM83 supplies BCLK/FS with STM32 slave reception and asynchronous rate conversion, or whether a verified BM83 client-mode configuration can follow STM32's 48 kHz clock. Avoid two drivers on the same clock net. **Done when:** schematic signal directions and the IOC agree with the chosen module configuration.

- [ ] **24 — Annotate and correct component metadata.** Many added components remain `R?`, `C?`, `IC?`, `P?`, `J?` or `X?`. Annotate the whole project while preserving intentional multipart grouping. The codec reset capacitor says `Value=10nF` but retains `CC0402KRX7R7BB104`, a 100 nF part number; reconcile value and MPN. Give capacitors explicit values in the displayed comments/BOM, resolve generic audio headers to real connectors, and record footprints, ratings, tolerances and sourcing. **Done when:** every physical part is uniquely identified and the BOM matches intended values.

- [ ] **25 — Run native Altium compilation/ERC and produce review exports.** Check all required pins and cross-sheet labels, connector pin mapping, `TIM2_CH1` attachment, one-pin nets, incompatible drivers, power errors and annotation conflicts. Reconfirm the corrected SDA/reset, auxiliary-summing, control, LCD, UART and USB nets in the compiled project. Export the compiled netlist, schematic PDF and BOM from the same revision; inspect schematic-to-PCB update preview. **Done when:** ERC findings are explained and the exports agree with the source files.

- [ ] **26 — Add bring-up access before routing.** Provide practical access to +5 V, +3V3, +1.8/±12 V and grounds, NRST/BOOT0, codec reset/I2C/MCLK/BCLK/WCLK/DIN/DOUT, ADC input/output nodes, card detect and UART. For USB/HyperRAM high-speed nets, choose probe access that avoids long stubs. Record the board bring-up sequence: supply checks → SWD/UART/LEDs → codec control/clocks → DAC/ADC → controls/display → memory/storage → USB → optional BM83. Hardware verification remains pending until the board exists.

## Recommended work order

1. Connect codec DRVDD_1 and filter-capacitor returns; retain the restored 1 MΩ guitar input (01, 06, 27).
2. Connect the filter return to DGND, finish PF12/ADC1_INP6, switch bias and connector pin 14 (06).
3. Complete the ±12 V power entry/regulator tree and compile cross-sheet connectivity (07–08).
4. Finalize the mono connection to the reused LM1875 board using its verified C1 input coupling; finish headphone load/part selection, input-capacitor polarity and analog headroom/protection (03–04, 09–12).
5. Finish display power/pinout, LEDs, USB protection, MCU/HSE, SD_RESET and HyperRAM details (13–20).
6. Complete or defer Bluetooth; annotate, compile/ERC and export a netlist/PDF/BOM before routing (21–26).

## Re-review verification

- The previous native-record connectivity review at `97976bce` traced the audio and MCU connections; this update checks the latest main changes to the SD-detect pull-up, LCD/UART interface and USB shield alongside those findings.
- Confirmed project decisions: 1 MΩ guitar input, independent guitar/auxiliary ADC routing, external LM1875 reuse, self-powered ±12 V supply, and no USB bus power.
- Current open circuit blockers remain codec DRVDD_1, the shared return of eight control-filter capacitors, three reversed polarized codec-input capacitors, analog input overload/protection, and the undocumented regulator/power-entry tree.
- The Waveshare module's 5 V supply requirement is recorded in the [manufacturer documentation](https://www.waveshare.com/wiki/3.5inch_RPi_LCD_%28A%29); confirm the Rev 4.0 board and adapter pinout before finalizing the connector.
- No native Altium compile/ERC, cross-sheet netlist, footprint audit or physical tests were run. The report is a source review, not board-release sign-off.

## Source references

- Current project sources: [`layout/guitarDSP`](https://github.com/bogdan-tirzioru/guitarAmp/tree/c599cde4b6e07f61a3ce9b34e6d7a115bc9e0c7e/layout/guitarDSP), [`firmwaredsp.ioc`](https://github.com/bogdan-tirzioru/guitarAmp/blob/c599cde4b6e07f61a3ce9b34e6d7a115bc9e0c7e/firmwaredsp/firmwaredsp.ioc), [`README.md`](https://github.com/bogdan-tirzioru/guitarAmp/blob/c599cde4b6e07f61a3ce9b34e6d7a115bc9e0c7e/README.md), [`docs/tlv320aic3104.md`](https://github.com/bogdan-tirzioru/guitarAmp/blob/c599cde4b6e07f61a3ce9b34e6d7a115bc9e0c7e/docs/tlv320aic3104.md).
- [TI TLV320AIC3104 datasheet, SLAS510G](https://www.ti.com/lit/ds/symlink/tlv320aic3104.pdf): pin functions, supply limits, analog input levels, AC-coupled headphones, outputs and sequencing.
- [Reused LM1875 power-amplifier schematic](https://github.com/bogdan-tirzioru/amplifier/blob/b777c9ec8a359d5651b260d3cbb7d02f36bd271f/guitaramp/PCB_Project/PowerAmp.SchDoc), amplifier master `b777c9e`.
- [TI LM1875 datasheet, SNAS524A](https://www.ti.com/lit/ds/symlink/lm1875.pdf): power-amplifier pin functions, operating conditions and stability guidance.
- [TI OPA2134 datasheet](https://www.ti.com/lit/ds/symlink/opa2134.pdf): supply, input/output limits and analog-stage checks.
- [ST STM32H5Exxx datasheet, DS14971](https://www.st.com/resource/en/datasheet/stm32h5e5zj.pdf): exact MCU pinout, power scheme and USB supply.
- `AN/ABM3B-8.000MHZ-10-D1G-T.pdf`: Abracon family datasheet/order-code explanation; confirm the full ordered part.
- BM83 and S70KL1282 exact-part datasheets should be added/pinned to the hardware reference set before closing their verification items.
