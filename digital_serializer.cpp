#include <iostream>
#include <string>
#include <string_view>
#include <sstream>

#include "board.hpp"


void greet()
{
	std::cout << BLOCK << " DIGITAL LOGIC " << BLOCK << '\n';
	info("Simulator starting...");
	info("`h` for help.");
	newline();
}

bool prompt()
{
	std::cout << "> ";
	return true;
}

void create_component(std::string_view name)
{
	auto instance = Board::instance();

	instance->create_new(name);
	instance->set_context(name);

	log("New component created: ", name, ". Context switched.\n");
}

void handle_input(std::string_view str)
{

	std::string command = make_lower(str);

	switch (command.at(0))
	{
		break; case 'H':
					 case 'h':
		{
				desc("R                 ", "Simulate the current component.");
				desc("C <component_name>", "Create a new component.");
				desc("L                 ", "List all components.");
				desc("P                 ", "List all current components.");
				desc("S <component_name>", "Set the current component as current.");
				desc("A <component_name>", "Add the specified component to current configuration.");
				desc("T     <pin_number>", "Toggle the pin specified.");
				desc("I            <+/->", "Add/Delete input pin.");
				desc("O            <+/->", "Add/Delete output pin.");
				desc("D                 ", "Dump current component information.");	
				desc("W     <src> <dest>", "Wire source pin and destination pin.");
				desc("W     <src> <dest>", "Wire source pin and destination pin.");
				desc("E <component_name>", "Serialize the current component.");
		}
		break; case 'e':
		{
			auto board = Board::instance();
			auto current = board->context().second;

			/**
			 * Use the current context.
			 */
			if (current == nullptr)
			{
				log("Current context is empty, please select a configuration\n");
				return;
			}

			current->serialize();

			
		}
		break; case 'w':
		{
			auto board = Board::instance();
			auto current = board->context().second;
			std::stringstream ss {command};

			if (current == nullptr)
			{
				log("Current context is empty, please select a configuration\n");
				return;
			}

			std::string c;
			std::size_t p1, p2;

			ss >> c;
			ss >> p1;
			ss >> p2;

			if (!current->wire_pins(p1, p2))
			{
				log("Failed to wire pin ", p1, " and ", p2, '\n');
			}
			else
			{
				log("Successfully wired", p1, " and ", p2, '\n');
			}
		}
		break; case 'd':
		{
			auto board = Board::instance();
			auto current = board->context().second;
			if (current == nullptr)
			{
				log("Current context is empty, please select a configuration\n");
				return;
			}

			current->info();
		}
		break; case 'R':
					 case 'r':
		{
				auto board = Board::instance();
				auto current = board->context().second;
				if (current == nullptr)
				{
					log("Current context is empty, please select a configuration\n");
					return;
				}
				log("=== Preparing ===\n");

				current->reset();

				newline();
				current->simulate();

				log("=== Output pins ===\n");

				auto count = 0;
				for (const auto& pin : current->output_pins)
				{
					log("pin[", count, "] ", pin.state == PinState::ACTIVE ? 1 : 0, "\n");
					count++;
				}
		}
		break; case 'I':
					 case 'i':
					 case 'o':
					 case 'O':
		{
				auto board = Board::instance();
				auto current = board->context().second;
				if (current == nullptr)
				{
					log("Current context is empty, please select a configuration\n");
					return;
				}

				if (auto id = str.find(" "); id < str.size() - 1)
				{
					auto symbol = std::string(str.substr(id+1));

					if (symbol == "+")
					{

						if (command[0] == 'i')
						{
							info("Input pin added.");
							current->add_input_pin();
						}
						else 
						{
							info("Output pin added.");
							current->add_output_pin();
						}
					}
					else if (symbol == "-")
					{
						// TODO: Implement this later...
					}

				}				
				else
				{
					info("Please specify action symbol.");
				}
		}
		break; case 'T':
					 case 't':
		{
				auto board = Board::instance();
				auto current = board->context().second;
				if (current == nullptr)
				{
					log("Current context is empty, please select a configuration\n");
					return;
				}

			std::string delimiter = " ";

			if (auto id = str.find(delimiter); id < str.size() - 1)
			{
				auto pin_id = static_cast<std::size_t>(std::stoi(std::string(str.substr(id+1))));

				if (current->has_pin(pin_id) && current->toggle_pin(pin_id))
				{
					log("Toggled pin ", pin_id, " to ", (current->get_pin_state(pin_id) == PinState::ACTIVE ? "active" : "inactive"), ".\n");
				}
				else
				{
					info("Pin ID invalid.");
				}
			}
			else
			{
				info("Please specify pin ID.");
			}
		}
		break; case 'P':
					 case 'p':
		{
				auto board = Board::instance();
				auto current = board->context();
				if (current.second == nullptr)
				{
					log("Current context is empty, please select a configuration\n");
					return;
				}

				log(current.first);
				newline();

				for (auto& c : current.second->subgates)
				{
					log(c.second->name);
					newline();
				}

		}
		break; case 'C':
					 case 'c':
		{
			std::string delimiter = " ";

			if (auto id = str.find(delimiter); id < str.size() - 1)
			{
				std::string name = std::string(str.substr(id+1));
				create_component(name);
			}
			else
			{
				info("Please provide a component name.");
			}
		}
		break; case 'A':
					 case 'a':
		{
			auto board = Board::instance();
			auto current = board->context();

			if (current.second == nullptr)
			{
				log("Current context is empty, please select a configuration\n");
				return;
			}

			if (auto id = str.find(" "); id < str.size() - 1)
			{
				std::string name = std::string(str.substr(id+1));

				if (auto component = board->get_component(name); component != nullptr)
				{
					if (component->serialized)
					{
						auto id = current.second->add_subgate(component);
						log("Component successfully added with ID ", id, "\n");
					}
					else
					{
						log("Component ", component->name, " is not yet serialized\n");
					}
				}
				else
				{
					log("Component with given name `", name, "` not found!\n");
				}
			}
			else
			{
				info("Please provide a component name.\n");
			}
		}
		break; case 'l':
					 case 'L':
		{
				auto component_names = Board::instance()->get_names();

 				if (component_names.empty())
				{
					info("No component found.");
					return;
				}
 
				for (auto n : component_names)
				{
					log(n, '\n');
				}
		}
		break; case 's':
					 case 'S':
		{
			std::string delimiter = " ";

			if (auto id = str.find(delimiter); id < str.size() - 1)
			{
				std::string name = std::string(str.substr(id+1));

				auto board = Board::instance();

				if (board->found(name))
				{
						if (board->get_component(name)->type != GateType::CUSTOM)
						{
							log("Gate type `", name, "` is built in, getting context is forbidden.\n");
						}
						else
						{
							board->set_context(name);
							log("Context switched, current: ", name, '\n');
						}
				}
				else
				{
					log("Component `", name, "` not found.\n");
				}
			}
			else
			{
				info("Please provide a component name.");
			}
		}
		break; default:
		{
				info("Invalid command.");
		}
	}
}

int main()
{
	Board board;
	std::string line;

	greet();

	while(prompt() && std::getline(std::cin, line))
	{
		if (line.empty()) continue;
		handle_input(line);
		newline();
	}
}