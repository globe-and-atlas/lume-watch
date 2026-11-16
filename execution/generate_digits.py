"""Generate reproducible original layout masks from OFL Archivo Black."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
ROOT=Path(__file__).resolve().parents[1]
font=ImageFont.truetype(str(ROOT/'assets/fonts/ArchivoBlack-Regular.ttf'),44)
rows=[]
for digit in '0123456789':
    box=font.getbbox(digit);im=Image.new('L',(box[2]-box[0],box[3]-box[1]));ImageDraw.Draw(im).text((-box[0],-box[1]),digit,font=font,fill=255)
    im=im.resize((28,35),Image.Resampling.LANCZOS)
    rows.append([sum(1<<x for x in range(28) if im.getpixel((x,y))>=128) for y in range(35)])
(ROOT/'watchface/src/c/digits.h').write_text('#pragma once\n#define DIGIT_W 28\n#define DIGIT_H 35\nstatic const uint32_t DIGITS[10][35]={\n'+',\n'.join('{'+','.join(hex(row)+'u' for row in glyph)+'}' for glyph in rows)+'\n};\n')
