// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_GAMEMAP_MANAGER_H_
#define VIGILANTE_GAMEMAP_MANAGER_H_

#include <set>
#include <string>
#include <memory>
#include <functional>

#include <cocos2d.h>
#include <Box2D/Box2D.h>
#include "GameMap.h"
#include "WorldContactListener.h"
#include "Controllable.h"
#include "character/Character.h"
#include "item/Item.h"

namespace vigilante {

// Forward Declaration
class Player;

class GameMapManager {
 public:
  static GameMapManager* getInstance();
  virtual ~GameMapManager() = default;

  void update(float delta);

  // Safely loads the specified GameMap using a worker thread
  // which executes independently in background.
  //
  // IMPORTANT: When the function returns, the new GameMap will *NOT*
  //            have been loaded.
  //
  //            Q1. "Can't we just block the main thread until the worker thread
  //                 finishes loading the new GameMap?", you may ask.
  //
  //            A1. "No. The main thread have to work on Shade's FadeIn effect,
  //                 so we have to keep the main thread running."
  //            -----------------------------------------------------------------
  //            Q2. "So how do I know when the new GameMap has finished loading?"
  //
  //            A2. "If you need to do anything after the new GameMap has been loaded,
  //                 then pass a callable object (as the 2nd parameter).
  //                 It is guaranteed to be called after the new GameMap is loaded."
  //
  // Before loading the new GameMap, we need to ensure that
  // there are no pending callbacks anymore. This is important because
  // suppose that we have a pending callback which manipulates a sprite,
  // but this sprite has already been removed from the scene, now
  // when this callback is invoked -- we will get a segfault on linux
  // (or EXC_BAD_ACCESS on macOS).
  //
  // @param tmxMapFileName: the target .tmx file to load
  // @param afterLoadingGameMap: guaranteed to be called after the GameMap
  //                             has been loaded (optional).
  void loadGameMap(const std::string& tmxMapFileName,
                   const std::function<void ()>& afterLoadingGameMap=[]() {});

  cocos2d::Layer* getLayer() const;
  b2World* getWorld() const;
  GameMap* getGameMap() const;
  Player* getPlayer() const;

 private:
  explicit GameMapManager(const b2Vec2& gravity);

  // Internal function - NOT safe if used with CallbackManager!
  // Used by GameMap::loadGameMap().
  GameMap* doLoadGameMap(const std::string& tmxMapFileName);

  cocos2d::Layer* _layer;
  std::unique_ptr<WorldContactListener> _worldContactListener;
  std::unique_ptr<b2World> _world;
  std::unique_ptr<GameMap> _gameMap;
  std::unique_ptr<Player> _player;
};

}  // namespace vigilante

#endif  // VIGILANTE_GAMEMAP_MANAGER_H_
