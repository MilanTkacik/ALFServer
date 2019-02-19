/// \file Sca.h
/// \brief Implementation of ALICE Lowlevel Frontend (ALF) SCA operations
///
/// \author Pascal Boeschoten (pascal.boeschoten@cern.ch)

#include "Sca.h"
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include "AlfException.h"
#include "Register.h"
#include "Utilities/Util.h"

// TODO Sort out magic numbers

namespace AliceO2 {
namespace roc {
namespace CommandLineUtilities {
namespace Alf {

#define CRU_BASE_INDEX Sca::registers["'add_gbt_sc'"]
#define CRU_RESET Sca::registers["'add_gbt_sc_rst'"]
#define CRU_LINK Sca::registers["'add_gbt_sc_link'"]
#define WRITE_DATA Sca::registers["'add_gbt_sca_wr_data'"]
#define WRITE_CONTROL Sca::registers["'add_gbt_sca_wr_ctr'"]
#define WRITE_COMMAND Sca::registers["'add_gbt_sca_wr_cmd'"]
#define READ_MONITOR Sca::registers["'add_gbt_sca_rd_mon'"]
#define READ_DATA Sca::registers["'add_gbt_sca_rd_data'"]
#define READ_CONTROL Sca::registers["'add_gbt_sca_rd_ctr'"]
#define READ_COMMAND Sca::registers["'add_gbt_sca_rd_cmd'"]

constexpr auto BUSY_TIMEOUT = std::chrono::milliseconds(10);
constexpr auto CHANNEL_BUSY_TIMEOUT = std::chrono::milliseconds(10);
constexpr int CRU_MAX_LINKS = 7;

std::map<std::string, uint32_t> Sca::registers;

Sca::Sca(RegisterReadWriteInterface &bar2, CardType::type cardType, int link) : mBar2(bar2)
{
    loadRegisters();

    if (cardType == CardType::Crorc)
    {
        BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("CRORC card not supported"));
    }

    if (link >= CRU_MAX_LINKS)
    {
        BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("Maximum link number exceeded"));
    }

    barWrite(CRU_LINK, link);
    barWrite(CRU_RESET, 0x1);
    barWrite(CRU_RESET, 0x0);
}

void Sca::initialize()
{
  init();
  gpioEnable();
}

void Sca::init()
{
  barWrite(WRITE_CONTROL, 0x1);
  waitOnBusyClear();
  barWrite(WRITE_CONTROL, 0x2);
  waitOnBusyClear();
  //barWrite(WRITE_CONTROL, 0x1);
  //waitOnBusyClear();
  barWrite(WRITE_CONTROL, 0x0);
}

void Sca::write(uint32_t command, uint32_t data)
{
  barWrite(WRITE_DATA, data);
  barWrite(WRITE_COMMAND, command);
  auto transactionId = (command >> 16) & 0xff;
  if (transactionId == 0x0 || transactionId == 0xff) {
    BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("Invalid transaction ID"));
  }
  executeCommand();
}

auto Sca::read() -> ReadResult
{
  auto data = barRead(READ_DATA);
  auto command = barRead(READ_COMMAND);
//  printf("Sca::read   DATA=0x%x   CH=0x%x   TR=0x%x   CMD=0x%x\n", data, command >> 24, (command >> 16) & 0xff, command & 0xff);

  auto endTime = std::chrono::steady_clock::now() + CHANNEL_BUSY_TIMEOUT;
  while (std::chrono::steady_clock::now() < endTime){
    if (!isChannelBusy(barRead(READ_COMMAND))) {
      checkError(command);
      return { command, data };
    }
  }
  BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("Exceeded timeout on channel busy wait"));
}

bool Sca::isChannelBusy(uint32_t command)
{
  return (command & 0xff) == 0x40;
}

void Sca::checkError(uint32_t command)
{
  uint32_t errorCode = command & 0xff;

  auto toString = [&](int flag){
    switch (flag) {
      case 1:
        return "invalid channel request";
      case 2:
        return "invalid command request";
      case 3:
        return "invalid transaction number";
      case 4:
        return "invalid length";
      case 5:
        return "channel not enabled";
      case 6:
        return "channel busy";
      case 7:
        return "channel busy";
      case 0:
      default:
        return "generic error flag";
    }
  };

  // Check which error bits are enabled
  std::vector<int> flags;
  for (int flag = 0; flag < 7; ++flag) {
    if (Utilities::getBit(errorCode, flag) == 1) {
      flags.push_back(flag);
    }
  }

  // Turn into an error message
  if (!flags.empty()) {
    std::stringstream stream;
    stream << "error code 0x" << errorCode << ": ";
    for (size_t i = 0; i < flags.size(); ++i) {
      stream << toString(flags[i]);
      if (i < flags.size()) {
        stream << ", ";
      }
    }
    BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message(stream.str()));
  }
}

void Sca::gpioEnable()
{
  // Enable GPIO
  // WR CONTROL REG B
  write(0x00010002, 0xff000000);
  read();
  // RD CONTROL REG B
  write(0x00020003, 0xff000000);
  read();

  // WR GPIO DIR
  write(0x02030020, 0xffffffff);
  // RD GPIO DIR
  write(0x02040021, 0x0);
  read();
}

auto Sca::gpioWrite(uint32_t data) -> ReadResult
{
//  printf("Sca::gpioWrite DATA=0x%x\n", data);
  initialize();
  // WR REGISTER OUT DATA
  write(0x02040010, data);
  // RD DATA
  write(0x02050011, 0x0);
  read();
  // RD REGISTER DATAIN
  write(0x02060001, 0x0);
  return read();
}

auto Sca::gpioRead() -> ReadResult
{
//  printf("Sca::gpioRead\n", data);
  // RD DATA
  write(0x02050011, 0x0);
  return read();
}


void Sca::barWrite(uint32_t index, uint32_t data)
{
  mBar2.writeRegister(index + CRU_BASE_INDEX, data);
}

uint32_t Sca::barRead(uint32_t index)
{
  return mBar2.readRegister(index + CRU_BASE_INDEX);
}

void Sca::executeCommand()
{
  barWrite(WRITE_CONTROL, 0x4);
  barWrite(WRITE_CONTROL, 0x0);
  waitOnBusyClear();
}

void Sca::waitOnBusyClear()
{
  auto endTime = std::chrono::steady_clock::now() + BUSY_TIMEOUT;
  while (std::chrono::steady_clock::now() < endTime){
    if ((((barRead(READ_CONTROL)) >> 31) & 0x1) == 0) {
      return;
    }
  }
  BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("Exceeded timeout on busy wait"));
}

void Sca::loadRegisters()
{
    if (Sca::registers.size() != 10)
    {
        std::ifstream iFile(getenv(std::string("CRU_TABLE_PATH").c_str()));

        if (!iFile.is_open())
        {
            BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("Cannot open CRU config file"));
        }

        std::stringstream iBuffer;
        iBuffer << iFile.rdbuf();
        std::string fileData = iBuffer.str();

        Sca::registers["'add_gbt_sc'"] = getRegisterAddress(fileData, "'add_gbt_sc'");
        Sca::registers["'add_gbt_sc_rst'"] = getRegisterAddress(fileData, "'add_gbt_sc_rst'");
        Sca::registers["'add_gbt_sc_link'"] = getRegisterAddress(fileData, "'add_gbt_sc_link'");
        Sca::registers["'add_gbt_sca_wr_data'"] = getRegisterAddress(fileData, "'add_gbt_sca_wr_data'");
        Sca::registers["'add_gbt_sca_wr_ctr'"] = getRegisterAddress(fileData, "'add_gbt_sca_wr_ctr'");
        Sca::registers["'add_gbt_sca_wr_cmd'"] = getRegisterAddress(fileData, "'add_gbt_sca_wr_cmd'");
        Sca::registers["'add_gbt_sca_rd_mon'"] = getRegisterAddress(fileData, "'add_gbt_sca_rd_mon'");
        Sca::registers["'add_gbt_sca_rd_data'"] = getRegisterAddress(fileData, "'add_gbt_sca_rd_data'");
        Sca::registers["'add_gbt_sca_rd_ctr'"] = getRegisterAddress(fileData, "'add_gbt_sca_rd_ctr'");
        Sca::registers["'add_gbt_sca_rd_cmd'"] = getRegisterAddress(fileData, "'add_gbt_sca_rd_cmd'");

        iFile.close();
    }
}

uint32_t Sca::getRegisterAddress(std::string& data, const std::string& reg)
{
    size_t pos = data.find(reg);
    if (pos == std::string::npos)
    {
        BOOST_THROW_EXCEPTION(ScaException() << ErrorInfo::Message("Cannot find register in CRU config file"));
    }

    size_t begin = data.find("0x", pos) + 2;
    size_t end = data.find(",", begin);

    return (uint32_t)std::stoul(data.substr(begin, end - begin), NULL, 16) / 4;
}

} // namespace Alf
} // namespace CommandLineUtilities
} // namespace roc
} // namespace AliceO2
