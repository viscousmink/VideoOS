#include <circle/startup.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/device.h>
#include <circle/logger.h>
#include <circle/types.h>

// Minimal QEMU-compatible serial device deriving directly from CDevice
class CQemuSerialDevice : public CDevice
{
public:
    CQemuSerialDevice(void) {}

    // Pure virtual overrides from CDevice
    int Write(const void *pBuffer, size_t nCount) override
    {
        volatile unsigned int * const UART0_DR = (unsigned int *)0xFE201000;
        volatile unsigned int * const UART0_FR = (unsigned int *)0xFE201018;

        const char *pChar = (const char *)pBuffer;
        for (size_t i = 0; i < nCount; i++)
        {
            // Wait while TX FIFO is full (bit 5)
            while (*UART0_FR & (1 << 5))
            {
                asm volatile("nop");
            }
            *UART0_DR = pChar[i];
        }
        return (int)nCount;
    }

    int Read(void *pBuffer, size_t nCount) override
    {
        return -1; // Read not implemented
    }
};

class CKernel
{
public:
    CKernel(void)
        : m_Options(),
          m_Serial(),
          m_Logger(m_Options.GetLogLevel(), nullptr) // nullptr timer for polled logging
    {
    }

    bool Initialize(void)
    {
        // Attach our custom QEMU serial writer to Circle's CLogger
        if (!m_Logger.Initialize(&m_Serial))
        {
            return false;
        }
        return true;
    }

    void Run(void)
    {
        m_Logger.Write("CKernel", LogNotice, "========================================");
        m_Logger.Write("CKernel", LogNotice, "  Circle Subsystem Initialized (QEMU)!  ");
        m_Logger.Write("CKernel", LogNotice, "  Running on Raspberry Pi 4 (AArch64)   ");
        m_Logger.Write("CKernel", LogNotice, "========================================");

        unsigned nCounter = 0;
        while (1)
        {
            m_Logger.Write("CKernel", LogNotice, "Heartbeat tick: %u", nCounter++);

            for (volatile int i = 0; i < 20000000; i++)
            {
                asm volatile("nop");
            }
        }
    }

private:
    CKernelOptions    m_Options;
    CQemuSerialDevice m_Serial;
    CLogger           m_Logger;
};

int main(void)
{
    CKernel Kernel;
    if (Kernel.Initialize())
    {
        Kernel.Run();
    }

    return 0;
}

/*int main(void)
{
    volatile unsigned int * const UART0_DR = (unsigned int *)0xFE201000;
    volatile unsigned int * const UART0_FR = (unsigned int *)0xFE201018;

    auto write_char = [&](char c) {
        // Wait while Transmit FIFO is Full (TXFF bit 5 is set)
        while (*UART0_FR & (1 << 5)) {
            asm volatile("nop");
        }
        *UART0_DR = c;
    };

    const char *msg = "\r\n*** HELLO FROM RASPBERRY PI 4 BARE-METAL ***\r\n";
    for (const char *p = msg; *p; p++) {
        write_char(*p);
    }

    while (1) {}
}*/