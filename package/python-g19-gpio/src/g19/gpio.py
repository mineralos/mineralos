from ._gpio import ffi, lib

lib.Xil_Mmap()

def GPIOGreenLedOn():
	lib.GPIOGreenLedOn()

def GPIOGreenLedOff():
	lib.GPIOGreenLedOff()

def GPIORedLedOn():
	lib.GPIORedLedOn()

def GPIORedLedOff():
	lib.GPIORedLedOff()

def GetKey_IPSet():
	return lib.GetKey_IPSet()
