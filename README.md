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
- ~~Tự động làm mới token khi hết hạn (refresh token).~~ (hiện đang bảo trì, dùng file send.py)
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

## 3. Sơ đồ mạch

![Sơ đồ mạch](Sơ%20đồ%20mạch.png)

## 4️. Hướng dẫn cấu hình (Configuration Guide)

ESP32-C3 hỗ trợ cấu hình bằng **Web UI** hoặc **Python script**.  
Dưới đây là 2 cách bạn có thể lựa chọn:

### 🔹 Cách 1: Cấu hình qua Web UI
1. Nạp firmware vào ESP32-C3.  
2. Kết nối WiFi từ máy tính/điện thoại vào Access Point do ESP32 phát ra:
   - SSID: `ESP32C3_Config`  
   - Password: `12345678`
3. Mở trình duyệt và truy cập `http://192.168.4.1`.  
4. Giao diện cấu hình sẽ hiển thị:  
   - **Access Token**: Dán access token lấy từ Spotify.  
   - **Refresh Token**: Dán refresh token (nếu có).  
   - **WiFi SSID/Password**: Nhập thông tin WiFi STA để ESP32 tự kết nối.  
5. Bấm **Save** → ESP32 sẽ lưu cấu hình và tự động reboot.

> ⚠️ Lưu ý: Nếu bạn chưa có token, hãy dùng Cách 2.

---

### 🔹 Cách 2: Cấu hình tự động bằng Python script
1. Chạy file `send.py` trên PC:
2. Script sẽ mở trình duyệt để bạn đăng nhập Spotify.
3. Sau khi đăng nhập thành công, Token sẽ được lưu ở token_pkce.json, Token đồng thời được gửi tự động sang ESP32 qua HTTP (/set-config), ESP32 báo “Configuration Saved! Rebooting…” và khởi động lại.
4. Sau khi reboot, ESP32 đã sẵn sàng để điều khiển Spotify.
### 🔹 Kiểm tra cấu hình
- Mở Serial Monitor (115200 baud).
- Sau khi kết nối WiFi thành công, ESP32 sẽ in ra:
```
STA IP: 192.168.x.x
[SETUP] Ready. Visit http://192.168.4.1
```
- Trên màn OLED cũng hiển thị IP WiFi STA (nếu kết nối thành công).
### 📝 Ghi chú
- Nếu bạn muốn chỉnh WiFi mặc định trực tiếp trong code (thay vì cấu hình Web UI), hãy sửa đoạn:
``` cpp
preferences.begin("spotify", false);
wifiSsid = preferences.getString("wifi_ssid", "Ten_wifi");
wifiPass = preferences.getString("wifi_pass", "Mat_khau");
preferences.end();
```
- Khi đó ESP32 sẽ thử kết nối với WiFi mặc định này nếu chưa có config trong Preferences.
