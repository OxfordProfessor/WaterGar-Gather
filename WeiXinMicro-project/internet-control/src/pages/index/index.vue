<template>
  <view class="wrapper">
    <view class="header-wrapper">
      <view class="header-title">
        <span>空气质量-良好</span>
        <span>武汉市</span>
      </view>
      <view class="header-text">
        <span>{{Para}}</span>
        <span>阴</span>
      </view>
      <view class="weather-advice">空气质量良好、健康人群外出活动无需可以刻意防护</view>
    </view>
    <view class="body-wrapper">
      <view class="body">
        <view class="data-wrapper">
          <view class="data">
            <img class="data-logo" src="/static/images/PH.png"/>
            <view class="data-text">
              <view class="data-title">PH值</view>
              <view class="data-value">{{ph}}</view>
            </view>
          </view>
          <view class="data-TDS">
            <img class="data-logo" src="/static/images/TDS.png"/>
            <view class="data-text">
              <view class="data-title">TDS值</view>
              <view class ="data-value-unit">
                <view class="data-value">{{tds}}</view>
                <view class="data-unit">mg/L</view>
              </view>
            </view>
          </view>
        </view>
      </view>
    </view>
    <view class="body-wrapper">
      <view class="body">
        <view class="data-wrapper-two">
          <div class="wrap.ph">
            <view class="data-title-PH">水质PH值曲线图</view>
            <mpvue-echarts :echarts="echarts" :onInit="initChart" canvasId="ph"/>
          </div>
        </view>
        <view class="data-wrapper-three">
          <div class="wrap.tds">
            <view class="data-title-TDS">水质TDS值曲线图</view>
            <mpvue-echarts :echarts="echarts" :onInit="onitChart" canvasId="tds"/>
          </div> 
        </view>
      </view>
    </view>
  </view>
</template>

<script>
var mqtt = require('aliyun-mqtt/mqtt.min.js') //根据自己存放的路径修改
const crypto = require('aliyun-mqtt/hex_hmac_sha1.js'); //根据自己存放的路径修改
//import * as echarts from 'echarts'
//import  * as echarts from 'chart/wx-canvas.js'
import * as echarts from 'chart/echarts.min.js'
//import * as echarts from 'chart/echarts.simple.min.js'
import mpvueEcharts from 'chart/echarts.vue'
import global_ from 'Global/Global'
//import mpvueEcharts from 'mpvue-echarts'
//import * as echarts from 'chart/echarts.js'

export default {
  data(){
    return{
      waterinfo:{},
      ischange:'',
      timer: null, // 定时器
      ph: 0,
      tds: 0,
      voltage: 0,
      storage: 0,
      echarts,
      Comeback: 0,
      resdata: [8.7, 6.2, 6.8, 7.3, 9.2, 5.6, 8.3],  //定义一个数组，用来动态传递图表数据
      tdsdata: [301.45, 342.5, 245.3, 287.9, 303.2, 402.5, 305.3]
    }
  },
  methods:{
    initChart (canvas, width, height) {
	  const chart = echarts.init(canvas, null, {
	    width: width,
	    height: height
	  })
	  canvas.setChart(chart)
	  var option = {
	    backgroundColor: '#fff',
	    color: ['#37A2DA', '#67E0E3', '#9FE6B8'],
	    tooltip: {
	      trigger: 'axis'
	    },
	    legend: {
	      data: ['PH曲线']
	    },
	    grid: {
        left: 20,
        right: 20,
        bottom: 10,
        top: 10,
	      containLabel: true
	    },
	    xAxis: {
	      type: 'category',
	      boundaryGap: false,
	      axisLine: {
	        lineStyle: {
	          color: '#ccc'
	        }
	      },
	      data: ['0:00', '4:00', '8:00', '12:00', '16:00', '20:00', '24:00']
	    },
	    yAxis: {
	      x: 'center',
	      type: 'value',
	      min: 0, // 坐标轴最小值
	      max: 14,
	      scale: true,
	      // 坐标轴的颜色
	      axisLine: {
	        lineStyle: {
	          color: '#ccc'
	        }
	      }
	    },
	    series: [{
	      name: '水质PH值曲线',
	      type: 'line',
	      smooth: true, // true是曲线，false是直线
	      data: this.resdata,
	      // 设置折线区域颜色
	      areaStyle: {normal:{}},
	      itemStyle: {
	        normal: {
	          color: new echarts.graphic.LinearGradient(
	            0, 0, 0, 1,
	            [
	              {offset: 0, color: 'skyblue'}
	            ]
	          )
	        }
	      }
	    }]
	  }
	  chart.setOption(option,true)
	  return chart
	},
	onitChart (canvas, width, height) {
	  const chart = echarts.init(canvas, null, {
	    width: width,
	    height: height
	  })
	  canvas.setChart(chart)
	  var option = {
	    backgroundColor: '#fff',
	    color: ['#37A2DA', '#67E0E3', '#9FE6B8'],
	    tooltip: {
	      trigger: 'axis'
	    },
	    legend: {
	      data: ['TDS曲线']
	    },
	    grid: {
        left: 20,
        right: 20,
        bottom: 10,
        top: 10,
	      containLabel: true
	    },
	    xAxis: {
	      type: 'category',
	      boundaryGap: false,
	      axisLine: {
	        lineStyle: {
	          color: '#ccc'
	        }
	      },
	      data: ['0:00', '4:00', '8:00', '12:00', '16:00', '20:00', '24:00']
	    },
	    yAxis: {
	      x: 'center',
	      type: 'value',
	      min: 0, // 坐标轴最小值
	      max: 600,
	      scale: true,
	      // 坐标轴的颜色
	      axisLine: {
	        lineStyle: {
	          color: '#ccc'
	        }
	      }
	    },
	    series: [{
	      name: '水质TDS值曲线',
	      type: 'line',
	      smooth: true, // true是曲线，false是直线
	      data: this.tdsdata,
	      // 设置折线区域颜色
	      areaStyle: {normal:{}},
	      itemStyle: {
	        normal: {
	          color: new echarts.graphic.LinearGradient(
	            0, 0, 0, 1,
	            [
	              {offset: 0, color: 'green'}
	            ]
	          )
	        }
	      }
	    }]
	  }
	  chart.setOption(option,true)   //这里的true一定要有，不然的话会导致你的图表不根据数据的变化进行更新
	  return chart
	},
  },
  
  mounted(){
    
  },
  components: {
    mpvueEcharts,
  },
  onShow(){
    let that = this
    this.timer = setInterval(function() {
      that.ph = global_.ph
      that.tds = global_.tds
	  }, 3000)
  },
  onLoad: function(options){    //接收页面传参

  },
} 
</script>


<style lang="scss" scoped>
.wrapper{
  padding: 15px;
  .header-wrapper{
    background-color: #3d7ef9;
    border-radius: 20px;
    color: #fff;
    box-shadow: #d6d6d6 0px 0px 5px;
    padding: 15px 30px;
    .header-title{
      display: flex;
      justify-content: space-between;
    }
    .header-text{
      font-size: 32px;
      font-weight: 400;
      display: flex;
      justify-content: space-between;
    }
    .weather-advice{
      margin-top: 20px;
      font-size: 12px;
    }
  }
  .data-wrapper{
    margin-top: 20px;
    display: flex;
    justify-content: space-between;
    .data{
      background-color: #fff;
      width: 150px;
      height: 80px;
      border-radius: 20px;
      display: flex;
      justify-content: space-around;
      padding: 0 8px;
      box-shadow: #d6d6d6 0px 0px 5px;
      .data-logo{
        height: 60px;
        width: 60px;
        margin-top: 15px;
      }
      .data-text{
        margin-top: 15px;
        color: #7f7f7f;
        .data-value{
          font-size: 26px;
        }
      }
    }
    .data-TDS{
      background-color: #fff;
      width: 150px;
      height: 80px;
      border-radius: 20px;
      display: flex;
      justify-content: space-around;
      padding: 0 8px;
      box-shadow: #d6d6d6 0px 0px 5px;
      .data-logo{
        height: 60px;
        width: 60px;
        margin-top: 15px;
      }
      .data-text{
        margin-top: 15px;
        color: #7f7f7f;
        .data-value-unit{
          display: flex;
          justify-content: space-between;
          .data-value{
            font-size: 26px;
          }
          .data-unit{
            font-size: 15px;
          }
        }
      }
    }
  }
  .data-wrapper-two{
    width: 350px;
    height: 360px;
    margin-top: 20px;
    background-color: #fff;
    border-radius: 20px;
    color: #fff;
    box-shadow: #d6d6d6 0px 0px 10px;
    padding: 10px 0px;
    .wrap.ph{
      width: 100%;
      height: 300px;
      .data-title-PH{
      text-align:center;
      align-items:center;
      justify-content: center;
      padding: 10px;
      margin-top: 5px;
      font-size: 20px;
      color: #7f7f7f;
      }
    }
  }
  .data-wrapper-three{
    width: 350px;
    height: 360px;
    margin-top: 30px;
    background-color: #fff;
    border-radius: 20px;
    color: #fff;
    box-shadow: #d6d6d6 0px 0px 5px;
    padding: 10px 0px;
    .wrap.tds{
      width: 100%;
      height: 300px;
      .data-title-TDS{
        text-align:center;
        align-items:center;
        justify-content: center;
        padding: 10px;
        margin-top: 5px;
        font-size: 20px;
        color: #7f7f7f;
      }
    }
  }
}

</style>