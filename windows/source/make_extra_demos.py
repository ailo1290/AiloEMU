#!/usr/bin/env python3
"""Original SNES/GB/GBC/GBA integration cartridges, CC0-1.0.
Python 3; GBA assembly requires the optional keystone-engine package.
No Nintendo logo, commercial graphics, code or BIOS is included.
"""
from pathlib import Path
import ast,struct
ROOT=Path(__file__).resolve().parents[1]
font=next(ast.literal_eval(n.value) for n in ast.parse((ROOT/'source/make_demo.py').read_text()).body if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='font' for t in n.targets))
font.update({'5':['11111','10000','10000','11110','00001','00001','11110'], '8':['01110','10001','10001','01110','10001','10001','01110']})

class ASM:
 def __init__(self):self.b=bytearray();self.labels={};self.fix=[]
 def e(self,*v):self.b.extend(v)
 def label(self,s):self.labels[s]=0x8000+len(self.b)
 def addr(self,op,s):self.e(op,0,0);self.fix.append((len(self.b)-2,s,False))
 def br(self,op,s):self.e(op,0);self.fix.append((len(self.b)-1,s,True))
 def done(self):
  for i,s,rel in self.fix:
   v=self.labels[s]
   if rel:
    v-=0x8000+i+1;assert -128<=v<=127;self.b[i]=v&255
   else:self.b[i:i+2]=struct.pack('<H',v)
  return self.b

def snes():
 a=ASM();a.label('reset');a.e(0x78,0x18,0xFB,0xC2,0x30,0xA2,0xFF,0x1F,0x9A,0xE2,0x20)
 def sta(addr):a.e(0x8D,addr&255,addr>>8)
 def value(addr,v):a.e(0xA9,v);sta(addr)
 def stz(addr):a.e(0x9C,addr&255,addr>>8)
 value(0x2100,0x80)
 for r in [0x4200,0x420B,0x420C,0x2105,0x210B,0x212C,0x212D,0x2130,0x2131]:stz(r)
 stz(0);stz(1);stz(2);stz(3)
 value(0x2115,0x80);stz(0x2116);stz(0x2117)
 a.e(0xA2,0,0);a.label('tiles')
 a.addr(0xBD,'font');sta(0x2118);a.e(0xE8);a.addr(0xBD,'font');sta(0x2119);a.e(0xE8,0xE0,0,8);a.br(0xD0,'tiles')
 stz(0x2116);value(0x2117,0x10);a.e(0xA2,0,0);a.label('map')
 a.addr(0xBD,'mapdata');sta(0x2118);a.e(0xE8);a.addr(0xBD,'mapdata');sta(0x2119);a.e(0xE8,0xE0,0,8);a.br(0xD0,'map')
 value(0x2107,0x10)
 for i in range(2):stz(0x210D);stz(0x210E)
 stz(0x2121)
 for b in [0x21,0x08,0xF0,0x7F]:value(0x2122,b)
 value(0x212C,1);value(0x2100,0x0F);value(0x4200,1)
 a.label('notblank');a.e(0xAD,0x12,0x42);a.br(0x30,'notblank')
 a.label('blank');a.e(0xAD,0x12,0x42);a.br(0x10,'blank')
 a.label('joywait');a.e(0xAD,0x12,0x42,0x29,1);a.br(0xD0,'joywait')
 a.e(0xAD,0x18,0x42);sta(0);a.e(0xAD,0x19,0x42);sta(1)
 a.e(0xE6,2)
 # Write a rising A press to the SRAM counter, mirrored in WRAM[4].
 a.e(0xA5,0,0x29,0x80);a.br(0xF0,'noA');a.e(0xA5,3,0x29,0x80);a.br(0xD0,'noA')
 a.e(0xAF,0,0,0x70,0x1A,0x8F,0,0,0x70)
 a.label('noA');a.e(0xA5,0);sta(3);a.e(0xAF,0,0,0x70);sta(4)
 stz(0x2121);a.e(0xA5,0,0x05,1);a.br(0xF0,'idlecolor');value(0x2122,0x18);value(0x2122,0x20);a.addr(0x4C,'colorend')
 a.label('idlecolor');value(0x2122,0x21);value(0x2122,0x08)
 a.label('colorend');a.addr(0x4C,'notblank');a.label('irq');a.e(0x40)
 a.label('font');tiles=bytearray(2048)
 for ch,rows in font.items():
  if ord(ch)>=128:continue
  for y,row in enumerate(rows):tiles[ord(ch)*16+(y+1)*2]=int(row,2)<<2
 a.e(*tiles)
 a.label('mapdata');tilemap=bytearray(2048)
 def line(y,t):
  x=(32-len(t))//2
  for i,ch in enumerate(t):tilemap[(y*32+x+i)*2]=ord(ch)
 for y,t in [(4,'AILOEMU'),(7,'SNES ORIGINAL DEMO'),(12,'PRESS A B X Y L R'),(15,'THE BACKGROUND CHANGES'),(20,'F5 SAVE - F8 LOAD'),(24,'NO COMMERCIAL ROM')]:line(y,t)
 a.e(*tilemap)
 rom=a.done();rom.extend(bytes(32768-len(rom)))
 rom[0x7FC0:0x7FD5]=b'AILOEMU SNES DEMO'.ljust(21,b' ')
 rom[0x7FD5:0x7FDC]=bytes([0x20,2,5,3,1,0,0]) # LoROM + SRAM, 32K ROM, 8K SRAM, NTSC
 for offset in [0x7FE4,0x7FE6,0x7FE8,0x7FEA,0x7FEE,0x7FF4,0x7FF8,0x7FFA,0x7FFE]:rom[offset:offset+2]=struct.pack('<H',a.labels['irq'])
 rom[0x7FFC:0x7FFE]=struct.pack('<H',a.labels['reset'])
 rom[0x7FDC:0x7FE0]=b'\xff\xff\0\0';checksum=sum(rom)&65535
 rom[0x7FDC:0x7FE0]=struct.pack('<HH',checksum^65535,checksum)
 (ROOT/'roms/Ailo-SNES-Demo.sfc').write_bytes(rom)

def gba():
 from keystone import Ks,KS_ARCH_ARM,KS_MODE_ARM,KS_MODE_LITTLE_ENDIAN
 assembly='''
 mov r10, #0x04000000
 mov r0, #0x0400
 orr r0, r0, #3
 strh r0, [r10]
 mov r8, #0x02000000
 mov r7, #0
 str r7, [r8]
 ldr r0, =0x08001000
 mov r1, #0x06000000
 ldr r2, =38400
 copy:
 ldrh r3, [r0], #2
 strh r3, [r1], #2
 subs r2, r2, #1
 bne copy
 mov r0, #0x80
 strh r0, [r10, #0x84]
 ldr r0, =0x1177
 strh r0, [r10, #0x80]
 mov r0, #2
 strh r0, [r10, #0x82]
 wait_active:
 ldrh r0, [r10, #6]
 cmp r0, #160
 bhs wait_active
 wait_blank:
 ldrh r0, [r10, #6]
 cmp r0, #160
 blo wait_blank
 ldr r1, =0x04000130
 ldrh r2, [r1]
 mvn r2, r2
 ldr r0, =0x3FF
 and r2, r2, r0
 str r2, [r8]
 add r7, r7, #1
 str r7, [r8, #4]
 ldr r1, =0x0E000000
 ldrb r3, [r1]
 tst r2, #1
 beq no_save
 ldr r4, [r8, #8]
 tst r4, #1
 bne no_save
 add r3, r3, #1
 strb r3, [r1]
 no_save:
 str r2, [r8, #8]
 str r3, [r8, #12]
 tst r2, #1
 beq quiet
 ldr r0, =0xF080
 strh r0, [r10, #0x62]
 ldr r0, =0x8700
 strh r0, [r10, #0x64]
 b sound_done
 quiet:
 mov r0, #0
 strh r0, [r10, #0x62]
 sound_done:
 ldr r1, =0x060106D0
 ldr r0, =0x7FE0
 cmp r2, #0
 ldrne r0, =0x03FF
 mov r4, #6
 barrow:
 mov r5, #160
 barpixel:
 strh r0, [r1], #2
 subs r5, r5, #1
 bne barpixel
 add r1, r1, #160
 subs r4, r4, #1
 bne barrow
 b wait_active
 '''
 (ROOT/'source/gba_demo.s').write_text(assembly)
 ks=Ks(KS_ARCH_ARM,KS_MODE_ARM|KS_MODE_LITTLE_ENDIAN)
 code,_=ks.asm(assembly,addr=0x080000C0)
 rom=bytearray(128*1024);rom[:4]=struct.pack('<I',0xEA00002E)
 rom[0xA0:0xAC]=b'AILO GBA DEM';rom[0xAC:0xB0]=b'AILO';rom[0xB0:0xB2]=b'00';rom[0xB2]=0x96
 rom[0xBD]=(-sum(rom[0xA0:0xBD])-0x19)&255
 rom[0xC0:0xC0+len(code)]=bytes(code);rom[0x800:0x809]=b'SRAM_V113'
 # Mode 3 framebuffer with original font and simple framed layout.
 img=[0x1042]*(240*160)
 def line(y,t):
  x=(240-len(t)*7)//2
  for i,ch in enumerate(t):
   for yy,row in enumerate(font.get(ch,['00000']*7)):
    for xx,v in enumerate(row):
     if v=='1':img[(y+yy)*240+x+i*7+xx]=0x7FE0
 for y,t in [(16,'AILOEMU'),(40,'GBA ORIGINAL DEMO'),(68,'A B L R - COLOR'),(88,'HOLD A - TONE'),(110,'F5 SAVE - F8 LOAD')]:line(y,t)
 # Fixed 160 x 6 input indicator at x=40,y=140.
 for y in range(140,146):
  for x in range(40,200):img[y*240+x]=0x7FE0
 rom[0x1000:0x1000+76800]=struct.pack('<38400H',*img)
 (ROOT/'roms/Ailo-GBA-Demo.gba').write_bytes(rom)

def gameboy(color=False):
 # Small original cartridge with 8 KiB battery RAM. Only mGBA's four-byte
 # format-recognition signature is present; no commercial game content.
 rom=bytearray(32768)
 code=bytearray()
 labels={};fix=[]
 def emit(*v):code.extend(v)
 def label(name):labels[name]=0x150+len(code)
 def jr(op,name):emit(op,0);fix.append((len(code)-1,name))
 emit(0xF3,0x31,0xFE,0xFF,0xAF,0xE0,0x40) # DI; SP; LCD off
 emit(0x21,0x00,0x80,0x01,0x00,0x20)       # clear 8000-9fff
 label('clear');emit(0x36,0,0x23,0x0B,0x78,0xB1);jr(0x20,'clear')
 emit(0x21,0x10,0x80,0x11,0x00,0x03,0x06,16)
 label('tile');emit(0x1A,0x22,0x13,0x05);jr(0x20,'tile')
 emit(0x21,0x00,0x98,0x01,0x00,0x04)
 label('map');emit(0x36,1,0x23,0x0B,0x78,0xB1);jr(0x20,'map')
 emit(0x3E,0x80,0xE0,0x68)
 for value in [0xFF,0x7F,0xB5,0x56,0x4A,0x29,0,0]:emit(0x3E,value,0xE0,0x69)
 emit(0x3E,0xE4,0xE0,0x47,0x3E,0x91,0xE0,0x40)
 label('active');emit(0xF0,0x44,0xFE,0x90);jr(0x30,'active')
 label('blank');emit(0xF0,0x44,0xFE,0x90);jr(0x38,'blank')
 emit(0x3E,0x10,0xE0,0,0xF0,0,0x2F,0xE6,0x0F,0xEA,0,0xC0,0x47)
 emit(0xE6,1);jr(0x28,'nosave')
 emit(0xFA,2,0xC0,0xE6,1);jr(0x20,'nosave')
 emit(0xFA,0,0xA0,0x3C,0xEA,0,0xA0)
 label('nosave');emit(0x78,0xEA,2,0xC0,0xFA,1,0xC0,0x3C,0xEA,1,0xC0,0x78,0xB7);jr(0x28,'idle')
 emit(0x3E,0x82,0xE0,0x68,0x3E,0x1F,0xE0,0x69,0x3E,0x1B,0xE0,0x47);jr(0x18,'active')
 label('idle');emit(0x3E,0x82,0xE0,0x68,0x3E,0xB5,0xE0,0x69,0x3E,0xE4,0xE0,0x47);jr(0x18,'active')
 for pos,name in fix:
  delta=labels[name]-(0x150+pos+1);assert -128<=delta<=127
  code[pos]=delta&255
 rom[0x100:0x104]=bytes([0xC3,0x50,0x01,0])
 rom[0x104:0x108]=bytes([0xCE,0xED,0x66,0x66])
 rom[0x134:0x143]=(b'AILOEMU GBC' if color else b'AILOEMU GB').ljust(15,b' ')
 rom[0x143]=0x80 if color else 0
 rom[0x147]=0x09 # ROM + RAM + battery
 rom[0x148]=0;rom[0x149]=2;rom[0x14A]=1
 rom[0x14D]=(-sum(rom[0x134:0x14D])-0x19)&255
 rom[0x150:0x150+len(code)]=code
 rom[0x300:0x310]=bytes([0xAA,0x55]*8)
 checksum=(sum(rom)-rom[0x14E]-rom[0x14F])&0xFFFF
 rom[0x14E:0x150]=struct.pack('>H',checksum)
 name='Ailo-GBC-Demo.gbc' if color else 'Ailo-GB-Demo.gb'
 (ROOT/'roms'/name).write_bytes(rom)

if __name__=='__main__':
 snes();gameboy(False);gameboy(True);gba();print('Created original SNES, GB, GBC and GBA demos')
