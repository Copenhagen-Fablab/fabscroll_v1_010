// helper functions for the timers that are restarted manually

void start_remote_fetch_timer() {
  // Start the main timer
  remote_fetch_timer_state = TIMER_START;  // Reset main timer state
  remote_fetch_ticker.once_ms(remote_fetch_interval, []() {
    remote_fetch_timer_state = TIMER_ENDED;
  });
}

void start_restore_default_text_timer() {
  // Start the default text timer
  default_text_timer_state = TIMER_START;  // Reset default text timer state
  restore_default_text_ticker.once_ms(restore_default_text_interval, []() {
    default_text_timer_state = TIMER_ENDED;
  });
}
