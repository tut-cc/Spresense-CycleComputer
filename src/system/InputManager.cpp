#include "InputManager.h"

namespace application {

InputManager::InputManager() : btnA(Config::Pin::BTN_A), btnB(Config::Pin::BTN_B) {}

void InputManager::begin() {
  btnA.begin();
  btnB.begin();
}

InputEvent InputManager::update() {
  bool aPressed = btnA.isPressed();
  bool bPressed = btnB.isPressed();

  if ((aPressed && btnB.isHeld()) || (bPressed && btnA.isHeld())) {
    return InputEvent::BTN_BOTH;
  }

  if (aPressed)
    return InputEvent::BTN_A;
  if (bPressed)
    return InputEvent::BTN_B;

  return InputEvent::NONE;
}

} // namespace application
