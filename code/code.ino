
//* ---------------------
//* 这是一个基于 Arduino 的电子琴，支持 12 种乐器音色，并采用 4 通道的合成方式。
//* 主要功能包括：
//* - 通过 18 个按键控制不同的音符
//* - 使用 ADSR（Attack, Decay, Sustain, Release）包络控制音量
//* - 采用 FM（频率调制）技术实现更丰富的音色
//* - 通过 PWM（脉宽调制）输出声音信号
//* 
//* 项目特点：
//* - 多乐器支持：提供 12 种不同的乐器音色
//* - FM 调制：对每个音符进行 FM 频率调制，以增强音色的表现力
//* - ADSR 包络：通过起音、衰减、延音和释放控制音符的动态表现
//* - 高效的按键扫描：利用端口直接读取按键状态，提高按键响应速度
//* - 低延迟 PWM 生成：使用 9 位快速 PWM 产生高质量音频输出



// 乐器定义
#define ninstr 12 // 定义乐器数量为12种：钢琴、木琴、笛音等

// 每种乐器的音量值
unsigned int ldness[ninstr]  = {64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64}; // 响度

// 每种乐器的基准音高
unsigned int pitch0[ninstr]  = {12, 12, 12, 12, 24, 24, 0, 12, 24, 12, 12, 24}; // 基准音高

// ADSR 包络的起音参数
unsigned int ADSR_a[ninstr]  = {4096, 8192, 8192, 8192, 4096, 512, 512, 8192, 128, 128, 256, 256}; // 起音参数

// ADSR 包络的衰减参数
unsigned int ADSR_d[ninstr]  = {8, 32, 16, 16, 8, 16, 16, 8, 16, 16, 64, 32}; // 衰减参数

// ADSR 包络的延音参数
unsigned int ADSR_s[ninstr]  = {0, 0, 0, 0, 0, 0, 0, 0, 240, 240, 192, 192}; // 延音参数

// ADSR 包络的释放参数
unsigned int ADSR_r[ninstr]  = {64, 128, 32, 32, 16, 32, 32, 32, 32, 32, 64, 64}; // 释放参数

// 每种乐器的 FM 调制频率增量，与音高相关
unsigned int FM_inc[ninstr]  = {256, 512, 768, 400, 200, 96, 528, 244, 256, 128, 64, 160}; // FM 频率增量

// FM 调制幅度起始值
unsigned int FM_a1[ninstr]  = {128, 512, 512, 1024, 512, 0, 1024, 2048, 256, 256, 384, 256}; // FM 幅度起始值

// FM 调制幅度结束值
unsigned int FM_a2[ninstr]  = {64, 0, 128, 128, 128, 512, 768, 512, 128, 128, 256, 128}; // FM 幅度结束值

// FM 衰减参数
unsigned int FM_dec[ninstr]  = {64, 128, 128, 128, 32, 128, 128, 128, 128, 128, 64, 64}; // FM 衰减参数

// 定义音高到按键的映射
#define keyC4   0
#define keyC4s  1
#define keyD4   2
#define keyD4s  3
#define keyE4   4
#define keyF4   5
#define keyF4s  6
#define keyG4   7
#define keyG4s  8
#define keyA4   9
#define keyA4s 10
#define keyB4  11
#define keyC5  12
#define keyC5s 13
#define keyD5  14
#define keyD5s 15
#define keyE5  16
#define keyF5  17

#define nokey 255    // 无按键按下
#define instrkey 254 // 乐器选择按键

// 定义18键键盘的引脚到按键的映射
#define pinD0 keyC5    // Arduino 引脚 D0
#define pinD1 keyB4    // Arduino 引脚 D1
#define pinD2 keyA4s   // Arduino 引脚 D2
#define pinD3 keyA4    // Arduino 引脚 D3
#define pinD4 keyG4s   // Arduino 引脚 D4
#define pinD5 keyG4    // Arduino 引脚 D5
#define pinD6 keyF4s   // Arduino 引脚 D6
#define pinD7 keyF4    // Arduino 引脚 D7
#define pinB0 keyE4    // Arduino 引脚 D8
#define pinB1 nokey    // Arduino 引脚 D9 用于音频输出
#define pinB2 keyD4s   // Arduino 引脚 D10
#define pinB3 keyD4    // Arduino 引脚 D11
#define pinB4 keyC4s   // Arduino 引脚 D12
#define pinB5 keyC4    // Arduino 引脚 D13
#define pinB6 nokey    // Arduino 引脚 D14 不使用
#define pinB7 nokey    // Arduino 引脚 D15 不使用
#define pinC0 keyC5s   // Arduino 引脚 A0
#define pinC1 keyD5    // Arduino 引脚 A1
#define pinC2 keyD5s   // Arduino 引脚 A2
#define pinC3 keyE5    // Arduino 引脚 A3
#define pinC4 keyF5    // Arduino 引脚 A4
#define pinC5 instrkey // Arduino 引脚 A5
#define pinC6 nokey    // Arduino 引脚 A6 不使用
#define pinC7 nokey    // Arduino 引脚 A7 不使用

// 设置包含正弦值的数组，使用有符号的8位数
const float pi = 3.14159265;
char sine[256];

// 初始化正弦波数组
void setsine() {
  for (int i = 0; i < 256; ++i) {
    sine[i] = (sin(2 * 3.14159265 * (i + 0.5) / 256)) * 128;
    // 计算并存储256个正弦波样本，范围为-128到127
  }
}

// 设置频率/相位增量，从C3=0到B6（A4定义为440Hz）
unsigned int tone_inc[48];
void settones() {
  for (byte i = 0; i < 48; i++) {
    tone_inc[i] = 440.0 * pow(2.0, ((i - 21) / 12.0)) * 65536.0 / (16000000.0 / 512) + 0.5;
    // 计算每个音调的相位增量，基于A4=440Hz
  }
}

// 按钮状态变量
byte butstatD = 0;
byte butstatB = 0;
byte butstatC = 0;
byte prevbutstatD = 0;
byte prevbutstatB = 0;
byte prevbutstatC = 0;

byte instr = 0; // 当前选择的乐器

void setup() {

  // 禁用所有中断以避免声音出现杂音
  noInterrupts();

  // 初始化正弦波数组
  setsine();

  // 初始化音调频率相位增量数组
  settones();

  // 在引脚9上设置快速PWM信号，使用TIMER1，9位分辨率，31250Hz
  pinMode(9, OUTPUT);
  TCCR1A = 0B10000010; // 设置为9位快速PWM模式
  TCCR1B = 0B00001001; // 设置TIMER1的控制寄存器B

  // 设置输入引脚并启用上拉电阻
  if(pinD0 != nokey){DDRD &= ~(1<<0); PORTD |= (1<<0);};
  if(pinD1 != nokey){DDRD &= ~(1<<1); PORTD |= (1<<1);};
  if(pinD2 != nokey){DDRD &= ~(1<<2); PORTD |= (1<<2);};
  if(pinD3 != nokey){DDRD &= ~(1<<3); PORTD |= (1<<3);};
  if(pinD4 != nokey){DDRD &= ~(1<<4); PORTD |= (1<<4);};
  if(pinD5 != nokey){DDRD &= ~(1<<5); PORTD |= (1<<5);};
  if(pinD6 != nokey){DDRD &= ~(1<<6); PORTD |= (1<<6);};
  if(pinD7 != nokey){DDRD &= ~(1<<7); PORTD |= (1<<7);};
  if(pinB0 != nokey){DDRB &= ~(1<<0); PORTB |= (1<<0);};
  if(pinB1 != nokey){DDRB &= ~(1<<1); PORTB |= (1<<1);};
  if(pinB2 != nokey){DDRB &= ~(1<<2); PORTB |= (1<<2);};
  if(pinB3 != nokey){DDRB &= ~(1<<3); PORTB |= (1<<3);};
  if(pinB4 != nokey){DDRB &= ~(1<<4); PORTB |= (1<<4);};
  if(pinB5 != nokey){DDRB &= ~(1<<5); PORTB |= (1<<5);};
  if(pinB6 != nokey){DDRB &= ~(1<<6); PORTB |= (1<<6);};
  if(pinB7 != nokey){DDRB &= ~(1<<7); PORTB |= (1<<7);};
  if(pinC0 != nokey){DDRC &= ~(1<<0); PORTC |= (1<<0);};
  if(pinC1 != nokey){DDRC &= ~(1<<1); PORTC |= (1<<1);};
  if(pinC2 != nokey){DDRC &= ~(1<<2); PORTC |= (1<<2);};
  if(pinC3 != nokey){DDRC &= ~(1<<3); PORTC |= (1<<3);};
  if(pinC4 != nokey){DDRC &= ~(1<<4); PORTC |= (1<<4);};
  if(pinC5 != nokey){DDRC &= ~(1<<5); PORTC |= (1<<5);};
  if(pinC6 != nokey){DDRC &= ~(1<<6); PORTC |= (1<<6);};
  if(pinC7 != nokey){DDRC &= ~(1<<7); PORTC |= (1<<7);};

  // 在启动时存储按钮的初始状态
  butstatD = PIND;
  butstatB = PINB;
  butstatC = PINC;
}

// 初始化脉冲长度设置的主要参数
#define nch 4 // 定义可以同时发声的通道数为4
unsigned int phase[nch]  = {0, 0, 0, 0}; // 各通道的相位
int          inc[nch]    = {0, 0, 0, 0}; // 各通道的相位增量
byte         amp[nch]    = {0, 0, 0, 0}; // 各通道的振幅
unsigned int FMphase[nch]= {0, 0, 0, 0}; // 各通道的FM相位
unsigned int FMinc[nch]  = {0, 0, 0, 0}; // 各通道的FM相位增量
unsigned int FMamp[nch]  = {0, 0, 0, 0}; // 各通道的FM振幅

// 主函数（强制内联）用于更新脉冲长度
inline void setPWM() __attribute__((always_inline));
inline void setPWM() {

  // 等待计时器完成一个循环
  while ((TIFR1 & 0B00000001) == 0);

  // 通过写入1来清除溢出位
  TIFR1 |= 0B00000001;

  // 增加各通道的FM相位
  FMphase[0] += FMinc[0];
  FMphase[1] += FMinc[1];
  FMphase[2] += FMinc[2];
  FMphase[3] += FMinc[3];

  // 增加各通道的音符相位
  phase[0] += inc[0];
  phase[1] += inc[1];
  phase[2] += inc[2];
  phase[3] += inc[3];

  // 计算输出值，并为TIMER1A设置脉冲宽度
  int val = sine[(phase[0] + sine[FMphase[0] >> 8] * FMamp[0]) >> 8] * amp[0];
  val += sine[(phase[1] + sine[FMphase[1] >> 8] * FMamp[1]) >> 8] * amp[1];
  val += sine[(phase[2] + sine[FMphase[2] >> 8] * FMamp[2]) >> 8] * amp[2];
  val += sine[(phase[3] + sine[FMphase[3] >> 8] * FMamp[3]) >> 8] * amp[3];

  // 设置脉冲长度
  OCR1A = val / 128 + 256;
}

// 每个正在播放的音符的属性
byte         iADSR[nch]     = {0, 0, 0, 0}; // ADSR状态
unsigned int envADSR[nch]   = {0, 0, 0, 0}; // ADSR环境值
unsigned int ADSRa[nch]     = {0, 0, 0, 0}; // 起音参数
unsigned int ADSRd[nch]     = {0, 0, 0, 0}; // 衰减参数
unsigned int ADSRs[nch]     = {0, 0, 0, 0}; // 延音参数
unsigned int ADSRr[nch]     = {0, 0, 0, 0}; // 释放参数
byte         amp_base[nch]  = {0, 0, 0, 0}; // 基础振幅
unsigned int inc_base[nch]  = {0, 0, 0, 0}; // 基础相位增量
unsigned int FMa0[nch]      = {0, 0, 0, 0}; // FM振幅起始值
int          FMda[nch]      = {0, 0, 0, 0}; // FM振幅变化量
unsigned int FMinc_base[nch]= {0, 0, 0, 0}; // FM基础相位增量
unsigned int FMdec[nch]     = {0, 0, 0, 0}; // FM衰减参数
unsigned int FMexp[nch]     = {0, 0, 0, 0}; // FM指数衰减
unsigned int FMval[nch]     = {0, 0, 0, 0}; // FM值
byte         keych[nch]     = {0, 0, 0, 0}; // 各通道当前按下的键
unsigned int tch[nch]       = {0, 0, 0, 0}; // 各通道的时间计数器

// 主循环。循环的持续时间由setPWM调用次数决定
// 每次setPWM调用对应512个周期=32微秒
// Tloop=32微秒 * #setPWM。#setPWM=15时，Tloop=0.48毫秒
void loop() {

  // 读取并解释输入按钮
  prevbutstatD = butstatD;
  prevbutstatB = butstatB;
  prevbutstatC = butstatC;
  butstatD = PIND;
  butstatB = PINB;
  butstatC = PINC;
  byte keypressed = nokey;  // 初始化按下的键为无
  byte keyreleased = nokey; // 初始化释放的键为无

  // 检查D端口的按键状态变化
  if(butstatD != prevbutstatD){
    if (pinD0 != nokey && (butstatD & (1<<0)) == 0 && (prevbutstatD & (1<<0)) >  0 ) keypressed  = pinD0;
    if (pinD0 != nokey && (butstatD & (1<<0)) >  0 && (prevbutstatD & (1<<0)) == 0 ) keyreleased = pinD0;
    if (pinD1 != nokey && (butstatD & (1<<1)) == 0 && (prevbutstatD & (1<<1)) >  0 ) keypressed  = pinD1;
    if (pinD1 != nokey && (butstatD & (1<<1)) >  0 && (prevbutstatD & (1<<1)) == 0 ) keyreleased = pinD1;
    if (pinD2 != nokey && (butstatD & (1<<2)) == 0 && (prevbutstatD & (1<<2)) >  0 ) keypressed  = pinD2;
    if (pinD2 != nokey && (butstatD & (1<<2)) >  0 && (prevbutstatD & (1<<2)) == 0 ) keyreleased = pinD2;
    if (pinD3 != nokey && (butstatD & (1<<3)) == 0 && (prevbutstatD & (1<<3)) >  0 ) keypressed  = pinD3;
    if (pinD3 != nokey && (butstatD & (1<<3)) >  0 && (prevbutstatD & (1<<3)) == 0 ) keyreleased = pinD3;
    if (pinD4 != nokey && (butstatD & (1<<4)) == 0 && (prevbutstatD & (1<<4)) >  0 ) keypressed  = pinD4;
    if (pinD4 != nokey && (butstatD & (1<<4)) >  0 && (prevbutstatD & (1<<4)) == 0 ) keyreleased = pinD4;
    if (pinD5 != nokey && (butstatD & (1<<5)) == 0 && (prevbutstatD & (1<<5)) >  0 ) keypressed  = pinD5;
    if (pinD5 != nokey && (butstatD & (1<<5)) >  0 && (prevbutstatD & (1<<5)) == 0 ) keyreleased = pinD5;
    if (pinD6 != nokey && (butstatD & (1<<6)) == 0 && (prevbutstatD & (1<<6)) >  0 ) keypressed  = pinD6;
    if (pinD6 != nokey && (butstatD & (1<<6)) >  0 && (prevbutstatD & (1<<6)) == 0 ) keyreleased = pinD6;
    if (pinD7 != nokey && (butstatD & (1<<7)) == 0 && (prevbutstatD & (1<<7)) >  0 ) keypressed  = pinD7;
    if (pinD7 != nokey && (butstatD & (1<<7)) >  0 && (prevbutstatD & (1<<7)) == 0 ) keyreleased = pinD7;
  }

  // 检查B端口的按键状态变化
  if(butstatB != prevbutstatB){
    if (pinB0 != nokey && (butstatB & (1<<0)) == 0 && (prevbutstatB & (1<<0)) >  0 ) keypressed  = pinB0;
    if (pinB0 != nokey && (butstatB & (1<<0)) >  0 && (prevbutstatB & (1<<0)) == 0 ) keyreleased = pinB0;
    if (pinB1 != nokey && (butstatB & (1<<1)) == 0 && (prevbutstatB & (1<<1)) >  0 ) keypressed  = pinB1;
    if (pinB1 != nokey && (butstatB & (1<<1)) >  0 && (prevbutstatB & (1<<1)) == 0 ) keyreleased = pinB1;
    if (pinB2 != nokey && (butstatB & (1<<2)) == 0 && (prevbutstatB & (1<<2)) >  0 ) keypressed  = pinB2;
    if (pinB2 != nokey && (butstatB & (1<<2)) >  0 && (prevbutstatB & (1<<2)) == 0 ) keyreleased = pinB2;
    if (pinB3 != nokey && (butstatB & (1<<3)) == 0 && (prevbutstatB & (1<<3)) >  0 ) keypressed  = pinB3;
    if (pinB3 != nokey && (butstatB & (1<<3)) >  0 && (prevbutstatB & (1<<3)) == 0 ) keyreleased = pinB3;
    if (pinB4 != nokey && (butstatB & (1<<4)) == 0 && (prevbutstatB & (1<<4)) >  0 ) keypressed  = pinB4;
    if (pinB4 != nokey && (butstatB & (1<<4)) >  0 && (prevbutstatB & (1<<4)) == 0 ) keyreleased = pinB4;
    if (pinB5 != nokey && (butstatB & (1<<5)) == 0 && (prevbutstatB & (1<<5)) >  0 ) keypressed  = pinB5;
    if (pinB5 != nokey && (butstatB & (1<<5)) >  0 && (prevbutstatB & (1<<5)) == 0 ) keyreleased = pinB5;
    if (pinB6 != nokey && (butstatB & (1<<6)) == 0 && (prevbutstatB & (1<<6)) >  0 ) keypressed  = pinB6;
    if (pinB6 != nokey && (butstatB & (1<<6)) >  0 && (prevbutstatB & (1<<6)) == 0 ) keyreleased = pinB6;
    if (pinB7 != nokey && (butstatB & (1<<7)) == 0 && (prevbutstatB & (1<<7)) >  0 ) keypressed  = pinB7;
    if (pinB7 != nokey && (butstatB & (1<<7)) >  0 && (prevbutstatB & (1<<7)) == 0 ) keyreleased = pinB7;
  }

  // 检查C端口的按键状态变化
  if(butstatC != prevbutstatC){
    if (pinC0 != nokey && (butstatC & (1<<0)) == 0 && (prevbutstatC & (1<<0)) >  0 ) keypressed  = pinC0;
    if (pinC0 != nokey && (butstatC & (1<<0)) >  0 && (prevbutstatC & (1<<0)) == 0 ) keyreleased = pinC0;
    if (pinC1 != nokey && (butstatC & (1<<1)) == 0 && (prevbutstatC & (1<<1)) >  0 ) keypressed  = pinC1;
    if (pinC1 != nokey && (butstatC & (1<<1)) >  0 && (prevbutstatC & (1<<1)) == 0 ) keyreleased = pinC1;
    if (pinC2 != nokey && (butstatC & (1<<2)) == 0 && (prevbutstatC & (1<<2)) >  0 ) keypressed  = pinC2;
    if (pinC2 != nokey && (butstatC & (1<<2)) >  0 && (prevbutstatC & (1<<2)) == 0 ) keyreleased = pinC2;
    if (pinC3 != nokey && (butstatC & (1<<3)) == 0 && (prevbutstatC & (1<<3)) >  0 ) keypressed  = pinC3;
    if (pinC3 != nokey && (butstatC & (1<<3)) >  0 && (prevbutstatC & (1<<3)) == 0 ) keyreleased = pinC3;
    if (pinC4 != nokey && (butstatC & (1<<4)) == 0 && (prevbutstatC & (1<<4)) >  0 ) keypressed  = pinC4;
    if (pinC4 != nokey && (butstatC & (1<<4)) >  0 && (prevbutstatC & (1<<4)) == 0 ) keyreleased = pinC4;
    if (pinC5 != nokey && (butstatC & (1<<5)) == 0 && (prevbutstatC & (1<<5)) >  0 ) keypressed  = pinC5;
    if (pinC5 != nokey && (butstatC & (1<<5)) >  0 && (prevbutstatC & (1<<5)) == 0 ) keyreleased = pinC5;
    if (pinC6 != nokey && (butstatC & (1<<6)) == 0 && (prevbutstatC & (1<<6)) >  0 ) keypressed  = pinC6;
    if (pinC6 != nokey && (butstatC & (1<<6)) >  0 && (prevbutstatC & (1<<6)) == 0 ) keyreleased = pinC6;
    if (pinC7 != nokey && (butstatC & (1<<7)) == 0 && (prevbutstatC & (1<<7)) >  0 ) keypressed  = pinC7;
    if (pinC7 != nokey && (butstatC & (1<<7)) >  0 && (prevbutstatC & (1<<7)) == 0 ) keyreleased = pinC7;
  }

  setPWM(); //#1

  // 如果乐器选择按键被按下，则切换乐器
  if (keypressed == instrkey) {
    instr++;
    if (instr >= ninstr) instr = 0; // 超过乐器数量后循环回第一个乐器
    keypressed = keyA4; // 设置为默认键A4
  }
  if (keyreleased == instrkey) keyreleased = keyA4;

  setPWM(); //#2

  // 找到最适合开始新音符的通道
  byte nextch = 255;
  // 首先检查按键是否仍在被按下
  if (iADSR[0] > 0 && keypressed == keych[0]) nextch = 0;
  if (iADSR[1] > 0 && keypressed == keych[1]) nextch = 1;
  if (iADSR[2] > 0 && keypressed == keych[2]) nextch = 2;
  if (iADSR[3] > 0 && keypressed == keych[3]) nextch = 3;
  // 如果没有正在播放的通道，检查是否有空闲通道
  if (nextch == 255) {
    if (iADSR[0] == 0) nextch = 0;
    if (iADSR[1] == 0) nextch = 1;
    if (iADSR[2] == 0) nextch = 2;
    if (iADSR[3] == 0) nextch = 3;
  }
  // 如果所有通道都在使用，选择播放时间最长的通道进行覆盖
  if (nextch == 255) {
    nextch = 0;
    if (tch[1] > tch[nextch]) nextch = 1;
    if (tch[2] > tch[nextch]) nextch = 2;
    if (tch[3] > tch[nextch]) nextch = 3;
  }

  setPWM(); //#3

  // 如果有新按键按下，初始化新音符
  if (keypressed != nokey) {
    phase[nextch] = 0;
    amp_base[nextch] = ldness[instr];
    inc_base[nextch] = tone_inc[pitch0[instr] + keypressed];
    ADSRa[nextch] = ADSR_a[instr];
    ADSRd[nextch] = ADSR_d[instr];
    ADSRs[nextch] = ADSR_s[instr] << 8;
    ADSRr[nextch] = ADSR_r[instr];
    iADSR[nextch] = 1; // 设置ADSR状态为起音阶段
    FMphase[nextch] = 0;
    FMinc_base[nextch] = ((long)inc_base[nextch] * FM_inc[instr]) / 256;
    FMa0[nextch] = FM_a2[instr];
    FMda[nextch] = FM_a1[instr] - FM_a2[instr];
    FMexp[nextch] = 0xFFFF;
    FMdec[nextch] = FM_dec[instr];
    keych[nextch] = keypressed;
    tch[nextch] = 0;
  }

  setPWM(); //#4

  // 如果有按键被释放，停止相应的音符
  if (keyreleased != nokey) {
    if (keych[0] == keyreleased) iADSR[0] = 4; // 设置ADSR状态为释放阶段
    if (keych[1] == keyreleased) iADSR[1] = 4;
    if (keych[2] == keyreleased) iADSR[2] = 4;
    if (keych[3] == keyreleased) iADSR[3] = 4;
  }

  setPWM(); //#5

  // 更新FM衰减指数
  FMexp[0] -= (long)FMexp[0] * FMdec[0] >> 16;
  FMexp[1] -= (long)FMexp[1] * FMdec[1] >> 16;
  FMexp[2] -= (long)FMexp[2] * FMdec[2] >> 16;
  FMexp[3] -= (long)FMexp[3] * FMdec[3] >> 16;

  setPWM(); //#6

  // 调整ADSR包络
  for (byte ich = 0; ich < nch; ich++) {
    if (iADSR[ich] == 4) { // 释放阶段
      if (envADSR[ich] <= ADSRr[ich]) {
        envADSR[ich] = 0;
        iADSR[ich] = 0; // 音符结束
      }
      else envADSR[ich] -= ADSRr[ich];
    }
    if (iADSR[ich] == 2) { // 衰减阶段
      if (envADSR[ich] <= (ADSRs[ich] + ADSRd[ich])) {
        envADSR[ich] = ADSRs[ich];
        iADSR[ich] = 3; // 进入延音阶段
      }
      else envADSR[ich] -= ADSRd[ich];
    }
    if (iADSR[ich] == 1) { // 起音阶段
      if ((0xFFFF - envADSR[ich]) <= ADSRa[ich]) {
        envADSR[ich] = 0xFFFF;
        iADSR[ich] = 2; // 进入衰减阶段
      }
      else envADSR[ich] += ADSRa[ich];
    }
    tch[ich]++; // 增加时间计数器
    setPWM(); //#7-10
  }

  // 更新通道0的音调
  amp[0] = (amp_base[0] * (envADSR[0] >> 8)) >> 8; // 计算当前振幅
  inc[0] = inc_base[0]; // 设置相位增量
  FMamp[0] = FMa0[0] + ((long)FMda[0] * FMexp[0] >> 16); // 计算FM振幅
  FMinc[0] = FMinc_base[0]; // 设置FM相位增量
  setPWM(); //#11

  // 更新通道1的音调
  amp[1] = (amp_base[1] * (envADSR[1] >> 8)) >> 8;
  inc[1] = inc_base[1];
  FMamp[1] = FMa0[1] + ((long)FMda[1] * FMexp[1] >> 16);
  FMinc[1] = FMinc_base[1];
  setPWM(); //#12

  // 更新通道2的音调
  amp[2] = (amp_base[2] * (envADSR[2] >> 8)) >> 8;
  inc[2] = inc_base[2];
  FMamp[2] = FMa0[2] + ((long)FMda[2] * FMexp[2] >> 16);
  FMinc[2] = FMinc_base[2];
  setPWM(); //#13

  // 更新通道3的音调
  amp[3] = (amp_base[3] * (envADSR[3] >> 8)) >> 8;
  inc[3] = inc_base[3];
  FMamp[3] = FMa0[3] + ((long)FMda[3] * FMexp[3] >> 16);
  FMinc[3] = FMinc_base[3];
  setPWM(); //#14

  // 更新各通道的时间计数器
  tch[0]++;
  tch[1]++;
  tch[2]++;
  tch[3]++;

  setPWM(); //#15

}