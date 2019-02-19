/// \file Swt.cxx
/// \brief Implementation of ALICE Lowlevel Frontend (ALF) SWT operations
///
/// \author Milan Tkacik (milan.tkacik@cern.ch)
#include "Swt.h"
#include "AlfException.h"
#include <unistd.h>
#include <chrono>

namespace AliceO2 {
namespace roc {
namespace CommandLineUtilities {
namespace Alf {

namespace Registers {
	constexpr int SWT_BASE_ADDR = 0x0F00000;
	constexpr int SWT_WR_L = 0x40;
	constexpr int SWT_WR_M = 0x44;
	constexpr int SWT_WR_H = 0x48;
	constexpr int SWT_RD_L = 0x50;
	constexpr int SWT_RD_M = 0x54;
	constexpr int SWT_RD_H = 0x58;
	constexpr int SWT_RD_MON = 0x5C;
	constexpr int SWT_WORD_WR = 0x4C;
	constexpr int SWT_WORD_RD = 0x4C;
	constexpr int SWT_CHANNEL = 0x60;
	constexpr int SWT_RESET = 0x64;
}

Swt::Swt(RegisterReadWriteInterface& bar2, int gbtChannel): mBar2(bar2)
{
	this->gbtChannel = gbtChannel;
    barWrite(Registers::SWT_CHANNEL, gbtChannel);
    reset();
}

uint32_t Swt::write(const uint32_t data[3])
{
	barWrite(Registers::SWT_WR_L, data[0]);
	barWrite(Registers::SWT_WR_M, data[1]);
	barWrite(Registers::SWT_WR_H, data[2]);

	barWrite(Registers::SWT_WORD_WR, 0x1);
	barWrite(Registers::SWT_WORD_WR, 0x0);

	return barRead(Registers::SWT_RD_MON);
}

uint32_t Swt::write(const SwtData data)
{
    return write(data.data);
}

uint32_t Swt::read(uint32_t data[3])
{
	uint32_t mon;

	auto endTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(10);
	do
	{
		mon = barRead(Registers::SWT_RD_MON);
		if ((mon >> 16) > 1)
		{
			printf("Mon register: 0x%X\n", mon);
            BOOST_THROW_EXCEPTION(SwtException() << ErrorInfo::Message("Error reading SWT registers!"));
		}

		if (std::chrono::steady_clock::now() >= endTime)
		{			
			printf("Mon register: 0x%X\n", mon);
            BOOST_THROW_EXCEPTION(SwtException() << ErrorInfo::Message("Exceeded timeout on busy wait!"));
		}
	}
	while ((mon >> 16) == 0);

	barWrite(Registers::SWT_WORD_RD, 0x2);
	barWrite(Registers::SWT_WORD_RD, 0x0);

	data[0] = barRead(Registers::SWT_RD_L);
	data[1] = barRead(Registers::SWT_RD_M);
	data[2] = barRead(Registers::SWT_RD_H);

	return barRead(Registers::SWT_RD_MON);
}

Swt::SwtData Swt::read()
{
    SwtData data;
    read(data.data);
    return data;
}

void Swt::reset()
{
	barWrite(Registers::SWT_RESET, 0x1);
	barWrite(Registers::SWT_RESET, 0x0);
}

void Swt::barWrite(int index, uint32_t data)
{
  mBar2.writeRegister((index + Registers::SWT_BASE_ADDR) / 4, data);
}

uint32_t Swt::barRead(int index)
{
  return mBar2.readRegister((index + Registers::SWT_BASE_ADDR) / 4);
}

} // namespace Alf
} // namespace CommandLineUtilities
} // namespace roc
} // namespace AliceO2

