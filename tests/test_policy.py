from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def test_native_policy(tmp_path):
    source=tmp_path/'policy.c'
    source.write_text('#include <assert.h>\n#include "policy.h"\nint main(void){assert(hour_angle(10,8)==3040);assert(hour_angle(23,59)==3595);assert(minute_angle(15)==900);assert(second_angle(45)==2700);assert(display_hour(0,1,false)==12);assert(display_hour(23,2,false)==23);assert(uses24(0,true));assert(!uses24(0,false));return 0;}')
    subprocess.run(['cc','-std=c99','-Wall','-Werror','-I',str(ROOT/'watchface/src/c'),str(source),'-o',str(tmp_path/'policy')],check=True)
    subprocess.run([str(tmp_path/'policy')],check=True)
def test_digit_masks_reproducible():
    target=ROOT/'watchface/src/c/digits.h';before=target.read_bytes()
    subprocess.run(['python3',str(ROOT/'execution/generate_digits.py')],check=True)
    assert before==target.read_bytes()
def test_cardinal_bounds():
    for x,y,w,h in [(73,8,56,35),(11,88,28,35),(161,88,28,35),(86,168,28,35)]:
        assert 0<x<x+w<200
        assert 0<y<y+h<=203
