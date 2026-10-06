# guitarAmp schematic to-do list

Re-reviewed: 6 October 2026. Previous review: 5 October 2026. Repository: [bogdan-tirzioru/guitarAmp](https://github.com/bogdan-tirzioru/guitarAmp).

Baseline: `main`, commit [`1bf40d91ba2a38fb19274ef5c1492021644c28ca`](https://github.com/bogdan-tirzioru/guitarAmp/tree/1bf40d91ba2a38fb19274ef5c1492021644c28ca), after “connect the analog lines” (`020e4d4`, 5 October 2026). Compared against the previous schematic baseline `13b2ba61d08a8cd7ee21c4b1fe8f92e8c576da78`. The repository primary branch is `main`; there is no `master` branch.

## Review scope

Read the native Altium component, pin, wire, junction and net-label records from all four current `.SchDoc` files. Reconstructed local electrical connections using the stored coordinates, including fractional coordinates, and compared the MCU assignments with `firmwaredsp/firmwaredsp.ioc`, the README, and the codec driver documentation/source. For this re-review, checked the changed native records and reconstructed local connections in `audio.SchDoc` and `microcontroller.SchDoc`, including pin orientation/length and fractional coordinates. `memory.SchDoc`, `bluetooth.SchDoc`, the project configuration, IOC, README and codec firmware are unchanged from the previous baseline. Rechecked TI SLAS510G pin functions, input impedance, supply rules and audio coupling requirements. Retained the previous MCU/HyperRAM/HSE checks where the sources have not changed.

This is a source-level schematic review. Altium project compilation/ERC, the compiled cross-sheet netlist, PCB footprints and physical performance have not been validated. Items involving detached labels and connector grouping should be confirmed in Altium. No schematic, IOC or firmware files were changed.

A checked item means the stated source-level correction is verified; it does not imply native Altium ERC or hardware validation. Existing item numbers are preserved.

Priority: **P0** = blocking circuit/connectivity fixes; **P1** = complete before schematic freeze; **P2** = optional feature work or release checks.

## Changes verified on 6 October

| Item | Status | Evidence / remaining action |
| --- | --- | --- |
| 01 codec DRVDD_1 | Open | Pin 18 still has no electrical connection |
| 02 codec SDA/reset | Resolved in source | `I2C1_SDA` now attaches to pin 9; `SA1_Reset` attaches to pin 31 and its RC |
| 03 headphone / amplifier routing | Partial | HPLOUT/HPROUT now feed headphones; LEFT/RIGHT_LOP feed `amp_out`; interface to reused LM1875 board still to verify |
| 04 headphone capacitors | Partial | Both changed from 2.2 µF to 220 µF with positive terminals toward codec; minimum load, cutoff target and real parts still to specify |
| 05 independent capture | Resolved in source | Guitar → LINE1LP; auxiliary L/R each through 10 kΩ and 2.2 µF → common LINE1RP node; matches the default LINE1/LINE1 driver |
| 06 controls | Partial | Seven ADC branches connected; PF12 branch and capacitor ground remain incomplete; connector changed to 14-pin family with only pins 1–13 placed |
| 13 encoder | Partial | PF0/PF1/PF2 now reach the control connector; A/B have 10 kΩ pull-ups; switch has no pull-up and firmware uses `GPIO_NOPULL` |
| 27 guitar input impedance | **New P0 regression** | Input bias resistor changed from 1 MΩ to 10 kΩ; restore the high-impedance guitar input |

Items 07–26 remain open except the specific partial improvements above. Correct auxiliary summing does not close its level/protection or capacitor-polarity requirements (09–10).

**Architecture clarification, 6 October:** the LM1875 power amplifier will be an existing external board reused from the user's previous project. Its absence from these schematic sheets is intentional. Item 12 covers integration with that board; amplifier circuit design, PCB layout and thermal design belong to the reused board.

## Current state

| Sheet | Already present | Main remaining work |
| --- | --- | --- |
| `microcontroller.SchDoc` | STM32H5E5ZJT6, debug/reset/BOOT0, HSE circuit, MCU supply decoupling, memory/codec labels, USB-C, control/encoder connector with RC filters, seven connected ADC branches and encoder A/B pull-ups | Eighth ADC branch, filter ground, switch bias/connector completion, display, LEDs, console connector, power definition and protection |
| `memory.SchDoc` | S70KL1282DPBHI020 HyperRAM, decoupling and reset/CS pull-ups; microSD socket, signal pull-ups and decoupling | Card-detect pull-up/validation, unused reset decision, exact memory requirements and layout constraints |
| `audio.SchDoc` | TLV320AIC3104, OPA2134 guitar preamp/buffer, gain/level trimmers, input/output headers, attached SDA/reset labels, resistor-isolated auxiliary mono summing, corrected output selection and 220 µF headphone coupling | Missing codec supply, guitar impedance regression, amplifier DC interface, coupling polarity and analog level/protection design |
| `bluetooth.SchDoc` | BM83SM1-00AB symbol | All electrical connections; optional population/clock strategy |

## P0: fix these first

- [ ] **01 — Connect codec DRVDD_1, pin 18.** `audio.SchDoc`: this pin has no wire or supply label; it supplies the analog ADC as well as output-driver circuitry. AVDD, IOVDD and DRVDD_2 already connect to +3V3; DVDD connects to +1.8. Connect DRVDD_1 to the intended analog 3.3 V supply and add its local decoupling. Check AVDD/DRVDD supply relationships and startup sequencing against the TI datasheet. **Done when:** both DRVDD pins are powered in the compiled netlist and every codec supply has the specified decoupling.

- [x] **02 — Reattach the codec SDA and reset labels — resolved in source.** `audio.SchDoc`: `I2C1_SDA` now has an exact local connection to SDA pin 9; `SA1_Reset` attaches to /RESET pin 31 and the existing reset RC. On the MCU sheet, the matching labels reach PB7 pin 137 and PE0 pin 141. The detached-label fault is corrected. Native compiled cross-sheet proof remains under 08; reset capacitor value/MPN consistency remains under 24.

- [ ] **03 — Finalize headphone and amplifier output interfaces — routing corrected, interface partial.** `head_out` now comes from HPROUT pin 23 and HPLOUT pin 19 through 220 µF capacitors. HPRCOM/HPLCOM are unconnected, matching the current disabled COM profile. `amp_out` now comes directly from RIGHT_LOP pin 29 and LEFT_LOP pin 27; RIGHT/LEFT_LOM remain unused. This fixes the previous output selection. The LM1875 is an existing external board reused from the previous project. The line-output connector has no DC-blocking capacitors on this sheet, so check the reused board's input circuit: if it already provides suitable AC coupling, use that; otherwise provide coupling in the interconnect or on this PCB. The reused board's input coupling has not been inspected in this review. Record the single-ended line-output configuration, level and impedance. **Current pinout:** `head_out` 1=R, 2=L, 3=DGND; `amp_out` 1=R_LOP, 2=L_LOP, 3=DGND. **Done when:** those names appear on the schematic and the completed amplifier interface handles codec output common mode and startup transients. Never tie output drivers together.

- [ ] **04 — Finalize headphone capacitor and load specification — values corrected, selection partial.** Both series capacitors now read **220 µF**, with positive terminals toward HPLOUT/HPROUT. The previous 2.2 µF bass-loss issue is corrected for a 32 Ω baseline: `fc = 1/(2πRC)` is approximately **22.6 Hz at 32 Ω** and **45.2 Hz at 16 Ω**, ignoring driver impedance and capacitor tolerance. Specify the minimum headphone impedance and acceptable bass response; if approximately 20 Hz cutoff at 16 Ω is required, 470 µF gives approximately 21.2 Hz. Finalize MPN, voltage rating, tolerance, leakage, effective capacitance and startup behavior. **Done when:** the supported minimum load and cutoff target are recorded and the chosen parts meet them.

- [x] **05 — Preserve independent guitar and mono auxiliary capture — resolved in source.** Guitar still reaches LINE1LP pin 10. `line_in` pin 2 and pin 3 now each pass through a separate **10 kΩ resistor and 2.2 µF capacitor** before joining at LINE1RP pin 12. LINE2L/LINE2R are unused; LINE1RM pin 13 now has a 100 nF AC connection to DGND, like LINE1LM. The isolation resistors prevent directly shorting the source L/R outputs, and the allocation now matches the firmware default LINE1/LINE1 inputs. For low-impedance source outputs and the datasheet nominal 20 kΩ input at 0 dB routing, the midband approximation is `Vaux ≈ 0.4 × (VL + VR)`; equal in-phase L/R gives about 0.8 times one channel, not unity gain. Final levels/protection and the reversed polarized input-capacitor orientation remain under 09–10.

- [ ] **06 — Finish the control/encoder connector — partial.** Seven 100 Ω / 100 nF branches now connect by matching local labels to PC0, PC1, PC2, PA0, PA1, PC4 and PF11. Connector pin **4** reaches its resistor/capacitor signal node but that node has no ADC label/wire; MCU **PF12 pin 50** is still unconnected. Connect them using `ADC1_INP6`, matching the IOC. The return node shared by **all eight 100 nF capacitors is still floating**, despite connector pin 13 separately reaching DGND: add an actual DGND connection to the capacitor return. The connector is now the `2514-` 14-pin family; visible pins 1–13 are unique, with **pin 14 not placed**. Add/document its treatment and annotate the multipart connector. Encoder A/B now reach PF0/PF1 with 10 kΩ pull-ups, and push reaches PF2, but PF2 has no bias resistor and `MX_GPIO_Init()` uses `GPIO_NOPULL`: define the switch idle state and edge. `TIM2_CH1` label is detached by 2 schematic coordinate units; the direct PF0-to-pin-1 wire is connected, so this is a label-cleanup issue rather than a broken A path. **Done when:** all eight ADC paths and their ground are complete, the switch has a defined idle state, and the physical connector pinout is complete.

- [ ] **27 — Restore the guitar input impedance — new regression.** `audio.SchDoc`: the shunt bias resistor from OPA2134 +IN_A pin 3 to DGND changed from **1 MΩ to 10 kΩ** (component UID `LIQTFEBO`, still `R?`). This makes the midband input approximately 10 kΩ and heavily loads a passive guitar pickup. With the existing 220 nF input capacitor, the ideal-source RC cutoff moves from approximately **0.72 Hz to 72.3 Hz**, before accounting for the pickup impedance. Restore **1 MΩ** for the agreed passive-guitar input unless a deliberate alternative high-impedance design replaces it. **Done when:** the input resistor value/BOM and specified guitar input impedance agree.

- [ ] **07 — Define how every power rail is supplied.** The project uses +3V3, +1.8, +12 and -12 power symbols, but the four sheets contain no regulator/power-input implementation that defines their origin. Choose an explicit external supply interface or add the required power circuits. Specify polarity, current budget, analog noise filtering, input protection and startup behavior. The reused LM1875 board retains its own power requirements; document its supply connection and signal-ground relationship to the DSP/preamp board. Define any shared supply arrangement explicitly. **Done when:** every rail has an identified source, a connector or regulator, ratings and a documented return path.

- [ ] **08 — Compile and prove cross-sheet connectivity.** The project has four standalone schematic documents; the structure file lists `audio.SchDoc` as top-level and the other three as `NoMainPathDocument`. There are no sheet-symbol/port records linking the sheets. Confirm the intended flat/global net scope or create an explicit hierarchy with ports. **Done when:** compiled nets demonstrably connect every OCTO, SD, SAI1, I2C1 and codec-reset signal between the correct physical pins. Matching visible label text alone is insufficient.

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

- [ ] **12 — Verify the interface to the reused LM1875 board.** The user confirmed that an already-built power-amplifier board from the previous project will be reused externally. Its absence from the DSP schematic is intentional. Record the board identification or schematic reference and check its input connector pinout, input impedance/sensitivity, existing input coupling capacitor, signal ground and supply arrangement. Select the required `amp_out` channel(s); if a mono input needs L/R summing, use a designed summing network rather than tying codec outputs together. Document any existing enable/mute provision if available. Amplifier gain/feedback, stability, speaker-load and thermal design remain part of the reused board. **Done when:** the connection from `amp_out` to that board is documented and compatible, including DC blocking already present on either board or added where needed.

- [ ] **13 — Complete controls mechanically and electrically.** Document the eight potentiometer values/tapers, endpoint supply and ground, cable pinout, connector keying and open-wiper behavior. A/B pull-ups are now present; finish encoder push bias and debounce, confirm contact-common wiring and selected interrupt edge. Add protection for controls connected by exposed cables. Check total source resistance and ADC sample time; the 100 Ω series resistor alone does not describe the source impedance of a potentiometer. Label gain/level trimmers separately from user-facing DSP controls. **Done when:** connector pinout, ADC-channel table and control functions agree.

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

- [ ] **25 — Run native Altium compilation/ERC and produce review exports.** Resolve unconnected required pins, unplaced connector pin 14, the detached `TIM2_CH1` label, unexpected one-pin nets, incompatible output drivers, power errors and annotation conflicts. Reconfirm source-level corrections 02/05 in the compiled project. Add no-connect markers only for intentionally unused pins. Export a compiled netlist, schematic PDF and BOM from this same revision. Review the schematic-to-PCB update preview without accepting unintended changes. **Done when:** remaining ERC findings are individually explained and the exports agree with the source files.

- [ ] **26 — Add bring-up access before routing.** Provide practical access to +3V3/+1.8/±12 V and grounds, NRST/BOOT0, codec reset/I2C/MCLK/BCLK/WCLK/DIN/DOUT, ADC input/output nodes, card detect and UART. For USB/HyperRAM high-speed nets, choose probe access that avoids long stubs. Record the board bring-up sequence: supply checks → SWD/UART/LEDs → codec control/clocks → DAC/ADC → controls/display → memory/storage → USB → optional BM83. Hardware verification remains pending until the board exists.

## Recommended work order

1. Connect codec DRVDD_1 and restore the 1 MΩ guitar input (01, 27).
2. Connect the filter return to DGND, finish PF12/ADC1_INP6, switch bias and connector pin 14 (06).
3. Define power sources and compile cross-sheet connectivity (07–08).
4. Verify the reused LM1875 board interface and existing input coupling; finish headphone load/part selection, input-capacitor polarity and analog headroom/protection (03–04, 09–12).
5. Complete display/LED/UART/USB and MCU/HSE/microSD/HyperRAM details (13–20).
6. Complete or defer Bluetooth; annotate, compile/ERC and export a netlist/PDF/BOM before routing (21–26).

## Re-review verification

- Parsed all four current native SchDoc files; compared changed electrical records against the 4 October baseline, reconstructing local wire/pin/label connections without a proximity tolerance.
- Checked connector-to-filter paths, seven matching ADC nets, the floating common capacitor node, PF12, encoder bias, codec supplies, SDA/reset, auxiliary summing, input bias value and both output headers.
- Compared the allocation with unchanged IOC and codec default profile. `memory.SchDoc` and `bluetooth.SchDoc` are byte-identical to the previous baseline; their open findings remain applicable.
- No native Altium compilation/ERC, compiled cross-sheet netlist, footprint audit or physical tests were run. No firmware build is required for this documentation-only update. The report is a schematic design review, not a board release sign-off.

## Source references

- Current project sources: [`layout/guitarDSP`](https://github.com/bogdan-tirzioru/guitarAmp/tree/1bf40d91ba2a38fb19274ef5c1492021644c28ca/layout/guitarDSP), [`firmwaredsp.ioc`](https://github.com/bogdan-tirzioru/guitarAmp/blob/1bf40d91ba2a38fb19274ef5c1492021644c28ca/firmwaredsp/firmwaredsp.ioc), [`README.md`](https://github.com/bogdan-tirzioru/guitarAmp/blob/1bf40d91ba2a38fb19274ef5c1492021644c28ca/README.md), [`docs/tlv320aic3104.md`](https://github.com/bogdan-tirzioru/guitarAmp/blob/1bf40d91ba2a38fb19274ef5c1492021644c28ca/docs/tlv320aic3104.md).
- [TI TLV320AIC3104 datasheet, SLAS510G](https://www.ti.com/lit/ds/symlink/tlv320aic3104.pdf): pin functions, supply limits, analog input levels, AC-coupled headphones, outputs and sequencing.
- [TI OPA2134 datasheet](https://www.ti.com/lit/ds/symlink/opa2134.pdf): supply, input/output limits and analog-stage checks.
- [ST STM32H5Exxx datasheet, DS14971](https://www.st.com/resource/en/datasheet/stm32h5e5zj.pdf): exact MCU pinout, power scheme and USB supply.
- `AN/ABM3B-8.000MHZ-10-D1G-T.pdf`: Abracon family datasheet/order-code explanation; confirm the full ordered part.
- BM83 and S70KL1282 exact-part datasheets should be added/pinned to the hardware reference set before closing their verification items.
