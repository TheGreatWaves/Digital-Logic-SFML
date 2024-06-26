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
#ifndef VM_H
#define VM_H

#include <string>
#include <sstream>
#include <string_view>
#include <filesystem>
namespace fs = std::filesystem;

#include "../core/parser_base.hpp"
#include "token_vm.hpp"
#include "../assembler/assembler.hpp"

class CodeStringBuilder
{
public:
 [[nodiscard]] auto build() const -> std::string 
 {
  return m_code.str();
 }

 auto increment() -> void
 {
  m_size++;
 }

 auto loc() -> std::size_t 
 {
  return m_size;
 }

 auto write_A(uint16_t value) -> CodeStringBuilder&
 {
  m_code << '\t' << "@" << value << '\n';
  increment();
  return *this;
 }

 auto write_A(std::string_view label) -> CodeStringBuilder&
 {
  m_code << '\t' << "@" << label << '\n';
  increment();
  return *this;
 }

 template <typename T>
 auto write_A(std::string_view file_name, T variable_name, std::string_view separator = ".") -> CodeStringBuilder&
 {
  m_code << '\t' << "@" << file_name << separator << variable_name << '\n';
  increment();
  return *this;
 }

 auto newline() -> CodeStringBuilder&
 {
  m_code << '\n';
  return *this;
 }


 
 auto write_comment(std::convertible_to<std::string_view> auto&& ... comment) -> CodeStringBuilder&
 {
  m_code << "// ";
  for (auto v : std::initializer_list<std::string_view>{ comment... })
    m_code << v << " ";
  m_code << '\n';
  return *this;
 }

 auto write_assignment(std::string_view dest, std::string_view source) -> CodeStringBuilder&
 {
  m_code << '\t' << dest << "=" << source << '\n';
  increment();
  return *this;
 }

 auto write_jump(std::string_view value, std::string_view condition) -> CodeStringBuilder&
 {
  m_code << '\t' << value << ";" << condition << '\n';
  increment();
  return *this;
 }

 auto write_label(std::string_view name) -> CodeStringBuilder&
 {
  m_code << '(' << name << ')' << '\n';
  increment();
  return *this;
 }

 auto write_label(std::string_view name, std::uint16_t count) -> CodeStringBuilder&
 {
  m_code << '(' << name << '_' << count << ')' << '\n';
  increment();
  return *this;
 }

private: 
 std::stringstream m_code {};
 std::size_t       m_size {};
};

class VMTranslator : BaseParser<VMTokenType>
{
private:
 using TokenType = VMTokenType;

public:
 [[nodiscard]] explicit VMTranslator(const std::string& file_path)
     : BaseParser<VMTokenType>(file_path)
     , m_assembler{}
     , m_filename{fs::path(file_path).stem()}
 {}

 auto print() noexcept -> void
 {
  m_assembler.print_code();
 }

 auto loc() noexcept -> const std::size_t 
 {
  return m_builder.loc();
 }

 [[nodiscard]] auto to_instructions() const -> const std::array<uint16_t, 32768> 
 {
  return m_assembler.to_instructions();
 }

 /**
  * Parse the source code and excute the instructions.
  */
 [[nodiscard]] auto parse() noexcept -> bool
 {
  advance();

  while (!match(TokenType::EndOfFile))
   instruction();

  const auto source = m_builder.build();
  std::cout << source << '\n';
  m_assembler.set_source(source);

  if (!m_assembler.parse())
   return false;

  return !this->has_error;
 }

 auto instruction() -> void
 {
  if (match(TokenType::Push))
   handle_push();
  else if (match(TokenType::Pop))
   handle_pop();
  else if (match(TokenType::Add))
   handle_add();
  else if (match(TokenType::And))
   handle_and();
  else if (match(TokenType::Or))
   handle_or();
  else if (match(TokenType::Sub))
   handle_sub();
  else if (match(TokenType::Neg))
   handle_neg();
  else if (match(TokenType::Not))
   handle_not();
  else if (match(TokenType::Eq))
   handle_eq();
  else if (match(TokenType::Gt))
   handle_gt();
  else if (match(TokenType::Lt))
   handle_lt();
  else if (match(TokenType::Label))
   handle_label();
  else if (match(TokenType::Goto))
   handle_goto();
  else if (match(TokenType::If))
   handle_if_goto();
  else
  {
   const std::string current = this->current.lexeme;
   report_error("Invalid token: " + current);
  }

  // TODO: Handle error.
  if (this->has_error) advance();
 }

 auto write_pop_segment(std::string_view segment) -> void
 {
     advance();
     const std::string segment_name = previous.lexeme;
     consume(TokenType::Number, "Expected index after '" + segment_name + "'");
     const std::string index = previous.lexeme;
     m_builder.write_comment("pop", segment_name, index)
              .write_A("SP")
              .write_assignment("M", "M-1")
              .write_assignment("A", "M")
              .write_assignment("D", "M")
              .write_A(segment)
              .write_assignment("D", "D+M")
              .write_A(index)
              .write_assignment("D", "D+A")
              .write_A("SP")
              .write_assignment("A", "M")
              .write_assignment("A", "M")
              .write_assignment("A", "D-A")
              .write_assignment("M", "D-A")
              .newline();
 }
 
 auto write_push_segment(std::string_view segment) -> void
 {
     advance();
     const std::string segment_name = previous.lexeme;
     consume(TokenType::Number, "Expected index after '" + segment_name + "'");
     const std::string index = previous.lexeme;
     m_builder.write_comment("push", segment_name, index)
              .write_A(segment)
              .write_assignment("D", "M")
              .write_A(index)
              .write_assignment("A", "D+A")
              .write_assignment("D", "M")
              .write_A("SP")
              .write_assignment("A", "M")
              .write_assignment("M", "D")
              .write_A("SP")
              .write_assignment("M", "M+1")
              .newline();
 }

 auto handle_push() -> void 
 {
  switch (this->current.type)
  {
    break; case TokenType::Constant: 
    {
     advance();
     consume(TokenType::Number, "Expected index after 'constant'");
     const std::string index = previous.lexeme;

     m_builder.write_comment("push constant", index)
              .write_A(index)
              .write_assignment("D", "A")
              .write_A("SP")
              .write_assignment("A", "M")
              .write_assignment("M", "D")
              .write_A("SP")
              .write_assignment("M", "M+1")
              .newline();
    }
    break; case TokenType::Static:
    {
     advance();
     consume(TokenType::Number, "Expected index after 'static'");
     const std::string index = previous.lexeme;

     m_builder.write_comment("push static", index)
              .write_A(m_filename, index)
              .write_assignment("D", "M")
              .write_A("SP")
              .write_assignment("A", "M")
              .write_assignment("M", "D")
              .write_A("SP")
              .write_assignment("M", "M+1")
              .newline();
    }
    break; case TokenType::Temp:
    {
     advance();
     consume(TokenType::Number, "Expected index after 'temp'");
     const std::string index_string = previous.lexeme;
     const uint32_t index = std::stoi(index_string);

     if (index > 7) report_error("Temp index out of range: " + index_string);

     m_builder.write_comment("push temp", index_string)
              .write_A(index + 5)
              .write_assignment("D", "M")
              .write_A("SP")
              .write_assignment("A", "M")
              .write_assignment("M", "D")
              .write_A("SP")
              .write_assignment("M", "M+1")
              .newline();
    }
    break; case TokenType::Pointer:
    {
     advance();
     consume(TokenType::Number, "Expected index after 'pointer'");
     const std::string index = previous.lexeme;

     if (index != "1" && index != "0") report_error("Invalid pointer for push");

     m_builder.write_comment("push pointer", index)
              .write_A(index == "1" ? "THAT" : "THIS")
              .write_assignment("D", "M")
              .write_A("SP")
              .write_assignment("A", "M")
              .write_assignment("M", "D")
              .write_A("SP")
              .write_assignment("M", "M+1")
              .newline();
    }
    break; case TokenType::Local:    write_push_segment("LCL");
    break; case TokenType::Argument: write_push_segment("ARG");
    break; case TokenType::This:     write_push_segment("THIS");
    break; case TokenType::That:     write_push_segment("THAT");
    break; default: { report_error("Unexpected segment found in push statement"); }
  }
 }
 
 auto handle_pop() -> void 
 {
  switch (this->current.type)
  {
    break; case TokenType::Static:
    {
     advance();
     consume(TokenType::Number, "Expected index after 'static'");
     const std::string index = previous.lexeme;

     m_builder.write_comment("pop static", index)
              .write_A("SP")
              .write_assignment("M", "M-1")
              .write_assignment("A", "M")
              .write_assignment("D", "M")
              .write_A(m_filename, index)
              .write_assignment("M", "D")
              .newline();
    }
    break; case TokenType::Temp:
    {
     advance();
     consume(TokenType::Number, "Expected index after 'temp'");
     const std::string index_string = previous.lexeme;
     const uint32_t index = std::stoi(index_string);

     if (index > 7) report_error("Temp index out of range: " + index_string);

     m_builder.write_comment("pop temp", index_string)
              .write_A("SP")
              .write_assignment("M", "M-1")
              .write_assignment("A", "M")
              .write_assignment("D", "M")
              .write_A(index + 5)
              .write_assignment("M", "D")
              .newline();
    }
    break; case TokenType::Pointer:
    {
     advance();
     consume(TokenType::Number, "Expected index after 'pointer'");
     const std::string index = previous.lexeme;

     if (index != "1" && index != "0") report_error("Invalid pointer for pop");

     m_builder.write_comment("pop pointer", index)
              .write_A("SP")
              .write_assignment("M", "M-1")
              .write_assignment("A", "M")
              .write_assignment("D", "M")
              .write_A(index == "1" ? "THAT" : "THIS")
              .write_assignment("M", "D")
              .newline();
    }
    break; case TokenType::Local:    write_pop_segment("LCL");
    break; case TokenType::Argument: write_pop_segment("ARG");
    break; case TokenType::This:     write_pop_segment("THIS");
    break; case TokenType::That:     write_pop_segment("THAT");
    break; default: { report_error("Unexpected segment found in pop statement"); }
  }
 }

 auto handle_add() -> void 
 {
  m_builder.write_comment("add")
           .write_A("SP")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("A", "A-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A+1")
           .write_assignment("D", "D+M") // This changes for different function
           .write_A("SP")
           .write_assignment("M", "M-1")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("M", "D")
           .newline();
 }

 auto handle_and() -> void 
 {
  m_builder.write_comment("and")
           .write_A("SP")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("A", "A-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A+1")
           .write_assignment("D", "D&M") // This changes for different function
           .write_A("SP")
           .write_assignment("M", "M-1")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("M", "D")
           .newline();
 }

 auto handle_or() -> void 
 {
  m_builder.write_comment("or")
           .write_A("SP")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("A", "A-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A+1")
           .write_assignment("D", "D|M") // This changes for different function
           .write_A("SP")
           .write_assignment("M", "M-1")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("M", "D")
           .newline();
 }

 auto handle_sub() -> void 
 {
  m_builder.write_comment("sub")
           .write_A("SP")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("A", "A-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A+1")
           .write_assignment("D", "D-M") // This changes for different function
           .write_A("SP")
           .write_assignment("M", "M-1")
           .write_assignment("A", "M")
           .write_assignment("A", "A-1")
           .write_assignment("M", "D")
           .newline();
 }

 auto handle_neg() -> void
 {
  m_builder.write_comment("neg")
           .write_A("SP")
           .write_assignment("A", "M-1")
           .write_assignment("M", "-M")
           .newline();
 }

 auto handle_not() -> void
 {
  m_builder.write_comment("not")
           .write_A("SP")
           .write_assignment("A", "M-1")
           .write_assignment("M", "!M")
           .newline();
 }

 auto handle_eq() -> void
 {
  m_builder.write_comment("eq")
           .write_A("SP")
           .write_assignment("AM", "M-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A-1")
           .write_assignment("D", "D-M")
           .write_assignment("M", "-1")
           .write_A("EQ_label", m_count, "_")
           .write_jump("D", "JEQ")
           .write_A("SP")
           .write_assignment("A", "M-1")
           .write_assignment("M", "0")
           .write_label("EQ_label", m_count)
           .newline();
  m_count++;
 }

 auto handle_gt() -> void
 {
  m_builder.write_comment("gt")
           .write_A("SP")
           .write_assignment("AM", "M-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A-1")
           .write_assignment("D", "D-M")
           .write_assignment("M", "-1")
           .write_A("GT_label", m_count, "_")
           .write_jump("D", "JLT")
           .write_A("SP")
           .write_assignment("A", "M-1")
           .write_assignment("M", "0")
           .write_label("GT_label", m_count)
           .newline();
  m_count++;
 }

 auto handle_lt() -> void
 {
  m_builder.write_comment("lt")
           .write_A("SP")
           .write_assignment("AM", "M-1")
           .write_assignment("D", "M")
           .write_assignment("A", "A-1")
           .write_assignment("D", "D-M")
           .write_assignment("M", "-1")
           .write_A("LT_label", m_count, "_")
           .write_jump("D", "JGT")
           .write_A("SP")
           .write_assignment("A", "M-1")
           .write_assignment("M", "0")
           .write_label("LT_label", m_count)
           .newline();
  m_count++;
 }

 auto handle_label() -> void
 {
  consume(TokenType::Identifier, "Expected label name");
  const std::string label_name = previous.lexeme;
  m_builder.write_label(label_name);
 }
 
 auto handle_goto() -> void
 {
  consume(TokenType::Identifier, "Expected label name");
  const std::string label_name = previous.lexeme;
  m_builder.write_A(label_name)
           .write_jump("0", "JMP");
 }

 auto handle_if_goto() -> void
 {
  consume(TokenType::Dash, "Expected '-' after if");
  consume(TokenType::Goto, "Expected 'goto' after '-'");
  consume(TokenType::Identifier, "Expected label name");
  const std::string label_name = previous.lexeme;
  m_builder.write_A("SP")
           .write_assignment("AM", "M-1")
           .write_assignment("D", "M")
           .write_A(label_name)
           .write_jump("D", "JGT");
 }

private:
 Assembler         m_assembler {};
 CodeStringBuilder m_builder   {};
 const std::string m_filename  {};
 std::uint16_t     m_count     {};
};

#endif // VM_H
