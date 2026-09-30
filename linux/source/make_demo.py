#!/usr/bin/env python3
"""Original NROM test cartridge. No commercial code, graphics or music.
AiloEMU demo and this generator: CC0-1.0 (public-domain dedication).
Uses a tiny assembler so rebuilding needs only Python 3, not a NES SDK.
"""
from pathlib import Path
import sys

class Asm:
    def __init__(self): self.code=bytearray(); self.labels={}; self.fix=[]
    def label(self,s): self.labels[s]=0x8000+len(self.code)
    def emit(self,*b): self.code.extend(b)
    def addr(self,op,label): self.emit(op,0,0); self.fix.append((len(self.code)-2,label,False))
    def branch(self,op,label): self.emit(op,0); self.fix.append((len(self.code)-1,label,True))
    def finish(self):
        for i,label,relative in self.fix:
            a=self.labels[label]
            if relative:
                n=a-(0x8000+i+1)
                assert -128<=n<=127,(label,n)
                self.code[i]=n&255
            else: self.code[i:i+2]=a.to_bytes(2,'little')
        return self.code

font={
'A':['01110','10001','10001','11111','10001','10001','10001'],
'B':['11110','10001','10001','11110','10001','10001','11110'],
'C':['01111','10000','10000','10000','10000','10000','01111'],
'D':['11110','10001','10001','10001','10001','10001','11110'],
'E':['11111','10000','10000','11110','10000','10000','11111'],
'F':['11111','10000','10000','11110','10000','10000','10000'],
'G':['01111','10000','10000','10111','10001','10001','01111'],
'H':['10001','10001','10001','11111','10001','10001','10001'],
'I':['11111','00100','00100','00100','00100','00100','11111'],
'J':['00111','00010','00010','00010','10010','10010','01100'],
'K':['10001','10010','10100','11000','10100','10010','10001'],
'L':['10000','10000','10000','10000','10000','10000','11111'],
'M':['10001','11011','10101','10101','10001','10001','10001'],
'N':['10001','11001','10101','10011','10001','10001','10001'],
'O':['01110','10001','10001','10001','10001','10001','01110'],
'P':['11110','10001','10001','11110','10000','10000','10000'],
'Q':['01110','10001','10001','10001','10101','10010','01101'],
'R':['11110','10001','10001','11110','10100','10010','10001'],
'S':['01111','10000','10000','01110','00001','00001','11110'],
'T':['11111','00100','00100','00100','00100','00100','00100'],
'U':['10001','10001','10001','10001','10001','10001','01110'],
'V':['10001','10001','10001','10001','10001','01010','00100'],
'W':['10001','10001','10001','10101','10101','10101','01010'],
'X':['10001','10001','01010','00100','01010','10001','10001'],
'Y':['10001','10001','01010','00100','00100','00100','00100'],
'Z':['11111','00001','00010','00100','01000','10000','11111'],
'1':['00100','01100','00100','00100','00100','00100','01110'],
'2':['01110','10001','00001','00010','00100','01000','11111'],
'0':['01110','10001','10011','10101','11001','10001','01110'],
'-':['00000','00000','00000','11111','00000','00000','00000'],
'.':['00000','00000','00000','00000','00000','00110','00110'],
}
chrrom=bytearray(8192)
for char,rows in font.items():
    pattern=bytes([0]+[int(row,2)<<2 for row in rows])
    chrrom[ord(char)*16:ord(char)*16+8]=pattern
chrrom[1*16:1*16+8]=bytes([0x18,0x3C,0x7E,0xFF,0x7E,0x3C,0x24,0x42])
chrrom[2*16:2*16+8]=bytes([0,0,0,0x18,0x18,0,0,0])
chrrom[3*16:3*16+8]=bytes([0xFF]*8)
nt=bytearray(1024)
def text(row,s):
    x=(32-len(s))//2; nt[row*32+x:row*32+x+len(s)]=s.encode('ascii')
text(3,'AILOEMU');text(5,'ORIGINAL TEST CARTRIDGE');text(8,'MOVE YOUR PIXEL SHIP')
text(22,'ARROWS - MOVE');text(24,'X - TONE    Z - COLOR');text(26,'ENTER - CENTER')
for x in range(2,30): nt[10*32+x]=nt[20*32+x]=3
for y in range(11,20):nt[y*32+2]=nt[y*32+29]=3
for x,y in [(7,13),(19,12),(24,17),(11,18),(16,15)]:nt[y*32+x]=2

a=Asm();a.label('reset')
a.emit(0x78,0xD8,0xA2,0xFF,0x9A,0xE8) # SEI CLD LDX FF TXS INX
for addr in (0x2000,0x2001,0x4010):a.emit(0x8E,addr&255,addr>>8)
a.emit(0xA9,0x40,0x8D,0x17,0x40,0x2C,0x02,0x20)
a.label('vblank1');a.emit(0x2C,0x02,0x20);a.branch(0x10,'vblank1')
a.emit(0xA9,0,0xA2,0);a.label('clear')
for page in range(8):a.emit(0x9D,0,page)
a.emit(0xE8);a.branch(0xD0,'clear')
a.emit(0xA9,0xFF,0xA2,0);a.label('hide');a.emit(0x9D,0,2,0xE8);a.branch(0xD0,'hide')
a.label('vblank2');a.emit(0x2C,0x02,0x20);a.branch(0x10,'vblank2')
# Palette data.
a.emit(0xA9,0x3F,0x8D,6,0x20,0xA9,0,0x8D,6,0x20,0xA2,0)
a.label('palettecopy');a.addr(0xBD,'palette');a.emit(0x8D,7,0x20,0xE8,0xE0,32);a.branch(0xD0,'palettecopy')
a.emit(0xA9,0x20,0x8D,6,0x20,0xA9,0,0x8D,6,0x20,0xA2,0)
for p in range(4):
    a.label('ntcopy'+str(p));a.addr(0xBD,'nametable'+str(p));a.emit(0x8D,7,0x20,0xE8);a.branch(0xD0,'ntcopy'+str(p))
a.emit(0xA9,120,0x85,0,0xA9,120,0x85,1) # ship x/y
a.emit(0xA9,0,0x8D,0,0x20,0x8D,5,0x20,0x8D,5,0x20,0xA9,0x1E,0x8D,1,0x20)
a.label('frame');a.emit(0x2C,2,0x20);a.branch(0x10,'frame')
# Read controllers. Accumulation order results A=0x80 ... Right=0x01.
a.emit(0xA9,1,0x8D,0x16,0x40,0xA9,0,0x8D,0x16,0x40,0x85,2,0x85,3,0xA2,8)
a.label('readpad');a.emit(0xAD,0x16,0x40,0x4A,0x26,2,0xAD,0x17,0x40,0x4A,0x26,3,0xCA);a.branch(0xD0,'readpad')
for mask,opcode,var,name in [(1,0xE6,0,'right'),(2,0xC6,0,'left'),(4,0xE6,1,'down'),(8,0xC6,1,'up')]:
    a.emit(0xA5,2,0x29,mask);a.branch(0xF0,'skip'+name);a.emit(opcode,var);a.label('skip'+name)
a.emit(0xA5,2,0x29,0x10);a.branch(0xF0,'nocenter');a.emit(0xA9,120,0x85,0,0x85,1);a.label('nocenter')
# Simple tone while A is held (pulse channel, constant volume).
a.emit(0xA5,2,0x29,0x80);a.branch(0xF0,'silent')
a.emit(0xA9,1,0x8D,0x15,0x40,0xA9,0xBF,0x8D,0,0x40,0xA9,0xFD,0x8D,2,0x40,0xA9,8,0x8D,3,0x40)
a.addr(0x4C,'sounddone');a.label('silent');a.emit(0xA9,0,0x8D,0x15,0x40);a.label('sounddone')
# Increment SRAM counter when A is pressed, for persistence diagnostics.
a.emit(0xA5,2,0x29,0x80);a.branch(0xF0,'saveend');a.emit(0xA5,4);a.branch(0x30,'saveend');a.emit(0xEE,0,0x60)
a.label('saveend');a.emit(0xA5,2,0x85,4)
# OAM and palette updates during vblank.
a.emit(0xA5,1,0x8D,0,2,0xA9,1,0x8D,1,2,0xA9,0,0x8D,2,2,0x8D,3,0x20,0xA5,0,0x8D,3,2,0xA9,2,0x8D,0x14,0x40)
a.emit(0xA9,0x3F,0x8D,6,0x20,0xA9,0x11,0x8D,6,0x20,0xA5,2,0x29,0x40);a.branch(0xF0,'normalcolor')
a.emit(0xA9,0x27);a.addr(0x4C,'setcolor');a.label('normalcolor');a.emit(0xA9,0x2A)
a.label('setcolor');a.emit(0x8D,7,0x20,0xA9,0,0x8D,0,0x20,0x8D,5,0x20,0x8D,5,0x20,0xE6,5)
a.addr(0x4C,'frame');a.label('interrupt');a.emit(0x40)
a.label('palette');a.emit(*([0x0F,0x21,0x11,0x30]*4+[0x0F,0x2A,0x30,0x16]*4))
for p in range(4):a.label('nametable'+str(p));a.emit(*nt[p*256:(p+1)*256])
prg=a.finish();prg.extend(bytes(16384-len(prg)))
prg[-6:]=b''.join(a.labels[l].to_bytes(2,'little') for l in ['interrupt','reset','interrupt'])
header=b'NES\x1a'+bytes([1,1,2,0])+bytes(8) # mapper 0, battery-backed 8K RAM
target=Path(sys.argv[1]) if len(sys.argv)>1 else Path(__file__).resolve().parents[1]/'roms'/'Ailo-Demo.nes'
target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(header+prg+chrrom)
print('Created original demo:',target)
