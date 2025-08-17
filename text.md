# 文字跑馬燈
## 安裝Python依賴套件

```bash
pip install requests pillow
```

## 使用方法

1. **基本文字顯示**:
```python
python main.py
```
預設會顯示 "Hello" 文字，紅色滾動效果。

2. **自定義參數**:
```python
# 在main.py中修改參數
main("你的文字", color="#00FF00", time_interval=0.1, font_path='arial.ttf')
```

## 參數說明

- `text`: 要顯示的文字內容
- `color`: 文字顏色 (十六進制格式，如 "#FF0000" 為紅色)
- `time_interval`: 幀間隔時間 (秒)，控制滾動速度
- `font_path`: 字體檔案路徑，支援TTF/TTC格式

## 功能特色

- **16x16矩陣適配**: 自動將文字轉換為16x16像素矩陣
- **平滑滾動**: 支援從右到左的平滑文字滾動效果
- **多語言支持**: 支援中文、英文等多種語言字體
- **顏色自定義**: 可設定任意顏色的文字顯示
- **速度控制**: 可調整滾動速度以符合不同需求