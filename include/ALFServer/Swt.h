/// \file Swt.h
/// \brief Definition of ALICE Lowlevel Frontend (ALF) SWT operations
///
/// \author Milan Tkacik (milan.tkacik@cern.ch)

#ifndef ALICEO2_READOUTCARD_UTILITIES_ALF_ALICELOWLEVELFRONTEND_SWT_H
#define ALICEO2_READOUTCARD_UTILITIES_ALF_ALICELOWLEVELFRONTEND_SWT_H

#include <string>
#include "ReadoutCard/CardType.h"
#include "ReadoutCard/RegisterReadWriteInterface.h"

namespace AliceO2 {
namespace roc {
namespace CommandLineUtilities {
namespace Alf {

/// Class for interfacing with the C-RORC's and CRU's Slow-Control Adapter (SCA)
class Swt
{
  public:
    struct SwtData
    {
        uint32_t data[3];
    };

    /// \param bar2 SWT is on BAR 2
    Swt(RegisterReadWriteInterface& bar2, int gbtChannel);

    uint32_t write(const uint32_t data[3]);
    uint32_t write(const SwtData data);
    uint32_t read(uint32_t data[3]);
    SwtData read();
    void reset();

  private:
    /// Interface for BAR 2
    RegisterReadWriteInterface& mBar2;
    int gbtChannel;

    void barWrite(int index, uint32_t data);
    uint32_t barRead(int index);
};


} // namespace Alf
} // namespace CommandLineUtilities
} // namespace roc
} // namespace AliceO2

#endif // ALICEO2_READOUTCARD_UTILITIES_ALF_ALICELOWLEVELFRONTEND_SWT_H
