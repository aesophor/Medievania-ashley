// Copyright (c) 2018-2020 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_TRADE_WINDOW_H_
#define VIGILANTE_TRADE_WINDOW_H_

#include <memory>

#include <cocos2d.h>
#include <ui/UIImageView.h>
#include <ui/UILayout.h>
#include "ui/Window.h"
#include "character/Character.h"
#include "ui/TabView.h"
#include "ui/trade/TradeListView.h"

namespace vigilante {

class TradeWindow : public Window {
 public:
  TradeWindow(Character* buyer, Character* seller);
  virtual ~TradeWindow() = default;

  virtual void update() override;  // Window
  virtual void handleInput() override;  // Window

  Character* getBuyer() const;
  Character* getSeller() const;

 private:
  cocos2d::ui::ImageView* _contentBackground;
  std::unique_ptr<TabView> _tabView;
  std::unique_ptr<TradeListView> _tradeListView;

  Character* _buyer;
  Character* _seller;
};

}  // namespace vigilante

#endif  // VIGILANTE_TRADE_WINDOW_H_