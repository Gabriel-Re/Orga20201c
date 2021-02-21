#include "recode57.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum encoding bom_to_encoding(uint8_t *bom){
  enum encoding result = UTF8;

  if(bom[0] == 254 && bom[1] == 255){
    result = UTF16BE;
  }else if(bom[0] == 00 && bom[1] == 00 && bom[2] == 254 && bom[3] == 255){
    result = UTF32BE;
  }else if(bom[0] == 255 && bom[1] == 254 && bom[2] == 00 && bom[3] == 00){
    result = UTF32LE;
  }else if(bom[0] == 255 && bom[1] == 254){
    result = UTF16LE;
  }
  return result;
}

size_t write_bom(enum encoding enc, uint8_t *buf){
  uint8_t mask_ff = 0XFF;
  uint8_t mask_fe = 0XFE;
  int cantidad_bytes_bom = 0;

  if(enc == UTF16BE){
    buf[0] = mask_fe;
    buf[1] = mask_ff;
    cantidad_bytes_bom = 2;
  }else if(enc == UTF32BE){
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = mask_fe;
    buf[3] = mask_ff;
    cantidad_bytes_bom = 4;
  }else if(enc == UTF32LE){
    buf[0] = mask_ff;
    buf[1] = mask_fe;
    buf[2] = 0;
    buf[3] = 0;
    cantidad_bytes_bom = 4;
  }else if(enc == UTF16LE){
    buf[0] = mask_ff;
    buf[1] = mask_fe;
    cantidad_bytes_bom = 2;
  }
  return cantidad_bytes_bom;
}

size_t read_codepoint(enum encoding enc, uint8_t *buf, size_t n, codepoint *dest){
  codepoint codepoint = 0;
  size_t bytes = 0;
  if((enc == UTF32LE || enc == UTF32BE) && n >= 4){
    if(enc == UTF32BE){
      codepoint |= buf[0] << 24;
      codepoint |= buf[1] << 16;
      codepoint |= buf[2] << 8;
      codepoint |= buf[3];
    }else{
      codepoint |= buf[0];
      codepoint |= buf[1] << 8;
      codepoint |= buf[2] << 16;
      codepoint |= buf[3] << 24;
    }
    bytes = 4;
  }
  else if(enc == UTF16LE || enc == UTF16BE){
    if(n >= 2){
      if(enc == UTF16BE){
        if(buf[0] >> 2 != 0x36){
          codepoint |= buf[0] << 8;
          codepoint |= buf[1];
          bytes = 2;
        }else{
          if(n >= 4){
            codepoint |= (buf[0] & 3) << 18;// 3 = 0000 0011 sabiendo que buf[0] = 1101 10xx
            codepoint |= buf[1] << 10;
            codepoint |= (buf[2] & 3) << 8;// 3 = 0000 0011 sabiendo que buf[2] = 1101 11xx
            codepoint |= buf[3];
            codepoint = codepoint + 65536;//65536 = 0001 0000 0000 0000 0000
            bytes = 4;
          }
        }
      }else{
        if(buf[1] >> 2 != 0x36){
          codepoint |= buf[0];
          codepoint |= buf[1] << 8;
          bytes = 2;
        }else{
          if(n >= 4){
            codepoint |= buf[0] << 10;
            codepoint |= (buf[1] & 3) << 18;// 3 = 0000 0011 sabiendo que buf[1] = 1101 10xx
            codepoint |= buf[2];
            codepoint |= (buf[3] & 3) << 8;// 3 = 0000 0011 sabiendo que buf[3] = 1101 11xx
            codepoint = codepoint + 65536;//65536 = 0001 0000 0000 0000 0000
            bytes = 4;
          }
        }
      }
    }
  }else if(enc == UTF8){
    if(n > 0){
      if(buf[0] < 0X80){
        codepoint |= buf[0];
        bytes = 1;
      }else if(buf[0] < 0XE0){//224 = 1110 xxxx
        codepoint |= ((buf[0] & 31) << 6); //31 = 0001 1111
        codepoint |= buf[1] & 63; //63 = 0011 1111
        bytes = 2;
      }else if(buf[0] < 0XF0){//240 = 1111 00000
        codepoint |= ((buf[0] & 15) << 12); //15 = 0000 1111, SABIENDO QUE AL SER 3 es 1110
        codepoint |= ((buf[1] & 63) << 6); //63 = 0011 1111
        codepoint |= buf[2] & 63; //63 = 0011 1111
        bytes = 3;
      }else{
        codepoint |= ((buf[0] & 7) << 18); //7 = 0000 0111, SABIENDO QUE AL SER 4 es 1111 0
        codepoint |= ((buf[1] & 63) << 12); //63 = 0011 1111
        codepoint |= ((buf[2] & 63) << 6); //63 = 0011 1111
        codepoint |= buf[3] & 63; //63 = 0011 1111
        bytes = 4;
      }
    }
  }
  *dest = codepoint;
  return bytes;
}

size_t write_codepoint(enum encoding enc, codepoint cp, uint8_t *outbuf){
  size_t bytes = 0;
  codepoint aux = cp;
  if(enc == UTF32LE || enc == UTF32BE){
    if(enc == UTF32LE){
      outbuf[0] = aux & 255; // 00
      outbuf[1] = (aux >> 8) & 255; // 00
      outbuf[2] = (aux >> 16) & 255; // fe
      outbuf[3] = (aux >> 24) & 255; // ff
    }else{
      outbuf[0] = (aux >> 24) & 255; // 00
      outbuf[1] = (aux >> 16) & 255; // 00
      outbuf[2] = (aux >> 8) & 255; // Fe
      outbuf[3] = aux & 255; // ff
    }
    bytes = 4;
  }else if((enc == UTF16LE || enc == UTF16BE)){
    if(cp <= 0XFFFF){
      if(enc == UTF16LE){
        outbuf[0] = aux & 255;// FE
        outbuf[1] = (aux >> 8) & 255;//FF
      }else{
        outbuf[0] = (aux >> 8) & 255;//FF
        outbuf[1] = aux & 255;//FE
      }
      bytes = 2;
    }else{
      aux = aux - 65536;//65536 = 0001 0000 0000 0000 0000
      if(enc == UTF16LE){
        outbuf[0] = (aux >> 10) & 255;
        outbuf[1] = 216 | (aux >> 18);// 216 = 1101 10xx
        outbuf[2] = aux & 255;
        outbuf[3] = 220 | ((aux >> 8) & 3);// 220 = 1101 11xx y 3 = 0000 0011 respetando 1101 11xx
      }else{
        outbuf[0] = 216 | (aux >> 18);// 216 = 1101 10xx
        outbuf[1] = (aux >> 10) & 255;
        outbuf[2] = 220 | ((aux >> 8) & 3);// 220 = 1101 11xx y 3 = 0000 0011 respetando 1101 11xx
        outbuf[3] = aux & 255;
      }
      bytes = 4;
    }
  }else if(enc == UTF8){
    if(cp < 0X80){
      outbuf[0] = aux;
      bytes = 1;
    }else if(cp < 0X800){
      outbuf[0] = 192 | (aux >> 6);// 192 = 1100 0000 (1000 0000 >> 6 = 1111 1111)
      outbuf[1] = 128 | (aux & 63);// 128 = 1000 0000, 63 = 0011 1111
      bytes = 2;                  // 1000 0000 | 0011 1111  = 10xxxxxx
    }else if(cp < 0X10000){
      outbuf[0] = 224 | (aux >> 12);// 224 = 1110 0000 (>> 12 = 1111 1111 1111 1110)
      outbuf[1] = 128 | ((aux >> 6) & 63);// ((>> 6 = 1111 1110) & 63 = 0011 1111) | 128 = 10xx xxxx
      outbuf[2] = 128 | (aux & 63);// 0011 1111 | 1011 1111 = 10xx xxxx
      bytes = 3;
    }else{
      outbuf[0] = 240 | (aux >> 18);// 240 = 1111 0000 (>> 12 = 1111 1111 1111 1111)
      outbuf[1] = 128 | ((aux >> 12) & 63);// ((>> 12 = 1111 1111) & 63 = 0011 1111) | 128 = 10xx xxxx
      outbuf[2] = 128 | ((aux >> 6) & 63);// ((>> 6 = 1111 1111) & 63 = 0011 1111) | 128 = 10xx xxxx
      outbuf[3] = 128 | (aux & 63);// 0011 1111 | 1011 1111 = 10xx xxxx
      bytes = 4;
    }
  }
  return bytes;
}
