#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Created on 2022-05-03 20:41
The last revision time on 2022-02-18 10:20

@author: ZhangKaiyang
@QQ: 3173244086
"""

import cv2
import serial
import tkinter
import numpy as np
from matplotlib import pyplot as plt

a = tkinter.Tk(screenName=':1.0')  # 树莓派默认显示设备
# ser = serial.Serial('com3', 115200, timeout=1)  # 使用USB连接串行口,发送数据给stm32
lower_blue = np.array([50, 50, 200])  # 规定寻找物体色彩最低阈值
upper_blue = np.array([130, 255, 255])  # 规定寻找物体色彩最高阈值

def show_img(img, figsize=(6, 6)):  # 显示拉普拉斯变换后的图像
    plt.figure(figsize=figsize)
    plt.imshow(img)
    plt.axis('off')

def Coordinate(frame):  # 找出边缘，并返回图像中心坐标
    frame_ab = frame.copy()      # 复制一份图像
    # 对图像执行harris
    Harris_detector = cv2.cornerHarris(frame_ab, 2, 3, 0.04)
    # 腐蚀harris结果
    dst = cv2.dilate(Harris_detector, None)
    # 设置阈值
    thres = 0.01 * dst.max()
    frame_ab[dst > thres] = 255
    frame_ab[dst < thres] = 0
    # show_img(frame_ab)
    # 形态学处理
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
    frame_opened = cv2.morphologyEx(frame_ab, cv2.MORPH_CLOSE, kernel, iterations=8)  # 开运算1
    # show_img(frame_opened)
    # 边缘检测处理
    frame_edge = cv2.Canny(frame_opened, 50, 200)
    plt.figure(figsize=(16, 5))
    plt.subplot(121)
    # plt.imshow(frame_edge) 将图像存入要显示的图像中
    # 从拉普拉斯变换好的图像中寻找边缘，然后在原图中绘制边框
    contours, hierarchy = cv2.findContours(frame_edge, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
    frame_rec = frame.copy()    # 复制一份
    # 寻找边缘
    if not contours:  # 没找到，返回固定坐标
        # plt.imshow(frame_rec)
        # plt.show()
        cv2.imshow('finally',frame)
        Coordinate_X = 0
        Coordinate_Y = 0
    else:  # 找到边缘
        #   以下注释部分在OPENCV4中不需要有，而在OPENCV2、3中需要
        #        e = len(contours)       # 测量边缘长度
        #        for i in range(e):
        #            if cv2.contourArea(contours[i]) > cv2.contourArea(contours[0]):
        #               contours[0] = contours[i]               # 获取边缘数据
        x, y, w, h = cv2.boundingRect(contours[0])  # 获取最小包围矩形的边长及坐标数据
        # 写出绘制边框要用的数组
        brcnt = np.array([[x, y], [x + w, y], [x + w, y + h], [x, y + h]])
        # 在原图上绘制边框
        cv2.drawContours(frame, [brcnt], -1, (255, 255, 255), 2)
        cv2.imshow('finally',frame)
        # 处理返回坐标
        Coordinate_X = ((2*x)+w)/2
        Coordinate_Y = ((2*y)+h)/2
        # 显示图像
        plt.subplot(122)
        # plt.imshow(frame_rec)     //将图像存入要显示的图像中
        # plt.show()              //展示所有图像
    return Coordinate_X, Coordinate_Y


def SerialTransmission():   # 数据发送函数
    # 由于传输的是两个数据，要让STM32能够识别到分别是X, Y的数据，必须加上相关标志位，然后在STM32上对标志位写处理协议
    data = '#' + str(X) + '$' + str(Y) + '\r\n'
    # 发送数据
#    ser.write(data.encode())


if __name__ == "__main__":  # 主运行函数
    cap = cv2.VideoCapture(0)  # 打开摄像头
#   ser.open()  # 打开串口
    while cap.isOpened():  # 当摄像头成功打开后
        ret, RGB = cap.read()  # 读取图像,捕捉帧，捕捉成功后ret为True，否则为False，RGB为读取的图像
        RGB_Mirror = cv2.flip(RGB, 1)  # 对图像取镜像，保证画面一致性
        # 将RGB图像转化为HSV图像格式
        HSV = cv2.cvtColor(RGB_Mirror, cv2.COLOR_BGR2HSV)
        gray = cv2.cvtColor(HSV, cv2.COLOR_BGR2GRAY)   # 图像处理为灰度图，否则无法寻找边缘
        # 获取坐标数据
        X, Y = Coordinate(gray)
        # 发送数据
        SerialTransmission()
        # 监视器打印物体坐标
        print([X, Y])       
        c = cv2.waitKey(1)
        if c == 27:
            break
#    ser.close()  # 关闭串口
    cap.release()   # 释放视频
    cv2.destroyAllWindows()     # 关闭所有窗口
