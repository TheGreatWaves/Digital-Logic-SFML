#ifndef COMPUTER_HPP
#define COMPUTER_HPP

#include <cstdint>
#include <array>
#include <iostream>
#include <iomanip>

namespace emulator {

namespace instruction {

struct Instruction
{
 uint16_t C_instruction : 1 {0}; // C instruction
 uint16_t A_instruction : 1 {0}; // A instruction
 uint16_t write_A        : 1 {0}; // Load A
 uint16_t write_D        : 1 {0}; // Load D
 uint16_t read_memory   : 1 {0}; // Use memory instead of address (A)
 uint16_t jlz           : 1 {0}; // Cond: < 0
 uint16_t jez           : 1 {0}; // Cond: = 0
 uint16_t jgz           : 1 {0}; // Cond: > 0
 uint16_t raw               {0}; // Instruction value
 uint16_t zx            : 1 {0}; // Zero x input
 uint16_t nx            : 1 {0}; // Negate x input
 uint16_t zy            : 1 {0}; // Zero y input
 uint16_t ny            : 1 {0}; // Negate y input
 uint16_t f             : 1 {0}; // 1: Add, 0: And
 uint16_t no            : 1 {0}; // Negate output
 uint16_t write_memory  : 1 {0}; // Write to memory
};

inline auto from_uint16_t(uint16_t instruction) -> Instruction 
{
 const bool C_instruction = static_cast<uint16_t>(instruction >> 15) & 1;
 const bool A_instruction = (~C_instruction) & 1;

 const bool dest_A = (instruction >> 5) & 1;
 const bool dest_D = (instruction >> 4) & 1;
 const bool dest_M = (instruction >> 3) & 1;

 const bool load_A = A_instruction || (C_instruction && dest_A);
 const bool load_D = C_instruction && dest_D;

 const bool memory = (instruction >> 12) & 1;

 const bool jlz = (instruction >> 2) & 1;
 const bool jez = (instruction >> 1) & 1;
 const bool jgz = (instruction >> 0) & 1;

 const bool zx = (instruction >> 11) & 1; 
 const bool nx = (instruction >> 10) & 1; 
 const bool zy = (instruction >> 9) & 1; 
 const bool ny = (instruction >> 8) & 1; 
 const bool f  = (instruction >> 7) & 1; 
 const bool no = (instruction >> 6) & 1; 

 const bool write_memory = C_instruction && dest_M;

 return Instruction 
 {  
  .C_instruction = C_instruction,
  .A_instruction = A_instruction,
  .write_A        = load_A,
  .write_D        = load_D,
  .read_memory        = memory,
  .jlz           = jlz,
  .jez           = jez,
  .jgz           = jgz,
  .raw           = instruction,
  .zx            = zx, 
  .nx            = nx, 
  .zy            = zy, 
  .ny            = ny, 
  .f             = f, 
  .no            = no,
  .write_memory  = write_memory
 };
}
 
} // namespace instruction

namespace alu {

struct ALUResult
{
 uint32_t out : 16 {0}; // 16-bit Output
 uint32_t zr  : 1  {0}; // out == 0
 uint32_t ng  : 1  {0}; // out < 0
};

struct ALUArgs
{
 uint32_t x  : 16 {0}; // First operand
 uint32_t y  : 16 {0}; // Second operand
 uint32_t zx : 1  {0}; // Zero x operand
 uint32_t nx : 1  {0}; // Negate x operand
 uint32_t zy : 1  {0}; // Zero y operand
 uint32_t ny : 1  {0}; // Negate y operand
 uint32_t f  : 1  {0}; // Add: 1, And: 0
 uint32_t no : 1  {0}; // Negate output
};

 inline auto args_from_instruction(uint16_t x, uint16_t y, const instruction::Instruction& instruction) -> ALUArgs
 {
   alu::ALUArgs args {};
   args.x = x; 
   args.y = y;
   args.zx = instruction.zx; 
   args.nx = instruction.nx; 
   args.zy = instruction.zy; 
   args.ny = instruction.ny; 
   args.f = instruction.f; 
   args.no = instruction.no; 
   return args;
 }

inline auto compute(ALUArgs&& args) -> ALUResult {
 if (args.zx) args.x = 0;
 if (args.nx) args.x = ~args.x;
 if (args.zy) args.y = 0;
 if (args.ny) args.y = ~args.y;

 uint16_t result = args.f 
                 ? args.x + args.y 
                 : args.x & args.y;

 if (args.no) result = ~result;

 return ALUResult{
  .out = result,
  .zr  = result == 0,
  .ng  = static_cast<uint32_t>((result >> 15))
 };
}
 
} // namespace alu

class Computer
{
public:
 /**
  * Default Ctor
  */
  constexpr Computer() 
  {
   set_up_memory();
  }

 /**
  * Methods
  */
 inline auto process() -> void
 {
  const auto instruction = fetch();
  // std::cout << "instruction: " << instruction.raw << '\n';

  // Handle A instruction.
  if (instruction.A_instruction)
  {
   write_A(instruction.raw);
   m_pc++;
  }
  // Handle C instruction.
  else 
  {
   const auto x = fetch_operand_x(); 
   const auto y = fetch_operand_y(instruction.read_memory) ;

   auto args = alu::args_from_instruction(x, y, instruction);
   const auto result = alu::compute(std::move(args));

   // Handle write
   if (instruction.write_memory) write_M(result.out);
   if (instruction.write_A) write_A(result.out);
   if (instruction.write_D) write_D(result.out);

   // Handle jump
   const auto positive = !result.ng;
   const auto jez = instruction.jez && result.zr;

   // std::cout << "\tx: " << x << '\n';
   // std::cout << "\ty: " << y << '\n';
   // std::cout << "\tresult: " << result.out << '\n';
   // std::cout << "\tng: " << result.ng << '\n';
   // std::cout << "\tzr: " << result.zr << '\n';
   // std::cout << "\tjez: " << instruction.jez << '\n';
   // std::cout << "\tjlz: " << instruction.jlz << '\n';
   // std::cout << "\tjgz: " << instruction.jgz << '\n';
   
   const auto jgz = instruction.jgz && positive && !result.zr;
   const auto jlz = instruction.jlz && result.ng && !result.zr;
   const auto jlez = jez || jlz;
   const auto jgez = jez || jgz;
   const auto jump = jlez || jgez;
   // std::cout << "\tjump: " << jump << '\n';

   if (jump)
    write_pc(m_A);
   else
    m_pc++;
  }
 }

 inline auto print_state() -> void
 {
  std::cout << "D " << m_D << '\n';
  std::cout << "A " << m_A << '\n';
  std::cout << "PC " << m_pc << '\n';

  for (std::size_t i{0}; i < 8; i++)
  {
   std::cout << "Ram[" << std::setw(3) << std::right << i << "] " << m_ram[i] << '\n';
  }

  for (std::size_t i{0}; i < 10; i++)
  {
   std::cout << "Stack[" << std::setw(3) << std::right << i << "] " << static_cast<int16_t>(m_ram[i + 256]) << '\n';
  }

  for (std::size_t i{0}; i < 5; i++)
  {
   std::cout << "Static[" << std::setw(3) << std::right << i << "] " << m_ram[i + 16] << '\n';
  }

  for (std::size_t i{0}; i < 10; i++)
  {
   std::cout << "Local[" << std::setw(3) << std::right << i << "] " << m_ram[i + 300] << '\n';
  }
 }


 inline auto process(std::size_t cycles) -> void
 {
  for (std::size_t cycle {0}; cycle < cycles; cycle++)
   process();
 }

 inline auto load_instructions(const std::array<uint16_t, 32768>& instruction)
 {
  m_instruction = instruction;
 }

 inline auto reset() -> void
 {
  m_pc = 0;
 }

private:
 inline auto fetch_operand_x() -> uint16_t 
 {
  return m_D;
 }

 inline auto fetch_operand_y(bool from_memory = false) -> uint16_t 
 {
  return from_memory
       ? m_ram[m_A]
       : m_A;
 }

 inline auto write_A(uint16_t value) -> void
 {
  // std::cout << "\t\t\tWrite A: " << value << '\n';
  m_A = value;
 }

 inline auto write_D(uint16_t value) -> void
 {
  // std::cout << "\t\t\tWrite D: " << value << '\n';
  m_D = value;
 }

 inline auto write_pc(uint16_t value) -> void
 {
  m_pc = value;
 }

 inline auto write_M(uint16_t value) -> void
 {
  // std::cout << "\t\t\tWrite RAM[" << m_A << "]: " << value << '\n';
  m_ram[m_A] = value;
 }

 inline auto fetch() -> instruction::Instruction 
 {
  return instruction::from_uint16_t(m_instruction.at(m_pc)); 
 }

 /**
  * Stack Operations
  */
 inline auto set_up_memory() -> void
 {
  m_ram[0] = 256;  // Stack Pointer
  m_ram[1] = 300;  // Base address of local
  m_ram[2] = 400;  // Base address of argument
  m_ram[3] = 3000; // Base address of this
  m_ram[4] = 3010; // Base address of that
 }

 inline auto increment_stack_pointer() -> void
 {
  m_ram[0]++;
 }

 inline auto decrement_stack_pointer() -> void
 {
  m_ram[0]--;
 }

 inline auto get_stack_pointer() -> uint16_t
 {
  return m_ram[0];
 }

 inline auto stack_push_constant(uint16_t value) -> void
 {
  m_ram[get_stack_pointer()] = value;
  increment_stack_pointer();
 }


private:
 /**
  *  Members
  */
 uint16_t m_pc                             {0}; // Program counter
 uint16_t m_D                              {0}; // D Register
 uint16_t m_A                              {0}; // A Register
 std::array<uint16_t, 16384> m_ram         {0}; // Memory
 std::array<uint16_t, 32768> m_instruction {0}; // Instruction memory
};
 
} // namespace emulator

#endif // COMPUTER_HPP
