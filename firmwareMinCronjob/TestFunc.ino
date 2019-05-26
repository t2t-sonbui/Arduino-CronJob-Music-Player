#include <EEPROM.h>


void test_cron_match()
{

  const char* err = NULL;
  memset(&expr, 0, sizeof(expr));
  cron_parse_expr("* * * * * *", &expr, &err);
  // cron_parse_expr("*/10 * * * 1-12 0-6", &expr, &err);//10 giay 1 lan,Thang 1-12, chu nhat den thu 7
  if (err)  {
    Serial.print(F(" -> Error:"));
    Serial.println(err);
  }

  boolean test_match;
  Serial.println(F("0/10 * 17 * * ?"));
  Serial.println(F("--------------------"));
  test_match = check_cron_match(&expr, 0, 30, 17, 1, 1, 1);//True
  Serial.println(test_match);
  test_match = check_cron_match(&expr, 1, 30, 17, 1, 1, 1);//False
  Serial.println(test_match);
  test_match = check_cron_match(&expr, 10, 15, 17, 1, 1, 1);//True
  Serial.println(test_match);
  test_match = check_cron_match(&expr, 5, 15, 17, 1, 1, 2);//Fakse
  Serial.println(test_match);
  Serial.println(F("--------------------"));
  test_match = check_cron_match(&expr, 0, 30, 8, 1, 1, 1);//False
  Serial.println(test_match);
  test_match = check_cron_match(&expr, 0, 30, 9, 1, 1, 1);//False
  Serial.println(test_match);
  test_match = check_cron_match(&expr, 0, 30, 10, 1, 1, 1);//False
  Serial.println(test_match);
  test_match = check_cron_match(&expr, 0, 30, 11, 1, 1, 1);//False
  Serial.println(test_match);
  Serial.println(F("--------------------"));

}



void reset_eeprom()
{
  int i;
  for (i = 0; i < 1024; i++)
  {
    EEPROM.write(i, 1);
  }
}
void test_eeprom()
{
  int i;
  for (i = 0; i < 1024; i++)
  {
    Serial.print(F("-"));
    Serial.print( EEPROM.read(i));
  }
}

void test_get_song(int job)
{
  song_play song_list_rev[EEP_LIST_SONG_SIZE];
  get_eep_job_song(job, song_list_rev);
  Serial.println(F("-------------------"));
  int i;
  for (i = 0; i < EEP_LIST_SONG_SIZE; i++)
  {
    song_play song_rev = song_list_rev[i];
    Serial.print(F("-{"));
    Serial.print(song_rev.song_index);
    Serial.print(F("-"));
    Serial.print(song_rev.song_volume);
    Serial.print(F("}-"));
  }
  Serial.println();
  Serial.println(F("--------------------"));
}
void test_get_expr(int job)
{
  char job_rev[EEP_JOB_EXPR_SIZE + 1];
  get_job_expr_expr(job, job_rev );
  Serial.println(F("-------------------"));
  Serial.print(F("\""));
  Serial.print(job_rev);
  Serial.print(F("\""));
  Serial.println();
  Serial.println(F("--------------------"));
}

void test_save_job(int job)
{

  song_play s1 = make_song_play(1 + job, 1 + job);
  song_play s2 = make_song_play(2 + job, 2 + job);
  song_play s3 = make_song_play(3 + job, 3 + job);
  song_play s4 = make_song_play(4 + job, 4 + job);
  song_play s5 = make_song_play(5 + job, 5 + job);
  song_play song_list[EEP_LIST_SONG_SIZE] = {s1, s2, s3, s4, s5};
  save_eep_job_song(job, song_list);
  char expr[EEP_JOB_EXPR_SIZE + 1];
  int i;
  for (i = 0; i < EEP_JOB_EXPR_SIZE; i++)
  {
    expr[i] = job + 0x30;
  }
  expr[i] = '\0';
  //    save_eep_job_expr(0, "0,3 0/30 8-10 * 1-12 *");//
  save_eep_job_expr(job, expr);
}

void test_get_job(int job)
{
  song_play song_list_rev[EEP_LIST_SONG_SIZE];
  char job_rev[EEP_JOB_EXPR_SIZE + 1];
  get_job_expr_expr(job, job_rev );
  get_eep_job_song(job, song_list_rev);
  Serial.println(F("-------------------"));
  Serial.print(F("\""));
  Serial.print(job_rev);
  Serial.print(F("\""));
  int i;
  for (i = 0; i < EEP_LIST_SONG_SIZE; i++)
  {
    song_play song_rev = song_list_rev[i];
    Serial.print(F("-{"));
    Serial.print(song_rev.song_index);
    Serial.print(F("-"));
    Serial.print(song_rev.song_volume);
    Serial.print(F("}-"));
  }
  Serial.println();
  Serial.println(F("--------------------"));
}

void test_job_save()
{
  int job;
  for (job = 0; job < EEP_MAX_JOB; job++)
  {
    test_save_job(job);
  }

  for ( job = 0; job < EEP_MAX_JOB; job++)
  {
    test_get_job(job);
  }
}
