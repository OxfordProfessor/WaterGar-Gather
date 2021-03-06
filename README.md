## [水面垃圾清理装置](https://github.com/OxfordProfessor/WaterGar-Gather.git)

本程序基于STM32F407芯片的FreeRTOS操作系统，采用正点原子ESP8266-wifi(ESP-01-S系列)作为传输模块，
采用Mqtt网络传输协议，以阿里云物联网平台为云服务器，由微信小程序/App获取传感器信息并操控相关硬件，
可以自动收集水面垃圾、并可以人为辅助控制与APP获取机器的相关数据。


项目的基本思路是通过树莓派调用Opencv库，书写相关识别算法，自动识别水面垃圾，识别后返回物体坐标给STM32，
同时传输视频流给上位机APP（微信小程序），STM32书写协议解析收到的坐标，然后操控机器收取水面垃圾，
操作者可以通过微信小程序辅助控制机器人，同时，水面垃圾机器人还具有低电量及垃圾仓已满自动返回功能，
操作者还可以通过微信小程序获取STM32采集到的水质曲线，方便了解水域信息。

2022-05-11 08:50 V2.2

        1. 完成了所有STM32底层的配置
        2. STM32定义任务栈，并书写要执行的任务
        3. 完成书写触摸屏页面
        4. 完成了微信小程序所有逻辑部分正常书写
        5. 修复了数据更新，但无法及时显示在屏幕上的问题
    待实现：
        2. 根据STM32端完善微信小程序中的动态图表与返回信息，并发布小程序
        3. 完成Opencv获取代码视频流
        4. 修缮上位机摇杆中的触摸问题——触摸点不精准

     
