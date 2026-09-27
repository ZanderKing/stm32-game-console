# Sotong Games — STM32 Game Console

An embedded, motion-controlled game console built on the **ST B-L4S5I-IOT01** development board (STM32L4S5). Two party games are played by physically moving the board, with live feedback on an onboard OLED and a Wi-Fi web dashboard that streams orientation and environmental data in real time.

Built for an embedded systems module (EE2028), this project combines bare-metal peripheral drivers, a real-time orientation filter, and a lightweight web server running on the microcontroller.

## Games

### 1. Red Light, Green Light (`game1.c`)
A tilt-sensitivity game. The board cycles between **green light** (move freely) and **red light** (hold still) on a 10-second timer.
- During green light it just reports environment data.
- During red light it fuses **accelerometer + gyroscope + magnetometer** through a complementary filter to estimate roll, pitch, and yaw.
- If the board tilts more than 5° (roll/pitch) or 10° (yaw) from the pose captured at the start of the phase, the player is eliminated.

### 2. Catch and Run (`game2.c`)
A "don't get caught" game driven by a magnetometer.
- The magnetic field strength sets the intensity of an approaching "enforcer": slower blinking means safe, faster blinking means danger.
- A tap on the board (accelerometer tap detection) sets a flag; the player then has a 3-second window to tap again to escape — otherwise it's game over.
- Temperature, pressure, and humidity spikes also trigger "dangerous environment" warnings.

## Hardware

| Component | Role |
|-----------|------|
| ST B-L4S5I-IOT01 (STM32L4S5) | Main MCU + onboard sensors |
| LSM6DSL accelerometer/gyro | Tilt, tap and motion detection |
| LIS3MDL magnetometer | Yaw estimation + game 2 intensity |
| HTS221 temp/humidity, LPS22HB pressure | Environmental telemetry |
| SSD1306 OLED (I²C) | In-game graphics and animation |
| 7-segment display (shift register) | Auxiliary display |
| Onboard LEDs / button | Status + game menu |
| ESP8266 Wi-Fi (Inventek eS-WiFi) | JSON telemetry + web dashboard |

## How It Works

- **Timer-driven sampling:** `TIM6` fires every 10 ms and sets a `sensor_req` flag, giving a 100 Hz sensor loop.
- **Orientation fusion:** `game1.c` implements a complementary filter with hard/soft-iron magnetometer calibration to keep roll, pitch, and yaw stable.
- **State machine:** a double-tap of the user button switches between games; interrupts handle the button, tap detection, and Wi-Fi data-ready line.
- **On-device web server:** the ESP8266 serves a self-contained HTML/Chart.js dashboard over `GET /` and streams live values as JSON over `GET /live`, so the game can be observed from any browser on the same network.

## Project Structure

```
Core/Src/
  main.c        # clock, GPIO, I2C, UART-DMA, TIM6 setup and game dispatch
  game1.c       # Red Light, Green Light
  game2.c       # Catch and Run
  media.c       # OLED/7-segment fonts and frame animation
  miscfunc.c    # UART printf + Wi-Fi web dashboard
  stm32l4xx_it.c
Drivers/
  BSP/          # ST board support package
  CMSIS/        # ARM CMSIS + STM32L4xx HAL
  MY_ADDITION/  # custom Wi-Fi, SSD1306 and sensor helpers
```

## Build & Flash

1. Open the project in **STM32CubeIDE** (`File > Open Projects from File System`).
2. Select the `Debug` build configuration and build.
3. Connect the board via ST-LINK and flash (`Run > Debug`).
4. In `Core/Src/miscfunc.c`, set your network before flashing:

   ```c
   char ssid[] = "YOUR_WIFI_SSID";
   char pass[] = "YOUR_WIFI_PASSWORD";
   ```

## Controls

| Input | Action |
|-------|--------|
| Double-tap user button | Switch between the two games |
| Tilt the board | Move (Red Light, Green Light) |
| Tap the board | React / escape (Catch and Run) |
| Press button after game over | Return to menu |

## Web Dashboard

Once connected, open the board's IP (shown on the UART console) in a browser. The dashboard shows live roll/pitch/yaw, temperature, humidity, pressure, current game, and player state, with rolling charts and a green/red light indicator.

## Notes

- Wi-Fi credentials are placeholders — do not commit real network details.
- Vendor HAL/CMSIS code is included so the project builds out of the box.
- Only the application code under `Core/` and `Drivers/MY_ADDITION/` is original work.
