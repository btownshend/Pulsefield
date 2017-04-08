NavierStokesSolver fluidSolver;
double visc, diff, limitVelocity;
int oldMouseX = 1, oldMouseY = 1;
private int bordercolor;
private double scale;
PImage buffer;
    PImage iao;
 
int rainbow = 0;
 
void setup() {
    fluidSolver = new NavierStokesSolver();
 
    frameRate(60);
    size(1280, 720);
 
    buffer = new PImage(width, height);
 
    visc = 0.001;
    diff = 3.0;
    scale = 0.3;
 
    limitVelocity = 200;
  //  iao = loadImage("IAO.jpg");
  //  background(iao);
    colorMode(HSB, 255);
    bordercolor = color(0,255,255);
    strokeWeight(7);
}
 
public void draw() {
    handleMouseMotion();
    double dt = 1 / frameRate;
    fluidSolver.tick(dt, visc, diff);
    fluidCanvasStep();
 
    colorMode(HSB, 255);
    bordercolor = color(rainbow, 255,255);
    rainbow++;
    rainbow = (rainbow > 255) ? 0 : rainbow;
     
    drawBorders();
}
 
private void drawBorders() {
    stroke(bordercolor);
    line(0,0,width-1,0);
    line(0,0,0,height-1);
    line(width-1,0,width-1,height-1);
    line(0,height-1,width-1,height-1);
     
    fill(bordercolor);
}
 
private void handleMouseMotion() {
    int n = NavierStokesSolver.N;
    float cellHeight = height / n;
    float cellWidth = width / n;
 
    double mouseDx = mouseX - oldMouseX;
    double mouseDy = mouseY - oldMouseY;
    int cellX = (int) Math.floor(mouseX / cellWidth);
    int cellY = (int) Math.floor(mouseY / cellHeight);
 
    mouseDx = (abs((float) mouseDx) > limitVelocity) ? Math.signum(mouseDx)
            * limitVelocity : mouseDx;
    mouseDy = (abs((float) mouseDy) > limitVelocity) ? Math.signum(mouseDy)
            * limitVelocity : mouseDy;
 
    fluidSolver.applyForce(cellX, cellY, mouseDx*0.2, mouseDy*0.2);
 
    oldMouseX = mouseX;
    oldMouseY = mouseY;
}
 
private void fluidCanvasStep() {
    int n = NavierStokesSolver.N;
    double widthInverse = 1.0 / width;
    double heightInverse = 1.0 / height;
 
    loadPixels();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double u = x * widthInverse;
            double v = y * heightInverse;
 
            double warpedPosition[] = fluidSolver.getInverseWarpPosition(u, v,
                    scale);
 
            double warpX = warpedPosition[0];
            double warpY = warpedPosition[1];
 
            warpX *= width;
            warpY *= height;
 
            int collor = getSubPixel(warpX, warpY);
 
            buffer.set(x, y, collor);
        }
    }
    background(buffer);
}
 
public int getSubPixel(double warpX, double warpY) {
    if (warpX < 0 || warpY < 0 || warpX > width - 1 || warpY > height - 1) {
        return bordercolor;
    }
    int x = (int) Math.floor(warpX);
    int y = (int) Math.floor(warpY);
    double u = warpX - x;
    double v = warpY - y;
 
    y = constrain(y, 0, height - 2);
    x = constrain(x, 0, width - 2);
 
    int indexTopLeft = x + y * width;
    int indexTopRight = x + 1 + y * width;
    int indexBottomLeft = x + (y + 1) * width;
    int indexBottomRight = x + 1 + (y + 1) * width;
 
    try {
        return lerpColor(lerpColor(pixels[indexTopLeft], pixels[indexTopRight],
                (float) u), lerpColor(pixels[indexBottomLeft],
                pixels[indexBottomRight], (float) u), (float) v);
    } catch (Exception e) {
        System.out.println("error caught trying to get color for pixel position "
                + x + ", " + y);
        return bordercolor;
    }
}
 
public int lerpColor(int c1, int c2, float l){
    colorMode(RGB,255);
    float r1 = red(c1)+0.5;
    float g1 = green(c1)+0.5;
    float b1 = blue(c1)+0.5;
 
    float r2 = red(c2)+0.5;
    float g2 = green(c2)+0.5;
    float b2 = blue(c2)+0.5;
     
    return color( lerp(r1, r2, l), lerp(g1, g2, l), lerp(b1, b2, l) );
}

