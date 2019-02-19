/// \file Sca.h
/// \brief Definition of ALICE Lowlevel Frontend (ALF) SCA operations
///
/// \author Pascal Boeschoten (pascal.boeschoten@cern.ch)

#ifndef ALICEO2_READOUTCARD_UTILITIES_ALF_ALICELOWLEVELFRONTEND_SCA_H
#define ALICEO2_READOUTCARD_UTILITIES_ALF_ALICELOWLEVELFRONTEND_SCA_H

#include <string>
#include <map>
#include "ReadoutCard/CardType.h"
#include "ReadoutCard/RegisterReadWriteInterface.h"

namespace AliceO2 {
namespace roc {
namespace CommandLineUtilities {
namespace Alf {

/// Class for interfacing with the C-RORC's and CRU's Slow-Control Adapter (SCA)
class Sca
{
  public:
    struct ReadResult
    {
        uint32_t command;
        uint32_t data;
    };

    struct CommandData
    {
        uint32_t command;
        uint32_t data;
    };

    /// \param bar2 SCA is on BAR 2
    /// \param cardType Needed to get offset for SCA registers
    /// \param link Needed to get offset for SCA registers
    Sca(RegisterReadWriteInterface& bar2, CardType::type cardType, int link);

    void initialize();
    void write(uint32_t command, uint32_t data);
    void write(CommandData commandData)
    {
        write(commandData.command, commandData.data);
    }
    ReadResult read();
    ReadResult gpioRead();
    ReadResult gpioWrite(uint32_t data);

  private:
    void init();
    void gpioEnable();
    void barWrite(uint32_t index, uint32_t data);
    uint32_t barRead(uint32_t index);
    void executeCommand();
    void waitOnBusyClear();
    void checkError(uint32_t command);
    bool isChannelBusy(uint32_t command);

    /// Interface for BAR 2
    RegisterReadWriteInterface& mBar2;

    static std::map<std::string, uint32_t> registers;
    static void loadRegisters();
    static uint32_t getRegisterAddress(std::string& data, const std::string& reg);
};


} // namespace Alf
} // namespace CommandLineUtilities
} // namespace roc
} // namespace AliceO2

#endif // ALICEO2_READOUTCARD_UTILITIES_ALF_ALICELOWLEVELFRONTEND_SCA_H
