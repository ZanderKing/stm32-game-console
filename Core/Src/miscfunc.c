/*
* miscfunc.c
*
*  Created on: Oct 24, 2025
*      Author: alvin
*/
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "miscfunc.h"
#include "../../Drivers/MY_ADDITION/wifi/wifi_all.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef  hdma_usart1_tx;
extern ES_WIFIObject_t EsWifiObj;
extern UART_HandleTypeDef hDiscoUart;
extern ES_WIFI_APs_t APs;
extern ES_WIFI_APConfig_t ApConfig;

#define UART_TX_BUF_SIZE 128
char uart_tx_buf[UART_TX_BUF_SIZE];
volatile uint8_t uart_tx_busy = 0;
LiveIMU_t liveIMU = {0,0,0,0};


void uart_printf(const char *fmt, ...)
{
	uint32_t time = HAL_GetTick();
	while (uart_tx_busy) {
	    if (HAL_GetTick() - time > 50){
	    	break;
	    }
	}
   va_list args;
   va_start(args, fmt);
   vsnprintf(uart_tx_buf, sizeof(uart_tx_buf), fmt, args);
   va_end(args);
   HAL_StatusTypeDef st = HAL_UART_Transmit_DMA(&huart1,(uint8_t*)uart_tx_buf, strlen(uart_tx_buf));
   if (st == HAL_OK) {
       uart_tx_busy = 1;
   }
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
   if (huart == &huart1){
   	uart_tx_busy = 0;
   }
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
   if (huart == &huart1){
   	uart_tx_busy = 0;
   }
}
void WiFi(void)
{  WIFI_Status_t st;
  st = WIFI_Init();
  if (st != WIFI_STATUS_OK) return;
  char ssid[] = "YOUR_WIFI_SSID";
  char pass[] = "YOUR_WIFI_PASSWORD";
  WIFI_Connect(ssid, pass, WIFI_ECN_WPA2_PSK);
  uint8_t ip[4];
  WIFI_GetIP_Address(ip, sizeof(ip));
  uart_printf("IP: %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  WIFI_StartServer(0, WIFI_TCP_PROTOCOL, 1, "", 80);
  //HAL_Delay(5000); // this is needed to let other devices
}
const char dashboardPage[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/html; charset=UTF-8\r\n"
	"Connection: close\r\n\r\n"
	"<!DOCTYPE html><html><head>"
	"<meta name='viewport' content='width=device-width,initial-scale=1.0'>"
	"<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
	"<style>"
	"body{margin:0;padding:20px;background:#000;color:#ED1B76;font-family:Helvetica;text-align:center;}"
	"h1{color:#ED1B76;font-size:22px;margin-bottom:10px;font-weight:700;}"
	".row{display:grid;gap:10px;max-width:900px;margin:0 auto;}"
	".card{background:#111;border-radius:10px;padding:14px;min-width:120px;}"
	".val{color:#ED1B76;font-size:20px;font-weight:bold;}"
	".lab{font-size:12px;opacity:.7;margin-top:3px;color:#ED1B76;}"
	".charts{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:10px;max-width:900px;margin:25px auto;}"
	".chartBox{background:#111;border-radius:10px;padding:10px;}"
	"canvas{width:100%;height:180px;}"
	"</style></head><body>"
	"<h1>Sotong Games: Frontman's Dashboard</h1>"
	"<div class='row'>"
	"<div class='card'><div id='roll' class='val'>0</div><div class='lab'>Roll</div></div>"
	"<div class='card'><div id='pitch' class='val'>0</div><div class='lab'>Pitch</div></div>"
	"<div class='card'><div id='yaw' class='val'>0</div><div class='lab'>Yaw</div></div>"
	"<div class='card'><div id='heading' class='val'>0</div><div class='lab'>Heading</div></div>"
	"<div class='card'><div id='temp' class='val'>--</div><div class='lab'>Temp °C</div></div>"
	"<div class='card'><div id='hum' class='val'>--</div><div class='lab'>Humidity %</div></div>"
	"<div class='card'><div id='pres' class='val'>--</div><div class='lab'>Pressure hPa</div></div>"
	"<div class='card'><div id='phase' class='val'>--</div><div class='lab'>Phase,for G1</div></div>"
	"<div class='card'><div id='alive' class='val'>--</div><div class='lab'>Player</div></div>"
	"<div class='card'><div id='game' class='val'>--</div><div class='lab'>Game</div></div>"
	//"<div class='card'><div id='dur' class='val'>--</div><div class='lab'>Duration(ms)</div></div>"
	"</div>"
	"<div class='charts'>"
	"<div class='chartBox'><canvas id='rollChart'></canvas></div>"
	"<div class='chartBox'><canvas id='pitchChart'></canvas></div>"
	"<div class='chartBox'><canvas id='yawChart'></canvas></div>"
	"<div class='chartBox'><canvas id='tempChart'></canvas></div>"
	"<div class='chartBox'><canvas id='humChart'></canvas></div>"
	"<div class='chartBox'><canvas id='presChart'></canvas></div>"
	"</div>"
	"<script>"
	"function makeChart(id,label,color){"
	"return new Chart(document.getElementById(id).getContext('2d'),{"
	"type:'line',data:{labels:[],datasets:[{label:label,data:[],borderColor:color,borderWidth:1.5,fill:false,tension:0.25}]},"
	"options:{plugins:{legend:{labels:{color:'#ED1B76'}}},scales:{x:{ticks:{color:'#888'}},y:{ticks:{color:'#888'}}}}});}"
	"const rollChart=makeChart('rollChart','Roll (°)','rgba(237,27,118,0.9)');"
	"const pitchChart=makeChart('pitchChart','Pitch (°)','rgba(0,224,255,0.9)');"
	"const yawChart=makeChart('yawChart','Yaw (°)','rgba(255,215,0,0.9)');"
	"const tempChart=makeChart('tempChart','Temperature (°C)','rgba(255,99,132,0.9)');"
	"const humChart=makeChart('humChart','Humidity (%)','rgba(0,255,180,0.9)');"
	"const presChart=makeChart('presChart','Pressure (hPa)','rgba(150,120,255,0.9)');"
	"function addPoint(chart,val){const now=new Date().toLocaleTimeString();"
	"chart.data.labels.push(now);chart.data.datasets[0].data.push(val);"
	"if(chart.data.labels.length>20){chart.data.labels.shift();chart.data.datasets[0].data.shift();}"
	"chart.update();}"
	"function upd(){fetch('/live').then(r=>r.json()).then(j=>{"
	"document.getElementById('roll').textContent=j.roll.toFixed(2);"
	"document.getElementById('pitch').textContent=j.pitch.toFixed(2);"
	"document.getElementById('yaw').textContent=j.yaw.toFixed(2);"
	"document.getElementById('heading').textContent=j.heading.toFixed(2);"
	"document.getElementById('temp').textContent=j.temp.toFixed(1);"
	"document.getElementById('hum').textContent=j.hum.toFixed(1);"
	"document.getElementById('pres').textContent=j.pres.toFixed(1);"
	"document.getElementById('phase').textContent=(j.mode?'🔴 Red Light':'🟢 Green Light');"
	"document.getElementById('game').textContent=(j.game === 1 ? 'Red Light Green Light' : j.game === 2 ? 'Catch and Run':'Unknown');"
	//"document.getElementById('dur').textContent=j.dur.toFixed(0);"
	"let a=document.getElementById('alive');a.textContent=(j.alive?'Alive ✅':'Dead ❌');a.style.color=(j.alive?'#00FF88':'#FF4444');"
	"addPoint(rollChart,j.roll);addPoint(pitchChart,j.pitch);addPoint(yawChart,j.yaw);"
	"addPoint(tempChart,j.temp);addPoint(humChart,j.hum);addPoint(presChart,j.pres);"
	"}).catch(e=>console.log(e));}"
	"setInterval(upd,1000);upd();"
	"</script>"
	"</body></html>";

void WiFi_Process(void)
{
  uint8_t rip[4];
  uint16_t rport;
  uint8_t rx[512];
  uint16_t rx_len;
  if (WIFI_WaitServerConnection(0, 5, rip, sizeof(rip), &rport) != WIFI_STATUS_OK)
      return;
  WIFI_ReceiveData(0, rx, sizeof(rx), &rx_len, 400);
  rx[rx_len] = 0;
  if (strstr((char*)rx, "GET /live") != NULL)
  {
      char jsonBuf[256];
      int len = snprintf(
          jsonBuf, sizeof(jsonBuf),
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: application/json\r\n"
          "Connection: close\r\n\r\n"
          "{"
          "\"roll\":%.2f,"
          "\"pitch\":%.2f,"
          "\"yaw\":%.2f,"
          "\"heading\":%.2f,"
          "\"temp\":%.2f,"
          "\"hum\":%.2f,"
          "\"pres\":%.2f,"
          "\"mode\":%d,"
          "\"alive\":%d,"
          "\"game\":%d"
         // "\"dur\":%lu"
          "}",
          liveIMU.roll_deg,
          liveIMU.pitch_deg,
          liveIMU.yaw_deg,
          liveIMU.heading_deg,
          liveIMU.temp_c,
          liveIMU.hum_pct,
          liveIMU.pres_hpa,
          liveIMU.mode,
          liveIMU.alive,
          liveIMU.lastGame
          //(unsigned long)liveIMU.duration_ms
      );
      uint16_t sent = 0;
      WIFI_SendData(0, (uint8_t*)jsonBuf, len, &sent, 5);
  }
  else // Send Dashboard over first
  {
      uint16_t total = strlen(dashboardPage);
      uint16_t sent = 0;
      while(sent < total) // sometimes TCP needs multiple sends
      {
          uint16_t chunk = total - sent;
          if(chunk > 512) chunk = 512;
          uint16_t pushed = 0;
          WIFI_SendData(0, (uint8_t*)(dashboardPage+sent), chunk, &pushed, 50);
          sent += pushed;
      }
  }
  WIFI_CloseServerConnection(0);
}



