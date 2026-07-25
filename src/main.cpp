#include <circle/startup.h>
#include <circle/koptions.h>
#include <circle/device.h>
#include <circle/devicenameservice.h> // Required header
#include <circle/2dgraphics.h>
#include <circle/screen.h>
#include <circle/logger.h>
#include <circle/types.h>

class CQemuSerialDevice : public CDevice
{
public:
    CQemuSerialDevice(void) {}

    int Write(const void *pBuffer, size_t nCount) override
    {
        volatile unsigned int * const UART0_DR = (unsigned int *)0xFE201000;
        volatile unsigned int * const UART0_FR = (unsigned int *)0xFE201018;

        const char *pChar = (const char *)pBuffer;
        for (size_t i = 0; i < nCount; i++)
        {
            while (*UART0_FR & (1 << 5))
            {
                asm volatile("nop");
            }
            *UART0_DR = pChar[i];
        }
        return (int)nCount;
    }

    int Read(void *pBuffer, size_t nCount) override { return -1; }
};

class CKernel
{
public:
    CKernel(void)
        : m_Options(),
          m_DeviceNameService(),
          m_Graphics(m_Options.GetWidth(), m_Options.GetHeight()),
          m_Serial(),
          m_Logger(m_Options.GetLogLevel(), nullptr)
    {
    }

    bool Initialize(void)
    {
        if (!m_Logger.Initialize(&m_Serial)) return false;

        m_Logger.Write("CKernel", LogNotice, "Initializing C2DGraphics...");

        // Initialize the graphics subsystem and underlying framebuffer
        if (!m_Graphics.Initialize())
        {
            m_Logger.Write("CKernel", LogError, "C2DGraphics failed to initialize!");
        }
        
        return true;
    }

    void Run(void)
    {
        T2DColor bgColor   = (T2DColor)0xFF000080; // Dark Blue
        T2DColor redColor  = (T2DColor)0xFFFF0000; // Solid Red
        T2DColor textColor = (T2DColor)0xFFFFFFFF; // White

        // 1. Clear the screen to a background color (ARGB hex format)
        m_Graphics.ClearScreen(bgColor);

        // 2. Draw a solid red rectangle in the top right
        unsigned rectWidth = 100;
        unsigned rectHeight = 100;
        unsigned startX = m_Graphics.GetWidth() - rectWidth - 20;
        unsigned startY = 20;
        
        // C2DGraphics provides standard primitive functions
        m_Graphics.DrawRect(startX, startY, rectWidth, rectHeight, redColor);

        // 3. Draw text natively using Circle's built-in font renderer
        m_Graphics.DrawText(20, 20, textColor, "Circle C2DGraphics Engine Online!");

        // 4. Swap buffers (pushes the drawn frame to the screen)
        m_Graphics.UpdateDisplay();

        unsigned nCounter = 0;
        while (1)
        {
            //m_Logger.Write("CKernel", LogNotice, "Heartbeat tick: %u", nCounter++);
            for (volatile int i = 0; i < 20000000; i++) asm volatile("nop");
        }
    }

private:
    CKernelOptions     m_Options;
    CDeviceNameService m_DeviceNameService; // Declared BEFORE m_Screen
    C2DGraphics        m_Graphics;
    CQemuSerialDevice  m_Serial;
    CLogger            m_Logger;
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