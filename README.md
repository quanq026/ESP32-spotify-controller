# 🎵 ESP32-C3 Spotify Remote

- Điều khiển và hiển thị trạng thái Spotify bằng **ESP32-C3 + OLED SSD1306**.  
- Dự án sử dụng **Spotify Web API**, với token được cấp phát từ script Python (PKCE) và gửi sang ESP32 qua HTTP.
- LƯU Ý: ĐỂ DÙNG FULL TÍNH NĂNG (HẦU HẾT TÍNH NĂNG) YÊU CẦU TÀI KHOẢN SPOTIFY PHẢI LÀ PREMIUM!!!

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

## 3,1. Linh kiện cần chuẩn bị
1. Các công cụ hàn cần thiết
2. 4 con ốc m3 10mm [https://shopee.vn/%E1%BB%90c-V%C3%ADt-%C4%90%E1%BA%A7u-D%E1%BA%B9t-inox-304-%E1%BB%90c-v%C3%ADt-%C4%91%E1%BA%A7u-pake-d%E1%BA%B9p-m%E1%BB%8Fng-ph%E1%BA%B3ng-V%C3%ADt-M3-M4-M5-i.823408794.21450012113]
3. ESP32-C3 Super mini [https://shopee.vn/Samirob-ESP32-C3-ESP32-S3-ESP32-H2-ESP32-C6-Ban-Ph%C3%A1t-Tri%E1%BB%83n-ESP32-C3-SuperMini-WiFi-Bluetooth-ESP32C3-B%E1%BA%A3ng-M%E1%BB%9F-R%E1%BB%99ng-i.578443443.24400570619]
4. Oled 0.96 icnh [https://shopee.vn/M%C3%B4-%C4%90un-Hi%E1%BB%83N-Th%E1%BB%8B-OLED-4pin-7pin-0.96-IIC-I2C-OLED-0.96-inch-128X64-OLED-0.96-IIC-I2C-Chuy%C3%AAn-D%E1%BB%A5ng-Cho-arduino-i.578443443.23972641115]
5. 4 Keycaps (tùy sở thích cá nhân)
6. 4 swicth cơ (tùy sở thích cá nhân)
7. Mạch sạc TP4056 [https://shopee.vn/-L%E1%BA%ADp-Tr%C3%ACnh-Nh%C3%BAng-A-Z-155-M%E1%BA%A1ch-S%E1%BA%A1c-Pin-Lithium-TP4056-1A-C%E1%BB%95ng-Type-C-C%C3%B3-B%E1%BA%A3o-V%E1%BB%87-Pin-i.107147748.29590726472?sp_atk=985e49b4-1ac7-4f4e-a482-5bb3186ef46d&xptdk=985e49b4-1ac7-4f4e-a482-5bb3186ef46d]
8. Pin 702530 500 mAh [https://shopee.vn/-L%E1%BA%ADp-Tr%C3%ACnh-Nh%C3%BAng-A-Z-Pin-lithium-3.7v-C%C3%B3-M%E1%BA%A1ch-B%E1%BA%A3o-V%E1%BB%87-Nhi%E1%BB%81u-Lo%E1%BA%A1i-i.107147748.24844147461]
9. 3D case
10. Jack type C cái loại 2 dây [https://shopee.vn/Jack-type-C-%C4%91%E1%BA%A7u-c%C3%A1i-(-%C4%91%E1%BA%A7u-c%E1%BA%AFm-N%E1%BB%AF)-ch%E1%BA%BF-s%E1%BA%A1c-ch%E1%BA%BF-loa-diy-c%E1%BB%B1c-d%E1%BB%85-la%CC%80m-i.121774202.17998407987?sp_atk=865da5e9-a789-44c7-a24e-507e3f657d9e&xptdk=865da5e9-a789-44c7-a24e-507e3f657d9e]
## 3,2. Sơ đồ mạch

![Sơ đồ mạch](Sơ%20đồ%20mạch.png)

## 3,3. Công cụ nên có
- Đồ nghề hàn (máy hàn, thiếc, nhựa thông, bọt biển, bông sắt)
- Nhíp bé
- Khẩu trang
- Keo nến
- Tua vít
- Dụng cụ tuốt dây
- rất nhiều dây để nối linh kiện
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
* Chú ý: bạn có 2 cách để truy cập web config:
- 1 là truy cập vào AP ESP32C3_Config rồi nhập http://192.168.4.1
- 2 là kết nối cùng với wifi của ESP32 rồi kiểm tra ip STA khi khởi động ESP32 rồi vào web nhập ip hiện lên trên màn hình OLED (cách này sẽ tiện hơn)

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
```cpp
preferences.begin("spotify", false);
wifiSsid = preferences.getString("wifi_ssid", "Ten_wifi");
wifiPass = preferences.getString("wifi_pass", "Mat_khau");
preferences.end();
```
- Khi đó ESP32 sẽ thử kết nối với WiFi mặc định này nếu chưa có config trong Preferences.
  ## 5️⃣ Cách lấy Client ID / Client Secret cho Spotify

Để script Python (`spotify_pkce_esp32.py`) hoạt động, bạn cần có **Client ID** và **Client Secret** từ Spotify Developer.
![Web Config](web%20config.jpg)

## 5. Cách lấy Client ID / Client Secret để đưa vào send.py để tạo token
1. Vào trang [Spotify Developer Dashboard](https://developer.spotify.com/dashboard).  
2. Đăng nhập bằng tài khoản Spotify của bạn.  
3. Nhấn **Create an App** → đặt tên (ví dụ: `ESP32 Controller`).  
4. Sau khi tạo xong, mở app bạn vừa tạo:
   - **Client ID** sẽ hiển thị ngay.  
   - Nhấn nút **View Client Secret** để lấy **Client Secret**.  
5. Thêm `Redirect URI`:
   - Bấm **Edit Settings**.  
   - Trong phần **Redirect URIs**, thêm:  
     ```
     http://127.0.0.1:8000/callback
     ```
   - Nhấn **Save**.
![Dashboard](Spotify%20Dev.png)
### Thêm vào file Python
Mở file `send.py` và sửa 2 dòng sau:

```python
CLIENT_ID = "your_client_id_here"
CLIENT_SECRET = "your_client_secret_here"
```
## 6. Hướng dẫn biên dịch
- ESP32-C3 SuperMini → chọn ESP32C3 Dev Module.
