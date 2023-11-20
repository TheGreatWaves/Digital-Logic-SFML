/** 
 * MIT License
 * 
 * Copyright (c) 2023 Ochawin A.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#ifndef COMMON
#define COMMON

#include <string>

/**
 * Attributes related to saving gate information.
 */
constexpr const char* DEFAULT_GATE_DIRECTORY{ "gates" };
constexpr const char* DEFAULT_RECIPE_SAVE_DIRECTORY{ "sketches" };
constexpr const char* GATE_EXTENSION{ ".gate" };
constexpr const char* META_EXTENSION{ ".meta" };
constexpr const char* HDL_EXTENSION{ ".hdl" };

const std::string SCRIPTS_DIR{ "scripts" };

const std::string SEPERATOR {"/"};
const std::string GATE_RECIPE_DIRECTORY = DEFAULT_GATE_DIRECTORY + SEPERATOR + DEFAULT_RECIPE_SAVE_DIRECTORY + SEPERATOR;

inline auto hdl_file(const std::string& name) -> std::string 
{
 return std::move(SCRIPTS_DIR + SEPERATOR + name + HDL_EXTENSION);
}

/**
 * Max number of input pins, referring to a pin ID bigger than 
 * this number wi return an output pin.
 */
constexpr std::size_t INPUT_PIN_LIMIT{ 100 };

/**
 * GUI wire's signal speed.
 */
constexpr float WIRE_GROWTH_RATE = 900.f;

#endif /* COMMON */