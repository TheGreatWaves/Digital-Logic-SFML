#pragma once
#ifndef TEXT_BOX
#define TEXT_BOX

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics.hpp>

#include <algorithm>
#include <iostream>

#include "context.hpp"

inline float get_text_max_height(const sf::Text& l_text) 
{
	auto charSize = l_text.getCharacterSize();
	auto font = l_text.getFont();
	auto string = l_text.getString().toAnsiString();
	bool bold = (l_text.getStyle() & sf::Text::Bold);
	float max = 0.f;
	for (size_t i = 0; i < string.length(); ++i) 
	{
		sf::Uint32 character = string[i];
		auto glyph = font->getGlyph(character, charSize, bold);
		auto height = glyph.bounds.height;
		if (height <= max) 
		{ 
			continue; 
		}
		max = height;
	}
	return max;
}

class TextBoxGui : public sf::Drawable
{
public:

	TextBoxGui(const std::string& default_str = "Unnamed", bool can_edit = true)
	: m_default_str(default_str)
	, m_can_edit(can_edit)
	{
		m_font.loadFromFile("resources/HelveticaNeueLTStd-It.otf");
		m_input_text = "";
		m_text = std::make_unique<sf::Text>(default_str, m_font);
		m_text->setCharacterSize(30);
		m_text->setFillColor(sf::Color::White);
		m_edit_mode = false;
	}

	void set_text_color(const sf::Color& color)
	{
		m_text->setFillColor(color);
	}

	void draw(sf::RenderTarget &target, sf::RenderStates states) const override
	{
		target.draw(*m_text, states);
	}

	void set_font_size(unsigned int size)
	{
		m_text->setCharacterSize(size);
	}

	void set_position(const sf::Vector2f pos)
	{
		m_text->setPosition(pos);
	}

	void set_string(const std::string& str)
	{
		m_text->setString(str);
	}

	void handle_events(const sf::Event& event)
	{
		if (!m_can_edit) return;

		if (m_edit_mode)
		{
			Context::instance()->edit_mode = Mode::TEXT;
			m_text->setFillColor(sf::Color(100, 100, 100));
		}
		else
		{
			m_text->setFillColor(sf::Color::White);
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			m_edit_mode = (m_text->getGlobalBounds().contains(sf::Vector2f{static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)}));
		}
		else if (m_edit_mode && event.type == sf::Event::TextEntered)
		{
			if (std::isprint(event.text.unicode))
			{
				m_input_text += event.text.unicode;
				m_edited = true;
			}
		}
		else if (m_edit_mode && event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::BackSpace)
			{
				if (!m_input_text.empty())
				{
					m_input_text.pop_back();
					m_edited = true;
				}
			}
			else if (event.key.code == sf::Keyboard::Return)
			{
				m_edit_mode = false;
				m_updated = true;
				Context::instance()->edit_mode = Mode::IDLE;
			}
		}

		if (m_edited)
		{
			m_text->setString(m_input_text);
			m_edited = false;
		}

		if (m_input_text.empty())
		{
			m_text->setString(m_default_str);
		}
	}

	const std::string& get_string()
	{
		return m_input_text;
	}

	float get_width()
	{
		return m_text->getGlobalBounds().width;
	}

	bool was_edited()
	{
		auto updated = m_updated;
		m_updated = false;
		return updated;
	}
	
	float get_height()
	{
		return get_text_max_height(*m_text);
	}

	void reset()
	{
		m_text->setString(m_default_str);
		m_input_text = "";
	}

private:
	bool                      m_can_edit;
	bool                      m_edit_mode;
	bool                      m_edited;	
	bool                      m_updated = false;
	sf::Font                  m_font;
	std::unique_ptr<sf::Text> m_text;
	std::string               m_input_text;
	std::string               m_default_str;
};


#endif /* TEXT_BOX */
