#include <circle/startup.h>
#include <circle/koptions.h>
#include <circle/device.h>
#include <circle/devicenameservice.h>
#include <circle/2dgraphics.h>
#include <circle/logger.h>
#include <circle/types.h>

// File system and hardware includes
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <SDCard/emmc.h>       // Correct EMMC Header
#include <circle/fs/fat/fatfs.h>       // Correct FATFS Header
#include <fatfs/ff.h>

#include <stdint.h>

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
            while (*UART0_FR & (1 << 5)) { asm volatile("nop"); }
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
          m_Logger(m_Options.GetLogLevel(), nullptr),
          m_Interrupt(),
          m_Timer(&m_Interrupt),
          m_EMMC(&m_Interrupt, &m_Timer)
    {
    }

    bool Initialize(void)
    {
        if (!m_Logger.Initialize(&m_Serial)) return false;

        m_Interrupt.Initialize();
        m_Timer.Initialize();
        m_Graphics.Initialize();

        m_Logger.Write("CKernel", LogNotice, "Initializing SD Card...");
        
        if (!m_EMMC.Initialize())
        {
            m_Logger.Write("CKernel", LogError, "EMMC (SD Card) initialization failed!");
        }
        else
        {
            m_Logger.Write("CKernel", LogNotice, "FAT File System Mounted Successfully!");
        }
        
        return true;
    }

    void DrawBMP(const char* filename, unsigned startX, unsigned startY)
    {
        FIL file;
        
        // f_open replaces fopen
        FRESULT res = f_open(&file, "SD:/image.bmp", FA_READ);
        if (res != FR_OK)
        {
            m_Logger.Write("CKernel", LogError, "Could not open %s (Error %d)", filename, res);
            return;
        }

        uint8_t header[54];
        UINT bytesRead;
        
        // f_read replaces fread
        f_read(&file, header, 54, &bytesRead);
        
        if (bytesRead != 54 || header[0] != 'B' || header[1] != 'M')
        {
            m_Logger.Write("CKernel", LogError, "Invalid BMP file");
            f_close(&file);
            return;
        }

        uint32_t dataOffset = *(uint32_t*)&header[10];
        uint32_t width      = *(uint32_t*)&header[18];
        int32_t  height     = *(int32_t*)&header[22];
        uint16_t bpp        = *(uint16_t*)&header[28];

        if (bpp != 24)
        {
            m_Logger.Write("CKernel", LogError, "Only 24-bit BMPs are supported");
            f_close(&file);
            return;
        }

        // f_lseek replaces fseek
        f_lseek(&file, dataOffset);
        
        uint32_t rowPadding = (4 - (width * 3) % 4) % 4;
        uint8_t* rowBuffer = new uint8_t[width * 3 + rowPadding];

        for (int y = height - 1; y >= 0; y--)
        {
            f_read(&file, rowBuffer, width * 3 + rowPadding, &bytesRead);
            
            for (unsigned x = 0; x < width; x++)
            {
                uint8_t b = rowBuffer[x * 3 + 0];
                uint8_t g = rowBuffer[x * 3 + 1];
                uint8_t r = rowBuffer[x * 3 + 2];
                
                T2DColor color = (T2DColor)(0xFF000000 | (r << 16) | (g << 8) | b);
                m_Graphics.DrawPixel(startX + x, startY + y, color);
            }
        }

        delete[] rowBuffer;
        
        // f_close replaces fclose
        f_close(&file);
        m_Logger.Write("CKernel", LogNotice, "BMP drawn successfully!");
    }

    void Run(void)
    {
        // Paint screen Dark Gray using standard DrawPixel loop
        T2DColor bgColor = (T2DColor)0xFF202020;
        for (unsigned y = 0; y < m_Graphics.GetHeight(); y++)
        {
            for (unsigned x = 0; x < m_Graphics.GetWidth(); x++)
            {
                m_Graphics.DrawPixel(x, y, bgColor);
            }
        }
        
        m_Graphics.DrawText(20, 20, (T2DColor)0xFFFFFFFF, "Searching for image.bmp on SD Card...");
        m_Graphics.UpdateDisplay();

        // Decode and draw the image!
        DrawBMP("image.bmp", 50, 60);
        
        m_Graphics.UpdateDisplay();

        while (1) { asm volatile("nop"); }
    }

private:
    CKernelOptions     m_Options;
    CDeviceNameService m_DeviceNameService;
    C2DGraphics        m_Graphics;
    CQemuSerialDevice  m_Serial;
    CLogger            m_Logger;
    
    CInterruptSystem   m_Interrupt;
    CTimer             m_Timer;
    CEMMCDevice        m_EMMC;
    FATFS              m_FileSystem;
};

int main(void)
{
    CKernel Kernel;
    if (Kernel.Initialize()) { Kernel.Run(); }
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