# ESP32 LED 矩陣控制面板

![img](https://github.com/HansHans135/esp32-led-control-panel/raw/main/image/1.png)

這是一個基於ESP32的16x16 LED矩陣控制面板，支持WiFi連接、Web界面控制、拖動繪圖和狀態記憶功能。


![img](https://github.com/HansHans135/esp32-led-control-panel/raw/main/image/2.png)

## 功能特色

- 🌐 **WiFi管理**: 使用WiFiManager進行簡易WiFi設置
- 🎨 **拖動繪圖**: 支持滑鼠拖動和觸控繪圖
- 💾 **狀態記憶**: 可儲存和載入LED圖案
- 📱 **響應式設計**: 支持手機和電腦操作
- 🌈 **顏色選擇**: 完整的顏色選擇器
- 🖼️ **即時預覽**: 網頁與實體LED同步顯示

## 硬體需求

- ESP32開發板
- 16x16 WS2812B LED矩陣 (256顆LED/蛇行排列)
- 適當的電源供應 (5V, 建議15A以上)
- 1000µF電容器 (連接在LED電源正負極之間)
- 300-500Ω電阻 (連接在ESP32資料腳和LED資料輸入之間)

## 接線說明

```
ESP32         LED矩陣
GPIO4(D2)  →  DIN (資料輸入)
5V         →  +5V (透過外部電源)
GND        →  GND
```

## 軟體需求

### Arduino庫依賴

需要安裝以下Arduino庫：

```cpp
#include <Adafruit_NeoPixel.h>    // LED控制
#include <WiFiManager.h>          // WiFi管理
#include <ArduinoJson.h>          // JSON處理
```

### 安裝庫的方法

在Arduino IDE中：
1. 工具 → 管理程式庫
2. 搜尋並安裝：
   - `Adafruit NeoPixel`
   - `WiFiManager`
   - `ArduinoJson`


## 使用說明

### 1. 硬體組裝
1. 按照接線說明連接ESP32和LED矩陣
2. 確保電源供應充足
3. 添加電容器和電阻保護電路

### 2. 程式上傳
1. 開啟 `led.ino` 在Arduino IDE中
2. 選擇正確的ESP32開發板
3. 上傳程式到ESP32

### 3. WiFi設置
1. ESP32啟動後會建立名為 "ESP32_LED_Matrix" 的WiFi熱點
2. 連接到該熱點
3. 瀏覽器會自動開啟設置頁面，或手動訪問 192.168.4.1
4. 選擇您的WiFi網路並輸入密碼
5. 設置完成後ESP32會重啟並連接到您的WiFi

### 4. 控制面板使用
1. 在串列監視器中找到ESP32的IP地址
2. 在瀏覽器中訪問該IP地址
3. 使用控制面板：
   - **顏色選擇**: 點擊顏色選擇器選擇繪圖顏色
   - **繪圖模式**: 點擊或是拖曳來繪畫


## 自定義設置

### 修改矩陣大小
在 `led.ino` 中修改：
```cpp
#define LED_COUNT 256  // 總LED數量
#define MATRIX_WIDTH 16  // 矩陣寬度
#define MATRIX_HEIGHT 16  // 矩陣高度
```

### 修改連接腳位
```cpp
#define LED_PIN 4  // 修改為您使用的GPIO腳位
```

### 修改接線模式
如果您的LED矩陣接線方式不同，請修改 `xyToIndex()` 函數。

## 進階功能
[文字跑馬燈](text.md)


## 授權

此專案基於MIT授權，歡迎自由使用和修改。

## 貢獻

歡迎提交問題報告和改進建議！