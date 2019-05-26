#include <Wire.h>
#include "RTClib.h"
// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <MemoryFree.h>

#define EEP_JOB_SIZE 26//8+3+1+4+2|5|5=26 byte/1 job: Lu 1 list 5 bai, duowc 39 chuong trinh
#define EEP_MIN_SIZE 8
#define EEP_HOURS_SIZE 3
#define EEP_WDAY_SIZE 1
#define EEP_DAY_SIZE 4
#define EEP_MONTH_SIZE 2

#define EEP_LIST_SONG_SIZE 5

#define EEP_JOB_SONG_SIZE 10
#define EEP_JOB_EXPR_SIZE 29//30//so ky tu in duoc  30byte *, byte cuoi la null
//"String"|5|5=30+10+1 byte/1 job: Lu 1 list 5 bai, duowc 24 chuong trinh:"* */15 7 * 1-12 0,1,2,3,4,5,6": Chia theo gio va phut cho duoc nhieu chuong trinh nhat
#define EEP_MAX_JOB 23//

// These are the pins used for the breakout example
#define BREAKOUT_RESET  8      // VS1053 reset pin (output)
#define BREAKOUT_CS     6     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    7      // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 9     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 2       // VS1053 Data request, ideally an Interrupt pin


//#define UART_SHOW_DEBUG true


Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
// create shield-example object!
//Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
///

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
unsigned long previousMillis = 0;        // will store last time LED was updated


typedef struct {
  uint8_t seconds[8];
  uint8_t minutes[8];
  uint8_t hours[3];
  uint8_t days_of_week[1];
  uint8_t days_of_month[4];
  uint8_t months[2];
} cron_expr;

typedef struct {
  uint8_t song_index;
  uint8_t song_volume;
} song_play;


bool setupComplete = true;  // bien chuong trinh bao hieu ket thuc set up bang cach tat di bat lai
RTC_DS1307 rtc;
DateTime now;
cron_expr expr;


song_play song_list_timeline[EEP_LIST_SONG_SIZE];//Ham luu danh sach cuoi cung de chay
uint8_t curr_song_index = 0;//Bai hat dang chay
int last_job_match = -2; //Chuong trinh chay cuoi cung
int curr_job_running = -1;//Chuong trinh duoc len lich dang chay. Neu job trung ma dang play nhac thi tiep tuc play ko can lam lai tu dau
boolean play_list_done = false;//co thong bao xem da chay xong lít chua. chay xong roi thi moi can lap lai
boolean curr_job_is_running = false;//

void playing_list()
{
  if (!musicPlayer.playingMusic) {//Se co loi neu dang chay bai hat nghe thu thi no se ko phat laij
    song_play song_rev = song_list_timeline[curr_song_index];
    if (song_rev.song_index != 255)//Neu list ko duoc ghi se hien 255
    {
      playSongVolume(song_rev.song_volume);
      playSongIndex(song_rev.song_index);
      curr_song_index++;//Tang len de lan toi chay
    }
    else
    {
      play_list_done = true;//Can play lai neu can
      curr_job_running = -1;  //Neu da xong het list thi reset lai toan bo chuong trinh:curr_job_running
    }
  }
}

void check_cron_running()
{
  int job = 0;
  boolean cron_match;
  char job_rev[EEP_JOB_EXPR_SIZE + 1];
  last_job_match = -2;//rst last_job
  for (job = 0;  job < EEP_MAX_JOB; job++) {
    get_job_expr_expr(job, job_rev );
    #ifdef UART_SHOW_DEBUG
   
      Serial.print(F("Cheking: "));
      Serial.print(F("\""));
      Serial.print(job_rev);
      Serial.print(F("\""));
    
    #endif
    const char* err = NULL;
    memset(&expr, 0, sizeof(expr));
    // cron_parse_expr("* */15 1,2,3,4,5,6,7,8,9,10 * 1-12 0-5", &expr, &err);
    // cron_parse_expr("*/10 * * * 1-12 0-6", &expr, &err);//10 giay 1 lan,Thang 1-12, chu nhat den thu 7
    cron_parse_expr(job_rev, &expr, &err);
    if (err)  {
      #ifdef UART_SHOW_DEBUG
        //      char *err_show;
        //      err_show = (char*)calloc(50, sizeof(char));
        Serial.print(F(" -> Error:"));
        Serial.println(err);
        //      getError(err, err_show);
        //      Serial.println(err_show);
        //      free( err_show );
      #endif
    }
    else
    {
      cron_match = check_cron_match(&expr, now.second(), now.minute(), now.hour(), now.day(), now.month(), now.dayOfTheWeek());
      if (cron_match)
      {
        #ifdef UART_SHOW_DEBUG
        Serial.println(F(" -> Match_cron"));
        #endif
        last_job_match = job;
        get_eep_job_song(job, song_list_timeline);
        //
        //        Serial.print(F("last_job_match:")); Serial.println(last_job_match) ;
        //        Serial.print(F("curr_job_running:")); Serial.println(curr_job_running) ;
        //        Serial.print(F("play_list_done:")); Serial.println(play_list_done) ;

        if ( play_list_done == true) //Neu no da ket thuc roi thi resset lai o day
          curr_job_running = -1;
        break;//Bo qua nhung cai sau luon. chi chay cai dau tien
      }
      else 
      {
        #ifdef UART_SHOW_DEBUG
        Serial.println(F(" ->Not_match!!!"));
        #endif
      }
    }
  }
  //Can them mot cai de thay doi current job neu cai cuoi cung no chayj boi trung voi cai lastjob
  if ((last_job_match != curr_job_running) && ( last_job_match != -2)) //Neu co nhieu cai trung nhau thi chi chay cai cuoi cung phu hop va phai la cai moi. Nếu là cái cũ thì cứ chạy tiếp
  {
    int i;
    #ifdef UART_SHOW_DEBUG
    Serial.print(F("(new):"));
    #endif
    curr_job_running = last_job_match;
    curr_song_index = 0;//Reset bien dau can chat
    //Hien danh sach bai hat
    #ifdef UART_SHOW_DEBUG
    Serial.print(F("\""));
    #endif
    for (i = 0; i < EEP_LIST_SONG_SIZE; i++)
    {
      song_play song_rev = song_list_timeline[i];
      if (song_rev.song_index != 255)//Neu list ko duoc ghi se hien 255
      {
        #ifdef UART_SHOW_DEBUG
        if (i > 0)Serial.print(F("|"));
        Serial.print(song_rev.song_index);
        Serial.print(F("-"));
        Serial.print(song_rev.song_volume);
        #endif
      }
      else break;
    }
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("\""));
    #endif
    stopSong();//dung moi cai cu
    play_list_done = false;
  }
  //

}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Init..."));
  Serial.println(F("[START]"));
  Serial.println(F("328P-Version:1.0.0"));
  Serial.println(F("[STOP]"));
  // reserve 45 bytes for the inputString:
  inputString.reserve(45);
  rtc_time_init();

  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    #endif
    while (1);
  }
  #ifdef UART_SHOW_DEBUG
  Serial.println(F("VS1053 found"));
  #endif
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  if (!SD.begin(CARDCS)) {
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("SD failed, or not present"));
    #endif
    while (1);  // don't do anything more
  }
  #ifdef UART_SHOW_DEBUG
  Serial.println(F("SD OK!"));
  #endif
  
//  getSongList("/"); // list files

  Serial.println(F("[START]"));
  getSongList("/"); // list files
  Serial.println(F("[STOP]"));
  
  musicPlayer.setVolume(20, 20);// Set volume for left, right channels. lower numbers == louder volume!

  // This option uses a pin interrupt. No timers required! But DREQ
  // must be on an interrupt pin. For Uno/Duemilanove/Diecimilla
  // that's Digital #2 or #3
  // See http://arduino.cc/en/Reference/attachInterrupt for other pins
  // *** This method is preferred
  
  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
  {
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("DREQ pin is not an interrupt pin"));
    #endif
  }
  now = rtc.now();
//  test_job_save();
//excuteResetAllJob();

}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    if (musicPlayer.playingMusic) {
      #ifdef UART_SHOW_DEBUG
      Serial.println(F("+++++++++++++++++++++++"));
      #endif
    }
    else {
      #ifdef UART_SHOW_DEBUG
      Serial.println(F(">>>>>>>>>>>>>>>>>>>>>>>"));
      #endif
    }
    now = rtc.now();
    #ifdef UART_SHOW_DEBUG
    Serial.print(F("Since midnight 1/1/1970 = "));
    Serial.println(now.unixtime());
    Serial.println(F("--------------------"));;
    #endif
    if(setupComplete)check_cron_running();//chi chay trong khi khong phai che do setup
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("--------------------"));
    #endif
//    //
//     Serial.print(F("MemFree="));
//     Serial.println(freeMemory());
//    //
  }
  if(setupComplete)playing_list();//Chay list moi

  if (stringComplete) {
    checkCommand(inputString );

    inputString = "";// clear the string:
    stringComplete = false;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  setupComplete=false;//chua co co dat la
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {//<enter>
      stringComplete = true;
    }
  }
}
