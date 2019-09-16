#include <gameboy/cpu.hpp>

#include <common/instructionbuilder.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>

using namespace Amber;
using namespace Common;
using namespace Gameboy;

static uint64_t g_Counter;

CPU::CPU(Memory16& a_Memory):
	m_Memory(a_Memory)
{
	InstructionBuilder<Opcode::Enum, MicroOp, &CPU::Break> instruction_builder;
	InstructionBuilder<ExtendedOpcode::Enum, MicroOp, &CPU::Break> extended_instruction_builder;
	
	for (size_t i = 0; i < 256; ++i)
	{
		const auto opcode = static_cast<Opcode::Enum>(i);
		const auto extended_opcode = static_cast<ExtendedOpcode::Enum>(i);

		instruction_builder.Begin(opcode, &CPU::NotImplemented);
		extended_instruction_builder.Begin(extended_opcode, &CPU::NotImplemented);
	}

	// Misc instructions
	instruction_builder.Begin(Opcode::NOP);
	instruction_builder.Begin(Opcode::DI, &CPU::Delay<&CPU::DisableInterrupts, 1>);
	instruction_builder.Begin(Opcode::EI, &CPU::Delay<&CPU::EnableInterrupts, 1>);
	instruction_builder.Begin(Opcode::HALT, &CPU::Delay<&CPU::Halt, 1>);
	instruction_builder.Begin(Opcode::EXT, &CPU::DecodeExtendedInstruction);
	instruction_builder.Begin(Opcode::BREAKPOINT_STOP, &CPU::BreakpointStop);
	instruction_builder.Begin(Opcode::BREAKPOINT_CONTINUE, &CPU::BreakpointContinue);

	// 8-bit load register to register
	instruction_builder.Begin(Opcode::LD_A_A, &CPU::LD_r_r<RegisterA, RegisterA>);
	instruction_builder.Begin(Opcode::LD_A_B, &CPU::LD_r_r<RegisterA, RegisterB>);
	instruction_builder.Begin(Opcode::LD_A_C, &CPU::LD_r_r<RegisterA, RegisterC>);
	instruction_builder.Begin(Opcode::LD_A_D, &CPU::LD_r_r<RegisterA, RegisterD>);
	instruction_builder.Begin(Opcode::LD_A_E, &CPU::LD_r_r<RegisterA, RegisterE>);
	instruction_builder.Begin(Opcode::LD_A_H, &CPU::LD_r_r<RegisterA, RegisterH>);
	instruction_builder.Begin(Opcode::LD_A_L, &CPU::LD_r_r<RegisterA, RegisterL>);

	instruction_builder.Begin(Opcode::LD_B_A, &CPU::LD_r_r<RegisterB, RegisterA>);
	instruction_builder.Begin(Opcode::LD_B_B, &CPU::LD_r_r<RegisterB, RegisterB>);
	instruction_builder.Begin(Opcode::LD_B_C, &CPU::LD_r_r<RegisterB, RegisterC>);
	instruction_builder.Begin(Opcode::LD_B_D, &CPU::LD_r_r<RegisterB, RegisterD>);
	instruction_builder.Begin(Opcode::LD_B_E, &CPU::LD_r_r<RegisterB, RegisterE>);
	instruction_builder.Begin(Opcode::LD_B_H, &CPU::LD_r_r<RegisterB, RegisterH>);
	instruction_builder.Begin(Opcode::LD_B_L, &CPU::LD_r_r<RegisterB, RegisterL>);

	instruction_builder.Begin(Opcode::LD_C_A, &CPU::LD_r_r<RegisterC, RegisterA>);
	instruction_builder.Begin(Opcode::LD_C_B, &CPU::LD_r_r<RegisterC, RegisterB>);
	instruction_builder.Begin(Opcode::LD_C_C, &CPU::LD_r_r<RegisterC, RegisterC>);
	instruction_builder.Begin(Opcode::LD_C_D, &CPU::LD_r_r<RegisterC, RegisterD>);
	instruction_builder.Begin(Opcode::LD_C_E, &CPU::LD_r_r<RegisterC, RegisterE>);
	instruction_builder.Begin(Opcode::LD_C_H, &CPU::LD_r_r<RegisterC, RegisterH>);
	instruction_builder.Begin(Opcode::LD_C_L, &CPU::LD_r_r<RegisterC, RegisterL>);

	instruction_builder.Begin(Opcode::LD_D_A, &CPU::LD_r_r<RegisterD, RegisterA>);
	instruction_builder.Begin(Opcode::LD_D_B, &CPU::LD_r_r<RegisterD, RegisterB>);
	instruction_builder.Begin(Opcode::LD_D_C, &CPU::LD_r_r<RegisterD, RegisterC>);
	instruction_builder.Begin(Opcode::LD_D_D, &CPU::LD_r_r<RegisterD, RegisterD>);
	instruction_builder.Begin(Opcode::LD_D_E, &CPU::LD_r_r<RegisterD, RegisterE>);
	instruction_builder.Begin(Opcode::LD_D_H, &CPU::LD_r_r<RegisterD, RegisterH>);
	instruction_builder.Begin(Opcode::LD_D_L, &CPU::LD_r_r<RegisterD, RegisterL>);

	instruction_builder.Begin(Opcode::LD_E_A, &CPU::LD_r_r<RegisterE, RegisterA>);
	instruction_builder.Begin(Opcode::LD_E_B, &CPU::LD_r_r<RegisterE, RegisterB>);
	instruction_builder.Begin(Opcode::LD_E_C, &CPU::LD_r_r<RegisterE, RegisterC>);
	instruction_builder.Begin(Opcode::LD_E_D, &CPU::LD_r_r<RegisterE, RegisterD>);
	instruction_builder.Begin(Opcode::LD_E_E, &CPU::LD_r_r<RegisterE, RegisterE>);
	instruction_builder.Begin(Opcode::LD_E_H, &CPU::LD_r_r<RegisterE, RegisterH>);
	instruction_builder.Begin(Opcode::LD_E_L, &CPU::LD_r_r<RegisterE, RegisterL>);

	instruction_builder.Begin(Opcode::LD_H_A, &CPU::LD_r_r<RegisterH, RegisterA>);
	instruction_builder.Begin(Opcode::LD_H_B, &CPU::LD_r_r<RegisterH, RegisterB>);
	instruction_builder.Begin(Opcode::LD_H_C, &CPU::LD_r_r<RegisterH, RegisterC>);
	instruction_builder.Begin(Opcode::LD_H_D, &CPU::LD_r_r<RegisterH, RegisterD>);
	instruction_builder.Begin(Opcode::LD_H_E, &CPU::LD_r_r<RegisterH, RegisterE>);
	instruction_builder.Begin(Opcode::LD_H_H, &CPU::LD_r_r<RegisterH, RegisterH>);
	instruction_builder.Begin(Opcode::LD_H_L, &CPU::LD_r_r<RegisterH, RegisterL>);

	instruction_builder.Begin(Opcode::LD_L_A, &CPU::LD_r_r<RegisterL, RegisterA>);
	instruction_builder.Begin(Opcode::LD_L_B, &CPU::LD_r_r<RegisterL, RegisterB>);
	instruction_builder.Begin(Opcode::LD_L_C, &CPU::LD_r_r<RegisterL, RegisterC>);
	instruction_builder.Begin(Opcode::LD_L_D, &CPU::LD_r_r<RegisterL, RegisterD>);
	instruction_builder.Begin(Opcode::LD_L_E, &CPU::LD_r_r<RegisterL, RegisterE>);
	instruction_builder.Begin(Opcode::LD_L_H, &CPU::LD_r_r<RegisterL, RegisterH>);
	instruction_builder.Begin(Opcode::LD_L_L, &CPU::LD_r_r<RegisterL, RegisterL>);

	// 8-bit load next byte to register
	instruction_builder.Begin(Opcode::LD_A_n).Cycle(&CPU::LD_r_n<RegisterA>);
	instruction_builder.Begin(Opcode::LD_B_n).Cycle(&CPU::LD_r_n<RegisterB>);
	instruction_builder.Begin(Opcode::LD_C_n).Cycle(&CPU::LD_r_n<RegisterC>);
	instruction_builder.Begin(Opcode::LD_D_n).Cycle(&CPU::LD_r_n<RegisterD>);
	instruction_builder.Begin(Opcode::LD_E_n).Cycle(&CPU::LD_r_n<RegisterE>);
	instruction_builder.Begin(Opcode::LD_H_n).Cycle(&CPU::LD_r_n<RegisterH>);
	instruction_builder.Begin(Opcode::LD_L_n).Cycle(&CPU::LD_r_n<RegisterL>);

	// 8-bit load byte at address to register
	instruction_builder.Begin(Opcode::LD_A_aBC).Cycle(&CPU::LD_r_arr<RegisterA, RegisterBC>);
	instruction_builder.Begin(Opcode::LD_A_aDE).Cycle(&CPU::LD_r_arr<RegisterA, RegisterDE>);
	instruction_builder.Begin(Opcode::LD_A_aHL).Cycle(&CPU::LD_r_arr<RegisterA, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_B_aHL).Cycle(&CPU::LD_r_arr<RegisterB, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_C_aHL).Cycle(&CPU::LD_r_arr<RegisterC, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_D_aHL).Cycle(&CPU::LD_r_arr<RegisterD, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_E_aHL).Cycle(&CPU::LD_r_arr<RegisterE, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_H_aHL).Cycle(&CPU::LD_r_arr<RegisterH, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_L_aHL).Cycle(&CPU::LD_r_arr<RegisterL, RegisterHL>);

	// 8-bit load misc to register
	instruction_builder.Begin(Opcode::LD_A_ann).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::LD_r_arr<RegisterA, RegisterXY>);
	instruction_builder.Begin(Opcode::LD_A_aFFC).Cycle(&CPU::LD_rr_FFr<RegisterXY, RegisterC>, &CPU::LD_r_arr<RegisterA, RegisterXY>);
	instruction_builder.Begin(Opcode::LD_A_aFFn).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::LD_rr_FFr<RegisterXY, RegisterX>).Cycle(&CPU::LD_r_arr<RegisterA, RegisterXY>);

	// 8 bit load register to address
	instruction_builder.Begin(Opcode::LD_aBC_A).Cycle(&CPU::LD_arr_r<RegisterBC, RegisterA>);
	instruction_builder.Begin(Opcode::LD_aDE_A).Cycle(&CPU::LD_arr_r<RegisterDE, RegisterA>);
	instruction_builder.Begin(Opcode::LD_aHL_A).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterA>);
	instruction_builder.Begin(Opcode::LD_aHL_B).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterB>);
	instruction_builder.Begin(Opcode::LD_aHL_C).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterC>);
	instruction_builder.Begin(Opcode::LD_aHL_D).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterD>);
	instruction_builder.Begin(Opcode::LD_aHL_E).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterE>);
	instruction_builder.Begin(Opcode::LD_aHL_H).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterH>);
	instruction_builder.Begin(Opcode::LD_aHL_L).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterL>);

	// 8 bit load next byte to address
	instruction_builder.Begin(Opcode::LD_aHL_n).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 8-bit load misc to address
	instruction_builder.Begin(Opcode::LD_ann_A).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::LD_arr_r<RegisterXY, RegisterA>);
	instruction_builder.Begin(Opcode::LD_aFFC_A).Cycle(&CPU::LD_rr_FFr<RegisterXY, RegisterC>, &CPU::LD_arr_r<RegisterXY, RegisterA>);
	instruction_builder.Begin(Opcode::LD_aFFn_A).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::LD_rr_FFr<RegisterXY, RegisterX>).Cycle(&CPU::LD_arr_r<RegisterXY, RegisterA>);

	// 8-bit load and increment/decrement
	instruction_builder.Begin(Opcode::LDI_A_aHL).Cycle(&CPU::LD_r_arr<RegisterA, RegisterHL>, &CPU::UnaryOp_rr<RegisterHL, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::LDD_A_aHL).Cycle(&CPU::LD_r_arr<RegisterA, RegisterHL>, &CPU::UnaryOp_rr<RegisterHL, &CPU::Decrement16>);
	instruction_builder.Begin(Opcode::LDI_aHL_A).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterA>, &CPU::UnaryOp_rr<RegisterHL, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::LDD_aHL_A).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterA>, &CPU::UnaryOp_rr<RegisterHL, &CPU::Decrement16>);

	// 16-bit loads
	instruction_builder.Begin(Opcode::LD_BC_nn).Cycle(&CPU::LD_r_n<RegisterC>).Cycle(&CPU::LD_r_n<RegisterB>);
	instruction_builder.Begin(Opcode::LD_DE_nn).Cycle(&CPU::LD_r_n<RegisterE>).Cycle(&CPU::LD_r_n<RegisterD>);
	instruction_builder.Begin(Opcode::LD_HL_nn).Cycle(&CPU::LD_r_n<RegisterL>).Cycle(&CPU::LD_r_n<RegisterH>);
	instruction_builder.Begin(Opcode::LD_HL_SPn).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::LD_rr_rrr<RegisterHL, RegisterSP, RegisterX>);
	instruction_builder.Begin(Opcode::LD_SP_nn).Cycle(&CPU::LD_r_n<RegisterSP_P>).Cycle(&CPU::LD_r_n<RegisterSP_S>);
	instruction_builder.Begin(Opcode::LD_SP_HL).Cycle(&CPU::LD_rr_rr< RegisterSP, RegisterHL>);
	instruction_builder.Begin(Opcode::LD_ann_SP).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::LD_arr_r<RegisterXY, RegisterSP_P>, &CPU::UnaryOp_rr<RegisterXY, &CPU::Increment16>, &CPU::LD_arr_r<RegisterXY, RegisterSP_S>);

	// 8-bit add instructions
	instruction_builder.Begin(Opcode::ADD_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Add8<false>>);
	instruction_builder.Begin(Opcode::ADD_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Add8<false>>);

	// 16-bit add instructions
	instruction_builder.Begin(Opcode::ADD_HL_BC).Cycle(&CPU::BinaryOp_rr_rr<RegisterHL, RegisterBC, &CPU::Add16>);
	instruction_builder.Begin(Opcode::ADD_HL_DE).Cycle(&CPU::BinaryOp_rr_rr<RegisterHL, RegisterDE, &CPU::Add16>);
	instruction_builder.Begin(Opcode::ADD_HL_HL).Cycle(&CPU::BinaryOp_rr_rr<RegisterHL, RegisterHL, &CPU::Add16>);
	instruction_builder.Begin(Opcode::ADD_HL_SP).Cycle(&CPU::BinaryOp_rr_rr<RegisterHL, RegisterSP, &CPU::Add16>);
	instruction_builder.Begin(Opcode::ADD_SP_n).Cycle(&CPU::LD_r_n<RegisterX>).Cycle().Cycle(&CPU::ADD_rr_r<RegisterSP, RegisterX>);

	// 8-bit subtract instructions
	instruction_builder.Begin(Opcode::SUB_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Subtract8<false>>);
	instruction_builder.Begin(Opcode::SUB_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Subtract8<false>>);

	// 8-bit add + carry instructions
	instruction_builder.Begin(Opcode::ADC_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Add8<true>>);
	instruction_builder.Begin(Opcode::ADC_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Add8<true>>);

	// 8-bit subtract + carry instructions
	instruction_builder.Begin(Opcode::SBC_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Subtract8<true>>);
	instruction_builder.Begin(Opcode::SBC_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Subtract8<true>>);

	// 8-bit AND instructions
	instruction_builder.Begin(Opcode::AND_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::AND8>);
	instruction_builder.Begin(Opcode::AND_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::AND8>);

	// 8-bit OR instructions
	instruction_builder.Begin(Opcode::OR_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::OR8>);
	instruction_builder.Begin(Opcode::OR_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::OR8>);

	// 8-bit XOR instructions
	instruction_builder.Begin(Opcode::XOR_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::XOR8>);
	instruction_builder.Begin(Opcode::XOR_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::XOR8>);

	// 8-bit compare instructions
	instruction_builder.Begin(Opcode::CP_A_A, &CPU::BinaryOp_r_r<RegisterA, RegisterA, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_B, &CPU::BinaryOp_r_r<RegisterA, RegisterB, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_C, &CPU::BinaryOp_r_r<RegisterA, RegisterC, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_D, &CPU::BinaryOp_r_r<RegisterA, RegisterD, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_E, &CPU::BinaryOp_r_r<RegisterA, RegisterE, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_H, &CPU::BinaryOp_r_r<RegisterA, RegisterH, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_L, &CPU::BinaryOp_r_r<RegisterA, RegisterL, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Subtract8<false>, false>);
	instruction_builder.Begin(Opcode::CP_A_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::BinaryOp_r_r<RegisterA, RegisterX, &CPU::Subtract8<false>, false>);

	// 8-bit increment instructions
	instruction_builder.Begin(Opcode::INC_A, &CPU::UnaryOp_r<RegisterA, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_B, &CPU::UnaryOp_r<RegisterB, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_C, &CPU::UnaryOp_r<RegisterC, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_D, &CPU::UnaryOp_r<RegisterD, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_E, &CPU::UnaryOp_r<RegisterE, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_H, &CPU::UnaryOp_r<RegisterH, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_L, &CPU::UnaryOp_r<RegisterL, &CPU::Increment8>);
	instruction_builder.Begin(Opcode::INC_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::UnaryOp_r<RegisterX, &CPU::Increment8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 16-bit increment instructions
	instruction_builder.Begin(Opcode::INC_BC).Cycle(&CPU::UnaryOp_rr<RegisterBC, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::INC_DE).Cycle(&CPU::UnaryOp_rr<RegisterDE, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::INC_HL).Cycle(&CPU::UnaryOp_rr<RegisterHL, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::INC_SP).Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>);

	// 8-bit decrement instructions
	instruction_builder.Begin(Opcode::DEC_A, &CPU::UnaryOp_r<RegisterA, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_B, &CPU::UnaryOp_r<RegisterB, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_C, &CPU::UnaryOp_r<RegisterC, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_D, &CPU::UnaryOp_r<RegisterD, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_E, &CPU::UnaryOp_r<RegisterE, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_H, &CPU::UnaryOp_r<RegisterH, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_L, &CPU::UnaryOp_r<RegisterL, &CPU::Decrement8>);
	instruction_builder.Begin(Opcode::DEC_aHL).Cycle(&CPU::LD_r_arr<RegisterX, RegisterHL>, &CPU::UnaryOp_r<RegisterX, &CPU::Decrement8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 16-bit decrement instructions
	instruction_builder.Begin(Opcode::DEC_BC).Cycle(&CPU::UnaryOp_rr<RegisterBC, &CPU::Decrement16>);
	instruction_builder.Begin(Opcode::DEC_DE).Cycle(&CPU::UnaryOp_rr<RegisterDE, &CPU::Decrement16>);
	instruction_builder.Begin(Opcode::DEC_HL).Cycle(&CPU::UnaryOp_rr<RegisterHL, &CPU::Decrement16>);
	instruction_builder.Begin(Opcode::DEC_SP).Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>);

	// 8-bit rotate instructions
	instruction_builder.Begin(Opcode::RLC_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateLeft8<true>>);
	instruction_builder.Begin(Opcode::RL_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateLeftThroughCarry8<true>>);
	instruction_builder.Begin(Opcode::RRC_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateRight8<true>>);
	instruction_builder.Begin(Opcode::RR_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateRightThroughCarry8<true>>);

	// Absolute jump instructions
	instruction_builder.Begin(Opcode::JP_nn).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::JP_NZ_nn).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagZero, false>).Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::JP_Z_nn).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagZero, true>).Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::JP_NC_nn).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagCarry, false>).Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::JP_C_nn).Cycle(&CPU::LD_r_n<RegisterY>).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagCarry, true>).Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::JP_HL, &CPU::JP_rr<RegisterHL>);

	// Relative jump instructions
	instruction_builder.Begin(Opcode::JR_n).Cycle(&CPU::LD_r_n<RegisterX>).Cycle(&CPU::JR_rr_r<RegisterPC, RegisterX>);
	instruction_builder.Begin(Opcode::JR_NZ_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagZero, false>).Cycle(&CPU::JR_rr_r<RegisterPC, RegisterX>);
	instruction_builder.Begin(Opcode::JR_Z_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagZero, true>).Cycle(&CPU::JR_rr_r<RegisterPC, RegisterX>);
	instruction_builder.Begin(Opcode::JR_NC_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagCarry, false>).Cycle(&CPU::JR_rr_r<RegisterPC, RegisterX>);
	instruction_builder.Begin(Opcode::JR_C_n).Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagCarry, true>).Cycle(&CPU::JR_rr_r<RegisterPC, RegisterX>);

	// Push instructions
	instruction_builder.Begin(Opcode::PUSH_AF)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterA>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterF>)
		.Cycle();
	instruction_builder.Begin(Opcode::PUSH_BC)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterB>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterC>)
		.Cycle();
	instruction_builder.Begin(Opcode::PUSH_DE)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterD>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterE>)
		.Cycle();
	instruction_builder.Begin(Opcode::PUSH_HL)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterH>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterL>)
		.Cycle();

	// Pop instructions
	instruction_builder.Begin(Opcode::POP_AF)
		.Cycle(&CPU::LD_r_arr<RegisterF, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>, &CPU::MASK_r<RegisterF, 0b1111'0000>)
		.Cycle(&CPU::LD_r_arr<RegisterA, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::POP_BC)
		.Cycle(&CPU::LD_r_arr<RegisterC, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterB, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::POP_DE)
		.Cycle(&CPU::LD_r_arr<RegisterE, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterD, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>);
	instruction_builder.Begin(Opcode::POP_HL)
		.Cycle(&CPU::LD_r_arr<RegisterL, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterH, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>);

	// Call instructions
	instruction_builder.Begin(Opcode::CALL_nn)
		.Cycle(&CPU::LD_r_n<RegisterY>)
		.Cycle(&CPU::LD_r_n<RegisterX>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::CALL_NZ_nn)
		.Cycle(&CPU::LD_r_n<RegisterY>)
		.Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagZero, false>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::CALL_Z_nn)
		.Cycle(&CPU::LD_r_n<RegisterY>)
		.Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagZero, true>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::CALL_NC_nn)
		.Cycle(&CPU::LD_r_n<RegisterY>)
		.Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagCarry, false>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::CALL_C_nn)
		.Cycle(&CPU::LD_r_n<RegisterY>)
		.Cycle(&CPU::LD_r_n<RegisterX>, &CPU::FlagCondition<FlagCarry, true>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP_rr<RegisterXY>);

	// Return instructions
	instruction_builder.Begin(Opcode::RET)
		.Cycle(&CPU::LD_r_arr<RegisterY, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterX, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::RET_NZ)
		.Cycle(&CPU::FlagCondition<FlagZero, false>)
		.Cycle(&CPU::LD_r_arr<RegisterY, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterX, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::RET_Z)
		.Cycle(&CPU::FlagCondition<FlagZero, true>)
		.Cycle(&CPU::LD_r_arr<RegisterY, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterX, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::RET_NC)
		.Cycle(&CPU::FlagCondition<FlagCarry, false>)
		.Cycle(&CPU::LD_r_arr<RegisterY, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterX, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::JP_rr<RegisterXY>);
	instruction_builder.Begin(Opcode::RET_C)
		.Cycle(&CPU::FlagCondition<FlagCarry, true>)
		.Cycle(&CPU::LD_r_arr<RegisterY, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterX, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::JP_rr<RegisterXY>);

	// Return instructions
	instruction_builder.Begin(Opcode::RETI)
		.Cycle(&CPU::LD_r_arr<RegisterY, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::LD_r_arr<RegisterX, RegisterSP>, &CPU::UnaryOp_rr<RegisterSP, &CPU::Increment16>)
		.Cycle(&CPU::JP_rr<RegisterXY>, &CPU::EnableInterrupts);

	// Restart instructions
	instruction_builder.Begin(Opcode::RST_00)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0000>);
	instruction_builder.Begin(Opcode::RST_08)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0008>);
	instruction_builder.Begin(Opcode::RST_10)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0010>);
	instruction_builder.Begin(Opcode::RST_18)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0018>);
	instruction_builder.Begin(Opcode::RST_20)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0020>);
	instruction_builder.Begin(Opcode::RST_28)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0028>);
	instruction_builder.Begin(Opcode::RST_30)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0030>);
	instruction_builder.Begin(Opcode::RST_38)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_P>)
		.Cycle(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>, &CPU::LD_arr_r<RegisterSP, RegisterPC_C>)
		.Cycle(&CPU::JP<0x0038>);

	// Misc instructions
	instruction_builder.Begin(Opcode::CPL_A, &CPU::UnaryOp_r<RegisterA, &CPU::Complement8>);
	instruction_builder.Begin(Opcode::DA_A, &CPU::UnaryOp_r<RegisterA, &CPU::DecimalAdjust8>);
	instruction_builder.Begin(Opcode::CCF, &CPU::CCF);
	instruction_builder.Begin(Opcode::SCF, &CPU::SCF);

	// 8-bit rotate instructions
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_B, &CPU::UnaryOp_r<RegisterB, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_C, &CPU::UnaryOp_r<RegisterC, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_D, &CPU::UnaryOp_r<RegisterD, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_E, &CPU::UnaryOp_r<RegisterE, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_H, &CPU::UnaryOp_r<RegisterH, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_L, &CPU::UnaryOp_r<RegisterL, &CPU::RotateLeft8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RLC_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::RotateLeft8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	extended_instruction_builder.Begin(ExtendedOpcode::RL_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_B, &CPU::UnaryOp_r<RegisterB, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_C, &CPU::UnaryOp_r<RegisterC, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_D, &CPU::UnaryOp_r<RegisterD, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_E, &CPU::UnaryOp_r<RegisterE, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_H, &CPU::UnaryOp_r<RegisterH, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_L, &CPU::UnaryOp_r<RegisterL, &CPU::RotateLeftThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RL_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::RotateLeftThroughCarry8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	extended_instruction_builder.Begin(ExtendedOpcode::RRC_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_B, &CPU::UnaryOp_r<RegisterB, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_C, &CPU::UnaryOp_r<RegisterC, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_D, &CPU::UnaryOp_r<RegisterD, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_E, &CPU::UnaryOp_r<RegisterE, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_H, &CPU::UnaryOp_r<RegisterH, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_L, &CPU::UnaryOp_r<RegisterL, &CPU::RotateRight8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RRC_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::RotateRight8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	extended_instruction_builder.Begin(ExtendedOpcode::RR_A, &CPU::UnaryOp_r<RegisterA, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_B, &CPU::UnaryOp_r<RegisterB, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_C, &CPU::UnaryOp_r<RegisterC, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_D, &CPU::UnaryOp_r<RegisterD, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_E, &CPU::UnaryOp_r<RegisterE, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_H, &CPU::UnaryOp_r<RegisterH, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_L, &CPU::UnaryOp_r<RegisterL, &CPU::RotateRightThroughCarry8>);
	extended_instruction_builder.Begin(ExtendedOpcode::RR_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::RotateRightThroughCarry8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 8-bit shift instructions
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_A, &CPU::UnaryOp_r<RegisterA, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_B, &CPU::UnaryOp_r<RegisterB, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_C, &CPU::UnaryOp_r<RegisterC, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_D, &CPU::UnaryOp_r<RegisterD, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_E, &CPU::UnaryOp_r<RegisterE, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_H, &CPU::UnaryOp_r<RegisterH, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_L, &CPU::UnaryOp_r<RegisterL, &CPU::ShiftLeft8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SLA_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ShiftLeft8<true>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	extended_instruction_builder.Begin(ExtendedOpcode::SRA_A, &CPU::UnaryOp_r<RegisterA, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_B, &CPU::UnaryOp_r<RegisterB, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_C, &CPU::UnaryOp_r<RegisterC, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_D, &CPU::UnaryOp_r<RegisterD, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_E, &CPU::UnaryOp_r<RegisterE, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_H, &CPU::UnaryOp_r<RegisterH, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_L, &CPU::UnaryOp_r<RegisterL, &CPU::ShiftRight8<false>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRA_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ShiftRight8<false>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	extended_instruction_builder.Begin(ExtendedOpcode::SRL_A, &CPU::UnaryOp_r<RegisterA, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_B, &CPU::UnaryOp_r<RegisterB, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_C, &CPU::UnaryOp_r<RegisterC, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_D, &CPU::UnaryOp_r<RegisterD, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_E, &CPU::UnaryOp_r<RegisterE, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_H, &CPU::UnaryOp_r<RegisterH, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_L, &CPU::UnaryOp_r<RegisterL, &CPU::ShiftRight8<true>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SRL_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ShiftRight8<true>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 8-bit swap instructions
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_A, &CPU::UnaryOp_r<RegisterA, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_B, &CPU::UnaryOp_r<RegisterB, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_C, &CPU::UnaryOp_r<RegisterC, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_D, &CPU::UnaryOp_r<RegisterD, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_E, &CPU::UnaryOp_r<RegisterE, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_H, &CPU::UnaryOp_r<RegisterH, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_L, &CPU::UnaryOp_r<RegisterL, &CPU::Swap8>);
	extended_instruction_builder.Begin(ExtendedOpcode::SWAP_aHL, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::Swap8>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 8-bit test bit instructions
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_0, &CPU::BIT_r_b<RegisterA, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_1, &CPU::BIT_r_b<RegisterA, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_2, &CPU::BIT_r_b<RegisterA, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_3, &CPU::BIT_r_b<RegisterA, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_4, &CPU::BIT_r_b<RegisterA, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_5, &CPU::BIT_r_b<RegisterA, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_6, &CPU::BIT_r_b<RegisterA, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_A_7, &CPU::BIT_r_b<RegisterA, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_0, &CPU::BIT_r_b<RegisterB, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_1, &CPU::BIT_r_b<RegisterB, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_2, &CPU::BIT_r_b<RegisterB, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_3, &CPU::BIT_r_b<RegisterB, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_4, &CPU::BIT_r_b<RegisterB, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_5, &CPU::BIT_r_b<RegisterB, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_6, &CPU::BIT_r_b<RegisterB, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_B_7, &CPU::BIT_r_b<RegisterB, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_0, &CPU::BIT_r_b<RegisterC, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_1, &CPU::BIT_r_b<RegisterC, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_2, &CPU::BIT_r_b<RegisterC, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_3, &CPU::BIT_r_b<RegisterC, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_4, &CPU::BIT_r_b<RegisterC, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_5, &CPU::BIT_r_b<RegisterC, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_6, &CPU::BIT_r_b<RegisterC, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_C_7, &CPU::BIT_r_b<RegisterC, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_0, &CPU::BIT_r_b<RegisterD, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_1, &CPU::BIT_r_b<RegisterD, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_2, &CPU::BIT_r_b<RegisterD, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_3, &CPU::BIT_r_b<RegisterD, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_4, &CPU::BIT_r_b<RegisterD, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_5, &CPU::BIT_r_b<RegisterD, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_6, &CPU::BIT_r_b<RegisterD, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_D_7, &CPU::BIT_r_b<RegisterD, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_0, &CPU::BIT_r_b<RegisterE, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_1, &CPU::BIT_r_b<RegisterE, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_2, &CPU::BIT_r_b<RegisterE, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_3, &CPU::BIT_r_b<RegisterE, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_4, &CPU::BIT_r_b<RegisterE, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_5, &CPU::BIT_r_b<RegisterE, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_6, &CPU::BIT_r_b<RegisterE, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_E_7, &CPU::BIT_r_b<RegisterE, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_0, &CPU::BIT_r_b<RegisterH, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_1, &CPU::BIT_r_b<RegisterH, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_2, &CPU::BIT_r_b<RegisterH, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_3, &CPU::BIT_r_b<RegisterH, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_4, &CPU::BIT_r_b<RegisterH, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_5, &CPU::BIT_r_b<RegisterH, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_6, &CPU::BIT_r_b<RegisterH, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_H_7, &CPU::BIT_r_b<RegisterH, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_0, &CPU::BIT_r_b<RegisterL, 0>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_1, &CPU::BIT_r_b<RegisterL, 1>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_2, &CPU::BIT_r_b<RegisterL, 2>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_3, &CPU::BIT_r_b<RegisterL, 3>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_4, &CPU::BIT_r_b<RegisterL, 4>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_5, &CPU::BIT_r_b<RegisterL, 5>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_6, &CPU::BIT_r_b<RegisterL, 6>);
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_L_7, &CPU::BIT_r_b<RegisterL, 7>);

	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_0, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 0>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_1, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 1>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_2, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 2>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_3, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 3>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_4, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 4>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_5, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 5>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_6, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 6>).Cycle();
	extended_instruction_builder.Begin(ExtendedOpcode::BIT_aHL_7, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::BIT_r_b<RegisterX, 7>).Cycle();

	// 8-bit reset bit instructions
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_0, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_1, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_2, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_3, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_4, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_5, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_6, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_A_7, &CPU::UnaryOp_r<RegisterA, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_0, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_1, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_2, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_3, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_4, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_5, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_6, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_B_7, &CPU::UnaryOp_r<RegisterB, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_0, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_1, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_2, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_3, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_4, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_5, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_6, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_C_7, &CPU::UnaryOp_r<RegisterC, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_0, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_1, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_2, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_3, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_4, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_5, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_6, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_D_7, &CPU::UnaryOp_r<RegisterD, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_0, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_1, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_2, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_3, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_4, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_5, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_6, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_E_7, &CPU::UnaryOp_r<RegisterE, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_0, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_1, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_2, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_3, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_4, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_5, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_6, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_H_7, &CPU::UnaryOp_r<RegisterH, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_0, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_1, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_2, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_3, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_4, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_5, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_6, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_L_7, &CPU::UnaryOp_r<RegisterL, &CPU::ResetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_0, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<0>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_1, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<1>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_2, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<2>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_3, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<3>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_4, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<4>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_5, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<5>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_6, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<6>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::RES_aHL_7, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::ResetBit8<7>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// 8-bit set bit instructions
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_0, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_1, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_2, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_3, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_4, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_5, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_6, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_A_7, &CPU::UnaryOp_r<RegisterA, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_0, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_1, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_2, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_3, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_4, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_5, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_6, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_B_7, &CPU::UnaryOp_r<RegisterB, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_0, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_1, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_2, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_3, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_4, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_5, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_6, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_C_7, &CPU::UnaryOp_r<RegisterC, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_0, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_1, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_2, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_3, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_4, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_5, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_6, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_D_7, &CPU::UnaryOp_r<RegisterD, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_0, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_1, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_2, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_3, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_4, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_5, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_6, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_E_7, &CPU::UnaryOp_r<RegisterE, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_0, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_1, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_2, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_3, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_4, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_5, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_6, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_H_7, &CPU::UnaryOp_r<RegisterH, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_0, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<0>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_1, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<1>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_2, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<2>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_3, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<3>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_4, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<4>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_5, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<5>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_6, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<6>>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_L_7, &CPU::UnaryOp_r<RegisterL, &CPU::SetBit8<7>>);

	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_0, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<0>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_1, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<1>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_2, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<2>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_3, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<3>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_4, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<4>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_5, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<5>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_6, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<6>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);
	extended_instruction_builder.Begin(ExtendedOpcode::SET_aHL_7, &CPU::LD_r_arr<RegisterX, RegisterHL>).Cycle(&CPU::UnaryOp_r<RegisterX, &CPU::SetBit8<7>>).Cycle(&CPU::LD_arr_r<RegisterHL, RegisterX>);

	// Build instruction sets
	m_Instructions = instruction_builder.Build();
	m_ExtendedInstructions = extended_instruction_builder.Build();

	// Reset CPU
	Reset();
}

Memory16& CPU::GetMemory() const noexcept
{
	return m_Memory;
}

bool CPU::LoadFlag(uint8_t a_Flag) const noexcept
{
	const uint8_t f = LoadRegister8(RegisterF);
	const uint8_t mask = (1 << a_Flag);
	return  (f & mask) != 0;
}

void CPU::StoreFlag(uint8_t a_Flag, bool a_Value) noexcept
{
	const uint8_t f = LoadRegister8(RegisterF);
	const uint8_t mask = (1 << a_Flag);

	if (a_Value)
	{
		StoreRegister8(RegisterF, f | mask);
	}
	else
	{
		StoreRegister8(RegisterF, f & (~mask));
	}
}

uint8_t CPU::GetInterruptEnable() const noexcept
{
	return m_InterruptEnable;
}

uint8_t CPU::GetInterruptRequests() const noexcept
{
	return m_InterruptRequests;
}

void CPU::SetInterruptEnable(uint8_t a_Interrupts) noexcept
{
	if (m_InterruptEnable != a_Interrupts)
	{
		m_InterruptEnable = a_Interrupts;
		InsertOp(&CPU::ProcessInterrupts, m_OpDone);
	}
}

void CPU::SetInterruptRequests(uint8_t a_Interrupts) noexcept
{
	if (m_InterruptRequests != a_Interrupts)
	{
		m_InterruptRequests = a_Interrupts;
		InsertOp(&CPU::ProcessInterrupts, m_OpDone);
	}
}

void CPU::RequestInterrupts(uint8_t a_Interrupts) noexcept
{
	SetInterruptRequests(GetInterruptRequests() | a_Interrupts);
}

void CPU::StartDMA(uint8_t a_Address)
{
	m_DMAAddress = a_Address;
	m_DMACounter = 0;
	PushOp(&CPU::DMA);
}

bool CPU::HasBreakpoint(uint16_t a_Address) const
{
	return m_Memory.GetReplaced8(a_Address).has_value();
}

void CPU::SetBreakpoint(uint16_t a_Address, bool a_Enabled)
{
	if (a_Enabled)
	{
		m_Memory.Replace8(a_Address, Opcode::BREAKPOINT_STOP);
	}
	else
	{
		m_Memory.Restore8(a_Address);
	}
}

void CPU::SetBreakpointCallback(std::function<void()>&& a_Callback)
{
	m_BreakpointCallback = std::move(a_Callback);
}

bool CPU::Tick()
{
	m_OpBreak = false;
	while (!m_OpBreak)
	{
		const MicroOp op = PopOp();
		CallOp(op);
	}

	++g_Counter;
	return m_OpFront == m_OpDone;
}

void CPU::Reset()
{
	m_OpBack = m_OpFront;

	#ifdef _DEBUG
	std::memset(m_MicroOps, 0, sizeof(m_MicroOps));
	#endif

	PushOp(&CPU::DecodeInstruction);
	StoreRegister16(RegisterPC, 0);
}

uint8_t CPU::PeekNext8() const noexcept
{
	return m_Memory.Load8(LoadRegister16(RegisterPC));
}

uint16_t CPU::PeekNext16() const noexcept
{
	return m_Memory.Load16(LoadRegister16(RegisterPC));
}

uint8_t CPU::ReadNext8() noexcept
{
	const uint8_t byte = PeekNext8();
	StoreRegister16(RegisterPC, LoadRegister16(RegisterPC) + 1);
	return byte;
}

uint16_t CPU::ReadNext16() noexcept
{
	const uint16_t word = PeekNext16();
	StoreRegister16(RegisterPC, LoadRegister16(RegisterPC) + 2);
	return word;
}

void CPU::IncrementOpCounter(size_t& a_Counter)
{
	++a_Counter;
	a_Counter %= std::size(m_MicroOps);
}

void CPU::PushOp(MicroOp a_Op)
{
	#ifdef _DEBUG
	assert(m_MicroOps[m_OpBack] == nullptr);
	#endif

	m_MicroOps[m_OpBack] = a_Op;
	IncrementOpCounter(m_OpBack);
}

CPU::MicroOp CPU::PopOp()
{
	const MicroOp op = m_MicroOps[m_OpFront];

	#ifdef _DEBUG
	assert(m_MicroOps[m_OpFront] != nullptr);
	m_MicroOps[m_OpFront] = nullptr;
	#endif

	IncrementOpCounter(m_OpFront);

	return op;
}

void CPU::InsertOp(MicroOp a_Op, size_t a_Index)
{
	for (size_t i = m_OpBack; i != a_Index; i = (i - 1) % std::size(m_MicroOps))
	{
		const size_t prev = (i - 1) % std::size(m_MicroOps);

		m_MicroOps[i] = m_MicroOps[prev];
	}

	m_MicroOps[a_Index] = a_Op;
	IncrementOpCounter(m_OpBack);
}

void CPU::InsertOpAndIncrementDone(MicroOp a_Op)
{
	InsertOp(a_Op, m_OpDone);
	IncrementOpCounter(m_OpDone);
}

void CPU::CallOp(MicroOp a_Op)
{
	(this->*a_Op)();
}

template <bool Carry>
uint8_t CPU::Add8(uint8_t a_Left, uint8_t a_Right) noexcept
{
	uint8_t carry = 0;
	if constexpr (Carry)
	{
		if (LoadFlag(FlagCarry))
		{
			carry = 1;
		}
	}
	uint16_t result = static_cast<uint16_t>(a_Left) + static_cast<uint16_t>(a_Right) + static_cast<uint16_t>(carry);

	StoreFlag(FlagZero, (result & 0xFF) == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, (((a_Left & 0x0F) + (a_Right & 0x0F) + carry) & 0xF0) != 0);
	StoreFlag(FlagCarry, (result & 0xFF00) != 0);

	return static_cast<uint8_t>(result);
}

template <bool Carry>
uint8_t CPU::Subtract8(uint8_t a_Left, uint8_t a_Right) noexcept
{
	uint8_t carry = 0;
	if constexpr (Carry)
	{
		if (LoadFlag(FlagCarry))
		{
			carry = 1;
		}
	}
	uint16_t result = (static_cast<uint16_t>(a_Left) - static_cast<uint16_t>(a_Right)) - static_cast<uint16_t>(carry);

	StoreFlag(FlagZero, (result & 0xFF) == 0);
	StoreFlag(FlagSubtract, true);
	StoreFlag(FlagHalfCarry, ((a_Left ^ a_Right ^ (result & 0xFF)) & 0b1000) != 0);
	StoreFlag(FlagCarry, (result & 0xFF00) != 0);

	return static_cast<uint8_t>(result);
}

uint16_t CPU::Add16(uint16_t a_Left, uint16_t a_Right) noexcept
{
	const uint32_t result = static_cast<uint32_t>(a_Left) + static_cast<uint32_t>(a_Right);

	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, (((a_Left & 0xFFF) + (a_Right & 0xFFF)) & 0xF000) != 0);
	StoreFlag(FlagCarry, (result & 0xFFFF0000) != 0);

	return static_cast<uint16_t>(result);
}

uint8_t CPU::Increment8(uint8_t a_Value) noexcept
{
	StoreFlag(FlagZero, a_Value == 0xFF);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, (a_Value & 0xF) == 0xF);
	// Carry flag is not affected by increments

	return ++a_Value;
}

uint8_t CPU::Decrement8(uint8_t a_Value) noexcept
{
	StoreFlag(FlagZero, a_Value == 0x01);
	StoreFlag(FlagSubtract, true);
	StoreFlag(FlagHalfCarry, (a_Value & 0xF) == 0);
	// Carry flag is not affected by decrements

	return --a_Value;
}

uint16_t CPU::Increment16(uint16_t a_Value) noexcept
{
	return ++a_Value;
}

uint16_t CPU::Decrement16(uint16_t a_Value) noexcept
{
	return --a_Value;
}

uint8_t CPU::AND8(uint8_t a_Left, uint8_t a_Right) noexcept
{
	const uint8_t result = (a_Left & a_Right);

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, true);
	StoreFlag(FlagCarry, false);

	return result;
}

uint8_t CPU::OR8(uint8_t a_Left, uint8_t a_Right) noexcept
{
	const uint8_t result = (a_Left | a_Right);

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, false);

	return result;
}

uint8_t CPU::XOR8(uint8_t a_Left, uint8_t a_Right) noexcept
{
	const uint8_t result = (a_Left ^ a_Right);

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, false);

	return result;
}

uint8_t CPU::Swap8(uint8_t a_Value) noexcept
{
	const uint8_t result = ((a_Value & 0x0F) << 4) | ((a_Value & 0xF0) >> 4);

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, false);

	return result;
}

uint8_t CPU::Complement8(uint8_t a_Value) noexcept
{
	const uint8_t result = ~a_Value;

	StoreFlag(FlagSubtract, true);
	StoreFlag(FlagHalfCarry, true);

	return result;
}

template <bool ResetZero>
uint8_t CPU::RotateLeft8(uint8_t a_Value) noexcept
{
	const uint8_t result = (a_Value << 1) | (a_Value >> 7);

	StoreFlag(FlagZero, result == 0 && !ResetZero);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, (a_Value & 0x80) != 0);

	return result;
}

template <bool ResetZero>
uint8_t CPU::RotateLeftThroughCarry8(uint8_t a_Value) noexcept
{
	const uint8_t result = (a_Value << 1) | (LoadFlag(FlagCarry) ? 0x01 : 0x00);

	StoreFlag(FlagZero, result == 0 && !ResetZero);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, (a_Value & 0x80) != 0);

	return result;
}

template <bool ResetZero>
uint8_t CPU::RotateRight8(uint8_t a_Value) noexcept
{
	const uint8_t result = (a_Value >> 1) | (a_Value << 7);

	StoreFlag(FlagZero, result == 0 && !ResetZero);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, (a_Value & 0x01) != 0);

	return result;
}

template <bool ResetZero>
uint8_t CPU::RotateRightThroughCarry8(uint8_t a_Value) noexcept
{
	const uint8_t result = (a_Value >> 1) | (LoadFlag(FlagCarry) ? 0x80 : 0x00);

	StoreFlag(FlagZero, result == 0 && !ResetZero);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, (a_Value & 0x01) != 0);

	return result;
}

template <bool FillWithZero>
uint8_t CPU::ShiftLeft8(uint8_t a_Value)
{
	uint8_t result = a_Value << 1;
	if constexpr (!FillWithZero)
	{
		result |= a_Value & 0b0000'0001;
	}

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, (a_Value & 0b1000'0000) != 0);

	return result;
}

template <bool FillWithZero>
uint8_t CPU::ShiftRight8(uint8_t a_Value)
{
	uint8_t result = a_Value >> 1;
	if constexpr (!FillWithZero)
	{
		result |= a_Value & 0b1000'0000;
	}

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, (a_Value & 0b0000'0001) != 0);

	return result;
}

template <uint8_t Bit>
uint8_t CPU::SetBit8(uint8_t a_Value)
{
	const uint8_t mask = 1 << Bit;
	const uint8_t result = a_Value | mask;
	return result;
}

template <uint8_t Bit>
uint8_t CPU::ResetBit8(uint8_t a_Value)
{
	const uint8_t mask = ~(1 << Bit);
	const uint8_t result = a_Value & mask;
	return result;
}

template <bool Flags>
uint16_t CPU::SignedAdd16(uint16_t a_Left, uint8_t a_Right) noexcept
{
	uint16_t result;
	if ((a_Right & 0x80) == 0)
	{
		result = a_Left + a_Right;
	}
	else
	{
		result = a_Left - (static_cast<uint16_t>(static_cast<uint8_t>(~a_Right)) + 1);
	}

	if constexpr (Flags)
	{
		StoreFlag(FlagZero, false);
		StoreFlag(FlagSubtract, false);
		StoreFlag(FlagHalfCarry, (((a_Left & 0x0f) + (a_Right & 0x0f)) & 0x10) != 0);
		StoreFlag(FlagCarry, (((a_Left & 0xff) + (a_Right & 0xff)) & 0x100) != 0);
	}

	return result;
}

uint8_t CPU::DecimalAdjust8(uint8_t a_Value) noexcept
{
	uint8_t result = a_Value;

	if (LoadFlag(FlagSubtract))
	{
		if (LoadFlag(FlagCarry))
		{
			result -= 0x60;
		}
		if (LoadFlag(FlagHalfCarry))
		{
			result -= 0x06;
		}
	}
	else
	{
		if (LoadFlag(FlagCarry) || result > 0x99)
		{
			result += 0x60;
			StoreFlag(FlagCarry, true);
		}
		if (LoadFlag(FlagHalfCarry) || (result & 0x0F) > 0x09)
		{
			result += 0x06;
		}
	}

	StoreFlag(FlagZero, result == 0);
	StoreFlag(FlagHalfCarry, false);

	return result;
}

template <uint8_t Destination, CPU::UnaryOp8 Op, bool Store>
void CPU::UnaryOp_r() noexcept
{
	const uint8_t destination_value = LoadRegister8(Destination);
	const uint8_t result = (this->*Op)(destination_value);
	if constexpr (Store)
	{
		StoreRegister8(Destination, result);
	}
}

template <uint8_t Destination, CPU::UnaryOp16 Op, bool Store>
void CPU::UnaryOp_rr() noexcept
{
	const uint16_t destination_value = LoadRegister16(Destination);
	const uint16_t result = (this->*Op)(destination_value);
	if constexpr (Store)
	{
		StoreRegister16(Destination, result);
	}
}

template <uint8_t Destination, CPU::BinaryOp8 Op, bool Store>
void CPU::BinaryOp_r_x(uint8_t a_Value) noexcept
{
	const uint8_t destination_value = LoadRegister8(Destination);
	const uint8_t result = (this->*Op)(destination_value, a_Value);
	if constexpr (Store)
	{
		StoreRegister8(Destination, result);
	}
}

template <uint8_t Destination, uint8_t Source, CPU::BinaryOp8 Op, bool Store>
void CPU::BinaryOp_r_r() noexcept
{
	const uint8_t value = LoadRegister8(Source);
	BinaryOp_r_x<Destination, Op, Store>(value);
}

template <uint8_t Destination, CPU::BinaryOp16 Op, bool Store>
void CPU::BinaryOp_rr_xx(uint16_t a_Value) noexcept
{
	const uint16_t destination_value = LoadRegister16(Destination);
	const uint16_t result = (this->*Op)(destination_value, a_Value);
	if constexpr (Store)
	{
		StoreRegister16(Destination, result);
	}
}

template <uint8_t Destination, uint8_t Source, CPU::BinaryOp16 Op, bool Store>
void CPU::BinaryOp_rr_rr() noexcept
{
	const uint16_t value = LoadRegister16(Source);
	BinaryOp_rr_xx<Destination, Op, Store>(value);
}

template <CPU::MicroOp Op, uint8_t Counter>
void CPU::Delay()
{
	if constexpr (Counter == 0)
	{
		InsertOp(Op, m_OpDone);
	}
	else
	{
		PushOp(&CPU::Delay<Op, Counter - 1>);
	}
}

void CPU::NotImplemented()
{
	const uint16_t pc = LoadRegister16(RegisterPC);
	if (m_Memory.Load8(pc - 2) == Opcode::EXT)
	{
		StoreRegister16(RegisterPC, pc - 2_u16);
	}
	else
	{
		StoreRegister16(RegisterPC, pc - 1_u16);
	}

	if (m_BreakpointCallback)
	{
		m_BreakpointCallback();
	}
}

void CPU::DecodeInstruction()
{
	// Debug tracing
	{
		static bool bootrom = true;
		if (LoadRegister16(RegisterPC) == 0x100)
		{
			g_Counter = 0;
			//bootrom = false;
		}

		if (!bootrom)
		{
			const auto opcode = static_cast<Opcode::Enum>(PeekNext8());

			std::cout << "A:" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(LoadRegister8(RegisterA));
			std::cout << " F:" << (LoadFlag(FlagZero) ? 'Z' : '-') << (LoadFlag(FlagSubtract) ? 'N' : '-') << (LoadFlag(FlagHalfCarry) ? 'H' : '-') << (LoadFlag(FlagCarry) ? 'C' : '-');
			std::cout << " BC:" << std::hex << std::setfill('0') << std::setw(4) << LoadRegister16(RegisterBC);
			std::cout << " DE:" << std::hex << std::setfill('0') << std::setw(4) << LoadRegister16(RegisterDE);
			std::cout << " HL:" << std::hex << std::setfill('0') << std::setw(4) << LoadRegister16(RegisterHL);
			std::cout << " SP:" << std::hex << std::setfill('0') << std::setw(4) << LoadRegister16(RegisterSP);
			std::cout << " PC:" << std::hex << std::setfill('0') << std::setw(4) << LoadRegister16(RegisterPC);
			std::cout << " (cy: " << std::dec << g_Counter * 4 << ')';
			//std::cout << " LY: " << std::dec << static_cast<int>(m_Memory.Load8(0xFF44));
			std::cout << " |[00]0x" << std::hex << std::setfill('0') << std::setw(4) << LoadRegister16(RegisterPC) << ":";

			for (uint16_t i = 0; i < *Opcode::GetSize(opcode); ++i)
			{
				std::cout << ' ' << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(m_Memory.Load8(LoadRegister16(RegisterPC) + i));
			}

			std::cout << std::endl;
		}
	}

	// Decode instruction
	const auto opcode = static_cast<Opcode::Enum>(ReadNext8());
	const size_t instruction_size = m_Instructions->GetInstructionSize(opcode);
	const MicroOp* const ops = m_Instructions->GetInstructionOps(opcode);

	for (size_t i = 0; i < instruction_size; ++i)
	{
		PushOp(ops[i]);
	}

	m_OpDone = m_OpBack;

	PushOp(&CPU::DecodeInstruction);
}

void CPU::DecodeExtendedInstruction()
{
	const auto opcode = static_cast<ExtendedOpcode::Enum>(ReadNext8());
	const size_t instruction_size = m_ExtendedInstructions->GetInstructionSize(opcode);
	const MicroOp* const ops = m_ExtendedInstructions->GetInstructionOps(opcode);

	for (size_t i = 0; i < instruction_size; ++i)
	{
		InsertOpAndIncrementDone(ops[i]);
	}
}

void CPU::BreakpointStop()
{
	// Check if it is an actual breakpoint or just an invalid opcode
	const uint16_t instruction_address = LoadRegister16(RegisterPC) - 1_u16;
	const auto replaced_byte = m_Memory.GetReplaced8(instruction_address);
	if (!replaced_byte)
	{
		NotImplemented();
		return;
	}

	// Replace breakpoint instruction
	m_Memory.Replace8(instruction_address, Opcode::BREAKPOINT_CONTINUE);

	// Restore program counter
	StoreRegister16(RegisterPC, instruction_address);

	// Call breakpoint callback
	if (m_BreakpointCallback)
	{
		m_BreakpointCallback();
	}
}

void CPU::BreakpointContinue()
{
	// Check if it is an actual breakpoint or just an invalid opcode
	const uint16_t instruction_address = LoadRegister16(RegisterPC) - 1_u16;
	const auto replaced_byte = m_Memory.GetReplaced8(instruction_address);
	if (!replaced_byte)
	{
		NotImplemented();
		return;
	}

	// Restore breakpoint instruction
	m_Memory.Replace8(instruction_address, Opcode::BREAKPOINT_STOP);

	// Insert the real instruction
	const auto opcode = static_cast<Opcode::Enum>(*replaced_byte);
	const size_t instruction_size = m_Instructions->GetInstructionSize(opcode);
	const MicroOp* const ops = m_Instructions->GetInstructionOps(opcode);

	for (size_t i = 0; i < instruction_size; ++i)
	{
		InsertOpAndIncrementDone(ops[i]);
	}
}

void CPU::Break()
{
	m_OpBreak = true;
}

void CPU::Skip()
{
	while (m_OpFront != m_OpDone)
	{
		PopOp();
	}

	Break();
}

template <uint8_t Flag, bool Set>
void CPU::FlagCondition()
{
	if (LoadFlag(Flag) != Set)
	{
		Skip();
	}
}

void CPU::DisableInterrupts()
{
	m_InterruptMasterEnable = false;
}

void CPU::EnableInterrupts()
{
	m_InterruptMasterEnable = true;
	ProcessInterrupts();
}

void CPU::ProcessInterrupts()
{
	if (!m_InterruptMasterEnable)
	{
		return;
	}

	for (uint8_t i = 0; i < 5; ++i)
	{
		const uint8_t interrupt_mask = 1 << i;
		if ((m_InterruptEnable & interrupt_mask) == 0)
		{
			continue;
		}

		if ((m_InterruptRequests & interrupt_mask) == 0)
		{
			continue;
		}

		// Resume if halted
		m_Halted = false;

		// Reset interrupt flags (TODO: when does this happen exactly?)
		m_InterruptMasterEnable = false;
		m_InterruptRequests ^= interrupt_mask;

		// Set done to front
		m_OpDone = m_OpFront;

		// Wait 2 cycles
		InsertOpAndIncrementDone(&CPU::Break);
		InsertOpAndIncrementDone(&CPU::Break);

		// Push PC on the stack
		InsertOpAndIncrementDone(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>);
		InsertOpAndIncrementDone(&CPU::LD_arr_r<RegisterSP, RegisterPC_P>);
		InsertOpAndIncrementDone(&CPU::Break);

		InsertOpAndIncrementDone(&CPU::UnaryOp_rr<RegisterSP, &CPU::Decrement16>);
		InsertOpAndIncrementDone(&CPU::LD_arr_r<RegisterSP, RegisterPC_C>);
		InsertOpAndIncrementDone(&CPU::Break);

		// Jump to interrupt handler
		StoreRegister16(RegisterXY, 0x40 + i * 8);
		InsertOpAndIncrementDone(&CPU::JP_rr<RegisterXY>);
		InsertOpAndIncrementDone(&CPU::Break);
	}
}

void CPU::Halt()
{
	m_Halted = true;
	CheckHalt();
}

void CPU::CheckHalt()
{
	IncrementOpCounter(m_OpDone);
	if (m_Halted)
	{
		InsertOp(&CPU::CheckHalt, m_OpFront);
		Break();
	}
	else
	{
		bool boop = true;
	}
}

template <uint8_t Destination, uint8_t Mask>
void CPU::MASK_r()
{
	StoreRegister8(Destination, LoadRegister8(Destination) & Mask);
}

void CPU::DMA()
{
	const uint16_t source_address = (static_cast<uint16_t>(m_DMAAddress) << 8) | m_DMACounter;
	const uint16_t destination_address = 0xFE00 | m_DMACounter;

	m_Memory.Store8(destination_address, m_Memory.Load8(source_address));

	++m_DMACounter;
	if (m_DMACounter != 0xA0)
	{
		PushOp(&CPU::DMA);
	}
}

template <uint8_t Destination>
void CPU::LD_r_x(uint8_t a_Value)
{
	StoreRegister8(Destination, a_Value);
}

template <uint8_t Destination>
void CPU::LD_r_n()
{
	const uint8_t value = ReadNext8();
	LD_r_x<Destination>(value);
}

template <uint8_t Destination, uint8_t Source>
void CPU::LD_r_r()
{
	const uint8_t value = LoadRegister8(Source);
	LD_r_x<Destination>(value);
}

template <uint8_t Destination>
void CPU::LD_r_axx(uint16_t a_Address)
{
	const uint8_t value = m_Memory.Load8(a_Address);
	LD_r_x<Destination>(value);
}

template <uint8_t Destination, uint8_t Source>
void CPU::LD_r_arr()
{
	const uint16_t address = LoadRegister16(Source);
	LD_r_axx<Destination>(address);
}

void CPU::LD_axx_x(uint16_t a_Address, uint8_t a_Value)
{
	m_Memory.Store8(a_Address, a_Value);
}

template <uint8_t Destination>
void CPU::LD_arr_x(uint8_t a_Value)
{
	const uint16_t address = LoadRegister16(Destination);
	LD_axx_x(address, a_Value);
}

template <uint8_t Destination, uint8_t Source>
void CPU::LD_arr_r()
{
	const uint8_t value = LoadRegister8(Source);
	LD_arr_x<Destination>(value);
}

template <uint8_t Destination>
void CPU::LD_rr_xx(uint16_t a_Value)
{
	StoreRegister16(Destination, a_Value);
}

template <uint8_t Destination, uint8_t Source>
void CPU::LD_rr_rr()
{
	const uint16_t value = LoadRegister16(Source);
	LD_rr_xx<Destination>(value);
}

template <uint8_t Destination, uint8_t Source>
void CPU::LD_rr_xxr(uint16_t a_Base)
{
	const uint8_t offset = LoadRegister8(Source);
	const uint16_t value = a_Base | offset;
	LD_rr_xx<Destination>(value);
}

template <uint8_t Destination, uint8_t Source>
void CPU::LD_rr_FFr()
{
	const uint16_t base = 0xFF00;
	LD_rr_xxr<Destination, Source>(base);
}

template <uint8_t Destination, uint8_t BaseSource, uint8_t OffsetSource>
void CPU::LD_rr_rrr()
{
	const uint16_t base = LoadRegister16(BaseSource);
	const uint8_t offset = LoadRegister8(OffsetSource);
	const uint16_t result = SignedAdd16(base, offset);
	LD_rr_xx<Destination>(result);
}

template <uint8_t Destination, uint8_t Source>
void CPU::ADD_rr_r() noexcept
{
	const uint16_t base = LoadRegister16(Destination);
	const uint8_t offset = LoadRegister8(Source);
	const uint16_t value = SignedAdd16(base, offset);
	LD_rr_xx<Destination>(value);
}

void CPU::JP_xx(uint16_t a_Address) noexcept
{
	StoreRegister16(RegisterPC, a_Address);
}

template <uint8_t Source>
void CPU::JP_rr() noexcept
{
	const uint16_t address = LoadRegister16(Source);
	JP_xx(address);
}

template <uint16_t Address>
void CPU::JP() noexcept
{
	JP_xx(Address);
}

template <uint8_t Base, uint8_t Offset>
void CPU::JR_rr_r() noexcept
{
	const uint16_t base = LoadRegister16(Base);
	const uint8_t offset = LoadRegister8(Offset);
	const uint16_t address = SignedAdd16<false>(base, offset);

	JP_xx(address);
}

template <uint8_t Bit>
void CPU::BIT_x_b(uint8_t a_Value) noexcept
{
	StoreFlag(FlagZero, (a_Value & (1 << Bit)) == 0);
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, true);
}

template <uint8_t Source, uint8_t Bit>
void CPU::BIT_r_b() noexcept
{
	const uint8_t value = LoadRegister8(Source);
	BIT_x_b<Bit>(value);
}

void CPU::CCF() noexcept
{
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, !LoadFlag(FlagCarry));
}

void CPU::SCF() noexcept
{
	StoreFlag(FlagSubtract, false);
	StoreFlag(FlagHalfCarry, false);
	StoreFlag(FlagCarry, true);
}