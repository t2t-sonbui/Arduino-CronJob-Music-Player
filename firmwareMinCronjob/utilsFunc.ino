boolean split_range(const char*input, char*res)
{
  int err = 0;
  char** parts = NULL;
  size_t len = 0;
  parts = split_str(input, '-', &len);
  if (2 != len) {
    #ifdef UART_SHOW_DEBUG
    Serial.print(F("split_range: Parse data error !=2"));
    #endif
    err = 1;
    goto return_error;
  }
  res[0] = parse_uint(parts[0], &err);
  if (err) {
    #ifdef UART_SHOW_DEBUG
    Serial.print(F("split_range: Parse data error parse_uint res[0]"));
    #endif
    goto return_error;
  }
  res[1] = parse_uint(parts[1], &err);
  if (err) {
    #ifdef UART_SHOW_DEBUG
    Serial.print(F("split_range: Parse data error parse_uint res[1]"));
    #endif
    goto return_error;
  }
  goto return_error;
return_error:
  free_splitted(parts, len);
  if (err) return false;
  else return true;

}
