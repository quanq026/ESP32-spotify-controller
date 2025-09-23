# pip install requests
import base64
import hashlib
import json
import os
import threading
import time
import webbrowser
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlencode, urlparse, parse_qs
import requests

# ======================
# Config
# ======================
CLIENT_ID = "xxx"
CLIENT_SECRET = "yyy"
REDIRECT_URI = "http://127.0.0.1:8000/callback"
SCOPES = [
    "user-read-playback-state",
    "user-modify-playback-state",
    "user-library-modify",
    "user-library-read"
]
AUTH_URL = "https://accounts.spotify.com/authorize"
TOKEN_URL = "https://accounts.spotify.com/api/token"
TOKEN_FILE = "token_pkce.json"

# Thay IP này = IP của ESP32 (STA hoặc AP mode)
ESP32_IP = "192.168.1.11"

# ======================
# PKCE helpers
# ======================
def gen_code_verifier():
    v = base64.urlsafe_b64encode(os.urandom(64)).rstrip(b"=")
    return v.decode("ascii")

def gen_code_challenge(verifier: str) -> str:
    digest = hashlib.sha256(verifier.encode("ascii")).digest()
    return base64.urlsafe_b64encode(digest).rstrip(b"=").decode("ascii")

def save_token(data: dict):
    with open(TOKEN_FILE, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

# ======================
# Local HTTP server
# ======================
class AuthHandler(BaseHTTPRequestHandler):
    auth_code = None
    auth_error = None

    def do_GET(self):
        parsed = urlparse(self.path)
        if parsed.path != "/callback":
            self.send_response(404)
            self.end_headers()
            return

        qs = parse_qs(parsed.query)
        if "error" in qs:
            AuthHandler.auth_error = qs.get("error", ["unknown"])[0]
        else:
            AuthHandler.auth_code = qs.get("code", [None])[0]

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"<h1>OK</h1><p>You can close this window.</p>")

def start_server():
    httpd = HTTPServer(("127.0.0.1", 8000), AuthHandler)
    httpd.timeout = 300
    httpd.handle_request()

# ======================
# Send token to ESP32
# ======================
def send_token_http(ip: str, token_data: dict):
    url = f"http://{ip}/set-config"
    payload = {
        "atoken": token_data["access_token"],
        "rtoken": token_data.get("refresh_token", "")
    }
    try:
        resp = requests.post(url, data=payload, timeout=5)
        if resp.status_code == 200:
            print("✅ Token gửi thành công tới ESP32.")
        else:
            print(f"❌ ESP32 trả về {resp.status_code}: {resp.text}")
    except Exception as e:
        print("❌ Không gửi được token:", e)

# ======================
# Re-auth only
# ======================
def do_pkce_authorize():
    verifier = gen_code_verifier()
    challenge = gen_code_challenge(verifier)

    params = {
        "client_id": CLIENT_ID,
        "client_secret": CLIENT_SECRET,
        "response_type": "code",
        "redirect_uri": REDIRECT_URI,
        "scope": " ".join(SCOPES),
        "code_challenge_method": "S256",
        "code_challenge": challenge,
    }
    url = f"{AUTH_URL}?{urlencode(params)}"

    print("🌐 Mở trình duyệt để đăng nhập Spotify...")
    threading.Thread(target=start_server, daemon=True).start()
    webbrowser.open(url)

    for _ in range(300):
        if AuthHandler.auth_error:
            raise RuntimeError(f"OAuth error: {AuthHandler.auth_error}")
        if AuthHandler.auth_code:
            code = AuthHandler.auth_code
            break
        time.sleep(0.2)
    else:
        raise TimeoutError("Không nhận được authorization code.")

    data = {
        "grant_type": "authorization_code",
        "code": code,
        "redirect_uri": REDIRECT_URI,
        "client_id": CLIENT_ID,
        "code_verifier": verifier,
    }
    resp = requests.post(TOKEN_URL, data=data, timeout=15)
    if resp.status_code != 200:
        raise RuntimeError(f"Token exchange failed: {resp.status_code} {resp.text}")

    token_data = resp.json()
    save_token(token_data)
    print("✅ Đăng nhập thành công, token đã lưu local.")

    # Gửi sang ESP32
    print("👉 Đang gửi token sang ESP32...")
    send_token_http(ESP32_IP, token_data)
    print("⚡ ESP32 sẽ lưu token và reboot.")

    return token_data

# ======================
# Entry point
# ======================
if __name__ == "__main__":
    if os.path.exists(TOKEN_FILE):
        os.remove(TOKEN_FILE)
    do_pkce_authorize()

