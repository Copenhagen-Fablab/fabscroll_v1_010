void display_refresh() {  // refresh display; use with display_refresh_timer every few ms
  display.display();
}

void update_leds_loop() {
  // Check the current mode and call the appropriate drawing function
  switch (current_scroll_mode) {
    case SINGLE_LINE_SCROLL:
      single_line_scroll_draw();
      break;
    case DOUBLE_LINE_SCROLL:
      double_line_scroll_draw();
      break;
    case OTHER_MODE:
      // Handle other modes
      break;
    default:
      // Default case
      break;
  }
}

void single_line_scroll_draw() {
  // scroll one line, temp text, scrolling left

  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(2);      // Set text size to 2
  display.clearDisplay();      // Clear the display buffer

  // Calculate the horizontal position of the text
  static int16_t xPos = display.width();
  xPos--;  // Scroll to the left

  // Set the text color
  display.setTextColor(0xFF);

  // Calculate text bounds
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(textBuffer, xPos, 1, &x1, &y1, &w, &h);

  // If the text has scrolled completely off the screen
  if (xPos <= -w) {
    // Reset xPos to start the scrolling again
    xPos = display.width();
  }

  // Check if new text has been inserted
  if (newTextInserted) {
    // Reset xPos to start the scrolling again
    xPos = display.width();
    // Reset the flag
    newTextInserted = false;
  }

  // Set the cursor position
  display.setCursor(xPos, 0);  //

  // Print the textBuffer content
  display.println(textBuffer);

  // Show the updated frame on the display
  display.showBuffer();
}

void double_line_scroll_draw() {
  // scroll two lines, one above the other
  // top line temp text, scrolling left
  // bottom line default text, scrolling right

  // Static X positions for each line
  static int16_t xPosLine1 = display.width();
  static int16_t xPosLine2 = -display.width();  // todo --  this starting position value is not correct

  display.setTextWrap(false);  // Disable text wrapping
  display.setTextSize(1);      // Set text size
  display.clearDisplay();      // Clear the display buffer

  // Calculate text bounds for line 1
  int16_t x1Line1, y1Line1;
  uint16_t wLine1, hLine1;
  display.getTextBounds(textBuffer, 0, 0, &x1Line1, &y1Line1, &wLine1,
                        &hLine1);

  // Calculate text bounds for line 2
  int16_t x1Line2, y1Line2;
  uint16_t wLine2, hLine2;
  display.getTextBounds(defaultTextBuffer, 0, 0, &x1Line2, &y1Line2, &wLine2,
                        &hLine2);

  // Update X position for line 1 (scrolling to the left)
  xPosLine1--;
  if (xPosLine1 < -wLine1) {
    xPosLine1 = display.width();
  }

  // Update X position for line 2 (scrolling to the right)
  xPosLine2++;
  if (xPosLine2 >= display.width()) {
    xPosLine2 = -wLine2;
  }

  // Check if new text has been inserted
  if (newTextInserted) {
    // Reset xPos to start the scrolling again
    xPosLine1 = display.width();
    xPosLine2 = -display.width();
    // Reset the flag
    newTextInserted = false;
  }

  // Set text color
  display.setTextColor(0xFF);

  // Print line 1 at its current position
  display.setCursor(xPosLine1, 0);
  display.println(textBuffer);

  // Print line 2 at its current position
  display.setCursor(xPosLine2, 8);  // Adjust Y position for the second line
  display.println(defaultTextBuffer);

  // Show the updated frame on the display
  display.showBuffer();
}

void switch_scroll_mode() {
  // Change the mode
  switch (current_scroll_mode) {
    case SINGLE_LINE_SCROLL:
      current_scroll_mode = DOUBLE_LINE_SCROLL;
      newTextInserted = true;  //signal to display text from beginning
      break;
    case DOUBLE_LINE_SCROLL:
      current_scroll_mode = SINGLE_LINE_SCROLL;
      newTextInserted = true;  //signal to display text from beginning
      break;
    case OTHER_MODE:
      // placeholder to add new modes/animations
      break;
    default:
      // Default case
      current_scroll_mode = SINGLE_LINE_SCROLL;
      newTextInserted = true;  //signal to display text from beginning
      break;
  }
}