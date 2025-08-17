import requests
import json
import time
import os
import sys
from PIL import Image, ImageDraw, ImageFont



class Config:
    SERVER_URL = "http://192.168.50.249"
    DEFAULT_HEADERS = {"content-type": "application/json"}
    REQUEST_TIMEOUT = 5

def hex_to_dec(hex_color):
    hex_color = hex_color.lstrip('#')
    return int(hex_color, 16)

def load_animation_from_json(file_path):
    with open(file_path, 'r',encoding="utf-8") as file:
        data = json.load(file)
        return data

def send_request(endpoint, data, headers=None):
    if headers is None:
        headers = Config.DEFAULT_HEADERS
    
    url = f"{Config.SERVER_URL}/{endpoint}"
    response = requests.post(
        url,
        headers=headers,
        data=json.dumps(data),
        timeout=Config.REQUEST_TIMEOUT
    )
    # print(f"{endpoint} 狀態碼: {response.status_code}")
        

def text_to_16x16(text, font_path=None, as_1d=False, color="#FFFFFF"):
    img = Image.new('L', (16, 16), color=255)
    draw = ImageDraw.Draw(img)

    if font_path:
        size = 16
        font = None
        while size > 4:
            f = ImageFont.truetype(font_path, size)
            try:
                bbox = draw.textbbox((0, 0), text, font=f)
            except AttributeError:
                w, h = draw.textsize(text, font=f)
                bbox = (0, 0, w, h)
            w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
            if w <= 16 and h <= 16:
                font = f
                break
            size -= 1
        if font is None:
            font = ImageFont.truetype(font_path, 8)
    else:
        font = ImageFont.load_default()
        try:
            bbox = draw.textbbox((0, 0), text, font=font)
        except AttributeError:
            w, h = draw.textsize(text, font=font)
            bbox = (0, 0, w, h)
        w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
        if w > 16 or h > 16:
            tmp = Image.new('L', (32, 32), 255)
            td = ImageDraw.Draw(tmp)
            try:
                td.text((16, 16), text, font=font, fill=0, anchor='mm')
            except TypeError:
                tw, th = td.textsize(text, font=font)
                td.text(((32 - tw) / 2, (32 - th) / 2), text, font=font, fill=0)
            img = tmp.resize((16, 16), Image.LANCZOS)
            bw = img.point(lambda p: 0 if p < 128 else 255)
            pixels = [[1 if bw.getpixel((x, y)) == 0 else 0 for x in range(16)] for y in range(16)]
            return pixels, bw

    try:
        bbox = draw.textbbox((0, 0), text, font=font)
    except AttributeError:
        w, h = draw.textsize(text, font=font)
        bbox = (0, 0, w, h)
    w, h = bbox[2] - bbox[0], bbox[3] - bbox[1]
    x = (16 - w) // 2 - bbox[0]
    y = (16 - h) // 2 - bbox[1]
    draw.text((x, y), text, font=font, fill=0)

    bw = img.point(lambda p: 0 if p < 128 else 255)
    pixels = [[color if bw.getpixel((x, y)) == 0 else "#000000" for x in range(16)] for y in range(16)]

    if as_1d:
        flat = [v for row in pixels for v in row]
        return flat, bw

    return pixels, bw

def play_animation(frames, frame_interval):
        
    print(f"開始播放，共 {len(frames)} 幀，每幀間隔 {frame_interval} 秒")
    while True:
        for i, frame in enumerate(frames):
            update_data = {
                "colors": [hex_to_dec(color) for color in frame]
            }
            send_request("update", update_data)
            print(f"\r已播放第 {i+1}/{len(frames)} 幀", end="", flush=True)
            time.sleep(frame_interval)
        print()

def main(text, color="#FFFFFF", time_interval=0.05, font_path='msjh.ttc'):
        char_matrices = []
        for i in text:
            char_matrices.append(text_to_16x16(i, as_1d=True, font_path=font_path,color=color)[0])
        display_width = 16
        blank_display = ['#000000'] * 16 * 16
        data = []
        total_width = len(char_matrices) * 16
        for start_pos in range(display_width, -total_width-1, -1):
            frame = blank_display.copy()
            for char_idx, char_matrix in enumerate(char_matrices):
                char_start_x = start_pos + char_idx * 16
                for y in range(16):
                    for x in range(16):
                        display_x = char_start_x + x
                        display_y = y
                        if 0 <= display_x < 16:
                            display_pos = display_y * 16 + display_x
                            char_pos = y * 16 + x
                            if char_matrix[char_pos] != '#000000':
                                frame[display_pos] = char_matrix[char_pos]
            
            data.append(frame)
        play_animation(data, time_interval)
    
if __name__ == "__main__":
    main("Hello", color="#F50000", time_interval=0)