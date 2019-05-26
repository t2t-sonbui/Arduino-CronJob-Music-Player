#include <EEPROM.h>

//void save_eep_job(uint8_t index, const cron_expr* expr,  song_play *songs)//Chuong trinh nay khong phu hop de luu vi khi luu se thanh 1 chiu rat dai khi khoi phuc lai
//{
//  uint8_t i = 0;
//  uint16_t start_index_save = index * EEP_JOB_SIZE;
//  for (i = 0; i < EEP_MIN_SIZE; i ++) { //Save minutes
//    EEPROM.write(start_index_save + i, expr->minutes[i]);
//  }
//  for (i = 0; i < EEP_HOURS_SIZE; i ++) { //Save hours
//    EEPROM.write(start_index_save + i + EEP_HOURS_SIZE, expr->hours[i]);
//  }
//  for (i = 0; i < EEP_WDAY_SIZE; i ++) { //Save days_of_week
//    EEPROM.write(start_index_save + i + EEP_HOURS_SIZE + EEP_WDAY_SIZE, expr->days_of_week[i]);
//  }
//  for (i = 0; i < EEP_DAY_SIZE; i ++) { //Save days_of_month
//    EEPROM.write(start_index_save + i + EEP_HOURS_SIZE + EEP_WDAY_SIZE + EEP_DAY_SIZE, expr->days_of_month[i]);
//  }
//  for (i = 0; i < EEP_MONTH_SIZE; i ++) { //Save months
//    EEPROM.write(start_index_save + i + EEP_HOURS_SIZE + EEP_WDAY_SIZE + EEP_DAY_SIZE + EEP_MONTH_SIZE, expr->months[i]);
//  }
//  //save list song
//  uint16_t start_song_address = EEP_LIST_SONG_SIZE + EEP_HOURS_SIZE + EEP_WDAY_SIZE + EEP_DAY_SIZE + EEP_MONTH_SIZE + start_index_save;
//  for (i = 0; i < EEP_LIST_SONG_SIZE; i ++) { //Save list song
//    EEPROM.write(start_song_address + i * 2, songs[i].song_index);
//    EEPROM.write(start_song_address + i * 2 + 1, songs[i].song_volume);
//  }
//}
//void make_song_play_list(uint8_t index, const cron_expr* expr, song_play *songs)
//{
//  uint8_t i = 0;
//  uint16_t start_index_save = index * EEP_JOB_SIZE;
//  uint16_t start_song_address = EEP_LIST_SONG_SIZE + EEP_HOURS_SIZE + EEP_WDAY_SIZE + EEP_DAY_SIZE + EEP_MONTH_SIZE + start_index_save;
//  for (i = 0; i < EEP_LIST_SONG_SIZE; i ++) { //Get list song
//    songs[i].song_index = EEPROM.read(start_song_address + i * 2);
//    songs[i].song_volume = EEPROM.read(start_song_address + i * 2 + 1 );
//  }
//}

void save_eep_job_expr(uint8_t index, const char* expr)
{
  uint8_t i = 0;
  uint16_t start_index_save = index * (EEP_JOB_EXPR_SIZE + 1 + EEP_JOB_SONG_SIZE); //+1 la vi co them null

  while (* (expr + i) != '\0')
  {
    if (i > EEP_JOB_EXPR_SIZE)
    {
      EEPROM.write(start_index_save + EEP_JOB_EXPR_SIZE, '\0');
      break;
    }
    EEPROM.write(start_index_save + i, *(expr + i));
    i++;
  }
  EEPROM.write(start_index_save + i, '\0');//ghi ky tu cuoi cung -EEPROM.write(start_index_save + i, *(expr + i))
}
void get_job_expr_expr(uint8_t index, char* expr)
{
  uint8_t i = 0;
  uint16_t start_index_save = index * (EEP_JOB_EXPR_SIZE + 1 + EEP_JOB_SONG_SIZE); //+1 la vi co them null
  while (i < EEP_JOB_EXPR_SIZE)
  {
    * (expr + i) = EEPROM.read(start_index_save + i ); //
    if ( (*(expr + i)) == '\0') break;
    i++;
  }
  * (expr + EEP_JOB_EXPR_SIZE) = '\0';//Reset if max char
}

void reset_eep_job_song(uint8_t index)
{
  uint8_t i = 0;
  uint16_t start_index_save = index * (EEP_JOB_EXPR_SIZE + 1 + EEP_JOB_SONG_SIZE);
  uint16_t start_song_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 ;
  uint16_t start_volume_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 + EEP_LIST_SONG_SIZE ;

  for (i = 0; i < EEP_LIST_SONG_SIZE; i ++) { //Save list song
    EEPROM.write(start_song_address + i, 255);
    EEPROM.write(start_volume_address + i, 255);
  }
}
void save_eep_job_song_index(uint8_t index, uint8_t song_index,  uint8_t *s_index, uint8_t *s_volume)
{

  uint16_t start_index_save = index * (EEP_JOB_EXPR_SIZE + 1 + EEP_JOB_SONG_SIZE);
  uint16_t start_song_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 ;
  uint16_t start_volume_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 + EEP_LIST_SONG_SIZE ;
  EEPROM.write(start_song_address + song_index, *s_index);
  EEPROM.write(start_volume_address + song_index, *s_volume);
}

void save_eep_job_song(uint8_t index,  const song_play *songs)
{
  uint8_t i = 0;
  uint16_t start_index_save = index * (EEP_JOB_EXPR_SIZE + 1 + EEP_JOB_SONG_SIZE);
  uint16_t start_song_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 ;
  uint16_t start_volume_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 + EEP_LIST_SONG_SIZE ;

  for (i = 0; i < EEP_LIST_SONG_SIZE; i ++) { //Save list song
    EEPROM.write(start_song_address + i, songs[i].song_index);
    EEPROM.write(start_volume_address + i, songs[i].song_volume);
  }

}

void get_eep_job_song(uint8_t index, song_play *songs)
{
  uint8_t i = 0;
  uint16_t start_index_save = index * (EEP_JOB_EXPR_SIZE + 1 + EEP_JOB_SONG_SIZE);
  uint16_t start_song_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 ;
  uint16_t start_volume_address = start_index_save + EEP_JOB_EXPR_SIZE + 1 + EEP_LIST_SONG_SIZE ;

  for (i = 0; i < EEP_LIST_SONG_SIZE; i ++) { //Save list song
    songs[i].song_index = EEPROM.read(start_song_address + i);
    songs[i].song_volume = EEPROM.read(start_volume_address + i );
  }

}
