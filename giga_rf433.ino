#include "Arduino_GigaDisplay_GFX.h"
#include "Arduino_GigaDisplayTouch.h"
#include <string>
#include <algorithm>
#include <cstring>

Arduino_GigaDisplayTouch touchDetector;
GigaDisplay_GFX display;

#define WHITE 0xffff
#define BLACK 0x0000
#define GRAY 0xad75

#define screen_size_x 480
#define screen_size_y 800

bool canPress = true;
uint8_t selectedIndex = 0;
char* currentHierarchy = "MENU";
uint8_t maximumSelectionCt = 3;
int remotecreationCount = 0;
int selectedRemoteIndex = 0;
char* menuItems[5] = {
  "New Remote",
  "View Remotes",
  "Help",
  "About",
  ".ND"
};  // has to be 5, setmenuview func
char* editRemoteMenu[5] = { "Use Remote", "Edit Remote Type", "Edit Functions", "Delete Remote", ".ND" };
char* editRemoteType[2] = { "Infrared [ IR ]", "Radio [ RF ]" };
char* deletionMenu[2] = { "Keep Remote", "Delete Remote" };
char* actualEditRemoteTypes[2] = { "IR", "RF" };

char* remoteNames[5] = {
  "EMPTY ", "EMPTY ", "EMPTY ", "EMPTY ", "EMPTY "
};

char* remoteCMDS[5][5] = {
  { ".ND", ".ND", ".ND", ".ND", ".ND" },
  { ".ND", ".ND", ".ND", ".ND", ".ND" },
  { ".ND", ".ND", ".ND", ".ND", ".ND" },
  { ".ND", ".ND", ".ND", ".ND", ".ND" },
  { ".ND", ".ND", ".ND", ".ND", ".ND" },
};

char* remoteTypes[5] = {
  ".ND", ".ND", ".ND", ".ND", ".ND"
};

char* Keypad() {
  const char* keys[] = {
    "QWERTYUIO",
    "ASDFGHJKL",
    "ZXCVBNMP_"
  };
  static char input[32] = "";
  int inputIndex = 0;
  display.fillScreen(BLACK);
  display.setCursor(20, 20);
  display.setTextSize(10);
  display.print("CREATE");
  display.setCursor(20, 130);
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.print("Enter a name..");

  for (uint8_t row = 0; row < 3; ++row) {
    for (uint8_t col = 0; col < strlen(keys[row]); ++col) {
      uint16_t x = 13 + col * 50;
      uint16_t y = 260 + row * 60;
      display.drawRoundRect(x, y, 48, 48, 5, WHITE);
      display.setCursor(x + 12, y + 12);
      display.print(keys[row][col]);
    }
  }

  display.drawRoundRect(325, 475, 133, 60, 5, WHITE);
  display.setCursor(345, 490);
  display.print("UNDO");

  display.drawRoundRect(325, 550, 133, 60, 5, WHITE);
  display.setCursor(345, 565);
  display.print("NEXT");

  display.drawRoundRect(25, 475, 133, 60, 5, WHITE);
  display.setCursor(45, 490);
  display.print("BACK");

  GDTpoint_t points[5];
  uint8_t contacts;

  while (true) {
    contacts = touchDetector.getTouchPoints(points);
    if (contacts == 1) {
      uint16_t x = points[0].x;
      uint16_t y = points[0].y;

      for (uint8_t row = 0; row < 3; ++row) {
        for (uint8_t col = 0; col < strlen(keys[row]); ++col) {
          uint16_t keyX = 13 + col * 50;
          uint16_t keyY = 260 + row * 60;
          if (x >= keyX && x <= keyX + 48 && y >= keyY && y <= keyY + 48) {
            if (inputIndex < sizeof(input) - 1) {
              input[inputIndex++] = keys[row][col];
              input[inputIndex] = '\0';
              display.fillRect(20, 170, 400, 40, BLACK);
              display.setCursor(20, 190);
              display.print(input);
            }
            while (contacts > 0) {
              delay(50);
              contacts = touchDetector.getTouchPoints(points);
            }
          }
        }
      }
      if (x >= 325 && x <= 458 && y >= 560 && y <= 620) {
        break;
      }
      if (x >= 325 && x <= 458 && y >= 475 && y <= 535) {
        Serial.println("UNDO");
        if (inputIndex > 0) {
          input[--inputIndex] = '\0';
          display.fillRect(20, 170, 400, 50, BLACK);
          display.setCursor(20, 190);
          display.print(input);
        }
        while (contacts > 0) {
          delay(50);
          contacts = touchDetector.getTouchPoints(points);
        }
      }
      if (x >= 25 && x <= 158 && y >= 475 && y <= 535) {
        Serial.println("BACK");
        return "0";
      }
    }
  }
  return input;
}


void drawButtons() {
  display.drawRoundRect(100, 675, 75, 75, 5, WHITE);
  display.drawRoundRect(200, 675, 75, 75, 5, WHITE);
  display.drawRoundRect(300, 675, 75, 75, 5, WHITE);
  display.fillTriangle(115, 726, 136, 694, 160, 726, WHITE);
  display.fillTriangle(215, 698, 236, 730, 260, 698, WHITE);
  display.fillCircle(337, 712, 18, WHITE);
}
void invertRow(size_t i) {
  for (int16_t x = 0; x < 480; x++) {
    for (int16_t y = ((130 + i * 50) - 5); y < ((130 + i * 50) + 35); y++) {  // 40 is the height
      uint16_t pixel = display.getPixel(x, y);
      display.drawPixel(x, y, ~pixel);
    }
  }
}
void drawMenu(char* items[], size_t size, uint8_t selectedIndex, const char* name, bool doReturn) {
  if (name == "TYPECONFIG") { name = "CONFIG"; };

  display.fillScreen(BLACK);
  display.setCursor(20, 20);
  display.setTextSize(12);
  display.print(name);
  display.setTextSize(4);
  display.setTextColor(WHITE);

  size_t totalItems = size + (doReturn ? 1 : 0);  // how many rows to draw

  for (size_t i = 0; i < totalItems; ++i) {
    display.setCursor(20, 130 + i * 50);
    if (i == 0 && doReturn) {
      if (selectedIndex == 0) {
        display.print("<< Back");
        invertRow(i);
      } else {
        display.print("< Back");
      }
    } else {
      size_t itemIndex = doReturn ? i - 1 : i;
      if (items[itemIndex] != ".ND") {
        if (i == selectedIndex) {
          String menuItem = "> ";
          if (strcmp(items[itemIndex], "EMPTY ") == 0) {
            menuItem = "- ";
            menuItem += "EMPTY SLOT";
            display.print(menuItem);
          } else {
            menuItem += items[itemIndex];
            display.print(menuItem);
            invertRow(i);
          }
        } else {
          if (strcmp(items[itemIndex], "EMPTY ") == 0) {
            display.setTextColor(GRAY);
            display.setTextSize(3);
          }
          display.print(items[itemIndex]);
          display.setTextSize(4);
          display.setTextColor(WHITE);
        }
      }
    }
  }
  drawButtons();
}

void setMenuView(char* spot, int index) {
  selectedIndex = index;
  size_t itemCount;
  if (spot == "MENU") {
    maximumSelectionCt = 3;
    itemCount = sizeof(menuItems) / sizeof(menuItems[0]);
    drawMenu(menuItems, itemCount, index, spot, false);
  } else if (spot == "VIEW") {
    maximumSelectionCt = 5;  //or remotecreationCount
    itemCount = sizeof(remoteNames) / sizeof(remoteNames[0]);
    drawMenu(remoteNames, itemCount, index, spot, true);
  } else if (spot == "EDIT") {
    maximumSelectionCt = 4;
    itemCount = sizeof(editRemoteMenu) / sizeof(editRemoteMenu[0]);
    drawMenu(editRemoteMenu, itemCount, index, spot, true);
  } else if (spot == "TYPECONFIG") {
    maximumSelectionCt = 2;
    itemCount = sizeof(editRemoteType) / sizeof(editRemoteType[0]);
    drawMenu(editRemoteType, itemCount, index, spot, false);
  } else if (spot == "DELETE") {
    maximumSelectionCt = 2;
    itemCount = sizeof(deletionMenu) / sizeof(deletionMenu[0]);
    drawMenu(deletionMenu, itemCount, index, spot, false);
  }
  currentHierarchy = spot;
}
void popup(const char* text1) {
  delay(100);
  display.fillRect(60, 300, 360, 200, BLACK);
  display.drawRect(60, 300, 360, 200, WHITE);
  display.setTextSize(3);

  int lineHeight = 26;
  int cursorY = 310;
  int cursorX = 75;

  const char* ptr = text1;
  while (*ptr) {  // make a new line every time '/' occurs
    if (*ptr == '/') {
      cursorY += lineHeight;
      cursorX = 75;
    } else {
      display.setCursor(cursorX, cursorY);
      display.write(*ptr);
      cursorX += 18;
    }
    ptr++;
  }

  display.setCursor(70, 470);
  display.write("Tap to continue");
  delay(300);

  GDTpoint_t points[5];
  uint8_t contacts = touchDetector.getTouchPoints(points);

  while (contacts == 0) {
    delay(50);
    contacts = touchDetector.getTouchPoints(points);
  }
  contacts = touchDetector.getTouchPoints(points);
  while (contacts < 1) {
    delay(50);
    contacts = touchDetector.getTouchPoints(points);
  }
}

void editRemoteNames(uint8_t index, char* newValue) {
  remoteNames[index] = (char*)malloc(strlen(newValue) + 1);
  if (remoteNames[index] != NULL) {
    strcpy(remoteNames[index], newValue);
    remotecreationCount += 1;
    Serial.println(remotecreationCount);
  } else {
    popup("ERR/mem_alloc fail");
  }
}

void setup() {
  Serial.begin(9600);
  display.begin();
  display.fillScreen(WHITE);
  display.fillScreen(BLACK);
  display.setTextSize(10);
  display.setCursor(10, 10);
  display.print("Booting");
  delay(330);
  display.setTextSize(4);
  display.setCursor(10, 110);
  if (touchDetector.begin()) {
    display.print("RGDS Touch - OK");
  } else {
    display.print("RGDS Touch - X");
    while (1)
      ;
  }
  delay(100);
  display.setCursor(10, 160);
  display.print("Initializing..");
  Serial.println("Initializing...");
  display.setCursor(10, 210);
  Serial.println("Initialization Complete");
  display.print("Initialization - OK");
  delay(80);
  display.fillScreen(BLACK);
  setMenuView("MENU", 0);
  for (int16_t y = 0; y < 480; y++) {
    for (int16_t x = 0; x < 800; x++) {
      uint16_t pixel = display.getPixel(x, y);
      display.drawPixel(x, y, ~pixel);
    }
  }
  delay(50);
  setMenuView("MENU", 0);
}

void loop() {
  uint8_t interactionCase = 0;
  uint8_t contacts;
  GDTpoint_t points[5];
  contacts = touchDetector.getTouchPoints(points);

  if (contacts == 1) {
    uint16_t x = points[0].x;
    uint16_t y = points[0].y;

    if (x >= 100 && x <= 175 && y >= 675 && y <= 750) {  // up
      display.fillRoundRect(100, 675, 75, 75, 5, WHITE);
      display.fillTriangle(115, 726, 136, 694, 160, 726, BLACK);
      interactionCase = 1;
    } else if (x >= 200 && x <= 275 && y >= 675 && y <= 750) {  // down
      display.fillRoundRect(200, 675, 75, 75, 5, WHITE);
      display.fillTriangle(215, 698, 236, 730, 260, 698, BLACK);
      interactionCase = 2;
    } else if (x >= 300 && x <= 375 && y >= 675 && y <= 750) {  // Button 3 (Select)
      display.fillRoundRect(300, 675, 75, 75, 5, WHITE);
      display.fillCircle(337, 712, 18, BLACK);
      interactionCase = 3;
    }

    while (contacts > 0) {
      delay(50);
      contacts = touchDetector.getTouchPoints(points);
    }
    if (interactionCase != 0) {
      display.fillRect(0, 650, 480, 150, BLACK);
      drawButtons();
      if (interactionCase == 1) {  // up
        if (selectedIndex > 0) {
          selectedIndex--;
        }
      } else if (interactionCase == 2) {  // down
        if (selectedIndex < maximumSelectionCt) {
          selectedIndex++;
        }
      }

      setMenuView(currentHierarchy, selectedIndex);

      if (interactionCase == 3) {  // Select
        if (currentHierarchy == "MENU") {
          switch (selectedIndex) {
            case 0:
              {
                currentHierarchy = "KEYPAD";
                char* name = Keypad();

                if (name == "0") {
                  setMenuView("MENU", 0);
                  break;
                }

                for (int i = 0; i < 5; i++) {
                  if (strcmp(remoteNames[i], "EMPTY ") == 0) {
                    editRemoteNames(i, name);
                    break;
                  }
                }
                selectedRemoteIndex = remotecreationCount;
                popup("Choose the method/of communication/used by your remote");
                setMenuView("TYPECONFIG", 0);
                break;
              }
            case 1:
              {
                setMenuView("VIEW", 0);
                break;
              }
            default:
              Serial.println("Help / About");
          }
        } else if (currentHierarchy == "VIEW") {
          if (selectedIndex == 0) {
            setMenuView("MENU", 1);
          } else {
            if (strcmp(remoteNames[selectedIndex - 1],"EMPTY ") != 0) {
              Serial.print("remote selected: ");
              Serial.println(remoteNames[selectedIndex - 1]);
              selectedRemoteIndex = selectedIndex;
              setMenuView("EDIT", 0);      
            }
          }
        } else if (currentHierarchy == "EDIT") {
          switch (selectedIndex) {
            case 0:  // edit back
              {
                setMenuView("VIEW", selectedRemoteIndex);
                break;
              }
            case 2:  // edit type
              {
                setMenuView("TYPECONFIG", 0);
                break;
              }
            case 4:  // delete
              {
                setMenuView("DELETE", 0);
                break;
              }
            default:  // other
              {
                //setMenuView("VIEW", selectedRemoteIndex);
                for (uint16_t i = 0; i <= 800; i += 50) { // Start at 20px, grow by 10px each iteration
                  display.fillRect(0, 401-(i/2), 480, 25, BLACK);
                  display.drawRect(0, 400-(i/2), 480, i, WHITE);
                  display.fillRect(0, 374+(i/2), 480, 25, BLACK);
                  delay(3); // Optional delay for animation effect
                }
                delay(100);
                drawButtons();
                break;
              }
          }
        } else if (currentHierarchy == "TYPECONFIG") {  // choose transmission type
          remoteTypes[selectedRemoteIndex - 1] = actualEditRemoteTypes[selectedIndex];
          setMenuView("EDIT", 0);
        } else if (currentHierarchy == "DELETE") {
          switch (selectedIndex) {
            case 0:  //delete decline
              {
                setMenuView("EDIT", 4);
                break;
              }
            default:  //delete confirm
              {
                editRemoteNames(selectedRemoteIndex - 1, "EMPTY ");
                remotecreationCount -= 2;
                for (uint8_t i; i < 5; i++) {
                  remoteCMDS[selectedRemoteIndex - 1][i] = ".ND";
                }
                setMenuView("VIEW", 0);
              }
          }
        }
      }
    }
  }
}