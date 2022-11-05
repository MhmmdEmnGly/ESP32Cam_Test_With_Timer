
#define CAMERA_MODEL_AI_THINKER // Has PSRAM     

#include <Base64.h>
#include <ArduinoJson.h>
#include "HTTPClient.h"
#include "Base64.h"
#include <base64.h>
#include "esp_camera.h"
#include <WiFi.h>             
#include "camera_pins.h"
#include <WString.h>

 


/*varibles*/
/*const char* ssid = "HBS";
const char* password = "kayserim38";*/
const char* ssid = "Camlica-b1";
const char* password = "52338038";

const char* herokuapp = "https://gas-reader.herokuapp.com";

String pdata;

const int ledPin = 4;

String response;

//--------------------------------------------------TIMER-----------------------------------

volatile int interruptCounter =0;
int totalInterruptCounter=0;

//----------------------------------------------------------------------------------


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 


void IRAM_ATTR Sayac() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
  
 
}

//-----------------------------------------------------------------------------




void startCameraServer();

WiFiClient wifi;

void setup() {
  
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  Serial.setDebugOutput(true);
  Serial.println();



  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA); //640 x 480
  s->set_brightness(s, 1);     // -2 to 2
  s->set_contrast(s, 1);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 1);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

//---------------------------------------TIMER SETUP --------------------------------------

  //            0.indekssteki timer -> yani timer 1 i kullanıyoruz
  // 80MHZ = 80 000 000 /80 den yani 1milyon mikrosaniye yani 1 saniye
  // true de çalış demek
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &Sayac, true);
  timerAlarmWrite(timer, 3000000, true);
  timerAlarmEnable(timer);

//----------------------------------------------------------------------------
 


}

void loop() {
  
  
  HTTPClient http; 
  http.begin("https://gas-reader.herokuapp.com/base");  
  http.addHeader("Content-Type", "application/json");
  
  Serial.println("making POST request");

//*********

 if (interruptCounter > 0) {

    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
 
    totalInterruptCounter++;

    digitalWrite(ledPin,HIGH); // flash acıldı

    Serial.print("An interrupt as occurred. Total number: ");
    
    Serial.print("Resim Cekildi....");
    Serial.println(totalInterruptCounter);

    pdata = grabImage();
    
  }
  digitalWrite(ledPin,LOW); // flash kapatıldı

  
  //*******************

  String jsonData = "{\"base64\":\"" + pdata + "\"}";

  //Serial.println("Base-64 kodu: " + pdata);
  // burada kod patlıyor.?
  Serial.println("JSOnDATA---->  "+jsonData);
  
  int httpResponseCode = http.POST(jsonData);
 
  if(httpResponseCode>0)
  {
    response = http.getString();                       
    Serial.println(httpResponseCode);   
    Serial.println(response);
  }
  else 
  {
    Serial.printf("Error occurred while sending HTTP POST: %s\n");
    //httpClient.errorToString(statusCode).c_str()   
  }
  
  Serial.print("Status code: ");

  Serial.print("Response: ");
  
  
  Serial.println("Wait five seconds");

  Serial.println(httpResponseCode);   
  Serial.println(response);
  
 // delay(10000);



  // TIMER İLE RESİM CEKME 


}


String grabImage(){

  camera_fb_t* fb = esp_camera_fb_get();
 
  String encoded = base64::encode(fb->buf, fb->len);
  Serial.write(encoded.c_str(), encoded.length());    
  Serial.println();
 
  esp_camera_fb_return(fb);
  
  return encoded;
}

String Photo2Base64() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    return "";
  }

  String imageFile = "data:image/jpeg;base64,";
  char *input = (char *)fb->buf;
  char output[base64_enc_len(3)];
  for (int i = 0; i < fb->len; i++) {
    base64_encode(output, (input++), 3);
    if (i % 3 == 0) imageFile += urlencode(String(output));
  }

  esp_camera_fb_return(fb);

  return imageFile;
}



String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
    yield();
  }
  return encodedString;
}
