/**
 * Tool belt for selecting components.
 * The implementation looks horribly wasteful, and it probably is.
 */
#pragma once
#ifndef BELT_GUI
#define BELT_GUI

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Vector2.hpp>
#include "SFML/System.hpp"

#include "../board.hpp"
#include "context.hpp"
#include "text_box.hpp"
#include "component_gui.hpp"

constexpr float OPTIONS_OFFSET = 5.f;

const sf::Color IDLE_COLOR = sf::Color(69, 71, 90);
const sf::Color HOVER_COLOR = sf::Color(92, 94, 113);

class BeltGui {
public:
  BeltGui(const sf::Vector2f size)
  : m_text_box("", false)
  , m_target_size(size)
  {
    m_selection_box.setFillColor(IDLE_COLOR);
  }

  void set_string(const std::string& name, float& x_pos)
  {
    m_text_box.set_string(name);
    m_selection_box.setSize({m_text_box.get_width() + 5.f, 40.f});

    m_selection_box.setPosition(x_pos + 10.f, m_target_size.y - (m_selection_box.getSize().y + 10.f));
    m_text_box.set_position({m_selection_box.getPosition().x + 3.f, m_selection_box.getPosition().y});

    x_pos += m_selection_box.getSize().x + OPTIONS_OFFSET;
  }

  void set_hover(bool condition)
  {
    if (condition)
    {
      m_selection_box.setFillColor(HOVER_COLOR);
    }
    else
    {
      m_selection_box.setFillColor(IDLE_COLOR);
    }
  }

  void draw(sf::RenderTarget &target, sf::RenderStates states) 
  {
    if (m_active_component != nullptr)
    {
      m_active_component->draw(target, states);
    }

    auto names = Board::instance()->get_names();

    float x_pos = 0.f;

    set_string("Save", x_pos);
    set_hover(m_active_index==-2);
    target.draw(m_selection_box, states);
    target.draw(m_text_box, states);

    auto count = 0;
    for (auto name : names)
    {
      set_string(std::string(name), x_pos);
      set_hover(m_active_index==count);
      target.draw(m_selection_box, states);
      target.draw(m_text_box, states);
      count++;
    }
  }

  void update(const sf::Time &dt) 
  {
    auto names = Board::instance()->get_names();
    auto mouse_pos = sf::Mouse::getPosition(*Context::instance()->window);

    if (m_active_component != nullptr)
    {
      m_active_component->set_position(static_cast<sf::Vector2f>(mouse_pos));
    }

    float x_pos = 0.f;

    m_active_index = -1;

    set_string("Save", x_pos);
    if (m_selection_box.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos)))
    {
      m_active_index = -2;
    }

    int count = 0;
    for (auto name : names)
    {
      set_string(std::string(name), x_pos);
      if (m_selection_box.getGlobalBounds().contains(static_cast<sf::Vector2f>(mouse_pos)))
      {
        m_active_index = count;
        break;
      }
      count++;
    }
  }

  void select_belt()
  {
    switch (m_active_index)
    {
      break; case -2:
      {
        const auto& name = Context::instance()->current_component_name;

        if (!name.empty())
        {
          Board::instance()->create_new(name);
        }
      }
      break; case -1: return;
      break; default:
      {
        auto names = Board::instance()->get_names();
        auto chosen = names[m_active_index];
        m_active_component = std::make_unique<ComponentGui>(chosen);
      }
    }
  }

  void place_component();

  void handle_events(const sf::Event &event) 
  {
    if (event.type == event.MouseButtonPressed)
    {
      if (event.mouseButton.button == sf::Mouse::Left)
      {
        if (m_active_component != nullptr)
        {
          place_component();
        }
        else
        {
          select_belt();
        }
      }
    }
  }

private:
  int                           m_active_index{};
  std::unique_ptr<ComponentGui> m_active_component{};
  TextBoxGui                    m_text_box;
  const sf::Vector2f            m_target_size;
  sf::RectangleShape            m_selection_box;
};

#endif /* BELT_GUI */
