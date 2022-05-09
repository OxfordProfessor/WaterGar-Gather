底层完全配置完成，只需要修改部分端口与远程服务器端口
*HANDWARE/CAMERA树莓派通信串口
*HANDWARE/sram屏幕信息储存串口FSMC
*HANDWARE/TOUCH触摸屏通信串口T-MISO,T-PEN.....
  |24CXX
  |W25QXX
  均为LCD显示屏上的触摸芯片，不必做修改
  |ILI93XX
  为触摸协议,不必做修改
*HANDWARE/GUI显示屏的显示页面
*HANDWARE/SPI外部Flash端口
  |stmflash
  为芯片与FLASH的通信协议,不必做修改
*HANDWARE/PWM六个电机连接端口
*HANDWARE/SERSOR TDS传感器，PH传感器，超声波传感器连接端口
*FAFTS/sdio_sdcard.c SD卡连接串口
*NET/Esp8266 wifi账号密码，通信串口，服务器端口与地址
*NET/protocol 服务器加密的三元组

ds18b20.c为测试文件，用不到
myiic.c为ds18b20配置文件，用不到
注意内存空间MEM_MAX_SIZE不要太大！否则将内存溢出报错