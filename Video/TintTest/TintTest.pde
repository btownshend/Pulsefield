PImage img;

void setup() {
  size(640, 480);
  img = loadImage("texture.png");
}

void draw() {
  background(255);  
  noStroke(); 

  // Bright red
  int sz=200;
  int alpha=127;
  int sat=255;
  int bri=255;
  colorMode(HSB, 255);


  tint(0, sat, bri, alpha);
  image(img, 200, 200, sz, sz);
  tint(80, sat, bri, alpha);

  image(img, 250, 300, sz, sz);

  tint(160, sat, bri, alpha);

  image(img, 150, 300, sz, sz);
}

