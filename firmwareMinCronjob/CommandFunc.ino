
#define CMD_STOP_PLAY_SONG                  "R0"//<R0><>
#define ACTION_STOP_PLAY_SONG         0
#define CMD_PLAY_SONG_INDEX                 "R1"//<R1><11>
#define ACTION_PLAY_SONG_INDEX        1
#define CMD_PLAY_SONG_NAME                  "R2"//<R2><song_name>--songname <13 char
#define ACTION_PLAY_SONG_NAME         2
#define CMD_SET_VOLUME                      "R3"//<R3><90>
#define ACTION_SET_VOLUME             3
//
#define CMD_SET_JOB_TIME                     "S1"//<S1><0,*/10 * * * * *>
#define ACTION_SET_JOB_TIME           4
#define CMD_RESET_JOB_LIST                   "S2"//<S2><0>
#define ACTION_RESET_JOB_LIST         5
#define CMD_SET_JOB_SONG                     "S3"//<S3><0,1:1-1>
#define ACTION_SET_JOB_SONG           6
#define CMD_SET_TIME_STAMP                   "S4"//<S4><1536564895>
#define ACTION_SET_TIME_STAMP         7
#define CMD_RESET_ALL_JOB                    "S5"//<S5><>
#define ACTION_RESET_ALL_JOB          11



//
#define CMD_GET_JOB_TIME                     "G1"//<G1><0>
#define ACTION_GET_JOB_TIME           8
#define CMD_GET_JOB_PLAY_LIST                "G2"//<G2><0>
#define ACTION_GET_JOB_PLAY_LIST      9
#define CMD_GET_PLAY_LIST                    "G3"//<G3><>
#define ACTION_GET_PLAY_LIST          10

#define START_RESPONSE                        "[START]"
#define STOP_RESPONSE                         "[STOP]"
#define OK_RESPONSE                           "[START]OK[STOP]"
#define ERROR_RESPONSE                        "[START]ERROR[STOP]"


boolean parseCommand(String cmd, String pattern)
{
  return cmd.equals(pattern);
}
void excuteStopSong()
{
  stopSong();
  Serial.println(F(OK_RESPONSE));
}
void excuteSetVolume(String data)//volume chay tu 1-100
{
  uint8_t volume = data.toInt();
  playSongVolume(volume);
  Serial.println(F(OK_RESPONSE));
}
void excuteSetTimeStamp(String data)
{
  char c[data.length() + 1];
  data.toCharArray(c, data.length() + 1);

  long time_set = atol(c);//strtol(c, NULL, 16); ;
  setTime(time_set);
  Serial.println(F(OK_RESPONSE));
}

void excuteGetPlayList()//Sau khi goi ham nay se khong play duoc vai hat lon hon 9. Hoat dong uart bi tat
{
//  stopSong();//
  Serial.println(F(START_RESPONSE));
  delay(100);
  getSongList("/"); // list files
  Serial.println(F(STOP_RESPONSE));
}
void excutePlaySongIndex(String data)//index
{
//  stopSong();
    uint8_t index_set = data.toInt();
    playSongIndex(index_set);
    Serial.println(F(OK_RESPONSE));

}

//void excutePlaySongIndexWithVolume(String data)//index,volume//Thieu Ram nen cat bot cho nhan index
//{
////  stopSong();
//
//  uint8_t firstOpenBracket = data.indexOf(',');
//  String indexStr = data.substring(0, firstOpenBracket);
//  Serial.println(indexStr);
//  String volumeStr = data.substring(firstOpenBracket + 1);
//  Serial.println(volumeStr);
//  if (indexStr.length())
//  {
//    uint8_t index_set = indexStr.toInt();
//    playSongIndex(index_set);
//  }
//  if (volumeStr.length())  {
//    uint8_t volume_set = volumeStr.toInt();
//    playSongVolume(volume_set);
//  }
//  else
//  {
//    playSongVolume(70);
//  }
//  Serial.println(F(OK_RESPONSE));
//}



void excuteSaveTime(String data)//0,* * * * * *
{
  uint8_t firstOpenBracket = data.indexOf(',');
  String indexStr = data.substring(0, firstOpenBracket);
  String cronStr = data.substring(firstOpenBracket + 1);

  if (indexStr.length() && cronStr.length())
  {
    uint8_t expr_size = cronStr.length() + 1;//=1 do add them 1 o nho de null
    char *expr;//Khai bao
    expr = (char*)malloc(sizeof(char) * expr_size); //Cap phat o nho va tro den vi tri dau
    cronStr.toCharArray(expr, expr_size);
    uint8_t index_set = indexStr.toInt();
    //Todo: Add error for index over and expr lenght over
    save_eep_job_expr(index_set, expr);
    free(expr);
    Serial.println(F(OK_RESPONSE));
  }
}
void excuteGetTime(String data)//0
{
  uint8_t firstOpenBracket = data.indexOf(',');
  String indexStr = data.substring(0, firstOpenBracket);
  if (indexStr.length())
  {
    uint8_t index_set = indexStr.toInt();
    char job_rev[EEP_JOB_EXPR_SIZE + 1];
    get_job_expr_expr(index_set, job_rev );

    Serial.println(F(START_RESPONSE));
    //    Serial.print(F("\""));
    delay(10);
    Serial.print(job_rev);
    //        Serial.print(F("\""));
    Serial.println();
    delay(10);
    Serial.println(F(STOP_RESPONSE));
    delay(10);

  }
}
//void excuteSaveListC(String data)//0,1-1|2-2|3-3|4-4|5-5//Chay sai neu thieu RAM vi ham spli ko hoat dong duoc
//{
//  uint8_t firstOpenBracket = data.indexOf(',');
//  String indexStr = data.substring(0, firstOpenBracket);
//  String listStr = data.substring(firstOpenBracket + 1);
//
//  if (indexStr.length() && listStr.length())
//  {
//    uint8_t index_set = indexStr.toInt();
//    uint8_t expr_size = listStr.length() + 1;//=1 do add them 1 o nho de null
//    char *expr;//Khai bao
//    expr = (char*)malloc(sizeof(char) * expr_size); //Cap phat o nho va tro den vi tri dau
//    listStr.toCharArray(expr, expr_size);
//    size_t len = 0;
//    char** fields = NULL;
//    fields = split_str(expr, '|', &len);
//    if ((len < 1) || (len > EEP_LIST_SONG_SIZE))//NEu ==0 la chi co 1 bai
//    {
//#ifdef UART_SHOW_DEBUG
//      Serial.print(F("excuteSaveList: Parse data error-len"));
//#endif
//      goto return_res;
//    }
//    song_play song_list[EEP_LIST_SONG_SIZE];
//    memset(song_list, 255, EEP_LIST_SONG_SIZE * sizeof(song_play));//Set 255 to all
//    //Tach thanh cac bai hat
//    for (int i = 0; i < len; i++)
//    {
//      uint8_t res[2];
//      if (split_range(fields[i], res))
//      {
//        song_play sp = make_song_play(res[0], res[1]);
//        song_list[i] = sp;
//      }
//      else
//        goto return_res;
//    }
//    save_eep_job_song(index_set, song_list);
//    goto return_res;
//
//return_res:
//
//    free_splitted(fields, len);
//    free(expr);
//  }
//}

void excuteResetList(String data)//0
{
  uint8_t index = data.toInt();
  reset_eep_job_song( index);
  Serial.println(F(OK_RESPONSE));
}

void excuteResetAllJob()
{
  int job;
  for (job = 0; job < EEP_MAX_JOB; job++)
  {
    reset_eep_job_song(job);
    save_eep_job_expr(job, "x x x x x x");

  }
  Serial.println(F(OK_RESPONSE));

}

void excuteSaveSong(String data)//0,1:1-1////Thieu Ram khi chay
{
  uint8_t song_index;
  uint8_t index_set;
  uint8_t temp_length;
  uint8_t firstOpenBracket = data.indexOf(',');
  uint8_t secondOpenBracket = data.indexOf(':');
  String tempStr = data.substring(0, firstOpenBracket);
  index_set = tempStr.toInt();
  tempStr = data.substring(firstOpenBracket + 1, secondOpenBracket);
  song_index = tempStr.toInt();
  tempStr = data.substring(secondOpenBracket + 1);

  tempStr.trim();
  //
  temp_length = tempStr.length();
  if (temp_length > 7)
  {
#ifdef UART_SHOW_DEBUG
    Serial.print(F("excuteSaveList:Error-fields length >7"));
#endif
    temp_length = 7;
  }
  if (temp_length) {
    uint8_t bracket = tempStr.indexOf('-');
    String s_Str = tempStr.substring(0, bracket);
    uint8_t s_index = s_Str.toInt();
    s_Str = tempStr.substring( bracket + 1);
    uint8_t s_volume = s_Str.toInt();
#ifdef UART_SHOW_DEBUG
    Serial.print(F("index_set"));  Serial.println(index_set);
    Serial.print(F("song_index"));  Serial.println(song_index);
    Serial.print(F("s_index"));  Serial.println(s_index);
    Serial.print(F("s_volume"));  Serial.println(s_volume);
#endif
    save_eep_job_song_index(index_set, song_index, &s_index, &s_volume);
  }//
  tempStr = "";

  Serial.println(F(OK_RESPONSE));

}

void excuteGetJobPlayList(String data)//0
{

  uint8_t firstOpenBracket = data.indexOf(',');
  String indexStr = data.substring(0, firstOpenBracket);
  if (indexStr.length())
  {

    uint8_t index_set = indexStr.toInt();
    song_play song_list_rev[EEP_LIST_SONG_SIZE];
    get_eep_job_song(index_set, song_list_rev);
    int i;
    Serial.println(F(START_RESPONSE));
    delay(10);
    //    Serial.print(F("\""));
    for (i = 0; i < EEP_LIST_SONG_SIZE; i++)
    {

      song_play song_rev = song_list_rev[i];
      if (song_rev.song_index != 255)//Neu list ko duoc ghi se hien 255
      {
        if (i > 0)Serial.print(F("|"));
        Serial.print(song_rev.song_index);
        Serial.print(F("-"));
        Serial.print(song_rev.song_volume);

      }
      else break;
    }
    //    Serial.print(F("\""));
    Serial.println();
    delay(10);
    Serial.println(F(STOP_RESPONSE));
    delay(10);
  }
}

void excutePlaySongIndexC(String data)//index,volume
{
  char *cmd_data;//Khai bao
  uint8_t data_size = data.length() + 1;
  cmd_data = (char*)malloc(sizeof(char) * data_size);
  data.toCharArray(cmd_data, 40);
  size_t len = 0;
  char** fields = NULL;
  fields = split_str(cmd_data, ',', &len);

  if (len != 1) {
#ifdef UART_SHOW_DEBUG
    Serial.print(F("excutePlaySongIndex: Parse data error"));
#endif
    goto return_res;
  }

  goto return_res;
return_res:
  free_splitted(fields, len);
  free(cmd_data);
}

void actionExcute(int action, String data)
{
  switch (action) {
    case ACTION_STOP_PLAY_SONG:
      excuteStopSong();
      break;
    case ACTION_PLAY_SONG_INDEX:
      excutePlaySongIndex(data);
      break;
    case ACTION_SET_VOLUME:
      excuteSetVolume(data);
      break;
    case ACTION_SET_JOB_TIME:
      excuteSaveTime(data);
      break;
    case ACTION_RESET_JOB_LIST:
      excuteResetList(data);
      break;
    case ACTION_SET_JOB_SONG:
      excuteSaveSong(data);
      break;
    case ACTION_SET_TIME_STAMP:
      excuteSetTimeStamp(data);
      break;
    case ACTION_GET_JOB_TIME:
      excuteGetTime(data);
      break;
    case ACTION_GET_JOB_PLAY_LIST:
      excuteGetJobPlayList(data);
      break;
    case ACTION_GET_PLAY_LIST:
      excuteGetPlayList();
      break;
//    case ACTION_RESET_ALL_JOB://Thieu Ram
//      excuteResetAllJob();
//      break;
    default:
      // code to be executed if n doesn't match any constant
      break;
  }
}
void checkCommand(String input )
{
  uint8_t firstOpenBracket = input.indexOf('<');
  uint8_t firstCloseBracket = input.indexOf('>');
  uint8_t secondOpenBracket = input.indexOf('<', firstOpenBracket + 1);
  uint8_t secondCloseBracket = input.indexOf('>', firstCloseBracket + 1);
  String cmd = input.substring(firstOpenBracket + 1, firstCloseBracket);
  String cmdData = input.substring(secondOpenBracket + 1, secondCloseBracket);

  if (parseCommand(cmd, CMD_STOP_PLAY_SONG))
  {
    actionExcute(ACTION_STOP_PLAY_SONG, cmdData);
  }
  else if (parseCommand(cmd, CMD_PLAY_SONG_INDEX))
  {
    actionExcute(ACTION_PLAY_SONG_INDEX, cmdData);
  }
  else if (parseCommand(cmd, CMD_SET_VOLUME))
  {
    actionExcute(ACTION_SET_VOLUME, cmdData);
  }
  else if (parseCommand(cmd, CMD_SET_JOB_TIME))
  {
    actionExcute(ACTION_SET_JOB_TIME, cmdData);
  }
  else if (parseCommand(cmd, CMD_RESET_JOB_LIST))
  {
    actionExcute(ACTION_RESET_JOB_LIST, cmdData);
  }
  else if (parseCommand(cmd, CMD_SET_JOB_SONG))
  {
    actionExcute(ACTION_SET_JOB_SONG, cmdData);
  }
  else if (parseCommand(cmd, CMD_SET_TIME_STAMP))
  {
    actionExcute(ACTION_SET_TIME_STAMP, cmdData);
  }
  else if (parseCommand(cmd, CMD_GET_JOB_TIME))
  {
    actionExcute(ACTION_GET_JOB_TIME, cmdData);
  }
  else if (parseCommand(cmd, CMD_GET_JOB_PLAY_LIST))
  {
    actionExcute(ACTION_GET_JOB_PLAY_LIST, cmdData);
  }
  else if (parseCommand(cmd, CMD_GET_PLAY_LIST))
  {
    actionExcute(ACTION_GET_PLAY_LIST, cmdData);
  }
//  else if (parseCommand(cmd, CMD_RESET_ALL_JOB))//Thieu Ram
//  {
//    actionExcute(ACTION_RESET_ALL_JOB, cmdData);
//  }

    //
     Serial.print(F("MemFree="));
     Serial.println(freeMemory());
    //
}
