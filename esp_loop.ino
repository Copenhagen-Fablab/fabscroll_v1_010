// begin main loop

void loop() {
  // Main code loop
  switch (current_state) {
    case CONNECT_WIFI:
      connectWiFi();
      current_state = FETCH_FILE;
      break;
    case FETCH_FILE:
      if (WiFi.status() == WL_CONNECTED) {
        fetchFile(url_serv, port_number, url_24h, textBuffer,
                  previousTextBuffer);  // try to fetch temp/override text file
        fetchFile(url_serv, port_number, url_def, defaultTextBuffer,
                  previousDefaultTextBuffer);  // try to fetch default text file
        //void fetchFile(const char *url_serv, uint16_t port_number, const char *url_loc, char *destinationBuffer, char *previousBuffer)
        current_state = DISCONNECT_WIFI;
      }
      break;
    case DISCONNECT_WIFI:
      disconnectWiFi();
      current_state = WAIT;
      start_remote_fetch_timer();  // Set timer to switch state to TIMER_ENDED after interval
      break;
    case WAIT:
      // Check if the (updated text check) timer has expired
      if (remote_fetch_timer_state == TIMER_ENDED) {
        current_state = CONNECT_WIFI;  // Transition to CONNECT_WIFI after timer expires
        update_leds_timer.detach();    // we don't need to keep running the animation while changing text
      }
      // Check if the default text timer has expired
      if (default_text_timer_state == TIMER_ENDED) {
        update_leds_timer.detach();
        // then restore default text/delete temp text
        strncpy(textBuffer, defaultTextBuffer, sizeof(textBuffer) - 1);
        textBuffer[sizeof(textBuffer) - 1] = '\0';
        start_restore_default_text_timer();  // Restart the default text timer
        newTextInserted = true;              //signal to display text from beginning
      }
      // Main code loop in WAIT state
      // just reenable animation if needed
      if (!update_leds_timer.active()) {
        update_leds_timer.attach_ms(txt_scroll_speed, update_leds_loop);
      }
      break;

    default:
      // oops?? we should never reach this
      current_state = CONNECT_WIFI;
      break;
  }
}

// end main loop
