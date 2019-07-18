// Copyright (c) 2019 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#include "input/InputManager.h"

#include "map/GameMapManager.h"
#include "ui/pause_menu/PauseMenuDialog.h"

using std::set;
using std::array;
using cocos2d::Scene;
using cocos2d::Event;
using cocos2d::EventKeyboard;
using cocos2d::EventListenerKeyboard;

namespace vigilante {

InputManager* InputManager::_instance = nullptr;

const array<EventKeyboard::KeyCode, InputManager::Hotkey::SIZE> InputManager::_kBindableKeys = {{
  EventKeyboard::KeyCode::KEY_LEFT_SHIFT,
  EventKeyboard::KeyCode::KEY_LEFT_CTRL,
  EventKeyboard::KeyCode::KEY_X,
  EventKeyboard::KeyCode::KEY_C,
  EventKeyboard::KeyCode::KEY_V
}};


InputManager* InputManager::getInstance() {
  if (!_instance) {
    _instance = new InputManager();
  }
  return _instance;
}

InputManager::InputManager()
    : _scene(),
      _keyboardEvLstnr(),
      _pressedKeys(),
      _hotkeys(),
      _isAssigningHotkey(),
      _keybindable(),
      _pauseMenuDialog() {}


void InputManager::activate(Scene* scene) {
  _scene = scene;
  _keyboardEvLstnr = EventListenerKeyboard::create();

  _keyboardEvLstnr->onKeyPressed = [=](EventKeyboard::KeyCode keyCode, Event* event) {
    if (!_isAssigningHotkey) {
      if (_pressedKeys.find(keyCode) == _pressedKeys.end()) {
        _pressedKeys.insert(keyCode);
      }
    } else {
      setHotkeyAction(keyCode, _keybindable);
      _isAssigningHotkey = false;
      _keybindable = nullptr;
      _pauseMenuDialog->setVisible(false);
      _pauseMenuDialog->getPauseMenu()->update();
    }
  };

  _keyboardEvLstnr->onKeyReleased = [=](EventKeyboard::KeyCode keyCode, Event* event) {
    _pressedKeys.erase(keyCode);
  };
  
  _scene->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_keyboardEvLstnr, scene);
}

void InputManager::deactivate() {
  _scene->getEventDispatcher()->removeEventListener(_keyboardEvLstnr);
  _keyboardEvLstnr = nullptr;
  _scene = nullptr;
}


bool InputManager::isKeyPressed(EventKeyboard::KeyCode keyCode) const {
  return _pressedKeys.find(keyCode) != _pressedKeys.end();
}

bool InputManager::isKeyJustPressed(EventKeyboard::KeyCode keyCode) {
  // std::set::erase() returns the number of elements erased.
  return _pressedKeys.erase(keyCode) > 0;
}


Keybindable* InputManager::getHotkeyAction(EventKeyboard::KeyCode keyCode) const {
  for (size_t i = 0; i < _kBindableKeys.size(); i++) {
    if (keyCode == _kBindableKeys[i]) {
      return _hotkeys[i];
    }
  }
  return nullptr;
}

void InputManager::setHotkeyAction(EventKeyboard::KeyCode keyCode, Keybindable* keybindable) {
  for (size_t i = 0; i < _kBindableKeys.size(); i++) {
    if (keyCode == _kBindableKeys[i]) {
      clearHotkeyAction(keybindable->getHotkey());
      if (_hotkeys[i]) {
        clearHotkeyAction(_hotkeys[i]->getHotkey());
      }

      _hotkeys[i] = keybindable;
      keybindable->setHotkey(keyCode);
      return;
    }
  }
}

void InputManager::clearHotkeyAction(EventKeyboard::KeyCode keyCode) {
  // If keyCode == KeyCode::KEY_NONE, return at once.
  if (!static_cast<bool>(keyCode)) {
    return;
  }

  for (size_t i = 0; i < _kBindableKeys.size(); i++) {
    if (keyCode == _kBindableKeys[i]) {
      if (_hotkeys[i]) {
        _hotkeys[i]->setHotkey(EventKeyboard::KeyCode::KEY_NONE);
      }
      _hotkeys[i] = nullptr;
      return;
    }
  }
}

void InputManager::promptHotkey(Keybindable* keybindable, PauseMenuDialog* pauseMenuDialog) {
  _isAssigningHotkey = true;
  _keybindable = keybindable;
  _pauseMenuDialog = pauseMenuDialog;
}

} // namespace vigilante