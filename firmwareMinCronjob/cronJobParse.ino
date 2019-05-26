
boolean check_cron_match(const cron_expr* target, uint8_t m_sec, uint8_t m_min, uint8_t  m_hour, uint8_t m_day, uint8_t m_month, uint8_t m_wday)
{
  boolean secMatch, minMatch, hourMatch, dayMatch, monMatch, wdayMatch;
  secMatch = cron_get_bit(target->seconds, m_sec);
  minMatch = cron_get_bit(target->minutes, m_min);
  hourMatch = cron_get_bit(target->hours, m_hour);
  dayMatch = cron_get_bit(target->days_of_month, m_day);
  monMatch = cron_get_bit(target->months, m_month);
  wdayMatch = cron_get_bit(target->days_of_week, m_wday);

  if (secMatch && minMatch && hourMatch && dayMatch && monMatch && wdayMatch) {
    return true;
  } else {
    return false;
  }
}
