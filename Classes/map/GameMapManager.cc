#include "GameMapManager.h"

#include "Box2D/Box2D.h"

#include "GameAssetManager.h"
#include "character/Player.h"
#include "character/Enemy.h"
#include "item/Equipment.h"
#include "map/fx/Dust.h"
#include "util/box2d/b2BodyBuilder.h"
#include "util/CategoryBits.h"
#include "util/Constants.h"

using std::set;
using std::string;
using std::unique_ptr;
using cocos2d::Director;
using cocos2d::Layer;
using cocos2d::TMXTiledMap;
using cocos2d::TMXObjectGroup;

namespace vigilante {

GameMapManager* GameMapManager::_instance = nullptr;

GameMapManager* GameMapManager::getInstance() {
  if (!_instance) {
    _instance = new (std::nothrow) GameMapManager;
    if (_instance && _instance->init()) {
      _instance->autorelease();
      return _instance;
    }
    CC_SAFE_DELETE(_instance);
    return nullptr;
  }
  return _instance;
}

bool GameMapManager::init() {
  if (!Layer::init()) {
    return false;
  }
  _world = unique_ptr<b2World>(new b2World({0, kGravity}));
  _worldContactListener = unique_ptr<WorldContactListener>(new WorldContactListener());

  _world->SetAllowSleeping(true);
  _world->SetContinuousPhysics(true);
  _world->SetContactListener(_worldContactListener.get());

  _map = nullptr;
  _player = nullptr;

  return true;
}


void GameMapManager::load(const string& mapFileName) {
  if (_map) {
    removeChild(_map);
    _map = nullptr;
  }

  // Release previous TMXTiledMap object.
  _map = TMXTiledMap::create(mapFileName);
  addChild(_map, 0);
 
  // Create box2d objects from layers
  createPolylines(_world.get(), _map, "Ground", category_bits::kGround, true, 2);
  createPolylines(_world.get(), _map, "Wall", category_bits::kWall, true, 1);
  createRectangles(_world.get(), _map, "Platform", category_bits::kPlatform, true, 2);
  createRectangles(_world.get(), _map, "Portal", category_bits::kPortal, false, 0);
  createPolylines(_world.get(), _map, "CliffMarker", category_bits::kCliffMarker, false, 0);

  // Spawn the player.
  _player = spawnPlayer();
  _characters.insert(unique_ptr<Player>(_player));
  addChild(_player->getBodySpritesheet());

  // Spawn an enemy.
  Enemy* enemy = new Enemy("Castle Guard", 300, 100);
  _characters.insert(unique_ptr<Enemy>(enemy));
  addChild(enemy->getBodySpritesheet());

  // Spawn an item.
  for (int i = 0; i < 10; i++) {
    Item* item = new Equipment(Equipment::Type::WEAPON, "Rusty Axe" + std::to_string(i), "An old rusty axe", asset_manager::kRustyAxeIcon, asset_manager::kRustyAxeSpritesheet + ".png", 200, 80);
    _items.insert(item);
    addChild(item->getSprite(), 32);
  }
}

void GameMapManager::createDustFx(Character* character) {
	auto feetPos = character->getB2Body()->GetPosition();
	float dustX = feetPos.x;// - 32.f / kPpm / 2;
	float dustY = feetPos.y - .1f;// - 32.f / kPpm / .065f;
  Dust(this, dustX, dustY);
}


Player* GameMapManager::spawnPlayer() {
  TMXObjectGroup* objGroup = _map->getObjectGroup("Player");

  auto& playerValMap = objGroup->getObjects()[0].asValueMap();
  float x = playerValMap["x"].asFloat();
  float y = playerValMap["y"].asFloat();
  cocos2d::log("[INFO] Spawning player at: x=%f y=%f", x, y);

  return new Player("Aesophor", x, y);
}


b2World* GameMapManager::getWorld() const {
  return _world.get();
}

TMXTiledMap* GameMapManager::getMap() const {
  return _map;
}

Player* GameMapManager::getPlayer() const {
  return _player;
}


set<unique_ptr<Character>>& GameMapManager::getCharacters() {
  return _characters;
}

set<Item*>& GameMapManager::getItems() {
  return _items;
}


void GameMapManager::createRectangles(b2World* world,
                                      TMXTiledMap* map,
                                      const string& layerName,
                                      short categoryBits,
                                      bool isCollidable,
                                      float friction) {
  TMXObjectGroup* portals = map->getObjectGroup(layerName);
  //log("%s\n", _map->getProperty("backgroundMusic").asString().c_str());
  
  for (auto& obj : portals->getObjects()) {
    auto& valMap = obj.asValueMap();
    float x = valMap["x"].asFloat();
    float y = valMap["y"].asFloat();
    float w = valMap["width"].asFloat();
    float h = valMap["height"].asFloat();

    b2BodyBuilder bodyBuilder(world);

    bodyBuilder.type(b2BodyType::b2_staticBody)
      .position(x + w / 2, y + h / 2, kPpm)
      .buildBody();

    bodyBuilder.newRectangleFixture(w / 2, h / 2, kPpm)
      .categoryBits(categoryBits)
      .setSensor(!isCollidable)
      .friction(friction)
      .buildFixture();
  }
}

void GameMapManager::createPolylines(b2World* world,
                                     TMXTiledMap* map,
                                     const string& layerName,
                                     short categoryBits,
                                     bool isCollidable,
                                     float friction) {
  float scaleFactor = Director::getInstance()->getContentScaleFactor();

  for (auto& obj : map->getObjectGroup(layerName)->getObjects()) {
    auto& valMap = obj.asValueMap();
    float xRef = valMap["x"].asFloat();
    float yRef = valMap["y"].asFloat();

    auto& valVec = valMap["polylinePoints"].asValueVector();
    b2Vec2 vertices[valVec.size()];
    for (size_t i = 0; i < valVec.size(); i++) {
      float x = valVec[i].asValueMap()["x"].asFloat() / scaleFactor;
      float y = valVec[i].asValueMap()["y"].asFloat() / scaleFactor;
      vertices[i] = {xRef + x, yRef - y};
    }

    b2BodyBuilder bodyBuilder(world);

    bodyBuilder.type(b2BodyType::b2_staticBody)
      .position(0, 0, kPpm)
      .buildBody();

    bodyBuilder.newPolylineFixture(vertices, valVec.size(), kPpm)
      .categoryBits(categoryBits)
      .setSensor(!isCollidable)
      .friction(friction)
      .buildFixture();
  }
}

} // namespace vigilante
