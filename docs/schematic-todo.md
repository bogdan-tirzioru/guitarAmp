# guitarAmp schematic to-do list

Re-reviewed: 11 October 2026. Previous detailed review: 10 October 2026. Repository: [bogdan-tirzioru/guitarAmp](https://github.com/bogdan-tirzioru/guitarAmp).

Current schematic review baseline: `main`, commit [`a92aa2296f59b654e6f1917f9ccb693b2685753c`](https://github.com/bogdan-tirzioru/guitarAmp/tree/a92aa2296f59b654e6f1917f9ccb693b2685753c), titled `adnotation of schematic` (11 October 2026). This review includes the preceding `9aff26e` power-delivery implementation and `3b8f99d` Bluetooth power correction. All five current `.SchDoc` files, their committed previews, and the project/structure files were inspected; firmware configuration is unchanged from the earlier reviewed baseline.

## Review scope and limits

Reviewed the native Altium compound-file records for components, active multipart pins, wires, junctions, net labels, power symbols, designators and parameters. Local connectivity was traced using pin endpoints and full coordinates, including fractional coordinates, and compared with committed sheet previews. This verifies the source drawing more precisely than the small previews alone. It is not an Altium compiled netlist: project-wide net scope, footprint pad mapping, ERC and hardware measurements remain pending.

Confirmed new corrections: BM83 VDD_IO is disconnected; EP1/EP2 are connected to DGND; the missing BM83 UART label is present; encoder push has R54 = 10 kΩ pull-up; TIM2_CH1 is attached; display +5 V/+3V3/ground connections are present; physical components are annotated; and amplifier outputs now use separate two-pin headers. The new power tree is drawn and locally connected. Codec DRVDD_1 is still open.

A status marked resolved means the source shows the correction; it does not prove compiled cross-sheet connectivity or hardware performance.

Priority: **P0** = blocking circuit/connectivity fixes; **P1** = complete before schematic freeze; **P2** = optional feature work or release checks.

## Current review summary

| Area | Current source status | Remaining action |
| --- | --- | --- |
| Codec power | IC3 DRVDD_1 pin 18 remains open; the other supply pins retain their intended rails | Connect pin 18 and local decoupling; verify sequencing |
| Guitar input | R33 = 1 MΩ, C40 = 220 nF; independent ADC-left path retained | Finish protection, gain/headroom and BOM ratings |
| Auxiliary input | R35/R36 = 10 kΩ sum stereo to mono ADC right; C39/C45/C46 are non-polar 2.2 µF parts | Specify effective capacitance/ratings and overload protection |
| Controls | All eight ADC paths and capacitor returns are connected; TIM2_CH1 attaches to PF0; R54 now biases PF2 high | Define debounce/contact wiring and open X2 pin 14; prove compiled mapping |
| Storage | Detect pull-up and existing HyperRAM wiring retained; parts annotated | Confirm detect polarity, SD_RESET role and exact-part layout requirements |
| LCD and console | X3 pin 14 = +5 V, pin 13 = +3V3, pins 2/4/11 = DGND; P1 console pinout traced | Verify actual module adapter; document UART directions and unused X3 pin 12 |
| USB | Self-powered interface and shield network retained | Finish HS ESD, VBUS detection/backfeed and chassis bonding |
| Power tree | X12 ±12 V/DGND input → IC6 LM7805 +5 V → U1 KF33 +3V3 and IC7 LM317 +1.8-labelled rail | Fix C50 BOM/value ambiguity; finish bypass/bulk, minimum load, current and thermal budgets |
| Bluetooth | BAT_IN = +3V3; VDD_IO and SYS_PWR open; ground pads now grounded; UART labels complete locally | Prove BAT_IN stays ≥3.2 V under tolerance/load; finish bulk/reset supervision, configuration and clocks |
| Annotation/BOM | No `?` designators remain in the five source sheets | Resolve value/MPN inconsistencies, multipart grouping and footprint/connector metadata |

The external LM1875 board is intentionally reused; its amplifier circuit is not part of the guitarAmp schematic. The guitarAmp board must still document the shared ±12 V input and audio-ground connection.

## Current sheet state

| Sheet | Present in the current design | Main remaining work |
| --- | --- | --- |
| `microcontroller.SchDoc` | STM32H5E5ZJT6, SWD/reset/BOOT0, HSE, MCU decoupling, USB-C/shield, eight ADC controls, three LED drivers, UART and powered LCD/touch connector; encoder A/B/push pull-ups and TIM2_CH1 attached | X2 pin 14/X3 pin 12 treatment, external control wiring/debounce, USB protection, module adapter verification and compiled connectivity |
| `memory.SchDoc` | Annotated HyperRAM and microSD circuits with detect pull-up | Detect polarity, SD_RESET role, exact-part verification and routing constraints |
| `audio.SchDoc` | Annotated TLV320AIC3104/OPA2134, high-impedance guitar input, independent guitar/aux capture, non-polar coupling, 220 µF headphone coupling; P2 Amp R and P3 Amp L | DRVDD_1 remains open; overload/protection, capacitor BOM and headroom |
| `bluetooth.SchDoc` | Annotated BM83/BC847 reset/MFB circuits; BAT_IN on +3V3; VDD_IO/SYS_PWR open; GND and EP1/EP2 grounded; I²S/UART labels attached | BAT_IN tolerance/transient margin, bulk/reset supervision, clock ownership and unused pins/configuration |
| `powersupply.SchDoc` | X12 (+12/DGND/-12), IC6 LM7805, U1 KF33, IC7 LM317, R42/R43 divider, C49–C52 and TP1 on +3V3 | Capacitor values/MPNs/ESR, entry bypass/protection, regulator load/thermal limits, tolerance and startup validation |

## P0: fix these first

- [ ] **01 — Connect codec DRVDD_1, pin 18.** `audio.SchDoc`: this pin has no wire or supply label; it supplies the analog ADC as well as output-driver circuitry. AVDD, IOVDD and DRVDD_2 already connect to +3V3; DVDD connects to +1.8. Connect DRVDD_1 to the intended analog 3.3 V supply and add its local decoupling. Check AVDD/DRVDD supply relationships and startup sequencing against the TI datasheet. **Done when:** both DRVDD pins are powered in the compiled netlist and every codec supply has the specified decoupling.

- [x] **02 — Reattach the codec SDA and reset labels — resolved in source.** `audio.SchDoc`: `I2C1_SDA` now has an exact local connection to SDA pin 9; `SA1_Reset` attaches to /RESET pin 31 and the existing reset RC. On the MCU sheet, the matching labels reach PB7 pin 137 and PE0 pin 141. The detached-label fault is corrected. Native compiled cross-sheet proof remains under 08; reset capacitor value/MPN consistency remains under 24.

- [ ] **03 — Finalize headphone and amplifier output interfaces — routing corrected, interface partial.** `head_out` now comes from HPROUT pin 23 and HPLOUT pin 19 through 220 µF capacitors. HPRCOM/HPLCOM are unconnected, matching the current disabled COM profile. `P2` (`Amp R`) pin 2 now comes directly from RIGHT_LOP pin 29 and `P3` (`Amp L`) pin 2 from LEFT_LOP pin 27; RIGHT/LEFT_LOM remain unused. This fixes the previous output selection. The LM1875 is an existing external board reused from the previous project. The reused board's `PowerAmp.SchDoc` now confirms **C1 = 2.2 µF** between input connector P2 pin 2 and LM1875 +IN, with its positive terminal toward P2. This supplies the required DC blocking for the codec's positive output common mode; a second series capacitor on the DSP board is not required for this documented path. Record the chosen single codec output and ground connection as shown below, and confirm the fitted board matches this schematic. Record the single-ended line-output configuration, level and impedance. **Current pinout:** P4 `head_out` 1=R, 2=L, 3=DGND; P2 `Amp R` 1=DGND, 2=RIGHT_LOP; P3 `Amp L` 1=DGND, 2=LEFT_LOP. Both amplifier headers are separate two-pin outputs; annotate the chosen mono connection. **Done when:** those names appear on the schematic and the completed amplifier interface handles codec output common mode and startup transients. Never tie output drivers together.

- [ ] **04 — Finalize headphone capacitor and load specification — values corrected, selection partial.** Both series capacitors now read **220 µF**, with positive terminals toward HPLOUT/HPROUT. The previous 2.2 µF bass-loss issue is corrected for a 32 Ω baseline: `fc = 1/(2πRC)` is approximately **22.6 Hz at 32 Ω** and **45.2 Hz at 16 Ω**, ignoring driver impedance and capacitor tolerance. Specify the minimum headphone impedance and acceptable bass response; if approximately 20 Hz cutoff at 16 Ω is required, 470 µF gives approximately 21.2 Hz. Finalize MPN, voltage rating, tolerance, leakage, effective capacitance and startup behavior. **Done when:** the supported minimum load and cutoff target are recorded and the chosen parts meet them.

- [x] **05 — Preserve independent guitar and mono auxiliary capture — resolved in source.** Guitar still reaches LINE1LP pin 10. `line_in` pin 2 and pin 3 now each pass through a separate **10 kΩ resistor and 2.2 µF capacitor** before joining at LINE1RP pin 12. LINE2L/LINE2R are unused; LINE1RM pin 13 now has a 100 nF AC connection to DGND, like LINE1LM. The isolation resistors prevent directly shorting the source L/R outputs, and the allocation now matches the firmware default LINE1/LINE1 inputs. For low-impedance source outputs and the datasheet nominal 20 kΩ input at 0 dB routing, the midband approximation is `Vaux ≈ 0.4 × (VL + VR)`; equal in-phase L/R gives about 0.8 times one channel, not unity gain. Final levels/protection remain under 09–10; the three 2.2 µF coupling capacitors have been replaced with non-polar SMD parts, with component/BOM and frequency-response checks remaining under 10.

- [ ] **06 — Complete the control connector — connectivity/bias fixes resolved in source.** X2 has all eight analog paths and their shared capacitor DGND return connected. PF0/encoder A now carries the attached `TIM2_CH1` label; A/B pull-ups are R55/R56 = 10 kΩ. PF2/encoder push now has R54 = 10 kΩ to +3V3, so its idle bias is no longer missing. X2 pin 14 remains open. Document external switch common, active-low contact action and debounce. **Done when:** compiled nets confirm all eight ADC paths and returns, pin 14 has intentional treatment, and the external controls/debounce are documented.

- [x] **27 — Restore the guitar input impedance — resolved in the current source.** The latest reviewed audio sheet shows the agreed **1 MΩ** bias resistor at the OPA2134 input, restoring the intended high-impedance passive-guitar input. Confirm the annotated value and part number during BOM cleanup.

- [ ] **07 — Validate the implemented power entry and regulator tree.** `powersupply.SchDoc` is now present. Source tracing confirms **X12 pin 1 = +12, pin 2 = DGND, pin 3 = -12**. IC6 LM7805CT/NOPB takes +12 V and creates +5 V; U1 KF33 takes +5 V and creates +3V3; IC7 LM317 takes +5 V and creates the +1.8-labelled rail. The negative rail is passed through for the analog stage; it is not generated here. U1 ground pins 2/3/6/7 are grounded, pin 4 NC is open, and pin 5 INHIBIT is grounded: the KFXX test circuit identifies low as ON, so this connection is intentional. TP1 reaches +3V3. USB does not feed this tree. Close the detailed power checks below, especially C50 part selection, input bypassing, current/thermal margins and minimum-load behavior. **Done when:** source tree, BOM, worst-case input/load/temperature and startup behavior agree.

- [ ] **08 — Compile and prove cross-sheet connectivity.** The project now includes **five** standalone schematic sheets, including `powersupply.SchDoc`; its structure still lists audio as top-level and the remaining sheets as `NoMainPathDocument`. No sheet-symbol/port records link them. Local pin-to-wire/label attachments were checked from native records, including codec SDA/reset and all BM83 signal labels. Confirm the intended flat/global net scope or create an explicit hierarchy. **Done when:** the compiled netlist proves all shared power rails and OCTO, SD, SAI1/SAI2, I2C1, codec-reset, BM83-reset/MFB and UART nets between physical pins.

- [ ] **28 — BM83 supply/ground corrections — resolved wiring, supply margin still open.** IC5 BAT_IN pin 23 alone connects to +3V3; VDD_IO pin 25 is now open, SYS_PWR pin 24 remains open, and EP1/EP2 both connect to DGND. These remove the earlier VDD_IO and ground-pad faults. Microchip specifies BAT_IN **3.2–4.2 V**: nominal 3.3 V is inside this range and is not inherently invalid. However, U1 KF33's published 3.3 V tolerance reaches **3.168 V** over its stated -25 to +85 °C test range, below BAT_IN minimum even before wiring/transient drops. Guarantee ≥3.2 V at the module under the actual operating range/load, or select a supply with sufficient margin (a dedicated approximately 3.8 V rail is one option). Do not feed BAT_IN with +5 V or use SYS_PWR/VDD_IO as board rails. C47/C48 are 100 nF rail decouplers; review module-local bulk capacitance and reset supervision. **Done when:** the corrected wiring and guaranteed BAT_IN range are proven in the compiled design and power budget.

### Detailed power-stage review

| Node / parts | Verified source connection | Required closure |
| --- | --- | --- |
| X12 power input | 1=+12, 2=DGND, 3=-12 | Specify whether these are regulated DC or rectified transformer rails, min/max including ripple/no-load, current rating, fuse/polarity protection and wiring returns. ±15 V may be possible but needs the full rail/component and heat audit; a transformer secondary RMS rating is not the same as the rectified DC voltage. |
| IC6 LM7805CT/NOPB | IN=+12, GND=DGND, OUT=+5V | No local input bypass/bulk capacitor is drawn on +12 V. Provide bypass appropriate to the regulator's distance from the upstream reservoir and cable inductance; add/size +5 V bulk for downstream transient loads. Verify heatsink and aggregate current including LCD, both downstream regulators and their quiescent currents. |
| U1 KF33 | VIN=+5V, VOUT=+3V3; INHIBIT low (ON), all GND pins grounded; NC open | 500 mA nominal output capability must cover MCU, HyperRAM, SD, codec/headphones, BM83 and any display load with transient/thermal margin. Its 1 A current-limit figure is not a 1 A continuous operating rating. Confirm exact SO-8 MPN and copper cooling. |
| C49/C51/C50 | C49=100 nF at U1 input; C51=100 nF and C50 labelled 2.2 µF at +3V3 | KFXX specifies output capacitance 2–10 µF and ESR 0.1–10 Ω. Select C50 to meet capacitance/ESR over tolerance/temperature; nominal 2.2 µF ±20% alone can fall below 2 µF. Resolve the conflicting 0.47 µF MPN/library alternatives. Check aggregate local rail capacitance/stability. |
| IC7/R42/R43 | VIN=+5V; R42=240 Ω from OUT to ADJ; R43=100 Ω ADJ to DGND | `Vout = 1.25 × (1 + 100/240) + Iadj × 100`: approximately **1.776 V**, or **1.781 V** at 50 µA Iadj. This is close to nominal 1.8 V, not an exact setting. Approximately 105 Ω for R43 gives 1.8 V nominal before Iadj. Evaluate full reference/resistor tolerance against codec DVDD limits before selecting parts. |
| LM317 light-load behavior | Divider draws about 5.2 mA; no separate output preload shown | The generic `317` selection is not enough to guarantee minimum-load regulation: specify exact device and check startup/reset/power-down codec load against its datasheet minimum. Change divider/preload or regulator if required; do not rely solely on typical running codec current. |
| C52 / 1.8 V decoupling | C52 (`0.1`) is on IC7 **input**, not output; codec C35=100 nF remains on DVDD | State C52 units. Add/size local output bulk for transient response as needed; a missing output capacitor is not automatically an LM317 stability fault. Check startup sequencing and input-collapse/reverse-current protection with the selected capacitors. |
| R41 / TP1 | R41=10 kΩ loads +5V (~0.5 mA); TP1 reaches +3V3 | Document R41 purpose; it does not satisfy an LM317 output preload requirement. Add accessible +5V/+1.8/input/ground measurement points as needed. |

First-order dissipation, excluding regulator quiescent current: **LM7805: (Vin−5) × I5_total**; KF33: **1.7 × I3V3**; LM317: **(5−V1V8) × I1V8_total**. At 0.5 A total +5 V current, LM7805 dissipates **3.5 W from +12 V** or **5 W from +15 V**; these are examples, not measured/approved board budgets. At 0.5 A +3V3 output, KF33 dissipates about **0.85 W** before ground current. Determine worst-case currents, enclosure ambient and thermal paths before accepting the linear tree. If excessive, replace the +12→+5 stage with a suitable buck while preserving clean analog distribution.

### Reused power-amplifier connection

The power-amplifier schematic already contains the required input coupling. Example mapping using the **left** codec channel; the right channel can be chosen instead. Confirm physical connector pin numbering on the built boards.

| DSP board | Reused power-amplifier board | Function |
| --- | --- | --- |
| DSP P3 `Amp L` pin 2, LEFT_LOP | Amplifier P2 pin 2 | Mono audio signal through existing C1 |
| DSP P3 `Amp L` pin 1, DGND | Amplifier P2 pin 1, GND | Signal return |
| DSP P2 `Amp R` pin 2, RIGHT_LOP | Unconnected in this example | Second line-output channel |

Produce the guitar/auxiliary mono mix in STM32 and send it to the selected DAC channel. Never wire LEFT_LOP and RIGHT_LOP together. On the reused amplifier, P1 is the supply connector and P3 is the speaker connector; these designators are independent of the DSP board headers. Keep speaker-current return wiring out of the codec signal-return path. The source-level check confirms P2/C1/R3/feedback/supply connectivity; the existing assembly, PCB correspondence, actual supply voltage and performance have not been validated here.

### Current control connector pinout

The `2514-` multipart control connector is now annotated `X2`, parts A through N; verify the physical footprint numbering and compiled pin-to-net mapping before layout.

| Connector pin | Function / intended MCU pin | Source-level status |
| --- | --- | --- |
| 1 | Encoder A → PF0, MCU pin 10 | Connected directly; R55 = 10 kΩ pull-up; `TIM2_CH1` label attached |
| 2 | Encoder B → PF1, MCU pin 11 | Connected; `TIM2_CH2` label and 10 kΩ pull-up present |
| 3 | Encoder push → PF2, MCU pin 12 | Connected; `ENC_EXTI2`; R54 = 10 kΩ pull-up now present; document debounce |
| 4 | Analog control → PF12, MCU pin 50, ADC1_INP6 | Connected in the latest committed sheet; compiled-net verification pending |
| 5 | Analog control → PC0, MCU pin 26, ADC1_INP10 | Signal connected |
| 6 | Analog control → PC1, MCU pin 27, ADC1_INP11 | Signal connected |
| 7 | Analog control → PC2, MCU pin 28, ADC1_INP12 | Signal connected |
| 8 | Analog control → PA0, MCU pin 34, ADC1_INP0 | Signal connected |
| 9 | Analog control → PA1, MCU pin 35, ADC1_INP1 | Signal connected |
| 10 | Analog control → PC4, MCU pin 44, ADC1_INP4 | Signal connected |
| 11 | Analog control → PF11, MCU pin 49, ADC1_INP2 | Signal connected |
| 12 | +3V3 | Connected |
| 13 | DGND | Connected |
| 14 | Unassigned | X2N pin is placed but left open; assign a function or document/mark it intentionally unused |

The latest committed sheet shows all eight analog-filter capacitor returns connected to DGND. Assign user-facing control names and document external pot/encoder wiring; compiled-net verification remains under 08.

### Other MCU interface status

| Function | CubeMX assignment | Current schematic state and remaining check |
| --- | --- | --- |
| Status LEDs | PC13 `LED0`; PC14 `LED1`; PC15 `LED2` | Source tracing confirms active-high BC847 sinks: LED0 → T1/D1, LED1 → T3/D3, LED2 → T2/D2; 1 kΩ base and 100 Ω LED resistors. Specify LED current/MPNs and reset state. |
| Serial console | PA2 USART2_TX / PA3 USART2_RX | P1: 1=DGND, 2=PA3 MCU RX, 3=PA2 MCU TX; no supply pin. Document adapter-view `log_tx`/`log_rx` names and 3.3 V logic. |
| Display SPI | PB13 SCK / PB15 MOSI / PB14 MISO | X3: 1=SCK, 3=MISO, 5=MOSI; 14=+5 V, 13=+3V3, 2/4/11=DGND. Verify actual module adapter/current and shared MISO; pin 12 open. |
| Display controls | PB12 LCD_CS; PE14 LCD_RS; PE15 LCD_RESET | Source mapping confirmed: X3 6=LCD_CS, 9=LCD_RS, 8=LCD_RESET. Verify inactive CS and reset state. |
| Touch controls | PB10 TOUCH_CS; PE13 TOUCH_IRQ | Source mapping confirmed: X3 7=TOUCH_CS, 10=TOUCH_IRQ. Verify IRQ bias/polarity and CS/MISO behavior. |

## P1: complete before schematic freeze

- [ ] **09 — Finalize guitar/auxiliary gain and codec headroom.** The restored R33 = 1 MΩ bias resistor is retained (27). The OPA2134 A non-inverting stage retains a 2 kΩ ground resistor and 0–10 kΩ feedback resistance: nominal gain 1–6. The second 10 kΩ trimmer attenuates its output before the B-channel follower. Record expected passive/active pickup and auxiliary source levels, trimmer range, summing attenuation and maximum ADC level, including the ±12 V op-amp output during overload. The codec nominal 0 dB full-scale input is 0.707 Vrms at DRVDD1 = 3.3 V; AC coupling does not limit excessive AC excursions. **Done when:** gain/attenuation and any series protection or limiting keep both inputs within intended operating and absolute-maximum limits.

- [ ] **10 — Finalize audio coupling components and input protection.** Choose explicit voltage ratings and MPNs for the 220 nF guitar input capacitor and the three 2.2 µF non-polar SMD preamp/auxiliary coupling capacitors. The three 2.2 µF parts no longer have a polarity-orientation issue. Verify their effective capacitance, tolerance and low-frequency response with the selected input impedance/routing and source/summing resistances. The headphone capacitors retain their required positive-toward-codec orientation. Add suitable low-leakage guitar-jack ESD/RF protection and line-input protection. **Done when:** coupling-capacitor values/parts and overload/ESD design are documented.

- [ ] **11 — Finish codec analog support and startup behavior.** Review LINE1LM and the newly added LINE1RM 100 nF connections to ground and define every unused input/output disposition for the selected single-ended profile. Verify exposed-pad ground, local bulk/ceramic decoupling, quiet analog rail distribution, MCLK source and optional signal damping. Keep the existing STM32 SAI1 master-clock baseline unless deliberately changed. Define mute/ramp and jack-insertion behavior; add headphone detect or amplifier mute control if required. **Done when:** all codec pins have an intentional function or documented unused treatment and startup has a defined safe output state.

- [ ] **12 — Finalize wiring to the reused LM1875 board — schematic interface verified.** Reference: [`guitaramp/PCB_Project/PowerAmp.SchDoc`](https://github.com/bogdan-tirzioru/amplifier/blob/b777c9ec8a359d5651b260d3cbb7d02f36bd271f/guitaramp/PCB_Project/PowerAmp.SchDoc). This is one mono LM1875 channel with **P2 pin 2 = audio input, P2 pin 1 = GND**. **C1 = 2.2 µF** provides input AC coupling, positive toward P2; **R3 = 22 kΩ** biases the amplifier side to ground. With a low-impedance codec source, the input coupling cutoff is approximately **3.3 Hz**. **R2 = 1 MΩ** is an additional shunt on the connector side of C1, not the amplifier's audio-band input impedance; the codec sees approximately `22 kΩ || 1 MΩ ≈ 21.5 kΩ` in the audio midband. **R1 = 20 kΩ**, **R5 = 1 kΩ** give audio-band non-inverting gain **1 + 20k/1k = 21** (26.4 dB), with C6 = 22 µF in the feedback ground leg. **P1 = +12 / GND / -12 on pins 1/2/3** as labelled in the schematic. **P3 pins 1/2 = speaker output / GND**. No enable/mute control is present in this circuit. Use codec mute/ramp for startup and start with the existing -12 dB DAC setting; set the final level from actual supply rails, load and measured unclipped output. Do not infer 20 W output from the part name at the schematic ±12 V supply. Send the mono DSP mix to one selected DAC/line-output channel and wire it directly to P2, bypassing the old analog preamp. **Done when:** the fitted board/connector orientation and actual supply match the reference, the selected mono channel/interconnect are recorded on the DSP schematic, and startup/level behavior is checked on hardware.

- [ ] **13 — Complete controls mechanically and electrically.** Document the eight potentiometer values/tapers, endpoint supply and ground, cable pinout, connector keying and open-wiper behavior. A/B pull-ups are now present; encoder push now also has R54 = 10 kΩ pull-up; finish debounce and confirm contact-common wiring and selected interrupt edge. Add protection for controls connected by exposed cables. Check total source resistance and ADC sample time; the 100 Ω series resistor alone does not describe the source impedance of a potentiometer. Label gain/level trimmers separately from user-facing DSP controls. **Done when:** connector pinout, ADC-channel table and control functions agree.

- [ ] **14 — Verify the powered LCD/touch interface against the actual module.** X3 now provides **pin 14 = +5 V, pin 13 = +3V3, pins 2/4/11 = DGND**. Source mapping: 1=PB13 SCK, 3=PB14 MISO, 5=PB15 MOSI, 6=PB12 LCD_CS, 7=PB10 TOUCH_CS, 8=PE15 LCD_RESET, 9=PE14 LCD_RS, 10=PE13 TOUCH_IRQ; pin 12 is open. This closes the missing-supply drawing concern. It is a custom connector: verify the adapter against the actual Waveshare 3.5inch RPi LCD (A) revision, current budget, inactive CS levels, touch IRQ bias and shared-MISO release. Several power/unused connector parts still carry the copied comment `LCD_CS`; rename them to their actual function. **Done when:** the adapter pinout, rail/current budget, signal states and unused pin 12 treatment are documented.

- [ ] **15 — Verify status LEDs and service access — source mapping checked.** LED0/1/2 labels attach to MCU PC13/PC14/PC15 respectively and drive T1/T3/T2 BC847 bases through R48/R50/R49 = 1 kΩ. Emitters are grounded; collectors sink D1/D3/D2 cathodes; anodes connect to +3V3 through R44/R46/R45 = 100 Ω. This is active-high drive. Specify LED MPN/forward voltage and resulting current; add defined base-off bias if needed at reset. P1 `serial_log`: **pin 1 = DGND, pin 2 = PA3 MCU RX, pin 3 = PA2 MCU TX**. Its `log_tx`/`log_rx` labels are named from the external adapter perspective; document that convention or rename for clarity. It has no supply pin. **Done when:** LED current/startup behavior and service pinouts are explicit and agree with firmware.

- [ ] **16 — Finish USB protection and verify data-only VBUS behavior.** The design is self-powered; USB VBUS is for detection and must not feed the board rails. D+/D− reach the dedicated HS PHY pins, Type-C CC pull-downs are present, and the shield connection has been updated. Add suitable low-capacitance HS ESD protection; verify the PA9 VBUS-divider threshold/leakage and behavior when the MCU is unpowered; document how connector shield, chassis and DGND are bonded. **Done when:** there is no USB-to-board power path and the protection, detection and shield scheme are explicit.

- [ ] **17 — Verify MCU supplies using the correct H5E5 documentation.** MCU VDD, VDDA, VREF+, VDDIO2 and VDDUSB connect to +3V3; VBAT also uses +3V3. VCAP pins 70/142 and VDD11USB pin 95 share the `VDD11USB` net, with two 2.2 µF capacitors. The VDD11USB-to-VCAP connection is supported for USB HS use; do not treat it as a missing external 1.1 V regulator. Verify the complete LDO-mode circuit, capacitor effective values/ESR and firmware supply configuration. The checked-in `AN/DS_stm32h5f5zj.pdf` targets the H5F family; add the exact H5E5 reference and audit the symbol/footprint pin numbers. **Done when:** a supply/pinout check against STM32H5E5ZJT6 is recorded.

- [ ] **18 — Finalize the 8 MHz HSE implementation.** Q1 is labelled only `ABM3B`; C8/C9 are 15 pF, with a 1 MΩ parallel resistor. Record the exact `ABM3B-8.000MHZ-10-D1G-T` selection if that is the intended part. For its 10 pF load, equal 15 pF capacitors provide 7.5 pF plus parasitic load: 15 pF can be plausible, but calculate with MCU/PCB parasitics, check ESR/startup margin/drive level and the need for the external resistor. Use C0G/NP0 load capacitors. **Done when:** frequency, full part number, load calculation and oscillator checks are documented.

- [ ] **19 — Close microSD detect and reset handling.** A pull-up is now present on `SD_CARD_DET`/PD0. Confirm the socket contact's insertion polarity and the resulting idle/inserted levels. PD1's `SD_RESET` label does not reach a card reset pin; define PD1 as intentionally unused or redesign it as a controlled card-power switch. **Done when:** card-detect behavior is documented and PD1's role is unambiguous.

- [ ] **20 — Close HyperRAM electrical and layout requirements.** The eight DQ nets, RWDS, CK, CS# and RESET# map correctly to the present OCTOSPI assignments; VCC/VCCQ are +3V3, and CS#/RESET# have 10 kΩ pull-ups. Confirm the exact S70KL1282DPBHI020 package, 128 Mbit/16 MiB capacity, 3 V variant, CK# treatment, RFU-pin rules and dual-die latency requirements against the exact part datasheet. Record the initial bus frequency, optional series resistor positions, impedance/length constraints and decoupling placement. **Done when:** the complete bus and its operating assumptions are documented before routing.

## P2: optional Bluetooth and release checks

- [ ] **21 — Decide whether Bluetooth is included in revision 1.** The BM83 sheet now has partial circuitry, so it is no longer only a symbol. Decide whether to retain it for revision 1 or mark the module and all related components DNP. If retained, close item 28 and item 22, decide the I²S clock relationship in item 23, and resolve cross-sheet compilation before layout.

- [ ] **22 — Complete the BM83 circuit if Bluetooth is included.** The current source implements the intended BC847 open-collector reset topology: PD10 through 4.7 kΩ to the base, 100 kΩ base-emitter pull-down, emitter to DGND and collector to RST_N pin 43. PD10 high asserts reset; low/high-impedance releases it. MFB pin 26 is driven from PD14 through 1 kΩ, with a 100 kΩ pull-down on the module side; high wakes/keeps the module active and low requests power-save. Firmware should assert MFB high 2–3 ms before UART commands; the host guide also shows about 500 ms high during its reset/startup sequence. The I²S and UART pins now have local net labels for DT1, RFS1, SCLK1, UART_RXD, UART_TXD and UART_TX_IND; prove their MCU connections in the compiled design. BAT_IN/ground wiring is corrected as described in 28; finish supply margin/bulk decoupling and reset supervision, straps/configuration and UART programming access, CTS/RTS treatment, and no-connects for intentionally unused pins. Microchip recommends reset supervision when BAT_IN is externally supplied; check its RST_N threshold/timing against SYS_PWR fall. Keep MCLK1 separate from the codec MCLK net and document antenna clearance/edge placement. **Done when:** every BM83 pin has a verified connection or intentional no-connect and its power/reset behavior matches the selected host-mode configuration.

- [ ] **23 — If included, settle BM83 clock ownership.** The current IOC sets SAI2 as master. Document whether BM83 supplies BCLK/FS with STM32 slave reception and asynchronous rate conversion, or whether a verified BM83 client-mode configuration can follow STM32's 48 kHz clock. Avoid two drivers on the same clock net. **Done when:** schematic signal directions and the IOC agree with the chosen module configuration.

- [ ] **24 — Annotation completed; correct remaining component metadata.** No `?` designators remain in any of the five native source sheets; intentional multipart connector instances share X1/X2/X3/X12. Verify grouping/uniqueness in Altium rather than counting those instances as duplicate physical parts. C33 reset still has `Value=10nF` but `Manufacturer_Part_Number=CC0402KRX7R7BB104` (100 nF). C50 displays 2.2 µF but its library parameters include multiple MPN alternatives and end with `EEE-HC1HR47R`, a 0.47 µF code; select one actual part consistent with the required KF33 output capacitance and ESR. C52 displays `0.1` without units: explicitly state 100 nF if intended. Replace generic headers and copied X3 power-pin comments with real connector/function metadata; record full regulator MPNs, footprints, ratings and tolerances. **Done when:** the BOM/export has one unambiguous intended physical part and value per designator.

- [ ] **25 — Run native Altium compilation/ERC and produce review exports.** Check all required pins and cross-sheet labels, connector pin mapping, `TIM2_CH1` attachment, BM83 reset/MFB polarity and supply nets, ground-pad mapping, intentional BM83 no-connects, one-pin nets, incompatible drivers, power errors and annotation conflicts, including grouped multipart connectors and the new powersupply sheet. Reconfirm corrected SDA/reset, auxiliary-summing, controls, LCD, UART, USB and BT signals in the compiled project. Export the compiled netlist, schematic PDF and BOM from the same revision; inspect schematic-to-PCB update preview. **Done when:** ERC findings are explained and the exports agree with the source files.

- [ ] **26 — Add bring-up access before routing.** Provide practical access to +5 V, +3V3, +1.8/±12 V and grounds, NRST/BOOT0, codec reset/I2C/MCLK/BCLK/WCLK/DIN/DOUT, ADC input/output nodes, card detect and UART. For USB/HyperRAM high-speed nets, choose probe access that avoids long stubs. Record the board bring-up sequence: supply checks → SWD/UART/LEDs → codec control/clocks → DAC/ADC → controls/display → memory/storage → USB → optional BM83. Hardware verification remains pending until the board exists.

## Recommended work order

1. Connect codec DRVDD_1 (01); resolve C50 selection/stability and power-stage current/thermal/light-load checks (07, 24).
2. Close BM83 BAT_IN margin and reset/bulk support if populated (28, 22).
3. Compile the five-sheet project and prove shared power/signal nets (08, 25).
4. Finish analog headroom/protection, headphone load and mono wiring to the reused amplifier (03–04, 09–12).
5. Document control debounce/cables and unused X2/X3 pins; finish display adapter, USB, MCU/HSE, SD_RESET and HyperRAM constraints (06, 13–20).
6. Settle Bluetooth clock/configuration, clean the BOM, export netlist/PDF/BOM and review the PCB update before routing (21–26).

## Re-review verification

- Reviewed all five native `.SchDoc` sources and previews at `a92aa22`, plus project and structure files. Extracted active multipart pins and used full fractional coordinates for local pin/wire/net-label tracing. This is a source audit, not native Altium compilation.
- Verified power connector and all three regulator signal/ground paths; LM317 divider orientation/calculation; C52 input placement; U1 INHIBIT and NC treatment; TP1 and rail connections.
- Verified retained guitar/auxiliary paths, codec supply/output pins, new two-pin amplifier headers, BM83 supply/ground/UART corrections, encoder pull-ups/TIM2_CH1, LCD power/signal pins, serial-console directions and LED driver topology. Memory source retains detect and HyperRAM circuitry.
- Annotation is implemented across the source sheets. C33 value/MPN mismatch remains; C50 capacitor/MPN ambiguity and C52 units now also need closure.
- Codec DRVDD_1 remains open. The power tree now exists; remaining power work is validation/protection/component selection, not initial circuit creation. BM83 VDD_IO and ground-pad errors are resolved; BAT_IN tolerance/transient margin is still open.
- Confirmed decisions retained: high-impedance guitar input, independent guitar/mono-aux capture, external mono LM1875 reuse, and self-powered board with USB VBUS used only for detection. Actual external supply voltage/transformer selection remain to be specified.
- No Altium ERC/compiled cross-sheet netlist, footprint audit or physical tests were run. No firmware changed; firmware/Jenkins tests would not validate these schematic changes.

## Source references

- Current baseline [`a92aa22`](https://github.com/bogdan-tirzioru/guitarAmp/tree/a92aa2296f59b654e6f1917f9ccb693b2685753c); includes preceding power implementation [`9aff26e`](https://github.com/bogdan-tirzioru/guitarAmp/commit/9aff26e5f566b67affb6b08da5fc951339384c87) and BT supply fix [`3b8f99d`](https://github.com/bogdan-tirzioru/guitarAmp/commit/3b8f99ddd7c5d40f6759a44be3bf5c930b3d7c10). All five native sheets and their committed previews under [`layout/guitarDSP`](https://github.com/bogdan-tirzioru/guitarAmp/tree/a92aa2296f59b654e6f1917f9ccb693b2685753c/layout/guitarDSP) were inspected. Previous reviewed source baseline: `31296e9`.
- [ST KFXX datasheet, DocID4337 Rev 14](https://www.st.com/resource/en/datasheet/kfxx.pdf): pinout, INHIBIT low=ON test circuit, 500 mA rating, tolerance, capacitor/ESR and thermal data.
- [TI LM7805/LM340 datasheet](https://www.ti.com/lit/ds/symlink/lm7800.pdf): regulator pinout, bypass, operating and thermal requirements.
- [TI LM317 datasheet, SLVS044Z](https://www.ti.com/lit/ds/symlink/lm317.pdf): output setting, minimum-load dependence, transient support, pinout and reverse-current guidance; select exact IC7 MPN before closing its limits.
- [TI TLV320AIC3104 datasheet, SLAS510G](https://www.ti.com/lit/ds/symlink/tlv320aic3104.pdf): pin functions, supply limits, analog input levels, AC-coupled headphones, outputs and sequencing.
- [Microchip BM83 datasheet, DS70005402H](https://ww1.microchip.com/downloads/aemDocuments/documents/WSG/ProductDocuments/DataSheets/BM83-Bluetooth-Stereo-Audio-Module-Data-Sheet-DS70005402.pdf): current pin functions, BAT_IN range, SYS_PWR/VDD_IO treatment, reset timing, I²S interface and recommended footprint/ground pads.
- [Reused LM1875 power-amplifier schematic](https://github.com/bogdan-tirzioru/amplifier/blob/b777c9ec8a359d5651b260d3cbb7d02f36bd271f/guitaramp/PCB_Project/PowerAmp.SchDoc), amplifier master `b777c9e`.
- [TI LM1875 datasheet, SNAS524A](https://www.ti.com/lit/ds/symlink/lm1875.pdf): power-amplifier pin functions, operating conditions and stability guidance.
- [TI OPA2134 datasheet](https://www.ti.com/lit/ds/symlink/opa2134.pdf): supply, input/output limits and analog-stage checks.
- [ST STM32H5Exxx datasheet, DS14971](https://www.st.com/resource/en/datasheet/stm32h5e5zj.pdf): exact MCU pinout, power scheme and USB supply.
- `AN/ABM3B-8.000MHZ-10-D1G-T.pdf`: Abracon family datasheet/order-code explanation; confirm the full ordered part.
- The S70KL1282 exact-part datasheet should be added/pinned to the hardware reference set before closing its verification item.
