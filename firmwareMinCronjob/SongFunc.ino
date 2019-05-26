
#define SERIAL_PRINT(msg) Serial.print(msg)
#define SERIAL_PRINTLN(msg) Serial.println(msg)
song_play make_song_play(uint8_t index, uint8_t volume)
{
  song_play song;
  song.song_index = index;
  song.song_volume = volume;
  return song;

}
void playSongVolume(uint8_t volume)//Max la 254,min la 1
{
  #ifdef UART_SHOW_DEBUG
    Serial.print(F("Play Song Volume:"));
    Serial.println(volume);
  #endif
  if (volume == 0)  musicPlayer.setVolume(255, 255);
  else
  {
    uint8_t volume_set = (100 - volume);//Giao dong tu (155-254)
    //Volum,e set chay ti 1-100
    musicPlayer.setVolume(volume_set, volume_set);// Set volume for left, right channels. lower numbers == louder volume!
  }
}

void playSongIndex(int index)
{
  if (!musicPlayer.stopped())musicPlayer.stopPlaying();
  #ifdef UART_SHOW_DEBUG
    Serial.print(F("Play index song:"));
    Serial.println(index);
  #endif
  if (!musicPlayer.stopped())musicPlayer.stopPlaying();
  char *name_song_play = findSongName("/", index);
  if (name_song_play && !name_song_play[0]) {
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("Not found song"));
    #endif
  }
  else
  {
    #ifdef UART_SHOW_DEBUG
    Serial.println("Found song:" + String(name_song_play));
    #endif
    if (! musicPlayer.startPlayingFile(name_song_play)) {
      #ifdef UART_SHOW_DEBUG
      Serial.println("Could not open file:" + String(name_song_play));
      #endif
      while (1);
    }
    #ifdef UART_SHOW_DEBUG
    Serial.println(F("Started playing"));
    #endif
  }
}

void playSongName(const char *name_song_play)
{
  if (!musicPlayer.stopped())musicPlayer.stopPlaying();
  #ifdef UART_SHOW_DEBUG
  Serial.println("Found song:" + String(name_song_play));
  #endif
  if (! musicPlayer.startPlayingFile(name_song_play)) {
    #ifdef UART_SHOW_DEBUG
    Serial.println("Could not open file:" + String(name_song_play));
    #endif
    while (1);
  }
  #ifdef UART_SHOW_DEBUG
  Serial.println(F("Started playing"));
  #endif
}

void stopSong()
{
  if (!musicPlayer.stopped())musicPlayer.stopPlaying();
 // musicPlayer.softReset();
}


int endsWith(const char *str, const char *suffix)
{
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix >  lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

int endsWithMusic(const char *str) {
  return (endsWith(str, ".MP3") || endsWith(str, ".mp3") || endsWith(str, ".WAV") || endsWith(str, ".wav"));
}


char *findSongName(const char *list_path, uint8_t songIndex)
{
  if (!musicPlayer.stopped())musicPlayer.stopPlaying();
  uint8_t start_index = 0;
  File root = SD.open(list_path, 0);
  root.rewindDirectory();
  char *fileName;
  while (true) {
    File entry =  root.openNextFile();
    if (! entry) {
      root.rewindDirectory();
      break;
    }
    fileName = entry.name();
    if (endsWithMusic(fileName)) {
      if (start_index == songIndex) {
        entry.close();//free xay ra trong day
        break;
      }
      start_index++;
    }
    entry.close();
  }
  root.close();
  return fileName;
}


void getSongList(const char *list_path)
{
  if (!musicPlayer.stopped())musicPlayer.stopPlaying();
  uint8_t song_index = 0;
  File pathGet = SD.open(list_path, 0);
  pathGet.rewindDirectory();// keo file tro lai ban dau thu muc
  while (true) {
    File entry =  pathGet.openNextFile();
    if (! entry) {
      break;
    }
    if (!entry.isDirectory()) {
    if (endsWithMusic(entry.name())) {
      SERIAL_PRINTLN(String(song_index) + ":"  + String(entry.name()));
      song_index++;
    }
    }
    entry.close();
//    delay(100);
  }
  pathGet.rewindDirectory();// keo file tro lai ban dau
  pathGet.close();
}
