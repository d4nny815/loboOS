typedef unsigned short uint16_t;

void kmain(void) {
  volatile uint16_t* video = (uint16_t*)0xB8000;

  video[0] = ('S' | 0x2f00);
  video[1] = ('I' | 0x2f00);
  video[2] = ('C' | 0x2f00);
  video[3] = ('K' | 0x2f00);

  while(1);
}
