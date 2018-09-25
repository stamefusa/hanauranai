#include <SPI.h>
#include <MsTimer2.h>

// ピン定義。
#define PIN_SPI_MOSI 11
#define PIN_SPI_MISO 12
#define PIN_SPI_SCK 13
#define PIN_SPI_SS 10
#define PIN_BUSY 9
#define PIN_FLOWER 7
#define PIN_BUTTON 6

int state = LOW, pre_state = LOW;
int button = LOW, pre_button = LOW;
int mode = 0; // 0:開始前 1:動作中 2:終了

void setup()
{
  delay(1000);
  pinMode(PIN_SPI_MOSI, OUTPUT);
  pinMode(PIN_SPI_MISO, INPUT);
  pinMode(PIN_SPI_SCK, OUTPUT);
  pinMode(PIN_SPI_SS, OUTPUT);
  pinMode(PIN_BUSY, INPUT);
  pinMode(PIN_FLOWER, INPUT_PULLUP);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  SPI.begin();
  SPI.setDataMode(SPI_MODE3);
  SPI.setBitOrder(MSBFIRST);
  Serial.begin(9600);
  digitalWrite(PIN_SPI_SS, HIGH);

  L6470_resetdevice(); //L6470リセット
  L6470_setup();  //L6470を設定

  MsTimer2::set(50, fulash);//シリアルモニター用のタイマー割り込み
  MsTimer2::start();
  delay(4000);

  /*
  Serial.println("move start.");
  L6470_run(0, 100000); //指定方向に連続回転
  delay(20000);

  L6470_softstop();//回転停止、保持トルクあり
  Serial.println("move stopped.");
  delay(2000);
  Serial.println("move to zero.");
  L6470_goto(0);//指定座標に最短でいける回転方向で移動
  //L6470_gohome();//座標原点に移動

  L6470_softstop();//回転停止、保持トルクあり
  Serial.println("move stopped.");
  L6470_hardhiz();//回転急停止、保持トルクなし
  Serial.println("move hardstopped.");
  */
  /*
    Serial.println("move start.");
    L6470_move(1,1600);//指定方向に指定数ステップする
    L6470_busydelay(5000); //busyフラグがHIGHになってから、指定ミリ秒待つ。
    L6470_run(0,10000);//指定方向に連続回転
    delay(6000);
    L6470_run(1,20000);//指定方向に連続回転
    delay(6000);
    Serial.println("move next.");
    L6470_softstop();//回転停止、保持トルクあり
    L6470_busydelay(5000);
    L6470_goto(0x6789);//指定座標に最短でいける回転方向で移動
    L6470_busydelay(5000);
    L6470_run(0,0x4567);
    delay(6000);
    L6470_hardhiz();//回転急停止、保持トルクなし
  */
}

void loop() {
  if (mode == 2) {
    return;
  }
  
  state = digitalRead(PIN_FLOWER);
  button = digitalRead(PIN_BUTTON);
  if (state == HIGH && pre_state == LOW && mode == 0) {
    Serial.println("move start.");
    L6470_run(0, 100000); //指定方向に連続回転
    delay(30000);

    L6470_softstop();//回転停止、保持トルクあり
    Serial.println("move stopped.");
    delay(2000);

    mode = 1;
    
  } else if (button != pre_button && mode == 1) {
    
    Serial.println("move to zero.");
    L6470_goto(0);//指定座標に最短でいける回転方向で移動
    //L6470_gohome();//座標原点に移動

    L6470_softstop();//回転停止、保持トルクあり
    Serial.println("move stopped.");
    L6470_hardhiz();//回転急停止、保持トルクなし
    Serial.println("move hardstopped.");

    mode = 2;
  }
  pre_state = state;
  pre_button = button;
  delay(100);
}

void L6470_setup() {
  L6470_setparam_acc(0x40); //[R, WS] 加速度default 0x08A (12bit) (14.55*val+14.55[step/s^2])
  //L6470_setparam_acc(0x08A); //[R, WS] 加速度default 0x08A (12bit) (14.55*val+14.55[step/s^2])
  L6470_setparam_dec(0x40); //[R, WS] 減速度default 0x08A (12bit) (14.55*val+14.55[step/s^2])
  L6470_setparam_maxspeed(0x40); //[R, WR]最大速度default 0x041 (10bit) (15.25*val+15.25[step/s])
  L6470_setparam_minspeed(0x01); //[R, WS]最小速度default 0x000 (1+12bit) (0.238*val[step/s])
  L6470_setparam_fsspd(0x3ff); //[R, WR]μステップからフルステップへの切替点速度default 0x027 (10bit) (15.25*val+7.63[step/s])
  L6470_setparam_kvalhold(0x20); //[R, WR]停止時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)
  L6470_setparam_kvalrun(0x20); //[R, WR]定速回転時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)
  L6470_setparam_kvalacc(0x20); //[R, WR]加速時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)
  L6470_setparam_kvaldec(0x20); //[R, WR]減速時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)

  L6470_setparam_stepmood(0x03); //ステップモードdefault 0x07 (1+3+1+3bit)
}

void fulash() {
  Serial.print("0x");
  Serial.print( L6470_getparam_abspos(), HEX);
  Serial.print("  ");
  Serial.print("0x");
  Serial.println( L6470_getparam_speed(), HEX);
}
