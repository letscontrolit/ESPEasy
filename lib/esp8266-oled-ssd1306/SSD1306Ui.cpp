#include "SSD1306Ui.h"


SSD1306Ui::SSD1306Ui(SSD1306 *display) {
  this->display = display;
}

void SSD1306Ui::init() {
  this->display->init();
}

void SSD1306Ui::setTargetFPS(byte fps){
  int oldInterval = this->updateInterval;
  this->updateInterval = ((float) 1.0 / (float) fps) * 1000;

  // Calculate new ticksPerFrame
  float changeRatio = oldInterval / this->updateInterval;
  this->ticksPerFrame *= changeRatio;
  this->ticksPerTransition *= changeRatio;
}

// -/------ Automatic controll ------\-

void SSD1306Ui::enableAutoTransition(){
  this->autoTransition = true;
}
void SSD1306Ui::disableAutoTransition(){
  this->autoTransition = false;
}
void SSD1306Ui::setAutoTransitionForwards(){
  this->frameTransitionDirection = 1;
}
void SSD1306Ui::setAutoTransitionBackwards(){
  this->frameTransitionDirection = 1;
}
void SSD1306Ui::setTimePerFrame(int time){
  this->ticksPerFrame = (int) ( (float) time / (float) updateInterval);
}
void SSD1306Ui::setTimePerTransition(int time){
  this->ticksPerTransition = (int) ( (float) time / (float) updateInterval);
}


// -/------ Customize indicator position and style -------\-
void SSD1306Ui::setIndicatorPosition(IndicatorPosition pos) {
  this->indicatorPosition = pos;
  this->dirty = true;
}
void SSD1306Ui::setIndicatorDirection(IndicatorDirection dir) {
  this->indicatorDirection = dir;
}
void SSD1306Ui::setActiveSymbole(const char* symbole) {
  this->activeSymbole = symbole;
  this->dirty = true;
}
void SSD1306Ui::setInactiveSymbole(const char* symbole) {
  this->inactiveSymbole = symbole;
  this->dirty = true;
}


// -/----- Frame settings -----\-
void SSD1306Ui::setFrameAnimation(AnimationDirection dir) {
  this->frameAnimationDirection = dir;
}
void SSD1306Ui::setFrames(FrameCallback* frameFunctions, int frameCount) {
  this->frameCount     = frameCount;
  this->frameFunctions = frameFunctions;
}

// -/----- Overlays ------\-
void SSD1306Ui::setOverlays(OverlayCallback* overlayFunctions, int overlayCount){
  this->overlayCount     = overlayCount;
  this->overlayFunctions = overlayFunctions;
}


// -/----- Manuel control -----\-
void SSD1306Ui::nextFrame() {
  this->state.frameState = IN_TRANSITION;
  this->state.ticksSinceLastStateSwitch = 0;
  this->frameTransitionDirection = 1;
}
void SSD1306Ui::previousFrame() {
  this->state.frameState = IN_TRANSITION;
  this->state.ticksSinceLastStateSwitch = 0;
  this->frameTransitionDirection = -1;
}


// -/----- State information -----\-
SSD1306UiState SSD1306Ui::getUiState(){
  return this->state;
}


int SSD1306Ui::update(){
  int timeBudget = this->updateInterval - (millis() - this->state.lastUpdate);
  if ( timeBudget <= 0) {
    // Implement frame skipping to ensure time budget is keept
    if (this->autoTransition && this->state.lastUpdate != 0) this->state.ticksSinceLastStateSwitch += ceil(-timeBudget / this->updateInterval);

    this->state.lastUpdate = millis();
    this->tick();
  }
  return timeBudget;
}


void SSD1306Ui::tick() {
  this->state.ticksSinceLastStateSwitch++;

  switch (this->state.frameState) {
    case IN_TRANSITION:
        this->dirty = true;
        if (this->state.ticksSinceLastStateSwitch >= this->ticksPerTransition){
          this->state.frameState = FIXED;
          this->state.currentFrame = getNextFrameNumber();
          this->state.ticksSinceLastStateSwitch = 0;
        }
      break;
    case FIXED:
      if (this->state.ticksSinceLastStateSwitch >= this->ticksPerFrame){
          if (this->autoTransition){
            this->state.frameState = IN_TRANSITION;
            this->dirty = true;
          }
          this->state.ticksSinceLastStateSwitch = 0;
      }
      break;
  }

  if (this->dirty) {
    this->dirty = false;
    this->display->clear();
    this->drawIndicator();
    this->drawFrame();
    this->drawOverlays();
    this->display->display();
  }
}

void SSD1306Ui::drawFrame(){
  switch (this->state.frameState){
     case IN_TRANSITION: {
       float progress = (float) this->state.ticksSinceLastStateSwitch / (float) this->ticksPerTransition;
       int x, y, x1, y1;
       switch(this->frameAnimationDirection){
        case SLIDE_LEFT:
          x = -128 * progress;
          y = 0;
          x1 = x + 128;
          y1 = 0;
          break;
        case SLIDE_RIGHT:
          x = 128 * progress;
          y = 0;
          x1 = x - 128;
          y1 = 0;
          break;
        case SLIDE_UP:
          x = 0;
          y = -64 * progress;
          x1 = 0;
          y1 = y + 64;
          break;
        case SLIDE_DOWN:
          x = 0;
          y = 64 * progress;
          x1 = 0;
          y1 = y - 64;
          break;
       }

       // Invert animation if direction is reversed.
       int dir = frameTransitionDirection >= 0 ? 1 : -1;
       x *= dir; y *= dir; x1 *= dir; y1 *= dir;

       this->dirty |= (this->frameFunctions[this->state.currentFrame])(this->display, &this->state, x, y);
       this->dirty |= (this->frameFunctions[this->getNextFrameNumber()])(this->display, &this->state, x1, y1);
       break;
     }
     case FIXED:
      this->dirty |= (this->frameFunctions[this->state.currentFrame])(this->display, &this->state, 0, 0);
      break;
  }
}

void SSD1306Ui::drawIndicator() {
    byte posOfCurrentFrame;

    switch (this->indicatorDirection){
      case LEFT_RIGHT:
        posOfCurrentFrame = this->state.currentFrame;
        break;
      case RIGHT_LEFT:
        posOfCurrentFrame = (this->frameCount - 1) - this->state.currentFrame;
        break;
    }

    for (byte i = 0; i < this->frameCount; i++) {

      const char *image;

      if (posOfCurrentFrame == i) {
         image = this->activeSymbole;
      } else {
         image = this->inactiveSymbole;
      }

      int x,y;
      switch (this->indicatorPosition){
        case TOP:
          y = 0;
          x = 64 - (12 * frameCount / 2) + 12 * i;
          break;
        case BOTTOM:
          y = 56;
          x = 64 - (12 * frameCount / 2) + 12 * i;
          break;
        case RIGHT:
          x = 120;
          y = 32 - (12 * frameCount / 2) + 12 * i;
          break;
        case LEFT:
          x = 0;
          y = 32 - (12 * frameCount / 2) + 12 * i;
          break;
      }

      this->display->drawXbm(x, y, 8, 8, image);
    }
}

void SSD1306Ui::drawOverlays() {
 for (int i=0;i<this->overlayCount;i++){
    this->dirty |= (this->overlayFunctions[i])(this->display, &this->state);
 }
}

int SSD1306Ui::getNextFrameNumber(){
  int nextFrame = (this->state.currentFrame + this->frameTransitionDirection) % this->frameCount;
  if (nextFrame < 0){
    nextFrame = this->frameCount + nextFrame;
  }
  return nextFrame;
}
