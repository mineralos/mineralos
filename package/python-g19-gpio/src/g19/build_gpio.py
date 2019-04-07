import os
from cffi import FFI
ffi = FFI()

path = os.path.dirname(os.path.realpath(__file__))

with open(os.path.join(path, 'gpio.c'),'r') as f:
    ffi.set_source("g19._gpio", f.read(), libraries=["c"])

ffi.cdef(
    """
    void Xil_Mmap(void);
    void Xil_Out32(uint32_t phyaddr, uint32_t val);
    int Xil_In32(uint32_t phyaddr);
    void GPIOGreenLedOn(void);
    void GPIOGreenLedOff(void);
    void GPIORedLedOn(void);
    void GPIORedLedOff(void);
    int GetKey_IPSet(void);
    void set_en_core(uint32_t spi_id, uint32_t value);
    void set_led(uint32_t spi_id, uint32_t mode, uint32_t led_delay);
    """
)

if __name__ == "__main__":
    ffi.compile()