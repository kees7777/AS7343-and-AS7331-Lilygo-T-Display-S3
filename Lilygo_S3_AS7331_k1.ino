/*
    //  in USER_SETUP_ID 206 >  #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
    //  flash 16MB / ST7789  / 170 x 320 / 3,3 volt / 
    // connector  GND , 3V , 43 , 44
    // LILYGO T-DISPLAY S3
    // ADC2 gaat niet samen met WIFI

    Board	              ESP32S3 Dev Module
    Port	              Your port
    USB CDC On Boot	    Enable
    CPU Frequency	      240MHZ(WiFi)
    Core Debug Level	  None
    USB DFU On Boot	    Disable
    Erase All Flash Before Sketch Upload	    Disable
    Events Run On	      Core1
    Flash Mode	        QIO 80MHZ
    Flash Size	        16MB(128Mb)
    Arduino Runs On	    Core1
    USB Firmware MSC On Boot	      Disable
    Partition Scheme	  16M Flash(3M APP/9.9MB FATFS)
    PSRAM	              OPI PSRAM
    Upload Mode	        UART0/Hardware CDC
    Upload Speed	      921600
    USB Mode	          CDC and JTAG

*/

#define DEBUGMODE 0// 0 of 1 uitzetten als definitief

#include <Arduino.h>
#include <SparkFun_AS7331.h>
#include <SparkFun_AS7343.h>
#include <Wire.h>

#include <TFT_eSPI.h>
TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd); 
TFT_eSprite sprite2 = TFT_eSprite(&lcd); 

SfeAS7331ArdI2C myUVSensor;
SfeAS7343ArdI2C mySensor;

#define adc_BATTERIJ  4  // 1/2 x Vsupp. = ± 3.3
#define BUTTON_LEFT   0 // BOOT BUTTON
#define BUTTON_RIGHT  14
#define BACKLIGHT 38    // PIN ACHTERGROND VERLICHTING TFT > PWM

uint16_t bsx = 0, bsy = 0, barbreed = 20 , barhoog = 135 ,  val ;

uint16_t myData[18]; // Array to hold spectral data

int foutje ;
boolean leduit = 0 , ledaan = 1 ;
uint8_t UVGain = 11 ;

as7331_gain_t UVgainOptions[] = {
    GAIN_2048, // 0
    GAIN_1024,
    GAIN_512,
    GAIN_256,// 3
    GAIN_128,
    GAIN_64,
    GAIN_32,// 6
    GAIN_16,
    GAIN_8,
    GAIN_4,
    GAIN_2,
    GAIN_1
};

// as7331_conv_time_t UV_timeoptions[] = {
//     TIME_1MS ,
//     TIME_2MS,
//     TIME_4MS,
//     TIME_8MS,
//     TIME_16MS,
//     TIME_32MS,
//     TIME_64MS,
//     TIME_128MS,
//     TIME_256MS,
//     TIME_512MS,
//     TIME_1024MS,
//     TIME_2048MS,
//     TIME_4096MS,
//     TIME_8192MS,
//     TIME_16384MS
// } ;


// sfe_as7343_again_t gainSettings[] = {
//     AGAIN_0_5,
//     AGAIN_1,
//     AGAIN_2,
//     AGAIN_4,
//     AGAIN_8,
//     AGAIN_16,
//     AGAIN_32,
//     AGAIN_64,
//     AGAIN_128,
//     AGAIN_256,
//     AGAIN_512,
//     AGAIN_1024,
//     AGAIN_2048}
// ;

const char *labels[21] = {"855", "745",  "690",  "640",  "600",  "555",  "550",
   "515",  "475", "450",  "425",  "405",  "350","300","250","VIS" ,
   "UV-C","UV-B","UV-A","n-IR"};

const uint16_t kleur[18] = {
    0xFC9F, // 0 NIR 855nm pink
    0x8000, // 1 F8  745nm dark red
    0xC000, // 2 F7  690nm deep red
    0xF800, // 3 F6  640nm red
    0xFC00, // 4 FXL 600nm orange
    0xFFE0, // 5 FY  555nm yellow
    0xAFE0, // 6 F5  550nm yellow-green
    0x07E0, // 7 F4  515nm green
    0x07FF, // 8 F3  475nm cyan-blue
    0x001F, // 9 FZ  450nm blue
    0x000F, // 10 F2  425nm navy

    0x780F, // 11 F1  405nm violet UVA

    0x915C, // 12  350nm UVA
    0xB15C, // 13  300nm UVB
    0xE15C, // 14  250nm UVC

    0x7BEF, // 15  Visable
};

void leesUV(void){
  // Send a start measurement command.
    if (ksfTkErrOk != myUVSensor.setStartState(true))

    #if DEBUGMODE 
        Serial.println("Error starting reading UV module!");
    #endif

    // Wait for a bit longer than the conversion time.
    delay(2 + myUVSensor.getConversionTimeMillis());

    // Read UV values.
    if (ksfTkErrOk != myUVSensor.readAllUV())
        #if DEBUGMODE 
            Serial.println("Error reading UV.");
        #endif

   myData[12]  = myUVSensor.getUVA();
   myData[13]  = myUVSensor.getUVB();
   myData[14]  = myUVSensor.getUVC();
}

void leeskleur(boolean licht ){
    if (licht)  {mySensor.ledOn();  delay(100); }
    
    // Read all data registers
    // if it fails, print a failure message and continue
    if (mySensor.readSpectraDataFromSensor() == false)  {        
        #if DEBUGMODE 
            Serial.println("Failed to read spectral data.");  
        #endif
    }
       
    mySensor.ledOff();

    myData[0]  = mySensor.getChannelData(CH_NIR_855NM);
    myData[1]  = mySensor.getChannelData(CH_DARK_RED_F8_745NM);
    myData[2]  = mySensor.getChannelData(CH_RED_F7_690NM);
    myData[3]  = mySensor.getChannelData(CH_BROWN_F6_640NM);
    myData[4]  = mySensor.getChannelData(CH_ORANGE_FXL_600NM);
    myData[5]  = mySensor.getChannelData(CH_GREEN_FY_555NM);
    myData[6]  = mySensor.getChannelData(CH_GREEN_F5_550NM); myData[6]  = myData[6] * 2.3 ;
    myData[7]  = mySensor.getChannelData(CH_BLUE_F4_515NM);
    myData[8]  = mySensor.getChannelData(CH_LIGHT_BLUE_F3_475NM);
    myData[9]  = mySensor.getChannelData(CH_BLUE_FZ_450NM);
    myData[10]  = mySensor.getChannelData(CH_DARK_BLUE_F2_425NM);
    myData[11]  = mySensor.getChannelData(CH_PURPLE_F1_405NM);

    myData[15]  = mySensor.getChannelData(CH_VIS_1);

}

void lees_uv_temperatuur(void){
    #if DEBUGMODE
        foutje = myUVSensor.readTemp(); // 1 registers uitlezen
        Serial.println(myUVSensor.getTemp()); // 2 opvragen
    #endif
}

void bar_show(){

    sprite.fillSprite(TFT_BLACK) ;

    sprite2.setTextColor(TFT_WHITE  ); 
    sprite2.setTextSize(1);
      

    for (int i = 0; i < 16; i++) {
        sprite.drawRect(bsx + barbreed * i , bsy, barbreed + 1, barhoog + 2 , 0x03EF) ; 
        sprite2.drawString( labels[i] , 142 , 315 - barbreed * i );

    }
    sprite2.setTextColor(TFT_PURPLE ); sprite2.drawString( labels[16] , 140 , 26 );
    sprite2.setTextColor(TFT_VIOLET  ); sprite2.drawString( labels[17] , 140 , 46 );
    sprite2.setTextColor(TFT_BLUE   ); sprite2.drawString( labels[18] , 140 , 66 );
    sprite2.setTextColor(TFT_DARKCYAN   ); sprite2.drawString( labels[18] , 140 , 86 );
    sprite2.setTextColor(TFT_RED  ); sprite2.drawString( labels[19] , 140 , 306 );

    for (int i = 0; i < 12; i++) {  
        if (myData[15] > barhoog ){  val = map(myData[i], 0, myData[15], 0, barhoog)  ;   }
        else                         val = myData[i];
        sprite.fillRect(bsx +1 + barbreed * i, bsy + 1 + barhoog - val  , barbreed -2, val , kleur[i]);
    }

    for (int i = 12; i < 16; i++) {  
        if (myData[i] > barhoog ){ myData[i] = barhoog;  };
        sprite.fillRect(bsx +1 + barbreed * i, bsy + 1 + barhoog - myData[i]  , barbreed-2, myData[i] , kleur[i]); 
    }

    sprite2.pushRotated(& sprite , 90,TFT_BLACK); // text 90 graden geroteerd
    sprite.pushSprite(0,0); 
} 

void printgainUV(void){
        #if DEBUGMODE
        Serial.print("UV Sensor get gain value / get raw  ");
        Serial.print (myUVSensor.getGainValue()) ;Serial.print ("\t" ); 
        Serial.println(myUVSensor.getGainRaw());
        #endif
}
 
void setup() {

  pinMode(BUTTON_RIGHT,INPUT_PULLUP);
  pinMode(BUTTON_LEFT,INPUT_PULLUP);
    #if DEBUGMODE
       Serial.begin(115200);
       while (!Serial)    {        delay(100);    };
       Serial.println("AS7331 UV A/B/C One-shot mode ");
    #endif

    Wire.begin(43,44); // data , clock

//  UV sensor AS7331  ******************************************************************************
    // Initialize sensor and run default setup.
    if (myUVSensor.begin() == false)
        {
        #if DEBUGMODE
            Serial.println("Sensor failed to begin. Please check your wiring!");
            Serial.println("Halting...");
        #endif
        while (1) ;
    }
        #if DEBUGMODE
            Serial.println("UV  Sensor began.");
        #endif

    foutje = myUVSensor.setGain(UVgainOptions[UVGain] ); // set UV gain GAIN_16
    if (foutje == 0){    
        #if DEBUGMODE
        Serial.print("UV gain is set 0=");Serial.println(foutje);     
        #endif
    }

    // Set measurement mode and change device operating mode to measure.
    if (myUVSensor.prepareMeasurement(MEAS_MODE_CMD) == false)
        {
        #if DEBUGMODE
            Serial.println("UV Sensor did not get set properly.");
            Serial.println("Halting...");
        #endif
        while (1)  ;
    }

       #if DEBUGMODE
        Serial.println("Spectral measurement enabled.");
        Serial.print("UV Sensor get gain value / get raw  ");
        Serial.print (myUVSensor.getGainValue()) ;Serial.print ("\t" ); 
        Serial.println(myUVSensor.getGainRaw());
        #endif
//  UV sensor AS7331 end  ****

//  Kleursensor AS7343  ******************************************************************************
    // Serial.println("Set mode to command.");
    // Initialize sensor and run default setup.
    if (mySensor.begin() == false)
        {   
        #if DEBUGMODE
            Serial.println("Color Sensor failed to begin. Please check your wiring!");
            Serial.println("Halting...");
        #endif
        while (1)   ;
    }

        #if DEBUGMODE
            Serial.println("Color  Sensor began.");
        #endif

    // Power on the device
    if (mySensor.powerOn() == false)
    {   
        #if DEBUGMODE
            Serial.println("Failed to power on the device.");
            Serial.println("Halting...");
        #endif
        while (1)  ;
    }
    
    // Set the AutoSmux to output all 18 channels
    if (mySensor.setAutoSmux(AUTOSMUX_18_CHANNELS) == false)
    {   
        #if DEBUGMODE
            Serial.println("Failed to set AutoSmux.");
            Serial.println("Halting...");
        #endif
        while (1) ;
    }
    // Serial.println("AutoSmux set to 18 channels.");

    // Enable Spectral Measurement
    if (mySensor.enableSpectralMeasurement() == false)
    {  
        #if DEBUGMODE
            Serial.println("Failed to enable spectral measurement.");
            Serial.println("Halting...");
        #endif
        while (1) ;
    }
// Kleursensor AS7343  end ***

//  TFT display  ******************************************************************************
    lcd.init(); lcd.setRotation(1); lcd.setSwapBytes(true) ;
    sprite.createSprite(320 , 170 ); sprite.setSwapBytes(true); sprite.fillSprite(TFT_BLACK); 
    sprite.setTextDatum(3); sprite.setFreeFont(); sprite.pushSprite(0,0); sprite.setTextColor(TFT_WHITE);

    sprite2.createSprite(170 , 320);sprite2.setSwapBytes(true); sprite2.fillSprite(TFT_BLACK); 
    sprite2.setTextDatum(3); sprite2.setFreeFont(); 
//  TFT display  end ****
}

void loop() {
   
    leeskleur(leduit);
    leesUV();
    bar_show();
    delay(250);
    // lees_uv_temperatuur();
    if ( !digitalRead(BUTTON_LEFT )) { 
        delay(200);
        if (UVGain < 11 ) UVGain++ ;
        if (UVGain >11 ) UVGain = 11 ;
        foutje = myUVSensor.setOperationMode(DEVICE_MODE_CFG);
        foutje = myUVSensor.setGain(UVgainOptions[UVGain] );
        foutje = myUVSensor.setOperationMode(DEVICE_MODE_MEAS);

    #if DEBUGMODE       
    printgainUV(); Serial.print(UVGain);Serial.print("BUTTON_LEFT  "); Serial.println(myUVSensor.getUVA());
    #endif   
    }
    
    if ( !digitalRead(BUTTON_RIGHT )) { 
        delay(200);
        if (UVGain > 0 ) UVGain-- ;
        if (UVGain > 11 ) UVGain = 0 ;
        foutje = myUVSensor.setOperationMode(DEVICE_MODE_CFG);
        foutje = myUVSensor.setGain(UVgainOptions[UVGain] );
        foutje = myUVSensor.prepareMeasurement(MEAS_MODE_CMD);
        #if DEBUGMODE
        printgainUV(); Serial.print(UVGain); Serial.print("BUTTON_RIGHT  "); Serial.println(myUVSensor.getUVA()); 
        #endif  
    }




}
