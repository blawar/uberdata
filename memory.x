/* $Id: memory.x,v 1.15.4.1 2012/12/23 18:34:11 jsmcortina Exp $ */
/*
    Uncertain of the origin of this file.
    Re-written to support MS3.
    No copyright asserted by the Megasquirt-3 team
*/
  MEMORY
  {
    page0   (rwx) : ORIGIN = 0x0, LENGTH = 0x2000
    data1   (rw)  : ORIGIN = 0x1000, LENGTH = 0x1000
    data2   (rw)  : ORIGIN = 0x2000, LENGTH = 0x0800
    data    (rw)  : ORIGIN = 0x2800, LENGTH = 0x1800
    text3   (rx)  : ORIGIN = 0x4000, LENGTH = 0x4000
    cnfdata (rx)  : ORIGIN = 0x100000, LENGTH = 0x3000
    cnfdata2(rx)  : ORIGIN = 0x103000, LENGTH = 0x1000
    lookup  (rx)  : ORIGIN = 0x104000, LENGTH = 0x2400
    textf2  (rx)  : ORIGIN = 0x3d8000, LENGTH = 0x4000
    textf3  (rx)  : ORIGIN = 0x3dc000, LENGTH = 0x4000
    textf4  (rx)  : ORIGIN = 0x3e0000, LENGTH = 0x4000
    textf5  (rx)  : ORIGIN = 0x3e4000, LENGTH = 0x4000
    textf6  (rx)  : ORIGIN = 0x3e8000, LENGTH = 0x4000
    textf7  (rx)  : ORIGIN = 0x3ec000, LENGTH = 0x4000
    textf8  (rx)  : ORIGIN = 0x3f0000, LENGTH = 0x4000
    textf9  (rx)  : ORIGIN = 0x3f4000, LENGTH = 0x4000
    textfa  (rx)  : ORIGIN = 0x3f8000, LENGTH = 0x4000
    textfb  (rx)  : ORIGIN = 0x3fc000, LENGTH = 0x4000
    textfc  (rx)  : ORIGIN = 0x400000, LENGTH = 0x4000
    textfe  (rx)  : ORIGIN = 0x408000, LENGTH = 0x4000
    text    (rx)  : ORIGIN = 0xC000, LENGTH = 0x2f10
  }
  PROVIDE (_stack = 0x3fff);
