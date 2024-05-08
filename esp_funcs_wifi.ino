// wifi and text functions

void connectWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  // Wait for Wi-Fi connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
    attempts++;

    // If connection attempts exceed a certain threshold, break out of the loop
    if (attempts > MAX_CONNECTION_ATTEMPTS) {
      Serial.println(
        "Failed to connect to Wi-Fi. Going to WAIT state...");
      start_remote_fetch_timer();  // Set timer to switch state to TIMER_ENDED after interval
      current_state = WAIT;        // Transition to WAIT state - skip fetch and disconnect
      return;                      // Exit the function
    }
  }

  Serial.println("\nWi-Fi connected!");
}

void fetchFile(const char *url_serv, uint16_t port_number, const char *url_loc,
               char *destinationBuffer, char *previousBuffer) {
  Serial.println("Fetching file...");
  client.setInsecure();  // Skip ssl verification
  if (!client.connect(url_serv, port_number)) {
    Serial.println("Error: Connection to server failed.");
    return;
  }

  // request the file
  Serial.println("Connected to server");
  client.print("GET ");
  client.print(url_loc);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(url_serv);
  client.println("Connection: close");
  client.println();

  // ignore response headers
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  if (!client.available()) {
    Serial.println("Error: No data received from server.");
    client.stop();
    return;
  }

  // receive the actual data
  char tempBuffer[maxChars];
  int bytesRead = client.readBytes(tempBuffer, sizeof(tempBuffer));
  tempBuffer[bytesRead] = '\0';  // Null-terminate the string

  // Check for UTF-8 BOM and skip it
  if (bytesRead >= 3 && memcmp(tempBuffer, "\xEF\xBB\xBF", 3) == 0) {
    bytesRead -= 3;
    memmove(tempBuffer, tempBuffer + 3, bytesRead);
    tempBuffer[bytesRead] = '\0';  // Null-terminate the string
  }

  //Check for non-ASCII characters
  if (containsNonASCII(tempBuffer)) {
    Serial.println("Error: File contains non-ASCII characters.");
    client.stop();
    return;
  }

  replaceNonPrintableChars(tempBuffer);  //change invalid chars to spaces, like newline etc

  parseEscapeSequences(tempBuffer);  //experimental

  // Check if the number of characters fetched meets the minimum requirement
  if (bytesRead < minCharsToFetch) {
    Serial.println("Error: Insufficient data received from server.");
    client.stop();
    return;
  }

  // Check if the fetched file is different from the previous one
  if (strcmp(tempBuffer, previousBuffer) != 0) {
    // Update previousBuffer with the contents of tempBuffer
    strcpy(previousBuffer, tempBuffer);
    // Update destinationBuffer as well
    strncpy(destinationBuffer, tempBuffer, sizeof(tempBuffer) - 1);
    destinationBuffer[sizeof(tempBuffer) - 1] = '\0';
    Serial.println("File fetched and inserted successfully.");
    newTextInserted = true;              // Signal to display text from beginning
    start_restore_default_text_timer();  // Restart the default text timer
  } else {
    Serial.println("File already up to date.");
  }

  client.stop();
}

//experimental, eg \x92 will become 92 hex = "Æ"
void parseEscapeSequences(char *str) {
  char *src = str;
  char *dest = str;

  while (*src) {
    if (*src == '\\' && *(src + 1) == 'x') {
      // Found an escape sequence "\x"
      // Parse the hexadecimal digits
      char hex[3] = { 0 };
      hex[0] = *(src + 2);
      hex[1] = *(src + 3);

      // Convert the hexadecimal string to an integer
      char *endptr;
      unsigned int value = strtoul(hex, &endptr, 16);
      if (value != 0) {
        // Replace the escape sequence with the corresponding hex value
        *dest++ = value;
        // Move the source pointer past the escape sequence
        src += 4;
      }
    } else {
      // Copy the character if not part of an escape sequence
      *dest++ = *src++;
    }
  }

  // Null-terminate the modified string
  *dest = '\0';
}

// Function to replace non-printable characters with spaces in the provided string
void replaceNonPrintableChars(char *str) {
  for (size_t i = 0; str[i] != '\0'; i++) {
    // Check if the character is outside the printable ASCII range (32 to 126)
    if (static_cast<unsigned char>(str[i]) < 32
        || static_cast<unsigned char>(str[i]) > 126) {
      str[i] = ' ';  // Replace non-printable character with space
    }
  }
}

// Function to check if the provided string contains ASCII characters
bool containsNonASCII(const char *str) {
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (static_cast<unsigned char>(str[i]) > 127) {
      return true;  // Non-ASCII character found
    }
  }
  return false;  // No non-ASCII characters found
}

void disconnectWiFi() {
  WiFi.disconnect();
  Serial.println("Disconnected from WiFi.");
}
