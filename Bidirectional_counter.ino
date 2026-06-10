// Two IR sensors (active-LOW, INPUT_PULLUP)
// Prints only: "Current Count: <n>" and "Total Entered: <m>" to Serial Monitor
const int irPin1 = 2;
const int irPin2 = 3;

int in_count = 0;            // total people entered (cumulative)
int out_count = 0;           // total people exited (used to compute current)
int current_count = 0;       // current people inside

const unsigned long sequenceWindow = 400;
const unsigned long debounceMs = 30;
const unsigned long reportInterval = 500;

bool lastState1 = HIGH, lastState2 = HIGH;
unsigned long lastChange1 = 0, lastChange2 = 0;
unsigned long firstTriggerTime = 0;
int firstTriggered = 0;
unsigned long lastReport = 0;

void setup() {
  Serial.begin(115200);
  pinMode(irPin1, INPUT_PULLUP);
  pinMode(irPin2, INPUT_PULLUP);
  delay(50);
  sendCounts();
}

void loop() {
  unsigned long now = millis();

  bool s1 = digitalRead(irPin1);
  bool s2 = digitalRead(irPin2);

  if (s1 != lastState1 && (now - lastChange1) > debounceMs) {
    lastState1 = s1;
    lastChange1 = now;
    handleSensorChange(1, s1, now);
  }

  if (s2 != lastState2 && (now - lastChange2) > debounceMs) {
    lastState2 = s2;
    lastChange2 = now;
    handleSensorChange(2, s2, now);
  }

  if (firstTriggered != 0 && (now - firstTriggerTime) > sequenceWindow) {
    firstTriggered = 0;
    firstTriggerTime = 0;
  }

  if (now - lastReport >= reportInterval) {
    sendCounts();
    lastReport = now;
  }

  delay(5);
}

void handleSensorChange(int sensorId, bool state, unsigned long now) {
  // Only act on active-LOW triggers
  if (state == LOW) {
    if (firstTriggered == 0) {
      firstTriggered = sensorId;
      firstTriggerTime = now;
    } else if (firstTriggered != sensorId) {
      if ((now - firstTriggerTime) <= sequenceWindow) {
        // Sequence 1 -> 2 means entry
        if (firstTriggered == 1 && sensorId == 2) {
          ++in_count;           // increment total entered
          updateCounts();
        }
        // Sequence 2 -> 1 means exit (only count exit if someone is inside)
        else if (firstTriggered == 2 && sensorId == 1) {
          if (out_count < in_count) {
            ++out_count;
            updateCounts();
          }
        }
      }
      // reset sequence after a completed pair
      firstTriggered = 0;
      firstTriggerTime = 0;
    }
  }
}

void updateCounts() {
  current_count = in_count - out_count;
  if (current_count < 0) current_count = 0; // safety
  sendCounts();
}

void sendCounts() {
  Serial.print("Current Count: ");
  Serial.print(current_count);
  Serial.print("    Total Entered: ");
  Serial.println(in_count);
}
