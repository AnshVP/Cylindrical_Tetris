#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>

#ifndef PSTR
#define PSTR  // Make Arduino Due happy
#endif
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

const char* ssid = "Hotspot";
const char* password = "tutututututara";

WiFiServer server(80);

#define GRID_H 15
#define GRID_W 30
#define STRAND_LENGTH GRID_W* GRID_H
#define LED_DATA_PIN 4
// max size of each tetris piece
#define PIECE_W 3
#define PIECE_H 3
// how many kinds of pieces
#define NUM_PIECE_TYPES 7

const int pieces[][4][3][3] = {
  { { { 1, 1, 0 },
      { 1, 1, 0 },
      { 0, 0, 0 } },

    { { 1, 1, 0 },
      { 1, 1, 0 },
      { 0, 0, 0 } },

    { { 1, 1, 0 },
      { 1, 1, 0 },
      { 0, 0, 0 } },

    { { 1, 1, 0 },
      { 1, 1, 0 },
      { 0, 0, 0 } } },
  { { { 1, 1, 0 },
      { 0, 1, 1 },
      { 0, 0, 0 } },

    { { 0, 0, 1 },
      { 0, 1, 1 },
      { 0, 1, 0 } },

    { { 0, 0, 0 },
      { 1, 1, 0 },
      { 0, 1, 1 } },

    { { 0, 1, 0 },
      { 1, 1, 0 },
      { 1, 0, 0 } } },
  { { { 0, 1, 1 },
      { 1, 1, 0 },
      { 0, 0, 0 } },

    { { 0, 1, 0 },
      { 0, 1, 1 },
      { 0, 0, 1 } },

    { { 0, 0, 0 },
      { 0, 1, 1 },
      { 1, 1, 0 } },

    { { 1, 0, 0 },
      { 1, 1, 0 },
      { 0, 1, 0 } } },
  { { { 0, 1, 0 },
      { 1, 1, 1 },
      { 0, 0, 0 } },

    { { 0, 1, 0 },
      { 0, 1, 1 },
      { 0, 1, 0 } },

    { { 0, 0, 0 },
      { 1, 1, 1 },
      { 0, 1, 0 } },

    { { 0, 1, 0 },
      { 1, 1, 0 },
      { 0, 1, 0 } } },
  { { { 1, 0, 0 },
      { 1, 0, 0 },
      { 1, 1, 0 } },

    { { 1, 1, 1 },
      { 1, 0, 0 },
      { 0, 0, 0 } },

    { { 0, 1, 1 },
      { 0, 0, 1 },
      { 0, 0, 1 } },

    { { 0, 0, 0 },
      { 0, 0, 1 },
      { 1, 1, 1 } } },
  { { { 1, 0, 0 },
      { 1, 1, 1 },
      { 0, 0, 0 } },

    { { 1, 1, 0 },
      { 1, 0, 0 },
      { 1, 0, 0 } },

    { { 1, 1, 1 },
      { 0, 0, 1 },
      { 0, 0, 0 } },

    { { 0, 0, 1 },
      { 0, 0, 1 },
      { 0, 1, 1 } } },
  { { { 0, 0, 0 },
      { 1, 1, 1 },
      { 0, 0, 0 } },

    { { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 } },

    { { 0, 0, 0 },
      { 1, 1, 1 },
      { 0, 0, 0 } },

    { { 0, 1, 0 },
      { 0, 1, 0 },
      { 0, 1, 0 } } }
};

int piece_no = random(0, NUM_PIECE_TYPES);
int rotate = 0;
int current_y = 0;
int current_x = random(0, GRID_W);
int last = 0;
int score = 0;
bool pause = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRAND_LENGTH, LED_DATA_PIN, NEO_RGB + NEO_KHZ800);
int grid[GRID_H][GRID_W];
int fixed_grid[GRID_H][GRID_W];
int piece_x;
int piece_y;
int piece_id;
int piece_rotation;
int timer_game_over;
bool game_over = false;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(GRID_W, GRID_H, LED_DATA_PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

const uint16_t colors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

bool isGameOver() {
  int i = 0;
  for (i = 0; i < GRID_W; i++) {
    if (fixed_grid[0][i] != -1) {
      timer_game_over = millis();
      game_over = true;

      for (int l = 0; l < GRID_H; l++) {
        for (int m = 0; m < GRID_W; m++) {
          grid[l][m] = 0;
          fixed_grid[l][m] = -1;
        }
      }
      draw_grid();
      return true;
    }
  }
  return false;
}

int x = matrix.width();
int pass = 0;
String name = "";

void gameOver() {

  matrix.fillScreen(0);
  matrix.setCursor(x, 0);
  name = "GAME OVER";
  int len = name.length();
  matrix.print((name));
  if (--x < -(len * 6)) {
    x = matrix.width();
    if (++pass >= 3) pass = 0;
    matrix.setTextColor(colors[pass]);
  }
  matrix.show();
  delay(100);
  if ((millis() - timer_game_over) >= 17000) {
    game_over = false;
  }
}

void p(int x, int y, long color) {
  int a = (GRID_H - 1 - y) * GRID_W;
  a += x;
  a %= STRAND_LENGTH;
  strip.setPixelColor(a, color);
}

int piece_color[][3] = { { 145, 207, 2 }, { 0, 255, 0 }, { 209, 21, 134 }, { 39, 207, 184 }, { 196, 179, 24 }, { 115, 9, 104 }, { 255, 0, 0 } };

void draw_grid() {
  int x, y;
  for (y = 0; y < GRID_H; ++y) {
    for (x = 0; x < GRID_W; ++x) {
      if (grid[y][x] == 1) {
        p(x, y, strip.Color(piece_color[piece_no][0], piece_color[piece_no][1], piece_color[piece_no][2]));
      } else if (fixed_grid[y][x] != -1) {
        p(x, y, strip.Color(piece_color[fixed_grid[y][x]][0], piece_color[fixed_grid[y][x]][1], piece_color[fixed_grid[y][x]][2]));
      } else {
        p(x, y, 0);
      }
    }
  }
  strip.show();
  delay(100);
}

bool check_left() {
  int temp;
  for (int i = current_x; i < current_x + 3; i++) {
    for (int j = current_y; j < current_y + 3; j++) {
      if (i == GRID_W) {
        temp = 0;
      } else if (i == (GRID_W + 1)) {
        temp = 1;
      } else {
        temp = i;
      }
      if ((grid[j][temp] == 1) && ((temp != 0 && fixed_grid[j][temp - 1] != -1) || (temp == 0 && fixed_grid[j][GRID_W - 1] != -1))) {
        return false;
      }
    }
  }
  return true;
}

bool check_right() {
  int temp;
  for (int i = current_x; i < current_x + 3; i++) {
    for (int j = current_y; j < current_y + 3; j++) {
      if (i == GRID_W) {
        temp = 0;
      } else if (i == (GRID_W + 1)) {
        temp = 1;
      } else {
        temp = i;
      }
      if ((grid[j][temp] == 1) && ((temp != (GRID_W - 1) && fixed_grid[j][temp + 1] != -1) || (temp == (GRID_W - 1) && fixed_grid[j][0] != -1))) {
        return false;
      }
    }
  }
  return true;
}

bool check_down() {
  int temp;
  for (int i = current_y; i < current_y + 3; i++) {
    for (int j = current_x; j < current_x + 3; j++) {
      if (j == GRID_W) {
        temp = 0;
      } else if (j == (GRID_W + 1)) {
        temp = 1;
      } else {
        temp = j;
      }
      if ((grid[i][temp] == 1 && i == (GRID_W - 1)) || ((grid[i][temp] == 1) && fixed_grid[i + 1][temp] != -1)) {
        return false;
      }
    }
  }
  return true;
}

void add_piece(int grid_matrix[GRID_H][GRID_W]) {
  int temp_x;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if ((current_x + j) == GRID_W) {
        temp_x = 0;
      } else if ((current_x + j) == GRID_W + 1) {
        temp_x = 1;

      } else {
        temp_x = current_x + j;
      }

      grid_matrix[current_y + i][temp_x] = pieces[piece_no][rotate][i][j];
    }
  }
}

void rotate_piece(int prev_rotate) {
  int temp_grid[GRID_H][GRID_W];

  for (int i = 0; i < GRID_H; i++) {
    for (int j = 0; j < GRID_W; j++) {
      temp_grid[i][j] = 0;
    }
  }

  add_piece(temp_grid);

  int flag = 0;
  for (int i = 0; i < GRID_H; i++) {
    if (flag == 1) {
      break;
    }
    for (int j = 0; j < GRID_W; j++) {
      if (temp_grid[i][j] == 1 && fixed_grid[i][j] != -1) {
        flag = 1;
        break;
      }
    }
  }
  if (flag == 0) {
    add_piece(grid);
  } else {
    rotate = prev_rotate;
  }
  delay(100);
}

void move_down() {
  if ((millis() - last) >= 1000 && !pause) {
    if (!check_down()) {
      for (int i = 0; i < GRID_H; i++) {
        for (int j = 0; j < GRID_W; j++) {
          if (grid[i][j] == 1) {
            fixed_grid[i][j] = piece_no;
          }
        }
      }
      current_y = 0;
    } else {
      current_y += 1;
      int temp[GRID_W];
      for (int i = GRID_H - 1; i >= 0; i--) {
        for (int j = 0; j < GRID_W; j++) {
          if (i == GRID_H - 1) {
            temp[j] = grid[i][j];
            grid[i][j] = grid[i - 1][j];
          } else if (i == 0) {
            grid[i][j] = temp[j];
          } else {
            grid[i][j] = grid[i - 1][j];
          }
        }
      }
      last = millis();
    }
  }
}

void new_piece() {
  int temp;
  current_x = random(0, GRID_W);
  rotate = 0;
  piece_no = random(0, NUM_PIECE_TYPES);
  for (int i = 0; i < GRID_H; i++) {
    for (int j = 0; j < GRID_W; j++) {
      grid[i][j] = 0;
    }
  }
  add_piece(grid);
}

void delete_row(int row) {
  for (int i = 0; i < GRID_W; i++) {
    fixed_grid[row][i] = -1;
    score = score + 10;
  }
  for (int i = row; i > 0; i--) {
    for (int j = 0; j < GRID_W; j++) {
      fixed_grid[i][j] = fixed_grid[i - 1][j];
    }
  }
}

void check_row() {
  for (int i = GRID_H - 1; i >= 0; i--) {
    for (int j = 0; j < GRID_W; j++) {
      if (fixed_grid[i][j] == -1) {
        break;
      }
      if (j == GRID_W - 1 && fixed_grid[i][j] != -1) {
        delete_row(i);
      }
    }
  }
}

void new_game() {
  current_y = 0;
  pause = false;
  for (int i = 0; i < GRID_H; i++) {
    for (int j = 0; j < GRID_W; j++) {
      fixed_grid[i][j] = -1;
      grid[i][j] = 0;
    }
  }
  delay(100);
}

void setup() {
  Serial.begin(115200);
  matrix.begin();
  strip.begin();
  strip.setBrightness(5);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(colors[0]);

  Serial.println("");
  Serial.println("NodeMCU is connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  new_game();
  delay(3000);
}

void loop() {
  if (!isGameOver() && !game_over) {
    if (current_y == 0 && !pause) {
      new_piece();
    }

    WiFiClient client;
    client = server.available();

    if (client == 1) {
      String request = client.readStringUntil('\n');
      client.flush();
      Serial.println(request);
      if (request.indexOf("left") != -1 && check_left() && !pause) {
        if (current_x == 0) {
          current_x = GRID_W - 1;
        } else {
          current_x -= 1;
        }

        int temp;
        for (int i = 0; i < GRID_H; i++) {
          for (int j = 0; j < GRID_W; j++) {
            if (j == 0) {
              temp = grid[i][j];
              grid[i][j] = grid[i][j + 1];
            } else if (j == GRID_W - 1) {
              grid[i][j] = temp;
            } else {
              grid[i][j] = grid[i][j + 1];
            }
          }
        }
      }

      if (request.indexOf("right") != -1 && check_right() && !pause) {
        if (current_x == GRID_W - 1) {
          current_x = 0;
        } else {
          current_x += 1;
        }
        int temp;
        for (int i = 0; i < GRID_H; i++) {
          for (int j = GRID_W - 1; j >= 0; j--) {
            if (j == GRID_W - 1) {
              temp = grid[i][j];
              grid[i][j] = grid[i][j - 1];
            } else if (j == 0) {
              grid[i][j] = temp;
            } else {
              grid[i][j] = grid[i][j - 1];
            }
          }
        }
      }

      if (request.indexOf("clock") != -1 && !pause && !(current_y > GRID_H - 3)) {
        int prev_rotate = rotate;
        if (rotate == 3) {
          rotate = 0;
        } else {
          rotate += 1;
        }
        rotate_piece(prev_rotate);
      }

      if (request.indexOf("counter") != -1 && !pause && !(current_y > GRID_H - 3)) {
        int prev_rotate = rotate;
        if (rotate == 0) {
          rotate = 3;
        } else {
          rotate -= 1;
        }
        rotate_piece(prev_rotate);
      }

      if (request.indexOf("restart") != -1 && pause == true) {
        new_game();
      }

      if (request.indexOf("pause") != -1 && pause == false) {
        pause = true;
      }

      if (request.indexOf("resume") != -1 && pause == true) {
        pause = false;
      }

      client.println("HTTP/1.1 200 OK");
      client.println("<html>\n<head>\n<title> Tetris </title>\n</head>\n<body>\n\n");
      if (!pause)
        client.println("<div  style=\" margin-top: 1rem; margin-left: auto; margin-right: auto;\">\n  <div style=\"display: flex ;justify-content: space-evenly;\">\n    <a href=\"pause\">  <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer;\"> PAUSE  </button>  </a>\n  </div>\n</div> ");
      else
        client.println("<div  style=\" margin-top: 1rem; margin-left: auto; margin-right: auto;\">\n  <div style=\"display: flex ;justify-content: space-evenly;\">\n    <a href=\"resume\">  <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer;\"> RESUME  </button>  </a>\n  </div>\n</div> ");
      client.println("<div  style=\" margin-top: 1rem; margin-left: auto; margin-right: auto;\">\n"
                     "    <div style=\"display: flex ;justify-content: space-evenly;\">\n"
                     "      <a href=\"left\">  <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer;\"> LEFT  </button>  </a>\n"
                     "      <img  src=\"https://static1.dualshockersimages.com/wordpress/wp-content/uploads/2010/11/tetris-logo.jpg?q=50&fit=contain&w=1140&h=570&dpr=1.5\" style=\"height: 60px; width: 200px;\"/>\n"
                     "      <a href=\"right\"> <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer; \" > RIGHT </button> </a>\n"
                     "    </div>\n"
                     "</div> \n"
                     "<div style=\" margin-top: 1rem; margin-left: auto; margin-right: auto;\">\n"
                     "  <div style=\"display: flex ;justify-content: space-evenly;\">\n"
                     "    <a href=\"clock\">  <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer;\"> CLOCK  </button>  </a>\n"
                     "    <div style=\"width: 12.5rem;\"></div>\n"
                     "    <a href=\"counter\"> <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer;\" > COUNTER-CLOCK </button> </a>\n"
                     "</div>\n"
                     "</div>");
      client.println("<div  style=\" margin-top: 1rem; margin-left: auto; margin-right: auto;\">\n  <div style=\"display: flex ;justify-content: space-evenly;\">\n    <a href=\"restart\">  <button style=\"height:60px; background-color: yellow; width:200px; cursor: pointer;\"> RESTART  </button>  </a>\n  </div>\n</div> \n</body>\n</html>");
      client.println("<div  style=\" margin-top: 1rem; margin-left: auto; margin-right: auto;\">\n  <div style=\"display: flex ;justify-content: space-evenly;\">\n    <h1>Score: " + String(score) + "</h1>\n </div>");
      Serial.println("------------------");
      Serial.println("                  ");
    }
    check_row();
    draw_grid();
    move_down();
  } else {
    gameOver();
  }
  strip.show();
}