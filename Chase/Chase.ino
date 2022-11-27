#define A 4 // Top
#define B 5 // Top right
#define C 9 // Bottom right
#define D 8 // Bottom
#define E 7 // Bottom left
#define F 3 // Top left
#define G 2 // Center
#define DP 10  // decimal
#define common_cathode 0
#define common_anode 1
bool segMode = common_anode;           // set this to your segment type
int seg[]{ A, B, C, D, E, F, G, DP };  // segment pins
byte chars = 8;                        // max value in the array "Chars"

byte Chars[8][9]{
  { '0', 0, 0, 0, 0, 0, 1, 1, 0 },  //top left
  { '1', 1, 0, 0, 0, 0, 1, 0, 0 },  //top
  { '2', 1, 1, 0, 0, 0, 0, 0, 0 },  //top right
  { '3', 0, 1, 0, 0, 0, 0, 1, 0 },  //center
  { '4', 0, 0, 0, 0, 1, 0, 1, 0 },  //bottom left
  { '5', 0, 0, 0, 1, 1, 0, 0, 0 },  //bottom
  { '6', 0, 0, 1, 1, 0, 0, 0, 0 },  //bottom right
  { '7', 0, 0, 1, 0, 0, 0, 1, 0 },  //center
};

void setup() {
  // set segment pins as OUTPUT
  for (int i = 0; i <= chars; i++) {
    pinMode(seg[i], OUTPUT);
  }
}

void setState(bool mode)  //sets the hole segment state to "mode"
{
  for (int i = 0; i <= 6; i++) {
    digitalWrite(seg[i], mode);
  }
}

void Print(int num)  // print any number on the segment
{
  setState(segMode);  //turn off the segment

  if (segMode == 0) {  //cathode
    for (int i = 0; i < 8; i++) {
      digitalWrite(seg[i], Chars[num][i + 1]);
    }
  } else { //anode
    for (int i = 0; i < 8; i++) {
      digitalWrite(seg[i], !Chars[num][i + 1]);
    }
  }
}

void loop() {
  for (int i = 0; i < chars; i++)  //print
  {
    Print(i);
    delay(100);
  }
}
