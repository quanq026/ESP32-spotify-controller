# 🎵 ESP32-C3 Spotify Remote

Điều khiển và hiển thị trạng thái Spotify bằng **ESP32-C3 + OLED SSD1306**.  
Dự án sử dụng **Spotify Web API**, với token được cấp phát từ script Python (PKCE) và gửi sang ESP32 qua HTTP.

---

## ✨ Tính năng
- Hiển thị tên bài hát, nghệ sĩ, tiến độ phát trên màn OLED.
- Nút bấm vật lý để:
  - ⏮️ Bài trước
  - ⏯️ Play / Pause
  - ⏭️ Bài tiếp
  - ❤️ Like / Unlike
- Thanh progress bar trực quan.
- ~~Tự động làm mới token khi hết hạn (refresh token).~~ (hiện đang bảo trì)
- Web config tại `http://192.168.4.1`:
  - Nhập WiFi SSID/Password.
  - Nhập Access/Refresh Token thủ công.

---

## 🛠️ Phần cứng
- **ESP32-C3 SuperMini** (hoặc ESP32-C3 Dev Module).
- **OLED SSD1306 128x64 I²C**.
- 4 nút nhấn.

### Sơ đồ chân GPIO
| Chức năng | Pin ESP32-C3 |
|-----------|--------------|
| OLED SDA  | GPIO20       |
| OLED SCL  | GPIO21       |
| Button Prev | GPIO2      |
| Button Toggle (Play/Pause) | GPIO3 |
| Button Next | GPIO4      |
| Button Like/Dislike | GPIO5      |

---

## 💻 Phần mềm

### 1. ESP32 Firmware
- Thư viện cần cài trong Arduino IDE:
  - `WiFi`
  - `HTTPClient`
  - `ArduinoJson`
  - `Wire`
  - `Adafruit_GFX`
  - `Adafruit_SSD1306`
  - [`FontMaker`](https://github.com/daonguyen207/FontMaker)
  - `WebServer`
  - `Preferences`

### 2. Python Token Manager
- File: `send.py`
- Chức năng:
  1. Chạy PKCE flow để lấy **access token + refresh token** từ Spotify.
  2. Lưu token ra file `token_pkce.json`.
  3. Gửi token sang ESP32 qua HTTP (`/set-config`).


